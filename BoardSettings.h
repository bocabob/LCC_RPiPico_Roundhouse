/*
 *  This is the configuration file for the Raspberry Pi Pico node hardware.
 *  Defines and constants.
 */

#ifndef DEFINES_H
#define DEFINES_H

// ProjectConfig.h is the single source of truth for board selection.
// It is included here so that ALL translation units (.cpp files) get the same
// defines through their BoardSettings.h include chain.
#include "ProjectConfig.h"

#define ARDUINO_COMPATIBLE

// --------------------------------------------
//  Reserved sentinel values for board_configs/ headers.
//  PWR_VCC/PWR_GND/PWR_AGND/PWR_VREF mark connector pins that carry power/
//  ground rather than a GPIO signal. Must be defined before the board
//  dispatch below since board_configs/BoardPins_Node_v30.h uses them.
// --------------------------------------------
#define PWR_VCC     126
#define PWR_GND     125
#define PWR_AGND    124
#define PWR_VREF    123

// --------------------------------------------
//  Board hardware selection
//  Set in ProjectConfig.h — do not define here or in individual .cpp files.
// --------------------------------------------
#if defined(LCC_BOARD_NODE_V25)
  #include "board_configs/BoardPins_Node_v25.h"
#elif defined(LCC_BOARD_NODE_V26)
  #include "board_configs/BoardPins_Node_v26.h"
#elif defined(LCC_BOARD_NODE_V27)
  #include "board_configs/BoardPins_Node_v27.h"
#elif defined(LCC_BOARD_NODE_V28)
  #include "board_configs/BoardPins_Node_v28.h"
#elif defined(LCC_BOARD_NODE_V30)
  #include "board_configs/BoardPins_Node_v30.h"
#else
  #error "No board version defined. Set LCC_BOARD_NODE_V25/V26/V27/V28/V30 in ProjectConfig.h"
#endif

// --------------------------------------------
// Select ONE of these for Non-volatile Memory Storage
// --------------------------------------------
// #define USE_INTERNAL_FLASH_STORAGE
#define USE_I2C_STORAGE

// if using external NVM storage, select ONE of these supported types, otherwise use none
#define EXTERNAL_EEPROM     // if uncommented uses external eeprom, using Adafruit library
// #define EXTERNAL_FRAM         // if uncommented uses externalFRAM, using Adafruit library

// if using external NVM, use either Tillaart library or default to Adafruit library
#define USE_TILLAART       // if uncommented then uses Rob Tillaart EEPROM and/or FRAM library

// --------------------------------------------

// -------------------------------------------
// Select ONE of these for Configuration Memory Size
// --------------------------------------------
//#define CONFIG_MEM_SIZE      65536
// 32768 minus 64 bytes reserved for the protected NVM region above config
// memory (node identity block + headroom — see LCC_NODE_STANDARD.md §7.1)
#define CONFIG_MEM_SIZE      32704
//#define CONFIG_MEM_SIZE      16384
//#define CONFIG_MEM_SIZE      8192
//#define CONFIG_MEM_SIZE      4096
//#define CONFIG_MEM_SIZE      2048
//#define CONFIG_MEM_SIZE      1024
//#define CONFIG_MEM_SIZE      512
//#define CONFIG_MEM_SIZE      256
//#define CONFIG_MEM_SIZE      128
// --------------------------------------------

// Define the size of the EEPROM chip or use 4096 if using emulated internal flash storage
// #define I2C_DEVICESIZE      65536  // 24LC512
#define I2C_DEVICESIZE      32768  // 24LC256
// #define I2C_DEVICESIZE      16384  // 24LC128
// #define I2C_DEVICESIZE       8192  // 24LC64
// #define I2C_DEVICESIZE       4096  // 24LC32    // this is the size to use for internal FLASH EEPROM emulation
// #define I2C_DEVICESIZE       2048  // 24LC16
// #define I2C_DEVICESIZE       1024  // 24LC08
// #define I2C_DEVICESIZE        512  // 24LC04
// #define I2C_DEVICESIZE        256  // 24LC02
// #define I2C_DEVICESIZE        128  // 24LC01

/////////////////////////////////////////////////////////////////////////////////////
//  Define a valid (and free) I2C address, 0x60 is the default.
//
// #define I2C_ADDRESS 0x60
#define SERVO_ADDRESS 0x40
// #define EEPROM_ADDRESS 0x50
#define STORAGE_ADDR 0x50  // 0x50 is the default address!

// STOR_WIRE, I2C_SDA, I2C_SCL, SERVO_SDA, SERVO_SCL,
// NeoPixel_Pin*, MCP2517_*, BLUE/GOLD_BUTTON_PIN
// are all defined in the board_configs/BoardPins_*.h file selected above.

// Servo I2C peripheral — always I2C0 on all Node board versions
#define SERVO_I2C i2c0

#define NumOfLights 2
#define Light_A 10
#define Light_B 11

#define UNUSED_PIN 127

// Define current version of EEPROM configuration
#define EEPROM_VERSION 8

//  Enable debug outputs if required during troubleshooting.
#define NODE_DEBUG true  // uncomment for debug
#define NOTICE_PRINT Serial //serial or tft

#define TT_DEBUG true  // uncomment for debug
#define TT2_DEBUG true  // uncomment for debug

// NeoPixel defines
// NeoPixel_PinA/B/C/D are defined in the board header above.
#define NeoPixel_PIO pio0
const uint16_t PixelCount = 2; // number of NeoPixels in the string
const uint8_t PixelPin = NeoPixel_PinA;  // pin for the data line

#define RedLevel 50   // bridge center
#define BlueLevel 10  // bridge shack
#define GreenLevel 0

#define brightnesss 90
#define MAX_LUMINANCE 100
#define DIM_LUMINANCE 35
#define MIN_LUMINANCE 5

#define MAX_STRINGS 1
#define MAX_LIGHTS 20
#define i_max_pixel MAX_STRINGS * MAX_LIGHTS     // number of pixel drivers in the daisy-chain

// Servos
#define NUM_SERVOS 10
#define NUM_POS 2

#define myPWMmin 104  // PWMmin  minimal PWM signal for the servo
#define myPWMmax 570  // PWMmax  maximal PWM signal for the servo

#define i_max_servo 10   // modify as desired, you can have 16 for each PCA9685
#define SERVO_FREQ 60 // Analog servos run at ~50 Hz updates
#define SERVO_SPEED 20
#define START_POS 90
#define SERVO_DURATION 1000000 // default time in microseconds for a servo to move from min to max position

#define MinServoRange -90
#define MaxServoRange 90
#define angleMinimum -90
#define angleMaximum 90
#define defultMinAngle -45
#define defultMaxAngle 70
#define inversion 1

#define MAX_DOORS 16
#define NUM_DOORS 10

#define NUM_DOOR_EVENTS 2 + MAX_DOORS  // OpenAll + CloseAll + one ToggleDoor per door
#define NUM_LUM_EVENTS  5              // Interior, Exterior, HighLum, LowLum + spare
#define NUM_EVENT NUM_DOOR_EVENTS + NUM_LUM_EVENTS

/////////////////////////////////////////////////////////////////////////////////////
//  Define the LED blink rates for fast and slow blinking in milliseconds.
//
//  The LED will alternative on/off for these durations.
#define FREQUENCY 100

#endif
