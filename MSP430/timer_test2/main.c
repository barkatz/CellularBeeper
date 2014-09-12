#include <msp430.h>
#include "misc.h"
#include "utils.h"
  
/*
Using the capturer example.
There are 2 timers on the msp430g2553
Timer_A0
Timer_A1
Each has 3 comparators.

So
TA1CTL -> Timer_A1 control
TA0CTL -> Timer_A0 control

TAxCCTLY -> Timer_Ax CCTL y (x - timer number, y - compartor number)
*/
int main() {
  int j = 0;
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT

  // configure main clock
  BCSCTL1 = CALBC1_1MHZ;                    // set DCO to 1MHz
  DCOCTL  = CALDCO_1MHZ;


  // debug led
  P1DIR |= BIT0 + BIT6;
  P1OUT = BIT6;
  
  // Set P2.1 as Timer1_A input CCI0A
  P2DIR &= ~BIT0;               
  P2SEL |= BIT0;
  P2SEL2 &= ~BIT0;
  

  // P2OUT = 0x00;
  // P2SEL = 0x00;
  // P2DIR = 0xFF;

  // Start Timer A using src clk in continuous mode
  TA1CTL = TASSEL_2 | MC_2;
  
  // Sync, Rising edge, Capture, Int
  TA1CCTL0 = CM_2 | CCIS_0 | SCS | CAP | CCIE;
  
  // enter LPM0, interrupts enabled
  __bis_SR_register(LPM0_bits + GIE);       

}

/*
Rx timer interrupt
(Timer_A1, CCR0 -> )
*/
#pragma vector=TIMER1_A0_VECTOR
__interrupt void softuart_rx_int_handler() {
  TA1CCTL0 &= ~CCIFG;
  P1OUT ^= BIT0;  
}
