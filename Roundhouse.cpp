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

/*=============================================================
 * This file contains all functions pertinent to roundhouse
 * operation including servo-controlled door movements and
 * GPIO light control functions.
=============================================================*/

#include "Roundhouse.h"
#include "BoardSettings.h"
#include "mdebugging.h"
#include "config_mem_helper.h"
#include "callbacks.h"  // for _config_dirty deferred EEPROM write flag
#include "openlcb_user_config.h"
#include "src/openlcb/openlcb_application.h"
#include "src/utilities/mustangpeak_endian_helper.h"

#include <PCA9685_servo_driver.h>
#include <PCA9685_servo.h>


void StartMoveHandler(uint16_t Address);	// Servo callback
void StopMoveHandler(uint16_t Address);		// Servo callback
void init_servo(PCA9685_servo& servo, uint8_t mode, int8_t minRange, int8_t maxRange, int8_t position, uint8_t address, uint64_t TConstDur);

uint8_t ledState = 7;                               // Flag for the LED state: 4 on, 5 slow, 6 fast, 7 off.
bool ledOutput = LOW;                               // Boolean for the actual state of the output LED pin.
unsigned long ledMillis = 0;                        // Required for non blocking LED blink rate timing.

extern void DimmerHigh();
extern void DimmerLow();

LightAddress Lights[NumOfLights];


ServoAddress Servos[MAX_DOORS];
// _commandedDir: -1 = no user command pending (init/setup move – skip NVM write),
//                 0 = close commanded by MoveServo(),
//                 1 = open  commanded by MoveServo().
// Initialised to 0 by the C++ runtime; setServoDefaults() resets to -1 before
// setupServos() so that init-time servo moves don't trigger NVM writes.
static int8_t _commandedDir[MAX_DOORS];

// Per-door flag set by StopMoveHandler() (Core 1) when a servo finishes moving.
// Roundhouse_send_pending_door_pcers() (Core 0) reads this flag, sends the PCER,
// and clears it only on a successful send so it retries if the TX buffer was full.
// volatile ensures Core 0 always reads the value written by Core 1.
volatile bool _pending_door_pcer[MAX_DOORS];

// Per-door previous isMoving state used by driveServos() to detect the
// moving→stopped transition for each servo individually.  The PCA9685_servo
// library's onStopMove callback always passes address 0, so we cannot rely on
// it; instead we track transitions here using the known loop index.
static bool _servo_was_moving[MAX_DOORS];

// Create the PCA9685 controller and the servo vector (one entry per physical channel)
PCA9685_servo_driver myController(SERVO_I2C, SERVO_SDA, SERVO_SCL, SERVO_ADDRESS);
std::vector<PCA9685_servo> myServo = {PCA9685_servo(&myController, 0, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 1, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 2, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 3, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 4, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 5, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 6, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 7, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 8, myPWMmin, myPWMmax),
                                      PCA9685_servo(&myController, 9, myPWMmin, myPWMmax)}; // define a vector of servos, note this could extend to use multiple controller on the i2c bus

// necessary variables to drive the servo with constant speed or time
uint64_t TNow = 0;
uint64_t TPrevious = 0;
uint64_t TElapsed = 0;
uint64_t start_t = 0;


