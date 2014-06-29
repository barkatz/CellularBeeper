#ifndef _UART_H
#define _UART_H

#include "misc.h"

typedef enum uart_clock_source_t {
	UART_SRC_UCLK  = 0,
	UART_SRC_ACLK  = 1,
	UART_SRC_SMCLK = 2
} uart_clock_source_t;

int uart_init(uart_clock_source_t clk_src, word BR, byte MCTL);
void uart_putc(byte c);
void uart_puts(byte *msg);
void uart_write(byte *buf, byte count);
byte uart_getc();
byte uart_read(byte *buf, byte count);

#endif // _UART_H