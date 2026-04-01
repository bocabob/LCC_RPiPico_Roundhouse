# LCC_RPiPico_Roundhouse

An OpenLCB (LCC) node for a model railroad roundhouse, running on a Raspberry Pi Pico 2. It controls up to 16 servo-driven bay doors and two GPIO lights via the LCC event bus.

---

## Features

- **OpenLCB (LCC) node** — full CAN bus integration using the MustangPeak OpenLcbCLib
- **Up to 16 servo-controlled roundhouse doors** via PCA9685 I2C PWM driver
- **Individual door toggle** plus Open All / Close All events
- **GPIO lighting control** — two independently switched light outputs
- **NeoPixel support** (optional, infrastructure present)
- **Luminosity events** — High / Low brightness modes via CDI-configurable events
- **Persistent state** — door positions and light states stored in external I2C EEPROM; restored at power-up
- **Deferred EEPROM writes** — state changes coalesce into a single write 3 seconds after the last event, preventing write storms
- **LCC Fast Clock consumer** — receives and displays the layout fast clock
- **OTA firmware update** via LCC datagram/stream protocol (PicoOTA + LittleFS)
- **Serial CLI** for NVM management and diagnostic logging
- **Dual-core operation** — Core 0 handles LCC/CAN protocol; Core 1 drives servos in real time

---

## Hardware

| Component | Details |
|---|---|
| **MCU** | Raspberry Pi Pico 2 (`rp2040:rp2040:rpipico2`) |
| **CAN controller** | Microchip MCP2517/2518FD on SPI |
| **Servo driver** | PCA9685 I2C PWM driver at address `0x40` |
| **EEPROM** | 24LC256 (32 KB) at I2C address `0x50` |
| **Lights** | GPIO digital outputs (GPIO 10 and 11) |
| **NeoPixel** | Up to 4 strands on GPIO 2, 3, 6, 7 (optional) |

---

## Pin Assignments

All pin definitions are in [`BoardSettings.h`](BoardSettings.h).

| Signal | GPIO |
|---|---|
| MCP2517 CS | 17 |
| MCP2517 INT | 20 |
| MCP2517 SCK | 18 |
| MCP2517 SDI (MOSI) | 19 |
| MCP2517 SDO (MISO) | 16 |
| EEPROM SDA (Wire1) | 26 |
| EEPROM SCL (Wire1) | 27 |
| Servo SDA (i2c0) | 12 |
| Servo SCL (i2c0) | 13 |
| Light A | 10 |
| Light B | 11 |
| NeoPixel A | 2 |
| NeoPixel B | 6 |
| NeoPixel C | 7 |
| NeoPixel D | 3 |

---

## LCC Node ID

The node ID is set by `#define NODE_ID` in [`LCC_RPiPico_Roundhouse.ino`](LCC_RPiPico_Roundhouse.ino).
The default (`0x050101019416`) is in the Southern Piedmont range assigned to Bob Gamble. Change this to a unique ID from your assigned range before deploying.

---

## Dependencies

Install the following libraries via **Arduino Library Manager** or manually:

| Library | Purpose |
|---|---|
| `ACAN2517` by Pierre Molinaro | MCP2517/2518 CAN transceiver driver |
| `PCA9685_servo_driver` | PCA9685 I2C servo controller |
| `PCA9685_servo` | Individual servo abstraction (constant speed/time modes) |
| `I2C_eeprom` by Rob Tillaart | 24LC256 EEPROM read/write |
| `NeoPixelConnect` | NeoPixel strip control (used in `TTcomms.cpp`) |
| `NeoPixelBus` | NeoPixel type definitions (used in `TTvariables.h`) |
| `LibPrintf` | `printf()` support over Serial |
| `PicoOTA` | Over-the-air firmware update (Philhower core) |
| `LittleFS` | Flash filesystem for OTA image staging |
| `Wire` | I2C (built into Arduino core) |
| `SPI` | SPI (built into Arduino core) |

The OpenLCB stack (`src/openlcb/`, `src/drivers/canbus/`) is included in the repository as the MustangPeak OpenLcbCLib — no separate install needed.

> **Board package:** Use [Earle Philhower's RP2040 package](https://github.com/earlephilhower/arduino-pico#installation) — **not** the Mbed package.
> Board target: `rp2040:rp2040:rpipico2`

---

## Build Configuration

See [`sketch.yaml`](sketch.yaml) for the full build profile:

- Flash: 4 MB - 2 MB filesystem space for firmware updates
- Optimization: `Small`
- C++ standard: `gnu++17`

---

## Configuration Memory (CDI)

Node configuration (door count, servo ranges, door speed, event IDs, etc.) is stored in the external EEPROM and described by the CDI XML in [`Documentation/openlcb-config-2026-01-30.xml`](Documentation/openlcb-config-2026-01-30.xml). Use any LCC configuration tool (e.g., JMRI's PanelPro) to edit these settings over the LCC bus.

The memory layout is auto-generated — do not hand-edit [`Documentation/config_mem_map.h`](Documentation/config_mem_map.h).

---

## Serial CLI Commands

Connect at **115200 baud**. Available commands:

| Key | Action |
|---|---|
| `h` | Print help |
| `c` | Clear NVM to `0x00` |
| `i` | Reset NVM to CDI default values |
| `r` | Reset NVM to `0xFF` (factory fresh) |
| `p` | Toggle LCC message logging |
| `m` | Toggle config memory read/write logging |
| `t` | Print current fast clock time |
| `q` | Query fast clock |

---

## Architecture

The Pico runs two cores:

- **Core 0** (`setup()` / `loop()`): OpenLCB protocol stack, CAN comms, event handling, serial CLI
- **Core 1** (`setup1()` / `loop1()`): Real-time servo drive loop (`driveServos()`)

Key modules:

| File | Role |
|---|---|
| `LCC_RPiPico_Roundhouse.ino` | Entry point, node init, event registration, serial CLI |
| `Roundhouse.cpp` | PCA9685 servo control, door open/close, GPIO lights, servo polling loop |
| `TTcomms.cpp` | NeoPixel strip control (on/off/toggle/dimmer) |
| `callbacks.cpp` | OpenLCB event handlers, 100ms timer, CAN Rx/Tx, OTA firmware |
| `config_mem_helper.cpp` | EEPROM config storage, CDI memory map |
| `BoardSettings.h` | All hardware pin and address definitions |
| `TTvariables.h` | Shared type definitions (`ServoAddress`, `LightAddress`, `npStrings`) |

---

## License

BSD 2-Clause — see [`LICENSE`](LICENSE).
