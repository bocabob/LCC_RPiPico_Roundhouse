/*
 * Parts of this © 2022 Peter Cole, 2023-5 Bob Gamble
 *
 *  This file is a part of the LocoNet Turntable project
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

Byte Mapping:
0-3 is EEPROM Flag
4  is version of EEPROM data to trigger calibration
5-8 is long number of steps
9-11 current track number and direction
13 is trackCount
14 is refCount
16-268 is the Track data array

*/

#include "EEPROMFunctions.h"
#include <I2C_eeprom.h>
#include "TTconfig.h"

extern bool StorageReady;
bool stepsSet = false;
char eepromFlag[4] = {'T', 'T', 'L', 'N'};          // EEPROM location 0 to 3 should contain TTLN if we have stored steps.
const uint8_t eepromVersion = EEPROM_VERSION;       // Version of stored EEPROM data to invalidate stored steps if config changes.
I2C_eeprom ee(STORAGE_ADDR, I2C_DEVICESIZE_24LC256, &STOR_WIRE);

void setupEEPROM(){
  // Wire1.begin();
  ee.begin();
  
  if (ee.isConnected())
  {
    Serial.println("EEPROM has been setup ...");
    StorageReady = true;
  }
  else
  {
    Serial.println("ERROR: Can't find eeprom (stopped)...");
    // while (1);
  }
}

// Function to retrieve step count from EEPROM.
// Looks for identifier "TTLN" at 0 to 3.
// Looks for version in 4.
// MSB -> LSB of steps stored in 5 - 8.
long getSteps() {
  char data[4];
  long eepromSteps;
  stepsSet = true;
  for (uint8_t i = 0; i < 4; i ++) {
    data[i] = ee.readByte(i);
    if (data[i] != eepromFlag[i]) {
      stepsSet = false;
      break;
    }
  }
  uint8_t version = ee.readByte(4);
  if (version != eepromVersion) {
    Serial.println(F("EEPROM version outdated, calibration required"));
    stepsSet = false;
  }
//   if (stepsSet) {
//     eepromSteps = ((long)ee.readByte(5) << 24) + ((long)ee.readByte(6) << 16) + ((long)ee.readByte(7) << 8) + (long)ee.readByte(8);
//     if (eepromSteps <= sanitySteps) {
// #ifdef TT_DEBUG
//       Serial.print(F("DEBUG: TTLN steps read from EEPROM: "));
//       Serial.println(eepromSteps);
// #endif
//       return eepromSteps;
//     } else {
// #ifdef TT_DEBUG
//       Serial.print(F("DEBUG: TTLN steps in EEPROM are invalid: "));
//       Serial.println(eepromSteps);
// #endif
//       calibrating = true;
//       return 0;
//     }
//   } else {
// #ifdef TT_DEBUG
//     Serial.println(F("DEBUG: TTLN steps not defined in EEPROM"));
// #endif
//     calibrating = true;
    return 0;
//   }
}

// Function to write step count with "TTLN" identifier to EEPROM.
void writeSteps(long steps) {
  // (void) EEPROM;
  for (uint8_t i = 0; i < 4; i++) {
    ee.writeByte(i, eepromFlag[i]);
  }
  ee.writeByte(4, eepromVersion);
  ee.writeByte(5, (steps >> 24) & 0xFF);
  ee.writeByte(6, (steps >> 16) & 0xFF);
  ee.writeByte(7, (steps >> 8) & 0xFF);
  ee.writeByte(8, steps & 0xFF);
  #ifdef TT_DEBUG
      Serial.print(F("DEBUG: TTLN wrote steps in EEPROM: "));
      Serial.println(steps);
  #endif
}

void writeServos()
{
  int eeSpot = 16;
  /*
	int address;
	bool active;
	int Status;
	int ServoMin;
	int ServoMax;
	int Position;
  */
  rp2040.idleOtherCore();
  
  for (int i = 0; i < (sizeof(Servos) / sizeof(ServoAddress)); i++) {
  ee.writeBlock(eeSpot,(uint8_t *) &Servos[i],sizeof(ServoAddress));
  eeSpot += sizeof(ServoAddress); //Move address to the next byte after float 'f'.
  #ifdef TT_DEBUG
      Serial.print(F("DEBUG: TTLN wrote servo address @  "));
      Serial.print(eeSpot);
      Serial.print(F("  in EEPROM: "));
      Serial.println(Servos[i].address);
  #endif
    }
  rp2040.resumeOtherCore();
}

void writeServo(int ServoNumber)
{
  int eeSpot = 16+(ServoNumber*sizeof(ServoAddress));
  /*
	int address;
	bool active;
	int Status;
	int ServoMin;
	int ServoMax;
	int Position;
  */  
  ee.writeBlock(eeSpot,(uint8_t *) &Servos[ServoNumber],sizeof(ServoAddress));
  #ifdef TT_DEBUG
      Serial.print(F("DEBUG: TTLN wrote servo address @  "));
      Serial.print(eeSpot);
      Serial.print(F("  in EEPROM: "));
      Serial.println(Servos[ServoNumber].address);
  #endif
}

void getServo(int ServoNumber)
{
  int eeSpot = 16+(ServoNumber*sizeof(ServoAddress));
  /*
	int address;
	bool active;
	int Status;
	int ServoMin;
	int ServoMax;
	int Position;
  */  
  ee.readBlock(eeSpot,(uint8_t *) &Servos[ServoNumber],sizeof(ServoAddress));
  #ifdef TT_DEBUG
      Serial.print(F("DEBUG: TTLN read servo address @  "));
      Serial.print(eeSpot);
      Serial.print(F("  in EEPROM: "));
      Serial.println(Servos[ServoNumber].address);
  #endif
}


void getServos()
{
  int eeSpot = 16;
  /*
	int address;
	bool active;
	int Status;
	int ServoMin;
	int ServoMax;
	int Position;
  */
  rp2040.idleOtherCore();
  
  for (int i = 0; i < (sizeof(Servos) / sizeof(ServoAddress)); i++) {
  ee.readBlock(eeSpot,(uint8_t *) &Servos[i], sizeof(ServoAddress));
  eeSpot += sizeof(ServoAddress); //Move address to the next byte after float 'f'.
  #ifdef TT_DEBUG
      Serial.print(F("DEBUG: TTLN read servo address @  "));
      Serial.print(eeSpot);
      Serial.print(F("  in EEPROM: "));
      Serial.println(Servos[i].address);
  #endif
    }
  rp2040.resumeOtherCore();
}

long readEEPROM()
{  
  getServos();
     
  return 0;
}

void writeEEPROM()
{ 
  writeServos();
}

// Function to clear step count and identifier from EEPROM.
void clearEEPROM() {

  rp2040.idleOtherCore();

  for (uint8_t i = 0; i < (16+sizeof(Servos)); i++) {
    ee.writeByte(i, 0);
  }
  rp2040.resumeOtherCore();

  #ifdef TT_DEBUG
      Serial.print(F("DEBUG: TTLN cleared  "));
      Serial.print(16+sizeof(Servos));
      Serial.println(F("  bytes of EEPROM: "));
  #endif
}
