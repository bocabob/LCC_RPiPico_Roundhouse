/* 
 Rewritten by Bob Gamble for LCC 2025-1-10
  The OpenLCB single thread library is used for LCC

 This current program uses a Raspberry Pi Pico:
1) the OpenLCB single thread, Wire, and I2C_eeprom libraries;
2) 
3) 
6) NeoPixel for lights
7) 
8) I2C EEPROM for saving states

Pin usage specified in the configuration file:
  The LCC Shield uses pins for the HW CAN controller MCP2518
  The I2C uses pins
  The NeoPixel uses a pin 

Other pins for relays and LEDs may be used.
//==============================================================
// Pico LCC Node with 2518 HW controller
// Modified DPH 2024 RSG 2025
// Copyright 2019 Alex Shepherd and David Harris and Bob Gamble, 2025
//==============================================================

/* ==============================================================
This sketch implements various functions on a RPi Pico or Pico 2
It can use Wifi to connect to a OpenLCB hub, eg JMRI
Set CPU speed to 125 MHz
Note: use "USB stack = Pico SDK"
============================================================== */

/*  Set to 1 to Force Reset or initialize EEPROM to Program Defaults
    as defined in userInitAll() */
#define RESET_TO_PROGRAM_DEFAULTS 0
/* Need to do this at least once to initialize NVM space to MemStruct */


// Include required libraries.
#include <Arduino.h>

#include "TTconfig.h"
#include "TTvariables.h"
#include <Wire.h>    //I2C library
#include <SPI.h>    // Call up the TFT driver library
// #include<I2C_eeprom.h>

// To use ACAN2518: uncomment #include ACAN2518.h and #define NOCAN
#define NOCAN
#define ACAN_FREQ ACAN2517Settings::OSC_40MHz  // set for crystal freq feeding the MCP2515 chip
#define ACAN_RX_PIN  16    // set for the MCP2518 chip MISO pin
#define ACAN_CS_PIN  17    // set for the MCP2518 chip select pin
#define ACAN_CSK_PIN 18    // set for the MCP2518 chip clock pin
#define ACAN_TX_PIN  19    // set for the MCP2518 chip MOSI pin
#define ACAN_INT_PIN 20    // set for the MCP2518 interrupt pin

#include "ACAN2518.h"      // uses local ACAN2518 class, comment out if using GCSerial

//// Debugging -- uncomment to activate debugging statements:
    // dP(x) prints x, 
    // dPH(x) prints x in hex, 
    // dPS(string,x) prints string and x
#define DEBUG Serial

#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Board definitions
#define MANU "Bob Gamble"  // The manufacturer of node
#define MODEL "PicoNode" // The model of the board
#define HWVERSION "2.4B"   // Hardware version
#define SWVERSION "0.8"   // Software version

// To Reset the Node Number, Uncomment and edit the next line
// Need to do this at least once.  
#define NODE_ADDRESS  5,1,1,1,94,0x09   // 05 01 01 01 94 ** range assigned to Bob Gamble / Southern Piedmont


// User defs

#define EXTERNALEEPROM     // if uncommented uses external eeprom, using Adafruit library
// #define EXTERNALFRAM         // if uncommented uses externalFRAM, using Adafruit library
#define USE_TILLAART       // if uncommented then uses Rob Tillaart EEPROM and/or FRAM library
// #define USE_SPARKFUN       // if uncommented then uses SparkFun EEPROM library
// #define I2C_DEVICESIZE_24LC512      65536
// #define I2C_DEVICESIZE_24LC256      32768
// #define I2C_DEVICESIZE_24LC128      16384
// #define I2C_DEVICESIZE_24LC64        8192
// #define I2C_DEVICESIZE_24LC32        4096
// #define I2C_DEVICESIZE_24LC16        2048
// #define I2C_DEVICESIZE_24LC08        1024
// #define I2C_DEVICESIZE_24LC04         512
// #define I2C_DEVICESIZE_24LC02         256
// #define I2C_DEVICESIZE_24LC01         128
#define I2C_DEVICESIZE 32768
#define STORAGE_ADDR 0x50  // the default address!
#define STOR_WIRE Wire1
// #include "Storage.h"

