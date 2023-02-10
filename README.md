# sram-tester-6264

************************************************************

This is a modified version of the original testers:

derived from the 2114 SRAM tester by Carsten Skjerk

https://github.com/skjerk/Arduino-2114-SRAM-tester

----AND----

(c) Dennis Marttinen 2022

Licensed under the MIT license

https://github.com/twelho/sram-tester

************************************************************

This version tests the 6264 SRAM and adds options to test the full pattern and other (FASTER) patterns.
 
The pin arrangement is optimized so a stripboard-mounted ZIF socket and connector can be plugged into the ARDUINO MEGA 36-pin connector.  Configure your stripboard or breadboard so that SRAM pins 1-14 are connected inline with MEGA pins 26-52, and SRAM pins 15-28 are connected inline with MEGA pins 53-27.  The VCC and GND(MEGA 52, 27) lines are configured as inputs to protect the MEGA as these need to be jumpered to 5v and GND on the MEGA.  The CS and OE lines(MEGA 39, 43) are configured as OUTPUT LOW.

Once you have configured a stripboard or breadboard and verified all the lines are correct as described above, flash the sketch to an Arduino MEGA.  Open the serial monitor at 115200 baud (Ctrl-Shft-M).  The serial monitor will prompt for a test pattern.  Enter the desired pattern and test results will print after completion.  The program will continue to loop, prompting for a test pattern.  
