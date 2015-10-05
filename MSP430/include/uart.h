#ifndef _UART_H
#define _UART_H

#include "misc.h"

typedef enum uart_clock_source_t {
	UART_SRC_UCLK  = 0,
	UART_SRC_ACLK  = 1,
	UART_SRC_SMCLK = 2
} uart_clock_source_t;

int uart_init(uart_clock_source_t clk_src, word clk_rescaler, byte first_modulation, byte second_modulation);
void uart_putc(byte c);
void uart_puts(char *msg);
void uart_write(byte *buf, byte count);
byte uart_getc(byte *c);
byte uart_read(byte *buf, byte count);

void USCI_A0_ISR(void);

#endif // _UART_H
