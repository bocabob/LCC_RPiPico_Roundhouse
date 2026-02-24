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

/*=============================================================
 * This file contains all functions pertinent to turntable
 * operation including stepper movements, relay phase switching,
 * and LED/accessory related functions.
=============================================================*/

#ifndef ROUNDHOUSE_H
#define ROUNDHOUSE_H

#include <Arduino.h>
#include "BoardSettings.h"
#include "TTvariables.h"
#include <I2C_eeprom.h>
#include <SPI.h>    // Call up the TFT driver library


// #include <Adafruit_PWMServoDriver.h>
void RoundhouseCallback(uint16_t callin);
void setupServos();
void MoveServo(int i, int dir);
void LightSwitch(int Light, int dir);
void ToggleLight(int Light);
void driveServos();
void MoveServo(int i, int dir);
void touchCommand(int boxCode);
// LN code

void setTrackDefaults();
void setServoDefaults();
#endif
