#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include "misc.h"

#ifndef NDEBUG
  #define TRACE(fmt, args...) \
        uart_printf("[*] %s: " fmt "\r\n", __FUNCTION__, ##args)
#else
  #define TRACE(fmt, args...) ((void)0)
#endif

#define ASSERT(expr) ((void)((expr) ? 0 : \
                     (uart_printf("[!] %s: Assertion failed, file %s, \
                                  line %u\r\n", __FUNCTION__, \
                                  __FILE__, __LINE__), \
                      abort () )))

#define ERROR(fmt, args...) \
        uart_printf("[-] %s: " fmt "\r\n", __FUNCTION__, ##args)
        
void uart_printf(char *format, ...);
byte startswith(char *s, char *prefix);

#endif // _UTILS_H