// need to define number of all events in event table
// #define NUM_EVENT 5

#define NUM_SERVOS 10
#define NUM_POS 2

#define MAX_TRACKS 20
#define NUM_TRACKS 14
#define MAX_DOORS 16
#define NUM_DOORS 10

#define MAX_STRINGS 1
#define MAX_LIGHTS 20

#define NUM_TABLE_EVENTS  5
#define NUM_TRACK_EVENTS MAX_TRACKS * 2
#define NUM_DOOR_EVENTS 2 + MAX_DOORS // 2 All events and a toggle for each servo
// Light events
#define NUM_LUM_EVENTS  5
// #define NUM_LIGHT_EVENTS MAX_STRINGS * MAX_LIGHTS * 9
#define NUM_EVENT NUM_DOOR_EVENTS + NUM_LUM_EVENTS + NUM_TRACK_EVENTS + NUM_TABLE_EVENTS 


// for LCC single thread
#include "mdebugging.h"           // debugging
#include "processor.h"

#undef REBOOT
#define REBOOT watchdog_reboot(0,0,0); 

#include "processCAN.h"
#include "OpenLCBHeader.h"

// Include local files
#include "TTcomms.h"
#include "TTdrive.h"

////// DECLARATIONS

extern bool lastRunningState;   // Stores last running state to allow turning the stepper off after moves.
bool setupComplete = false;
bool setup1Complete = false;
bool StorageReady = false;

npStrings _Strings[MAX_STRINGS];

#include "CDI.h"

/*   Memory structure of EEPROM, must match CDI above

0    EventID Rehome;
1    EventID IncrementTrack;
2    EventID DecrementTrack;
3    EventID RotateTrack180 ;
4    EventID ToggleBridgeLights;
    
    struct {
      EventID Front;       // consumer eventID
      EventID Back;       // consumer eventID
    } tracks[MAX_TRACKS];
2 * MAX_TRACKS
// Door parameters - produced
    EventID OpenAll;
    EventID CloseAll;
    struct {
      EventID eidToggle;       // consumer Toggle door position eventID
    } doors[MAX_DOORS];

// Lights parameters
    EventID eidBridge;       // consumer Toggle Bridge Lights eventID
    EventID eidInterior;       // producer Toggle Interior Lights eventID
    EventID eidExterior;       // producer Toggle Exterior Lights eventID
    EventID eidHighLuminosity_On;       // consumer set to bright
    EventID eidLowLuminosity_On;       // consumer set to dim
*/    

    //  OpenLcb.produce(eventIndex);
    // Door events -  
#define IndexOpenAll NUM_TABLE_EVENTS + NUM_TRACK_EVENTS // PEID(OpenAll) OpenLcb.produce(IndexOpenAll);
#define IndexCloseAll IndexOpenAll + 1 // PEID(CloseAll) OpenLcb.produce(IndexCloseAll);
#define IndexDoor1 IndexCloseAll + 1 // PEID(doors[s].eidToggle) OpenLcb.produce(IndexDoor1 + i);
#define IndexLightIn  NUM_TABLE_EVENTS + NUM_TRACK_EVENTS + NUM_DOOR_EVENTS + 1   // Light events  PEID(eidInterior) OpenLcb.produce(IndexLightIn);
#define IndexLightEx IndexLightIn + 1 // PEID(eidExterior) OpenLcb.produce(IndexLightEx);

extern void writeNVMdefaults();
extern void readNVM();

void produceLightIn()
{
OpenLcb.produce(IndexLightIn);
}
void produceLightEx()
{
OpenLcb.produce(IndexLightEx);
}
void produceOpenAll()
{
OpenLcb.produce(IndexOpenAll);
}
void produceCloseAll()
{
OpenLcb.produce(IndexCloseAll);
}
void produceDoor(int servo)
{
OpenLcb.produce(IndexDoor1 + servo);
}

