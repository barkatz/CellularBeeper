#include <msp430.h>
#include "misc.h"
#include "i2c.h"

#define UCCTL0  UCB0CTL0
#define UCCTL1  UCB0CTL1
#define UCMODE  UCMODEB0
#define UCI2CSA UCB0I2CSA
#define UCBR0   UCB0BR0
#define UCBR1   UCB0BR1
#define UCIFG   UC0IFG
#define UCTXIFG UCB0TXIFG
#define UCTXBUF UCB0TXBUF

void i2c_init(byte slave_addr) {
  // TODO set the relevant pin modes

  UCCTL0 |= UCSWRST;           // Put the USCI in reset
  UCCTL0  = UCSYNC | UCMODE_3; // 7-bit addressing, single master in synchronous I2C mode
  UCI2CSA = slave_addr;        // Set the slave address
  UCCTL1  = UCTR | UCSSEL_2;   // Use SMCLK, Transmitter
  UCBR0   = 160;               // fSCL = SMCLK/12 = ~100kHz
  UCBR1   = 0;
  UCCTL0 &= ~UCSWRST;          // Clear the USCI reset
}

void i2c_write(byte data) {
  UCCTL1 |= UCTXSTT;          // Send a START condition
  while (!(UCIFG & UCTXIFG)); // Wait until the transmit buffer is ready (we can queue the byte to send)
  UCTXBUF = data;             // Send the data
  while (!(UCIFG & UCTXIFG)); // Wait until the transmit buffer is ready (we can prepare for a STOP)
  UCCTL1 |= UCTXSTP;          // Send a STOP condition
}