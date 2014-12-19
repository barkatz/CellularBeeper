#include <msp430.h>
#include "lcd.h"
#include "misc.h"
#include "utils.h"
#include "clock.h"

static uint8_t x;
static uint8_t y;

/*
Sets the nibble on DB7-DB4.
*/
static void set_data_4bit(uint8_t nibble);
/*
Toggles the enable line to send our data.
*/
static void toggle_enable(void);
static void do_write_op_4bit(uint8_t value, uint8_t rs_on, uint8_t msec);

/*
Sets the BIT in PORT to 1 if IS_ON. 
Zeros the BIT if not IS_ON.
2 Macros are needed to expand the port...
*/
#define SET_PORT(PORT,BIT,IS_ON) _SET_PORT(PORT, BIT, IS_ON)
#define _SET_PORT(PORT, BIT, IS_ON)    \
  if (IS_ON) {                        \
    PORT ## OUT |= BIT;               \
  } else {                            \
    PORT ## OUT &= ~BIT;              \
  }                                   \

/*
Clears and set the Enable/RW/RS lines.
*/
// Method 1:
// This is the classic way to do it, but msp-gcc won't do it correctly....
// In this method the ports should be define as P1/P2...
// 2 Macros are needed to expand the port macro
#undef  SET_PORT_BIT
#define SET_PORT_BIT(PORT, BIT)        _SET_PORT_BIT(PORT,BIT)
#define CLEAR_PORT_BIT(PORT, BIT)      _CLEAR_PORT_BIT(PORT, BIT)      
#undef  _SET_PORT_BIT
#define _SET_PORT_BIT(PORT, BIT)        (PORT ## OUT           |= BIT)
#define _CLEAR_PORT_BIT(PORT, BIT)      (PORT ## OUT           &= ~BIT)
#define SET_E     SET_PORT_BIT(ENABLE_PORT, ENABLE_BIT)
#define SET_RW    SET_PORT_BIT(READ_WRITE_PORT, READ_WRITE_BIT)
#define SET_RS    SET_PORT_BIT(REGISTER_SELECT_PORT, REGISTER_SELECT_BIT)
#define CLR_E     CLEAR_PORT_BIT(ENABLE_PORT, ENABLE_BIT)
#define CLR_RW    CLEAR_PORT_BIT(READ_WRITE_PORT, READ_WRITE_BIT)
#define CLR_RS    CLEAR_PORT_BIT(REGISTER_SELECT_PORT, REGISTER_SELECT_BIT)

// Method 2:
// This won't expand the ports.
// In this method the ports should be define as P1/P2...
// #define SET_E     (ENABLE_PORT ## OUT           |= ENABLE_BIT)
// #define SET_RW    (READ_WRITE_PORT ## OUT       |= READ_WRITE_BIT)
// #define SET_RS    (REGISTER_SELECT_PORT ## OUT  |= REGISTER_SELECT_BIT)
// #define CLR_E     (ENABLE_PORT ## OUT           &= ~ENABLE_BIT)
// #define CLR_RW    (READ_WRITE_PORT ## OUT       &= ~READ_WRITE_BIT)
// #define CLR_RS    (REGISTER_SELECT_PORT ## OUT  &= ~REGISTER_SELECT_BIT)

// Method 3:
// In this method the ports should be define as P1OUT/P2OUT...
// #define SET_E     (ENABLE_PORT            |= ENABLE_BIT)
// #define SET_RW    (READ_WRITE_PORT        |= READ_WRITE_BIT)
// #define SET_RS    (REGISTER_SELECT_PORT   |= REGISTER_SELECT_BIT)
// #define CLR_E     (ENABLE_PORT            &= ~ENABLE_BIT)
// #define CLR_RW    (READ_WRITE_PORT        &= ~READ_WRITE_BIT)
// #define CLR_RS    (REGISTER_SELECT_PORT   &= ~REGISTER_SELECT_BIT)