extern "C" {
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.
  //    -- PEID() is a macro that creates an entry in the table: offset and flags.
  //    -- PEID = Producer-EID, CEID = Consumer, and PCEID = Producer/Consumer
  //    -- note matching references to MemStruct.
  // useful macro to help fill the table  
  #define REG_TRACK_ALL CEID(Rehome), CEID(IncrementTrack), CEID(DecrementTrack), CEID(RotateTrack180), CEID(ToggleBridgeLights) 
  #define REG_TRACK_MOVE(s) CEID(tracks[s].Front), CEID(tracks[s].Back)
  #define REG_DOOR_ALL PEID(OpenAll), PEID(CloseAll)
  #define REG_DOOR_OUTPUT(s) PEID(doors[s].eidToggle)
  #define REG_LIGHTS CEID(eidBridge), PEID(eidInterior), PEID(eidExterior), CEID(eidHighLuminosity_On), CEID(eidLowLuminosity_On)
  // #define REG_DOOR_OUTPUT(s) CEID(strings[s].eidAllOn), CEID(strings[s].eidAllOff), CEID(strings[s].eidAllToggle), CEID(strings[s].eidEffectsOn), CEID(strings[s].eidEffectsOff), CEID(strings[s].eidEffectsToggle), CEID(strings[s].eidDimmerToggle)
  // #define REG_LIGHT_OUTPUT(s,p) CEID(strings[s].pixel[p].eidRedOn), CEID(strings[s].pixel[p].eidRedOff), CEID(strings[s].pixel[p].eidRed), CEID(strings[s].pixel[p].eidGrnOn), CEID(strings[s].pixel[p].eidGrnOff), CEID(strings[s].pixel[p].eidGrn), CEID(strings[s].pixel[p].eidBluOn), CEID(strings[s].pixel[p].eidBluOff), CEID(strings[s].pixel[p].eidBlu)
    /*
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        PEID(inputs[0].activation), PEID(inputs[0].inactivation),  // 1st channel - input, ie producer
        PEID(inputs[1].activation), PEID(inputs[1].inactivation),  // 2nd channel - input
        PEID(inputs[2].activation), PEID(inputs[2].inactivation),  // 3rd channel - input
        PEID(inputs[3].activation), PEID(inputs[3].inactivation),  // 4th channel - input
        PEID(inputs[4].activation), PEID(inputs[4].inactivation),  // 5th channel - input
        PEID(inputs[5].activation), PEID(inputs[5].inactivation),  // 6th channel - input
        PEID(inputs[6].activation), PEID(inputs[6].inactivation),  // 7th channel - input
        PEID(inputs[7].activation), PEID(inputs[7].inactivation),  // 8th channel - input
        CEID(outputs[0].setEvent),  CEID(outputs[0].resetEvent),   // 9th channel - output, ie consumer
        CEID(outputs[1].setEvent),  CEID(outputs[1].resetEvent),   // 10th channel - output
        CEID(outputs[2].setEvent),  CEID(outputs[2].resetEvent),   // 11th channel - output
        CEID(outputs[3].setEvent),  CEID(outputs[3].resetEvent),   // 12th channel - output
        CEID(outputs[4].setEvent),  CEID(outputs[4].resetEvent),   // 13th channel - output
        CEID(outputs[5].setEvent),  CEID(outputs[5].resetEvent),   // 14th channel - output
        CEID(outputs[6].setEvent),  CEID(outputs[6].resetEvent),   // 15th channel - output
        CEID(outputs[7].setEvent),  CEID(outputs[7].resetEvent),   // 16th channel - output
    };*/
    // ===== eventid Table =====
    //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        // REG_LEAD_MOVE(0), REG_LEAD_MOVE(1), REG_LEAD_MOVE(2), REG_LEAD_MOVE(3), REG_LEAD_MOVE(4),
        // REG_LEAD_MOVE(5), REG_LEAD_MOVE(6), REG_LEAD_MOVE(7), REG_LEAD_MOVE(8), REG_LEAD_MOVE(9),
        // REG_LEAD_MOVE(10), REG_LEAD_MOVE(11), REG_LEAD_MOVE(12), REG_LEAD_MOVE(13), REG_LEAD_MOVE(14),
        // REG_LEAD_MOVE(15), REG_LEAD_MOVE(16), REG_LEAD_MOVE(17), REG_LEAD_MOVE(18), REG_LEAD_MOVE(19),
        REG_TRACK_ALL,
        REG_TRACK_MOVE(0), REG_TRACK_MOVE(1), REG_TRACK_MOVE(2), REG_TRACK_MOVE(3), REG_TRACK_MOVE(4), // 2 events per track
        REG_TRACK_MOVE(5), REG_TRACK_MOVE(6), REG_TRACK_MOVE(7), REG_TRACK_MOVE(8), REG_TRACK_MOVE(9),
        REG_TRACK_MOVE(10), REG_TRACK_MOVE(11), REG_TRACK_MOVE(12), REG_TRACK_MOVE(13), REG_TRACK_MOVE(14),
        REG_TRACK_MOVE(15), REG_TRACK_MOVE(16), REG_TRACK_MOVE(17), REG_TRACK_MOVE(18), REG_TRACK_MOVE(19),
        //  REG_TRACK_MOVE(10), REG_TRACK_MOVE(11), 
        // REG_TRACK_MOVE(12), REG_TRACK_MOVE(13), REG_TRACK_MOVE(14), REG_TRACK_MOVE(15),
        REG_DOOR_ALL,
        // REG_GROUPS, // 14 events
        // REG_DOOR_OUTPUT(0)  // for MAX_DOORS = 1 (1 events per door)
        // REG_DOOR_OUTPUT(0),REG_DOOR_OUTPUT(1)  // for MAX_DOORS = 2
        // REG_DOOR_OUTPUT(0),REG_DOOR_OUTPUT(1),REG_DOOR_OUTPUT(2)  // for MAX_DOORS = 3
        // REG_DOOR_OUTPUT(0),REG_DOOR_OUTPUT(1),REG_DOOR_OUTPUT(2),REG_DOOR_OUTPUT(3)  // for MAX_DOORS = 4
        REG_DOOR_OUTPUT(0),REG_DOOR_OUTPUT(1),REG_DOOR_OUTPUT(2),REG_DOOR_OUTPUT(3), REG_DOOR_OUTPUT(4), REG_DOOR_OUTPUT(5),REG_DOOR_OUTPUT(6),REG_DOOR_OUTPUT(7),  // for MAX_DOORS = 8
        REG_DOOR_OUTPUT(8),REG_DOOR_OUTPUT(9),REG_DOOR_OUTPUT(10),REG_DOOR_OUTPUT(11), REG_DOOR_OUTPUT(12), REG_DOOR_OUTPUT(13),REG_DOOR_OUTPUT(14),REG_DOOR_OUTPUT(15),  // for MAX_DOORS = 16        // REG_LIGHT_OUTPUT(0,0),REG_LIGHT_OUTPUT(0,1),REG_LIGHT_OUTPUT(0,2),REG_LIGHT_OUTPUT(0,3),REG_LIGHT_OUTPUT(0,4),
        // REG_LIGHT_OUTPUT(0,5),REG_LIGHT_OUTPUT(0,6),REG_LIGHT_OUTPUT(0,7),REG_LIGHT_OUTPUT(0,8),REG_LIGHT_OUTPUT(0,9),
        // REG_LIGHT_OUTPUT(0,10),REG_LIGHT_OUTPUT(0,11),REG_LIGHT_OUTPUT(0,12),REG_LIGHT_OUTPUT(0,13),REG_LIGHT_OUTPUT(0,14),
        // REG_LIGHT_OUTPUT(0,15),REG_LIGHT_OUTPUT(0,16),REG_LIGHT_OUTPUT(0,17),REG_LIGHT_OUTPUT(0,18),REG_LIGHT_OUTPUT(0,19),
        // REG_LIGHT_OUTPUT(0,20),REG_LIGHT_OUTPUT(0,21),REG_LIGHT_OUTPUT(0,22),REG_LIGHT_OUTPUT(0,23),REG_LIGHT_OUTPUT(0,24),
        // REG_LIGHT_OUTPUT(1,0),REG_LIGHT_OUTPUT(1,1),REG_LIGHT_OUTPUT(1,2),REG_LIGHT_OUTPUT(1,3),REG_LIGHT_OUTPUT(1,4),
        // REG_LIGHT_OUTPUT(1,5),REG_LIGHT_OUTPUT(1,6),REG_LIGHT_OUTPUT(1,7),REG_LIGHT_OUTPUT(1,8),REG_LIGHT_OUTPUT(1,9),
        // REG_LIGHT_OUTPUT(1,10),REG_LIGHT_OUTPUT(1,11),REG_LIGHT_OUTPUT(1,12),REG_LIGHT_OUTPUT(1,13),REG_LIGHT_OUTPUT(1,14),
        // REG_LIGHT_OUTPUT(1,15),REG_LIGHT_OUTPUT(1,16),REG_LIGHT_OUTPUT(1,17),REG_LIGHT_OUTPUT(1,18),REG_LIGHT_OUTPUT(1,19)
        // REG_LIGHT_OUTPUT(1,20),REG_LIGHT_OUTPUT(1,21),REG_LIGHT_OUTPUT(1,22),REG_LIGHT_OUTPUT(1,23),REG_LIGHT_OUTPUT(1,24)
        REG_LIGHTS
    };    
    // SNIP Short node description for use by the Simple Node Information Protocol
    // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001" MANU "\000" MODEL "\000" HWVERSION "\000" SWVERSION ; // last zero in double-quote
} // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {   //0xD7,0x58,0x00,0,0,0};
        pSimple | pDatagram | pMemConfig | pPCEvents | !pIdent    | pTeach     | !pStream   | !pReservation, // 1st byte
        pACDI   | pSNIP     | pCDI       | !pRemote  | !pDisplay  | !pTraction | !pFunction | !pDCC        , // 2nd byte
        0, 0, 0, 0                                                                                           // remaining 4 bytes
    };

