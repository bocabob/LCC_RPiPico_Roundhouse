// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
    #define N(x) xN(x)     // allow the insertion of the value (x) ..
    #define xN(x) #x       // .. into the CDI string. 
const char configDefInfo[] PROGMEM =
// ===== Enter User definitions below =====
CDIheader R"(
  <name>Parameters</name>
  <description>Setup for Turntable control.</description> <!-- bypasses magic, nextEID, nodeID -->
  <group>
	  <name>Track</name>
    <description>Parameters for turntable trackage</description> <!-- bypasses magic, nextEID, nodeID -->
      <int size='1'>
        <name>Track Count</name>
          <description>Nunmber of tracks to the turntable</description>
        <min>1</min><max>)" N(MAX_TRACKS) R"(</max>
          <default>14</default>
      </int>
      <int size='1'>
        <name>Home Track</name>
          <description>Track to go to after homing</description>
        <min>1</min><max>)" N(MAX_TRACKS) R"(</max>
          <default>3</default>
      </int>
      <int size='1'>
        <name>Enable Reference Correction</name>
        <default>0</default>
        <map>
          <relation><property>0</property><value>Disabled</value></relation>
          <relation><property>1</property><value>Enabled</value></relation>
        </map>
      </int>
    <eventid><name>Rehome</name></eventid>
    <eventid><name>Increment Track</name></eventid>
    <eventid><name>Decrement Track</name></eventid>
    <eventid><name>Rotate Track 180 degrees</name></eventid>
    <eventid><name>Toggle Bridge Lights</name></eventid>
    <group replication=')" N(MAX_TRACKS) R"('> <!-- stuff magic to trigger resets -->
      <name>Tracks</name>
      <repname>T:0</repname>
      <description>Track</description>
      <string size='25'><name>Description</name></string>
      <string size='5'><name>Short Name</name></string>
      <eventid><name>Align Front</name></eventid>
      <eventid><name>Align Back</name></eventid>
      <int size='4'>
        <name>Stepper Position in Steps</name>
        <min>0</min><max>96000</max><default>10</default>
      </int>
    </group>
  </group>
  <group>
	  <name>Doors</name>
    <description>Parameters for Door control</description> <!-- bypasses magic, nextEID, nodeID -->
          <int size='1'>
            <name>Active Doors</name><description>Number of active doors</description>
            <min>0</min><max>)" N(NUM_DOORS) R"(</max><default>N(NUM_DOORS)</default>
          </int>
    <eventid><name>Open All Doors</name></eventid>
    <eventid><name>Close All Doors</name></eventid>
    <group replication=')" N(MAX_DOORS) R"('> <!-- stuff magic to trigger resets -->
      <name>Door</name>
      <repname>D:1</repname>
      <description>Door</description>
      <string size='16'><name>Description</name></string>
      <string size='5'><name>Short Name</name></string>   
      <eventid><name>Toggle Door</name></eventid>
      <int size='1'>
        <name>Track Number</name>
          <description>The track number associated with the door</description>
          <min>0</min><max>)" N(MAX_TRACKS) R"(</max>
          <default>0</default>
      </int>
    </group>
  </group>
  <group>
	  <name>Lights</name>
    <description>Parameters for Light control</description> <!-- bypasses magic, nextEID, nodeID -->
    <eventid><name>Toggle Bridge Lights</name></eventid>
    <eventid><name>Toggle Interior Lights</name></eventid>
    <eventid><name>Toggle Exterior Lights</name></eventid>
    <int size='1'>
      <name>Maximum Luminosity</name>
        <description>Full luminesence factor</description>
        <min>10</min><max>255</max><default>150</default>
      <hints><slider tickSpacing='60' immediate='no' showValue='yes'> </slider></hints>
    </int>
      <eventid><name>Full Luminosity On</name></eventid>
    <int size='1'>
      <name>Minimum Luminosity</name>
        <description>Dimmed luminesence factor</description>
        <min>10</min><max>255</max><default>50</default>
      <hints><slider tickSpacing='60' immediate='no' showValue='yes'> </slider></hints>
    </int>
      <eventid><name>Low Luminosity On</name></eventid>
  </group>
  )" CDIfooter;
// ===== Enter User definitions above =====
} // end extern

// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
#pragma pack(push, 1) // Start packing with 1-byte alignment
typedef struct { 
// ==== These are required by the included CDI header
    EVENT_SPACE_HEADER eventSpaceHeader;    // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!
    
    char nodeName[20];  // optional node-name, used by ACDI
    char nodeDesc[24];  // optional node-description, used by ACDI
// ===== Enter User definitions below to match the above CDI =====
// Track parameters
    uint8_t TrackCount;    // int8 Number of Tracks off turntable
    uint8_t HomeTrack;    // int8 initial track location after homing
    bool EnableReference;   // enable reference correction
    EventID Rehome;
    EventID IncrementTrack;
    EventID DecrementTrack;
    EventID RotateTrack180 ;
    EventID ToggleBridgeLights;
    struct {
      char trackName[25];        // description of this Track
      char trackShort[5];        // short description of this Track
      EventID Front;       // consumer eventID
      EventID Back;       // consumer eventID
      int32_t steps;       // position
    } tracks[MAX_TRACKS];
// Door parameters
    uint8_t DoorCount;    // int8 Number of Doors off turntable tracks
    EventID OpenAll;
    EventID CloseAll;
    struct {
      char doorName[16];        // description of this Door
      char doorShort[5];        // short description of this Door
      EventID eidToggle;       // consumer Toggle door position eventID
      uint8_t TrackLocation;    // int8 number of the track where the door is located
    } doors[MAX_DOORS];
// Lights parameters
    EventID eidBridge;       // consumer Toggle Bridge Lights eventID
    EventID eidInterior;       // consumer Toggel Interior Lights eventID
    EventID eidExterior;       // consumer Toggel Exterior Lights eventID
    uint8_t HighLuminosity;    // int8 factor on brightness when dimmer is off
    EventID eidHighLuminosity_On;       // consumer turn Group 0 ON eventID
    uint8_t LowLuminosity;    // int8 factor on brightness when dimmer is on
    EventID eidLowLuminosity_On;       // consumer turn Group 0 ON eventID
// data not used by CDI
  // ===== Enter User definitions above =====
  
  uint8_t MemVersion;
  // uint8_t curpos[NUM_SERVOS];
} MemStruct;                 // type definition
#pragma pack(pop) // Restore default alignment

    
#include "debugging.h"
#include "TTvariables.h"


void write16(uint32_t addr, uint16_t x) {
    NODECONFIG.write(addr+1, x);
    NODECONFIG.write(addr, x>>8);
  }
   
void write32(uint32_t addr, uint32_t x) {
    NODECONFIG.write(addr+3, x);
    NODECONFIG.write(addr+2, x>>8);
    NODECONFIG.write(addr+1, x>>16);
    NODECONFIG.write(addr, x>>24);
  }

