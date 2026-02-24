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

extern void writeNVMdefaults();
extern void readNVM();
extern void produceLightIn();
extern void produceLightEx();
extern void produceOpenAll();
extern void produceCloseAll();
extern void produceDoor(int servo);

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


// typedef struct
// {
// 	int address;
// 	int pin;
// 	bool active;
// }
// LightAddress;
LightAddress Lights[NumOfLights];

// int LightPin[NumOfLights] {Light_A, Light_B};
// int LightAddr[NumOfLights] {700, 701};

TrackAddress Tracks[MAX_TRACKS];

ServoAddress Servos[MAX_DOORS];

// Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(SERVO_ADDRESS); // to manage servos from 1  to 16 (addresses 100-115)

// create the controller and servos
PCA9685_servo_driver myController(i2c1, SERVO_SDA, SERVO_SCL, SERVO_ADDRESS);
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
//dPS((const char*)"\npceCallback: Event Index: ", index);
// dP("\neventid callback: index="); dP((uint16_t)index);
  if (index < NUM_TABLE_EVENTS){    
        switch (index) { //
          case 0:  // CEID(Rehome)
          touchCommand(4);
          break;
          case 1:  //  CEID(IncrementTrack) 
          touchCommand(9);
          break;
          case 2:  //  CEID(DecrementTrack) 
          touchCommand(5);
          break;
          case 3:  //  CEID(RotateTrack180) 
          touchCommand(1);
          break;
          case 4:  //  CEID(ToggleBridgeLights) 
          touchCommand(2);
          break;
          default:
            // do nothing
          break;
        }
  }
  else 
  if (index < NUM_TABLE_EVENTS + NUM_TRACK_EVENTS){
    index = index - NUM_TABLE_EVENTS;
    uint8_t track = index / 2;
    uint8_t outputState = index % 2;
    if (outputState) {
      // move back side to track
      // move to track backward
    }
    else {
      // move front side to track
      // move to track foreward
    }
      
      // toggle door to track w/redraw
      if (Tracks[track].doorPresent) 
      {
        // if (Servos[Tracks[track].servoNumber].Status)
        // {              MoveServo(Tracks[track].servoNumber, 0);            }
        // else
        // {              MoveServo(Tracks[track].servoNumber, 32);            }
      }
  }
  else {    
// skip Door events as they are produced, not consumed
    index = index - NUM_DOOR_EVENTS;
    if (index < NUM_LUM_EVENTS){
        switch (index) { //, , , , 
          case 0:  //   CEID(eidBridge)
          touchCommand(2);
          break;
          case 1:  //  PEID(eidInterior)
          // produced
          break;
          case 2:  //  PEID(eidExterior)
          // produced
          break;
          case 3:  //  CEID(eidHighLuminosity_On)
          DimmerHigh();      // go to high luminosity
          break;
          case 4:  //  CEID(eidLowLuminosity_On)
          DimmerLow();       // go to low luminosity 
          break;
          default:
            // do nothing
          break;
        }
      }
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
  myController.setOscillatorFrequency(27000000);
  myController.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
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
	// Drive each servo one at a time
  int i = 0;
  for(auto& servo : myServo)
  {
		if (Servos[i].active ) 
		{	
			servo.setPosition(Servos[i].Position);
			if (Servos[i].Status) // open door
			{
				Servos[i].Position++;
				if (Servos[i].Position >= Servos[i].ServoMax) // reached end
				{
          Servos[i].Position = Servos[i].ServoMax;
					Servos[i].active = false;
          writeServo(i);
   
          #ifdef TT_DEBUG
            Serial.print(F("Servo Open: "));
            Serial.print(i, DEC);
            Serial.print(',');
            Serial.print(Servos[i].Status, DEC);
            Serial.print(',');
            Serial.println(Servos[i].Position, DEC);			
          #endif          
      
          // LN_STATUS lnStatus = LocoNet.reportSensor(Servos[i].address,1);
          // reportSensor(&LNbus,Servos[i].address,1);
 
          #ifdef TT_DEBUG        
            Serial.print(F("Tx: Sensor: "));
            Serial.println(Servos[i].address);
            // Serial.print(F(" Status: "));
            // Serial.println(LocoNet.getStatusStr(lnStatus));
          #endif          
				}				
			}
			else  // close door
			{
				Servos[i].Position--;
				if (Servos[i].Position <= Servos[i].ServoMin) // reached end
				{
          Servos[i].Position = Servos[i].ServoMin;
					Servos[i].active = false;
          writeServo(i);
  
          #ifdef TT_DEBUG
            Serial.print(F("Servo Closed: "));
            Serial.print(i, DEC);
            Serial.print(',');
            Serial.print(Servos[i].Status, DEC);
            Serial.print(',');
            Serial.println(Servos[i].Position, DEC);
          #endif          
          
          // LN_STATUS lnStatus = LocoNet.reportSensor(Servos[i].address,0);
          // reportSensor(&LNbus,Servos[i].address,0);
   
          #ifdef TT_DEBUG       
            Serial.print(F("Tx: Sensor: "));
            Serial.println(Servos[i].address);
            // Serial.print(F(" Status: "));
            // Serial.println(LocoNet.getStatusStr(lnStatus));
          #endif          
				}
			}      
		}
    i++;
	}

}

