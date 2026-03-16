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
 * This file contains all functions pertinent to turntable
 * operation including stepper movements, relay phase switching,
 * and LED/accessory related functions.
=============================================================*/

#include "Roundhouse.h"
#include "BoardSettings.h"
// #include "TTcomms.h"
#include "mdebugging.h"
// #include "TTvariables.h"
#include "config_mem_helper.h"

// #include <Adafruit_PWMServoDriver.h>
#include <PCA9685_servo_driver.h>
#include <PCA9685_servo.h>


void StartMoveHandler(uint16_t Address);	// Servo callback
void StopMoveHandler(uint16_t Address);		// Servo callback
void init_servo(PCA9685_servo& servo, uint8_t mode, int16_t minRange, int16_t maxRange, int16_t position, uint8_t address, uint64_t TConstDur);

const int16_t totalMinutes = 21600;                 // Total minutes in one rotation (360 * 60)

uint8_t trackCount = 0;                             // count of turntable tracks

uint8_t ledState = 7;                               // Flag for the LED state: 4 on, 5 slow, 6 fast, 7 off.
bool ledOutput = LOW;                               // Boolean for the actual state of the output LED pin.
unsigned long ledMillis = 0;                        // Required for non blocking LED blink rate timing.

extern void DimmerHigh();
extern void DimmerLow();

// typedef struct
// {
// 	int pin;
// 	bool active;
// }
// LightAddress;
LightAddress Lights[NumOfLights];

// int LightPin[NumOfLights] {Light_A, Light_B};
// int LightAddr[NumOfLights] {700, 701};


ServoAddress Servos[MAX_DOORS];

// Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(SERVO_ADDRESS); // to manage servos from 1  to 16 (addresses 100-115)

