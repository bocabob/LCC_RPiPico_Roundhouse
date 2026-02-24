#include <sys/_stdint.h>
/**
 * Configuration Memory Helper Implementation
 * Auto-generated from OpenLCB configuration
 *
 * Uses chunked memory read/write approach:
 * - OpenLCB datagrams limited to 64 bytes max payload
 * - Large structs read/written in 64-byte chunks
 * - All segments in address space 0xFD (Configuration)
 */

#include "Arduino.h"
#include <LibPrintf.h>
#include "config_mem_helper.h"
#include "node_parameters.h"
#include "src/openlcb/openlcb_application.h"
#include "src/application_drivers/rpi_pico_drivers.h"
#include "src/utilities/mustangpeak_endian_helper.h"
#include <string.h>
#include <stdio.h>

static bool _direct_access = false;

config_mem_t ConfigMemHelper_config_data;
bool ConfigMemHelper_log_access = false;

bool ConfigMemHelper_toggle_log_access(void) {

  ConfigMemHelper_log_access = !ConfigMemHelper_log_access;

  return ConfigMemHelper_log_access;

}

static void _load_defaults_node(openlcb_node_t *openlcb_node, config_mem_t *config, uint16_t *consumer_index, uint16_t *producer_index) {

  const char *name_def = "Southern Piedmont";
  strncpy(config->nodeid.node_name, name_def, sizeof(config->nodeid.node_name)); // Will pad with nulls
  const char *descript_def = "Roundhouse Controller Node";
  strncpy(config->nodeid.node_description, descript_def, sizeof(config->nodeid.node_description)); // Will pad with nulls

}

static void _load_defaults_reset_control(openlcb_node_t *openlcb_node, config_mem_t *config, uint16_t *consumer_index, uint16_t *producer_index) {

  config->reset_control.flag = 238;
  
}

static void _load_defaults_attributes(openlcb_node_t *openlcb_node, config_mem_t *config, uint16_t *consumer_index, uint16_t *producer_index) {

  config->attributes.TrackCount = NUM_TRACKS;
  config->attributes.HomeTrack = 3; // Default home track is track 4 (0 indexed)
  config->attributes.EnableReference = false; // Disable reference correction by default
  config->attributes.Rehome = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++;// EventID for rehome
  config->attributes.IncrementTrack = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for increment track
  config->attributes.DecrementTrack = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for decrement track
  config->attributes.RotateTrack180 = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for rotate track 180
  config->attributes.ToggleBridgeLights = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for toggle bridge lights
  for (int t = 0; t < MAX_TRACKS; t++) {
    config->attributes.tracks[t].trackName = TrackName[t];
    config->attributes.tracks[t].trackShort = TrackTag[t];
    config->attributes.tracks[t].Front = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for track front
    config->attributes.tracks[t].Back = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for track back
    config->attributes.tracks[t].steps = (t+1)*100; // Default position in steps for each track
  }
  config->attributes.DoorCount = 10;
  config->attributes.OpenAll = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for open all doors
  config->attributes.CloseAll = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for close all doors
  for (int d = 0; d < MAX_DOORS; d++) {
    config->attributes.doors[d].doorName = "Door";
    config->attributes.doors[d].doorShort = "D";
    config->attributes.doors[d].eidToggle = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for toggle door
    config->attributes.doors[d].TrackLocation = d; // Default track location for each door
  }
  config->attributes.eidBridge = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for toggle bridge lights
  config->attributes.eidInterior = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for toggle interior lights
  config->attributes.eidExterior = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for toggle exterior lights
  config->attributes.HighLuminosity = 255; // Max brightness when dimmer is off
  config->attributes.eidHighLuminosity_On = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++; // EventID for high luminosity on
  config->attributes.LowLuminosity = 128; // Default brightness when dimmer is on (half brightness) 
  config->attributes.eidLowLuminosity_On = swap_endian64((openlcb_node->id << 16) + *consumer_index); (*consumer_index)++;  // EventID for low luminosity on
 
}


