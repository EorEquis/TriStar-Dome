# TriStar-Dome

Implementation of the ASCOM Dome interface for controlling a simple roll off roof.

It basically only does 2 things.  Open the roof.  Close the roof.  It's a simple game.

---

## Requirements

* A [Pololu Simple Motor Controller for brushed DC motors](https://www.pololu.com/category/94/pololu-simple-motor-controllers)
  * Tested and known to work with SMC18v15, SMC G2 24v12, and SMC G2 18v15.
  * it SHOULD work with any controller on that page.  Basically, it needs to support Pololu's binary command protocol.  If it does, it should work.
* Some sort of brushed DC motor that the SMC can run.
  * Size, speed, voltage, current, all up to you.  You are responsible for selecting a motor that will drive your roof, and not overload the motor controller.
  * 12V Windshield Wiper motors are known to be inexpensive, easily sourced, quite compatible with the SMC, and capable of moving a LARGE roof.  My own moves a 16' x 10' 2x4 framed roof sheethed with 7/16" zip board and corrugated metal roofing, and opens/closes it in about 1 minute.
* An arduino.  Nano or Uno are known to work, others should as well.  Some sort of SD Card shield or breakout is required to enable logging.
* 2 limit switches.  The code and diagrams presume normally closed (NC) switches.  NO switches CAN be used, but aren't reocmmended, and will require some DIY to reconfigure the SMC, wire correctly, and so on.
  * Limit switches can be pretty much whatever you want to cobble together.  Roller type, small micros, reed switches, up to you.
  * All you need to be able to do is have a switch that is normally closed until your roof gets wherever it's supposed to, at which point something opens the switch.
* Some means of supplying the necessary power to your motor.  Depends on motor, SMC, etc.
* Some sort of project box to hold the arduino, the SMC, and various connectors, if desired.

## Getting Started
* Connect the SMC to your motor and power supply per Pololu's documentation
* Use the Pololu app do make sure the motor works as desired.
* Configure the SMC as desired through the Pololu app.  There are several dozen settings, documented by Pololu, but the ones usually most applicable to a ROR environment are
  * Speed
  * Directions
  * Acceleration and Deceleration limits
  * Braking
  * Disable safe start
* Next connect your limit switches.  Use the Pololu app to configure them
  * Assign which switch to which direction
  * Train the SMC on the values seen when open/closed
  * Test their behaviour
* Wire up the arduino and SMC per the Fritzing diagram
  * The Reset Pin (pin 5 nominally) is not (currently) used by the latest arduino code on G2 controllers.
* Make note of the #defines at the top of Globals.h, and then upload sketch to your arduino
  * DEBUG prints extra info to the serial port.  Useful for troubleshooting, but **must be disabled for ASCOM driver to work**.
  * USEBUTTON enables or disables the use of a button to manually open/close/start/stop the roof.  Leave disabled if you have no button, as the pin can float without proper wireup, and cause weird behaviour.  Ask me how I know.
  * LOGGING enables some simple logging to an SD card.  This may be expanded in the future, but was mosty used to troubleshoot the button issue I certainly never had when I absolutely did not forget digital inputs can float.
* The serial monitor will allow you to test everything.  
  * Commands are documented in Observatory_Firmware.ino
  * Responses are just a shutter state followed by a #.  Shutter states are documented in Globas.h
  * All commands should be terminated with #
  
## Common problems
* Always receive SHUTTERERROR status.
  * Did you forget to disable DEBUG?
  * Check your condition.  If the motor is stopped, one of the limit switches needs to be open (triggered)...otherwise, we think the motor has stopped between open and closed, and that is indeed an error.
  * Connect to the SMC and see if it is in an error status.  If its error pin is high, we'll report SHUTTERERROR to ASCOM.  Safe Start and low/no VIn are frequently the issue here.
* I see garbage in the serial monitor
  * Check your baud rate
* ASCOM driver won't connect
  Are you using the correct serial port?  Remember, you want to connect the ASCOM driver to the Arduino, not the SMC.

## I still can't figure out _____.

* I'm happy to try to help.  Just email me.  Please understand this isn't a job or a product or anything...I have a life and a job and all that jazz, so I'll get to it when I can.

## My roof didn't close, didn't open, ran past a limit and fell into the swamp, exploded and burst into flames, or otherwise behaved in a not very norminal manner

* Sucks to be you.  You downloaded someone else's code, free of charge, and trusted your observatory to it.  You get what you get.
* If you're not comfortable with a bit of shitty arduino and VB code, making your own tweaks, wiring up your own electronics, and DIY risks in general, this probably isn't the project for you.
  
