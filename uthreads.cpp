//
// Created by Tomer Greenberg on 4/6/19.
//

#include "uthreads.h"
#include "Thread.h"
#include "general.h"
#include <queue>
#include <signal.h>


#define CONTEXT_SAVE_CODE 0
#define CONTEXT_LOAD_CODE 1


Thread *threads[MAX_THREAD_NUM];
int quantum_length;    // The number of microseconds in each quantum.
std::queue<int> readyList;   // A queue of ready thread ID's.
int runningThread;  // The ID of the currently running thread.


//////////////////////////////////
//////// HELPER FUNCTIONS ////////
//////////////////////////////////


/**
 * Checks if given thread exists and is in the valid range.
 */
bool is_tid_valid(int tid)
{
    // If the provided ID is invalid
    if (tid < 0 || tid >= MAX_THREAD_NUM)
    {
        std::cerr << LIB_ERROR_MSG << "invalid thread ID provided.\n";
        return false;
    }
    // If thread doesn't exist.
    else if (threads[tid] == nullptr)
    {
        std::cerr << LIB_ERROR_MSG << "could not find thread with the given ID.\n";
        return false;
    }
    return true;
}

/**
 * Tries to remove a thread ID from a vector.
 * @param list - a pointer to the vector.
 * @param tid - the thread ID to be removed.
 * @return 0 upon success, -1 upon failure.
 */
int remove_from_ready_queue(int tid)
{
    int curr;
    while (readyList.)
    for (int i=0; i<list->size(); i++)
    {
        if ((*list)[i] == tid)
        {
            list->erase(list->begin()+i);
            return SUCCESS_CODE;
        }
    }
    return FAIL_CODE;
}

/**
 * Signals the thread at the top of the ready list to run. This function is not responsible
 * to save the context or modify the data for the currently running thread.
 */
void switch_threads()
{
    if (readyList.empty())
    {
        int next = runningThread;
    }
    else
    {
        int next = readyList.erase(readyList.begin());     // Equivalent to pop().
    }
    runningThread = next;
    threads[runningThread]->setState(RUNNING);
    siglongjmp(threads[runningThread]->getContext(), CONTEXT_LOAD_CODE);
}


void print_thread_status
{
    std::cout << "thread states: {";
    for (int i=0; i<10; i++)
        {
            if (threads[i] != nullptr)
            {
                std::cout << i << ": " << threads[i]->getState() << ", ";
            }
        }
    std::cout << "}\n";
    std::cout << "ready llist: {";
    for(auto t=readList.begin(); t!=v.end(); ++t)
        {
            std::cout << *t << ", ";
        }
    std::cout << "}\n";
}


///////////////////////////////////
//////// LIBRARY FUNCTIONS ////////
///////////////////////////////////

int uthread_init(int quantum_usecs)
{
    if (quantum_usecs <= 0)
    {
        std::cerr << LIB_ERROR_MSG << "parameter quantum_usecs must be positive\n";
        return FAIL_CODE;
    }
    quantum_length = quantum_usecs;
    // Initiates main thread. Every other thread is initiated to nullptr.
    uthread_spawn(nullptr);
    return SUCCESS_CODE;
}


int uthread_spawn(void (*f)(void))
{
    for (int i=0; i<MAX_THREAD_NUM; i++)
    {
        if (threads[i] == nullptr)
        {
            threads[i] = new Thread(i, f);
            readyList.push_back(i);
            return i;
        }
    }
    // Maximum number of threads reached.
    return FAIL_CODE;
}


int uthread_terminate(int tid)
{
    if (!is_tid_valid(tid))
    {
        return FAIL_CODE;
    }
    // If the provided ID is the main thread
    else if (tid == 0)
    {
        for (int i=0; i<MAX_THREAD_NUM; i++)
        {
            if (threads[i] != nullptr)
            {
                delete threads[i];
                threads[i] = nullptr;
            }
        }
        exit(SUCCESS_CODE);
    }
    // If this is a valid thread.
    else
    {
        delete threads[tid];
        threads[tid] = nullptr;
        removeFromList(&readyList, tid);
        // If the running thread is being terminated.
        if (tid == runningThread)
        {
            switch_threads();
        }
    }
    return SUCCESS_CODE;
}


int uthread_block(int tid)
{
    if (!is_tid_valid(tid))
    {
        return FAIL_CODE;
    }
    else if (tid == 0)
    {
        std::cerr << LIB_ERROR_MSG << "main thread cannot be blocked.\n";
        return FAIL_CODE;
    }
    else
    {
        threads[tid]->setState(BLOCKED);
        removeFromList(&readyList, tid);
        if (tid == runningThread)
        {
            // If tid is the running thread, its context hasn't been saved previously.
            int ret_val = sigsetjmp(threads[tid], 1);
            // If the thread context was just saved.
            if (ret_val == CONTEXT_SAVE_CODE)
            {
                switch_threads();
            }
        }
    }
    return SUCCESS_CODE;
}


int uthread_resume(int tid)
{
    if (!is_tid_valid(tid))
    {
        return FAIL_CODE;
    }
    else if (threads[tid]->getState() == BLOCKED)
    {
        threads[tid]->setState(READY);
        readyList.push_back(tid);
    }
    //TODO make sure resuming ready/running thread should return 0.
    return SUCCESS_CODE;
}