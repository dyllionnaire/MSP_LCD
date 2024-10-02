# MSP_LCD
A wrapper object for driving the on-board LCD (limited to the 7.1 segment display) for the TI MSP430FG461x series MCUs.

In its current iteration, the only supported display characters are unsigned numerics:

  DECIMAL   = [0,9]
  
  HEX       = [A,F] + [a,f]
  
-and only for the first 7 digits (those corresponding to the 7.1 segment portion of the LCD). Printing to the LCD is done through either a character array using write(), which the program interprets as a LITERAL interpretation of the text passed and does not currently support any form of code conversion, or an unsigned integer using the writeNum(); thus, the maximum printable values are 9,999,999 using write() and (2^16)-1 using writeNum(). 

Additionally, the only display configuration currently supported for the four MUX modes available is the 4-MUX mode as well as no intrinsic support for driving peripheral displays -on the latter, though, some of the implemented features for manipulating the control registers may be useful still.

The LCD will run, in its configuration as per the initialization function, at 30Hz for FPS. This is achieved by setting
the LCD frequency flag to "128" for its divider, selected due to each LCD frame being composed of 4 display segment
states driven by the COM0-3 planes, which require 2 CPU cycles each (ON/OFF). Thus, we need to run the LCD at 240 Hz,
which we approximately achieve by dividing its clock, ACLK running @ 32kHz, with 128.

Please note that this is a student project intended to experiment with embedded systems programming and re-inforce
C programming techniques/style: use at your own discretion.
