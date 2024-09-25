/*
 * lcd.h
 *
 *  Tailored to create a more amenable interface with the LCD_A controller
 *  found on MSP430FG461x boards.
 * 
 *  CURRENT FEATURES:
 *      - Able to initiate LCD from functions alone.
 *      - Able to program LCD operating parameters from functions alone.
 *      - Able to perform unsigned writes to LCD digits 1-7
 * 
 *  WIP:
 *      - Display mode conversion, given a "from" and "to" argument
 *      - Display signed values
 *      - Display floating-point numbers
 *
 *  Integrated LCD is the SoftBaugh SBLCDA4
 *
 *  Created on: Sep 16, 2024
 *      Author: dayer1
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdbool.h>

#define NULL            0x0000
#define MAX             7               // Maximum for LCDMEM addressability (WIP: kept at just the "7" in 7.1 segments)

/*
    Memory-mappings are made in accordance with the memory-mapped table found in the April 2013
    edition of the "MSP430x4xx Family User Guide," table 26-2.
*/
volatile unsigned char*   LCD_ACTL    = (unsigned char*)(0x0090);  // LCD_A's master control register
volatile unsigned char*   LCD_APCTL0  = (unsigned char*)(0x00AC);  // LCD_A port 0's control register
volatile unsigned char*   LCD_APCTL1  = (unsigned char*)(0x00AD);  // LCD_A port 1's control register
volatile unsigned char*   LCD_AVCTL0  = (unsigned char*)(0x00AE);  // LCD_A voltage 0's control register
volatile unsigned char*   LCD_AVCTL1  = (unsigned char*)(0x00AF);  // LCD_A voltage 1's control register

volatile unsigned char*   BASE        = (unsigned char*)(0x0093);  // Pointer to 1st memory address for LCD digits  (i.e. digit 1)
volatile unsigned char*   MEMTOP      = (unsigned char*)(0x009A);  // Pointer to 8th memory address for LCD digits  (i.e. digit 8/7.1/"ONES")
volatile unsigned char*   TOP         = (unsigned char*)(0x00A4);  // Pointer to 20th memory address for LCD digits (i.e. digit 16)

extern volatile unsigned char P5SEL; // Port function select register is 1 byte, hence "char" 

////////////////////////////////////////////////////////////////////////////////
/*
                            SEGMENT REFERENCE
*/
////////////////////////////////////////////////////////////////////////////////

/*
    Maps segment bits to their respective "symbolic" segment names,
        - Mapping is only applicable to the 4-MUX configuration
   
    NOTE:
        - There is a quasi-identifier conflict with a-f in "segment" and A-F in "number"
        - That being, the only difference between them is case-sensitivity
        - Might be best to scope the enumeration by embedding in a struct
        - LCDMEMx = MSB{ a - b - c - h - f - g - e - d }LSB
*/
enum segment
{
    a   = 0x01,     //    aaaaa
    b   = 0x02,     //  f       b
    c   = 0x04,     //  f       b
    d   = 0x08,     //    ggggg
    e   = 0x40,     //  e       c
    f   = 0x10,     //  e       c
    g   = 0x20,     //    ddddd
    dp  = 0x80      //          (dp)
};
typedef enum segment SEGMENT, SEG;

enum digit
{
    FIRST = 1,
    SECOND,
    THIRD,
    FOURTH,
    FIFTH,
    SIXTH,
    SEVENTH,
    ONES
};
typedef enum digit DIGIT, DGT;

// Symbolic aliases for numeric constants
enum number
{
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    xA,
    xB,
    xC,
    xD,
    xE,
    xF // hex codes
};
typedef enum number NUMBER, NUM;

/*
    Used in parsing functions to dictate current input format and/or conversions-to

    NOTE:
        - (?) can use diff. enum identifier for same value (e.g. DECIMAL=0; BASE10=0;).
            However, I do not know if the CCS compiler would like this.
*/
enum mode
{
    DECIMAL = 0,
    BASE10  = 0,
    BINARY  = 1,
    BASE2   = 1,
    HEX     = 2,
    BASE16  = 2,
    BCD     = 3
};
typedef enum mode MODE;

const char R        = {e|g};           // Segment display pattern for: "r"     ; used for representing register mode
const char X        = {b|c|e|f|g};  // Segment display pattern for: "X"     ; used for representing hex mode
const char MINUS    = {g};              // Segment display pattern for: "-"     ; used for representing signed values
const char ERR[]    =                   // Segment display pattern for: "Err"   ; used for representign error mode
{
    a|d|e|f|g,
    e|g,
    e|g
};