static void _load_defaults_status(openlcb_node_t *openlcb_node, config_mem_t *config, uint16_t *consumer_index, uint16_t *producer_index) {
/*

  uint8_t event_state[2+12+28]; // Array to hold the state of each event (on/off/unknown) for the 42 events defined in the configuration

*/
  for (int i = 0; i < (5+2*MAX_TRACKS+2+MAX_DOORS+5); i++) {
    config->consumer_status[i] = EVENT_STATUS_UNKNOWN; // Default event state is unknown (0)
  }

  for (int i = 0; i < 1; i++) {
    config->producer_status[i] = EVENT_STATUS_UNKNOWN; // Default event state is unknown (0)
  }

}

uint16_t ConfigMemHelper_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

  // Hook into the Configuration Memory Write to update the data structures in parallel

  // Are we in the internal process of syncing the NMV with the struct?  If so just write to the NVM as we are syncing them.
  if (_direct_access) {  

    delay(10);

    return RPiPicoDrivers_config_mem_write(openlcb_node, address, count, buffer);

  }

  // This is a call from a Configuration Memory Protocol message from an external node or configuration tool so keep the NVM and data structure in sync
  
  if (ConfigMemHelper_log_access) {
    Serial.print("ConfigMemHelper_config_mem_write - Writing Address: ");
    Serial.print(address);
    Serial.print(", count: ");
    Serial.println(count);
  }
  
  // First write the value to the RAM structure
  uint8_t *byte_array = (uint8_t*) &ConfigMemHelper_config_data;
  byte_array += address;
  memcpy(byte_array, buffer, count);

  // Now write to the NVM
  return RPiPicoDrivers_config_mem_write(openlcb_node, address, count, buffer);

}

uint16_t ConfigMemHelper_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

  // Hook into the Configuration Memory Read to update the datastructures in parallel


  // Are we in the internal process of syncing the NMV with the struct?  If so just write to the NVM as we are syncing them.
  if (_direct_access) {

    delay(10);

    return RPiPicoDrivers_config_mem_read(openlcb_node, address, count, buffer);

  }

  // This is a call from a Configuration Memory Protocol message from an external node or configuration tool so the data structures should be in sync with NVM so just read what is in the buffers.

  if (ConfigMemHelper_log_access) {
    Serial.print("ConfigMemHelper_config_mem_read - Reading Address: ");
    Serial.print(address);
    Serial.print(", count: ");
    Serial.println(count);
  }

  uint8_t *byte_array = (uint8_t*) &ConfigMemHelper_config_data;
  byte_array += address;
  memcpy(buffer, byte_array, count);

  // Now read from the NVM
  // return RPiPicoDrivers_config_mem_read(openlcb_node, address, count, buffer);

  return count;

}

bool ConfigMemHelper_reset_and_write_default(openlcb_node_t *openlcb_node) {

  uint16_t consumer_index = 0;
  uint16_t producer_index = 0;

  // Just write this to the NVM don't try to keep the RAM buffer in sync, it is as we want it and just want that image in NVM
  _direct_access = true;

  _load_defaults_node(openlcb_node, &ConfigMemHelper_config_data, &consumer_index, &producer_index);
  _load_defaults_reset_control(openlcb_node, &ConfigMemHelper_config_data, &consumer_index, &producer_index);
  _load_defaults_attributes(openlcb_node, &ConfigMemHelper_config_data, &consumer_index, &producer_index);
  _load_defaults_status(openlcb_node, &ConfigMemHelper_config_data, &consumer_index, &producer_index);

  if (!ConfigMemHelper_write(openlcb_node, &ConfigMemHelper_config_data)) {

    Serial.println("Failed to write to ConfigMemHelper_write_node");
    _direct_access = false;
    return false;

  }

  _direct_access = false;
  return true;
}

/**
 * Read Config Mem from configuration memory
 *
 * Address: CONFIG_START_ADDR
 * Space: 0xFD (Configuration)
 * Type: config_mem_t
 */
