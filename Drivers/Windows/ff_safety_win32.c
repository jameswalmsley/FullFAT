#include "../../src/ff_safety.h"
#include <windows.h>

void *FF_CreateSemaphore(void) {
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, NULL);
	return (void *) hSem;
}

void FF_PendSemaphore(void *pSemaphore) {
	// Call your OS's PendSemaphore with the provided pSemaphore pointer.
	
	HANDLE hSem = (HANDLE) pSemaphore;
	WaitForSingleObject(hSem, INFINITE);	// Take the semaphore, reducing its count to 0.
}

void FF_ReleaseSemaphore(void *pSemaphore) {
	// Call your OS's ReleaseSemaphore with the provided pSemaphore pointer.
	//
	HANDLE hSem = (HANDLE) pSemaphore;
	ReleaseSemaphore(hSem, 1, NULL);	// Reduce Sem count by 1.
}

void FF_DestroySemaphore(void *pSemaphore) {
	// Call your OS's DestroySemaphore with the provided pSemaphore pointer.
	//

	HANDLE hSem = (HANDLE) pSemaphore;
	CloseHandle(hSem);
}

void FF_Yield(void) {
	// Call your OS's thread Yield function.
	// If this doesn't work, then a deadlock will occur
	SwitchToThread();
}

void FF_Sleep(FF_T_UINT32 TimeMs) {
	// Call your OS's thread sleep function,
	// Sleep for TimeMs milliseconds
	Sleep(TimeMs);
}