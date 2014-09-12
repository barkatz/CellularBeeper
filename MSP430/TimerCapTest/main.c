#include <msp430.h>

#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"
#include "uart.h"



#define PRX BIT0
//#define PRX BIT1

int main() {
#ifdef USE_WDT
  WDTCTL = WDT_ARST_1000;                   // start WDT
#else
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT
#endif
  // For debug
  P1DIR |= BIT0;
  //P1OUT = 0;

  // configure main clock
  BCSCTL1 = CALBC1_1MHZ;                    // set DCO to 1MHz
  DCOCTL  = CALDCO_1MHZ;

  /* 9600Hz from 1MHZ, according to http://mspgcc.sourceforge.net/baudrate.html */
  //uart_init(UART_SRC_SMCLK, 104, UCBRS0);
  //softuart_init(SOFTUART_SRC_SMCLK, 104);
  __bis_SR_register(GIE);
  P1DIR |= (BIT0+BIT6);

  // Set P2.0 to be CCI0A
  P2DIR &= ~PRX;
  P2SEL |= PRX;
  P2SEL2 &= ~PRX;

  // Set P1.1 to be CCI0A
  // P1DIR &= ~PRX;
  // P1SEL |= PRX;
  // P2SEL &= ~PRX;

  // Start timer A in SMCLK, continue
  TACTL = TASSEL_2 | MC_2 ;

  // Captuare mode on falling edge from CCI0A
  TACCTL0 = CAP | SCS | CM_2 | CCIS_0 | CCIE;
  
  __bis_SR_register(LPM0_bits + GIE);       // enter LPM0, interrupts enabled

  return 0;
}


/*
interrupt on capture
*/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void softuart_rx_int_handler() {
  // This is our start bit! Prepare for byte RX.
  // Clear capture mode, and set the timer to 1.5 of the bit timer 
  //(1 bit time to skip the start bit, 1/2 time to sample in the middle of the next bit)
  P1OUT ^= BIT6;
  if (TACCTL0 & CCI) {
    P1OUT |= BIT0;
  } else {
    P1OUT &= ~BIT0;
  } 

}

