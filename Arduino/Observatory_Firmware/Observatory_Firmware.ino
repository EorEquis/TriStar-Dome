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
 *  2.1.0       2020NOV02   EOR   Modify for new SMC G2, final testing before obs install
 **************************************************************************/
 
#include <SoftwareSerial.h>
#include "Globals.h" 

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

  // Set up pins
    pinMode(buttonPin, INPUT);

}   // end setup()

void loop() {

  // Refresh info packet at infoDelay
    currentMillis=millis();
    if (currentMillis - lastMillis > infoDelay)
      {
        lastMillis=currentMillis;
        shutterState=getInfo();
        buttonState = digitalRead(buttonPin);
        if (buttonState==HIGH)
          {
            if (currentMillis - lastButton > buttonDelay)   // debounce button for buttonDelay
              {
                lastButton=millis();
                buttonPressed = true;
                handleButton();                
              }
          }
        #ifdef DEBUG
            Serial.print("getInfo() called, shutterState is ");
            Serial.println(shutterState);
            Serial.print("limitStatus : ");
            Serial.println(limitStatus);
            Serial.print("errorStatus : ");
            Serial.println(errorStatus);
            Serial.print("currentSpeed : ");
            Serial.println(currentSpeed);
            Serial.print("targetSpeed : ");
            Serial.println(targetSpeed);            
        #endif 
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
          abortRoof();
        }   
  
      else if (strCmd == "open")
        {
          if (openRoof() == 0)
            {
              shutterState = SHUTTEROPENING;
              lastMillis=currentMillis;
            }
          // shutterState = SHUTTEROPENING;
          // Serial.print(openRoof());
          // Serial.println("#");
        }

      else if (strCmd == "clos")
        {
          if (closeRoof() == 0)
            {
              shutterState = SHUTTERCLOSING;
              lastMillis=currentMillis;
            }
          
          // Serial.print(closeRoof());
          // Serial.println("#");     
        }

    else if (strCmd == "info")
      {
        #ifdef DEBUG
            Serial.print("limitStatus : ");
            Serial.println(limitStatus);
            Serial.print("errorStatus : ");
            Serial.println(errorStatus);
            Serial.print("currentSpeed : ");
            Serial.println(currentSpeed);
        #endif 
        Serial.print(shutterState);
        Serial.println("#");
      }

    else if (strCmd == "reset")
      {
        resetSMC();
      }
  }
} // End loop()
