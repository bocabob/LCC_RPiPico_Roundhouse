/** \copyright
 * Copyright (c) 2025, Jim Kueneman, Bob Gamble, David Harris, and others
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file LCC_RPiPico_Roundhouse.ino
 *
 * This sketch creates an OpenLcb Roundhouse controller node.
 *
 * @author Jim Kueneman, Bob Gamble, David Harris, and others
 * @date 14 Mar 2025
 */

#include "Arduino.h"
#include <Wire.h>
#include <stdlib.h>
#include <LibPrintf.h>

#include "BoardSettings.h"
#include "TTvariables.h"

#include "callbacks.h"
#include "openlcb_user_config.h"
#include "config_mem_helper.h"
#include "NodeIdentity.h"

#include "src/pico/rpi_pico_drivers.h"
#include "src/pico/rpi_pico_can_drivers.h"

#include "src/drivers/canbus/can_config.h"
#include "src/openlcb/openlcb_config.h"
#include "src/openlcb/openlcb_application.h"
#include "src/utilities/mustangpeak_endian_helper.h"
#include "src/openlcb/openlcb_application_broadcast_time.h"

#include "Roundhouse.h"
#include "TTcomms.h"

// Fallback used only until the protected NVM identity block is provisioned
// (see NodeIdentity.h / LCC_NODE_STANDARD.md §7.1). Provision a permanent ID
// with the 'N' serial command rather than relying on this for new boards.
#define NODE_ID_DEFAULT 0x050101019436      // 05 01 01 01 94 ** range assigned to Bob Gamble / Southern Piedmont

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &RPiPicoCanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &RPiPicoCanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &RPiPicoDrivers_lock_shared_resources,
    .unlock_shared_resources = &RPiPicoDrivers_unlock_shared_resources,
    .on_rx                   = &Callbacks_on_can_rx_callback,
    .on_tx                   = &Callbacks_on_can_tx_callback,
    .on_alias_change         = &Callbacks_alias_change_callback,
};

static const openlcb_config_t openlcb_config = {
    .lock_shared_resources           = &RPiPicoDrivers_lock_shared_resources,
    .unlock_shared_resources         = &RPiPicoDrivers_unlock_shared_resources,
    .config_mem_read                 = &ConfigMemHelper_config_mem_read,
    .config_mem_write                = &ConfigMemHelper_config_mem_write,
    .reboot                          = &RPiPicoDrivers_reboot,
    .factory_reset                   = &Callbacks_operations_request_factory_reset,
    .freeze                          = &Callbacks_freeze,
    .unfreeze                        = &Callbacks_unfreeze,
    .firmware_write                  = &Callbacks_write_firmware,
    .on_100ms_timer                  = &Callbacks_on_100ms_timer_callback,
    .on_login_complete               = &Callbacks_on_login_complete,
    .on_consumed_event_identified    = &Callbacks_on_consumed_event_identified,
    .on_consumed_event_pcer          = &Callbacks_on_consumed_event_pcer,
    .on_broadcast_time_changed       = &Callbacks_on_broadcast_time_time_changed,
};

// Factory-reset gesture: hold Blue + Gold buttons for 2s at boot to wipe and
// reinitialize configuration memory to CDI defaults. Does NOT touch the
// protected NVM identity region above CONFIG_MEM_SIZE (§7.1) — node ID
// survives this reset, same as it survives the 'r'/'i' serial commands.
// No-op if BLUE_BUTTON_PIN/GOLD_BUTTON_PIN are unavailable on this board's
// active breakout combo (shared with another function) — see NodeConfig.h.
void _check_factory_reset_gesture(void) {
#if defined(BLUE_BUTTON_PIN) && defined(GOLD_BUTTON_PIN)
  pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GOLD_BUTTON_PIN, INPUT_PULLUP);

  if (digitalRead(BLUE_BUTTON_PIN) != LOW || digitalRead(GOLD_BUTTON_PIN) != LOW) {
    return;  // not held — normal boot
  }

  Serial.println("Blue+Gold held at boot — hold 2s to wipe and reinitialize NVM (release to cancel)...");
  uint32_t startMs = millis();
  while (digitalRead(BLUE_BUTTON_PIN) == LOW && digitalRead(GOLD_BUTTON_PIN) == LOW) {
    if (millis() - startMs >= 2000) {
      Serial.println("Wiping configuration memory to factory defaults...");
      ConfigMemHelper_reset_config_mem();
      ConfigMemHelper_reset_and_write_default(OpenLcbUserConfig_node_id);
      Serial.println("NVM wiped and reinitialized. Continuing boot...");
      return;
    }
    delay(20);
  }
  Serial.println("Released early — factory reset cancelled.");
#endif
}

