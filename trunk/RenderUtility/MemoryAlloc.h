#ifndef _MEMORY_ALLOC_H
#define _MEMORY_ALLOC_H

class MemoryAlloc
{
public:
	//pageLockedMemoryAlloc();
	static void* PageLockedMalloc(size_t size);
	static void PageLockedMallocFree(void* ptr);

private:
	;

};


#endif
