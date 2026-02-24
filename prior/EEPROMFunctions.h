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
*/

#ifndef EEPROMFUNCTIONS_H
#define EEPROMFUNCTIONS_H

#include <Arduino.h>
// #include "defines.h"
#include "TTdrive.h"
#include "TTvariables.h"

void setupEEPROM();
long getSteps();
void writeSteps(long steps);
void getTrack();
void writeTrack(uint8_t i,uint8_t Direction);
void writeCount();
void getCount();
void writeTracks();
void getTracks();
void writeReferences();
void getReferences();
void writeServos();
void getServos();
void writeServo(int ServoNumber);
void getServo(int ServoNumber);
long readEEPROM();
void writeEEPROM();
void clearEEPROM();

#endif