// ===== Process Consumer-eventIDs =====
void RoundhouseCallback(uint16_t callin) {
  dP("\nEventid callback: index="); dP((uint16_t)callin);
  uint16_t index;
  uint8_t current;
  uint8_t target;
  index = callin;
  // Invoked when an event is consumed;
  // drive actions as needed from index of all events.
  dPS((const char*)"\npceCallback: Event Index: ", index);
   
    if (index < 2) { // if the event is an ALL door event
        switch (index) { //, , , , 
          case 0:  //   OpenAll
            for (int i = 0; i < ConfigMemHelper_config_data.attributes.DoorCount; i++) {
              MoveServo(i, 1);
            }
          break;
          case 1:  //  CloseAll
            for (int i = 0; i < ConfigMemHelper_config_data.attributes.DoorCount; i++) {
              MoveServo(i, 0);
            }
          break;
          default:
            // do nothing
          break;
        }
      }
      else {
        if (index < 2 + ConfigMemHelper_config_data.attributes.DoorCount) { // if the event is a door event
        uint8_t servoNum = index - 2; // get the servo number from the event index
        // toggleDoor(servoNum);
                if (Servos[servoNum].Status)
                {              MoveServo(servoNum, 0);            }
                else
                {              MoveServo(servoNum, 1);            }
        }
        else {
        index = index - (2 + ConfigMemHelper_config_data.attributes.DoorCount); // adjust the index to account for the door events
        switch (index) { //, , , , 
          case 0:  //        
          ToggleLight(0);
          break;
          case 1:  // 
          ToggleLight(1);
          break;
          case 2:  //  PEID(eidExterior)
            DimmerHigh();      // turn dimmer off, go to high luminosity
          break;
          case 3:  //  CEID(eidLowLuminosity_On)
            DimmerLow();       // go to low luminosity 
          break;
          default:
            // do nothing
          break;
        }}
      }
}

void init_servo(PCA9685_servo& servo, uint8_t mode, int8_t minRange, int8_t maxRange, int8_t position, uint8_t address, uint64_t TConstDur)
{
    servo.setRange(minRange, maxRange);
    servo.setMode(mode);
    servo.setPosition(position); // move to mid point
    servo.setAddress(address);
    servo.setTConstantDuration(TConstDur);
    servo.setAngularVelocity(ConfigMemHelper_config_data.attributes.DoorSpeed);
    servo.setInvertMode(inversion);
    // NOTE: We do NOT assign onStartMove / onStopMove here.
    // The PCA9685_servo library always passes address 0 to these callbacks
    // regardless of which servo completed, so they cannot be used to identify
    // the door index reliably.  Stop detection is done by polling isMoving()
    // inside driveServos() where the loop index is known (see below).
}

void updateServoRangesFromConfig() {
  for (int i = 0; i < ConfigMemHelper_config_data.attributes.DoorCount && i < (int)myServo.size(); i++) {
      myServo[i].setRange((int8_t)(ConfigMemHelper_config_data.attributes.doors[i].servo_min - 90),
                          (int8_t)(ConfigMemHelper_config_data.attributes.doors[i].servo_max - 90));
      myServo[i].setAngularVelocity(ConfigMemHelper_config_data.attributes.DoorSpeed);
  }
}

void setupServos(){
  // I2C bus recovery: clock SCL 9 times to release a stuck slave
  gpio_init(SERVO_SCL);
  gpio_set_dir(SERVO_SCL, GPIO_OUT);
  for (int n = 0; n < 9; n++) {
    gpio_put(SERVO_SCL, 1); sleep_us(5);
    gpio_put(SERVO_SCL, 0); sleep_us(5);
  }
  gpio_put(SERVO_SCL, 1); sleep_us(5);  // STOP condition
  // Now hand the pin back to I2C hardware
  gpio_set_function(SERVO_SCL, GPIO_FUNC_I2C);

  // Initialize PCA9685
  myController.begin(100000);   // create connection to the PCA9685 and initialize it

  myController.setOscillatorFrequency(27000000);
  myController.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  // initialize the servos created
  int i{0};
  for(auto& servo : myServo){
      init_servo(servo, MODE_SCONSTANT, (int8_t)(ConfigMemHelper_config_data.attributes.doors[i].servo_min - 90), (int8_t)(ConfigMemHelper_config_data.attributes.doors[i].servo_max - 90), Servos[i].Position, i++, SERVO_DURATION);
  }

  start_t = time_us_64();
        
  /*
   * In theory the internal oscillator (clock) is 25MHz but it really isn't
   * that precise. You can 'calibrate' this by tweaking this number until
   * you get the PWM update frequency you're expecting!
   * The int.osc. for the PCA9685 chip is a range between about 23-27MHz and
   * is used for calculating things like writeMicroseconds()
   * Analog servos run at ~50 Hz updates, It is importaint to use an
   * oscilloscope in setting the int.osc frequency for the I2C PCA9685 chip.
   * 1) Attach the oscilloscope to one of the PWM signal pins and ground on
   *    the I2C PCA9685 chip you are setting the value for.
   * 2) Adjust setOscillatorFrequency() until the PWM update frequency is the
   *    expected value (50Hz for most ESCs)
   * Setting the value here is specific to each individual I2C PCA9685 chip and
   * affects the calculations for the PWM update frequency. 
   * Failure to correctly set the int.osc value will cause unexpected PWM results
   */
	delay(10);
}


