# LCC_RPiPico_Roundhouse

An OpenLCB (LCC) node for a model railroad roundhouse, running on a Raspberry Pi Pico /
Pico 2 (v3.0 generic Node board). It controls up to 16 servo-driven bay doors and two
GPIO lights via the LCC event bus, and reports confirmed door state live to a companion
Turntable node.

This project is part of a family of node firmwares sharing a common platform — see
[`LCC_RPiPico_Common/LCC_NODE_STANDARD.md`](../LCC_RPiPico_Common/LCC_NODE_STANDARD.md)
for the cross-project conventions this README assumes, and
[`LCC_RPiPico_Common/CLAUDE.md`](../LCC_RPiPico_Common/CLAUDE.md) as the entry point for
Claude Code sessions working across the family.

**Node ID range:** `05.01.01.01.94.xx` — assigned to Bob Gamble / Southern Piedmont

---

## Features

- **OpenLCB (LCC) node** — full CAN bus integration using the MustangPeak OpenLcbCLib
- **Up to 16 servo-controlled roundhouse doors** via a PCA9685 I2C PWM driver
- **Paired `DoorOpen`/`DoorClose` events** — each registered as both producer and
  consumer per door (the standard OpenLCB command+feedback pattern, same as turnout
  control): an explicit directional command is self-correcting regardless of
  Roundhouse's currently-tracked state, and confirmed door state broadcasts live via a
  PC Event Report on every completed move, not just at LCC login
- **Open All / Close All** events across every configured door
- **GPIO lighting control** — two independently switched light outputs
- **NeoPixel support** (optional, infrastructure present)
- **Luminosity events** — High / Low brightness modes via CDI-configurable events
- **Persistent state** — door positions and light states stored in external I2C
  EEPROM; restored at power-up
- **Deferred EEPROM writes** — state changes coalesce into a single write ~3 seconds
  after the last event, preventing write storms
- **LCC Fast Clock consumer** — receives and displays the layout fast clock
- **Protected node identity** — node ID survives config wipes/EEPROM_VERSION bumps;
  provisioned via the serial `N`/`Y` command pair (Node Standard §7.1)
- **Factory reset gesture** — hold Blue+Gold for 2s at boot to wipe config to CDI
  defaults without a serial connection (§7.2)
- **OTA firmware update** via LCC datagram/stream protocol (PicoOTA + LittleFS)
- **Serial CLI** for NVM management and diagnostic logging
- **Dual-core operation** — Core 0 handles LCC/CAN protocol; Core 1 drives servos in
  real time

---

## Board Support

The active hardware target is the **v3.0 generic Node board** — see the Node Standard's
§4 (Board Versioning) for the full family history. Unlike Turntable/PixelLights/
Clock_Lights, this project has no `NodeConfig.h` functional-pin layer; on v3.0, all of
its functional pins (servo I2C, lights, NeoPixel) are defined directly in
`board_configs/BoardPins_Node_v30.h` itself.

Edit [`ProjectConfig.h`](ProjectConfig.h) — the **single source of truth** for board
selection:

```cpp
#define LCC_BOARD_NODE_V30  // v3.0 board (current generic NODE board)
```

---

## Hardware — v3.0

| Component | Details |
|---|---|
| **MCU** | Raspberry Pi Pico / Pico 2, v3.0 generic Node carrier board |
| **CAN controller** | MCP2517/2518FD on SPI0 (gp0–4) — fixed-function on the Node board |
| **EEPROM** | 24LC256 (32 KB) on I2C1 (gp6/7), address `0x50` |
| **Servo driver** | PCA9685 I2C PWM driver, address `0x40` |
| **Lights** | GPIO digital outputs |
| **NeoPixel** | Optional, single string |

### Pin Assignments (v3.0)

| Signal | GPIO | Connector |
|---|---|---|
| CAN (MCP2517/18 SPI0) | gp0–4 | fixed |
| EEPROM I2C1 (SDA/SCL) | gp6/gp7 | fixed |
| NeoPixel | gp8 | I/O-1 pin 1 |
| Servo I2C SDA/SCL | gp12/13 | I/O-1 pins 7/8 |
| Light A | gp14 | I/O-1 pin 9 |
| Light B | gp15 | I/O-1 pin 10 |
| Blue Button | gp5 | I/O-2 pin 10 |
| Gold Button | gp28 | I/O-3 pin 5 |

See [`board_configs/BoardPins_Node_v30.h`](board_configs/BoardPins_Node_v30.h) for the
authoritative mapping.

---

## LCC Node Identity

The node's LCC node ID is **not** a hardcoded `#define` — it lives in a protected NVM
region that survives config wipes and `EEPROM_VERSION` bumps (Node Standard §7.1). On
an unprovisioned board it falls back to a legacy default and prints a warning:

