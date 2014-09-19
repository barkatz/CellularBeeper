#include <msp430.h>
#include <string.h>
#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"
#include "uart.h"
#include "sim900.h"


/*******************

********************/
#define CLOCKSPEED_16MHZ

#ifdef  CLOCKSPEED_16MHZ
#define CLBC1           CALBC1_16MHZ
#define CALDCO          CALDCO_16MHZ
#define BITTIME         (16000000/9600)
#define CLOCKS_PER_MSEC (16*1000000/1000)
#endif

#ifdef CLOCKSPEED_8MHZ
#define CLBC1           CALBC1_8MHZ
#define CALDCO          CALDCO_8MHZ
#define BITTIME         (4000000/9600)
#define CLOCKS_PER_MSEC (8*1000000/1000)
#endif

#ifdef  CLOCKSPEED_4MHZ
#define CLBC1           CALBC1_4MHZ
#define CALDCO          CALDCO_4MHZ
#define BITTIME         (4000000/9600)
#define CLOCKS_PER_MSEC (4*1000000/1000)
#endif

#ifdef CLOCKSPEED_1MHZ
#define CLBC1           CALBC1_1MHZ
#define CALDCO          CALDCO_1MHZ
#define BITTIME         (1000000/9600)
#define CLOCKS_PER_MSEC (1*1000000/1000)
#endif



void do_proxy();

void ms_sleep(uint16_t msec);

int main() {
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  // Debug...
  P1DIR |= BIT0;

  // configure main clock
  BCSCTL1 = CLBC1;                    // set DCO to the desired clock speed.
  DCOCTL  = CALDCO;

  /* 9600Hz from 1MHZ, according to http://mspgcc.sourceforge.net/baudrate.html */
  uart_init(UART_SRC_SMCLK, BITTIME, UCBRS0);
  softuart_init(SOFTUART_SRC_SMCLK, BITTIME);
  __bis_SR_register(GIE);
  
  // Wait for a while -> let gsm module wake up.
  ms_sleep(200);
  
  sim900_init();  

  ms_sleep(200);
  
  // Wait for interrupts...

  TRACE("Switching to proxy mode...");
  do_proxy();

  __bis_SR_register(LPM3_bits | GIE);
  
  return 0;
}

void ms_sleep(uint16_t msec) {
  uint16_t i;
  for (i=0;i<msec;i++) {
    __delay_cycles(CLOCKS_PER_MSEC);
  }
}



void do_proxy() {
  byte c;
  
  while(1) {
    while (uart_getc(&c)) {
      softuart_putc(c);
    }
    
    // Read a char from gsm module and write to to PC
     while (softuart_getc(&c)) {
        uart_putc(c);
     }
    
  }
}
