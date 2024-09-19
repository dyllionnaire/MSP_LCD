/*
 * lcd.h
 *
 *  Tailored to create a more amenable interface with the LCD_A controller
 *  found on MSP430FG461x boards.
 *
 *  Integrated LCD is the SoftBaugh SBLCDA4
 *
 *  Created on: Sep 16, 2024
 *      Author: dayer1
 */

#ifndef LCD_H_
#define LCD_H_

//#include <msp430.h>
#include <stdbool.h>

#define NULL        0x0000

#define LCD_ACTL     0x0090      // LCD_A's master control register
#define LCD_APCTL0   0x00AC      // LCD_A port 0's control register
#define LCD_APCTL1   0x00AD      // LCD_A port 1's control register
#define LCD_AVCTL0   0x00AE      // LCD_A voltage 0's control register
#define LCD_AVCTL1   0x00AF      // LCD_A voltage 1's control register

const void* BASE    = (void*)(0x0091); // Pointer to 1st memory address for LCD digits  (i.e. digit 1)
const void* MEMTOP  = (void*)(0x0098); // Pointer to 8th memory address for LCD digits  (i.e. digit 8/7.1/"ONES")
const void* TOP     = (void*)(0x00A4); // Pointer to 16th memory address for LCD digits (i.e. digit 16)

//#define P5SEL extern volatile unsigned char P5SEL

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
        - Might be best to scope the enumeration with a struct