```cpp
#define NODE_ID_DEFAULT 0x050101019436   // fallback only — provision a real ID below
```

Provision (or re-provision) a node over serial, two-step with confirmation:

```
N050101019436        → node replies "Confirm with 'Y' to write 05:01:01:01:94:36"
Y                    → node writes the identity block and reboots
```

---

## Dependencies

Install the following libraries via **Arduino Library Manager** or manually:

| Library | Purpose |
|---|---|
| `ACAN2517` by Pierre Molinaro | MCP2517/2518 CAN transceiver driver |
| `PCA9685_servo_driver` | PCA9685 I2C servo controller |
| `PCA9685_servo` | Individual servo abstraction (constant speed/time modes) |
| `I2C_eeprom` by Rob Tillaart | EEPROM read/write (`USE_TILLAART`) |
| `NeoPixelConnect` | NeoPixel strip control |
| `LibPrintf` | `printf()` support over Serial |
| `PicoOTA` | Over-the-air firmware update (Philhower core) |
| `LittleFS` | Flash filesystem for OTA image staging |
| `Wire`, `SPI` | Built into the Arduino core |

The OpenLCB stack (`src/openlcb/`, `src/drivers/canbus/`) is vendored as the
MustangPeak OpenLcbCLib — see it as a fixed external dependency (Node Standard §10);
do not modify files under `src/`.

> **Board package:** Use [Earle Philhower's RP2040 package](https://github.com/earlephilhower/arduino-pico#installation) — **not** the Mbed package.
> Board target: `rp2040:rp2040:rpipico2`

---

## Build Configuration

See [`sketch.yaml`](sketch.yaml) for the full build profile:

- Flash: 4 MB — 2 MB filesystem space for firmware updates
- Optimization: `Small`
- C++ standard: `gnu++17`

---

## Configuration Memory (CDI)

Node configuration (door count, servo ranges, door speed, event IDs, etc.) is stored in
external EEPROM and described by [`CDI.xml`](CDI.xml). Edit it with any LCC
configuration tool (e.g. JMRI's PanelPro) over the LCC bus.

`openlcb_user_config.c`'s compiled `_cdi_data[]` byte array must be kept in sync with
`CDI.xml` by hand any time the XML changes — regenerate it with
[`LCC_RPiPico_Common/cdi_to_c_array.py`](../LCC_RPiPico_Common/cdi_to_c_array.py)
rather than hand-editing the array (see the Node Standard §7 for the exact splicing
procedure).

---

## Serial CLI Commands

Connect at **115200 baud**. Commands common across the node family (Node Standard §11)
plus this project's own:

| Key | Action |
|---|---|
| `h` | Print help |
| `c` | Clear NVM to `0x00` |
| `i` | Reset NVM to CDI default values |
| `r` | Factory reset (NVM to `0xFF`, then reinitialize) |
| `p` | Toggle LCC message logging |
| `m` | Toggle config memory read/write logging |
| `t` | Print current fast clock time |
| `q` | Query fast clock |
| `N` | Provision/re-provision node identity — two-step, confirm with `Y` |

---

## Architecture

The Pico runs two cores (Node Standard §8 dual-core contract):

- **Core 0** (`setup()` / `loop()`): OpenLCB protocol stack, CAN comms, event handling,
  serial CLI
- **Core 1** (`setup1()` / `loop1()`): real-time servo drive loop (`driveServos()`) —
  never blocks on `delay()`, EEPROM I/O, or CAN traffic

Key modules:

| File | Role |
|---|---|
| `LCC_RPiPico_Roundhouse.ino` | Entry point: node init, event registration, serial CLI |
| `ProjectConfig.h` | Single switch: board macro |
| `BoardSettings.h` | Dispatches to `board_configs/`; also holds this project's functional pin defines (no separate `NodeConfig.h`) |
| `board_configs/BoardPins_Node_v30.h` | v3.0 pin topology *and* functional assignments (servo I2C, lights, NeoPixel) |
| `Roundhouse.cpp` | PCA9685 servo control, paired door-event dispatch, GPIO lights, servo polling loop |
| `TTcomms.cpp` | NeoPixel strip control (on/off/toggle/dimmer) |
| `callbacks.cpp` | OpenLCB event handlers, 100ms timer, CAN Rx/Tx, OTA firmware — the only place consumers/producers are registered (Node Standard §10) |
| `config_mem_helper.cpp` | EEPROM config storage, CDI memory map |
| `NodeIdentity.h` / `.cpp` | Protected-NVM node identity block (Node Standard §7.1) |
| `TTvariables.h` | Shared type definitions (`ServoAddress`, `LightAddress`, `npStrings`) |

---

## License

BSD 2-Clause — see [`LICENSE`](LICENSE).
