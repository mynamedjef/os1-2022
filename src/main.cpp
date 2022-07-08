//
// Created by djafere
//

#include "../h/tcb.hpp"
#include "../test/printing.hpp"
#include "../h/riscv.hpp"
#include "../h/userMain.hpp"
#include "../h/_thread.hpp"
#include "../h/_sleeplist.hpp"
#include "../h/MemoryAllocator.hpp"

void user_wrapper(void *sem)
{
    printString("userMain() started\n");
    userMain();
    printString("userMain() finished\n");
    sem_signal((sem_t)sem);
}

int main()
{
    MemoryAllocator::init_memory();
    Riscv::init();

    TCB::kernelThread();
    TCB::idleThread();
    TCB::outputThread();

    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);
    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    printString("main() started\n");

    // paljenje korisnika
    sem_t user_sem;
    sem_open(&user_sem, 0);

    thread_t user;
    thread_create(&user, user_wrapper, user_sem);

    // čekanje korisnika
    sem_wait(user_sem);

    printString("main() cleaning up\n");
    while (Riscv::bufout->count() > 0) { thread_dispatch(); } // čekanje da se ispiše sve iz bafera ako već nije
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);

    Riscv::cleanup();
    Scheduler::cleanup();
    _sem::cleanup();
    _thread::cleanup();
    TCB::cleanup();
//    printInt(MemoryAllocator::allocd - MemoryAllocator::deallocd);
    putc('\n');

    return 0;
}
