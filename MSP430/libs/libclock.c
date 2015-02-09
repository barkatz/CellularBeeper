#include <msp430.h>
#include <clock.h>

dword clock_speed;

void clock_init(clock_speed_t speed) {
  __bis_SR_register(SCG0);    // Disable the FLL control loop

  UCSCTL0 = 0x0000;           // Set lowest possible DCOx, MODx

  switch (speed) {
    case CLKSPEED_16MHZ:
      UCSCTL1 = DCORSEL_5;
      UCSCTL2 = FLLD_1 | 487;
      clock_speed = 16000000;
      break;
    case CLKSPEED_1MHZ:
      UCSCTL1 = DCORSEL_1;
      UCSCTL2 = FLLD_2 | 60;
      clock_speed = 1000000;
      break;
  }

  __bic_SR_register(SCG0);    // Enable the FLL control loop

  // Worst-case settling time for the DCO when the DCO range bits have been
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  // UG for optimization.
  dword settleTime = clock_speed/32768;
  while (settleTime--)
    __delay_cycles(32*32);
}

void ms_sleep(dword msec) {
  dword kcycles_to_sleep = msec*(clock_speed/1000000);

  dword i=0;
  for (i=0; i<kcycles_to_sleep; i++) {
    __delay_cycles(1000);
  }
}