// #include "TTdrive.h"

#define OLCB_NO_BLUE_GOLD
#ifndef OLCB_NO_BLUE_GOLD
    #define BLUE 40  // built-in blue LED
    #define GOLD 39  // built-in green LED
    ButtonLed blue(BLUE, LOW);
    ButtonLed gold(GOLD, LOW);
    
    uint32_t patterns[8] = { 0x00010001L, 0xFFFEFFFEL }; // two per channel, one per event
    ButtonLed pA(13, LOW);
    ButtonLed pB(14, LOW);
    ButtonLed pC(15, LOW);
    ButtonLed pD(16, LOW);
    ButtonLed* buttons[8] = { &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD };
#endif // OLCB_NO_BLUE_GOLD


// ===== Process Consumer-eventIDs =====
void pceCallback(uint16_t callin) {
  dP("\nEventid callback: index="); dP((uint16_t)callin); 
  uint16_t index;
  uint8_t current;
  uint8_t target;
  index = callin;
// Invoked when an event is consumed; 
// drive actions as needed from index of all events.
//
//dPS((const char*)"\npceCallback: Event Index: ", index);
// dP("\neventid callback: index="); dP((uint16_t)index);
  if (index < NUM_TABLE_EVENTS){    
        switch (index) { //
          case 0:  // CEID(Rehome)
          touchCommand(4);
          break;
          case 1:  //  CEID(IncrementTrack) 
          touchCommand(9);
          break;
          case 2:  //  CEID(DecrementTrack) 
          touchCommand(5);
          break;
          case 3:  //  CEID(RotateTrack180) 
          touchCommand(1);
          break;
          case 4:  //  CEID(ToggleBridgeLights) 
          touchCommand(2);
          break;
          default:
            // do nothing
          break;
        }
  }
  else 
  if (index < NUM_TABLE_EVENTS + NUM_TRACK_EVENTS){
    index = index - NUM_TABLE_EVENTS;
    uint8_t track = index / 2;
    uint8_t outputState = index % 2;
    if (outputState) {
      // move back side to track
      // move to track backward
    }
    else {
      // move front side to track
      // move to track foreward
    }
      
      // toggle door to track w/redraw
      if (Tracks[track].doorPresent) 
      {
        // if (Servos[Tracks[track].servoNumber].Status)
        // {              MoveServo(Tracks[track].servoNumber, 0);            }
        // else
        // {              MoveServo(Tracks[track].servoNumber, 32);            }
      }
  }
  else {    
// skip Door events as they are produced, not consumed
    index = index - NUM_DOOR_EVENTS;
    if (index < NUM_LUM_EVENTS){
        switch (index) { //, , , , 
          case 0:  //   CEID(eidBridge)
          touchCommand(2);
          break;
          case 1:  //  PEID(eidInterior)
          // produced
          break;
          case 2:  //  PEID(eidExterior)
          // produced
          break;
          case 3:  //  CEID(eidHighLuminosity_On)
          DimmerHigh();      // go to high luminosity
          break;
          case 4:  //  CEID(eidLowLuminosity_On)
          DimmerLow();       // go to low luminosity 
          break;
          default:
            // do nothing
          break;
        }
      }
  }
}