void StartMoveHandler(uint16_t Address)
{
    // called when a servo starts to move
    SetServoStatus(Address, 2); // set the Status of the servo to "deviate" when it starts to move
    return;
}

void StopMoveHandler(uint16_t Address)
{
    // Called by the PCA9685 servo library when a servo finishes its move.
    // Finalise the servo status using the direction that was commanded in MoveServo(),
    // update the NVM-backed position, and write to NVM so state survives a power cycle.
    if (Address >= MAX_DOORS) return;
    // Guard: if _commandedDir is -1 the move was initiated by setupServos() / init_servo()
    // (restoring the servo to its last known position at boot), NOT by a user command.
    // Skip the NVM write in that case to avoid overwriting the restored state with
    // a default "closed" value and to prevent a write storm at startup.
    if (_commandedDir[Address] < 0) return;
    // Compute the final angle from the commanded direction
    int8_t finalPos = _commandedDir[Address]
        ? (int8_t)(ConfigMemHelper_config_data.attributes.doors[Address].servo_max - 90)
        : (int8_t)(ConfigMemHelper_config_data.attributes.doors[Address].servo_min - 90);
    Servos[Address].Position = finalPos;
    ConfigMemHelper_config_data.Servos[Address].Position = finalPos;
    // SetServoStatus with 0 or 1 will also write NVM
    SetServoStatus(Address, _commandedDir[Address]);
    // Signal Core 0 to broadcast a PCER for this door so that consumers (e.g. the
    // Turntable display) update to the confirmed final state rather than relying
    // solely on the optimistic toggle they applied when they sent the command.
    _pending_door_pcer[Address] = true;
}

void driveServos()
/* call this from the program loop (Core 1).
   Advances each servo by one time-step, then checks whether it has just
   stopped moving.  We use an indexed loop so we always know which door
   number has completed — the PCA9685_servo library's onStopMove callback
   always passes 0 regardless of which servo stopped, so it cannot be used
   for multi-door identification. */
{
  TNow = time_us_64();          // time now in microseconds
  TElapsed = TNow - TPrevious;  // time, in microseconds, since the last loop
  TPrevious = TNow;             // store this ready for the next loop

  for (int i = 0; i < (int)myServo.size() && i < MAX_DOORS; i++) {
    myServo[i].loop(TElapsed);

    bool nowMoving = (bool)myServo[i].isMoving();

    // Detect the moving→stopped transition for this specific door index.
    if (_servo_was_moving[i] && !nowMoving) {
      StopMoveHandler((uint16_t)i);
    }

    _servo_was_moving[i] = nowMoving;
  }
}

void initializeHardware() {
  Lights[0].pin = Light_A;
  Lights[1].pin = Light_B;

  for (int Light = 0; Light < NumOfLights; Light++) {
    pinMode(Lights[Light].pin, OUTPUT);
    digitalWrite(Lights[Light].pin, LOW);  // start with lights off
    Lights[Light].active = false;
  }
}


