//
// Created by Tomer Greenberg on 4/6/19.
//

#include "uthreads.h"
#include "thread.h"
#include "general.h"
#include <queue>
#include <signal.h>
#include <sys/time.h>
#include <math.h>

//============================//
#define DEBUG
//============================//

#define ENV_SAVE_CODE 0
#define ENV_LOAD_CODE 1
#define SEC_TO_MICROSECS 1000000


Thread *threads[MAX_THREAD_NUM];
int quantum_length;    // The number of microseconds in each quantum.
std::deque<int> readyQueue;   // A queue of ready thread ID's.
int runningThread;  // The ID of the currently running thread.
int total_quanta = 1;   // Quantum counter for all threads in total.
sigset_t signal_set;    // Signal set used for signal masking.

// TODO - Check if these need be global.
struct sigaction sa = {0};
struct itimerval tv;     // Saves the timer interval


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
 * Blocks alarm signals.
 */
void block_timer()
{
#ifdef DEBUG
    std::cout << "blocking timer\n";
#endif
    if (sigprocmask(SIG_BLOCK, &signal_set, NULL) == FAIL_CODE)
    {
        std::cerr << SYS_ERROR_MSG << "failed to block signal set.\n";
        exit(1);
    }
}


/**
 * Stops the blocking of alarm signals.
 */
void unblock_timer()
{
#ifdef DEBUG
    std::cout << "unblocking timer\n";
#endif
    if (sigprocmask(SIG_UNBLOCK, &signal_set, NULL) == FAIL_CODE)
    {
        std::cerr << SYS_ERROR_MSG << "failed to unblock signal set.\n";
        exit(1);
    }
}


/**
 * Resets the virtual timer and unblocks the signals.
 */
void reset_timer()
{
#ifdef DEBUG
    std::cout << "resetting timer\n";
#endif
    if (setitimer(ITIMER_VIRTUAL, &tv, NULL) == FAIL_CODE)
    {
        std::cerr << SYS_ERROR_MSG << "failed to reset virtual timer.\n";
        exit(1);
    }
    unblock_timer();
}


/**
 * Initiates the virtual timer.
 */
void init_timer(int quantum_usecs)
{
    int seconds = floor(quantum_usecs/SEC_TO_MICROSECS);
    int usecs = quantum_usecs - seconds*SEC_TO_MICROSECS;
#ifdef DEBUG
    std::cout << "initiating timer with " << seconds << " secs and " << usecs << " microsecs\n";
#endif
    tv.it_value.tv_sec = seconds;
    tv.it_value.tv_usec = usecs;
    tv.it_interval.tv_sec = seconds;
    tv.it_interval.tv_usec = usecs;
    // This needs to be a separate function since virtual timer resets on self-blocking/terminating.
    reset_timer();
}


/**
 * Signals the thread at the top of the ready list to run. This function is not responsible
 * to save the env or modify the data for the currently running thread.
 */
void switch_thread()
{
#ifdef DEBUG
    std::cout << "switching threads\n";
#endif
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
    threads[runningThread]->inc_quantum_count();
    total_quanta++;
    reset_timer();
    siglongjmp(*(threads[runningThread]->getEnv()), ENV_LOAD_CODE);
}


/**
 * Handles virtual timer expiration.
 */
void timer_handler(int signum)
{
#ifdef DEBUG
    std::cout << "handling alarm signal\n";
#endif
    threads[runningThread]->setState(READY);
    readyQueue.push_back(runningThread);
    int ret_val = sigsetjmp(*(threads[runningThread]->getEnv()), 1);
    // If thread state env was just saved.
    if (ret_val == ENV_SAVE_CODE)
    {
        switch_thread();
    }
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
    // Checks input.
    if (quantum_usecs <= 0)
    {
        std::cerr << LIB_ERROR_MSG << "parameter quantum_usecs must be a positive integer.\n";
        return FAIL_CODE;
    }

    // Initiates main thread. Every other thread in threads array is auto-initiated to nullptr.
    Thread* main_thread = new Thread(0);
    threads[0] = main_thread;
    main_thread->setState(RUNNING);
    runningThread = 0;

    // Set timer_handler to handle timer signals.
    sa.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        std::cerr << SYS_ERROR_MSG << "failed to set signal action handler.\n";
        exit(1);
    }

    // Saves signal set for masking.
    if (sigemptyset(&signal_set) == FAIL_CODE)
    {
        std::cerr << SYS_ERROR_MSG << "failed to empty signal set.\n";
        exit(1);
    }
    if (sigaddset(&signal_set, SIGVTALRM) == FAIL_CODE)
    {
        std::cerr << SYS_ERROR_MSG << "failed to add signal to signal set.\n";
        exit(1);
    }

    // Initiates timer.
    init_timer(quantum_usecs);

    return SUCCESS_CODE;
}


int uthread_spawn(void (*f)(void))
{
    block_timer();
#ifdef DEBUG
    std::cout << "spawning thread\n";
#endif
    for (int i=0; i<MAX_THREAD_NUM; i++)
    {
        if (threads[i] == nullptr)
        {
            threads[i] = new Thread(i, f);
            readyQueue.push_back(i);
            unblock_timer();
            return i;
        }
    }
    // Maximum number of threads reached.
    unblock_timer();
    return FAIL_CODE;
}


int uthread_terminate(int tid)
{
    block_timer();
    if (!is_tid_valid(tid))
    {
        unblock_timer();
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
            // No need to unblock timer since it's reset.
            switch_thread();
        }
    }
    unblock_timer();
    return SUCCESS_CODE;
}


int uthread_block(int tid)
{
    block_timer();
    // Thread ID invalid or non existent.
    if (!is_tid_valid(tid))
    {
        unblock_timer();
        return FAIL_CODE;
    }
    // Trying to block the main thread.
    else if (tid == 0)
    {
        std::cerr << LIB_ERROR_MSG << "main thread cannot be blocked.\n";
        unblock_timer();
        return FAIL_CODE;
    }
    // Valid tid.
    else
    {
        threads[tid]->setState(BLOCKED);
        remove_from_ready_queue(tid);
        // Trying to block the running thread.
        if (tid == runningThread)
        {
            // If tid is the running thread, its env hasn't been saved previously.
            int ret_val = sigsetjmp(*(threads[tid]->getEnv()), 1);
            // If the thread env was just saved.
            if (ret_val == ENV_SAVE_CODE)
            {
                // Timer is reset and unblocked.
                switch_thread();
            }
        }
    }
    unblock_timer();
    // TODO - Check if a thread blocking itself should get 0 returned when it runs next.
    return SUCCESS_CODE;
}


int uthread_resume(int tid)
{
    block_timer();
    // If tid invalid and existing.
    if (!is_tid_valid(tid))
    {
        unblock_timer();
        return FAIL_CODE;
    }
    // If thread is blocked.
    else if (threads[tid]->getState() == BLOCKED)
    {
        threads[tid]->setState(READY);
        readyQueue.push_back(tid);
    }
    unblock_timer();
    //TODO make sure resuming ready/running thread should return 0.
    return SUCCESS_CODE;
}