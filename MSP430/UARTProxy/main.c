#include <msp430.h>

#include "misc.h"
#include "uart.h"
#include "utils.h"
#include "main.h"


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
  uart_init(UART_SRC_SMCLK, 104, UCBRS0);
  __bis_SR_register(GIE);
  
  //__bis_SR_register(LPM0_bits + GIE);       // enter LPM0, interrupts enabled

  while(1) {
    handle_uart(); 
  }
  
  return 0;
}

void handle_uart() {
  byte buf[16];
  byte count;
  count = uart_read(buf, 16);
  if (count > 0)
  	uart_write(buf, count);
}