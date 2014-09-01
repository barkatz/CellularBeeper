#include <msp430.h>
#include "misc.h"
#include "utils.h"
  
/*
A simple timer example which toggels pinX every second.
*/
int main() {
  WDTCTL = WDTPW | WDTHOLD;                 // stop WDT

  // configure main clock
  BCSCTL1 = CALBC1_1MHZ;                    // set DCO to 1MHz
  DCOCTL  = CALDCO_1MHZ;

  
  //
  // Compartor Control Register 0 of TimerA.
  // We select OUTMODE_0, and enable the timer interrupt.
  // 
  TA0CCTL0 = OUTMOD_0 | CCIE;
  //
  // We make p1.5 be the output of timer A
  // See msp430g2553.pdf Page 49/43 table.
  // TA0.1 -> TimerA0, comparator1 output
  // TA0.0 -> TimerA0, comparator0 output
  //
  P1DIR |= BIT5;
  P1SEL |= BIT5;
  P2SEL &= ~BIT5;
  // Prepare the next interrupt
  TA0CCR0 = TAR + 0xf000;
  
  
  // Start Timer A using src clk in continuous mode
  TA0CTL = TASSEL_2 | MC_2 | TAIE; 
  
  // enter LPM0, interrupts enabled
  __bis_SR_register(LPM0_bits + GIE);       
  
}
int c;
int bit = 0;
#pragma vector=TIMER0_A0_VECTOR
__interrupt void softuart_tx_int_handler() {
   if (c==0x800){
     // Prepare the next bit to be sent.
     if (bit) {
      TA0CCTL0 =  (TA0CCTL0 & ~OUTMOD_7) | OUTMOD_1;
      P1OUT |= BIT4;
     } else {
      TA0CCTL0 =  (TA0CCTL0 & ~OUTMOD_7) | OUTMOD_5;
      P1OUT &= ~BIT4;
     }
     bit = ~bit;
     c = 0;
  }
  c++;
  TA0CCR0 += 0x100;
}

