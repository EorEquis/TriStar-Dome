// Pololu SMC config
  const int rxPin = 3;          // pin 3 connects to SMC TX   : Green
  const int txPin = 4;          // pin 4 connects to SMC RX   : Orange
  const int resetPin = 5;       // pin 5 connects to SMC nRST : Grey
  const int errPin = 6;         // pin 6 connects to SMC ERR  : Blue

// Other Pins
  const int buttonPin = 7;      // pin 7 connects to button   : Yellow

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
  unsigned long lastButton = 0;

// ASCOM ShutterState Enumeration : http://www.ascom-standards.org/Help/Developer/html/T_ASCOM_DeviceInterface_ShutterState.htm
  #define SHUTTEROPEN 0
  #define SHUTTERCLOSED 1
  #define SHUTTEROPENING 2
  #define SHUTTERCLOSING 3
  #define SHUTTERERROR 4

// Other variables
  // #define DEBUG   // Uncomment to allow debug printout to serial
  String strCmd;
  unsigned int limitStatus = 0;
  unsigned int errorStatus = 0;
  int currentSpeed = 0;
  int targetSpeed = 0;
  int shutterState = 1;
  int buttonState = 0;
  const int motorSpeed = 2800;
  int infoDelay = 1000;
  int buttonDelay = 1000;
  bool buttonPressed = false;
  int motorState = 0;
  bool closedLimitSwitch = false;
  bool openLimitSwitch = false;
  

// SoftwareSerial for communication w/ SMC
  SoftwareSerial smcSerial = SoftwareSerial(rxPin, txPin);
