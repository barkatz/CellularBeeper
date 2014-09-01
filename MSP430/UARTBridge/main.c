#include <msp430.h>

#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"
#include "uart.h"

int main() {
  byte c;
  int i;
  int j;
  int k;
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
  uart_init(UART_SRC_SMCLK, 104, UCBRS0);
  __bis_SR_register(GIE);
  
  //__bis_SR_register(LPM0_bits + GIE);       // enter LPM0, interrupts enabled
  while(1) {
    // Read a char from PC, and write it to gsm module.
    //if (uart_getc(&c)) {
     c= 'a';
      softuart_putc(c);
    //}
    
    // Reada char from gsm module and write to to PC
    if (softuart_getc(&c)) {
      uart_putc(c);
    }    
    //for (i=0; i<0x1000; i++) {
    //  for (j=0; j<0x3000; j++) {
    //    softuart_puts("ATD + +972542405662;");
    //  }
    //}
    
    //uart_putc(c);
  }
  return 0;
}

