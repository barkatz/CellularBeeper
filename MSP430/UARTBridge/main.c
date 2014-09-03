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

  /* 9600Hz from 1MHZ, according to http://mspgcc.sourceforge.net/baudrate.html */
  softuart_init(SOFTUART_SRC_SMCLK, 109);
  uart_init(UART_SRC_SMCLK, 104, UCBRS0);
  __bis_SR_register(GIE);
  
  //__bis_SR_register(LPM0_bits + GIE);       // enter LPM0, interrupts enabled
   while(1) {
    // Read a char from PC, and write it to gsm module.
    //if (uart_getc(&c)) {
    //   softuart_putc(c);
    // }
    
    // // Reada char from gsm module and write to to PC
    // if (softuart_getc(&c)) {
    //   uart_putc(c);
    // }    
    softuart_putc('a');
    uart_puts("OK.\n");
    //__delay_cycles(1);
    
  }
  return 0;
}

