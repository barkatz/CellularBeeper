#include <msp430.h>
#include <string.h>
#include "misc.h"
#include "utils.h"
#include "main.h"
#include "softuart.h"
#include "uart.h"
#include "sim900.h"

/*******************
Pin layout 
P2.2      -> Softuart RX (with SIM900)
P2.0      -> Softuart TX (with SIM900)

P1.1/2    -> H/W UART   (with PC)

P1.7      -> RI (events from SIM900)
********************/
#define RI_PORT     P1
#define RI_BIT      BIT7


/*******************
Clocks speed
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
  __bis_SR_register(GIE); // Enable interrupts.
  //do_proxy();
  // Wait for a while -> let gsm module wake up (not rly needed.)
  ms_sleep(200);
  sim900_init();  
  ms_sleep(200);
  
  // Capture falling edge on RI port to capture calls and stuff.
  CLR_PORT_BIT(RI_PORT, DIR, RI_BIT);   // Make RI input
  //SET_PORT_BIT(RI_PORT, OUT, RI_BIT);   // Set pullup resistor ()
  CLR_PORT_BIT(RI_PORT, IFG, RI_BIT);   // Clear prev interrupts on port
  SET_PORT_BIT(RI_PORT, IES, RI_BIT);   // Capture falling transition (active low)
  SET_PORT_BIT(RI_PORT, IE, RI_BIT);    // Enable interrupts
  

  while (1) {
     TRACE("Going into Low power mode... Waiting for something...");
    __bis_SR_register(LPM4_bits | GIE);
    TRACE("RI changed :)...");
  }
  

  //do_proxy();

  
  
  return 0;
}

// XXX: THIS IS RI vector
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
  P1OUT ^= BIT0;
  CLR_PORT_BIT(RI_PORT, IFG, RI_BIT);   // Clear prev interrupts on port
  TRACE("HERE");
  __bic_SR_register_on_exit(LPM4_bits); // Go to main
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
