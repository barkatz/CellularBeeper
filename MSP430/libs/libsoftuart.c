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

The pins we will be using are:
 P1.1 for RX (same as the hardware uart).
 P1.2 for TX (same as the hardware uart).

Note that the TX pin can be whatever pin we want, but the RX bin need to be one of the CCI pins (i.e pins that could be connected as timer input).

*/

// Use these pins for IO
#define PTX      BIT2 // P1.2
#define PRX      BIT1 // P1.1

// Use Timer_A
#define TR      TAR     // Timer Register.
#define TCTL    TACTL   // Timer Control.

// Define TX Comapartor to be compartor 1.
#define TXCCTL  TACCTL1           // TX Comparator control.
#define TXCCR   TACCR1            // TX Comparator register.
#define TXINTID TIMER0_A1_VECTOR  
  
// Define RX Comparator to be compartor 0.
// Note that only the CCI0A/B registers can be used for input using this way.
#define RXCCTL  TACCTL0
#define RXCCR   TACCR0
#define RXINTID TIMER0_A0_VECTOR


static inline byte _softuart_try_putc(byte c);
static inline void _softuart_putc(byte c);
static inline byte _softuart_getc(byte *c);
// Configure capturares.
static inline void _softuart_prepare_rx();
static inline void _softuart_prepare_tx();

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


  // Keep bit time (how many CLK cycles per bit)
  bit_time = _bit_time;

  // Set the TX pin to be output
  P1DIR  |= PTX;

  // 
  // Sets the RX pin. this goes together with softuart_prepare_rx() which initializes SCCIx
  //
  // Set the RX pin to be the input CCI0A(p1.1) (see msp430g2553.pdf, Table 16 P.43)
  P1DIR &= ~PRX;
  P1SEL |= PRX;
  P1SEL2 &= ~PRX;
  // Set the RX pin to be the input CCI0B(p2.3) (see msp430g2553.pdf, Table 20 P.51)
  //P2DIR &= ~PRX;
  //P2SEL |= PRX;
  //P2SEL2 &= ~PRX;

  // preparse tx/rx compartors.
  _softuart_prepare_tx();
  _softuart_prepare_rx();

  // Start Timer A using src clk in continuous mode
  TCTL = src<<8 | MC_2 ;
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

static inline byte _softuart_getc(byte *c) {
  return fifo_try_get(&rx_softuart_fifo, c, 0);
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
          // Synchronize the TX comparator
          TXCCR  = TR;
          // Enable the TX comparator interrupt, and invoke it (this will load a new byte)
          TXCCTL |= CCIE + CCIFG;
        }
      }
    }
    return 1;
  } else {
    return 0;
  }
}

static inline void _softuart_prepare_rx() {
  // Reset the RX state
  rx_byte = 0;
  rx_bit_count = 0;
  // Set the RX capturer to synchronize on timer clock, capture the falling edge of
  // the required pin, and enable it:
  // CAP    -> Capture mode enabled.
  // SCS    -> Synchhronize the capture with next timer clock.
  // CM_2   -> capture on falling edge
  // CCIS_0 -> Selects CCIxA (which is-)
  
  // The signal which will be sampled can be read via CCI (the pin sampled is hardware specific.)
  RXCCTL = CAP | SCS | CM_2 | CCIS_0 | CCIE;
  //RXCCTL = CAP | SCS | CM_2 | CCIS_1 | CCIE;
}
static inline void _softuart_prepare_tx() {
  // start TX-ing a stop bit
  P1OUT |= PTX;
}

/*
Rx interrupt.
*/
#pragma vector=RXINTID
__interrupt void softuart_rx_int_handler() {

  //
  // If we are in CAPTURE mode it means we still didn't get a single byte.
  // 
  if (RXCCTL & CAP) {
    // This is our start bit! Prepare for byte RX.
    // Clear capture mode, and set the timer to 1.5 of the bit timer 
    //(1 bit time to skip the start bit, 1/2 time to sample in the middle of the next bit)
    rx_bit_count = 8;
    RXCCR += bit_time + (bit_time/2);
    RXCCTL = CCIE | SCS;
  //
  // If we are not in capture mode we are in a middle of recieving a byte...
  //
  } else {
      // Set the timer for the next bit
      RXCCR += bit_time;

    // We're currently RX-ing a byte
    if (rx_bit_count > 0) {      
      // Recieve order is from MSB to LSB. pack the bits...
      if (RXCCTL & CCI) {
         rx_byte |=  0x100;
      } 
      rx_byte >>= 1;
      rx_bit_count--;
    } else {
      // Push the RX-ed byte to the RX fifo
      fifo_try_put(&rx_softuart_fifo, rx_byte & 0xff, 1);

      // Prepare for the next start bit - set capture mode etc...
      _softuart_prepare_rx();
      
    }
  }
}


/**
Tx interrupt.
This interrupt will be triggered only when some one called _softuart_try_putc to try and transmit a byte.
The interrupt will send a bit every time, untill not more bytes are in the send queue.
Then, it will disable the interrupt (untill _softuart_try_putc is called again to transmit more bytes)
*/
#pragma vector=TXINTID
__interrupt void softuart_tx_int_handler() {
  byte c;
  // Check the capture interrupt flag of the TX Compartor Control registers.
  if (TXCCTL & CCIFG) { // This should only be checked if TXCCTL isn't TACCTL0
    // Clear interrupt.
    TXCCTL &= ~CCIFG; // Again, this should only be done if TXCCTL isn't TACCTL0

    // Synchronize the comparator for the next bit 
    TXCCR += bit_time;

    //
    // Are we in the middle of a byte?
    //
    if (tx_bit_count > 0) {
      // Transmit the next bit.
      if (tx_byte & 1) {
        P1OUT |= PTX;
      } else {
        P1OUT &= ~PTX;
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
        TXCCTL &= ~CCIE;
      }
    }
  }
}