void _check_for_nvm_initialization(void) {

  Serial.println("Checking for initialized NVM");

  if (!ConfigMemHelper_nvm_is_accessible()) {
    Serial.println("FATAL: NVM not accessible - check I2C wiring, address (0x50), and pullups on SDA/SCL");
    // while (true) { delay(1000); }  // halt
  }

  // If the first byte of the configuration memory is 0xFF then the space has never been accessed (fresh firmware load) and need to be initialized to 0x00
  if (ConfigMemHelper_is_config_mem_reset()) {

    Serial.println("Initializing Configuration Memory to 0x00");
    ConfigMemHelper_clear_config_mem();
    Serial.println("Writing default values to the Configuration Memory");
    ConfigMemHelper_reset_and_write_default(OpenLcbUserConfig_node_id);
    Serial.println("Defaults set...");

  } else {

    Serial.println("Configuration Memory has been previously initialized");

  }

}

void _register_producers(void) {
  // Register each door's ToggleDoor event as a producer so that the OpenLCB
  // stack will broadcast a "Producer Identified" message during LCB login,
  // carrying the NVM-restored open/closed state.  The Turntable node (and any
  // other node on the bus) will receive these messages to sync their displays.
  OpenLcbApplication_clear_producer_eventids(OpenLcbUserConfig_node_id);

  for (int i = 0; i < ConfigMemHelper_config_data.attributes.DoorCount; i++) {
    OpenLcbApplication_register_producer_eventid(
        OpenLcbUserConfig_node_id,
        swap_endian64(ConfigMemHelper_config_data.attributes.doors[i].ToggleDoor),
        ConfigMemHelper_config_data.producer_status[i]);  // NVM-restored SET/CLEAR/UNKNOWN
  }
}

void _register_consumers(void) {
  int index = 0;
  OpenLcbApplication_clear_consumer_eventids(OpenLcbUserConfig_node_id);

  OpenLcbApplication_register_consumer_eventid(OpenLcbUserConfig_node_id, swap_endian64(ConfigMemHelper_config_data.attributes.OpenAll), ConfigMemHelper_config_data.consumer_status[index++]);  // need to read the state from the NVM to know if it is on/off/unknown when registering the consumer event ID
  OpenLcbApplication_register_consumer_eventid(OpenLcbUserConfig_node_id, swap_endian64(ConfigMemHelper_config_data.attributes.CloseAll), ConfigMemHelper_config_data.consumer_status[index++]);

  for (int i = 0; i < ConfigMemHelper_config_data.attributes.DoorCount; i++) {
     OpenLcbApplication_register_consumer_eventid(OpenLcbUserConfig_node_id, swap_endian64(ConfigMemHelper_config_data.attributes.doors[i].ToggleDoor), ConfigMemHelper_config_data.consumer_status[index++]);  // need to read the state from the NVM to know if it is on/off/unknown when registering the consumer event ID
  }
  
  OpenLcbApplication_register_consumer_eventid(OpenLcbUserConfig_node_id, swap_endian64(ConfigMemHelper_config_data.attributes.ToggleInterior), ConfigMemHelper_config_data.consumer_status[index++]);
  OpenLcbApplication_register_consumer_eventid(OpenLcbUserConfig_node_id, swap_endian64(ConfigMemHelper_config_data.attributes.ToggleExterior), ConfigMemHelper_config_data.consumer_status[index++]);
  OpenLcbApplication_register_consumer_eventid(OpenLcbUserConfig_node_id, swap_endian64(ConfigMemHelper_config_data.attributes.HighLuminosity_On), ConfigMemHelper_config_data.consumer_status[index++]);
  OpenLcbApplication_register_consumer_eventid(OpenLcbUserConfig_node_id, swap_endian64(ConfigMemHelper_config_data.attributes.LowLuminosity_On), ConfigMemHelper_config_data.consumer_status[index++]);
  
  
}

bool node_initiated = false;

////// DECLARATIONS

bool setupComplete = false;
bool setup1Complete = false;
bool StorageReady = false;

npStrings _Strings[MAX_STRINGS];



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  uint32_t stimer = millis();
  while (!Serial && (millis() - stimer < 3000)) {   // wait for 3 secs for USB/serial connection to be established
    delay(100);
  }

  Serial.println("Can Statemachine init.....");
  
  RPiPicoCanDriver_setup();
  RPiPicoDriver_setup();

  CanConfig_initialize(&can_config);
  OpenLcbConfig_initialize(&openlcb_config);

  Callbacks_initialize();

  Serial.println("Creating Node.....");

  uint64_t node_id = NodeIdentity_read();
  if (node_id == 0) {
    Serial.println("*** Node identity not provisioned in protected NVM ***");
    Serial.println("Using built-in default ID for this boot. Provision a permanent ID with 'N<12-hex-digit-id>' then 'Y' to confirm.");
    node_id = NODE_ID_DEFAULT;
  }
  OpenLcbUserConfig_node_id = OpenLcbConfig_create_node(node_id, &OpenLcbUserConfig_node_parameters);
  // do this after initialization or the I2C will not be initialized

  _check_factory_reset_gesture();

  _check_for_nvm_initialization();

  // Read the NVM into the local data structures
  Serial.println("Loading NVM values into Config Mem data variable");
  ConfigMemHelper_read(OpenLcbUserConfig_node_id, &ConfigMemHelper_config_data);
  Set_Application_Values_From_Config(OpenLcbUserConfig_node_id, &ConfigMemHelper_config_data);
  Serial.println("Data variable loaded and ready for use");
  
  setServoDefaults();
	notice("Servos Set to Defaults");
  setupServos();
  initializeHardware();

	notice("Roundhouse Program Started");
  setupComplete = true;

  Serial.println(F("Setup zero complete"));

  while(!setup1Complete);
  delay(10);
  // Now use the data found in the data structures to register the current event IDs
  // need to read states from NVM to know the state when registering the consumer event IDs
  _register_consumers();
  _register_producers();

  OpenLcbApplicationBroadcastTime_setup_consumer(OpenLcbUserConfig_node_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK); // initialize the fast clock

  node_initiated = true;
	notice("Roundhouse Node Initiated");
}