/*
    Provides a pointer to the stored segment display pattern for 0-9 and A-F(10-16 in hex).
    Wrapper function to ensure that programmer cannot corrupt segment pattern via their own
    code.

    RETURNS:
        - "const char*" = iff the requested numeric is available
        - "NULL"        = if the number is not in range, i.e. not represented 
*/
const unsigned char *numSegs(NUMBER num)
{
    static const unsigned char map[] =
    {
        a|b|c|d|e|f,        // 0
        b|c,                // 1
        a|b|d|e|g,          // 2
        a|b|c|d|g,          // 3
        b|c|f|g,            // 4
        a|c|d|f|g,          // 5
        a|c|d|e|f|g,        // 6
        a|b|c,              // 7
        a|b|c|d|e|f|g,      // 8
        a|b|c|d|f|g,        // 9
        a|b|c|e|f|g,        // A = 10
        c|d|e|f|g,          // B = 11
        a|d|e|f,            // C = 12
        b|c|d|e|g,          // D = 13
        a|d|e|f|g,          // E = 14
        a|e|f|g             // F = 15
    };

    if ( num>=16 )
        return NULL;

    return &( map[num] );
}

////////////////////////////////////////////////////////////////////////////////
/*
                            TEMPLATE-TYPES

        Using bit-field templates as "stencils" for the LCDMEMs such that the
    compiler will interpret byte- and bit-access to LCDMEM registers via the
    access methods for bit-fields.
*/
////////////////////////////////////////////////////////////////////////////////

// Bit-field that is formatted for easy access to segments mapped for 4-MUX
union LCD_DIG
{
    unsigned char reg;
    struct
    {
        unsigned char a : 1;
        unsigned char b : 1;
        unsigned char c : 1;
        unsigned char d : 1;
        unsigned char e : 1;
        unsigned char f : 1;
        unsigned char g : 1;
        unsigned char h : 1;
    } seg;
};
typedef union LCD_DIG LCD_DIG;

/*
    Class encapsulating the associated LCD memory address
*/
struct LCD_MEM
{
    volatile LCD_DIG*     dig;
    unsigned int          id;
};
typedef struct LCD_MEM LCD_MEM;

// Bit-field that is formatted for easy mutation of the LCD_A's control register
union LCDACTL_REG
{
    unsigned char reg;
    struct
    {
        unsigned char LCD_ON    : 1;
        unsigned char unused    : 1;
        unsigned char LCD_SON   : 1;
        unsigned char LCD_MXx   : 2;
        unsigned char LCD_FREQx : 3;
    } flags;
};
typedef union LCDACTL_REG LCDACTL_REG;

// Bit-field for LCD_A Port 0 control register
union LCDAPCTL0_REG
{
    unsigned char reg;
    struct
    {
        unsigned char LCD_S0    : 1;
        unsigned char LCD_S4    : 1;
        unsigned char LCD_S8    : 1;
        unsigned char LCD_S12   : 1;
        unsigned char LCD_S16   : 1;
        unsigned char LCD_S20   : 1;
        unsigned char LCD_S24   : 1;
        unsigned char LCD_S28   : 1;      
    } flags;
};
typedef union LCDAPCTL0_REG LCDAPCTL0_REG;

// Bit-field for LCD_A Port 1 control register
union LCDAPCTL1_REG
{
    unsigned char reg;
    struct
    {
        unsigned char LCD_S32   : 1;
        unsigned char LCD_S36   : 1;
        unsigned char unused    : 6;
    } flags;
};
typedef union LCDAPCTL1_REG LCDAPCTL1_REG;

// Bit-field for the LCD_A Voltage 0 control register
union LCDAVCTL0_REG
{
    unsigned char reg;
    struct
    {
        unsigned char LCD_2B    : 1;
        unsigned char VLCD_REFx : 2;
        unsigned char LCD_CPEN  : 1;
        unsigned char VLCD_EXT  : 1;
        unsigned char REXT      : 1;
        unsigned char R03EXT    : 1;
        unsigned char unused    : 1;
    } flags;
};
typedef union LCDAVCTL0_REG LCDAVCTL0_REG;

// Bit-field for the LCD_A Voltage 1 control register
union LCDAVCTL1_REG
{
    unsigned char reg;
    struct
    {
        unsigned char unused    : 1;
        unsigned char VLCD_x    : 1;
        unsigned char unused2   : 6;
    } flags;
};
typedef union LCDAVCTL1_REG LCDAVCTL1_REG;

