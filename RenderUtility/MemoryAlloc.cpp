#include "MemoryAlloc.h"

#ifdef WIN32
#include <Windows.h>
#include <AclAPI.h>
#include <tchar.h>
#pragma comment(lib, "Advapi32.lib")
#else if LINUX
//
#endif

#include <stdio.h>
#include <stdlib.h>

void* MemoryAlloc::PageLockedMalloc(size_t size)
{
	void* buffer = NULL;

#ifdef WIN32 

	/*SYSTEM_INFO sSysInfo;         // useful information about the system
	GetSystemInfo(&sSysInfo);     // initialize the structure
	printf("This computer has page size %d.\n", sSysInfo.dwPageSize);

	//modify access right
	HANDLE currentProcess = GetCurrentProcess();
	GetSecurityInfo(currentProcess,)
	SetSecurityInfo(currentProcess,SE_KERNEL_OBJECT ,)*/

	//need the PROCESS_SET_QUOTA access right
	//set the minimal to the size*2
	BOOL isOK = SetProcessWorkingSetSize(GetCurrentProcess(), size*2, size*3);
	buffer = malloc(size);
	BOOL bLocked = VirtualLock((LPVOID)buffer, (DWORD)size);
	if(bLocked == FALSE)
		printf("Page lock Failed!\n");

#endif	

	return buffer;
}

void MemoryAlloc::PageLockedMallocFree(void* ptr)
{
	if(ptr!=NULL)
		free(ptr);
}