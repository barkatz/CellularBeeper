#ifndef _LCD_H
#define _LCD_H

#include "utils.h"

#define LCD_I2C_ADDR        0x27

#define BACKLIGHT_BIT       3
#define ENABLE_BIT 			   	2
#define READ_WRITE_BIT 		 	1
#define REGISTER_SELECT_BIT 0

/*
Needed for mili sleep.
*/
#define CYCLES_PER_MSEC ((1*1000000)/1000)

int  lcdi2c_init();
void lcdi2c_putc(char c);
void lcdi2c_puts(char *msg);
void lcdi2c_set_pos(uint8_t x, uint8_t y);
void lcdi2c_clear();
void lcdi2c_return_home();


#endif // _LCD_H