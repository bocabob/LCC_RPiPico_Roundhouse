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
 * This file contains all functions pertinent to roundhouse
 * operation including servo-controlled door movements and
 * GPIO light control functions.
=============================================================*/

#ifndef ROUNDHOUSE_H
#define ROUNDHOUSE_H

#include <Arduino.h>
#include "BoardSettings.h"
#include "TTvariables.h"
#include <I2C_eeprom.h>
#include <SPI.h>    // Call up the TFT driver library


void RoundhouseCallback(uint16_t callin);
void setupServos();
void initializeHardware();
void MoveServo(int i, int dir);
void LightSwitch(int Light, int dir);
void ToggleLight(int Light);
void driveServos();
void setServoDefaults();
void SetServoStatus(int i, int status);
void updateServoRangesFromConfig();

// Called from Core 0 (Callbacks_on_100ms_timer_callback) to send a PCER for any
// door whose servo finished moving since the last call.  StopMoveHandler (Core 1)
// sets a pending flag; this function services it safely on Core 0.
void Roundhouse_send_pending_door_pcers();

#endif
