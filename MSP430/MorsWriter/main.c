#include <msp430.h>
#include "misc.h"
#include "utils.h"
#include <uart.h>
  
/*
Define which pin will be used as led
*/
#define LED_PIN     BIT5

/*
How many clock cycles is a unit (DOT)
512 - one second
*/
uint16_t UNIT_TIME;  
uint16_t divisor;


/*
Define a mapping between characters to DOT/DASH
*/
#define DOT         1
#define DASH        0
#define IGNORE      2

static char chars_to_units[26][5] = {
  {DOT ,  DASH,   IGNORE, IGNORE, IGNORE },    /* A */
  {DASH,  DOT,    DOT,    DOT,    IGNORE },    /* B */
  {DASH,  DOT,    DASH,   DOT,    IGNORE },    /* C */
  {DASH,  DOT,    DOT,    IGNORE, IGNORE },    /* D */
  {DOT,   IGNORE, IGNORE, IGNORE, IGNORE },    /* E */
  {DOT,   DOT,    DASH,   DOT,    IGNORE },    /* F */
  {DASH,  DASH,   DOT,    IGNORE, IGNORE },    /* G */
  {DOT,   DOT,    DOT,    DOT,    IGNORE },    /* H */
  {DOT,   DOT,    IGNORE, IGNORE, IGNORE },    /* I */
  {DOT,   DASH,   DASH,   DASH,   IGNORE },    /* J */
  {DASH,  DOT,    DASH,   IGNORE, IGNORE },    /* K */
  {DOT,   DASH,   DOT,    DOT,    IGNORE },    /* L */
  {DASH,  DASH,   IGNORE, IGNORE, IGNORE },    /* M */
  {DASH,  DOT,    IGNORE, IGNORE, IGNORE },    /* N */
  {DASH,  DASH,   DASH,   IGNORE, IGNORE },    /* O */
  {DOT,   DASH,   DASH,   DOT,    IGNORE },    /* P */
  {DASH,  DASH,   DOT,    DASH,   IGNORE },    /* Q */
  {DOT,   DASH,   DOT,    IGNORE, IGNORE },    /* R */
  {DOT,   DOT,    DOT,    IGNORE, IGNORE },    /* S */
  {DASH,  IGNORE, IGNORE, IGNORE, IGNORE },    /* T */
  {DOT,   DOT,    DASH,   IGNORE, IGNORE },    /* U */
  {DOT,   DOT,    DOT,    DASH,   IGNORE },    /* V */
  {DOT,   DASH,   DASH,   IGNORE, IGNORE },    /* W */
  {DASH,  DOT,    DOT,    DASH,   IGNORE },    /* X */
  {DASH,  DOT,   DASH,    DASH,   IGNORE },    /* Y */
  {DASH,  DASH,   DOT,    DOT,    IGNORE },    /* Z */

};

/*
index_in_msg - the index of the current letter being transmited.
*/
static int index_in_msg = 0;
/*
index_in_char - the index of symbol inside a letter current being transmitted.
*/
static int index_in_char = 0;
/*
A flag to indicate that we should turn the led of
*/
static char space_between_symbols = 0;

/*
The msg to be broadcasted...
*/
static char msg[] = "six years have passed remember where you came from and where youre going to come back to wishing you the best sos pasten sos team sonic ";

/*
The current char to be brodcasted.
*/
static char cur_char;

#define BUTTON BIT3
/*
A simple timer example which toggels pinX every second.
*/
int main() {
  WDTCTL = WDTPW + WDTHOLD;   // Stop WDT
  
  BCSCTL1 = CALBC1_1MHZ;      // set DCO to 1MHz
  DCOCTL  = CALDCO_1MHZ;

  BCSCTL1 |= DIVA_3;          // ACLK/8
  BCSCTL3 |= XCAP_3;          //12.5pF cap- setting for 32768Hz crystal
  
  // Enable button int
  P1DIR |= BIT6;      // For button pushes
  P1DIR &= ~BUTTON;   // Set button pin as an input pin
  P1OUT |= BUTTON;    // Set pull up resistor on for button
  P1REN |= BUTTON;    // Enable pull up resistor for button to keep pin high until pressed
  P1IES |= BUTTON;    // Enable Interrupt to trigger on the falling edge (high (unpressed) to low (pressed) transition)
  P1IFG &= ~BUTTON;   // Clear the interrupt flag for the button
  P1IE |= BUTTON;     // Enable interrupts on port 1 for the button
  
  // Clear buttong push
  P1OUT &= ~BIT6;

  
  /*
  How many clock cycles is a unit (DOT)
  512 - one second
  */
  divisor = 2;
  UNIT_TIME  = (512/4)/divisor;
  P1DIR |= (BIT0); 
  P2DIR |= (LED_PIN);  // set led pin as output

  // uart_init(UART_SRC_SMCLK, 104, UCBRS0);
  // uart_puts("Initializing morse writer.\n");

  cur_char = msg[0];
  CCTL0 = CCIE; // CCR0 interrupt enabled
  CCR0 = 100; 
  TACTL = TASSEL_1 | ID_3 | MC_1; // ACLK, /8, upmode


  _BIS_SR(LPM3_bits + GIE); 
}


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
  P1OUT ^= BIT6;

  P1IFG &= ~BIT3;
  divisor *= 2;
  if (divisor >= 16)  { divisor = 2; }
  UNIT_TIME = (512/4)/divisor;
}

#define ON 1
#define OFF 0
static int switch_led(char on, int time_units) {
  if (on) {
    P2OUT |= LED_PIN;
    P1OUT |= BIT0;
  } else {
    P2OUT &= ~LED_PIN;
    P1OUT &= ~BIT0;
  }
  TAR = 0;
  CCR0 = UNIT_TIME*time_units;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
   // P1OUT ^= LED_PIN;
   // return;

  // Turn off the led between symbols for 1 time units.
  if (space_between_symbols == 1) {
    space_between_symbols = 0;
    //TRACE("short space (OFF) %u", TAR);
    switch_led(OFF, 1);
    return;
  } 

  // Are we transmiting a space? turn led of for 7 time units.
  // Note that we get here after an IGNORE (which is already 3 time units of OFF)
  // And after the ' '
  if (cur_char == ' ') {
    switch_led(OFF, 7);
    index_in_msg = (index_in_msg+1) % (sizeof(msg)-1);
    cur_char = msg[index_in_msg];
    //TRACE("space (OFF) %u", TAR);
    return;   
  }
  
  
  // Get the current symbol in the char and advance to the next symbol
  char cur_symbol = chars_to_units[cur_char-'a'][index_in_char++];
  // flag to turn off led after the next symbol
  space_between_symbols = 1;
  
  // What do we need to transmit
  if (cur_symbol == IGNORE) {
    // We reached end of letter, advance to the next letter, load it and zero the index in char.
    index_in_char = 0;
    index_in_msg = (index_in_msg+1) % (sizeof(msg)-1);
    cur_char = msg[index_in_msg];
    
    // We already waited 1 cycle after the last char transmitted.
    // We need to wait 2 more and transmit the next char.
    // Hence we don't need the extra sleep between symbols, and only 2 sleep cycles.
    space_between_symbols = 0;
    //TRACE("ignore (OFF) %u", TAR);
    switch_led(OFF, 3);
  } else if  (cur_symbol == DASH) {
      //TRACE("dash (ON) %u", TAR);
      switch_led(ON, 3);
  } else if (cur_symbol == DOT) {
      //TRACE("dot (ON) %u", TAR);
     switch_led(ON, 1);
  }
  
}

