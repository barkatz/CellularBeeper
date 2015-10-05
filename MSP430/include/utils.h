#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include "misc.h"

//#define TRACE_SOFTUART
//#define TRACE_UART
#define TRACE_LCD

#if defined(TRACE_SOFTUART)
    #include <softuart.h>
    #define trace_clear() 0
    #define trace_putc  softuart_putc
    #define trace_puts  softuart_puts
#elif defined(TRACE_UART)
    #include <uart.h>
    #define trace_clear() 0
    #define trace_putc  uart_putc
    #define trace_puts  uart_puts
#elif defined(TRACE_LCD)
    #include <lcdi2c.h>
    #define trace_clear lcdi2c_clear
    #define trace_putc  lcdi2c_putc
    #define trace_puts  lcdi2c_puts
#else
    #define trace_clear() 0
    #define trace_putc (void)(x)
    #define trace_puts (void)(x)
#endif

#ifndef NDEBUG
  #define TRACE(fmt, args...) \
        trace_printf("[*] %s: " fmt "\r\n", __FUNCTION__, ##args)
#else
  #define TRACE(fmt, args...) ((void)0)
#endif

#define ASSERT(expr) ((void)((expr) ? 0 : \
                     (trace_clear(), \
                     trace_printf("[!] %s: Assertion failed, file %s, " \
                                  "line %u\r\n", __FUNCTION__, \
                                  __FILE__, __LINE__), \
                      abort () )))

#define ERROR(fmt, args...) \
        trace_printf("[-] %s: " fmt "\r\n", __FUNCTION__, ##args)

void trace_printf(char *format, ...);
byte startswith(char *s, char *prefix);

#endif // _UTILS_H
