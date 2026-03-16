/*
 *  This is the configuration file for Turntable_LocoNet.
 */

// Configureation settings

#ifndef DEFINES_H
#define DEFINES_H

//  Enable debug outputs if required during troubleshooting.
// 

#define TT_DEBUG true  // uncomment for debug
#define TT2_DEBUG true  // uncomment for debug

/////////////////////////////////////////////////////////////////////////////////////
//  Define a valid (and free) I2C address, 0x60 is the default.
// 
// #define I2C_ADDRESS 0x60 
// #define KEYPAD_ADDRESS 0x20
// #define DISPLAY_ADDRESS 0x3C
#define SERVO_ADDRESS 0x40
#define EEPROM_ADDRESS 0x50

// Define current version of EEPROM configuration
#define EEPROM_VERSION 2

#define I2C_DEVICESIZE 32768
#define STORAGE_ADDR 0x50  // the default address!
#define STOR_WIRE Wire1
#define TOUCH_WIRE Wire
#define NOTICE_PRINT Serial //serial or tft

#define I2C_SDA  26
#define I2C_SCL  27
#define SERVO_SDA  26
#define SERVO_SCL  27
// #define TOUCH_SDA  4
// #define TOUCH_SCL  5

// #define KeyPad_Int_Pin 2    // Pin for interupt from I2C module
// #define BRIDGE_SENSOR_PIN 0    // Bridge Position Sensor Input (any - 11 / 23)
// #define HOME_SENSOR_PIN 1      // Home Position Sensor Input

#define NeoPixel_PinA 21        // (use any (mega 22-43) - 12 / 25) for the bridge / board interface
// #define TX_PIN   47   // LocoNet TX Pin (I use 7 or 47), RX is hard coded to Pin 8 (Uno - Nano) or 48 (Mega) in the library
// #define LOCONET_PIN_TX  16
// #define LOCONET_PIN_RX  17

// #include "ACAN2518.h"      // uses local ACAN2518 class, comment out if using GCSerial
#define ACAN_FREQ ACAN2517Settings::OSC_40MHz  // set for crystal freq feeding the MCP2515 chip
#define ACAN_RX_PIN  16    // set for the MCP2518 chip MISO pin
#define ACAN_CS_PIN  17    // set for the MCP2518 chip select pin
#define ACAN_CSK_PIN 18    // set for the MCP2518 chip clock pin
#define ACAN_TX_PIN  19    // set for the MCP2518 chip MOSI pin
#define ACAN_INT_PIN 20    // set for the MCP2518 interrupt pin
#define NOCAN

// #define STEPPER_INTERFACE 1
// #define STEPPER_ENABLE_PIN 14   // (use any - 13 / 31) Uncomment to enable Powering-Off the Stepper if its not running 
// #define STEPPER_STEP_PIN 15      // (use any - 4 / 27)
// #define STEPPER_DIR_PIN 2       // (use any - 5 / 29)

//                  The touch screen interrupt request pin (T_IRQ) is not used

// #define TOUCH_INT  3
// #define TOUCH_RST  -1
// TFT Screen pixel resolution in landscape orientation, change these to suit your display
// Defined in landscape orientation !
// #define HRES 800
// #define VRES 480
// #define ROTATION 3  // rotation for screen
// #define TOUCH_ROTATION 0  // rotation for touch
// #define DEBOUNCE_TOUCH 40
// #define DEBOUNCE_BOX 80

/* The TFT interface is defined in the User_Setup.h file in the TFT_eSPI Arduino library folder. These are my settings:
////////////////////////////////////////////////////////////////////////////////////////////
// RP2040 pins used
////////////////////////////////////////////////////////////////////////////////////////////

//#define TFT_CS   -1  // Do not define, chip select control pin permanently connected to 0V

// These pins can be moved and are controlled directly by the library software
#define TFT_DC   28    // Data Command control pin
#define TFT_RST   2    // Reset pin

//#define TFT_RD   -1  // Do not define, read pin permanently connected to 3V3

// Note: All the following pins are PIO hardware configured and driven
  #define TFT_WR   22

  // PIO requires these to be sequentially increasing - do not change
  #define TFT_D0    6
  #define TFT_D1    7
  #define TFT_D2    8
  #define TFT_D3    9
  #define TFT_D4   10
  #define TFT_D5   11
  #define TFT_D6   12
  #define TFT_D7   13

//#define TOUCH_CS -1     // Chip select pin (T_CS) of touch screen
*/

