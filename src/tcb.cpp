//
// Created by marko on 20.4.22..
//

#include "../h/tcb.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.h"
#include "../h/_sleeplist.hpp"

TCB *TCB::running = nullptr;

TCB *TCB::kernel = nullptr;

TCB *TCB::idle = nullptr;

uint64 TCB::timeSliceCounter = 0;

void TCB::idleWrapper(void*)
{
    while (true) { }
}

TCB *TCB::createThread(Body body, void *arg, uint64 *stack)
{
    TCB *thr = initThread(body, arg, stack);
    thr->ready();
    return thr;
}

TCB *TCB::initThread(Body body, void *arg, uint64 *stack)
{
    return new TCB(body, arg, stack);
}

TCB *TCB::kernelThread()
{
    if (!kernel) {
        TCB *thr = new TCB();
        running = kernel = thr;
    }
    return kernel;
}

TCB *TCB::idleThread()
{
    if (!idle) {
        uint64 *stack = (uint64*)__mem_alloc(sizeof(uint64) * DEFAULT_STACK_SIZE);
        idle = initThread(idleWrapper, nullptr, stack);
        idle->status = IDLE;
    }
    return idle;
}

int TCB::start()
{
    if (status != CREATED) {
        return -1;
    }
    ready();
    return 0;
}

int TCB::exit()
{
    if (running->status != RUNNING) {
        return -1;
    }
    running->status = FINISHED;
    dispatch();
    return 0;
}

int TCB::wait()
{
    if (running->status != RUNNING) {
        return -1;
    }
    running->status = WAITING;
    dispatch();
    return 0;
}

int TCB::release()
{
    if (status != WAITING) {
        return -1;
    }
    ready();
    return 0;
}

void TCB::dispatch()
{
    TCB *old = running;
    if (old->status == RUNNING) {
        old->ready();
    }

    running = Scheduler::get();
    if (running) {
        running->status = RUNNING;
    } else {
        running = idle;
    }

    TCB::contextSwitch(&old->context, &running->context);
}

void TCB::threadWrapper()
{
    Riscv::popSppSpie();
    running->body(running->arg);
    running->setStatus(FINISHED);
    thread_dispatch();
}

int TCB::sleep(time_t timeout)
{
    if (running->status != RUNNING) {
        return -1;
    }
    running->status = SLEEPING;
    _sleeplist::insert(running, timeout);
    dispatch();
    return 0;
}

int TCB::wake()
{
    if (status != SLEEPING) {
        return -1;
    }
    ready();
    return 0;
}

void TCB::ready()
{
    status = READY;
    Scheduler::put(this);
}
