void createFile()
  {
    char filename[] = "R_LOG000.CSV";
    for (uint8_t i = 0; i < 1000; i++) {
      filename[5] = i/100 + '0';
      filename[6] = (i%100)/10 + '0';
      filename[7] = (i%100)%10 + '0';
      if (! SD.exists(filename)) {
        // only open a new file if it doesn't exist
        logfile = SD.open(filename, FILE_WRITE);
        logfile.println("time,message");
        logfile.flush();
        #ifdef DEBUG
          Serial.print("Wrote header to ");
          Serial.println(filename);
        #endif   
        break;  // leave the loop!
      }
    }
  }
  
void writeToLog(String msg)
  {
    now = RTC.now();
    logfile.print(now.year(), DEC);
    logfile.print("/");
    logfile.print(now.month(), DEC);
    logfile.print("/");
    logfile.print(now.day(), DEC);
    logfile.print(" ");
    logfile.print(now.hour(), DEC);
    logfile.print(":");
    logfile.print(now.minute(), DEC);
    logfile.print(":");
    logfile.print(now.second(), DEC);
    logfile.print(",");
    logfile.println(msg);
    logfile.flush();    
  }
