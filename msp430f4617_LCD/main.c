#include <msp430.h>
#include <lcd.h>

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	/*
		While not explicitly checked, this function MUST ABSOLUTELY
		be invoked before any of the following functionality can be
		used.

		This function enables the on-board LCD for limited use of
		the 7 segments in the 7.1 display region of unsigned
		integers, represented in decimal or hex.

		By default, it is set to 30Hz FPS at a 1/3 voltage bias.

		NOTE: the wrapper is configured to support 4-MUX ONLY; any
		attempt to configure to other modes will not be internally
		supported.
	*/
	lcd_init();

	lcd_segsOn(0);
	lcd_all(1,1,MAX);
	lcd_segsOn(1);

	lcd_segsOn(0);
	lcd_all(0,1,MAX);
	rwrite( (DIGIT)3 , (NUMBER)5 );
	lcd_segsOn(1);

	write( "12345", 5 );

	return 0;
}
