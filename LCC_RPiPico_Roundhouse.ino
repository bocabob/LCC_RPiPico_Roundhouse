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
 * \file BasicNode.ino
 *
 * This sketh will create a very basic OpenLcb Node.  
 *
 * @author Jim Kueneman, Bob Gamble, David Harris, and others
 * @date 14 Mar 2025
 */

#include "Arduino.h"
#include <Wire.h>
#include <LibPrintf.h>

#include "BoardSettings.h"
// #include "TTconfig.h"
#include "TTvariables.h"

#include "callbacks.h"
#include "openlcb_user_config.h"
#include "config_mem_helper.h"

#include "src/pico/rpi_pico_drivers.h"
#include "src/pico/rpi_pico_can_drivers.h"

#include "src/drivers/canbus/can_config.h"
#include "src/openlcb/openlcb_config.h"
#include "src/openlcb/openlcb_application.h"
#include "src/utilities/mustangpeak_endian_helper.h"
#include "src/openlcb/openlcb_application_broadcast_time.h"

#include "Roundhouse.h"
#include "TTcomms.h"

// #define NODE_ID 0x050101010777
#define NODE_ID 0x050101019416      // 05 01 01 01 94 ** range assigned to Bob Gamble / Southern Piedmont
// #define NODE_ADDRESS  5,1,1,1,94,0x08   // 05 01 01 01 94 ** range assigned to Bob Gamble / Southern Piedmont

// MemStruct CDI_RAM;
// extern bool GroupState[MAX_GROUPS];   // defined in NPlights.cpp

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
    .firmware_write                  = &Callbacks_write_firemware,
    .on_100ms_timer                  = &Callbacks_on_100ms_timer_callback,
    .on_login_complete               = &Callbacks_on_login_complete,
    .on_consumed_event_identified    = &Callbacks_on_consumed_event_identified,
    .on_consumed_event_pcer          = &Callbacks_on_consumed_event_pcer,
    .on_broadcast_time_changed       = &Callbacks_on_broadcast_time_time_changed,
};

void _check_for_nvm_initialization(void) {

  Serial.println("Checking for initialized NVM");

  if (!ConfigMemHelper_nvm_is_accessible()) {
    Serial.println("FATAL: NVM not accessible - check I2C wiring, address (0x50), and pullups on SDA/SCL");
    while (true) { delay(1000); }  // halt
  }

  // If the first byte of the configuration memory is 0xFF then the space has never been accessed (fresh firmware load) and need to be initialized to 0x00
  if (ConfigMemHelper_is_config_mem_reset()) {

    Serial.println("Initializing Configuration Memory to 0x00");
    ConfigMemHelper_clear_config_mem();
    Serial.println("Writing default values to the Configuration Memory");
    ConfigMemHelper_reset_and_write_default(OpenLcbUserConfig_node_id);
    Serial.println("Defaults set...");

  } else {

    Serial.println("Configuration Memory has been previously initalized");

  }

}

void _register_producers(void) {

  OpenLcbApplication_clear_producer_eventids(OpenLcbUserConfig_node_id);

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

extern bool lastRunningState;   // Stores last running state to allow turning the stepper off after moves.
bool setupComplete = false;
bool setup1Complete = false;
bool StorageReady = false;

npStrings _Strings[MAX_STRINGS];



void setup() {
  // put your setup code here, to run once:

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
  OpenLcb_initialize(&openlcb_config);

  Callbacks_initialize();

  Serial.println("Creating Node.....");

  OpenLcbUserConfig_node_id = OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);
  // do this after initialization or the I2C will not be initialized

  _check_for_nvm_initialization();

  // Read the NVM into the local data structures
  Serial.println("Loading NVM values into Config Mem data variable");
  ConfigMemHelper_read(OpenLcbUserConfig_node_id, &ConfigMemHelper_config_data);
  Set_Application_Values_From_Config(OpenLcbUserConfig_node_id, &ConfigMemHelper_config_data);
  Serial.println("Data variable loaded and ready for use");
  
  // initStringFlags();
  // SetupPixels();
  // InitialzePixels();  // TODO: JDK THIS CAUSED A HANG ON MY BOARD.... 

  setServoDefaults();
	notice("Servos Set to Defaults");
  setupServos();
	
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
  // put your setup code here, to run once:
  // while (!Serial) {}
  delay(1000);  /* wait */
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
      case 't':    
        broadcast_clock_state_t* clock = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

        // clock->is_running = true;

        printf("Current time: %02d:%02d\n", clock->time.hour, clock->time.minute);  
      break;
    };  
  }

  // // put your main code here, to run repeatedly
  RPiPicoCanDriver_process_receive();

  OpenLcb_run();
  // ProcessPixels();
}

// ==== Loop Two for node function processes ==========================
void loop1() {
  static long nextdot = 0;
  if(millis()>nextdot) {
    nextdot = millis()+2000;
    //dP("\n.");
  }
    
  // ResetStrings(); // ResetDirty()
  // ProcessPixels();
  driveServos();
  
}