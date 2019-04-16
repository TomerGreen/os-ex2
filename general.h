//
// Created by Tomer Greenberg on 4/10/19.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>


#ifndef OS_EX2_GENERAL_H
#define OS_EX2_GENERAL_H

// Thread state enumeration.
enum State {READY, RUNNING, BLOCKED, TERMINATED};


#define SUCCESS_CODE 0
#define FAIL_CODE -1
#define SYS_ERROR_MSG "system error: "
#define LIB_ERROR_MSG "thread library error: "


#endif //OS_EX2_GENERAL_H
