#ifndef _I2C_H
#define _I2C_H

#include "misc.h"

void i2c_init(byte slave_addr);
void i2c_write(byte data);

#endif // _I2C_H