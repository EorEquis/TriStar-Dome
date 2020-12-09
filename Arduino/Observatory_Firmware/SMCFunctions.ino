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
    /** For some reason, this now breaks the new SMC...it seems to never come out of reset mode.  2020-11-02
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);  // reset SMC
    delay(1);  // wait 1 ms
    pinMode(resetPin, INPUT);  // let SMC run again
  **/
    // must wait at least 1 ms after reset before transmitting
    delay(10);
  
    // Set up motor controller.
    smcSerial.write(0xAA);  // send baud-indicator byte
  }
