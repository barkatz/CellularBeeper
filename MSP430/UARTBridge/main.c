#include <msp430.h>

#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"
#include "uart.h"


void do_proxy(); 
void test();


/*******************

********************/
#define CLOCKSPEED_16MHZ

#ifdef  CLOCKSPEED_16MHZ
#define CLBC1     CALBC1_16MHZ
#define CALDCO    CALDCO_16MHZ
#define BITTIME   (16000000/9600)


#elif   CLOCKSPEED_8MHZ
#define CLBC1     CALBC1_8MHZ
#define CALDCO    CALDCO_8MHZ
#define BITTIME   (4000000/9600)

#elif   CLOCKSPEED_4MHZ
#define CLBC1     CALBC1_4MHZ
#define CALDCO    CALDCO_4MHZ
#define BITTIME   (4000000/9600)


#elif   CLOCKSPEED_1MHZ
#define CLBC1 CALBC1_1MHZ
#define CALDCO CALDCO_1MHZ
#define BITTIME (1000000/9600)
#endif
int main() {
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif

  // configure main clock
  BCSCTL1 = CLBC1;                    // set DCO to the desired clock speed.
  DCOCTL  = CALDCO;

  /* 9600Hz from 1MHZ, according to http://mspgcc.sourceforge.net/baudrate.html */
  uart_init(UART_SRC_SMCLK, BITTIME, UCBRS0);
  softuart_init(SOFTUART_SRC_SMCLK, BITTIME);
  __bis_SR_register(GIE);
  
  do_proxy();
  return 0;
}


void test() {
  P1DIR |= BIT0;
  byte msg[] = "ATD + +972542405662;\r\n";
  //byte msg[] = "The quick brown fox jumped over the lazy dog\r\n";
  byte c;
  int i = 0;
  int j;
  while (1) {
    //softuart_putc(msg[i]);
    //while (softuart_getc(&c)) {
    //   P1OUT ^= BIT0;
    //}
    //i = (i+1) % sizeof(msg);

    // if (i == 0) {
    //   P1OUT |= BIT0;
    //   for (j=0; j<0x50; j++)  {
    //      __delay_cycles(1000);
    //   }
    //   P1OUT &= ~BIT0;
    //   for (j=0; j<0x500; j++)  {
    //      __delay_cycles(10000);
    //   }
    // }
 }
}

char dial[] = "ATD + +972542405662;\r\n";
char hang[] = "ATH\r\n";
char dial_or_hang = 1;

void do_proxy() {
  byte c;
  
  while(1) {
    // if (uart_getc(&c)) {
    //   if (dial_or_hang) {
    //     softuart_puts(dial);
    //   } else {
    //     softuart_puts(hang);
    //   }
    //   dial_or_hang ^= 1;
    // }
    // // Read a char from PC, and write it to gsm module.
    while (uart_getc(&c)) {
      softuart_putc(c);
    }
    
    // Read a char from gsm module and write to to PC
     while (softuart_getc(&c)) {
        uart_putc(c);
     }
    
  }
}
