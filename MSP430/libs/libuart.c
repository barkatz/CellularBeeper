#include <msp430.h>
#include <uart.h>
#include <misc.h>
#include <fifo.h>

static inline byte _uart_try_putc(byte c);
static inline void _uart_putc(byte c);


static FIFO tx_uart_fifo;
static FIFO rx_uart_fifo;

int uart_init(uart_clock_source_t clk_src, word clk_rescaler, byte first_modulation, byte second_modulation) {
#ifndef USE_DRIVERLIB
  P4SEL     |= BIT4 + BIT5;        // set P4.4 to TX and P4.5 to RX
#else
  GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN4);
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN5);
#endif

  // TODO make a version for USE_DRIVERLIB

  // USCI_A1 setup for the given parameters
  UCA1CTL1   = UCSWRST;            // Stop USCI

  UCA1CTL0   = 0;

  UCA1CTL1  |= clk_src << 6;       // Choose CLK source

  UCA1BR0    = clk_rescaler & 0xff;
  UCA1BR1    = clk_rescaler >> 16;

  UCA1MCTL   = first_modulation | second_modulation;

  UCA1CTL1 &= ~UCSWRST;           // Start USCI

  fifo_init(&tx_uart_fifo);
  fifo_init(&rx_uart_fifo);
  UCA1IE   |= UCRXIE;             // Enable RX interrupt

  return 0;
}

static inline byte _uart_getc(byte *c) {
  return fifo_try_get(&rx_uart_fifo, c, 0);
}

byte uart_getc(byte *c) {
  return _uart_getc(c);
}

static inline byte _uart_try_putc(byte c) {
  if (fifo_try_put(&tx_uart_fifo, c, 0)) {
    UCA1IE |= UCTXIE; // enable TX interrupt
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

void uart_puts(char *msg) {
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

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A1_VECTOR)))
#endif
void USCI_A1_ISR(void) {
  byte byte_ready, c;

  switch (UCA1IV) {
    case USCI_NONE:
      break;
    case USCI_UCRXIFG:
      fifo_try_put(&rx_uart_fifo, UCA1RXBUF, 1);
      break;
    case USCI_UCTXIFG:
      byte_ready = fifo_try_get(&tx_uart_fifo, &c, 1);
      if (byte_ready) {
        P1OUT ^= BIT0;
        // the byte to transmit is to be put in UCA1TXBUF
        UCA1TXBUF = c;
        // this clears UCTXIFG
      } else {
        P4OUT ^= ~BIT7;
        UCA1IE &= ~UCTXIE;  // disable TX interrupt
        UCA1IFG |= UCTXIFG; // generate pending TX interrupt
      }
      break;
    default:
      break;
  }
}
