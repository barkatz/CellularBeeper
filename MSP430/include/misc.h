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

#endif // _MISC_H