//
// Created by Tomer Greenberg on 4/6/19.
//

#include "uthreads.h"
#include "thread.h"
#include "general.h"
#include <queue>
#include <signal.h>


#define ENV_SAVE_CODE 0
#define ENV_LOAD_CODE 1


Thread *threads[MAX_THREAD_NUM];
int quantum_length;    // The number of microseconds in each quantum.
std::deque<int> readyQueue;   // A queue of ready thread ID's.
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
 * Tries to remove a thread ID from the ready queue.
 * @param tid - the thread ID to be removed.
 * @return 0 upon success, -1 upon failure.
 */
int remove_from_ready_queue(int tid)
{
    for (unsigned int i=0; i<readyQueue.size(); i++)
    {
        if (readyQueue[i] == tid)
        {
            readyQueue.erase(readyQueue.begin()+i);
            return SUCCESS_CODE;
        }
    }
    return FAIL_CODE;
}

/**
 * Signals the thread at the top of the ready list to run. This function is not responsible
 * to save the env or modify the data for the currently running thread.
 */
void switch_threads()
{
    int next;
    if (readyQueue.empty())
    {
        next = runningThread;
    }
    // If there are threads in the ready queue
    else
    {
        next = readyQueue[0];
        readyQueue.pop_front();
    }
    runningThread = next;
    threads[runningThread]->setState(RUNNING);
    siglongjmp(*(threads[runningThread]->getEnv()), ENV_LOAD_CODE);
}


//TODO Delete this debugging function when done.
void print_thread_status()
{
    std::cout << "thread states: {";
    for (int i=0; i<10; i++)
        {
            if (threads[i] != nullptr)
            {
                std::cout << i << ":";
                State state = threads[i]->getState();
                switch(state)
                {
                    case(READY) :
                        std::cout << "READY";
                        break;
                    case(RUNNING) :
                        std::cout << "RUNNING";
                        break;
                    case(BLOCKED) :
                        std::cout << "BLOCKED";
                    default :
                        break;
                }
                std::cout << ", ";
            }
        }
    std::cout << "}\n";
    std::cout << "ready queue: {";
    for(unsigned int i=0; i<readyQueue.size(); i++)
        {
            std::cout << readyQueue[i] << ", ";
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
    // Initiates main thread. Every other thread in threads array is initiated to nullptr.
    Thread* main_thread = new Thread(0);
    threads[0] = main_thread;
    main_thread->setState(RUNNING);
    runningThread = 0;
    return SUCCESS_CODE;
}


int uthread_spawn(void (*f)(void))
{
    for (int i=0; i<MAX_THREAD_NUM; i++)
    {
        if (threads[i] == nullptr)
        {
            threads[i] = new Thread(i, f);
            readyQueue.push_back(i);
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
        remove_from_ready_queue(tid);
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
        remove_from_ready_queue(tid);
        if (tid == runningThread)
        {
            // If tid is the running thread, its env hasn't been saved previously.
            int ret_val = sigsetjmp(*(threads[tid]->getEnv()), 1);
            // If the thread env was just saved.
            if (ret_val == ENV_SAVE_CODE)
            {
                switch_threads();
            }
        }
    }
    return SUCCESS_CODE;
}


int uthread_resume(int tid)
{
    // If tid invalid and existing.
    if (!is_tid_valid(tid))
    {
        return FAIL_CODE;
    }
    // If thread is blocked.
    else if (threads[tid]->getState() == BLOCKED)
    {
        threads[tid]->setState(READY);
        readyQueue.push_back(tid);
    }
    //TODO make sure resuming ready/running thread should return 0.
    return SUCCESS_CODE;
}