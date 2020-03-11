#include "main.h"
#include <xc.h>
#include <stdint.h>
#include "../digit-data/digits.h"


// Time structure, separated into four digits. Maintaining this as one integer
// would require modular math, but there's no modulus or division on this 
// processor so that would be too expensive.
union {
    int8_t array[4]; // Same four bytes, in array form
    struct {
        int8_t hour_h, // Hour, high digit
               hour_l, // Hour, low digit
               min_h,  // Minute, high digit
               min_l;  // Minute, low digit
    };
} static digs = {{2, 1, 5, 8}};


static volatile __bit // Bits: 0=no, 1=yes
    inc_h,   // Increment the hour?
    inc_m,   // Increment the minute?
    strobe;  // Pulse the strobe clock (either rising or falling)?
static uint8_t seg_data[7];          // One byte per segment, four bits each (one per digit)


__interrupt() void isr() {
    if (TMR0IF) { // clock timer interrupt
        TMR0H = T0_REL_H;  // Reload the timer value, high byte
        TMR0L = T0_REL_L;  // low byte
        TMR0IF = 0;        // Clear the timer interrupt flag
        inc_m = 1;         // Tell main to increment the minute
    }
    if (TMR1IF) { // strobe timer interrupt
        TMR1L = T1_REL_L;  // Reload the timer value, high byte
        TMR1H = T1_REL_H;  // low byte

        if (SR_CLOCK)
            SR_CLOCK = 0;  // Falling edge on the shift register clock line
        else {
            ANODES = 0;    // clear whatever is on the common digit bus
            SR_CLOCK = 1;  // Rising edge on the shift register clock line
            strobe = 1;    // Tell main to figure out new strobe data  
        }
        TMR1IF = 0;        // Clear the timer interrupt flag
    }
    if (IOCIF) { // interrupt on change button
        IOCIF = 0;      // clear the interrupt flag
        if (IOCCF4) {   // if hour button has been pressed
            inc_h = 1;  // tell main to increment the hour
            IOCCF4 = 0; // clear the change flag
        }
        if (IOCCF5) {   // if minute button has been pressed
            inc_m = 1;  // tell main to increment the minute
            IOCCF5 = 0; // clear the change flag
        }
    }
}


static void update_data() {
    // Non-reentrant
    // Fill seg_data by running through all segments, all digits
    static int8_t s, d;
    for (s = 6; s >= 0; s--) { // for every segment
        // the new segment byte, four bits, one per digit
        static uint8_t seg;
        seg = 0; 
        for (d = 3; d >= 0; d--) {    // for every digit
            seg |= (                  // add a bit to the new segment byte
                (
                    seg_patterns[     // index into the patterns array
                        digs.array[d] // grab the 0-9 digit for the current digit index
                    ]
                    >> s              // shift the appropriate segment bit into the
                                      // least-significant bit (LSB) position
                ) & 1                 // discard all but the LSB
            ) << d;                   // shift into the appropriate digit position
        }
        seg_data[s] = seg;            // copy the new segment byte to the array
    }
}


