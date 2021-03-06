                                 #include <SoftwareSerial.h>

// #defines
  // #define DEBUG   // Uncomment to allow debug printout to serial
  // #define USEBUTTON    // Uncomment to allow use of manual open/close button
//  #define LOGGING         // Uncomment to support SD Shield w/ RTC and logging to SD Card.

// Setup for SD Card shield
  #ifdef LOGGING
    #include <SD.h>
    #include <RTClib.h>
    const int chipSelect = 10;  // Digital pin 10 for the SD cs line
    DateTime now;    
    RTC_DS1307 RTC;             // Real Time Clock object
    File logfile;               // Logging file
  #endif

//  Setup for button
  #ifdef USEBUTTON
    const int buttonPin = 7;      // pin 7 connects to button   : Yellow
    unsigned long lastButton = 0; // Counter for last button press
    int buttonState = 0;          // Current sate of button
    int buttonDelay = 1000;       // Debounce delay
    bool buttonPressed = false; 
  #endif
      
// Pololu SMC config
  const int rxPin = 3;          // pin 3 connects to SMC TX   : Green
  const int txPin = 4;          // pin 4 connects to SMC RX   : Orange
  const int resetPin = 5;       // pin 5 connects to SMC nRST : Grey
  const int errPin = 6;         // pin 6 connects to SMC ERR  : Blue

// SMC Variable IDs
  #define ERROR_STATUS 0
  #define LIMIT_STATUS 3
  #define TARGET_SPEED 20
  #define SPEED 21

// SMC motor limit IDs
  #define DECELERATION 2

// Motor State Values

  #define STOPPED 1         // targetSpeed = 0, currentSpeed = 0
  #define MOVING 2          // targetSpeed = currentSpeed
  #define ACCELERATING 3    // targetSpeed > currentSpeed
  #define DECELERATING 4    // targetSpeed < currentSpeed
  #define DIRECTIONOPEN 1   // 1 : Positive motor speed opens roof.  -1 : Negative motor speed opens roof

/** Limit Variable Bits
    Bit 0: Motor is not allowed to run due to an error or safe-start violation.
    Bit 4: Motor speed is not equal to target speed because of acceleration, deceleration, or brake duration limits.
    Bit 7: AN1 limit/kill switch is active (scaled value ≥ 1600).
    Bit 8: AN2 limit/kill switch is active (scaled value ≥ 1600).
**/

  #define ERRORBIT 0
  #define SPEEDMISMATCH 4
  #define OPENLIMIT 8
  #define CLOSEDLIMIT 7
  
// Flags and counters
  unsigned long lastMillis = 0;
  unsigned long currentMillis = 0;
  
// ASCOM ShutterState Enumeration : http://www.ascom-standards.org/Help/Developer/html/T_ASCOM_DeviceInterface_ShutterState.htm
  #define SHUTTEROPEN 0
  #define SHUTTERCLOSED 1
  #define SHUTTEROPENING 2
  #define SHUTTERCLOSING 3
  #define SHUTTERERROR 4

// Other variables
  String strCmd;
  unsigned int limitStatus = 0;
  unsigned int errorStatus = 0;
  int currentSpeed = 0;
  int targetSpeed = 0;
  int shutterState = 1;
  const int motorSpeed = 2800;
  int infoDelay = 1000;
  int motorState = 0;
  bool closedLimitSwitch = false;
  bool openLimitSwitch = false;

// SoftwareSerial for communication w/ SMC
  SoftwareSerial smcSerial = SoftwareSerial(rxPin, txPin);
