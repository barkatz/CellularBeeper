#include <msp430.h>

#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"


void test_tx();
void do_proxy();


int main() {
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  // configure main clock
  BCSCTL1 = CALBC1_1MHZ;                    // set DCO to 1MHz
  DCOCTL  = CALDCO_1MHZ;

  /* 1MHZ from 32768Hz, according to http://mspgcc.sourceforge.net/baudrate.html */
  softuart_init(SOFTUART_SRC_SMCLK, 104);
  __bis_SR_register(GIE);
  
  //__bis_SR_register(LPM0_bits + GIE);       // enter LPM0, interrupts enabled
  //test_tx();
  do_proxy();

}

void test_tx() {
  char msg[] = "The quick brown fox jumped over the lazy dog. 12345\r\n";
  int i, j;
  i = 0;
  while (1) {
    softuart_putc(msg[i]);
    i = (i + 1) % sizeof(msg);
    for (j=0; j<5; j++) {
      __delay_cycles(100);
    }
  }
}

void do_proxy() {
  byte c;
  while(1) {
    if (softuart_getc(&c)) {
      softuart_putc(c);
    }
  }
}



