/*
 * Parts of this © 2022 Peter Cole, 2023-5 Bob Gamble
 *
 *  This file is a part of the LCC Turntable project
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  For a copy of the GNU General Public License see <https://www.gnu.org/licenses/>.
*/

#include "TTcomms.h"
#include "BoardSettings.h"
#include "TTvariables.h"
#include <NeoPixelConnect.h>

#include <SPI.h>    // Call up the TFT driver library

bool _changed = false;
uint32_t read_started_ms = millis();
uint32_t _last_change = millis();

// NeoPixel strip — GPIO pin PixelPin, PixelCount pixels
NeoPixelConnect strip(PixelPin, PixelCount, NeoPixel_PIO, 0);

bool pixelsOn = false;


void notice(const char *string)
{
  NOTICE_PRINT.println(string);
}



void TurnOnPixels()   // turn on the pixels
{
    for (int i = 0; i < PixelCount; i++) {     
    strip.neoPixelSetValue(i, 255,255,255,false); 
    }
    strip.neoPixelShow();
    pixelsOn = true;
    Serial.println("On ...");
}
void TurnOffPixels()      // turn off the pixels
{
    for (int i = 0; i < PixelCount; i++) {     
    strip.neoPixelSetValue(i,0,0,0,false); 
    }
    strip.neoPixelShow();
    pixelsOn = false;
}

void TogglePixels()   // toggle the pixels
{
  uint8_t SetRed = RedLevel;
  uint8_t SetBlue = BlueLevel;
  uint8_t SetGreen = GreenLevel;
  if (pixelsOn) { 
    SetRed = 0;
    SetBlue = 0;
    SetGreen = 0;
    pixelsOn = false;
    Serial.println("Off ...");
    }
    else {
      pixelsOn = true;
    Serial.println("On ...");
    }
  for (int i = 0; i < PixelCount; i++) { 
     strip.neoPixelSetValue(i, SetRed,SetGreen,SetBlue,false);
     }    
  strip.neoPixelShow();
}

void DimmerHigh()      // set to higher intensity
{
  // TODO: implement luminance control once NeoPixelBus SetLuminance API is available
  Serial.println("All Bright ...");
}
void DimmerLow()      // set to lower intensity
{
  // TODO: implement luminance control once NeoPixelBus SetLuminance API is available
  Serial.println("All Dim ...");
}