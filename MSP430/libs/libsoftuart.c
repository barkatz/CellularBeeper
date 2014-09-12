#include <msp430.h>
#include "softuart.h"
#include "misc.h"
#include "fifo.h"
#include "atomic.h"

/*
Software UART implementation using the MSP timers.
To fully understand timers configuration read slau144j.pdf, 12.3, Timer_A Registers.
To sum up:
TACTL    -> Timer_A Control.
TAR      -> The counter itself.
TACCTLX  -> The capture/compare control X
TACCRX   -> The capture/compare X
We will use Timer A, in continus mode, with 2 comparators  (Rx/Tx)

*/

// Use these pins for IO
#define PTX         BIT2              // P2.2 --> Timer1_A, CCI0A (Timer 1, Compartor/Capture 0)
#define PRX         BIT0              // P2.0 --> Timer1_A, CCI1A (Timer 1, Compartor/Capture 1) 

// Use Timer_A
#define TR          TA1R               // Timer Register.
#define TCTL        TA1CTL             // Timer Control.

// Define TX Comapartor to be compartor 1.
#define TXCCTL      TA1CCTL1           // TX Comparator control.
#define TXCCR       TA1CCR1            // TX Comparator register.
#define TXINTID     TIMER1_A1_VECTOR   // Timer 1, Compartor 1
  
// Define RX Comparator to be compartor 0.
#define RXCCTL      TA1CCTL0           // RX Comparator control.
#define RXCCR       TA1CCR0            // RX Comparator register.
#define RXINTID     TIMER1_A0_VECTOR   // Timer 1, Compartor 0



/**************************************
Local functions
**************************************/
static inline byte _softuart_try_putc(byte c);
static inline void _softuart_putc(byte c);
static inline byte _softuart_getc(byte *c);

static inline void _softuart_prepare_rx();
static inline void _softuart_prepare_tx();

/**************************************
Local variables
**************************************/
static FIFO tx_softuart_fifo;
static FIFO rx_softuart_fifo;
word tx_byte;
byte tx_bit_count;
word rx_byte;
byte rx_bit_count;

static word bit_time;

void softuart_init(softuart_clock_source_t src, word _bit_time) {
  // Reset the RX/TX state
  tx_bit_count = rx_bit_count = 0;
  fifo_init(&tx_softuart_fifo);
  fifo_init(&rx_softuart_fifo);

  /*
  Setup pins
  */
  P2DIR &= ~PRX;    // Set up PRX as input
  P2SEL |= PRX;     // Set up PRX as timer input
  P2SEL2 &= ~PRX;   // Set up PRX as timer input

  P2DIR |= PTX;     // Set up PTX as output
  P2SEL |= PTX;     // Set up PTX as timer output
  P2SEL2 &= ~PTX;   // Set up PTX as timer output    

  // debug
  P1DIR |= (BIT6 + BIT0);
  P1OUT &= ~(BIT6 + BIT0);
  
  // Keep bit time (how many CLK cycles per bit)
  bit_time = _bit_time;

  /*
  Prepare ctrl registers of timers.
  RX is set to capture the stop bit falling edge.
  TX is set to transmit stop bit
  */
  _softuart_prepare_tx();
  _softuart_prepare_rx();

  /*
  Configure clock
  Start timer in continus mode.
  */
  TCTL = src<<8 |  MC_2;                
}



static inline void _softuart_prepare_rx() {
  // Reset the RX state
  rx_byte = 0;
  rx_bit_count = 8;

  // Set the timer of the RX
  // SCS    - Synchronize
  // CM2    - Capture on falling edge.
  // CCIS0  - Take input from CCIxA
  // CAP    - Capture mode 
  // CCIE   - Interrupt enabled.
  RXCCTL = SCS | CCIS_0 | CM_2 | CAP | CCIE; 
}

static inline void _softuart_prepare_tx() {
  // Transmit stop bit
  TXCCTL = OUT;
}