void initializeHardware() {   
  
// int Lights.pin[NumOfLights] {Light_A, Light_B};
// int LightAddr[NumOfLights] {700, 701};
Lights[0].pin = Light_A;
Lights[0].address = 700;
Lights[1].pin = Light_B;
Lights[1].address = 701;
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
{     if (i > (MAX_DOORS - 1)) return;
      Serial.print(F("Activating Servo : "));
      Serial.println(i, DEC);

      Servos[i].active = true;
      Servos[i].Status = dir;

      if (dir)
      {
        Serial.print(F("Opening : "));
        Serial.println(Servos[i].address, DEC);
      }
      else
      {
        Serial.print(F("Closing : "));
        Serial.println(Servos[i].address, DEC);
      }
}  


void touchCommand(int boxCode)
{ 
/*
0 = null spot
 PageBoxes 20        
1 = bridge
2 = bridge shack
3 - 20 = page buttons

variable selection buttons = 2 * variables, add to pageboxes to get track count variable box #define TrackSelBoxes 24    
Track setting buttons = 7 * tracks, add to TrackSelBoxes to get track box starting point #define TrackBoxes 144      
Servo selection buttons = 2 * variables, add to TrackBoxes to get servo variable update box #define ServoSelBoxes 146   
Servo setting buttons = 6 * servos, add to TrackSelBoxes to get track box starting point #define ServoBox 206       
Turntable track operation buttons = 3 * tracks, add to ServoBox to get track box starting point #define TrackBox 266      

 PossibleBoxes  270    // space for array of click box boundaries (static boxes plus track boxes)

*/
// check for debounce
// box_started_ms = millis();      
// if (box_started_ms - box_last_change < box_db_time)     return;     // Debounce time not ellapsed.

// box_changed = false;  // Debounce time has not ellapsed.
// // box_changed = (box_current_state != box_last_state); // Report state change if current state vary from last state. 

// box_last_state = box_current_state;				// Save last state.
// box_current_state = boxCode;					// Assign new state as current state . 
// if (!(box_current_state == box_last_state)) {box_changed = true;};
// if (box_changed) {           // State changed.
//   Serial.println(' ');
//   Serial.print(" box changed   ");  
//   box_last_change = read_started_ms;        // Save current millis as last change time.
// }

#ifdef TT_DEBUG
  // tft.setCursor(300, 270,  2);
  // tft.print("HotSpot is ");tft.print(HotSpotBox(X_Coord,Y_Coord));tft.print("     ");
  // tft.setCursor(300, 285,  2);
  // tft.print("X = ");tft.print(X_Coord);tft.print("   ");
  // tft.setCursor(300, 300,  2);
  // tft.print("Y = ");tft.print(Y_Coord);tft.print("   ");
#endif  
  if (boxCode <= PageBoxes) { // main screen buttons
    switch (boxCode) {
    case 1:      // bridge 
      // Turn180();
      break;
    case 2:      // bridge shack
      // TogglePixels();
      // drawShack((absPosition(stepper.currentPosition())*360)/fullTurnSteps);
      break;
    case 3:      // find reference positions
      // initiateReferences();
      break;
    case 4:    // Re-home
      Serial.println(F("Reset Home Position...."));
      // initiateHoming();
      break;
    case 5:      // decrement
      // DecrementTrack();
      break;
    case 6:      // Bump Bar
      // BumpBar();
      break;      
    case 7:      // toggle RH interior lights
      produceLightIn();
      // ToggleLight(0);
      break;
    case 8:      // toggle RH exterior lights
      produceLightEx();
      // ToggleLight(1);
      break;    
    case 9:      // button 5
      // IncrementTrack();
      break;      
    case 10:      // open all doors 
      produceOpenAll();    
      // for (int i = 0; i <= trackCount; i++) {
      // if (Tracks[i].doorPresent) 
      // {
      //   // MoveServo(Tracks[i].servoNumber, 32);            
      //   drawTrack(i,((Tracks[i].trackFront*360)/fullTurnSteps));
      // }
      // }
      break;
    case 11:      // close all doors 
      produceCloseAll();
      // for (int i = 0; i <= trackCount; i++) {
      // if (Tracks[i].doorPresent) 
      // {
      //   // MoveServo(Tracks[i].servoNumber, 0);            
      //   drawTrack(i,((Tracks[i].trackFront*360)/fullTurnSteps));
      // }
      // }
      break;    
    case 12:      // settings page
      // drawSettingsPage();
      break;
    case 13:      // diagnostics page
      // drawDiagnosticPage();
      break;    
    case 14:      // turn table page
      // drawHomePage();
      // drawTracks();
      break;
    case 15:      // clear EEPROM
      // clearEEPROM();
      // drawDiagnosticPage();
      break;    
    case 16:      // read EEPROM
      {
      // long savedSteps = readEEPROM();
      // drawDiagnosticPage();
      break;
      }
    case 17:      // write EEPROM   
    #ifdef TT_DEBUG
      Serial.println(F("DEBUG: write EEPROM "));
      Serial.println(boxCode);
    #endif   
      // writeEEPROM();
      // drawDiagnosticPage();
      break;    
    case 18: // settings page
      // drawConfigPage();
      break;    
    case 19:
      // determine step count - make button?
      // initiateStepCount();
      break;
    case 20:
      // decrement fullTurnSteps
      // --fullTurnSteps;
      // drawSteps();
      break;    
    case 21:
      // increment fullTurnSteps
      // ++fullTurnSteps;
      // drawSteps();
      break;    
    case 22:
      // read step count from storage
      // fullTurnSteps = getSteps();
      // drawSteps();
      break;    
    case 23:
      // write step count to storage
      // writeSteps(fullTurnSteps);
      // drawSteps();
      break;    
    case 24:
      // read references from storage
      // getReferences();
      // drawDiagnosticPage();
      break;    
    case 25:
      // write references to storage
      // writeReferences();
      // drawConfigPage();
      break;    
    case 26:
      //  load default tracks - updated to CDI data      
      writeNVMdefaults();
      readNVM();
      // setTrackDefaults(); // this updates just to RAM, not CDI
      // drawDiagnosticPage();
      break;    
    case 27:
      // read tracks from storage
      // getTracks();
      // drawDiagnosticPage();
      break;    
    case 28:
      // write tracks to storage
      // writeTracks();
      // drawConfigPage();
      break;    
    case 29:
      //  load default servos
      // setServoDefaults();
      // drawDiagnosticPage();
      break;    
    case 30:
      // read servos from storage
      // getServos();
      // drawDiagnosticPage();
      break;    
    case 31:
      // write servos to storage
      // writeServos();
      // drawConfigPage();
      break;    
    default:
      /*
      
      */ 
    #ifdef TT_DEBUG
      Serial.println(F("DEBUG: default switch case "));
      Serial.println(boxCode);
    #endif   
      break;
    }    
  } 
  else  // process a track variable box  
  {
    if (boxCode <= TrackSelBoxes) {
      int action = (boxCode-PageBoxes-1);
      switch (action){
      case 0:
        // decrement track count
        // if (trackCount > 0){
        //    --trackCount;
        //    writeCount();
        // }
        break;
      case 1:
        // increment track count
        // if (trackCount < NUM_TRACKS) {
        //    ++trackCount;
        //    writeCount();
        // }
        break;
      case 2:
        // decrement track edit selection
        // if (refCount > 0) --refCount;
        break;
      case 3:
        // increment track edit selection
        // if (refCount < NumberOfReferences) ++refCount;
        break;
      case 4:
        // decrement track edit selection
        // if (editTrack > 0) --editTrack;
        break;
      case 5:
        // increment track edit selection
        // if (editTrack < trackCount) ++editTrack;
        break;
      default:
        // statements
        break;
      }   
      // drawSetting(editTrack);
    }
    else  // process a track box
    {
      if (boxCode < TrackBoxes) {
        int action = (boxCode-TrackSelBoxes-1) % 7;
        switch (action){
        case 0:
          // decrement address
          // if (Tracks[editTrack].address > 0) --Tracks[editTrack].address ;
          break;
        case 1:
          // increment address
          // if (Tracks[editTrack].address < MaxDCCaddress) ++Tracks[editTrack].address;
          break;
        case 2:
          // decrement step position
          // if (Tracks[editTrack].trackFront > 0) {
          //   --Tracks[editTrack].trackFront;
          //   --Tracks[editTrack].trackBack;
          // }
          break;
        case 3:
          // increment step position
          // if (Tracks[editTrack].trackFront < fullTurnSteps) {
          //   ++Tracks[editTrack].trackFront;
          //   ++Tracks[editTrack].trackBack;
          // }
          break;
        case 4:
          // toggle door presence to track w/redraw
          // if (Tracks[editTrack].doorPresent)
          // {Tracks[editTrack].doorPresent = false;}
          // else
          // {Tracks[editTrack].doorPresent = true;}
          break;
        case 5:
          // decrement servo number
          // if (Tracks[editTrack].servoNumber > 0) --Tracks[editTrack].servoNumber ;
          break;
        case 6:
          // increment servo number
          // if (Tracks[editTrack].servoNumber < MAX_DOORS) ++Tracks[editTrack].servoNumber;
          break;
        default:
          // statements
          break;
        }
        // drawSetting(editTrack);
      }
      else  // process a servo variable box  
      {
        if (boxCode <= ServoSelBoxes) {
          int action = (boxCode-TrackBoxes-1);
          switch (action){
          case 0:
            // decrement track count
            // if (editServo > 0) --editServo;
            break;
          case 1:
            // increment track count
            // if (editServo < MAX_DOORS) ++editServo;
            break;
          default:
            // statements
            break;
          }      
          // drawServo(editServo);
        }
        else {  // buttons on servo settings page ServoSelBoxes
          if (boxCode < TrackBox) {
            int action = (boxCode-ServoSelBoxes-1) % 6;
            switch (action){
          case 0:
            // decrement address
            // if (Servos[editServo].address > 0) --Servos[editServo].address ;
            break;
          case 1:
            // increment address
            // if (Servos[editServo].address < MaxDCCaddress) ++Servos[editServo].address;
            break;
          case 2:
            // decrement servo minimum range
            // if (Servos[editServo].ServoMin > MinServoRange) --Servos[editServo].ServoMin ;
            break;
          case 3:
            // increment servo minimum range
            // if (Servos[editServo].ServoMin < MaxServoRange) ++Servos[editServo].ServoMin;
            break;
          case 4:
            // decrement servo minimum range
            // if (Servos[editServo].ServoMax > MinServoRange) --Servos[editServo].ServoMax ;
            break;
          case 5:
            // increment servo minimum range
            // if (Servos[editServo].ServoMax < MaxServoRange) ++Servos[editServo].ServoMax;
            break;
            default:
              // statements
              break;
            }
            // drawServo(editServo);
          }
          else { // track buttons from the turntable diagram
            int track = (boxCode - TrackBox)/3;
            int action = (boxCode-TrackBox) % 3;
            switch (action){
            case 0:
              // move front side to track
              // move to track foreward
              // MoveToTrack(track,32);
              break;
            case 1:
              // move back side to track
              // move to track backward
              // MoveToTrack(track,0);
              break;
            case 2:
              // toggle door to track w/redraw
              if (Tracks[track].doorPresent) 
              {
                produceDoor(Tracks[track].servoNumber);
                // if (Servos[Tracks[track].servoNumber].Status)
                // {              MoveServo(Tracks[track].servoNumber, 0);            }
                // else
                // {              MoveServo(Tracks[track].servoNumber, 32);            }
                // drawTrack(track,((Tracks[track].trackFront*360)/fullTurnSteps));
              }
              break;
            default:
              // statements
              break;
            }
          }
        }
      }
    }
  }
  
#ifdef TT_DEBUG  
    // Serial.print(F("Buffer: "));
    // Serial.println(buff);
#endif          
}

void setTrackDefaults()
{
  // for (int i = 0; i < (sizeof(Tracks) / sizeof(TrackAddress)); i++) {
  //     // Tracks[i].address = TrackStartAddress - 1 + i;
  //     Tracks[i].trackFront = 0;
  //     // Tracks[i].trackBack = (FULL_TURN_STEPS / 2);
  //     Tracks[i].doorPresent = false;
  //     Tracks[i].servoNumber = 0;
  //   }
  // for (int i = 4; i < (sizeof(Tracks) / sizeof(TrackAddress)-1); i++) {
  //     Tracks[i].doorPresent = true;
  //     // Tracks[i].servoNumber = (i-4) % MAX_DOORS;
  //   }
  // trackCount = NUM_TRACKS;
  
  // // homeTrack = 3;
	// // track zero is the position of the homing sensor
	// // Tracks[1].address = 500; // TrackStartAddress
	// Tracks[1].trackFront = absPosition(entryTrack1);
	// Tracks[1].trackBack = absPosition(entryTrack1 + (FULL_TURN_STEPS / 2));

	// // Tracks[2].address = 501;
	// Tracks[2].trackFront = absPosition(entryTrack2);
	// Tracks[2].trackBack = absPosition(entryTrack2 + (FULL_TURN_STEPS / 2));

	// // Tracks[3].address = 502;
	// Tracks[3].trackFront = absPosition(entryTrack3);
	// Tracks[3].trackBack = absPosition(entryTrack3 + (FULL_TURN_STEPS / 2));

	// // Tracks[4].address = 503;
	// Tracks[4].trackFront = absPosition(houseTrack1);
	// Tracks[4].trackBack = absPosition(houseTrack1 + (FULL_TURN_STEPS / 2));

	// // Tracks[5].address = 504;
	// Tracks[5].trackFront = absPosition(houseTrack2);
	// Tracks[5].trackBack = absPosition(houseTrack2 + (FULL_TURN_STEPS / 2));

	// // Tracks[6].address = 505;
	// Tracks[6].trackFront = absPosition(houseTrack3);
	// Tracks[6].trackBack = absPosition(houseTrack3 + (FULL_TURN_STEPS / 2));

	// // Tracks[7].address = 506;
	// Tracks[7].trackFront = absPosition(houseTrack4);
	// Tracks[7].trackBack = absPosition(houseTrack4 + (FULL_TURN_STEPS / 2));

	// // Tracks[8].address = 507;
	// Tracks[8].trackFront = absPosition(houseTrack5);
	// Tracks[8].trackBack = absPosition(houseTrack5 + (FULL_TURN_STEPS / 2));

	// // Tracks[9].address = 508;
	// Tracks[9].trackFront = absPosition(houseTrack6);
	// Tracks[9].trackBack = absPosition(houseTrack6 + (FULL_TURN_STEPS / 2));

	// // Tracks[10].address = 509;
	// Tracks[10].trackFront = absPosition(houseTrack7);
	// Tracks[10].trackBack = absPosition(houseTrack7 + (FULL_TURN_STEPS / 2));

	// // Tracks[11].address = 510;
	// Tracks[11].trackFront = absPosition(houseTrack8);
	// Tracks[11].trackBack = absPosition(houseTrack8 + (FULL_TURN_STEPS / 2));

	// // Tracks[12].address = 511;
	// Tracks[12].trackFront = absPosition(houseTrack9);
	// Tracks[12].trackBack = absPosition(houseTrack9 + (FULL_TURN_STEPS / 2));

	// // Tracks[13].address = 512;
	// Tracks[13].trackFront = absPosition(houseTrack10);
	// Tracks[13].trackBack = absPosition(houseTrack10 + (FULL_TURN_STEPS / 2));

	// // Tracks[14].address = 513;
	// Tracks[14].trackFront = absPosition(houseTrack11);
	// Tracks[14].trackBack = absPosition(houseTrack11 + (FULL_TURN_STEPS / 2));
}

void setServoDefaults()
// memory for the Status of servo:  
// 	0 = deviate position
//  1 = correct position
//  2 = Status on start sketch

{	
  for (int i = 0; i < (sizeof(Servos) / sizeof(ServoAddress)); i++) {
      Servos[i].address = ServoStartingAddress + i;	// DCC address for this servo
      Servos[i].active = false; // servo in use flag
      Servos[i].Status = 2; // flag for opening or closing of servo
      Servos[i].Position = 45; // current position
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