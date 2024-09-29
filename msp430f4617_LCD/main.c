#ifdef msp430.h
#include <msp430.h>
#endif 
#include <lcd.h>

#define RTC_FORM 	0x00
#define RTC_HOLD 	0x40
#define RTC_MODE 	0x10
#define RTC_TEV		0x04
#define RTC_IE		0x02
#define RTC_FG		0x00

extern RTCCTL, BTCNT1, BTCNT2;
extern IE2_bit;

extern WDTCTL, WDTPW, WDTHOLD;

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
	lcd_all(1,1,15);

	RTCCTL = RTC_FORM|RTC_HOLD|RTC_MODE|RTC_TEV|RTC_IE|RTC_FG;

	BTCNT1 = 0;
	BTCNT2 = 0;

	IE2_bit.BTIE 	= 1;
	RTCCTL			&= ~RTC_HOLD;

	for(;;)
		#pragma __low_power_mode_3();

	return 0;
}

#pragma vector = BASICTIMER_VECTOR 
__interrupt void BASICTIMER_ISR (void) 
{ 
	unsigned int MSB, LSB;
	MSB = BTCNT2;
	MSB = MSB << 2;
	LSB = BTCNT1;

	writeNum( MSB+LSB ); 
	
}