void produceFromInputs() {
    // called from loop(), this looks at changes in input pins and
    // and decides which events to fire
    // with pce.produce(i);
    //  OpenLcb.produce(eventIndex);
    // Door events -  PEID(OpenAll), PEID(CloseAll), PEID(doors[s].eidToggle)
    // Light events  PEID(eidInterior), PEID(eidExterior)
}

void userSoftReset() {}
void userHardReset() {}

#include "OpenLCBMid.h"   // Essential - do not move or delete

/* Callback from a Configuration write
   Use this to detect changes in the ndde's configuration
   This may be useful to take immediate action on a change. */
void userConfigWritten(uint32_t address, uint16_t length, uint16_t func)
{
  dPS("\nuserConfigWritten: Addr: ", (uint32_t)address); 
  dPS("  Len: ", (uint16_t)length); 
  dPS("  Func: ", (uint8_t)func);

  // The simplest way to make sure any changes are reflect in the servos
  // is to just update all of them.
    // setSpeedForAllServos(NODECONFIG.read( ( EEADDR(ServoSpeed) )));
  // for(uint8_t i = 0; i < NUM_SERVOS; i++) {
  //   servo[i]->setSpeed(NODECONFIG.read( ( EEADDR(ServoSpeed) )));
  //   // curpos[i] = NODECONFIG.read( ( EEADDR(curpos[i]) ) );
  //   // servoSet(i, curpos[i]);
  //   servoSet(i, NODECONFIG.read( EEADDR(curpos[i]) ) );     
  // }
    // MoveAllDoors(0);      // open or close all door servos
  // for(int i=0; i<NUM_SERVOS; i++) {
    // servoSet(i, NODECONFIG.read( EEADDR(curpos[i]) ) );
  // }

  readNVM();
  // MakeDirty();
  // for(uint8_t i = 0; i < MAX_STRINGS; i++) {
  //  for(uint8_t j = 0; j < MAX_LIGHTS; j++) {   
  //     _Strings[i]._Head[j].Rstate = _Strings[i]._Head[j].Rstart;
  //     _Strings[i]._Head[j].Gstate = _Strings[i]._Head[j].Gstart;
  //     _Strings[i]._Head[j].Bstate = _Strings[i]._Head[j].Bstart;    
  //     _Strings[i]._Head[j].Rcount = 0;
  //     _Strings[i]._Head[j].Gcount = 0;
  //     _Strings[i]._Head[j].Bcount = 0;
  //   } 
  // }
}