/************************************************************************************************************************************
RX Interrupt

The interupts start in capture mode to capture the falling edge of the RX Pin (start of start bit)
It then switch to compare mode, to capture 8 bits of data.
After all 8 bits are recieved, they are pushed to queue, and the timer goes back to capture mode.
*************************************************************************************************************************************/
#pragma vector=RXINTID
__interrupt void softuart_rx_int_handler() {
  register int rxcctl = RXCCTL;

  // P1OUT ^= BIT0;
  // If we just captured the falling edge of the stop bit
  if (rxcctl & CAP) {
    // Make sure the input pin is LOW. If it is high, it is just a glitch!
    // For some reason this happens and i don't really understand if its a bug in the GSM chip or in my code :(
    if (P2IN & PRX) {
      return;
    }

    // Move to compare mode, skip the stop bit, and start sampling the bits.
    RXCCTL &= ~CAP;
    RXCCR += bit_time + bit_time/2;
    
  } else {
    // Set the timer for the next bit
    RXCCR += bit_time;

    // We're currently RX-ing a byte
    if (rx_bit_count > 0) {      
      // Check the input bit latched.
      // Recieve order is from MSB to LSB. pack the bits...
      if (rxcctl & SCCI) {
         rx_byte |=  0x100;
      }

      rx_byte >>= 1;
      rx_bit_count--;
    } else {
      // Push the RX-ed byte to the RX fifo
      fifo_try_put(&rx_softuart_fifo, rx_byte & 0xff, 1);

      // Reset timer and RX state.
      _softuart_prepare_rx();
    }
  }        
}



/************************************************************************************************************************************
TX Interrupt

The interrupt will send a bit every time, untill not more bytes are present in the send queue.
Then, it will disable the interrupt (untill _softuart_try_putc is called again to transmit more bytes).
*************************************************************************************************************************************/
#pragma vector=TXINTID
__interrupt void softuart_tx_int_handler() {
  byte c;
  // Check the capture interrupt flag of the TX Compartor Control registers.
  if (TXCCTL & CCIFG) { // This should only be checked if TXCCTL isn't TA1CCTL0
    // Clear interrupt.
    TXCCTL &= ~CCIFG; // Again, this should only be done if TXCCTL isn't TA1CCTL0

    // Synchronize the comparator for the next bit 
    TXCCR += bit_time;
    
    //
    // Are we in the middle of a byte?
    //
    if (tx_bit_count > 0) {
      // Transmit the next bit.
      if (tx_byte & 1) {
        TXCCTL = (TXCCTL & ~OUTMOD_7) | OUTMOD_1; // Set
      } else {
        TXCCTL = (TXCCTL & ~OUTMOD_7) | OUTMOD_5; // Reset
      }
      // Ditch the this bit
      tx_byte >>= 1;
      tx_bit_count--;
    //
    // If we finished transmiting a byte - check if there are more bytes in queue...
    //
    } else {
      // Try fetching the next byte to TX
      if (fifo_try_get(&tx_softuart_fifo, &c, 1)) {
        // Set the current byte to send, and add stop bit and start bits
        tx_byte = (0x100 | c) << 1;
        tx_bit_count = 10;
      } else {
        //
        // If there are no more bytes to send, stop the TX comparator
        //
        _softuart_prepare_tx();
        TXCCTL &= ~CCIE;
      }
    }
  }
}


void softuart_puts(byte *msg) {
  while (*msg)
    _softuart_putc(*msg++);
}

void softuart_write(byte *buf, byte count) {
  for (; count > 0; count--)
    _softuart_putc(*(buf++));
}

void softuart_putc(byte c) {
  _softuart_putc(c);
}

byte softuart_getc(byte *c) {
  return _softuart_getc(c);
}

byte softuart_read(byte *buf, byte count) {
  byte *buf_start = buf;
  while(count-- > 0 && softuart_getc(buf)) {
    buf++;
  }
  return (buf-buf_start);
}

static inline byte _softuart_getc(byte *c) {
  return fifo_try_get(&rx_softuart_fifo, c, 0);
}

static inline void _softuart_putc(byte c) {
  while (!_softuart_try_putc(c));
}

static inline byte _softuart_try_putc(byte c) {
  // Try to insert the byte into queue.  
  if (fifo_try_put(&tx_softuart_fifo, c, 0)) {
    // If the timer TX comparator is turned off,
    if (!(TXCCTL & CCIE)) {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!(TXCCTL & CCIE)) { // (check again atomically)
          // // Synchronize the TX comparator
          TXCCR  = TR;
          // // Enable the TX comparator interrupt, and invoke it (this will load a new byte)
          TXCCTL |= CCIE + CCIFG;
        }
      }
    }
    return 1;
  } else {
    return 0;
  }
}