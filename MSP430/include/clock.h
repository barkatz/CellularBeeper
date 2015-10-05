#ifndef _CLOCK_H
#define _CLOCK_H

#include "misc.h"

#define XT1_FREQ 32768

typedef enum clock_speed_t {
  CLKSPEED_16MHZ,
  CLKSPEED_1MHZ,
} clock_speed_t;

extern dword clock_speed;

void clock_init(clock_speed_t);
void ms_sleep(dword msec);

#endif // _CLOCK_H
