#include <msp430.h>
#include <lcd.h>

union RTC_CTL
{
	unsigned char reg;
	struct
	{
		unsigned char RTC_FG 	: 1;
		unsigned char RTC_IE 	: 1;
		unsigned char RTC_TEVx 	: 1;
		unsigned char RTC_MODEx : 1;
		unsigned char RTC_HOLD 	: 1;
		unsigned char RTC_BCD 	: 1;
	} flags;
}
typedef union RTC_CTL RTC_CTL;
volatile unsigned char* RTC_CTL_ADDR = (unsigned char*)(0x0041);	

unsigned int MINUTES = 0;

/**
 * Seconds-based counting program to utilize the custom wrapper object for the integrated LCD
 * in order to create an animated element for use in a presentation of the aforementioned wrapper.
 * 
 * Utilizes the RTC-counter functions, allowing for a total number of counts to (2^16)-1 max.
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	lcd_init();
	lcd_all(0,1,MAX);

	(RTC_CTL*) ctlr = (RTC_CTL*)(RTC_CTL_ADDR);
	ctrl->RTC_HOLD	= 0;

	ctrl->RTC_FG	= 0;
	ctrl->RTC_IE	= 1;
	ctrl->RTC_TEVx	= 0;
	ctrl->RTC_MODEx	= 3;
	ctrl->RTC_BCD	= 0;

	RTCNT1 = 0;
	RTCNT2 = 0;

	ctrl->RTC_HOLD	= 1;
	while(1)
		writeNum( MINUTES + RTCNT1 );

	return 0;
}

#pragma vector = BASICTIMER_VECTOR 
__interrupt void BASICTIMER_ISR (void) 
{ 
	MINUTES = 60*RTCNT2;
}