/*
void dumpEEPROM(uint16_t memoryAddress, uint16_t length)
{
  const int BLOCK_TO_LENGTH = 10;

  Serial.print("\t  ");
  for (int x = 0; x < 10; x++)
  {
    if (x != 0) Serial.print("    ");
    Serial.print(x);
  }
  Serial.println();

  // block to defined length
  memoryAddress = memoryAddress / BLOCK_TO_LENGTH * BLOCK_TO_LENGTH;
  length = (length + BLOCK_TO_LENGTH - 1) / BLOCK_TO_LENGTH * BLOCK_TO_LENGTH;

  byte b = storage.read(memoryAddress);
  for (unsigned int i = 0; i < length; i++)
  {
    char buf[6];
    if (memoryAddress % BLOCK_TO_LENGTH == 0)
    {
      if (i != 0) Serial.println();
      sprintf(buf, "%05d", memoryAddress);
      Serial.print(buf);
      Serial.print(":\t");
    }
    sprintf(buf, "%03d", b);
    Serial.print(buf);
    b = storage.read(++memoryAddress);
    Serial.print("  ");
  }
  Serial.println();
}
*/

// ==== Setup does initial configuration ======================

void setup()
{   
  #ifdef DEBUG
    uint32_t stimer = millis();
    Serial.begin(115200);
    while (!Serial && (millis() - stimer < 2000));   // wait for 2 secs for USB/serial connection to be established
    dP("\n Pico Node");
    // delay(1000);
  #endif 
  // Run startup configuration
  // initializeHardware(); 

// // old code *****************************************

  // set defaults

  // setTrackDefaults();
  // setServoDefaults();  
  

  while(!StorageReady);

  readNVM();

  // check EEPROM for saved values and initialize

  // If step count explicitly defined, use that

// #ifndef SENSOR_TESTING  // If we're not sensor testing, start Wire()
  // setupWire();
// #endif
    
  setupServos();

	notice("Turntable Program Started");
	
  setupComplete = true;
  Serial.println(F("Setup zero complete"));
  while(!setup1Complete);
  delay(10);

}

