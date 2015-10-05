#include <msp430.h>

#include "uart.h"
#include "clock.h"

#include "driverlib.h"

int main() {
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  clock_init(CLKSPEED_1MHZ);

  // Initialize UART for 9600 bps
  uart_init(UART_SRC_SMCLK, 104, UCBRF0, UCBRS1);
  __bis_SR_register(GIE);

  unsigned int i;
  while (1) {
    uart_putc('A');
    for (i=0; i < 100; i++)
      __delay_cycles(1024);
  }

  return 0;
}
