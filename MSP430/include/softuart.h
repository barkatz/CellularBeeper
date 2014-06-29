#ifndef _SOFTUART_H
#define _SOFTUART_H

#include "misc.h"

typedef enum softuart_clock_source_t {
	SOFTUART_SRC_TACLK = 0,
	SOFTUART_SRC_ACLK  = 1,
	SOFTUART_SRC_SMCLK = 2,
	SOFTUART_SRC_INCLK = 3
} softuart_clock_source_t;

void softuart_init(softuart_clock_source_t src, word _bit_time);

void softuart_puts(byte *msg);
void softuart_write(byte *buf, byte count);
byte softuart_getc(byte *c);
byte softuart_read(byte *buf, byte count);


#endif // _SOFTUART_H