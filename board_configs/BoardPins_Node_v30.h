/*
 * Pin definitions for LCC RPi Pico Node board v3.0
 *
 * Do not include this file directly — it is selected automatically
 * by BoardSettings.h based on the LCC_BOARD_NODE_V30 define in ProjectConfig.h.
 *
 * Unlike LCC_RPiPico_Turntable/PixelLights/Clock_Lights, this project has no
 * separate NodeConfig.h — functional assignments (NeoPixel, servo I2C,
 * buttons) live directly in this file, same pattern as the v2.5-v2.8 board
 * headers below. See BoardSettings.h's comment block for the full list of
 * macros this file is expected to define.
 *
 * Connector layout — 10-pin IDC (IO1, IO2):
 *   Pins 1-4  signal | Pin 5 GND | Pin 6 Vselect (jumper-selectable 3.3V/5V) | Pins 7-10 signal
 *
 * IO3 is a 5-pin analog header:
 *   Pin 1-2 signal | Pin 3 AGND | Pin 4 VREF | Pin 5 signal
 *
 * v3.0 layout — Roundhouse breakout on I/O-1 (jumper-selectable connector;
 * I/O-2 would work identically, I/O-1 was chosen to leave I/O-2 free):
 *   NeoPixel outputs : gp8-11 (I/O-1 pins 1-4) — RESERVED for a future NeoPixel
 *                      lighting upgrade; not yet used by any firmware in this
 *                      project (see NPlights-less Roundhouse.cpp) but the pins
 *                      are physically dedicated on the breakout so nothing
 *                      else should claim them.
 *   I2C0 for servo   : gp12 (SDA) / gp13 (SCL) — I/O-1 pins 7/8
 *   LEDs             : gp14 (Light_A) / gp15 (Light_B) — I/O-1 pins 9/10
 *   I2C1 for storage : gp6 (SDA) / gp7 (SCL) — Wire1 (fixed-function, not on any connector)
 *   CAN (SPI0)       : SDO=gp0, CS=gp1, SCK=gp2, SDI=gp3, INT=gp4 (fixed-function)
 *   Buttons          : Blue=gp5 (I/O-2:Pin10), Gold=gp28 (I/O-3:Pin5) — moved
 *                      from v2.8's gp21/gp22; see LCC_NODE_STANDARD.md §3 / §6.1
 *
 * Changes from v2.9:
 *   - IO2_PIN10 moved from gp26 to gp5 (gp5 was a standalone header pin on
 *     v2.9) — this fully frees gp26 for IO3 use; IO2 and IO3 no longer share
 *     a pin.
 *   - Blue/Gold buttons moved off gp21/gp22 (which are now plain IO2 Pin8/9
 *     with no button function) onto gp5 (shared with IO2 Pin10) and gp28
 *     (shared with IO3 Pin5).
 */

#ifndef BOARDPINS_NODE_V30_H
#define BOARDPINS_NODE_V30_H

// --------------------------------------------
//  IO1 — 10-pin IDC connector
// --------------------------------------------
#define IO1_PIN1    8
#define IO1_PIN2    9
#define IO1_PIN3   10
#define IO1_PIN4   11
#define IO1_PIN5   PWR_GND
#define IO1_PIN6   PWR_VCC   // Vselect — jumper-selectable 3.3V/5V, not fixed VCC
#define IO1_PIN7   12
#define IO1_PIN8   13
#define IO1_PIN9   14
#define IO1_PIN10  15

// --------------------------------------------
//  IO2 — 10-pin IDC connector
//  gp5 = Pin10 (also Blue Button) — moved from gp26 on v2.9
// --------------------------------------------
#define IO2_PIN1   16
#define IO2_PIN2   17
#define IO2_PIN3   18
#define IO2_PIN4   19
#define IO2_PIN5   PWR_GND
#define IO2_PIN6   PWR_VCC   // Vselect — jumper-selectable 3.3V/5V, not fixed VCC
#define IO2_PIN7   20
#define IO2_PIN8   21
#define IO2_PIN9   22
#define IO2_PIN10   5

// --------------------------------------------
//  IO3 — 5-pin analog header
//  Pin 3 = AGND, Pin 4 = VREF (not assignable GPIOs)
//  gp28 = Pin5 (also Gold Button)
// --------------------------------------------
#define IO3_PIN1   26
#define IO3_PIN2   27
#define IO3_PIN3   PWR_AGND
#define IO3_PIN4   PWR_VREF
#define IO3_PIN5   28

// --------------------------------------------
//  Buttons
// --------------------------------------------
#define BLUE_BUTTON_PIN   5   // shared with IO2_PIN10
#define GOLD_BUTTON_PIN  28   // shared with IO3_PIN5

// --------------------------------------------
//  I2C1 — EEPROM storage (Wire1, gp6/gp7)
//  Fixed-function traces — not on any connector.
// --------------------------------------------
#define I2C_SDA      6
#define I2C_SCL      7
#define STOR_WIRE    Wire1

// --------------------------------------------
//  CAN transceiver — MCP2517/18 via SPI0 (gp0-gp4)
//  Fixed-function traces — not on any connector.
// --------------------------------------------
#define MCP2517_SPI  SPI
#define MCP2517_SDO   0   // MISO (ACAN_RX_PIN)
#define MCP2517_CS    1
#define MCP2517_SCK   2
#define MCP2517_SDI   3   // MOSI (ACAN_TX_PIN)
#define MCP2517_INT   4

// --------------------------------------------
//  Roundhouse breakout on I/O-1 — functional pin assignments
//  Node board defines available physical resources (above); the breakout
//  plugged into I/O-1 determines what each pin is actually used for.
// --------------------------------------------

// NeoPixel outputs — RESERVED for a future NeoPixel lighting upgrade.
// Not yet wired up in firmware (Roundhouse.cpp has no NeoPixel code today),
// but these four physical pins are spoken for on the breakout, so nothing
// else should be assigned here.
#define NeoPixel_PinA   IO1_PIN1   // gp8
#define NeoPixel_PinB   IO1_PIN2   // gp9
#define NeoPixel_PinC   IO1_PIN3   // gp10
#define NeoPixel_PinD   IO1_PIN4   // gp11

// I2C0 — servo controller (PCA9685)
#define SERVO_SDA   IO1_PIN7   // gp12
#define SERVO_SCL   IO1_PIN8   // gp13

// LEDs
#define Light_A     IO1_PIN9    // gp14
#define Light_B     IO1_PIN10   // gp15

#endif  // BOARDPINS_NODE_V30_H
