/*************************************************************************
 * TriStar Observatory ASCOM Dome Firmware
 * 2017DEC04 - EOR : EorEquis@tristarobservatory.space
 * v1.0b
 * 
 * Not much to do here.  The ROR is treated as an ASCOM "shutter" in a Dome driver.
 * We pretty much just need to open and close the thing.
 * 
 *  Version     Date        By    Comment
 *  0.1.0       2017DEC04   EOR   Initial build
 *  0.2.0       2017DEC05   EOR   Expand shutterState handling, code getInfo and resetSMC as functions
 *  1.0.0       2017DEC05   EOR   Initial working build, tested in VS and SGP
 *  1.0.1       2017DEC06   EOR   Clean up some unnecessary code to set motor limits that can be written to SMC's NVRAM
 *  1.1.0       2017DEC07   EOR   Remove LCD stuff.  It's dumb.
 **************************************************************************/
 
#include <SoftwareSerial.h>

// Pololu SMC config
const int rxPin = 3;          // pin 3 connects to SMC TX   : Green
const int txPin = 4;          // pin 4 connects to SMC RX   : Orange
const int resetPin = 5;       // pin 5 connects to SMC nRST : Brown
const int errPin = 6;         // pin 6 connects to SMC ERR  : Blue

// SMC Variable IDs
#define ERROR_STATUS 0
#define LIMIT_STATUS 3
#define TARGET_SPEED 20
#define SPEED 21


// SMC motor limit IDs

#define DECELERATION 2

// Flags and counters
bool debugFlag = 0;
unsigned long lastMillis = 0;
unsigned long currentMillis = 0;

// ASCOM ShutterState Enumeration : http://www.ascom-standards.org/Help/Developer/html/T_ASCOM_DeviceInterface_ShutterState.htm
#define shutterOpen 0
#define shutterClosed 1
#define shutterOpening 2
#define shutterClosing 3
#define shutterError 4

// Other variables
String strCmd;
unsigned int limitStatus = 0;
unsigned int errorStatus = 0;
int currentSpeed = 0;
int targetSpeed = 0;
int shutterState = 1;

// SoftwareSerial for communication w/ SMC
SoftwareSerial smcSerial = SoftwareSerial(rxPin, txPin);


//***** Main Setup() and Loop() functions *****//

void setup() {
  // Serial lines
  Serial.begin(9600);       // For serial monitor troubleshooting
  smcSerial.begin(19200);   // Begin serial to SMC

  // Reset SMC when Arduino starts up
  resetSMC();
  
  // Set the startup values
  shutterState=getInfo();
  
  // Begin tracking elapsed time for getInfo()
  currentMillis=millis();
  lastMillis=currentMillis;  
}   // end setup()

void loop() {

  // Refresh info packet every 1s
  currentMillis=millis();
  if (currentMillis - lastMillis > 500)
  {
    lastMillis=currentMillis;
    shutterState=getInfo();  
    if (debugFlag)
      {
        Serial.print("getInfo() called, shutterState is ");
        Serial.println(shutterState);
        Serial.print("limitStatus : ");
        Serial.println(limitStatus);
        Serial.print("errorStatus : ");
        Serial.println(errorStatus);
        Serial.print("currentSpeed : ");
        Serial.println(currentSpeed);
      }
  }

  while (Serial.available() > 0)
  {
    strCmd = Serial.readStringUntil('#');
    /*******************************************************
     * Supported commands
     * abrt     :       Abort any/all current movement (Stop the roof NOW)
     * clos     :       Close Roof (Shutter)
     * open     :       Open Roof (Shutter)
     * info     :       Request from ASCOM for ShutterStatus
     * reset    :       Reset the SMC.  This is not an ASCOM method, but included for troubleshooting/testing
    *******************************************************/
    if (strCmd == "abrt")
    {
      setMotorLimit(DECELERATION, 0);
      setMotorSpeed(0);
      setMotorLimit(DECELERATION, 3);
    }

    else if (strCmd == "open")
    {
      if (shutterState == shutterClosed)
      {
        //exitSafeStart();
        setMotorSpeed(1600);
        Serial.print(shutterOpening);
        Serial.println("#");
      }
      else
      {
        Serial.print(shutterError);
        Serial.println("#");
      }
    }

    else if (strCmd == "clos")
    {
      if (shutterState == shutterOpen)
      {
        //exitSafeStart();
        setMotorSpeed(-1600);
        Serial.print(shutterClosing);
        Serial.println("#");
      }
      else
      {
        Serial.print(shutterError);
        Serial.println("#");
      }      
    }

    else if (strCmd == "info")
    {
      if (debugFlag)
      {
        Serial.print("limitStatus : ");
        Serial.println(limitStatus);
        Serial.print("errorStatus : ");
        Serial.println(errorStatus);
        Serial.print("currentSpeed : ");
        Serial.println(currentSpeed);
      }
      Serial.print(shutterState);
      Serial.println("#");
    }

    else if (strCmd == "reset")
    {
      resetSMC();
      //exitSafeStart();
    }
  }
} // End loop()

