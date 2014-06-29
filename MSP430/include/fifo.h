#ifndef _FIFO_H
#define _FIFO_H

#include "misc.h"

#define FIFO_BUF_SIZE 64 /* this must be <= 2**8 */

typedef struct uart_queue {
  byte buf[FIFO_BUF_SIZE];
  byte read_idx;
  byte write_idx;
  byte byte_count;

  byte debug;
} FIFO;

void fifo_init(FIFO *q);
byte fifo_try_put(FIFO *q, byte c, byte is_interrupt);
byte fifo_try_get(FIFO *q, byte *c, byte is_interrupt);

#endif // _FIFO_H