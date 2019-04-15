#include "thread.h"


Thread::Thread(int id, void (*f)(void)): id(id), f(f)
{
    state = READY;
    stack = new char[STACK_SIZE];
    address_t sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
    address_t pc = (address_t)f;
    sigsetjmp(env, 1);
    (env->__jmpbuf)[JB_SP] = translate_address(sp);
    (env->__jmpbuf)[JB_PC] = translate_address(pc);
    if (sigemptyset(&env->__saved_mask) == FAIL_CODE)
    {
        std::cerr << SYS_ERROR_MSG << "failed to initialize signal mask set.\n";
        exit(1);
    }
}


Thread::Thread(int id): id(id)
{
    // No need to call sigsetjmp since this will be done when the main thread is switched for the first time.
    stack = new char[STACK_SIZE];
}


Thread::~Thread()
{
    delete stack;
}


void Thread::setState(State state)
{
    this->state = state;
}


State Thread::getState() const
{
    return state;
}


int Thread::getId() const
{
    return id;
}


sigjmp_buf* Thread::getEnv()
{
    return &env;
}