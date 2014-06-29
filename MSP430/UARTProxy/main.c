#include <msp430.h>

#include "misc.h"
#include "uart.h"
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
  //uart_init(UART_SRC_SMCLK, 104, UCBRS0);
  //TRACE("foo");
  __bis_SR_register(GIE);
  softuart_init(SOFTUART_SRC_SMCLK, 104);

  //__bis_SR_register(LPM0_bits + GIE);       // enter LPM0, interrupts enabled

  while(1) {
    //handle_uart(); 
    if (softuart_getc(&c)) {
      softuart_putc(c);
    }
    /*for (i=0; i<1000; i++) {
       __delay_cycles(1000);
    }
    softuart_puts("hi this is soo cool :)!\r\n");*/
    //softuart_puts("A");
    	//__bis_SR_register(LPM0_bits);
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