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
  P1SEL  |= BIT6 + BIT7;       // Set P1.6 and P1.7 to USCI_B0
  P1SEL2 |= BIT6 + BIT7;       //

  UCCTL1 |= UCSWRST;            // Put the USCI in reset
  UCCTL0  = UCMST | UCSYNC | UCMODE_3; // 7-bit addressing, single master in synchronous I2C mode
  UCCTL1  = UCSSEL_2 | UCSWRST; // Use SMCLK
  UCBR0   = 12;                 // fSCL = SMCLK/12 = ~100kHz
  UCBR1   = 0;                  //
  UCI2CSA = slave_addr;         // Set the slave address
  UCCTL1 &= ~UCSWRST;           // Clear the USCI reset
}

void i2c_write(byte data) {
  UCCTL1 |= UCTR + UCTXSTT;   // Send a START condition
  while (!(UCIFG & UCTXIFG)); // Wait until the transmit buffer is ready (the START condition has been
                              // sent)
  UCTXBUF = data;             // Send the data
  while (!(UCIFG & UCTXIFG)); // Wait until the transmit buffer is ready (the data has been sent)
  UCCTL1 |= UCTXSTP;          // Send a STOP condition
  while (UCCTL1 & UCTXSTP);   // Wait until the transmit buffer is ready (previous STOP condition have
                              // been sent)
}