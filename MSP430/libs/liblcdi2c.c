#include <msp430.h>
#include "lcdi2c.h"
#include "misc.h"
#include "utils.h"
#include "i2c.h"

static uint8_t backlight_on;

static uint8_t x;
static uint8_t y;

static void expand_4_bits(uint8_t nibble, uint8_t rs, uint8_t en);
static void send_4_bits(uint8_t nibble, uint8_t rs);
static void send_byte(uint8_t data, uint8_t rs, uint32_t ms);
static void ms_sleep(uint32_t msec);

static void expand_4_bits(uint8_t nibble, uint8_t rs, uint8_t en) {
  i2c_write((nibble << 4) |
            (backlight_on << BACKLIGHT_BIT) |
            (rs << REGISTER_SELECT_BIT) |
            (0 << READ_WRITE_BIT) |
            (en << ENABLE_BIT));
}

static void send_4_bits(uint8_t nibble, uint8_t rs) {
  expand_4_bits(nibble, rs, 1);
  ms_sleep(1); // Should sleep >450ns
  expand_4_bits(nibble, rs, 0);
  ms_sleep(1); // Should sleep >37us
}

static void send_byte(uint8_t data, uint8_t rs, uint32_t ms) {
  send_4_bits((data & 0xf0) >> 4, rs);
  send_4_bits((data & 0x0f), rs);

  ms_sleep(ms);
}

static void ms_sleep(uint32_t msec) {
  uint32_t i=0;
  for (i=0; i<msec; i++) {
    __delay_cycles(CYCLES_PER_MSEC);
  }
}

/*
A driver for LCD module.
Pin layout can be changed in the header file
*/
int lcdi2c_init() {
  int i;

  backlight_on = 1;

  // TODO find out how many ms we actually need to sleep here
  ms_sleep(500);

  // Lower RS/RW -> We are writing control msgs.
  expand_4_bits(0, 0, 0);
  ms_sleep(10); // Just to make sure lines are down - can probably go off in prod.

  /*
   lcd module starts in 8 bit mode. 
   We need to change the mode to 4bit, but there are only 4 DB lines...
   The commands which changes to 4bit mode looks like: 0b001D NFXX. (D bit is 0 for 4 bit)
   Our command looks like this:
   0b0010 ZZZZ (we don't control the ZZZZ) but this changes us to 4 bits.
   ugly hack but it works
  */
  for (i = 0; i < 4; i++) {
    send_4_bits(BIT1, 0);
    ms_sleep(10);
  }
  
  // Function set:
  // DB5 -> must be 1
  // DB4 -> DL, interface data length 1-> 8 bit, 0-> 4 bit.
  // DB3 -> N, number of display line 1-> 2 lines, 0-> 1 line
  // DB2 -> F, Font type -> 5x11 or 5x8.
  send_byte(BIT5 | BIT3 | BIT2, 0, 100);    
  
  // Display ON/OFF Control
  // DB3 -> must be 1.
  // DB2 -> Display 1->ON 0->OFF
  // DB1 -> Cursor 1->ON 0->OFF
  // DB0 -> Blinking Cursor 1->ON 0->OFF
  send_byte(BIT3 | BIT2 | BIT0, 0, 100);
  
  
  // Entry Mode Set
  // DB2 -> must be 1
  // DB1 -> I/D - assign cursor moving direction - 1->Increment, 0->Decrement
  // DB0 -> SH - enable shift of the entire display
  send_byte(BIT2 | BIT1, 0, 100);

  // Clear display.
  send_byte(BIT0, 0, 100);

  x = y = 0;

  return 0;
}


void lcdi2c_set_pos(uint8_t _x, uint8_t _y) {
  if (_y == 0) {
    send_byte(0x80 | _x, 0, 2);
  } else {
    send_byte(0xC0 | _x, 0, 2);
  }
  x = _x;
  y = _y;
}

void lcdi2c_puts(char *msg) {
  while (*msg) {lcdi2c_putc(*msg++);}
}

void lcdi2c_putc(char c) {
 send_byte(c, 1, 2); 
 x++;
 // There are 40 charaters in a line, we need to drop line when we got here.
 if (x == 16){
  x = 0;
  y ^= 1;
  lcdi2c_set_pos(x,y);
 }
}

void lcdi2c_clear() {
  send_byte(BIT0, 0, 2);  
  x = y =  0;
}

void lcdi2c_return_home() {
  send_byte(BIT1, 0, 2);  
  x = 0;
}