void writeNVMdefaults()
{
  dP("\n  Writing CDI default values ");

  NODECONFIG.write(EEADDR(MemVersion), EEPROM_VERSION); 
  NODECONFIG.put(EEADDR(nodeName), ESTRING("PicoNode"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Roundhouse"));

  NODECONFIG.write(EEADDR(TrackCount), NUM_TRACKS);    // int8 number of tracks
  
  NODECONFIG.write(EEADDR(HomeTrack), 3);    // int8 track to go to after homeing
	// track zero is the position of the homing sensor

  NODECONFIG.write(EEADDR(EnableReference), false);    // int8 bool flag to process reference corrections

  for(uint8_t i = 0; i < MAX_TRACKS; i++) {
    NODECONFIG.put(EEADDR(tracks[i].trackName), ESTRING(TrackName[i]));
    NODECONFIG.put(EEADDR(tracks[i].trackShort), ESTRING(TrackTag[i]));
    NODECONFIG.write32(EEADDR(tracks[i].steps), 0);
  }
  
	// NODECONFIG.write32(EEADDR(tracks[1].steps), absPosition(entryTrack1));
	// NODECONFIG.write32(EEADDR(tracks[2].steps), absPosition(entryTrack2));
	// NODECONFIG.write32(EEADDR(tracks[3].steps), absPosition(entryTrack3));
	// NODECONFIG.write32(EEADDR(tracks[4].steps), absPosition(houseTrack1));
	// NODECONFIG.write32(EEADDR(tracks[5].steps), absPosition(houseTrack2));
	// NODECONFIG.write32(EEADDR(tracks[6].steps), absPosition(houseTrack3));
	// NODECONFIG.write32(EEADDR(tracks[7].steps), absPosition(houseTrack4));
	// NODECONFIG.write32(EEADDR(tracks[8].steps), absPosition(houseTrack5));
	// NODECONFIG.write32(EEADDR(tracks[9].steps), absPosition(houseTrack6));
	// NODECONFIG.write32(EEADDR(tracks[10].steps), absPosition(houseTrack7));
	// NODECONFIG.write32(EEADDR(tracks[11].steps), absPosition(houseTrack8));
	// NODECONFIG.write32(EEADDR(tracks[12].steps), absPosition(houseTrack9));
	// NODECONFIG.write32(EEADDR(tracks[13].steps), absPosition(houseTrack10));
	// NODECONFIG.write32(EEADDR(tracks[14].steps), absPosition(houseTrack11));

// Door parameters
    
  NODECONFIG.write(EEADDR(DoorCount), NUM_DOORS);    // int8 number of doors

  for(uint8_t i = 0; i < NUM_DOORS; i++) {
    NODECONFIG.put(EEADDR(doors[i].doorName), ESTRING("Door"));
    NODECONFIG.put(EEADDR(doors[i].doorShort), ESTRING("D"));
    NODECONFIG.write(EEADDR(doors[i].TrackLocation), 5 + i);
  }

// Lights parameters

  NODECONFIG.write(EEADDR(HighLuminosity), MAX_LUMINANCE);    // int8 factor on brightness when dimmer is off
  NODECONFIG.write(EEADDR(LowLuminosity), DIM_LUMINANCE);    // int8 factor on brightness when dimmer is on

}

// #include "NPlights.h"

// extern uint8_t _TrackCount;    // int8 number of strings
uint8_t _DoorCount;    // int8 number of strings
bool _EnableReference;   // flag to process reference corrections
uint8_t _HighLuminosity;    // int8 factor on brightness when dimmer is off
uint8_t _LowLuminosity;    // int8 factor on brightness when dimmer is on

// tracks _Tracks[MAX_TRACKS];
doors _Doors[MAX_DOORS];
// typedef struct
// {
// 	int address;
// 	long trackFront;
// 	long trackBack;
//   bool doorPresent;
//   int servoNumber;
// }
// TrackAddress;

void readNVM()
{
  dP("\n  Reading CDI values ");
  uint8_t j = 0;
  // trackCount = NODECONFIG.read(EEADDR(TrackCount));    // int8 number of tracks
  // homeTrack = NODECONFIG.read(EEADDR(HomeTrack));    // int8 track to go to after homing
  _EnableReference = NODECONFIG.read(EEADDR(EnableReference));    // flag to process reference corrections
  _DoorCount = NODECONFIG.read(EEADDR(DoorCount));    // int8 number of doors
 _HighLuminosity = NODECONFIG.read(EEADDR(HighLuminosity));    // int8 factor on brightness when dimmer is off
 _LowLuminosity = NODECONFIG.read(EEADDR(LowLuminosity));    // int8 factor on brightness when dimmer is on

  // dPS("\n  Track Count: ", (uint8_t)trackCount); dP("\n");

  for(uint8_t i = 0; i < MAX_TRACKS; i++) {
    Tracks[i].address = i;
    // NODECONFIG.get(EEADDR(tracks[i].trackName),_Tracks[i].trackName);
    // NODECONFIG.get(EEADDR(tracks[i].trackShort),_Tracks[i].trackShort);
    Tracks[i].trackFront = NODECONFIG.read32(EEADDR(tracks[i].steps));
	  // Tracks[i].trackBack = absPosition(Tracks[i].trackFront + (FULL_TURN_STEPS / 2));
  }

  for(uint8_t i = 0; i < MAX_DOORS; i++) {
    NODECONFIG.get(EEADDR(doors[i].doorName),_Doors[i].doorName);
    NODECONFIG.get(EEADDR(doors[i].doorShort),_Doors[i].doorShort);
    j = NODECONFIG.read(EEADDR(doors[i].TrackLocation));
    if ((j > 0) && (j <= MAX_TRACKS)) 
    {
      Tracks[j-1].servoNumber = i;
      Tracks[j-1].doorPresent = true;
    }
    else
    {
      Tracks[j-1].servoNumber = 0;
      Tracks[j-1].doorPresent = false;
    }
    _Doors[i].TrackLocation = j;
  }

  // home track read
}


void userInitAll()  // this sets Program Default values and must be run once.
{  
  uint8_t ReadVersion, i;
  i = EEPROM_VERSION;
  
  ReadVersion = NODECONFIG.read(EEADDR(MemVersion));    // int8 of version of MemStruct in NVM

  dP("\n  Initial read of NVM version "); dP(ReadVersion); dP("  Program NVM version "); dP(EEPROM_VERSION); dP("\n");

  if (ReadVersion != i) {
  //dP((ReadVersion == EEPROM_VERSION)); 
// return;
  writeNVMdefaults();
  readNVM();
  }
}
