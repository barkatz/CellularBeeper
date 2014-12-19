#include <msp430.h>
#include "lcdi2c.h"
#include "misc.h"
#include "utils.h"
#include "i2c.h"
#include "clock.h"

static uint8_t backlight_on;

static uint8_t rows;
static uint8_t cols;

static uint8_t x;
static uint8_t y;

static void expand_4_bits(uint8_t nibble, uint8_t rs, uint8_t en);
static void send_4_bits(uint8_t nibble, uint8_t rs);
static void send_byte(uint8_t data, uint8_t rs, uint32_t ms);

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

/*
A driver for LCD module.
Pin layout can be changed in the header file
*/
int lcdi2c_init(uint8_t cols_, uint8_t rows_) {
  int i;

  rows = rows_;
  cols = cols_;
  backlight_on = 1;
  /*
   Init sequence according to HD44780 datasheet, p46, fig 24.
  */

  // According to the datasheet we need to wait 40ms, but we take some spares
  ms_sleep(50);

  /*
  Set DB4/5 three times.
  */
  for (i = 0; i < 3; i++) {
    send_4_bits(BIT1 | BIT0, 0);
    ms_sleep(5); // According to the datasheet we need to wait 4.1ms, 100us, 0ms
  }
  send_4_bits(BIT1, 0);
  
  // Function Set
  // DB5 -> must be 1
  // DB4 -> DL, interface data length 1-> 8 bit, 0-> 4 bit.
  // DB3 -> N, number of display line 1-> 2 lines, 0-> 1 line
  // DB2 -> F, Font type -> 5x11 or 5x8.
  // Execution time is 37us
  send_byte(BIT5 | BIT3 | BIT2, 0, 1);
  
  // Display ON/OFF Control
  // DB3 -> must be 1.
  // DB2 -> Display 1->ON 0->OFF
  // DB1 -> Cursor 1->ON 0->OFF
  // DB0 -> Blinking Cursor 1->ON 0->OFF
  // Execution time is 37us
  send_byte(BIT3 | BIT2 | BIT0, 0, 1);
  
  // Entry Mode Set
  // DB2 -> must be 1
  // DB1 -> I/D - assign cursor moving direction - 1->Increment, 0->Decrement
  // DB0 -> SH - enable shift of the entire display
  // Execution time is 37us
  send_byte(BIT2 | BIT1, 0, 1);

  // Clear display.
  // Execution time is 1.53ms
  send_byte(BIT0, 0, 2);

  x = y = 0;

  return 0;
}


void lcdi2c_set_pos(uint8_t _x, uint8_t _y) {
  static byte row_offsets[] = {0x00, 0x40, 0x14, 0x54};

  // Execution time is ~40us
  send_byte(0x80 | row_offsets[_y] | _x, 0, 1);

  x = _x;
  y = _y;
}

void lcdi2c_puts(char *msg) {
  while (*msg) {lcdi2c_putc(*msg++);}
}

void lcdi2c_putc(char c) {
  // Execution time is ~4us
  if (c == '\r') {
    x = 0;
  } else if (c == '\n') {
    lcdi2c_newline();
  } else {
    send_byte(c, 1, 1); 
    x++;
    if (x >= cols)
      lcdi2c_newline();
  }
}

void lcdi2c_clear() {
  // Execution time is 1.53ms
  send_byte(BIT0, 0, 2);  
  x = y =  0;
}

void lcdi2c_return_home() {
  // Execution time is 1.52ms
  send_byte(BIT1, 0, 2);  
  x = 0;
}

void lcdi2c_newline() {
  x = 0;
  y = (y + 1) % rows;
  lcdi2c_set_pos(x,y);
}