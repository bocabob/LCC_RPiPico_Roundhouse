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
#include "TTconfig.h"
#include "TTvariables.h"
#include <NeoPixelConnect.h>

#include <SPI.h>    // Call up the TFT driver library

bool _changed = false;
uint32_t read_started_ms = millis();
uint32_t _last_change = millis();
// uint32_t _db_time = DEBOUNCE_TOUCH;	// Debounce time (ms).

// Create an instance of NeoPixelConnect and initialize it
// to use GPIO pin PixelPin as the control pin, for a string
// of PixelCount neopixels. Name the instance strip

NeoPixelConnect strip(PixelPin, PixelCount, NeoPixel_PIO, 0);

bool pixelsOn = false;
int activeScreen = 0;
int editTrack = 0;
int editServo = 0;


void notice(const char *string)
{
  NOTICE_PRINT.println(string);
}


void touchIO(int x)
{
 touchCommand(x);
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
uint8_t luminance;
  // DimON = false;
  // luminance = _HighLuminosity;
  // strip.SetLuminance(luminance); // requires different neopixel library
  // strip.Show();
  Serial.println("All Bright ...");
}
void DimmerLow()      // dim intensity
{
uint8_t luminance; 
  // DimON = true;
  // luminance = _LowLuminosity;
  // strip.SetLuminance(luminance); // requires different neopixel library
  // strip.Show();        
  Serial.println("All Dim ...");
}