void setup1() {
  // delay(2000);
  // while(!Serial || millis() < 3000UL);

  #ifdef DEBUG
    uint32_t stimer = millis();
    while (!Serial && (millis() - stimer < 2000));   // wait for 2 secs for USB/serial connection to be established
    // dP("\n Pico Node");
    // delay(1000);
  #endif 

  dP(F("\n File: ")); dP(__FILENAME__);

  #if true
    dP(F("\n BType: ")); dP(BTYPE);
    dP(F("\n Manufacturer: ")); dP(MANU);
    dP(F("\n Model: ")); dP(MODEL);
    dP(F("\n HW: ")); dP(HWVERSION);
    dP(F("\n SW: ")); dP(SWVERSION);
    dP(F("\n "));
  #endif 

  STOR_WIRE.setSDA(I2C_SDA);
  STOR_WIRE.setSCL(I2C_SCL);
  STOR_WIRE.begin();
  // // Wire1 for servo and EEPROM
  // Wire1.setSDA(SERVO_SDA);
  // Wire1.setSCL(SERVO_SCL);
  // Wire1.begin();

  setupEEPROM();

  delay(5);

  // storage.begin(34);
  // #if defined(EXTERNALEEPROM) || defined(EXTERNALFRAM)
  //   #if defined(USE_SPARKFUN)
  //     storage.begin(I2C_DEVICESIZE, &STOR_WIRE, STORAGE_ADDR);
  //   #else
  //     storage.begin(I2C_DEVICESIZE, &STOR_WIRE, STORAGE_ADDR);
  //   #endif
  // #endif

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_PROGRAM_DEFAULTS);


  dP("\n setup finished");
  dP("\n NUM_EVENT="); dP(NUM_EVENT); dP("\n");
  notice("LCC setup ");

  // for (int i = 0; i < (sizeof(Servos) / sizeof(ServoAddress)); i++)
  //   {   
  //     // LN_STATUS lnStatus = LocoNet.reportSensor(Servos[i].address,Servos[i].Status);
  //     reportSensor(&LNbus,Servos[i].address,Servos[i].Status);
  //   }  
  
  bool setup1Complete = true;
  Serial.println(F("Setup one complete"));
  while(!setupComplete);

}