//***** Program Functions *****//

int getInfo()  //TODO Write This
{
  limitStatus = getSMCVariable(LIMIT_STATUS);
  errorStatus = getSMCVariable(ERROR_STATUS);
  currentSpeed = getSMCVariable(SPEED);
  targetSpeed = getSMCVariable(TARGET_SPEED);

  // See https://www.pololu.com/docs/0J44/6.4 for Limit and Error Status flag registers

  /*
  if (currentSpeed == 0 && (limitStatus == 129 || limitStatus == 257))
  {
    //exitSafeStart();
    limitStatus = getSMCVariable(LIMIT_STATUS);
    errorStatus = getSMCVariable(ERROR_STATUS);
  }
  */
  if (errorStatus != 0)
  {
    return shutterError;
  }
  else if (currentSpeed == 0 && limitStatus == 128)
  {
    return shutterOpen;
  }

  else if (currentSpeed == 0 && limitStatus == 256)
  {
      return shutterClosed;
  }

  else if (currentSpeed > 0 && (limitStatus == 0 || limitStatus == 144))
  {
    return shutterOpening;
  }
  
  else if ((currentSpeed < 0 && (limitStatus == 0) || limitStatus == 272))
  {
    return shutterClosing;
  }

} //end getInfo()

// read an SMC serial byte
int readSMCByte()
{
  char c;
  if(smcSerial.readBytes(&c, 1) == 0){ return -1; }
  return (byte)c;
}

unsigned char setMotorLimit(unsigned char  limitID, unsigned int limitValue)
{
  smcSerial.write(0xA2);
  smcSerial.write(limitID);
  smcSerial.write(limitValue & 0x7F);
  smcSerial.write(limitValue >> 7);
  return readSMCByte();
}

// speed should be a number from -3200 to 3200
// TODO : I'll be using a set speed, so this can probbaly go away to save mem.
void setMotorSpeed(int speed)
{
  if (speed < 0)
  {
    smcSerial.write(0x86);  // motor reverse command
    speed = -speed;  // make speed positive
  }
  else
  {
    smcSerial.write(0x85);  // motor forward command
  }
  smcSerial.write(speed & 0x1F);
  smcSerial.write(speed >> 5);
}

// returns the specified variable as an unsigned integer.
// if the requested variable is signed, the value returned by this function
// should be typecast as an int.
int getSMCVariable(unsigned char variableID)
{
  smcSerial.write(0xA1);
  smcSerial.write(variableID);
  return readSMCByte() + 256 * readSMCByte();
}

void resetSMC()
{
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);  // reset SMC
  delay(1);  // wait 1 ms
  pinMode(resetPin, INPUT);  // let SMC run again

  // must wait at least 1 ms after reset before transmitting
  delay(10);

  // Set up motor controller.
  smcSerial.write(0xAA);  // send baud-indicator byte
}

