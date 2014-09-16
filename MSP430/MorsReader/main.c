#include <msp430.h>
#include "misc.h"
#include "utils.h"
// #include "uart.h"
#include "lcd.h"

/*
Define which pin will be used as led
*/
#define LED_PIN     BIT5

/*
How many clock cycles is a unit (DOT)
512 - one second
*/
#define UNIT_TIME   ((512/4)/16)


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


#define EPSILON 5

#define N_DASH          3
#define N_DOT           1
#define N_SPACE         (7+3+1) // +3 for after letter, +1 in between symbols???
#define N_LETTER_SPACE  (3+1) // + 1 in between symbols???
#define N_AFTER_SYM     1

// diff - the clock ticks from last event (total time the led was ON)
// type - times unit for DASH/DOT/SPACE etc...
// This checks if the clock ticks dif matches the right type
#define CHECK_RATIO(diff, type) (((diff) > ((type)*UNIT_TIME - EPSILON)) && ((diff) < ((type)*UNIT_TIME + EPSILON)))


// Led was on if: P1ES (interrupt from high to low)
#define WAS_LED_ON            (!(P1IES & LED_PIN))

//
// Need to use ACLK, but in the meanwhile we will slow it down using software.
//
static unsigned int counter = 0;
static unsigned int last_counter = 0;


/*
A simple timer example which toggels pinX every second.
*/
int main() {
  WDTCTL = WDTPW + WDTHOLD;         // Stop WDT
 
  BCSCTL1 = CALBC1_1MHZ;            // set DCO to 1MHz
  DCOCTL  = CALDCO_1MHZ;
  BCSCTL1 |= DIVA_3;                // ACLK/8
  BCSCTL3 |= XCAP_3;                // 12.5pF cap- setting for 32768Hz crystal

  TACTL = TASSEL_1 | ID_3 | MC_2;   // ACLK, /8, continue mode.

  P1DIR   &= ~LED_PIN;

  P1IES   |= LED_PIN;   // Hi/Lo edge
  P1IE    |= LED_PIN;   // Enable interrupt
  P1IFG   &= ~LED_PIN;

  // uart_init(UART_SRC_SMCLK, 104, UCBRS0);
  // uart_puts("Initializing morse reader.\n");
  lcd_init();
  lcd_puts("Waiting :)");
  lcd_return_home();
  // enter LPM0, interrupts enabled
  _BIS_SR(LPM3_bits + GIE); 

}


static char search_letter(char *buf, char size){
  char letter, index;
  char match = 0;
  for (letter=0; letter < 26; letter++) {
    for (index = 0; index < size; index++) {
      if (chars_to_units[letter][index] != buf[index]) {
        break;
      }
    }
    if (index == size) {
      return letter + 'a';
    }
  }
  return '?';
}

static char buf[5];
static char l;
static int cur=0;
#pragma vector=PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  unsigned int diff;
  P1IFG &= ~LED_PIN; // Clear the flag telling us where we came from
  P1IES ^= LED_PIN;  // Toggle the high/low bit

  
  // calculate the time difference from the last time.
  counter = TAR;
  diff = counter - last_counter;
  last_counter = counter;
  //  if (WAS_LED_ON) {
  //    TRACE("%u ON", diff);
  //  } else {
  //    TRACE("%u OFF", diff);
  //  }
  // return;  
  if (WAS_LED_ON) {
    if (CHECK_RATIO(diff, N_DASH)) {
      buf[cur++] = DASH;
      //TRACE("dash");
    } else if (CHECK_RATIO(diff, N_DOT)){
      buf[cur++] = DOT;
      //TRACE("dot");
    } else {
      cur = 0;
      lcd_putc('1');
      // TRACE("error1");
    }
  } else {// led was off
    // If its a space between 2 words
    if (CHECK_RATIO(diff, N_SPACE)) {
      buf[cur++] = IGNORE;
      l = search_letter(buf, cur);
      cur = 0;
      lcd_putc(l);
      lcd_putc(' ');
      // uart_putc(l);
      // uart_putc(' ');
      // TRACE("SPACE");
      cur = 0;
    // If its a space between 2 letters 
    } else if(CHECK_RATIO(diff, N_LETTER_SPACE)) {
      //TRACE("-");
      buf[cur++] = IGNORE;
      l = search_letter(buf, cur);
      cur = 0;
      lcd_putc(l);
      // uart_putc(l);
    // If its just a space between symbols..
    } else if (CHECK_RATIO(diff, N_AFTER_SYM)) {
      // Nothing to do - 
    } else {
      cur = 0;
      lcd_putc('2');
      // TRACE("error2 -> %u", diff);
    }
  }
  return; 
}


