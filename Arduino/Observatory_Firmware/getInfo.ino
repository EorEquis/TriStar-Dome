int getInfo()  
{
  limitStatus = getSMCVariable(LIMIT_STATUS);
  errorStatus = getSMCVariable(ERROR_STATUS);
  currentSpeed = getSMCVariable(SPEED);
  targetSpeed = getSMCVariable(TARGET_SPEED);

  if (currentSpeed == 0 && currentSpeed == targetSpeed)
    {
      motorState=STOPPED;
    }

  if (currentSpeed != 0 && currentSpeed == targetSpeed)
    {
      motorState=MOVING;
    }

  if (targetSpeed != 0 && abs(currentSpeed) < abs(targetSpeed))
    {
      motorState=ACCELERATING;
    }

  if (targetSpeed != 0 && abs(currentSpeed) > abs(targetSpeed))
    {
      motorState=DECELERATING;
    }
    
  // See https://www.pololu.com/docs/0J77/6.4 for Limit and Error Status flag registers

  if (errorStatus != 0)
    {
      return SHUTTERERROR;
    }
    
  else if (bitRead(limitStatus, OPENLIMIT) == 1 && bitRead(limitStatus, OPENLIMIT) == 1)
    {
      return SHUTTERERROR;
    }
    
  else if (motorState == STOPPED && bitRead(limitStatus, OPENLIMIT) == 1)
    {
      return SHUTTEROPEN;
    }

  else if (motorState == STOPPED && bitRead(limitStatus, CLOSEDLIMIT) == 1)
    {
        return SHUTTERCLOSED;
    }

  else if (targetSpeed * DIRECTIONOPEN > 0 || currentSpeed * DIRECTIONOPEN > 0)
    {
      return SHUTTEROPENING;
    }
  
  else if (targetSpeed * DIRECTIONOPEN < 0 || currentSpeed * DIRECTIONOPEN < 0)
    {
      return SHUTTERCLOSING;
    }

  else if ((motorState == DECELERATING || motorState == STOPPED) && (bitRead(limitStatus, OPENLIMIT) == 0 && bitRead(limitStatus, CLOSEDLIMIT) == 0) && buttonPressed)
    // Roof is not moving or is decelerating, neither limit switch is active, and the button was pressed.  We have intentionally halted the roof "in between"
    {
      return SHUTTEROPEN;
    }

  else if ((motorState == DECELERATING || motorState == STOPPED) && (bitRead(limitStatus, OPENLIMIT) == 0 && bitRead(limitStatus, CLOSEDLIMIT) == 0) && buttonPressed == false)
    // Roof is not moving or is decelerating, neither limit switch is active, and the button was not pressed.  Oh shit.
    {
      return SHUTTERERROR;
    }

} //end getInfo()