// create the controller and servos
PCA9685_servo_driver myController(SERVO_I2C, SERVO_SDA, SERVO_SCL, SERVO_ADDRESS);
// PCA9685_servo  myServo1 = PCA9685_servo(&myController, 0, 100, 540); // define a vector of servos, note this could extend to use multiple controller on the i2c bus
// std::vector<PCA9685_servo> myServo = {PCA9685_servo(&myController, 0, 100, 540)}; // define a vector of servos, note this could extend to use multiple controller on the i2c bus
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
uint64_t TEllapsed = 0;
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
//
dPS((const char*)"\npceCallback: Event Index: ", index);
dP("\neventid callback: index="); dP((uint16_t)index);
   
    if (index < 2) { // if the event is an ALL door event
        switch (index) { //, , , , 
          case 0:  //   OpenAll  
            for (int i = 0; i <= ConfigMemHelper_config_data.attributes.DoorCount; i++) {            
              MoveServo(i, 32);            
              // drawTrack(i,((Tracks[i].trackFront*360)/fullTurnSteps));            
            }
          break;
          case 1:  //  CloseAll
            for (int i = 0; i <= ConfigMemHelper_config_data.attributes.DoorCount; i++) {
              MoveServo(i, 0);            
              // drawTrack(i,((Tracks[i].trackFront*360)/fullTurnSteps));
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
                {              MoveServo(servoNum, 32);            }
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

void init_servo(PCA9685_servo& servo, uint8_t mode, int16_t minRange, int16_t maxRange, int16_t position, uint8_t address, uint64_t TConstDur)
{
    servo.setRange(minRange, maxRange);
    servo.setMode(mode);
    servo.setPosition(position); // move to mid point
    servo.setAddress(address);
    servo.setTConstantDuration(TConstDur);
    servo.setAngularVelocity(20);
    servo.setInvertMode(inversion);
}

void setupServos(){
  // Initialize PCA9685
  // Wire1.setSDA(SERVO_SDA);
  // Wire1.setSCL(SERVO_SCL);
	// pwm1.begin();   

  myController.begin(100000);   // create connection to the PCA9685 and initialize it

  myController.setOscillatorFrequency(27000000);
  myController.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  // initialize the servos created
  int i{0};
  for(auto& servo : myServo){
      init_servo(servo, MODE_SCONSTANT, angleMinimum, angleMaximum, Servos[i].Position, i++, 1000000);
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
    return;
}

void StopMoveHandler(uint16_t Address)
{
    // called when a servo stops moving
	return;
}

void driveServos()
/* call this from the program loop
When there is LCC traffic the code is interrupted to process the message and if there is a command to a servo that respective Status array is updated. 
The changes to the Status array will cause this code loop to move the servo as commanded */
{
  TNow = time_us_64();			// time now in microseconds
  TEllapsed = TNow - TPrevious;	// time, in microseconds, since the last loop
  TPrevious = TNow;				// store this ready for the next loop

  for(auto& servo : myServo){  // loop through the servos calling their loop function so they can do their thing
      servo.loop(TEllapsed);
  }
}

void initializeHardware() {   
  
// int Lights.pin[NumOfLights] {Light_A, Light_B};
// int LightAddr[NumOfLights] {700, 701};
Lights[0].pin = Light_A;
// Lights[0].address = 700;
Lights[1].pin = Light_B;
// Lights[1].address = 701;

// for (int Light=0;Light<NumOfLights;Light++){  // roundhouse lights, move to neoPixels
//     pinMode(Lights[Light].pin, OUTPUT);
// 	  digitalWrite(Lights[Light].pin, LOW); // turn off
//     Lights[Light].active = false;
//   }

// Configure LED and accessory output pins
  // pinMode(ledPin, OUTPUT);
  // pinMode(accPin, OUTPUT);
  
  // this resets all the neopixels to an off state
  // strip.Begin();
  // strip.Show();
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
    }
    else
    {
      digitalWrite(Lights[Light].pin, HIGH); // turn on
      Lights[Light].active = true;
    }
#ifdef DEBUG_PRINT
      // LN_STATUS lnStatus = LocoNet.reportSensor(Lights[Light].address, Lights[Light].active);
      // Serial.print(F("Tx: Sensor: "));
      // Serial.print(Lights[Light].address);
      // Serial.print(" Status: ");
      // Serial.println(LocoNet.getStatusStr(lnStatus));
#endif
}

void MoveServo(int i, int dir)
{     
  if (i > (MAX_DOORS - 1)) return;
  Serial.print(F("Activating Servo : "));
  Serial.println(i, DEC);

  Servos[i].active = true;
  Servos[i].Status = dir;
  // When MoveServo() activates a servo:
  // Call setPosition(finalTarget) ONCE, then just let loop() run.
    
  int16_t target = dir ? Servos[i].ServoMax : Servos[i].ServoMin;
  myServo[i].setPosition((int8_t)target);

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


void setServoDefaults()
// memory for the Status of servo:  
// 	0 = deviate position
//  1 = correct position
//  2 = Status on start sketch

{	
  for (int i = 0; i < (sizeof(Servos) / sizeof(ServoAddress)); i++) {
      // Servos[i].address = ServoStartingAddress + i;	// DCC address for this servo
      Servos[i].active = false; // servo in use flag
      Servos[i].Status = 2; // flag for opening or closing of servo
      Servos[i].Position = 0; // current position
    }
  
  // these are Status arrays for each servo
	Servos[0].ServoMin = -45;	// position of servo at close
	Servos[0].ServoMax = 70;	// position of servo at open

	Servos[1].ServoMin = -45;
	Servos[1].ServoMax = 70;

	Servos[2].ServoMin = -45;
	Servos[2].ServoMax = 50;

	Servos[3].ServoMin = -45;
	Servos[3].ServoMax = 50;

	Servos[4].ServoMin = -45;
	Servos[4].ServoMax = 60;
  
	Servos[5].ServoMin = -45;
	Servos[5].ServoMax = 45;
  
	Servos[6].ServoMin = -45;
	Servos[6].ServoMax = 56;
  
	Servos[7].ServoMin = -45;
	Servos[7].ServoMax = 40;
  
	Servos[8].ServoMin = -45;
	Servos[8].ServoMax = 45;
  
	Servos[9].ServoMin = -45;
	Servos[9].ServoMax = 45;

/*	repeat the above construct for any additional servos implemented 	*/
}