////////////////////////////////////////////////////////////////////////////////
/*
                            LCD OBJECT
*/
////////////////////////////////////////////////////////////////////////////////

/*
    Asserts that all segments at a memory address are either set or cleared
*/
bool m_all(bool val, LCD_MEM* mem)
{
    if ( !mem 
        &&(     (TOP < (unsigned char*)(mem->dig))
        &&      (BASE > (unsigned char*)(mem->dig)) )
        )
        return false; // Invalid address

    mem->dig->reg = val ? 0xFF : 0x00;
    return true;
}

/*
    Maps the next available memory address for the LCD segment map to the parameterized pointer

    RETURNS:
        - "false"   = no more memory addresses for LCD available
        - "true"    = pointer parameter has been allocated a memory address and had its value cleared 

    NOTES:
        - (?) change return type to 'int' for distinct numeric error-codes, assert '0' as clean exit
*/
bool m_init(LCD_MEM* mem)
{
    static unsigned int count = 0; // used as offset for LCDMEM address

    // Check for available addresses (cannot exceed 20)
    if ( count >= ((TOP - BASE) + 1) )
        return false;

    // Assert the LCDMEMx address as the LCD_DIG custom-type to provide more streamlined interfacing
    mem->dig    = (LCD_DIG*)( BASE + count );
    mem->id     = count++;

    // According to User Guide, the initial state of the memory registers are "unchanged;" 
    // thus, they are cleared to render display blank.
    m_all(0, mem);

    return true;
}

/*
    LCD CLASS

        Represents the integrated LCD for the MSP430, providing an instance for 
    accessing components of the LCD as well as functions which simplify the ability
    to drive the LCD without needing to know how to do so programatically.

    NOTE:
        - Initialization of structure members within a structure declaration
            may or may not be allowed in some compilers; if so, initialize via
            standard curly-braced list initialization.
*/
struct LCD
{
    volatile LCDACTL_REG*        ctrl;
    volatile LCDAPCTL0_REG*      port0;
    volatile LCDAPCTL1_REG*      port1;
    volatile LCDAVCTL0_REG*      volt0;
    volatile LCDAVCTL1_REG*      volt1;

    LCD_MEM mems[MAX];
} lcd;

bool lcd_freq(int f)
{
    volatile LCDACTL_REG* ctrl = lcd.ctrl;
    switch (f)
    {
        case 32:
            ctrl->flags.LCD_FREQx = 0b000;
            break;
        case 64:
            ctrl->flags.LCD_FREQx = 0b001;
            break;
        case 96:
            ctrl->flags.LCD_FREQx = 0b010;
            break;
        case 128:
            ctrl->flags.LCD_FREQx = 0b011;
            break;
        case 192:
            ctrl->flags.LCD_FREQx = 0b100;
            break;
        case 256:
            ctrl->flags.LCD_FREQx = 0b101;
            break;
        case 384:
            ctrl->flags.LCD_FREQx = 0b110;
            break;
        case 512:
            ctrl->flags.LCD_FREQx = 0b111;
            break;
        default:
            return false;
    }
     return true;
}

bool lcd_mux(unsigned int m)
{ return ( m>=1 && m<=4 ) ? ( lcd.ctrl->flags.LCD_MXx = (m-1) )+m : false; };

bool lcd_segsOn(bool t)
{ return lcd.ctrl->flags.LCD_SON = t; };

bool lcd_on(bool t)
{ return lcd.ctrl->flags.LCD_ON = t; };

/*
    Sets or clear pins to the segment groupings flags in Port 0 and 1's registers.
    
    Setting cascade to "false" allows for assignment to only those pins, without
    disrupting the bit values of those around it; otherwise, "cascade"-ing a value
    from a pin will preserve the upper pins while assigning the lower pins.

    Ranged assignment is not currently supported though could be substituted with
    individual assignments to pins selected of the range with cascade disabled. 
*/
bool lcd_segPins(unsigned int pin, bool val, bool cascade)
{
    if ( pin > 39 )
        return false; // only 39 segments in total are available

    lcd_segsOn(0);

    if ( pin >= 32 )
    {
        unsigned char flags = ( (pin/36) ? (2+cascade) : 1);
        if (val)
            lcd.port1->reg |= flags;
        else
            lcd.port1->reg &= ~flags;

        if (!cascade)
            return lcd_segsOn(1);
        
        pin = 31;
    }

    pin /= 4;
    unsigned char flags = 1;
    while( pin-- )
        flags = (flags<<1) + cascade;

    if (val)
        lcd.port0->reg |= flags;
    else
        lcd.port0->reg &= ~flags;

    return lcd_segsOn(1);
}

