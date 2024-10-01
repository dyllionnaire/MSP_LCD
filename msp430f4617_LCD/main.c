#include <msp430.h>
#include <lcd.h>

union RTC_CTL
{
    unsigned char reg;
    struct
    {
        unsigned char RTC_FG    : 1;
        unsigned char RTC_IE    : 1;
        unsigned char RTC_TEVx  : 2;
        unsigned char RTC_MODEx : 2;
        unsigned char RTC_HOLD  : 1;
        unsigned char RTC_BCD   : 1;
    } flags;
};
typedef union RTC_CTL RTC_CTL;
volatile unsigned char* RTC_CTL_ADDR = (unsigned char*)(0x0041);

unsigned int MINUTES = 0;

/**
 * Seconds-based counting program to utilize the custom wrapper object for the integrated LCD
 * in order to create an animated element for use in a presentation of the aforementioned wrapper.
 *
 * Utilizes the RTC-counter functions, allowing for a total number of counts to 3599 seconds.
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    lcd_init();
    lcd_all(0,1,MAX);

    volatile RTC_CTL* ctrl = (RTC_CTL*)(RTC_CTL_ADDR);
    ctrl->flags.RTC_HOLD  = 1;

    ctrl->flags.RTC_FG    = 0;
    ctrl->flags.RTC_IE    = 1;
    __bis_SR_register(GIE);

    ctrl->flags.RTC_BCD   = 0;
    ctrl->flags.RTC_TEVx  = 0;
    ctrl->flags.RTC_MODEx = 3;

    ctrl->flags.RTC_HOLD  = 0;

    RTCNT2 = 0;
    RTCNT1 = 0;
    while(1)
        writeNum( MINUTES + RTCNT1 );

    return 0;
}

#pragma vector = BASICTIMER_VECTOR
__interrupt void BASICTIMER_ISR (void)
{
    MINUTES = 60*RTCNT2;
}
