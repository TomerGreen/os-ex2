//
// Created by Tomer Greenberg on 4/10/19.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>


#ifndef OS_EX2_GENERAL_H
#define OS_EX2_GENERAL_H

// Thread state enumeration.
enum State {READY, RUNNING, BLOCKED, TERMINATED};


#define SUCCESS_CODE 0
#define FAIL_CODE -1
#define SYS_ERROR_MSG "system error: "
#define LIB_ERROR_MSG "thread library error: "

// Supplied code for address handling.
#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
		"rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#endif

#endif //OS_EX2_GENERAL_H
