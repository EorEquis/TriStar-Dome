int handleButton()
  {
    if (shutterState == shutterOpening || shutterState == shutterClosing)
      {
        abortRoof();
      }
    else if (shutterState == shutterClosed)
      {
        buttonPressed = false;
        Serial.print(openRoof());
        Serial.println("#");    
      }
    else if (shutterState == shutterOpen)
      {
        buttonPressed = false;
        Serial.print(closeRoof());
        Serial.println("#");        
      }
  }
