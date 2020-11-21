int handleButton()
  {
    #ifdef USEBUTTON
      if (shutterState == SHUTTEROPENING || shutterState == SHUTTERCLOSING)
        {
          abortRoof();
        }
      else if (shutterState == SHUTTERCLOSED)
        {
          buttonPressed = false;
          Serial.print(openRoof());
          Serial.println("#");    
        }
      else if (shutterState == SHUTTEROPEN)
        {
          buttonPressed = false;
          Serial.print(closeRoof());
          Serial.println("#");        
        }
    #endif
  }