*/
enum segment
{
    a   = 0x80,
    b   = 0x40,
    c   = 0x20,
    d   = 0x01,
    e   = 0x02,
    f   = 0x08,
    g   = 0x04,
    dp  = 0x10
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

enum mode
{
    DECIMAL,
    BASE10,
    BINARY,
    BASE2,
    HEX,
    BASE16,
    BCD
};
typedef enum mode MODE;

const char R = {e||g};              // Segment display pattern for: "r"     ; used for representing register mode
const char X = {b||c||e||f||g};     // Segment display pattern for: "X"     ; used for representing hex mode
const char ERR[] =                  // Segment display pattern for : "Err"  ; used for representign error mode
{
    a||d||e||f||g,
    e||g,
    e||g
};

/*
    Provides a pointer to the stored segment display pattern for 0-9 and A-F(10-16 in hex)
*/
const char *numSegs(enum number num)
{
    const static char map[] =
    {
        a||b||c||d||e||f,       // 0
        b||c,                   // 1
        a||b||d||e||g,          // 2
        a||b||c||d||g,          // 3
        b||c||f||g,             // 4
        a||c||d||f||g,          // 5
        a||c||d||e||f||g,       // 6
        a||b||c,                // 7
        a||b||c||d||e||f||g,    // 8
        a||b||c||d||f||g,       // 9
        a||b||c||e||f||g,       // A = 10
        c||d||e||f||g,          // B = 11
        a||d||e||f,             // C = 12
        b||c||d||e||g,           // D = 13
        a||d||e||f||g,          // E = 14
        a||e||f||g              // F = 15
    };

    return ( num>=0 && num<16) ? &(map[num]) : NULL;
}

////////////////////////////////////////////////////////////////////////////////
/*
                            CUSTOM-TYPES
*/
////////////////////////////////////////////////////////////////////////////////

// Bit-field that is formatted for easy access to segments mapped for 4-MUX
typedef struct
{
   char a : 1;
   char b : 1;
   char c : 1;
   char h : 1;
   char f : 1;
   char g : 1;
   char e : 1;
   char d : 1;
} INV_SEG;
typedef union
{
    char    reg;
    INV_SEG seg;
} LCD_DIG;

void m_all(bool, LCD_DIG*); // forward declaration to-be used by memory register initialization

/*
    Maps the next available memory address for the LCD segment map to the parameterized pointer

    RETURN CODES:
        - "false"   = bad pointer argument or no more memory addresses for LCD available
        - "true"    = pointer parameter has been allocated a memory address and had its value cleared 
*/
bool m_init(LCD_DIG* mem)
{
    static short count = 0;

    // Ensure pointer is a valid
    if ( !mem )
        return false;

    // Check for available dresses
    if ( count > ( (int)&TOP - (int)&BASE + 1) )
        return false;

    mem = (LCD_DIG*)(BASE + count);

    // According to User Guide, the initial state of the memory registers are "unchanged;" 
    // thus, they are cleared to render display blank.
    m_all(0, mem);

    return true;
}

/*
    Asserts that all segments at a memory address are either set or cleared
*/
void m_all(bool val, LCD_DIG* mem)
{
    mem->reg |= val ? 0xF : 0x0;
}

/*
    Class encapsulating the associated LCD memory address
*/
typedef struct
{
    LCD_DIG* mem;

    bool (*init)(LCD_DIG*);
    void (*all)(bool, LCD_DIG*);
} LCD_MEM;

// Bit-field that is formatted for easy mutation of the LCD_A's control register
typedef struct
{
    char LCD_FREQx   : 3;
    char LCD_MXx     : 2;
    char LCD_SON     : 1;
    char unused     : 1;
    char LCD_ON      : 1;
} LCDACTL_BITS;
typedef union
{
    char            reg;
    LCDACTL_BITS    flag;
} LCDACTL_REG;

// Bit-field for LCD_A Port 0 control register
typedef struct
{
    char LCD_S28 : 1;
    char LCD_S24 : 1;
    char LCD_S20 : 1;
    char LCD_S16 : 1;
    char LCD_S12 : 1;
    char LCD_S8  : 1;
    char LCD_S4  : 1;
    char LCD_S0  : 1;
} LCDAPCTL0_BITS;
typedef union
{
    char            reg;
    LCDAPCTL0_BITS  flag;
} LCDAPCTL0_REG;

// Bit-field for LCD_A Port 1 control register
typedef struct
{
    char unused : 6;
    char LCD_S36 : 1;
    char LCD_S32 : 1;
} LCDAPCTL1_BITS;
typedef union
{
    char            reg;
    LCDAPCTL1_BITS  flag;
} LCDAPCTL1_REG;

// Bit-field for the LCD_A Voltage 0 control register
typedef struct
{
    char unused     : 1;
    char R03EXT     : 1;
    char REXT       : 1;
    char VLCD_EXT    : 1;
    char LCD_CPEN    : 1;
    char VLCD_REFx   : 2;
    char LCD_2B      : 1;
} LCDAVCTL0_BITS;
typedef union
{
    char            reg;
    LCDAVCTL0_BITS  flag;
} LCDAVCTL0_REG;

// Bit-field for the LCD_A Voltage 1 control register
typedef struct
{
    char unused     : 3;
    char VLCDx      : 4;
    char unused2    : 1;
} LCDAVCTL1_BITS;
typedef union
{
    char            reg;
    LCDAVCTL1_BITS  flag;
} LCDAVCTL1_REG;

////////////////////////////////////////////////////////////////////////////////
/*
                            LCD OBJECT
*/
////////////////////////////////////////////////////////////////////////////////


/*
    LCD CLASS

        Represents the integrated LCD for the MSP430, providing an instance for 
    accessing components of the LCD as well as functions which simplify the ability
    to drive the LCD without needing to know how to do so programatically.
*/
typedef struct
{
    LCDACTL_REG*      ctrl;
    LCDAPCTL0_REG*    port0;
    LCDAPCTL1_REG*    port1;
    LCDAVCTL0_REG*    volt0;
    LCDAVCTL1_REG*    volt1;

    LCD_MEM mems[8];

    void (*init)(int, LCD*, LCD_MEM*);
    bool (*freq)(int, LCDACTL_REG*);
    bool (*mux)(int, LCDACTL_REG*);
    bool (*segsOn)(bool, LCDACTL_REG*);
    bool (*on)(bool, LCDACTL_REG*);
    void (*all)(bool, LCD_MEM*);
    bool (*write)(DIGIT, NUMBER, LCD_MEM*);
} LCD;

void lcd_init(unsigned short n, LCD* lcd, LCD_MEM* mems)
{
    unsigned short i = 0;
    while ( i < n )
    {
        mems[i].init   = &m_init;
        if ( mems[i].init(mems[i].mem) == false ) break;

        mems[i].all    = &m_all;
        mems[i].all(0, mems[i].mem);

        i++;
    }
    if ( i < n ); // handle inability to allocate memory

    lcd->ctrl    = (LCDACTL_REG*)(LCD_ACTL);
    lcd->port0   = (LCDAPCTL0_REG*)(LCD_APCTL0);
    lcd->port1   = (LCDAPCTL1_REG*)(LCD_APCTL1);
    lcd->volt0   = (LCDAVCTL0_REG*)(LCD_AVCTL0);
    lcd->volt1   = (LCDAVCTL1_REG*)(LCD_AVCTL1);

    lcd->freq   = &freq;
    lcd->mux    = &mux;
    lcd->segsOn = &segsOn;
    lcd->on     = &on;
    lcd->all    = &all;
    lcd->write  = &write;

    lcd->freq(128, lcd->ctrl);
    lcd->mux(4, lcd->ctrl);

    // P5SEL |= (0x10||0x08||0x04);         // Enables the COM1-3 pins for driving the LCD (COM0 has a dedicated pin)

    // Select MSP430 port pins to function as segment pins S0-15, representing the 8/7.1 digits
    lcd->port0->flag.LCD_S0   = 1;
    lcd->port0->flag.LCD_S4   = 1;
    lcd->port0->flag.LCD_S8   = 1;
    lcd->port0->flag.LCD_S12  = 1;

    // No charge pump used, default everything everything
    lcd->volt0->reg = 0;

    lcd->segsOn(1, lcd->ctrl);
    lcd->on(1, lcd->ctrl);
}

static LCD lcd;

bool freq(int f, LCDACTL_REG* ctrl)
{
    switch (f)
    {
        case 32:
            ctrl->flag.LCD_FREQx = 0b000;
            break;
        case 64:
            ctrl->flag.LCD_FREQx = 0b001;
            break;
        case 96:
            ctrl->flag.LCD_FREQx = 0b010;
            break;
        case 128:
            ctrl->flag.LCD_FREQx = 0b011;
            break;
        case 192:
            ctrl->flag.LCD_FREQx = 0b100;
            break;
        case 256:
            ctrl->flag.LCD_FREQx = 0b101;
            break;
        case 384:
            ctrl->flag.LCD_FREQx = 0b110;
            break;
        case 512:
            ctrl->flag.LCD_FREQx = 0b111;
            break;
        default:
            return false;
    }
     return true;
}

bool mux(int m, LCDACTL_REG* ctrl)
{ return ( m>=1 && m<4 ) ? ( ctrl->flag.LCD_MXx = (m-1) )+m : false; };

bool segsOn(bool t, LCDACTL_REG* ctrl)
{ return ctrl->flag.LCD_SON = t; };

bool on(bool t, LCDACTL_REG* ctrl)
{ return ctrl->flag.LCD_ON = t; };

void all(bool val, LCD_MEM* mems)
{
    unsigned short i = 0;
    while ( i<8 )
        mems[i].all(val, mems[i].mem); i++;
}

bool write(DIGIT d, NUMBER n, LCD_MEM* mems)
{
    if ( d == ONES ); // special handling, to not affect

    mems[d-1].all(0, mems[d-1].mem);
    mems[d-1].mem->reg = *(numSegs(n));

    return 0;
}

bool write(const char c[], unsigned short n, MODE md, LCD_MEM* mems)
{
    unsigned short i = 0;
    switch (md)
    {
        case DECIMAL:
        case BASE10:
            while ( i<n )
            {
                int num = c[ (n-i)-1 ] - 48;

                write((DIGIT)i, (NUMBER)num, mems); i++;
            }

            break;
        case BINARY:
        case BASE2:
            while ( i<n )
                write((DIGIT)i, (NUMBER)(c[ (n-i)-1 ]), mems); i++;

            break;
        case HEX:
        case BASE16:
            while ( i<n )
            {
                unsigned short index = (n-i)-1;

                // Indexed character is ['0','9']
                if ( c[index] >= 48 && c[index] <= 57 )
                    write((DIGIT)i, (NUMBER)( c[index]-48 ), mems);
                
                // Indexed character is ['A','F']
                if ( c[index] >= 65 && c[index] <= 70 )
                    write((DIGIT)i, (NUMBER)( c[index]-65 ), mems);

                // Indexed character is ['a','f']
                if ( c[index] >= 97 && c[index] <= 102 )
                    write((DIGIT)i, (NUMBER)( c[index]-97 ), mems);
            
                i++;
            }

            break;
        default:
            return false;
    }

    while ( i<8 )
        mems[i].all(0, mems[i].mem); i++;

    return true;
}

#endif /* LCD_H_ */
