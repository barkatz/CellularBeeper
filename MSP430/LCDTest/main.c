#include <msp430.h>
#include "i2c.h"
#include "lcdi2c.h"

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

int main() {
  int i;

#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  // configure main clock
  BCSCTL1 = CLBC1;                    // set DCO to the desired clock speed.
  DCOCTL  = CALDCO;

  // initialize the I2C library
  i2c_init(LCD_I2C_ADDR);

  // initialize the LCD module
  lcdi2c_init();
  lcdi2c_puts("Hello, world!");
  
  while(1);

  return 0;
}