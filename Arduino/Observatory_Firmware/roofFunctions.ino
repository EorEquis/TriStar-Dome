int openRoof()
  {
    if (shutterState == shutterClosed)
      {
        setMotorSpeed(motorSpeed);
        return shutterOpening;
      }
      else
      {
        return shutterError;
      }
  }

int closeRoof()
  {
    if (shutterState == shutterOpen)
      {
        setMotorSpeed(-1 * motorSpeed);
        return shutterClosing;
      }
    else
      {
        return shutterError;
      }     
  }

void abortRoof()
  {
    setMotorLimit(DECELERATION, 0);
    setMotorSpeed(0);
    setMotorLimit(DECELERATION, 3);
  }
