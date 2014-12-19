#ifndef _SIM900_H
#define _SIM900_H

typedef enum sim900_status_t {
  SIM900STS_READY,
} sim900_status_t;

extern sim900_status_t sim900_status;

void sim900_init();
void sim900_do_work();

#endif // _SIM900_H