#include "Thread.h"


Thread::Thread(int id, void (*f)(void)): id(id), f(f)
{
    state = READY;
    stack = new char[STACK_SIZE];
    address_t sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
    address_t pc = (address_t)f;
    sigsetjmp(context, 1);
    (context->__jmpbuf)[JB_SP] = translate_address(sp);
    (context->__jmpbuf)[JB_PC] = translate_address(pc);
    if (sigemptyset(&context->__saved_mask) == FAIL_CODE)
    {
        std::cerr << SYS_ERROR_MSG << "failed to initialize signal mask set.\n";
        exit(1);
    }
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


sigjmp_buf* Thread::getContext()
{
    return &context;
}