// ==== Loop One for LCC processes ==========================
void loop() {  
  //MDNS.update();  // IS THIS NEEDED?
  static long nextdot = 0;
  if(millis()>nextdot) {
    nextdot = millis()+2000;
    //dP("\n.");
  }
    
    // processLED();   // Process our LED.


  // touchIO();    // process touch input
  
    // processSerialInput();   // Receive and process and serial input for test commands.

  // #endif 

}


// ==== Loop Two for node function processes ==========================
void loop1()
{
  //MDNS.update();  // IS THIS NEEDED?
  bool activity = Olcb_process();

  static long nextdot = 0;
  if(millis()>nextdot) {
    nextdot = millis()+2000;
    //dP("\n.");
  }
  
  #ifndef OLCB_NO_BLUE_GOLD
    if (activity) {
      blue.blink(0x1); // blink blue to show that the frame was received
    }
    if (olcbcanTx.active) {
      gold.blink(0x1); // blink gold when a frame sent
      olcbcanTx.active = false;
    }
    // handle the status lights
    gold.process();
    blue.process();
  #endif // OLCB_NO_BLUE_GOLD
  
  //produceFromInputs();  // process inputs not needed
  // ProcessPixels();
  
  // driveServos();

  // if (!stepper.isRunning()) {
    
  // }
}


//   parser.onSwitchRequest([](uint16_t address, bool output, bool direction) {
//     uint8_t OutputPower = output;
//     uint8_t Direction = direction*32;
//     processSwitchRequest(address, OutputPower, Direction);
//     #ifdef TT_DEBUG
//       Serial.print("Switch Request: ");
//       Serial.print(address, DEC);
//       Serial.print(':');
//       Serial.print(direction ? "Closed" : "Thrown");
//       Serial.print(" - ");
//       Serial.println(output ? "On" : "Off");
//     #endif
//   });

// void processSwitchRequest( uint16_t Addr, uint8_t OutputPower, uint8_t Direction ) {

//   int outputInPair = (Direction == 32);
//   int InDir = (Direction > 0);

// #ifdef TT_DEBUG
// // the following prints to the serial monitor for debugging purposes
//   Serial.print(F("processSwitchRequestOutput: "));
//   Serial.print(Addr, DEC);
//   Serial.print(',');
//   Serial.print(Direction, DEC);
//   Serial.print(',');
//   Serial.print(OutputPower, DEC);
//   Serial.print(',');
//   Serial.print(F(" outputInPair = "));
//   Serial.println(outputInPair, DEC);
// #endif  

//   for (int i = 0; i < (sizeof(Tracks) / sizeof(TrackAddress)); i++)
//   {
//     if ((Addr == Tracks[i].address) && ((Addr != lastAddr) || (Direction != lastDirection)) && OutputPower)
//     {     
// #ifdef USE_SENSORS
//       // LN_STATUS lnStatus = LocoNet.reportSensor(lastAddr,0);
//     reportSensor(&LNbus, lastAddr, 0);
     
//       Serial.print(F("Tx:  Sensor Off: "));
//       Serial.println(Tracks[i].address, DEC);
//       // Serial.print(F(" Status: "));
//       // Serial.println(LocoNet.getStatusStr(lnStatus));
// #endif      
//       MoveToTrack(i,Direction);
//     }
//   }
  
//   if ((Addr == LightStartingAddress)) // hard coded address for controlling LED lights from pin Light_A
//   {
//     LightSwitch(0,InDir);   
//   }
//   if ((Addr == LightStartingAddress+1)) // hard coded address for controlling LED lights from pin Light_B
//   {
//     LightSwitch(1,InDir); 
//   }
//   for (int i = 0; i < (sizeof(Servos) / sizeof(ServoAddress)); i++)
//   {   
//     if ((Addr == Servos[i].address)  && (InDir != Servos[i].Status) && (OutputPower != 0))
//     {
//         MoveServo(i,InDir);
//         break;
//     }
//   }
// }

