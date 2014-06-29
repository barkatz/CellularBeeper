#include <msp430.h>
#include "softuart.h"
#include "misc.h"
#include "fifo.h"
#include "atomic.h"

// Use these P1 pins for IO
#define PTX      BIT2
#define PRX      BIT1

// Use Timer_A
#define TR      TAR
#define TCTL    TACTL

  // Using TACCx1 for TXCCx is specific for PTX being P1.2
#define TXCCTL  TACCTL1
#define TXCCR   TACCR1
#define TXINTID TIMER0_A1_VECTOR
  
  // Using TACCx0 for RXCCx is specific for PRX being P1.1
#define RXCCTL  TACCTL0
#define RXCCR   TACCR0
#define RXINTID TIMER0_A0_VECTOR


static inline byte _softuart_try_putc(byte c);
static inline void _softuart_putc(byte c);
static inline byte _softuart_getc(byte *c);
static inline void _softuart_prepare_rx();

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

  // Set the TX pin to be the output of our TX comparator
  P1DIR  |= PTX;
  P1SEL  |= PTX;
  P2SEL  &= ~PTX;

  // Set the RX pin to be the input of our RX capturer
  P1DIR &= ~PRX;
  P1SEL |= PRX;
  P2SEL &= ~PTX;

  // Set the TX comparator output mode to direct, and start TX-ing a stop bit
  TXCCTL = OUT | OUTMOD_0;
  _softuart_prepare_rx();

  // Start Timer A using SMCLK in continuous mode
  TCTL = src<<8 | MC_2;
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
  // Set the RX capturer to synchronize on timer clock, capture the falling edge of
  // the required pin, and enable it
  RXCCTL = CAP | SCS | CM_2 | CCIS_0 | CCIE;
}

#pragma vector=RXINTID
__interrupt void softuart_rx_int_handler() {
  if (RXCCTL & CAP) {
    P1DIR |= BIT0;
    P1OUT ^= BIT0;

    // This is our start bit! Prepare for byte RX
    rx_bit_count = 8;
    RXCCR += bit_time + (bit_time/2);
    RXCCTL = CCIE;

  } else {
    // We're currently RX-ing a byte

    if (rx_bit_count > 0) {
      // We've got a new bit
      rx_byte <<= 1;
      if (RXCCTL & CCI) {
        rx_byte |= 1;
      }

      rx_bit_count--;
    } else {
      // This is our stop bit
      P1DIR |= BIT6;
      P1OUT ^= BIT6;

      // Push the RX-ed byte to the RX fifo
      fifo_try_put(&rx_softuart_fifo, rx_byte & 0xff, 1);

      // Prepare for the next start bit
      _softuart_prepare_rx();
    }
  }
}

#pragma vector=TXINTID
__interrupt void softuart_tx_int_handler() {
  byte c;

  if (TXCCTL & CCIFG) { // This should only be checked if TXCCTL isn't TACCTL0
    TXCCTL &= ~CCIFG; // Again, this should only be done if TXCCTL isn't TACCTL0

    // Synchronize the comparator for the next bit
    TXCCR += bit_time;

    if (tx_bit_count > 0) {
      // Schedule a bit to TX for the next bit time
      if (tx_byte & 1) {
        TXCCTL = (TXCCTL & ~OUTMOD_7) | OUTMOD_1; // Set the output mode to Set (output 1 for the next bit)
      } else {
        TXCCTL = (TXCCTL & ~OUTMOD_7) | OUTMOD_5; // Set the output mode to Reset (output 0 for the next bit)
      }
      // Ditch the this bit
      tx_byte >>= 1;
      tx_bit_count--;

    } else {
      // Try fetching the next byte to TX
      if (fifo_try_get(&tx_softuart_fifo, &c, 1)) {
        // Set the current byte to send, and add stop bit and start bits
        tx_byte = (0x100 | c) << 1;
        tx_bit_count = 10;
      } else {
        // If there are no more bytes to send, stop the TX comparator
        TXCCTL &= ~CCIE;
      }
    }
  }
}
