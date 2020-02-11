#pragma once

// PIC16F15323 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = OFF      // Clock Switch Enable bit (The NOSC and NDIV bits cannot be changed by user software)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7 // WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC       // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Register not write protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF not write protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#define SR_CLOCK LATA2
#define SR_DATA  LATA4
#define SR_CLEAR LATA5
#define ANODES   LATC

// Timer 0 for clock:
// Use LFINTOSC=31kHz, prescale=32, postscale=15
// timer ticks = -LFINTOSC/pre/post * 60s = -3875
//      If LFINTOSC proves too inaccurate, then
//      Fosc/4=8MHz, prescale=2048, postscale=15
//      timer ticks = -Fosc/4/pre/post * 60s = -15625
// Timer 0 reload 16-bit value
#define T0_REL ((uint16_t)-3875)
// High byte
#define T0_REL_H ((uint8_t)(T0_REL >> 8))
// Low byte
#define T0_REL_L ((uint8_t)(T0_REL & 0xFF))

// Timer 1 for strobe:
// Use Fosc/4=8MHz, prescale=8
// timer ticks = -Fosc/4/8/7kHz ~ -143
// actual freq = Fosc/4/8/143 = 6.993kHz
//     This clock doesn't tick in the simulator:
//     Use LFINTOSC=31kHz, prescale=4
//     timer ticks = -LFINTOSC/pre/7kHz ~ -1
//     actual freq = LFINTOSC/4/1 = 7.75kHz
// Timer 1 reload 16-bit value
#define T1_REL ((uint16_t)-130)
// High byte
#define T1_REL_H ((uint8_t)(T1_REL >> 8))
// Low byte
#define T1_REL_L ((uint8_t)(T1_REL & 0xFF))

// PPS lock/unlock sequence
#define PPS_UNLOCK(op)      \
    asm("movlw 0x55");      \
    asm("movwf PPSLOCK");   \
    asm("movlw 0xAA");      \
    asm("movwf PPSLOCK");   \
    asm("b" #op "f PPSLOCK, 0")
