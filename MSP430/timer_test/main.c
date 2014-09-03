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

  P1DIR |= BIT0;
  P2DIR |= BIT2;

  // Prepare the next interrupt
  TA0CCR0 = TAR + 0xf000;
  
  
  // Start Timer A using src clk in continuous mode
  TA0CTL = TASSEL_2 | MC_2 | TAIE; 
  
  // enter LPM0, interrupts enabled
  __bis_SR_register(LPM0_bits + GIE);       
  
}

int c;
#pragma vector=TIMER0_A0_VECTOR
__interrupt void softuart_tx_int_handler() {
   if (c==0x10){
    P2OUT ^= BIT2;
    P1OUT ^= BIT0;
    c = 0;
  }
  c++;
  TA0CCR0 += 0x100;
}

