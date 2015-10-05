#include <msp430.h>
#include <clock.h>

#ifdef USE_DRIVERLIB
#include <driverlib.h>
#endif

dword clock_speed;

void clock_init(clock_speed_t speed) {
  // Turn on XT1 crystal
#ifndef USE_DRIVERLIB
  P5SEL |= BIT4;

  UCSCTL6 |= XCAP_3;                          // Internal load cap of 12pF

  do {                                        // Wait until XT1 stabilizes
    UCSCTL7 &= ~(XT1LFOFFG + DCOFFG);         // Clear XT1 fault flag
                                              //
                                              // The DCO fault flag is also cleared,
                                              // since it is supposed to be faulty at startup
                                              // and thus OFIFG will continue to be set unless
                                              // we clear it...
    SFRIFG1 &= ~OFIFG;                        // Clear oscillator fault flag
  }  while (SFRIFG1 & OFIFG);                 // Re-test oscillator fault flag

  UCSCTL6 &= ~XT1DRIVE_3;                     // Decrease XT1 drive as it is stabilized
#else
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN4);
  UCS_turnOnLFXT1(XT1DRIVE_0, UCS_XCAP_3);
#endif

  // Set the FLL reference select to XT1 (external 32KHz clock),
  // and set the FLL reference divider to 1
#ifndef USE_DRIVERLIB
  UCSCTL3 = SELREF__XT1CLK | FLLREFDIV_0;

  // Set the MCLK source to DCOCLKDIV
  UCSCTL4 = (UCSCTL4 & ~SELM_7) | SELM__DCOCLKDIV;

  // Set the SMCLK source to DCOCLKDIV
  UCSCTL4 = (UCSCTL4 & ~SELS_7) | SELS__DCOCLKDIV;
#else
  UCS_initClockSignal(
       UCS_FLLREF,
       UCS_XT1CLK_SELECT,
       UCS_CLOCK_DIVIDER_1);
#endif

  // Initialize the FLL
#ifndef USE_DRIVERLIB
  __bis_SR_register(SCG0);    // Disable the FLL control loop

  UCSCTL0 = 0x0000;           // Set lowest possible DCOx, MODx

  uint16_t ratio;
  switch (speed) {
    case CLKSPEED_16MHZ:
      clock_speed = 16000000;

      ratio = clock_speed/XT1_FREQ;

      UCSCTL1 = DCORSEL_6;
      UCSCTL2 = FLLD_1 | (ratio - 1); // FLLD doesn't really matter, since we feed MCLK and SMCLK off DCOCLKDIV and not DCOCLK
      break;
    case CLKSPEED_1MHZ:
      clock_speed = 1000000;

      ratio = clock_speed/XT1_FREQ;

      UCSCTL1 = DCORSEL_2;
      UCSCTL2 = FLLD_1 | (ratio - 1); // FLLD doesn't really matter, since we feed MCLK and SMCLK off DCOCLKDIV and not DCOCLK
      break;
  }

  __bic_SR_register(SCG0);    // Enable the FLL control loop

  do {                                      // Wait until DCO stabilizes
    UCSCTL7 &= ~(DCOFFG);                     // Clear DCO fault flag
    SFRIFG1 &= ~OFIFG;                        // Clear oscillator fault flag
  }  while (SFRIFG1 & OFIFG);                 // Re-test oscillator fault flag

  // Worst-case settling time for the DCO when the DCO range bits have been
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  // UG for optimization.
  while (ratio--)
    __delay_cycles(32*32);
#else
  switch (speed) {
    case CLKSPEED_16MHZ:
      clock_speed = 16000000;
      break;
    case CLKSPEED_1MHZ:
      clock_speed = 1000000;
      break;
  }

  UCS_initFLLSettle(clock_speed/1000, clock_speed/XT1_FREQ);
#endif
}

void ms_sleep(dword msec) {
  dword kcycles_to_sleep = msec*(clock_speed/1000000);

  dword i=0;
  for (i=0; i<kcycles_to_sleep; i++) {
    __delay_cycles(1000);
  }
}
