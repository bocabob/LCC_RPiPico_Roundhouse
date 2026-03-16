/*
 *  This is the configuration file for the Raspberry Pi Pico node hardware.
 */

// Configureation settings

#ifndef DEFINES_H
#define DEFINES_H

// #include <cstdint>

#define ARDUINO_COMPATIBLE

// --------------------------------------------
// Select ONE of these for Non-volitile Memory Storage
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
#define CONFIG_MEM_SIZE      32768
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
// #define KEYPAD_ADDRESS 0x20
// #define DISPLAY_ADDRESS 0x3C
#define SERVO_ADDRESS 0x40
// #define EEPROM_ADDRESS 0x50
#define STORAGE_ADDR 0x50  // 0x50 is the default address!
#define STOR_WIRE Wire1     // make Wire1 or Wire

#define SERVO_SDA  12    // for i2c0
#define SERVO_SCL  13    // for i2c0
#define SERVO_I2C i2c0 // (&i2c0_inst) ///< Identifier for I2C HW Block 
// #define SERVO_SDA  10   // for i2c1
// #define SERVO_SCL  11   // for i2c1
// #define SERVO_I2C i2c1 // (&i2c1_inst) ///< Identifier for I2C HW Block 1

#define I2C_SDA  26           // pin to use
#define I2C_SCL  27           // pin to use
// #define I2C2_SDA  10           // pin to use
// #define I2C2_SCL  11           // pin to use

#define NeoPixel_PinA 2           // pin to use for the board interface
#define NeoPixel_PinB 6           // pin to use for the board interface
#define NeoPixel_PinC 7           // pin to use for the board interface
#define NeoPixel_PinD 3           // pin to use for the board interface

#define NumOfLights 2
#define Light_A 10
#define Light_B 11

/////////////////////////////////////////////////////////////////////////////////////
//  Define the LED blink rates for fast and slow blinking in milliseconds.
// 
//  The LED will alternative on/off for these durations.
#define FREQUENCY 100

#define UNUSED_PIN 127

// Define current version of EEPROM configuration
#define EEPROM_VERSION 8

//  Enable debug outputs if required during troubleshooting.
#define NODE_DEBUG true  // uncomment for debug
#define NOTICE_PRINT Serial //serial or tft

#define TT_DEBUG true  // uncomment for debug
#define TT2_DEBUG true  // uncomment for debug

//——————————————————————————————————————————————————————————————————————————————
//  MCP2517 connections: adapt theses settings to your design
//  As hardware SPI is used, you should select pins that support SPI functions.
//  This code is designed to use SPI
//  If standard SPI, SPI2 pins are not used then define them
//    SCK input of MCP2517 is connected to pin #32
//    SDI input of MCP2517 is connected to pin #0
//    SDO output of MCP2517 is connected to pin #1
//  CS input of MCP2517 should be connected to a digital output port
//  INT output of MCP2517 should be connected to a digital input port, with interrupt capability

// static const byte MCP2517_SCK = 18 ; // SCK input of MCP2517/8
// static const byte MCP2517_SDI =  19 ; // SI input of MCP2517/8
// static const byte MCP2517_SDO =  16 ; // SO output of MCP2517/8
// static const byte MCP2517_CS  = 17 ; // CS input of MCP2517/8
// static const byte MCP2517_INT = 20 ; // INT output of MCP2517/8
//——————————————————————————————————————————————————————————————————————————————

#define MCP2517_SPI  SPI   // SPI port to use for MCP2517/8

#define MCP2517_CS  17   // CS input of MCP2517/8
#define MCP2517_INT 20  // INT output of MCP2517/8
#define MCP2517_SCK 18  // SCK input of MCP2517/8
#define MCP2517_SDI 19  // SI input of MCP2517/8
#define MCP2517_SDO 16  // SO output of MCP2517/8

/*
 *  This is the configuration file for the NeoPixel operation.
 */

// Configureation settings

// need to define number of all events in event table
// #define NUM_EVENT 5

#define NUM_SERVOS 10
#define NUM_POS 2

// Servos

#define myPWMmin 104  // PWMmin	minimal PWM signal for the servo. This is not the minimal pulse width of the servo, but rather the pulse length count. Min and max values usually within (150-600) range.
#define	myPWMmax 570  // PWMmax	maximal PWM signal for the servo. Just like the PWMmin, to be determined experimentally, by slowly raising the value and checking the motion range. 

#define i_max_servo 10   // modify as desired, you can have 16 for each PCA9685
#define MinServoRange -90
#define MaxServoRange 90
#define SERVO_FREQ 60 // Analog servos run at ~50 Hz updates

#define angleMinimum -35
#define angleMaximum 90
#define inversion 1


// #define MAX_TRACKS 20
// #define NUM_TRACKS 14
#define MAX_DOORS 16
#define NUM_DOORS 10

#define MAX_STRINGS 1
#define MAX_LIGHTS 20
// #define NumOfLights MAX_STRINGS * MAX_LIGHTS

// #define NUM_TABLE_EVENTS  5
// #define NUM_TRACK_EVENTS MAX_TRACKS * 2
#define NUM_DOOR_EVENTS 2 + MAX_DOORS // 2 All events and a toggle for each servo
// Light events
#define NUM_LUM_EVENTS  5
// #define NUM_LIGHT_EVENTS MAX_STRINGS * MAX_LIGHTS * 9
#define NUM_EVENT NUM_DOOR_EVENTS + NUM_LUM_EVENTS //+ NUM_TRACK_EVENTS + NUM_TABLE_EVENTS 

// #include <cstdint>

// #include <NeoPixelBusLg.h>

#define NeoPixel_PinA 2        // (use any (mega 22-43) - 12 / 25) for the bridge / board interface
#define NeoPixel_PinB 6        // (use any (mega 22-43) - 12 / 25) for the bridge / board interface
#define NeoPixel_PinC 7        // (use any (mega 22-43) - 12 / 25) for the bridge / board interface
#define NeoPixel_PinD 3        // (use any (mega 22-43) - 12 / 25) for the bridge / board interface

// NeoPixel defines

#define NeoPixel_PIO pio0
const uint16_t PixelCount = 2; // number of NeoPixels in the string
const uint8_t PixelPin = NeoPixel_PinA;  // pin for the data line, ignored for Esp8266

#define RedLevel 50   // bridge center
#define BlueLevel 10  // bridge shack
#define GreenLevel 0

// #define MAX_STRINGS 4 // also defined in NPlights.cpp and program .ino files and need to update event table
// #define MAX_LIGHTS 20 // also defined in NPlights.cpp and program .ino files

#define brightnesss 90
#define MAX_LUMINANCE 100
#define DIM_LUMINANCE 35
#define MIN_LUMINANCE 5
#define i_max_pixel MAX_STRINGS * MAX_LIGHTS     // number of pixel drivers in the daisy-chain

/////////////////////////////////////////////////////////////////////////////////////
//  Define the LED blink rates for fast and slow blinking in milliseconds.
// 
//  The LED will alternative on/off for these durations.
#define FREQUENCY 100

#endif