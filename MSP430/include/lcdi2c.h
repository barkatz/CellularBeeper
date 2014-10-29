#ifndef _LCD_H
#define _LCD_H

#include "utils.h"

/*
Define LCD pins layout.
*/
#define 	DB4_PORT	 		P1
#define  	DB4_BIT		 		BIT0
#define 	DB5_PORT	 		P1
#define  	DB5_BIT		 		BIT1
#define 	DB6_PORT	 		P1
#define  	DB6_BIT		 		BIT2
#define 	DB7_PORT	 		P1
#define  	DB7_BIT		 		BIT4


#define ENABLE_PORT 			P2
#define ENABLE_BIT 				BIT5

#define READ_WRITE_PORT 		P2
#define READ_WRITE_BIT 			BIT4

#define REGISTER_SELECT_PORT 	P2
#define REGISTER_SELECT_BIT		BIT3


/*
Needed for mili sleep.
*/
#define CYCLES_PER_MSEC ((1*1000000)/1000)


int lcd_init();
void lcd_putc(char c);
void lcd_puts(char *msg);
void lcd_set_pos(uint8_t x, uint8_t y);
void lcd_clear();
void lcd_return_home();


#endif // _LCD_H