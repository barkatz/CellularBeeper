#ifndef _SIM900_H
#define _SIM900_H

#include "misc.h"

int sim900_cmd(char* cmd);
void sim900_clear_response_buf();
int sim900_cmd_and_verify(char *cmd, char *expected_response);
int sim900_init(void);


#endif // _SIM900_H 