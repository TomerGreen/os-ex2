//
// Created by Tomer Greenberg on 4/9/19.
//
#include "uthreads.h"
#include <stdlib.h>
#include <unistd.h>
#include <iostream>


void f1()
{
    int i = 0;
    while (true)
    {
        if (i%1000000 == 0)
        {
            print_thread_status();
        }
        i++;
    }
}


void print(int a)
{
    std::cout << a << '\n';
}

int test_spawn_and_terminate()
{
    print(uthread_init(3000));
    print(uthread_spawn(f1));
    print(uthread_spawn(f1));
    print(uthread_terminate(1));
    print(uthread_spawn(f1));
    print(uthread_terminate(3));
    return 0;
}


int test_block_and_resume()
{
    uthread_init(3000);
    print_thread_status();
    uthread_spawn(f1);
    uthread_spawn(f1);
    print_thread_status();
    uthread_block(1);
    print_thread_status();
    uthread_block(2);
    print_thread_status();
    uthread_resume(2);
    print_thread_status();
    uthread_resume(1);
    print_thread_status();
    uthread_terminate(2);
    print_thread_status();
    uthread_terminate(0);
    return 0;
}


int test_basic_timer_use()
{
    uthread_init(3000);
    print_thread_status();
    uthread_spawn(f1);
    int i=0;
    while(true)
    {
        if (i%1000000 == 0)
        {
            print_thread_status();
        }
        i++;
    }
    return 0;
}


int main()
{
    test_basic_timer_use();
}