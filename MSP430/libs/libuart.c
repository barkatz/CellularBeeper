#include <msp430.h>
#include "uart.h"
#include "misc.h"
#include "fifo.h"

static inline byte _uart_try_putc(byte c);
static inline void _uart_putc(byte c);

static FIFO tx_uart_fifo;
static FIFO rx_uart_fifo;

int uart_init(uart_clock_source_t clk_src, word BR, byte MCTL) {
  
  // USCI_A0 setup for the given parameters
  
  UCA0CTL1 |= UCSWRST;            // Stop USCI

  P1SEL     = BIT1 + BIT2;        // set P1.1 to RX and P1.2 to TX
  P1SEL2    = BIT1 + BIT2;        //
  UCA0CTL1 |= clk_src << 6;       // Choose CLK source

  UCA0BR0   = BR & 0xff;
  UCA0BR1   = BR >> 8;
  UCA0MCTL  = MCTL;

  UCA0CTL1 &= ~UCSWRST;           // Start USCI

  fifo_init(&tx_uart_fifo);
  fifo_init(&rx_uart_fifo);
  IE2      |= UCA0RXIE;           // Enable RX interrupt

  return 0;
}

static inline byte _uart_try_putc(byte c) {
  if (fifo_try_put(&tx_uart_fifo, c, 0)) {
    IE2 |= UCA0TXIE; // enable TX interrupt
    return 1;
  } else {
    return 0;
  }
}

static inline void _uart_putc(byte c) {
  while (!_uart_try_putc(c));
}

void uart_putc(byte c) {
  _uart_putc(c);
}

void uart_puts(byte *msg) {
  while (*msg)
    _uart_putc(*msg++);
}

void uart_write(byte *buf, byte count) {
  for (; count > 0; count--)
    _uart_putc(*(buf++));
}

byte uart_read(byte *buf, byte count) {
  byte *orig_buf = buf;

  while(fifo_try_get(&rx_uart_fifo, buf++, 0));
  
  return (byte)(buf-orig_buf-1);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR() {
  if (IFG2 & UCA0RXIFG) {
    // the received byte is in UCA0RXBUF
    fifo_try_put(&rx_uart_fifo, UCA0RXBUF, 1);
  }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR() {
  byte byte_ready, c;
  if (IFG2 & UCA0TXIFG) {
    byte_ready = fifo_try_get(&tx_uart_fifo, &c, 1);
    if (byte_ready) {
      // the byte to transmit is to be put in UCA0TXBUF
      UCA0TXBUF = c;
    } else {
      IE2  &= ~UCA0TXIE; // disable TX interrupt
    }
  }
}