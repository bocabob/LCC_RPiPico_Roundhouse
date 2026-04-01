# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Environment

- **IDE**: Arduino IDE (primary) or VSCode with Arduino extension
- **Board**: Raspberry Pi Pico 2 (`rp2040:rp2040:rpipico2`) using [Philhower's RP2040 package](https://github.com/earlephilhower/arduino-pico#installation) — NOT the Mbed package
- **C++ Standard**: gnu++17
- **Build config**: [sketch.yaml](sketch.yaml) (4MB flash, optimization: small)

Required libraries (install via Arduino Library Manager):
- `ACAN2517` by Pierre Molinaro — CAN transceiver (MCP2517/18)
- `PCA9685_servo_driver` — PCA9685 I2C PWM controller
- `PCA9685_servo` — individual servo abstraction (constant speed/time modes)
- `I2C_eeprom` by Rob Tillaart — 24LC256 EEPROM read/write
- `NeoPixelConnect` — NeoPixel strip control (used in TTcomms.cpp)
- `NeoPixelBus` — NeoPixel type definitions (used in TTvariables.h)
- `LibPrintf` — `printf()` support over Serial
- `Wire`, `SPI` — built into Arduino core

There are no automated tests. Manual testing uses serial console commands in `loop()`:
- `'c'` clear NVM, `'i'` reset to CDI defaults, `'r'` factory reset, `'p'` toggle message logging, `'m'` toggle config memory logging, `'t'` print fast clock time, `'q'` query fast clock

## Architecture

This is an **OpenLCB (LCC) node** controlling a model railroad roundhouse — up to 16 servo-driven bay doors, two GPIO light outputs, and optional NeoPixel strip lighting — over CAN bus networking.

### Dual-Core Structure

The Pico runs two cores:
- **Core 0** (`setup()`/`loop()`): OpenLCB protocol, CAN comms, event handling, serial CLI
- **Core 1** (`setup1()`/`loop1()`): real-time servo drive loop (`driveServos()`)

Core synchronization: `setup1()` delays 1 second then sets `setup1Complete = true`; Core 0 waits at `while(!setup1Complete)` before registering consumers/producers. Core 1 then waits on `node_initiated` before entering `loop1()`.

### Key Module Responsibilities

| File | Role |
|---|---|
| [LCC_RPiPico_Roundhouse.ino](LCC_RPiPico_Roundhouse.ino) | Entry point, node init, consumer/producer registration, serial CLI |
| [Roundhouse.cpp](Roundhouse.cpp) | PCA9685 servo control, door open/close, GPIO lights, servo polling loop |
| [TTcomms.cpp](TTcomms.cpp) | NeoPixel strip control (on/off/toggle/dimmer) |
| [callbacks.cpp](callbacks.cpp) | OpenLCB event handlers (consumed/produced), 100ms timer, CAN rx/tx |
| [config_mem_helper.cpp](config_mem_helper.cpp) | I2C EEPROM config storage, CDI-driven memory map |
| [BoardSettings.h](BoardSettings.h) | All hardware pin and address definitions |
| [TTvariables.h](TTvariables.h) | Shared type definitions (`ServoAddress`, `LightAddress`, `npStrings`, `npHead`) |

### Configuration Memory

Config is stored in external I2C EEPROM using a CDI (XML)-generated memory map:
- [Documentation/config_mem_map.h](Documentation/config_mem_map.h) — auto-generated layout (do not hand-edit)
- [Documentation/config_mem_reset.c](Documentation/config_mem_reset.c) — auto-generated defaults
- [Documentation/openlcb-config-2026-01-30.xml](Documentation/openlcb-config-2026-01-30.xml) — CDI descriptor

Config structures use `#pragma pack(push, 1)` for exact memory layout.

### Key Data Flow

1. **Door toggle**: LCC consumed event → `callbacks.cpp` → `RoundhouseCallback()` → `MoveServo()` → PCA9685 PWM → servo moves
2. **Servo stop detection**: `driveServos()` (Core 1) polls each servo for moving→stopped transition → sets `_pending_door_pcer[]` flag
3. **PCER on stop**: `Roundhouse_send_pending_door_pcers()` (called from 100ms timer on Core 0) reads `_pending_door_pcer[]` flags and sends Producer/Consumer Event Report
4. **Open All / Close All**: LCC consumed event → `RoundhouseCallback()` → loop over all configured doors
5. **NeoPixel control**: LCC Interior/Exterior events → `callbacks.cpp` → `TogglePixels()` in `TTcomms.cpp`
6. **Deferred NVM writes**: state changes set `_config_dirty = true`; 100ms timer counts 30 ticks (~3 sec) then flushes to EEPROM

### Naming Conventions

- Functions: `camelCase` (`MoveServo`, `driveServos`) and `PascalCase` for public API (`RoundhouseCallback`)
- Global variables: `camelCase` or `snake_case`
- Structs: `PascalCase` (`ServoAddress`, `LightAddress`, `npHead`)
- Constants/defines: `UPPER_SNAKE_CASE`

### Important Implementation Notes

- **PCA9685 callback workaround**: the `PCA9685_servo_driver` library's callback always passes address `0` regardless of which servo stopped. `driveServos()` uses a polling loop indexed by servo number to detect the stopped state instead of relying on the callback.
- **Servo stop flag pattern**: Core 1 sets `_pending_door_pcer[i] = true` when servo `i` transitions to stopped. Core 0 reads and clears that flag in `Roundhouse_send_pending_door_pcers()` (called from the 100ms timer), keeping LCC event traffic on Core 0.
- **`_commandedDir[]` convention**: `-1` = initial boot move (skip NVM write); `0` = close; `1` = open
- **Deferred EEPROM writes**: direct EEPROM writes from the event callback block Core 0 for 200–500ms, stalling CAN bus processing. All state changes set `_config_dirty = true`; the 100ms timer flushes to EEPROM after a 3-second quiet period.
- The OpenLCB stack lives entirely in `src/openlcb/` and `src/drivers/canbus/` — this is the MustangPeak OpenLcbClib C library; do not modify files under `src/`.
