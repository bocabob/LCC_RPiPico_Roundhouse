#ifndef CONFIG_MEM_HELPER_H
#define CONFIG_MEM_HELPER_H

#include <stdint.h>
#include <stdbool.h>
#include "src/openlcb/openlcb_types.h"

/**
 * Configuration Memory Helper Functions
 * Provides utilities for reading/writing configuration memory
 */

// ===== Configuration Memory Structure =====
// Auto-generated from CDI XML via C Struct Generation Panel

#include "BoardSettings.h"
#include "TTvariables.h"

#pragma pack(push, 1)  // Byte-aligned structures

// Memory space 253, origin 0x00
#define CONFIG_START_ADDR 0x00
// Memory space 253, origin 0x00
#define NODE_ADDR 0x00
// Memory space 253, origin 0x7D (125)
#define RESET_CONTROL_ADDR 0x7D
// Memory space 253, origin 0x7E (126)
#define ATTRIBUTES_ADDR 0x7E

typedef struct{
  // Memory space 253, origin NODE_ADDR
  struct {           
    char node_name[62];
    char node_description[63];
  } nodeid;
   // Memory space 253, origin RESET_CONTROL_ADDR
  struct {                             
    uint8_t flag;
  } reset_control;
  // Memory space 253, origin ATTRIBUTES_ADDR
  struct {    
// Door parameters
    uint8_t DoorCount;    // int8 Number of Doors off turntable tracks
    uint8_t DoorSpeed;              /* 0x007F: int, 1b */
    event_id_t OpenAll;      // producer eventID
    event_id_t CloseAll;      // producer eventID
    struct {
      char doorName[16];        // description of this Door
      char doorShort[5];        // short description of this Door
      event_id_t ToggleDoor;       // producer Toggle door position eventID
      uint8_t  servo_min;   /* angle + 90; 0=−90°, 90=0°, 180=+90° */
      uint8_t  servo_max;   /* angle + 90 */
    } doors[MAX_DOORS];
// Lights parameters
    event_id_t ToggleInterior;       // producer Toggle Interior Lights eventID
    event_id_t ToggleExterior;       // producer Toggle Exterior Lights eventID
    uint8_t HighLuminosity;    // int8 factor on brightness when dimmer is off
    event_id_t HighLuminosity_On;       // consumer turn Group 0 ON eventID
    uint8_t LowLuminosity;    // int8 factor on brightness when dimmer is on
    event_id_t LowLuminosity_On;       // consumer turn Group 0 ON eventID
  } attributes;
  
// modify as desired
// data not used by CDI 
  event_status_enum consumer_status[2+MAX_DOORS+4]; // Array to hold the state of each event (on/off/unknown) for the events defined in the configuration
  event_status_enum producer_status[MAX_DOORS]; // Array to hold the door open/closed state for each door (one entry per door)
  LightAddress Lights[NumOfLights];
  ServoAddress Servos[MAX_DOORS];
  uint8_t MemVersion;
} config_mem_t;

#pragma pack(pop)


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Global configuration structure instance
extern config_mem_t ConfigMemHelper_config_data;

// ===== Configuration Memory Access Functions =====

// Reads the current NVM into the passed structure; the two are in sync when it returns.
extern bool ConfigMemHelper_read(openlcb_node_t *openlcb_node, config_mem_t *config);

// Writes the current passed structure to the NVM; the two are in sync when it returns.
extern bool ConfigMemHelper_write(openlcb_node_t *openlcb_node, config_mem_t *config);

// Loads the default values for the NVM into the structure and writes them to the NVM; the two are in sync when it returns.
extern bool ConfigMemHelper_reset_and_write_default(openlcb_node_t *openlcb_node);

// Sets all Configuration Memory to the default 0xFF that a newly programmed device would have
extern void ConfigMemHelper_reset_config_mem(void);

// Sets all Configuration Memory to 0x00 which is a valid "cleared memory"
extern void ConfigMemHelper_clear_config_mem(void);

// Tests if the first byte of the Configuration memory is 0xFF, if so then the memory not initialized (cleared 0x00 is initialized as byte 0 is the User Name string which initialized is a null string)
extern bool ConfigMemHelper_is_config_mem_reset(void);

// Returns true if the NVM can be read successfully; false if I2C communication failed
extern bool ConfigMemHelper_nvm_is_accessible(void);

// Hooks for the OpenLcbLib that allows snooping on Config Mem writes before passing them on to the Pico Drivers to write to NVM
extern uint16_t ConfigMemHelper_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

// Hooks for the OpenLcbLib that allows snooping on Config Mem reads before passing them on to the Pico Drivers to read from NVM
extern uint16_t ConfigMemHelper_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

extern bool ConfigMemHelper_toggle_log_access(void);

// Called by RPiPicoDrivers_config_mem_write to keep the RAM mirror in sync with every NVM write
extern void ConfigMemHelper_mirror_write(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

void Load_application_defaults(openlcb_node_t *openlcb_node);
void Set_Application_Values_From_Config(openlcb_node_t *openlcb_node, config_mem_t *config);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // CONFIG_MEM_HELPER_H