bool ConfigMemHelper_read(openlcb_node_t *openlcb_node, config_mem_t *config) {
  if (!openlcb_node || !config) {
    return false;
  }

  uint32_t address = CONFIG_START_ADDR;  // Starting address from #define
  uint32_t total_size = sizeof(config_mem_t);
  uint32_t bytes_remaining = total_size;
  uint8_t *dest = (uint8_t *)config;

  configuration_memory_buffer_t temp_buffer;

  _direct_access = true;

  // Read in chunks (max 64 bytes per datagram)
  while (bytes_remaining > 0) {
    uint16_t chunk_size = (bytes_remaining > LEN_DATAGRAM_MAX_PAYLOAD)
                            ? LEN_DATAGRAM_MAX_PAYLOAD
                            : bytes_remaining;

    uint16_t bytes_read = ConfigMemHelper_config_mem_read(
      openlcb_node,
      address,
      chunk_size,
      &temp_buffer);

    if (bytes_read != chunk_size) {
      _direct_access = false;
      return false;  // Error or partial read
    }

    // Copy chunk to destination
    memcpy(dest, temp_buffer, chunk_size);

    // Advance pointers for next chunk
    address += chunk_size;
    dest += chunk_size;
    bytes_remaining -= chunk_size;
  }

  _direct_access = false;
  return true;
}

/**
 * Write NODE segment to configuration memory
 *
 * Address: 0x00
 * Space: 0xFD (Configuration)
 * Type: config_mem_t
 *
 * Writes in chunks (max 64 bytes per call) to respect datagram size limits.
 *
 * @param openlcb_node Pointer to OpenLCB node
 * @param config Pointer to nodeid_t struct to write
 * @return true on success, false on error
 */
bool ConfigMemHelper_write(openlcb_node_t *openlcb_node, config_mem_t *config) {
  if (!openlcb_node || !config) {
    return false;
  }

  uint32_t address = NODE_ADDR;  // Starting address from #define
  uint32_t total_size = sizeof(config_mem_t);
  uint32_t bytes_remaining = total_size;
  uint8_t *src = (uint8_t *)config;

  configuration_memory_buffer_t temp_buffer;

  _direct_access = true;

  // Write in chunks (max 64 bytes per datagram)
  while (bytes_remaining > 0) {
    uint16_t chunk_size = (bytes_remaining > LEN_DATAGRAM_MAX_PAYLOAD)
                            ? LEN_DATAGRAM_MAX_PAYLOAD
                            : bytes_remaining;

    // Copy chunk to buffer
    memcpy(temp_buffer, src, chunk_size);

    uint16_t bytes_written = ConfigMemHelper_config_mem_write(
      openlcb_node,
      address,
      chunk_size,
      &temp_buffer);

    if (bytes_written != chunk_size) {

      _direct_access = false;
      return false;  // Error or partial write

    }

    // Advance pointers for next chunk
    address += chunk_size;
    src += chunk_size;
    bytes_remaining -= chunk_size;
  }

  _direct_access = false;
  return true;
}

void ConfigMemHelper_reset_config_mem(void) {

  configuration_memory_buffer_t buffer;

  _direct_access = true;

  memset(&buffer, 0xFF, sizeof(buffer));
  uint16_t address = 0;
  for (unsigned int i = 0; i < (CONFIG_MEM_SIZE / sizeof(buffer)); i++) {

    ConfigMemHelper_config_mem_write(NULL, address, sizeof(buffer), &buffer);
    address = address + sizeof(buffer);
  }

  _direct_access = false;

}

void ConfigMemHelper_clear_config_mem(void) {

  configuration_memory_buffer_t buffer;

  _direct_access = true;

  memset(&buffer, 0x00, sizeof(buffer));
  uint16_t address = 0;
  for (unsigned int i = 0; i < (CONFIG_MEM_SIZE / sizeof(buffer)); i++) {

    ConfigMemHelper_config_mem_write(NULL, address, sizeof(buffer), &buffer);
    address = address + sizeof(buffer);
  }

  _direct_access = false;
}

bool ConfigMemHelper_is_config_mem_reset(void) {

  configuration_memory_buffer_t buffer;
  
  _direct_access = true;

  ConfigMemHelper_config_mem_read(NULL, 0, 1, &buffer);

  _direct_access = false;

  return (buffer[0] == 0xFF);
}
