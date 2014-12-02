#ifndef _MISC_H
#define _MISC_H

typedef unsigned char byte;
typedef unsigned int  word;
typedef unsigned long dword;
typedef char          sbyte;
typedef int           sword;
typedef long          sdword;

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) (!MIN(x,y))

#define _SET_PORT_BIT(PORT, TYPE, BIT) (PORT ## TYPE |= BIT)
#define _CLR_PORT_BIT(PORT, TYPE, BIT) (PORT ## TYPE &= ~BIT)
#define SET_PORT_BIT(PORT, TYPE, BIT) _SET_PORT_BIT(PORT, TYPE, BIT)
#define CLR_PORT_BIT(PORT, TYPE, BIT) _CLR_PORT_BIT(PORT, TYPE, BIT)

#define _READ_BIT(PORT, TYPE, BIT) (PORT ## TYPE & BIT)
#define READ_BIT(PORT, TYPE, BIT)  _READ_BIT(PORT, TYPE, BIT) 
#define _WRITE_BIT(PORT, TYPE, BIT) (PORT ## TYPE |= BIT)
#define WRITE_BIT(PORT, TYPE, BIT)  _WRITE_BIT(PORT, TYPE, BIT) 

#endif // _MISC_H