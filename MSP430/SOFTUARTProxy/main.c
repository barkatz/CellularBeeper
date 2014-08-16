#include <msp430.h>

#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"

int main() {
  byte c;
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  // configure main clock
  BCSCTL1 = CALBC1_1MHZ;                    // set DCO to 1MHz
  DCOCTL  = CALDCO_1MHZ;

  /* 9600Hz from 32768Hz, according to http://mspgcc.sourceforge.net/baudrate.html */
  softuart_init(SOFTUART_SRC_SMCLK, 104);
  __bis_SR_register(GIE);
  
  //__bis_SR_register(LPM0_bits + GIE);       // enter LPM0, interrupts enabled

  while(1) {
    if (softuart_getc(&c)) {
      softuart_putc(c);
    }
  }
  
  return 0;
}