void LightSwitch(int Light, int dir)  // pin driven LED
{                                              
  if (Light > (NumOfLights - 1)) return;
  if (dir)
    {
      digitalWrite(Lights[Light].pin, HIGH); // turn on
    }
    else
    {
      digitalWrite(Lights[Light].pin, LOW); // turn off
    }
#ifdef USE_SENSORS
      // LN_STATUS lnStatus = LocoNet.reportSensor(LightAddr[Light], dir);
      // reportSensor(&LNbus, Lights[Light].address, dir);
      Serial.print(F("Tx: Sensor: "));
      Serial.println(Lights[Light].address);
      // Serial.print(" Status: ");
      // Serial.println(LocoNet.getStatusStr(lnStatus));
#endif
}    

void ToggleLight(int Light)  // pin driven LED
{                                               
if (Light > (NumOfLights - 1)) return;
  if (Lights[Light].active)
    {
      digitalWrite(Lights[Light].pin, LOW); // turn off
      Lights[Light].active = false;
      Serial.print(F("Light off: "));
      Serial.println(Light);
    }
    else
    {
      digitalWrite(Lights[Light].pin, HIGH); // turn on
      Lights[Light].active = true;
      Serial.print(F("Light on: "));
      Serial.println(Light);
    }
}

void MoveServo(int i, int dir)
{
  if (i > (MAX_DOORS - 1)) return;
  Serial.print(F("Activating Servo : "));
  Serial.println(i, DEC);

  _commandedDir[i] = (int8_t)dir;  // remember direction so StopMoveHandler can finalise state
  // Set status to "in motion" now.  The final 0/1 state (and the NVM write) happens
  // in StopMoveHandler() once the servo has actually reached its target position.
  // Do NOT call SetServoStatus(i, dir) here: that would trigger a ConfigMemHelper_write()
  // before the servo has moved, and for OpenAll/CloseAll commands it would produce
  // DoorCount consecutive blocking EEPROM writes (hundreds of ms each), stalling the
  // CAN bus and risking an I2C lockup if interrupted.
  SetServoStatus(i, 2);
  // When MoveServo() activates a servo:
  // Call setPosition(finalTarget) ONCE, then just let loop() run.
  
  int8_t target = dir ? (int8_t)(ConfigMemHelper_config_data.attributes.doors[i].servo_max - 90)
                       : (int8_t)(ConfigMemHelper_config_data.attributes.doors[i].servo_min - 90);
  myServo[i].setPosition(target);

    if (dir)
    {
      Serial.print(F("Opening : "));
      Serial.println(i, DEC);
    }
    else
    {
      Serial.print(F("Closing : "));
      Serial.println(i, DEC);
    }
}  

void SetServoStatus(int i, int status)
// Status values for a servo:
//   0 = closed (door at servo_min)
//   1 = open   (door at servo_max)
//   2 = in motion / unknown
{
  if (i > (MAX_DOORS - 1)) return;
  switch (status)
  {
  case 0:
    /* closed */
    OpenLcbUserConfig_node_id->consumers.list[i+2].status = EVENT_STATUS_CLEAR;
    ConfigMemHelper_config_data.consumer_status[i+2] = EVENT_STATUS_CLEAR;
    OpenLcbUserConfig_node_id->producers.list[i].status = EVENT_STATUS_CLEAR;
    ConfigMemHelper_config_data.producer_status[i] = EVENT_STATUS_CLEAR;
    break;
  case 1:
    /* open */
    OpenLcbUserConfig_node_id->consumers.list[i+2].status = EVENT_STATUS_SET;
    ConfigMemHelper_config_data.consumer_status[i+2] = EVENT_STATUS_SET;
    OpenLcbUserConfig_node_id->producers.list[i].status = EVENT_STATUS_SET;
    ConfigMemHelper_config_data.producer_status[i] = EVENT_STATUS_SET;
    break;
  case 2:
    /* in motion – consumer side goes UNKNOWN; producer stays at last known state */
    OpenLcbUserConfig_node_id->consumers.list[i+2].status = EVENT_STATUS_UNKNOWN;
    ConfigMemHelper_config_data.consumer_status[i+2] = EVENT_STATUS_UNKNOWN;
    break;

  default:
    break;
  }
  Servos[i].Status = status;
  // Mark the RAM config as dirty so the 100ms timer will flush to EEPROM within
  // ~3 seconds.  Writing directly here (blocking 200-500ms per call) caused
  // CAN bus stalls and potential I2C lockups on rapid open/close commands.
  if (status == 0 || status == 1) {
    ConfigMemHelper_config_data.Servos[i].Status = status;
    _config_dirty = true;
  }
}

