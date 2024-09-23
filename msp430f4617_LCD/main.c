#include <msp430.h> 
#include <lcd.h>

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	/*
		TEST 1: configuration
			- should display all segments across all digits
	*/
	lcd.init();
	lcd.all(1);

	/*
		TEST 2: root write
			- able to write a "0" to 1st digit
	*/
	lcd.write( (DIGIT)(1), (NUMBER)(0) );

	/*
		TEST 3: character write
			- should display "12345" that are right-aligned to LCD segments 1-5.
	*/
	lcd.write( "12345", 5);

	return 0;
}
