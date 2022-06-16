//
// Created by djafere on 5/9/22.
//

#ifndef _memory_allocator_hpp
#define _memory_allocator_hpp

#include "../lib/hw.h"
#include "MemDescr.hpp"


class MemoryAllocator {
public:
	static MemDescr *free, *occupied;
	
	static void *alloc(size_t);
	static int mem_free(void*);
	static void init_memory();
	
	// for debugging purposes
	static void print_descr(MemDescr*);
	static void prfree();
	static void proccupied();
	
};


#endif //_memory_allocator_hpp