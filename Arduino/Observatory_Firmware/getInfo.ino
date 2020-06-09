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
  if (currentSpeed == 0 && limitStatus == 128)
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

  else if (currentSpeed == 0 && limitStatus == 0 && buttonPressed)
    // Roof is not moving, neither limit switch is active, and the button was pressed.  We have intentionally halted the roof "in between"
    {
      return shutterOpen;
    }

  else if (currentSpeed == 0 && limitStatus == 0 && buttonPressed == false)
    // Roof is not moving, neither limit switch is active, and the button was not pressed.  Oh shit.
    {
      return shutterError;
    }
    
  else if (errorStatus != 0)
    {
      return shutterError;
    }
} //end getInfo()