/*
Sets the lines as output macro
*/
#define SET_DIR(PORT, BIT) _SET_DIR(PORT, BIT)
#define _SET_DIR(PORT, BIT) PORT ## DIR |= BIT


static void toggle_enable(void) {
  CLR_E;
  SET_E;
  CLR_E;
}

static void set_data_4bit(uint8_t nibble) {
  SET_PORT(DB4_PORT, DB4_BIT, nibble & BIT0); // DB4
  SET_PORT(DB5_PORT, DB5_BIT, nibble & BIT1); // DB5
  SET_PORT(DB6_PORT, DB6_BIT, nibble & BIT2); // DB6
  SET_PORT(DB7_PORT, DB7_BIT, nibble & BIT3); // DB7
}

static void do_write_op_4bit(uint8_t value, uint8_t rs_on, uint8_t msec) {
  /*
   Note that all times in data sheet are ~10-500 nano second (10^-9 seconds)
   We are running in 1MHZ-16MHZ - which is 10^-6 cycles.
   This means that each CPU cycle is 10^-6 -> each opcode takes at least 10^-6, which
   is way more then the minimum time we need to wait
  */
  
  // Clear all data bits
  set_data_4bit(0);
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
/*
A driver for LCD module.
Pin layout can be changed in the header file
*/
int lcd_init() {
  // Set all outputs
  SET_DIR(DB4_PORT, DB4_BIT);
  SET_DIR(DB5_PORT, DB5_BIT);
  SET_DIR(DB6_PORT, DB6_BIT);
  SET_DIR(DB7_PORT, DB7_BIT);
  
  SET_DIR(ENABLE_PORT, ENABLE_BIT);
  SET_DIR(READ_WRITE_PORT, READ_WRITE_BIT);
  SET_DIR(REGISTER_SELECT_PORT, REGISTER_SELECT_BIT);
  
  // Lower RS/RW -> We are writing control msgs.
  CLR_RS;
  CLR_RW;
  ms_sleep(10); // Just to make sure lines are down - can probably go off in prod.

  /*
   lcd module starts in 8 bit mode. 
   We need to change the mode to 4bit, but there are only 4 DB lines...
   The commands which changes to 4bit mode looks like: 0b001D NFXX. (D bit is 0 for 4 bit)
   Our command looks like this:
   0b0010 ZZZZ (we don't control the ZZZZ) but this changes us to 4 bits.
   ugly hack but it works
  */
  set_data_4bit(BIT1);
  toggle_enable(); // We toggle a few times just to be sure we're good.
  


  // Function set:
  // DB5 -> must be 1
  // DB4 -> DL, interface data length 1-> 8 bit, 0-> 4 bit.
  // DB3 -> N, number of display line 1-> 2 lines, 0-> 1 line
  // DB2 -> F, Font type -> 5x11 or 5x8.  
  do_write_op_4bit(BIT5 | BIT3 | BIT2 ,  0, 100);
    
  
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
  do_write_op_4bit(BIT2 | BIT1, 0, 100);

  // Clear display.
  do_write_op_4bit(BIT0 , 0, 100);
  x = y = 0;
  return 0;
}


void lcd_set_pos(uint8_t _x, uint8_t _y) {
  if (_y == 0) {
    do_write_op_4bit(0x80 | _x,0,2);
  } else {
    do_write_op_4bit(0xC0 | _x,0,2);
  }
  x = _x;
  y = _y;
}

void lcd_puts(char *msg) {
  while (*msg) {lcd_putc(*msg++);}
}

void lcd_putc(char c) {
 do_write_op_4bit(c , 1, 2); 
 x++;
 // There are 40 charaters in a line, we need to drop line when we got here.
 if (x == 16){
  x = 0;
  y ^= 1;
  lcd_set_pos(x,y);
 }
}

void lcd_clear() {
  do_write_op_4bit(BIT0 , 0, 2);  
  x = y =  0;
}

void lcd_return_home() {
  do_write_op_4bit(BIT1 , 0, 2);  
  x = 0;
}