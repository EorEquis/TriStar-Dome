/*************************************************************************
 * TriStar Observatory ASCOM Dome Firmware
 * 2017DEC04 - EOR : EorEquis@tristarobservatory.space
 * v1.0b
 * 
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
 *  1.1.1       2018FEB09   EOR   Slow the roof down, re-order status checks to not return error on connect
 *  2.0.0       2020JUN09   EOR   Sparta Obs Version, cleanup, add pushbutton
 **************************************************************************/
 
#include <SoftwareSerial.h>

// Pololu SMC config
  const int rxPin = 3;          // pin 3 connects to SMC TX   : Green
  const int txPin = 4;          // pin 4 connects to SMC RX   : Orange
  const int resetPin = 5;       // pin 5 connects to SMC nRST : Brown
  const int errPin = 6;         // pin 6 connects to SMC ERR  : Blue

// Other Pins
  const int buttonPin = 7;      // pin 7 connects to button   : Yellow

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
          }   // end if debug
      }   // end if millis

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
            setMotorSpeed(1400);
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
            setMotorSpeed(-1400);
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
