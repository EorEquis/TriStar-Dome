int openRoof()
  {
    if (shutterState == SHUTTERCLOSED)
      {
        #ifdef LOGGING
          writeToLog("openRoof()");
        #endif
        setMotorSpeed(motorSpeed);
        shutterState = SHUTTEROPENING;
        return 0;               // Success, opening roof
        //return SHUTTEROPENING;
      }
      else
      {
        return 1;               // Error, roof is not closed, cannot be opened.
        //return SHUTTERERROR;
      }
  }

int closeRoof()
  {
    if (shutterState == SHUTTEROPEN)
      {
        #ifdef LOGGING
          writeToLog("closeRoof()");
        #endif
        setMotorSpeed(-1 * motorSpeed);
        shutterState = SHUTTERCLOSING;
        return 0;               // Success, closing roof.
        //return SHUTTERCLOSING;
      }
    else
      {
        return 1;               // Error, roof is not open, cannot be closed.
        //return SHUTTERERROR;
      }     
  }

void abortRoof()
  {
    // setMotorLimit(DECELERATION, 0);
    setMotorSpeed(0);
    // setMotorLimit(DECELERATION, 3);
    #ifdef LOGGING
      writeToLog("abortRoof()");
    #endif    
  }