#define NeoPixel_PIO pio0
const uint16_t PixelCount = 2; // number of NeoPixels in the string
const uint8_t PixelPin = NeoPixel_PinA;  // pin for the data line, ignored for Esp8266
#define RedLevel 50   // bridge center
#define BlueLevel 10  // bridge shack
#define GreenLevel 0

#define MAX_TRACKS 20
#define NUM_TRACKS 14 // Define number of turntable tracks for default / max, includes non-track zero position for home sensor
#define MAX_DOORS 16
#define NUM_DOORS 10

#define MAX_STRINGS 1
#define MAX_LIGHTS 20
#define NumOfLights 2
#define Light_A 1
#define Light_B 0

// #define USE_SENSORS true  // uncomment for LocoNet sensor reporting

#define UNUSED_PIN 127

// Servos

#define myPWMmin 104  // PWMmin	minimal PWM signal for the servo. This is not the minimal pulse width of the servo, but rather the pulse length count. Min and max values usually within (150-600) range.
#define	myPWMmax 570  // PWMmax	maximal PWM signal for the servo. Just like the PWMmin, to be determined experimentally, by slowly raising the value and checking the motion range. 

// #define i_max_servo 10   // modify as desired, you can have 16 for each PCA9685
// #define MinServoRange -90
// #define MaxServoRange 90
#define SERVO_FREQ 60 // Analog servos run at ~50 Hz updates

#define angleMinimum -35
#define angleMaximum 90
#define inversion 1

// #define USE_PCA9685_SERVO_EXPANDER
// #define MAX_EASING_SERVOS 16
// #define TRACE
// #define SERVO_SPEED 20
// #define START_POS 90
// #define servo ServoEasing::ServoEasingArray
// #include <ServoEasing.hpp>  // great library for gettin slow servo action, including bounces
// ServoEasing* servo[NUM_SERVOS];  // alt to #define of servo

#define SERVO_WIRE Wire1

// #define MaxDCCaddress 1024

// Stepper tracks
// #define NumberOfReferences 10   // Define max number of stepper reference points
// #define TrackStartAddress 500 // the starting DCC address for track tracks
#define ServoStartingAddress 600 // the starting DCC address for servos
// #define LightStartingAddress 700 // the starting address for pin controlled lights

#define PageBoxes 32         // number of buttons on all pages for boxes in array. 
// setting buttons = 2 * variables, add to pageboxes to get track count variable box 
#define TrackSelBoxes 38         // ending number of buttons for variable update boxes in array. 
// setting buttons = 7 * tracks, add to TrackSelBoxes to get track box starting point
#define TrackBoxes 156         // ending number of buttons for track update boxes in array. 
// setting buttons = 2 * variables, add to TrackBoxes to get servo variable update box 
#define ServoSelBoxes 158         // ending number of buttons for variable update boxes in array. 
// setting buttons = 6 * servos, add to TrackSelBoxes to get track box starting point
#define ServoBox 218         // ending point for servo boxes in array
// setting buttons = 3 * tracks, add to ServoBox to get track box starting point
#define TrackBox 278         // ending point for turntable diagram track boxes in array. 3 boxes per track, add to get possible boxes
#define PossibleBoxes 500    // space for array of click box boundaries (static boxes plus track boxes)

// #define TT_DIA (VRES/2.5)+10    // turn table diameter plus 10
// #define TT_RAD (VRES/5)     // turn table radius
// #define TT_ROTATION 90      // rotation of TT diagram
// #define TT_STATION_SIZE 18    // size of track button
// #define TT_CX HRES/2        // center X position
// #define TT_CY VRES/2        // center Y position
// #define TT_B_WIDTH 12     // button width
// #define TT_S_WIDTH 8      // track width
// #define TT_S_SPACE 65     // space between tracks

// #define T_FRINGE  5   // added pixels around a touch box

#define brightnesss 90
#define MAX_LUMINANCE 100
#define DIM_LUMINANCE 35
#define MIN_LUMINANCE 5
#define FREQUENCY 100

/////////////////////////////////////////////////////////////////////////////////////
//  Define the LED blink rates for fast and slow blinking in milliseconds.
// 
//  The LED will alternative on/off for these durations.
#define LED_FAST 100
#define LED_SLOW 500

#endif
