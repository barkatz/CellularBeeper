#ifndef _ATOMIC_H
#define _ATOMIC_H

#include <msp430.h>

#include "misc.h"

/* This is based on http://mykyta.info/a/embedded/msp430/atomic_8h.html : */
/* Copyright (c) 2007 Dean Camera
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/*
* This header originated from an implementation for the AVR microcontroller. It has been ported to
* MSP430 and has identical functionality.
*
* \note The macros in this header file require the ISO/IEC 9899:1999 ("ISO C99") feature of for loop
*     variables that are declared inside the for loop itself.  For that reason, this header file can
*     only be used if the standard level of the compiler (option --std=) is set to either \c c99 or
*     \c gnu99.
*
*/

static inline __attribute__((always_inline))
unsigned int __read_status_register( void){
    unsigned int xReturn;
    __asm volatile( "MOV  R2, %0"
                : "=r"  (xReturn)); // output parameters
    return xReturn;
}

static __inline__ byte __iSeiRetVal(void){
    __enable_interrupt();
    return 1;
}

static __inline__ byte __iCliRetVal(void){
    __disable_interrupt();
    return 1;
}

static __inline__ void __iSeiParam(const byte *__s){
    __enable_interrupt();
    __asm__ volatile ("" ::: "memory");
    (void)__s;
}

static __inline__ void __iCliParam(const byte *__s){
    __disable_interrupt();
    __asm__ volatile ("" ::: "memory");
    (void)__s;
}

static __inline__ void __iRestore(const byte *__s){
    if((*__s) & GIE){
        __enable_interrupt();
    }else{
        __disable_interrupt();
    }
    __asm__ volatile ("" ::: "memory");
}

/*
 * Creates a block of code that is guaranteed to be executed atomically. Upon entering the block the
 * GIE flag in the status register (SR) is disabled, and re-enabled upon exiting the block from any
 * exit path.
 *
 * Two possible macro parameters are permitted, ATOMIC_RESTORESTATE and ATOMIC_FORCEON.
 */
#define ATOMIC_BLOCK(type) for ( type, __ToDo = __iCliRetVal(); \
                           __ToDo ; __ToDo = 0 )


/*
 * This is a possible parameter for ATOMIC_BLOCK. When used, it will cause the ATOMIC_BLOCK to
 * restore the previous state of the status register, saved before the GIE flag bit was disabled.
 * The net effect of this is to make the ATOMIC_BLOCK's contents guaranteed atomic, without changing
 * the state of the GIE flag when execution of the block completes.
 */
#define ATOMIC_RESTORESTATE byte sreg_save \
    __attribute__((__cleanup__(__iRestore))) = __read_status_register()

/*
 * This is a possible parameter for ATOMIC_BLOCK. When used, it will cause the ATOMIC_BLOCK to force
 * the state of the status register on exit, enabling the GIE flag bit.
 *
 * Care should be taken that ATOMIC_FORCEON is only used when it is known that interrupts are
 * enabled before the block's execution or when the side effects of enabling global interrupts at
 * the block's completion are known and understood.
 */
#define ATOMIC_FORCEON byte sreg_save \
    __attribute__((__cleanup__(__iSeiParam))) = 0

/*
 * Creates a block of code that is executed non-atomically. Upon entering the block the GIE flag in
 * the status register is enabled, and disabled upon exiting the block from any exit path. This is
 * useful when nested inside ATOMIC_BLOCK sections, allowing for non-atomic execution of small
 * blocks of code while maintaining the atomic access of the other sections of the parent
 * ATOMIC_BLOCK.
 *
 * Two possible macro parameters are permitted, NONATOMIC_RESTORESTATE and NONATOMIC_FORCEOFF.
 */
#define NONATOMIC_BLOCK(type) for ( type, __ToDo = __iSeiRetVal(); \
                              __ToDo ;  __ToDo = 0 )

/*
 * This is a possible parameter for NONATOMIC_BLOCK. When used, it will cause the NONATOMIC_BLOCK to
 * restore the previous state of the status register, saved before the GIE flag bit was enabled. The
 * net effect of this is to make the NONATOMIC_BLOCK's contents guaranteed non-atomic, without
 * changing the state of the GIE flag when execution of the block completes.
 */
#define NONATOMIC_RESTORESTATE byte sreg_save \
    __attribute__((__cleanup__(__iRestore))) = __read_status_register()

/*
 * This is a possible parameter for NONATOMIC_BLOCK. When used, it will cause the NONATOMIC_BLOCK to
 * force the state of the status register on exit, disabling the GIE flag bit.
 *
 * Care should be taken that NONATOMIC_FORCEOFF is only used when it is known that interrupts are
 * disabled before the block's execution or when the side effects of disabling global interrupts at
 * the block's completion are known and understood.
 */
#define NONATOMIC_FORCEOFF byte sreg_save \
    __attribute__((__cleanup__(__iCliParam))) = 0

/* End of stuff from http://mykyta.info/a/embedded/msp430/atomic_8h.html */


#define ATOMIC_INC(v) do { \
                        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { \
                          (v)++; \
                        } \
                      } while(0)
#define ATOMIC_DEC(v) do { \
                        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { \
                          (v)--; \
                        } \
                      } while(0)
#define ATOMIC_SUB(v,c) do { \
                        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { \
                          (v) -= (c); \
                        } \
                      } while(0)

#endif // _ATOMIC_H
