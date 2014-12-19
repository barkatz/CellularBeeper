#include <msp430.h>
#include "i2c.h"
#include "lcdi2c.h"
#include "softuart.h"
#include "uart.h"
#include "misc.h"
#include "clock.h"
#include "sim900.h"

// The base of the transistor controling the Vcc of the LCD module is connected to our P2.3 pin
#define PLCDPWRPIN_PORT         P2              
#define PLCDPWRPIN_BIT          BIT3

void lcd_power_init() {
  SET_PORT_BIT(PLCDPWRPIN_PORT, DIR, PLCDPWRPIN_BIT); // Set up PLCDPWRPIN as output
}

void lcd_power_set(byte on) {
  // OMG...
  #undef OUT
  if (on)
    SET_PORT_BIT(PLCDPWRPIN_PORT, OUT, PLCDPWRPIN_BIT);
  else
    CLR_PORT_BIT(PLCDPWRPIN_PORT, OUT, PLCDPWRPIN_BIT);

  // Sleeping until the LCD is up should be done at the driver
}

void init() {
  // configure watchdog
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  clock_init(CLKSPEED_1MHZ);

  uart_init(UART_SRC_SMCLK, 9600);
  softuart_init(SOFTUART_SRC_SMCLK, 9600);
  __bis_SR_register(GIE);

  // power on the LCD screen
  lcd_power_init();
  lcd_power_set(1);

  // initialize the I2C library
  i2c_init(LCD_I2C_ADDR);

  // initialize the LCD module
  lcdi2c_init(20, 4);
}

void do_proxy(); 
void do_work();

int main() {
  init();
  lcdi2c_puts("Ready... ");
  lcdi2c_return_home();
  //do_proxy();
  do_work();
  return 0;
}

void do_work() {
  while(1) {
    sim900_do_work();
  }
}

void do_proxy() {
  byte c;

  while(1) {
    // // Read a char from GSM
    while (uart_getc(&c)) {
      lcdi2c_putc(c);
      softuart_putc(c);
    }
    
    // Read a char from PC
    while (softuart_getc(&c)) {
      uart_putc(c);
    }
  }
}