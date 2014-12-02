#include <msp430.h>
#include "i2c.h"
#include "lcdi2c.h"
#include "softuart.h"
#include "uart.h"
#include "misc.h"

#define CLOCKSPEED_1MHZ

#ifdef  CLOCKSPEED_16MHZ
  #define CLBC1     CALBC1_16MHZ
  #define CALDCO    CALDCO_16MHZ
  #define BITTIME   (16000000/9600)
#elif   defined(CLOCKSPEED_8MHZ)
  #define CLBC1     CALBC1_8MHZ
  #define CALDCO    CALDCO_8MHZ
  #define BITTIME   (4000000/9600)
#elif   defined(CLOCKSPEED_4MHZ)
  #define CLBC1     CALBC1_4MHZ
  #define CALDCO    CALDCO_4MHZ
  #define BITTIME   (4000000/9600)
#elif   defined(CLOCKSPEED_1MHZ)
  #define CLBC1 CALBC1_1MHZ
  #define CALDCO CALDCO_1MHZ
  #define BITTIME (1000000/9600)
#endif

// The base of the transistor controling the Vcc of the LCD module is connected to our P2.3 pin
#define PLCDPWRPIN_PORT         P2              
#define PLCDPWRPIN_BIT          BIT3
// The base of the transistor connecting the PWRKEY pin of the SIM900 module to the ground is connected to our P2.4 pin
#define PSIM900PWRKEY_PORT      P2              
#define PSIM900PWRKEY_BIT       BIT4
// The STATUS pin of the SIM900 module is connected to our P2.5 pin
#define PSIM900STATUS_PORT      P2
#define PSIM900STATUS_BIT       BIT5

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

void sim900_power_init() {
  SET_PORT_BIT(PSIM900PWRKEY_PORT, DIR, PSIM900PWRKEY_BIT); // Set up PSIM900PWRKEY as output
}

void sim900_power_set(byte on) {
  if (on) {
    // TODO pull down PSIM900PWRKEYPIN for a little while
    // TODO wait for status PIN
  } else {
    // TODO pull down PSIM900PWRKEYPIN for a little while
    // TODO wait 500ms
  }
}

void power_on_sim900() {
  // TODO
}

void init() {
  // configure watchdog
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  // configure main clock
  BCSCTL1 = CLBC1;                    // set DCO to the desired clock speed.
  DCOCTL  = CALDCO;

  /* 9600Hz from 1MHZ, according to http://mspgcc.sourceforge.net/baudrate.html */
  uart_init(UART_SRC_SMCLK, BITTIME, UCBRS0);
  softuart_init(SOFTUART_SRC_SMCLK, BITTIME);
  __bis_SR_register(GIE);

  // power on the LCD screen
  lcd_power_init();
  lcd_power_set(1);

  // initialize the I2C library
  i2c_init(LCD_I2C_ADDR);

  // initialize the LCD module
  lcdi2c_init();
}

void do_proxy(); 

int main() {
  init();
  
  do_proxy();
  return 0;
}

void do_proxy() {
  byte c;

  while(1) {
    // // Read a char from PC, and write it to GSM module.
    while (uart_getc(&c)) {
      lcdi2c_putc(c);
      softuart_putc(c);
    }
    
    // Read a char from GSM module and write to to PC
    while (softuart_getc(&c)) {
      uart_putc(c);
    }
  }
}