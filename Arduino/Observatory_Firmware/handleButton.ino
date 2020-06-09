int handleButton()
  {
    if (shutterState == shutterOpening || shutterState == shutterClosing)
      {
        abortRoof();
      }
    else if (shutterState == shutterClosed)
      {
        Serial.print(openRoof());
        Serial.println("#");    
      }
    else if (shutterState == shutterOpen)
      {
        Serial.print(closeRoof());
        Serial.println("#");        
      }
  }