void setServoDefaults()
// Called after ConfigMemHelper_read() so ConfigMemHelper_config_data is already populated.
// If the NVM holds a valid state (0=closed, 1=open) restore the servo to that position;
// otherwise default to closed.  Does NOT call SetServoStatus() because the OpenLCB node
// event lists are not yet registered at this point – we write directly to the RAM mirrors.
{
  // Mark all directions as "no user command" so that the init-time servo moves triggered
  // by setupServos() / init_servo() do NOT cause NVM writes in StopMoveHandler().
  memset(_commandedDir, -1, sizeof(_commandedDir));
  // Clear all pending PCER flags so no spurious broadcasts fire on first boot.
  memset((void*)_pending_door_pcer, 0, sizeof(_pending_door_pcer));
  // Clear the isMoving transition-tracker so driveServos() starts clean.
  memset(_servo_was_moving, 0, sizeof(_servo_was_moving));

  for (int i = 0; i < MAX_DOORS; i++) {
    int nvmStatus = ConfigMemHelper_config_data.Servos[i].Status;
    if (nvmStatus == 0 || nvmStatus == 1) {
      // Valid NVM state – restore servo position and sync RAM status arrays
      Servos[i].Status   = ConfigMemHelper_config_data.Servos[i].Status;
      Servos[i].Position = ConfigMemHelper_config_data.Servos[i].Position;
      Servos[i].ServoMin = ConfigMemHelper_config_data.Servos[i].ServoMin;
      Servos[i].ServoMax = ConfigMemHelper_config_data.Servos[i].ServoMax;
      event_status_enum ev = (nvmStatus == 1) ? EVENT_STATUS_SET : EVENT_STATUS_CLEAR;
      ConfigMemHelper_config_data.consumer_status[i + 2] = ev;
      ConfigMemHelper_config_data.producer_status[i]     = ev;
    } else {
      // NVM not yet initialised – default to closed (servo_min)
      Servos[i].Status   = 0;
      Servos[i].Position = (int8_t)(ConfigMemHelper_config_data.attributes.doors[i].servo_min - 90);
      ConfigMemHelper_config_data.consumer_status[i + 2] = EVENT_STATUS_CLEAR;
      ConfigMemHelper_config_data.producer_status[i]     = EVENT_STATUS_CLEAR;
    }
  }
/*  servo_min and servo_max are configured per-door in CDI config (config_mem_helper) */
}

void Roundhouse_send_pending_door_pcers()
// Called from Core 0 (Callbacks_on_100ms_timer_callback) each 100 ms tick.
// For every door whose _pending_door_pcer flag is set, attempts to send a PCER
// so that consumers (e.g. the Turntable display) receive the confirmed final state.
// The flag is cleared only on a successful send; if the TX buffer is full the flag
// stays set and the send is retried on the next tick.
{
  for (int i = 0; i < ConfigMemHelper_config_data.attributes.DoorCount; i++) {
    if (_pending_door_pcer[i]) {
      if (OpenLcbApplication_send_event_pc_report(
              OpenLcbUserConfig_node_id,
              swap_endian64(ConfigMemHelper_config_data.attributes.doors[i].ToggleDoor))) {
        _pending_door_pcer[i] = false;
      }
    }
  }
}