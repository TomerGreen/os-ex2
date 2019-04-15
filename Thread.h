//
// Created by Tomer Greenberg on 4/6/19.
//

#ifndef THREAD_H
#define THREAD_H
#include <setjmp.h>
#include <signal.h>
#include "general.h"
#include "uthreads.h"


class Thread
{
    private:
        int id;
        State state;
        void (*f)(void);
        char *stack;
        sigjmp_buf env;

    public:

        /**
         * Constructor for a thread.
         */
        Thread(int id, void (*f)(void));

        /**
         * Constructor for the main thread.
         */
        Thread(int tid);

        /**
         * Destructor for a thread.
         */
        ~Thread();

        /**
         * Getter for ID.
         */
        int getId() const;

        /**
         * Getter for state.
         */
        State getState() const;

        /**
         * Setter for state.
         */
        void setState(State state);

        /**
         * Getter for stack pointer.
         */
        char *getStack();

        /**
         * Getter for thread env.
         */
        sigjmp_buf *getEnv();
};


#endif
