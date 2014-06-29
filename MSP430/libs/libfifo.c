#include "misc.h"
#include "fifo.h"
#include "atomic.h"

void fifo_init(FIFO *f) {
  f->read_idx = f->write_idx = f->byte_count = 0;
}

inline byte fifo_try_put(FIFO *q, byte c, byte is_interrupt) {
  if (q->byte_count >= FIFO_BUF_SIZE) {
    return 0;
  }
    
  q->buf[q->write_idx] = c;
  q->write_idx = (q->write_idx+1)%FIFO_BUF_SIZE;

  if (is_interrupt)
      q->byte_count++;
  else
    ATOMIC_INC(q->byte_count);

  return 1;
}

inline byte fifo_try_get(FIFO *q, byte *c, byte is_interrupt) {
  if (q->byte_count == 0) {
  	return 0;
  }

  *c = q->buf[q->read_idx];
  q->read_idx = (q->read_idx+1)%FIFO_BUF_SIZE;
  
  if (is_interrupt)
      q->byte_count--;
  else
    ATOMIC_DEC(q->byte_count);

  return 1;
}