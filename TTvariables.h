// Variables and Declarations

#ifndef VARIABLES_H
#define VARIABLES_H

#include "BoardSettings.h"
#include <SPI.h>

extern bool pixelsOn;

typedef struct
{
	int Status;
	int ServoMin;
	int ServoMax;
	int Position;
}
ServoAddress;

typedef struct
{
	int pin;
	bool active;
}
LightAddress;

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