// Variables and Declarations


#include "BoardSettings.h"
#include <SPI.h>    // Call up the TFT driver library



#ifndef VARIABLES_H
#define VARIABLES_H

extern bool pixelsOn;

// extern int LightPin[NumOfLights] {Light_A, Light_B};
// extern int LightAddr[NumOfLights] {700, 701};



// const char TrackName[MAX_TRACKS][25]=
// {
// 	"                     ",
//   "Entry Track 1  ",
//   "Entry Track 2  ",
//   "Entry Track 3  ",
//   "Roundhouse Bay 1 ",
//   "Roundhouse Bay 2 ",
//   "Roundhouse Bay 3 ",
//   "Roundhouse Bay 4 ",
//   "Roundhouse Bay 5 ",
//   "Roundhouse Bay 6 ",
//   "Roundhouse Bay 7 ",
//   "Roundhouse Bay 8 ",
//   "Roundhouse Bay 9 ",
//   "Roundhouse Bay 10",
//   "Machine Shop     ",
// 	"                     ",
// 	"                     ",
// 	"                     ",
// 	"                     ",
// 	"                     ",
// };

// const char TrackTag[MAX_TRACKS][5]=
// {
// 	" ",
//   "1",
//   "2",
//   "3",
//   "1",
//   "2",
//   "3",
//   "4",
//   "5",
//   "6",
//   "7",
//   "8",
//   "9",
//   "10",
//   "Shop",
// 	" ",
// 	" ",
// 	" ",
// 	" ",
// 	" ",
// };


typedef struct
{
	int Status;
	int ServoMin;
	int ServoMax;
	int Position;
}
ServoAddress;
// extern ServoAddress Servos[MAX_DOORS];

typedef struct
{
	int pin;
	bool active;
}
LightAddress;
// extern LightAddress Lights[NumOfLights];

// Door parameters
// extern uint8_t _DoorCount;    // int8 Number of Doors off turntable tracks

// typedef struct {
//   char doorName[16];        // description of this Door
//   char doorShort[5];        // short description of this Door
//   uint8_t TrackLocation;    // int8 number of the track where the door is located
// } doors;
// extern doors _Doors[MAX_DOORS];

#include <NeoPixelBus.h>

typedef struct {
  RgbColor color;
  // int effect;
  // bool on;
  uint8_t Rintensity;
  uint8_t Gintensity;
  uint8_t Bintensity;
  uint8_t RcyclesOn;     // cycles to be on
  uint8_t GcyclesOn;     // cycles to be on
  uint8_t BcyclesOn;     // cycles to be on
  uint8_t RcyclesOff;     // cycles to be off
  uint8_t GcyclesOff;     // cycles to be off
  uint8_t BcyclesOff;     // cycles to be off
  uint8_t Rstart;     // starting cycle = 0 or 1
  uint8_t Gstart;     // starting cycle = 0 or 1
  uint8_t Bstart;     // starting cycle = 0 or 1
  uint8_t Reffect;    // effect (0=off; 1=constant; 2= blink; 3=flicker)
  uint8_t Geffect;    // effect (0=off; 1=constant; 2= blink; 3=flicker)
  uint8_t Beffect;    // effect (0=off; 1=constant; 2= blink; 3=flicker)
  uint8_t Rgroup;
  uint8_t Ggroup;
  uint8_t Bgroup;
  bool Rstate;
  bool Gstate;
  bool Bstate;
  uint8_t Rcount;
  uint8_t Gcount;
  uint8_t Bcount;
  // uint32_t color;
  // public: static uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
  // uint32_t magenta = stripA.Color(255, 0, 255);
} npHead;  //

typedef struct {           // strings
  uint8_t heads;       // number of neopixel heads
  bool ON;
  bool EffectsON;
  bool DimON;
  npHead _Head[MAX_LIGHTS];
} npStrings;
extern npStrings _Strings[MAX_STRINGS];

#endif