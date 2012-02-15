#include "../../src/ff_safety.h"
#include <unistd.h>
#include <semaphore.h>

void *FF_CreateSemaphore(void) {
	sem_t *pSem = (sem_t *) malloc(sizeof(pSem));
	
	sem_init(pSem, 0, 1);

	return (void *) pSem;
}

void FF_PendSemaphore(void *pSemaphore) {
	// Call your OS's PendSemaphore with the provided pSemaphore pointer.
	
	sem_t *pSem = (sem_t *) pSemaphore;
	sem_wait(pSem);
}

void FF_ReleaseSemaphore(void *pSemaphore) {
	// Call your OS's ReleaseSemaphore with the provided pSemaphore pointer.
	//
	sem_t *pSem = (sem_t *) pSemaphore;
	sem_post(pSem);
}

void FF_DestroySemaphore(void *pSemaphore) {
	// Call your OS's DestroySemaphore with the provided pSemaphore pointer.
	//

	sem_t *pSem = (sem_t *) pSemaphore;

	sem_destroy(pSem);

	free(pSem);
}

void FF_Yield(void) {
	// Call your OS's thread Yield function.
	// If this doesn't work, then a deadlock will occur
	usleep(20);
}

void FF_Sleep(FF_T_UINT32 TimeMs) {
	// Call your OS's thread sleep function,
	// Sleep for TimeMs milliseconds
	usleep(TimeMs);
}