/*
    Writes a value to all digits [start,end]
*/
bool lcd_all(bool val,
    unsigned int start, unsigned int end)
{
    if (
              (unsigned int)(MEMTOP-BASE)+1 < start
        ||    (unsigned int)(MEMTOP-BASE)+1 < end
        )
        return false;

    unsigned int i = start-1;

    LCD_MEM temp;
    while ( i < end )
    {
        temp.dig = (LCD_DIG*)(BASE + i++);
        if ( m_all( val, &temp ) == false ) return false;
    }

    return true;
}

void lcd_init()
{
    if( lcd_all(0,1,MAX) == false)
        return;

    unsigned int i = 0;
    LCD_MEM* mems = lcd.mems;
    while ( i < MAX )
        if ( m_init( &(mems[i++]) ) == false ) break;

    if ( i != MAX )
        return; // handle inability to allocate memory                                                      

    lcd.ctrl    = (LCDACTL_REG*)(LCD_ACTL);
    lcd.port0   = (LCDAPCTL0_REG*)(LCD_APCTL0);
    lcd.port1   = (LCDAPCTL1_REG*)(LCD_APCTL1);
    lcd.volt0   = (LCDAVCTL0_REG*)(LCD_AVCTL0);
    lcd.volt1   = (LCDAVCTL1_REG*)(LCD_AVCTL1);                                                                                
    P5SEL |= (0x10|0x08|0x04); // Enables the COM1-3 pins for driving the LCD (COM0 has a dedicated pin)

    // set LCD frequency with an appropriate divider for 30Hz FPS
    lcd_freq(128);

    // set MUX-mode to 4-Mux -the only supported memory scheme currently.
    lcd_mux(4);

    lcd_segPins(39, 0, 1); // clear all pins
    lcd_segPins(39, 1, 1); // set used pins

    // No charge pump used, set to default (no contrast controls enabled)
    lcd.volt0->reg = 0;
    lcd.volt1->reg = 0;

    lcd_on(1);
}

/*
    "Root" function of all overloaded write methods -any implemented write functions should invoke this function.

    RETURNS:
        - "false"   = Non-represented digit or numeric
        - "true"    = Successful wrote numeric to LCDMEM
    NOTES:
        - (?) change return type to int for more characteristic error codes
*/
bool rwrite(DIGIT d, NUMBER n)
{
    if ( d > MAX )
        return false; // Anything beyond 7 digits currently not support

    LCD_MEM* mems = lcd.mems;

    m_all( 0, &(mems[d-1]) );
    if ( ( mems[d-1].dig->reg = *(numSegs(n)) ) == NULL )
        return false;

    return true;
}

/*
    Writes a character array of MAX length to the LCD's 7 segments; represents a LITERAL print.

    Currently only supports characters in the ranges of:
        - ['0','9'] = decimal digits
        - ['A','F'] = hex digits (uppercase)
        - ['a','f'] = hex digits (lowercase)
        - unsigned integers
    
    RETURNS:
        - "false"
            = bad character array
            = length beyond max representation
            = unsupport character in array
            = any "false" returns from root write function

        - "true" = character successfully written
*/
bool write(const unsigned char* c, unsigned int len)
{
    if ( len > MAX || len <= 0 || c == NULL )
        return false;
    
    lcd_segsOn(0);

    unsigned int index  = 0;
    while ( index < len )
    {
        unsigned int    cnum    = c[index];
        DIGIT           dgt     = (DIGIT)(len - index);
        NUMBER          num;

        if      ( cnum >= 48 && cnum <= 57 )    num = (NUMBER)( cnum - 48 );        // check if indexed char is ['0','9']
        else if ( cnum >= 65 && cnum <= 70 )    num = (NUMBER)( cnum - 65 );        // check if indexed char is ['A','F']
        else if ( cnum >= 97 && cnum <= 102 )   num = (NUMBER)( cnum - 97 );        // check if indexed char is ['a','f']
        else                                    return false;                       // ASCII character not printable (currently)

        if ( !rwrite(dgt, num) )
            return false;

        index++;
    }

    lcd_segsOn(1);

    return true;
}

#endif /* LCD_H_ */
