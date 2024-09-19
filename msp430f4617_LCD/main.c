#include <msp430.h> 
#include <lcd.h>

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	return 0;
}
