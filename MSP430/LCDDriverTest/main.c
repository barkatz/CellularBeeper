#include <msp430.h>
#include "misc.h"
#include "lcd.h"
#include "utils.h"


int main() {
  int i=0;

  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT

  // configure main clock
  BCSCTL1 = CALBC1_16MHZ;                    // set DCO to 1MHz
  DCOCTL  = CALDCO_16MHZ;
  

  lcd_init();

  while(1) {
    lcd_puts("Hi there. This is a super simple test. is it working :)?");
   __delay_cycles(5000000);
   lcd_clear();
  }
  _BIS_SR(LPM0_bits + GIE); 

  return 0;
}
























///////////////////////////
// OLD
//////////////////////////
// /* LCD Ports on MSP */
// static uint16_t db[8][2] = {
//   {(int) &P1OUT, BIT3}, // DB0
//   {(int) &P1OUT, BIT4}, // DB1
//   {(int) &P1OUT, BIT5}, // DB2
//   {(int) &P2OUT, BIT0}, // DB3
//   {(int) &P2OUT, BIT1}, // DB4
//   {(int) &P2OUT, BIT2}, // DB5
//   {(int) &P1OUT, BIT7}, // DB6
//   {(int) &P1OUT, BIT6}, // DB7
// };



// #define SET_E     (P2OUT |= BIT5)
// #define SET_RW    (P2OUT |= BIT4)
// #define SET_RS    (P2OUT |= BIT3)

// #define CLR_E     (P2OUT &= ~BIT5)
// #define CLR_RW    (P2OUT &= ~BIT4)
// #define CLR_RS    (P2OUT &= ~BIT3)


// void set_db_bit(uint8_t i, uint8_t bit)
// {
//   if (bit) {
//     *((uint16_t *)db[i][0]) |= db[i][1];
//   } else {
//     *((uint16_t *)db[i][0]) &= ~db[i][1];
//   }
// }

// void set_db(uint8_t byte)
// {
//   uint8_t i;
//   uint8_t bit;
//   for (i = 0; i < 8; i++) {
//     bit = byte & (1 << i);
//     set_db_bit(i, bit);
//   }
// }

// void toggle_enable(void) {
//   SET_E;
//   __delay_cycles(10);
//   CLR_E;
// }

// void write_to_ram(byte c) {
//   SET_RS;
//   set_db(c);
//   toggle_enable();
//   __delay_cycles(200);
// }

// void write_control(byte c) {
//   CLR_RS;
//   set_db(c);
//   toggle_enable();
//   __delay_cycles(200);
// }






// void clear_lcd() {
//   write_control(0x1);
//   P1OUT = 0x0;
//   P2OUT = 0x0;
//   // 1.53 milli seconds
//   __delay_cycles(1550);
// }

// void init_lcd() {
//   CLR_RW; // RW is always zero.
  
//   __delay_cycles(0x20);
//   set_db_bit(5,1);
//   __delay_cycles(0x20);
//   toggle_enable();
//   __delay_cycles(0x20);
//   toggle_enable();
//   __delay_cycles(0x20);
//   toggle_enable();
//   __delay_cycles(0x20);
//   __delay_cycles(0x2000);


//   write_control(0x30 | 0x8);
//   __delay_cycles(0x20);
//   // Display ON/OFF Control: ON, Cursor (ON, and also blinking!)
//   write_control(0x0F);
//   __delay_cycles(0x20);
//   // Entry Mode Set: Increment (cursor moves forward)
//   write_control(0x06);
//   __delay_cycles(0x20);

//  __delay_cycles(1550); 
//   clear_lcd();
//   __delay_cycles(0x20);
  
// }
    
// /*
// A simple timer example which toggels pinX every second.
// */
// int main() {
//   WDTCTL = WDTPW | WDTHOLD;                 // stop WDT

//   // configure main clock
//   BCSCTL1 = CALBC1_1MHZ;                    // set DCO to 1MHz
//   DCOCTL  = CALDCO_1MHZ;

//   // All outputs
//   P1DIR |= 0xff;
//   P2DIR |= 0xff;
//   P1OUT = 0;
//   P2OUT = 0;
//   __delay_cycles(0x100);

//   init_lcd();
  
// }

