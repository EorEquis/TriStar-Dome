// Pololu SMC config
  const int rxPin = 3;          // pin 3 connects to SMC TX   : Green
  const int txPin = 4;          // pin 4 connects to SMC RX   : Orange
  const int resetPin = 5;       // pin 5 connects to SMC nRST : Brown
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

// Flags and counters
  unsigned long lastMillis = 0;
  unsigned long currentMillis = 0;
  unsigned long lastButton = 0;

// ASCOM ShutterState Enumeration : http://www.ascom-standards.org/Help/Developer/html/T_ASCOM_DeviceInterface_ShutterState.htm
  #define shutterOpen 0
  #define shutterClosed 1
  #define shutterOpening 2
  #define shutterClosing 3
  #define shutterError 4

// Other variables
  //  #define DEBUG   // Uncomment to allow debug printout to serial
  String strCmd;
  unsigned int limitStatus = 0;
  unsigned int errorStatus = 0;
  int currentSpeed = 0;
  int targetSpeed = 0;
  int shutterState = 1;
  int buttonState = 0;
  const int motorSpeed = 1400;
  int infoDelay = 500;
  int buttonDelay = 1000;
  bool buttonPressed = false;

// SoftwareSerial for communication w/ SMC
  SoftwareSerial smcSerial = SoftwareSerial(rxPin, txPin);