void setup1() {
  delay(1000);  // brief wait to let Core 0 setup() reach while(!setup1Complete)
  setup1Complete = true;
  Serial.println(F("Setup one complete"));

  while (!node_initiated)
  {
    delay(1000);  /* wait */
    Serial.print(" . ");
  }
  
  Serial.println("Loop 1 started \n");
}

// ==== Loop One for LCC processes ==========================
void loop() {

  if (Serial.available()) {

    switch (Serial.read()) {

      case 'h':
        Serial.println("'h': Help");
        Serial.println("'c': Setting NVM to 0x00");
        Serial.println("'i': Resetting NVM to CDI default values");
        Serial.println("'p': Toggle Message Logging");
        Serial.println("'r': Resetting NVM to 0xFF for a fresh boot");
        Serial.println("'m': Toggle Config Mem read/write Logging");
        Serial.println("'t': Display current time");
        Serial.println("'q': Query current time");
        Serial.println("'N<12 hex digits>': Provision node identity, e.g. N050101019436 — confirm with 'Y'");
      break;
      case 'c':
        Serial.println("Setting NVM to 0x00...");
        ConfigMemHelper_clear_config_mem();  // reset all EEPROM to initalized nulls 0x00
        Serial.println("Setting NVM to 0x00 COMPLETE");
      break;
      case 'i':
        Serial.println("Resetting NVM to default values...");
        ConfigMemHelper_reset_and_write_default(OpenLcbUserConfig_node_id);  // reset all EEPROM to CDI defined defaults
        Serial.println("Resetting NVM to default values COMPLETE");
      break;
      case 'r':
        Serial.println("Setting NVM to 0xFF (factory fresh configuration...)");
        ConfigMemHelper_reset_config_mem();  // reset all EEPROM to factory new 0xFF
        Serial.println("Setting NVM to 0xFF (factory fresh configuration) COMPLETE");
      break;
      case 'p':
        Serial.print("Toggling Message Logging...)");
        Serial.println();
        Callbacks_toggle_log_messages(); 
      break;
      case 'm':
        Serial.print("Toggling Config Mem Logging...");
        Serial.println();
        if (ConfigMemHelper_toggle_log_access()) {
          Serial.println("Logging Access");
        } else {
          Serial.println("Not Logging Access");
        }
      break;
      case 'q':
        OpenLcbApplicationBroadcastTime_send_query(OpenLcbUserConfig_node_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
      break;
      case 't': {
        broadcast_clock_state_t* clock = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

        // clock->is_running = true;

        printf("Current time: %02d:%02d\n", clock->time.hour, clock->time.minute);
      }
      break;
      case 'N': {
        char hexbuf[13];
        size_t n = Serial.readBytesUntil('\n', hexbuf, 12);
        if (n > 0 && hexbuf[n - 1] == '\r') n--;
        hexbuf[n] = '\0';
        if (n != 12) {
          Serial.println("Usage: N<12 hex digits>, e.g. N050101019436");
        } else {
          uint64_t id = strtoull(hexbuf, NULL, 16);
          NodeIdentity_begin_provision(id);
          Serial.print("Confirm with 'Y' to write node ID 0x");
          Serial.println(hexbuf);
        }
      }
      break;
      case 'Y':
        if (NodeIdentity_provision_pending()) {
          if (NodeIdentity_confirm_provision()) {
            // Config memory (including node-ID-based default event IDs) was
            // written using whatever ID was active at the time — NODE_ID_DEFAULT
            // if this is a fresh board provisioned before its first boot's
            // defaults were ever written. Wipe to 0xFF (fresh) so
            // _check_for_nvm_initialization() rewrites the defaults next boot
            // using the just-provisioned real ID. Safe: the identity block
            // itself lives in the protected NVM region above CONFIG_MEM_SIZE,
            // untouched by this reset.
            ConfigMemHelper_reset_config_mem();
            Serial.println("Node identity written. Rebooting to regenerate event IDs from the new node ID...");
            delay(100);
            rp2040.reboot();
          } else {
            Serial.println("Failed to write node identity.");
          }
        } else {
          Serial.println("No pending provisioning request.");
        }
      break;
    };
  }

  RPiPicoCanDriver_process_receive();

  OpenLcbConfig_run();
}

// ==== Loop Two for node function processes ==========================
void loop1() {
  driveServos();
  
}