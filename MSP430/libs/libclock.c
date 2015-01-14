#include <msp430.h>
#include <clock.h>

dword clock_speed;

void clock_init(clock_speed_t speed) {
  switch (speed) {
    case CLKSPEED_16MHZ:
      BCSCTL1 = CALBC1_16MHZ;
      DCOCTL  = CALDCO_16MHZ;
      clock_speed = 16000000;
      break;
    case CLKSPEED_12MHZ:
      BCSCTL1 = CALBC1_12MHZ;
      DCOCTL  = CALDCO_12MHZ;
      clock_speed = 12000000;
      break;
    case CLKSPEED_8MHZ:
      BCSCTL1 = CALBC1_8MHZ;
      DCOCTL  = CALDCO_8MHZ;
      clock_speed = 8000000;
      break;
    case CLKSPEED_1MHZ:
      BCSCTL1 = CALBC1_1MHZ;
      DCOCTL  = CALDCO_1MHZ;
      clock_speed = 1000000;
      break;
  }  
}

void ms_sleep(dword msec) {
  dword kcycles_to_sleep = msec*(clock_speed/1000000);

  dword i=0;
  for (i=0; i<kcycles_to_sleep; i++) {
    __delay_cycles(1000);
  }
}
