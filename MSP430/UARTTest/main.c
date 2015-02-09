#include <msp430.h>

#include "misc.h"
#include "utils.h"
#include "uart.h"
#include "clock.h"

void do_proxy(); 

int main() {
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  clock_init(CLKSPEED_1MHZ);

  P1DIR |= BIT6;  // Set P1.6 to GPIO OUT
  P1DIR |= BIT0;  // Set P1.0 to GPIO OUT

  /*while(1)
  {
    P1OUT ^= BIT6;
    __delay_cycles(1000);
  }*/

  uart_init(UART_SRC_SMCLK, 9600);
  __bis_SR_register(GIE);

  while(1)
  {
    uart_putc('A');
    P1OUT ^= BIT0;
    __delay_cycles(20000);
  }

  return 0;
}