void main(void) {
    // Peripheral Pin Select init //////////////////////////////////////////////
    
    // These three get complaints from the simulator since RA2/4/5 are used as
    // outputs. Just set them to RA0 (0) since we leave that as an input and
    // don't use it.
    asm("banksel PPSLOCK");
    PPS_UNLOCK(c);
    asm("clrf CWG1PPS");
    asm("clrf CLCIN3PPS");
    asm("clrf T1GPPS");
    PPS_UNLOCK(s);
            
    // The rest of this initialization is roughly in bank order; see the .lst
    
    // Local variables /////////////////////////////////////////////////////////

    static int8_t seg_index = 0; // Current segment index, for strobing
    static __bit update;         // Whether we need to update the segment data

    /* Port configuration //////////////////////////////////////////////////////
    RA0: - PGD
    RA1: - PGC
    RA2: O Shift register clock
    RA3: - !MCLR
    RA4: O Shift register data
    RA5: O Shift register !clear
    
    RC0: O digit 0
    RC1: O digit 1
    RC2: O digit 2
    RC3: O digit 3
    RC4: I !hour button
    RC5: I !min button
    */
    // bits-----76543210-of the ports
    TRISA   = 0b11001011; // Tristate port A - 0=outputs for shift reg
    TRISC   = 0b11110000; // Tristate port C - 0=outputs for common digits
    LATA    = 0b11001011; // Latch port A - initial data
    WPUC    = 0b00110000; // Weak pull-up port C
    IOCCN   = 0b00110000; // Interrupt-on-change buttons: port C, negative edge
    INLVLC  = 0b11111111; // Schmitt trigger all of port C
    SLRCONA = 0; // Fast slew port A
    SLRCONC = 0; // Fast slew port C
    ANSELA  = 0;  // No ADC port A
    ANSELC  = 0;  // No ADC port C
    
    update_data();
    
    SR_CLOCK = 1;  // Clock the SR to clear it
    SR_CLEAR = 1;  // Stop clearing
    // The first time we get to the main loop, do a strobe re-load
    strobe = 1;
    
    // Timer 0 used for clock minutes
    TMR0H = T0_REL_H;  // high byte reload
    TMR0L = T0_REL_L;  // low byte reload
    T0CON1 = (0b010 << _T0CON1_T0CS_POSN)      // Fosc/4
           | _T0CON1_T0ASYNC_MASK              // Asynchronous
           | (0b1001 << _T0CON1_T0CKPS_POSN);  // 1:512 prescale
    T0CON0 = _T0CON0_T0EN_MASK                 // Timer on
           | _T0CON0_T016BIT_MASK              // 16-bit
           | (0b1110 << _T0CON0_T0OUTPS_POSN); // 1:15 postscale
    
    // Timer 1 used for strobe
    TMR1H = T1_REL_H;  // high byte
    TMR1L = T1_REL_L;  // low byte
    T1CLK = 0b0100;    // LFINTOSC 31kHz
    T1CON = (0b00 << _T1CON_CKPS_POSN) // 1:1 prescale
          | _T1CON_nSYNC_MASK          // Asynchronous
          // _T1CON_RD16_MASK          // 8-bit separate writes
          | _T1CON_ON_MASK;            // Timer on
    
    // Interrupt enable
    PIE0 = _PIE0_TMR0IE_MASK     // Timer 0 (clock)
         | _PIE0_IOCIE_MASK;     // Interrupt-on-change (buttons)
    PIE4 = _PIE4_TMR1IE_MASK;    // Timer 1 (strobe)
    INTCON = _INTCON_GIE_MASK    // Global interrupt enable
           | _INTCON_PEIE_MASK;  // Peripheral interrupt enable (for timer 1)
    
    IDLEN = 1; // Enable idle mode if sleep is issued
    
    for (;;) { // Loop forever  
        // Segment data loading ////////////////////////////////////////////////
        
        if (update) {    // if we need to change the time
            update = 0;  // clear the update flag
            update_data();
        }
        
        // Strobe management ///////////////////////////////////////////////////
        
        if (strobe) {
            // Shift register clock line just saw a rising edge
            // Load data for the current segment of all four digits
            LATC = seg_data[seg_index];
            
            if (++seg_index >= 7) {
                SR_DATA = 0;
                seg_index = 0;
            }
            
            // High on shift register data line only if we're on the first index
            SR_DATA = !seg_index;

            strobe = 0;
        }
        
        asm("sleep"); // CPU Idle until a new interrupt fires
        
        // Clock digit incrementing ////////////////////////////////////////////
        
        if (inc_m) {                   // if we need to increment the minute
            inc_m = 0;
            update = 1;
            digs.min_l++;              // increment the low digit
            if (digs.min_l >= 10) {    // if it rolls over 10
                digs.min_l = 0;        // set it back to zero
                digs.min_h++;          // increment the 10-minute (high) digit
                if (digs.min_h >= 6) { // if that rolls over 60 minutes
                    digs.min_h = 0;    // set it to zero
                    inc_h = 1;         // we'll need to increment the hour
                }
            }
        }
        
        if (inc_h) {                     // if we need to increment the hour
            inc_h = 0;
            update = 1;
            digs.hour_l++;               // increment the low digit
            if (digs.hour_h < 2) {       // if we're below 20:00 (8PM)
                if (digs.hour_l >= 10) { // and if the low digit rolls over 10:00
                    digs.hour_l = 0;     // set the low digit to 0
                    digs.hour_h++;       // increment the 10-hour (high) digit
                } 
            }
            else {                      // otherwise, if we're at or above 20:00
                if (digs.hour_l >= 4) { // if the low digit is above 4 (midnight, 24:00)
                    digs.hour_l = 0;    // set the low digit to zero
                    digs.hour_h = 0;    // set the high digit to zero as well
                }
            }
        }
    }
}
