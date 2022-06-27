//
// Created by djafere
//

#include "../h/tcb.hpp"
#include "../h/printing.hpp"
#include "../h/riscv.hpp"
#include "../h/userMain.hpp"
#include "../h/_thread.hpp"
#include "../h/_sleeplist.hpp"
#include "../h/MemoryAllocator.hpp"

int main()
{
    printString("main() started\n");

    MemoryAllocator::init_memory();

    TCB *kernel = TCB::kernelThread();
    TCB *idle = TCB::idleThread();

    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);
    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    thread_t user;
    thread_create(&user, userMain, (void*)1337);

    while (!user->isFinished()) {
        thread_dispatch();
    }

    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);

    delete kernel;
    delete user;
    delete idle;
    printString("main() finished\n");

    return 0;
}
