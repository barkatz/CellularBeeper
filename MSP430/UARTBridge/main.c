#include <msp430.h>

#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"
#include "uart.h"
#include "clock.h"

void do_proxy(); 

int main() {
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  clock_init(CLKSPEED_16MHZ);

  uart_init(UART_SRC_SMCLK, 9600);
  softuart_init(SOFTUART_SRC_SMCLK, 9600);
  __bis_SR_register(GIE);

  softuart_puts("UART Bridge:\r\n");
  
  do_proxy();
  return 0;
}

void do_proxy() {
  byte c;
    
  while(1) {
    // Read a char from PC, and write it to GSM module.
    while (uart_getc(&c))
      softuart_putc(c);
    
    // Read a char from GSM module and write to to PC
     while (softuart_getc(&c))
        uart_putc(c);
  }
}
