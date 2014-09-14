#include <msp430.h>
#include "misc.h"
#include "utils.h"

/*
Port Mapping:
P1.0-P1.3 -> DB4-DB7

P2.5 -> ENABLE
P2.4 -> RW
P2.3 -> RS
*/


#define CYCLES_PER_MSEC ((1000000*16) / 1000)


#define SET_E     (P2OUT |= BIT5)
#define SET_RW    (P2OUT |= BIT4)
#define SET_RS    (P2OUT |= BIT3)
#define CLR_E     (P2OUT &= ~BIT5)
#define CLR_RW    (P2OUT &= ~BIT4)
#define CLR_RS    (P2OUT &= ~BIT3)




#define SET_PORT(PORT, BIT, IS_ON)    \
  if (IS_ON) {                        \
    PORT ## OUT |= BIT;               \
  } else {                            \
    PORT ## OUT &= ~BIT;              \
  }                                   \


/*
 * Internal funcs
 */
static void set_data(uint8_t value);
static void do_write_op(uint8_t value, uint8_t rs_on, uint8_t msec);

static void set_data_4bit(uint8_t nibble);
static void do_write_op_4bit(uint8_t value, uint8_t rs_on, uint8_t msec);

static void toggle_enable(void);
static void ms_sleep(int msec);


static void toggle_enable() {
  CLR_E;
  SET_E;
  CLR_E;
}


static void set_data(uint8_t value) {
  P1OUT = value;
  // SET_PORT(P1, BIT3, value & BIT0); // DB0
  // SET_PORT(P1, BIT4, value & BIT1); // DB1
  // SET_PORT(P1, BIT5, value & BIT2); // DB2

  // SET_PORT(P2, BIT0, value & BIT3); // DB3
  // SET_PORT(P2, BIT1, value & BIT4); // DB4
  // SET_PORT(P2, BIT2, value & BIT5); // DB5

  // SET_PORT(P1, BIT7, value & BIT6); // DB6
  // SET_PORT(P1, BIT6, value & BIT7); // DB7
}

static void set_data_4bit(uint8_t nibble) {
  P1OUT = nibble;
  // SET_PORT(P2, BIT1, nibble & BIT0); // DB4
  // SET_PORT(P2, BIT2, nibble & BIT1); // DB5

  // SET_PORT(P1, BIT7, nibble & BIT2); // DB6
  // SET_PORT(P1, BIT6, nibble & BIT3); // DB7
}

void do_write_op(uint8_t value, uint8_t rs_on, uint8_t msec) {
  /*
   Note that all times in data sheet are ~10-500 nano second (10^-9 seconds)
   We are running in 1MHZ-16MHZ - which is 10^-6 cycles.
   This means that each CPU cycle is 10^-6 -> each opcode takes at least 10^-6, which
   is way more then the minimum time we need to wait
  */
  // Firstly prepare data on bass.
  set_data(value);
  
  // Secondly set the RS value (depending on the op)
  if (rs_on) {
    SET_RS;
  } else {
    CLR_RS;
  }
  // Clear RW line -> Write op
  CLR_RW;

  // Lastly toggle the enable (we should be slow enough)
  toggle_enable();

  // Sleep after operation
  ms_sleep(msec);
}

void do_write_op_4bit(uint8_t value, uint8_t rs_on, uint8_t msec) {
  /*
   Note that all times in data sheet are ~10-500 nano second (10^-9 seconds)
   We are running in 1MHZ-16MHZ - which is 10^-6 cycles.
   This means that each CPU cycle is 10^-6 -> each opcode takes at least 10^-6, which
   is way more then the minimum time we need to wait
  */
  
  // Clear all data bits
  set_data(0);
  // Put the 4 higher bytes on DB4, DB5, DB6, DB7
  set_data_4bit( ((value>>4) & 0xF) );
  
  // Set the RS value (depending on the op)
  if (rs_on) {
    SET_RS;
  } else {
    CLR_RS;
  }
  // Clear RW line -> Write op
  CLR_RW;

  // toggle the enable (we should be slow enough)
  toggle_enable();

  // Send the lower bits
  set_data_4bit( ((value) & 0xF) );

  // toggle the enable (we should be slow enough)
  toggle_enable();


  // Sleep after operation
  ms_sleep(msec);
}

void ms_sleep(int msec) {
  int i=0;
  for (i=0; i<msec; i++) {
    __delay_cycles(CYCLES_PER_MSEC);
  }
}



void init_lcd() {
  // Lower RS/RW -> We are writing control msgs.
  CLR_RS;
  CLR_RW;
  ms_sleep(20);

  //
  // Switch to 4bit mode.
  //
  P1OUT = BIT1;
  toggle_enable();
  toggle_enable();
  toggle_enable();
  

  // Function set:
  // DB5 -> must be 1
  // DB4 -> DL, interface data length 1-> 8 bit, 0-> 4 bit.
  // DB3 -> N, number of display line 1-> 2 lines, 0-> 1 line
  // DB2 -> F, Font type -> 5x11 or 5x8.
  
  do_write_op_4bit(BIT5 | BIT3 | BIT2,  0, 100);
    
  
  // Display ON/OFF Controll
  // DB3 -> must be 1.
  // DB2 -> Display 1->ON 0->OFF
  // DB1 -> Cursor 1->ON 0->OFF
  // DB0 -> Blinking Cursor 1->ON 0->OFF
  do_write_op_4bit(BIT3 | BIT2 | BIT0 , 0, 100);
  
  
  // Entry Mode Set
  // DB2 -> must be 1
  // DB1 -> I/D - assign cursor moving direction - 1->Increment, 0->Decrement
  // DB0 -> SH - enable shift of the entire display
  do_write_op_4bit(BIT2 | BIT1 , 0, 100);

  // Clear display.
  do_write_op_4bit(BIT0 , 0, 100);

}

int main() {
  int i;
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT

  // configure main clock
  BCSCTL1 = CALBC1_16MHZ;                    // set DCO to 1MHz
  DCOCTL  = CALDCO_16MHZ;
  P1DIR |= 0xff;
  P2DIR |= 0xff;
  P1OUT |= 0x0;
  P2OUT |= 0x0;
  ms_sleep(100000);

  init_lcd();
  while(1) {
    do_write_op_4bit(97, 1, 100);
    ms_sleep(1000);
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

