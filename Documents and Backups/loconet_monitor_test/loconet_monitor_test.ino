#include <LocoNet.h>

// LocoNet Packet Monitor
// Demonstrates the use of the:
//
//   LocoNet.processSwitchSensorMessage(LnPacket)
//
// function and examples of each of the notifyXXXXXXX user call-back functions



// modified for processing train commands

lnMsg        *LnPacket;
LocoNetThrottleClass Throttle ;

// attempt to catch throttle command
void notifyThrottleAddress( uint8_t UserData, TH_STATE State, uint16_t Address, uint8_t Slot )    {Serial.print("Address: "); Serial.println(Address);}
void notifyThrottleSpeed( uint8_t UserData, TH_STATE State, uint8_t Speed )                       {Serial.print("Speed: "); Serial.println(Speed);}
void notifyThrottleDirection( uint8_t UserData, TH_STATE State, uint8_t Direction )               {Serial.print("Direction: "); Serial.println(Direction);}
void notifyThrottleFunction( uint8_t UserData, uint8_t Function, uint8_t Value )                  {Serial.print("Function: "); Serial.print(Function); Serial.print("\tValue: "); Serial.println(Value);}
void notifyThrottleSlotStatus( uint8_t UserData, uint8_t Status )                                 {Serial.print("Slot Status: "); Serial.println(Status);}
void notifyThrottleError( uint8_t UserData, TH_ERROR Error )                                      {Serial.print("Error: "); Serial.println(Throttle.getErrorStr(Error));}
void notifyThrottleState( uint8_t UserData, TH_STATE PrevState, TH_STATE State )                  {Serial.print("State: "); Serial.println(Throttle.getStateStr(State));}


void process() {
  // Check for any received LocoNet packets
  LnPacket = LocoNet.receive() ;
  if ( LnPacket ) {
    // First print out the packet in HEX
    Serial.print("RX: ");
    uint8_t msgLen = getLnMsgSize(LnPacket);
    for (uint8_t x = 0; x < msgLen; x++)
    {
      uint8_t val = LnPacket->data[x];
      // Print a leading 0 if less than 16 to make 2 HEX digits
      if (val < 16)
        Serial.print('0');

      Serial.print(val, HEX);
      Serial.print(' ');
    }

    // If this packet was not a Switch or Sensor Message then print a new line
    if (!LocoNet.processSwitchSensorMessage(LnPacket)) {
      Serial.println();
      // if packet is not Switch or Sensor message, attempt to process it as Throttle Message
      Throttle.processMessage(LnPacket);
    }
  }
}

void setup() {
  // First initialize the LocoNet interface
  LocoNet.init(46);
  
  // Configure the serial port for 9600 baud
  Serial.begin(9600);
  Serial.println("LocoNet Monitor");

  // initialize LocoNetThrottleClass object in similar fashion to LocoNetThrottle.ino example code
  // train initialized on address 9001
  Throttle.init(0, 0, 9001);

  Serial.println("\nStealing Address!");
  Throttle.stealAddress(9001);
  process();
  process();    // repeated to process possible errors

  if (Throttle.getState() == TH_ST_FREE) {
    Serial.println("\nSetting Address!");
    Throttle.setAddress(9001);
  }

  Serial.println("\nExiting Setup\n");
}

void loop() {
  process();
      
//      Throttle.setAddress(9999);                                  // sets train address
//      Throttle.dispatchAddress(9999);                             // release address/slot for use 
//      Throttle.stealAddress(9999);                                // copies address from another train, and dispatches the source address
//      Throttle.freeAddress(9999);                                 // does a little less than dispatch
//      Throttle.releaseAddress();                                  // idk
//      Throttle.setDirection(0); 
//      Throttle.setDirection(1); 
//      Throttle.setDirection(!Throttle.getDirection());
//      Throttle.setSpeed(Throttle.getSpeed() - 1);
//      Throttle.setSpeed(Throttle.getSpeed() + 1);
}

// unmodified functions from LocoNetMonitor.ino below

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Sensor messages
void notifySensor( uint16_t Address, uint8_t State ) {
  Serial.print("Sensor: ");
  Serial.print(Address, DEC);
  Serial.print(" - ");
  Serial.println( State ? "Active" : "Inactive" );
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch Request messages
void notifySwitchRequest( uint16_t Address, uint8_t Output, uint8_t Direction ) {
  Serial.print("Switch Request: ");
  Serial.print(Address, DEC);
  Serial.print(':');
  Serial.print(Direction ? "Closed" : "Thrown");
  Serial.print(" - ");
  Serial.println(Output ? "On" : "Off");
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch Output Report messages
void notifySwitchOutputsReport( uint16_t Address, uint8_t ClosedOutput, uint8_t ThrownOutput) {
  Serial.print("Switch Outputs Report: ");
  Serial.print(Address, DEC);
  Serial.print(": Closed - ");
  Serial.print(ClosedOutput ? "On" : "Off");
  Serial.print(": Thrown - ");
  Serial.println(ThrownOutput ? "On" : "Off");
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch Sensor Report messages
void notifySwitchReport( uint16_t Address, uint8_t State, uint8_t Sensor ) {
  Serial.print("Switch Sensor Report: ");
  Serial.print(Address, DEC);
  Serial.print(':');
  Serial.print(Sensor ? "Switch" : "Aux");
  Serial.print(" - ");
  Serial.println( State ? "Active" : "Inactive" );
}

// This call-back function is called from LocoNet.processSwitchSensorMessage
// for all Switch State messages
void notifySwitchState( uint16_t Address, uint8_t Output, uint8_t Direction ) {
  Serial.print("Switch State: ");
  Serial.print(Address, DEC);
  Serial.print(':');
  Serial.print(Direction ? "Closed" : "Thrown");
  Serial.print(" - ");
  Serial.println(Output ? "On" : "Off");
}
