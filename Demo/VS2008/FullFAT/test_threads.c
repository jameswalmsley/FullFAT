/*****************************************************************************
 *  FullFAT - High Performance, Thread-Safe Embedded FAT File-System         *
 *  Copyright (C) 2009  James Walmsley (james@worm.me.uk)                    *
 *                                                                           *
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *  IMPORTANT NOTICE:                                                        *
 *  =================                                                        *
 *  Alternative Licensing is available directly from the Copyright holder,   *
 *  (James Walmsley). For more information consult LICENSING.TXT to obtain   *
 *  a Commercial license.                                                    *
 *                                                                           *
 *  See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.       *
 *                                                                           *
 *  Removing the above notice is illegal and will invalidate this license.   *
 *****************************************************************************
 *  See http://worm.me.uk/fullfat for more information.                      *
 *  Or  http://fullfat.googlecode.com/ for latest releases and the wiki.     *
 *****************************************************************************/

/**
 *	This file provides some commands allowing one to test multi-threaded I/O on FullFAT.
 **/


#include "test_threads.h"

struct _IO_THREAD {
	HANDLE		hThread;			///< Thread Handle.
	DWORD		dwThreadID;			///< Thread ID.
	int			nThreadNum;	
	unsigned long long	nThreadBytes;
	FF_FILE		*pFile;				///< Thread's File Handle.
	FF_T_BOOL	tKill;
	FF_T_BOOL	isDead;
	struct _IO_THREAD *pNext;		///< Pointer to the next thread.
};

typedef struct _IO_THREAD *THREAD;

static THREAD g_ThreadList = NULL;

DWORD WINAPI IOTestThread( LPVOID lpParam ) {
	THREAD hThread = (THREAD) lpParam;		// Cast the thread param to the FF File.

	FF_T_UINT16 i;
	FF_T_UINT16 buf[1024];

	for(i = 0; i < 1024; i++) {
		buf[i] = i;	
	}

	while(!hThread->tKill) {
		i = (FF_T_UINT16) FF_Write(hThread->pFile, 2, 1024, (FF_T_UINT8 *) buf);
		FF_Seek(hThread->pFile, 0, FF_SEEK_SET);		
		hThread->nThreadBytes += i;
		//Sleep(1000);
	}

	printf("Thread %d: Received Kill Signal\n", hThread->nThreadNum);
	hThread->isDead = FF_TRUE;

	return 0;
}



static void RemoveThread(THREAD hThread) {
	THREAD hThreads = g_ThreadList;

	if(hThreads == hThread) {
		g_ThreadList = hThread->pNext;
	} else {
		while(hThreads->pNext) {
			if(hThreads->pNext == hThread) {
				hThreads->pNext = hThread->pNext;
				free(hThread);
				break;
			}
			hThreads = hThreads->pNext;
		}
	}
}

void KillAllThreads(void) {
	while(g_ThreadList) {
		g_ThreadList->tKill = FF_TRUE;	// Signal Thread to Die.
		
		while(!g_ThreadList->isDead);	// Wait on Thread to Signal its death.
		
		FF_Close(g_ThreadList->pFile);	// Close the file.

		CloseHandle(g_ThreadList->hThread);	// Close thread handle.
		
		RemoveThread(g_ThreadList);	// Remove thread from list.
	}
}

static THREAD GetThread(int ThreadID) {
	THREAD hThread = g_ThreadList;

	while(hThread) {
		if(hThread->nThreadNum == ThreadID) {
			return hThread;
		}
		hThread = hThread->pNext;
	}
	
	return NULL;
}


int createthread_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	THREAD hThread = (THREAD) malloc(sizeof(struct _IO_THREAD));
	THREAD hThreadList = g_ThreadList;
	FF_T_INT8	Path[FF_MAX_PATH];
	FF_ERROR Error;
	
	if(argc == 2) {
		
		if(hThread) {	
			hThread->isDead = FF_FALSE;
			hThread->pNext = NULL;
			hThread->nThreadBytes = 0;
			hThread->nThreadNum = 0;
			hThread->tKill = FF_FALSE;

			ProcessPath(Path, argv[1], pEnv);

			hThread->pFile = FF_Open(pEnv->pIoman, Path, FF_GetModeBits("w+"), &Error);

			if(!hThread->pFile) {
				free(hThread);
				printf("Error opening file: %s\n", FF_GetErrMessage(Error));
				return 0;
			}

			hThread->hThread = CreateThread(0, 0, IOTestThread, hThread, 0, &hThread->dwThreadID);
			
			if(hThread->hThread) {
				if(!hThreadList) {
					g_ThreadList = hThread;
				} else {
					hThread->nThreadNum = 1;
					while(hThreadList->pNext) {
						hThreadList = hThreadList->pNext;
						hThread->nThreadNum += 1;
					}

					hThreadList->pNext = hThread;
				}

			} else {
				FF_Close(hThread->pFile);
				free(hThread);
				printf("Error creating thread!\n");
			}
		} else {
			printf("Not enough memory!\n");
		}

	} else {
		printf("Usage: %s [filename]\n", argv[0]);
	}

	return 0;

}
const FFT_ERR_TABLE mkthreadInfo[] =
{
	{"Generic or Unknown Error!",	-1},
	{"Creates a thread for testing multi-threaded I/O.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


int listthreads_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	THREAD hThread = g_ThreadList;

	if(!argc || !pEnv || !argv) {
		return -1;
	}

	while(hThread) {
		printf("Thread %d, %f MB processed.\n", hThread->nThreadNum, ((float) hThread->nThreadBytes / (1024 * 1024)));
		hThread = hThread->pNext;
	}

	return 0;
}
const FFT_ERR_TABLE listthreadsInfo[] =
{
	{"Generic or Unknown Error!",	-1},
	{"Lists all the running I/O threads.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};

int killthread_cmd(int argc, char **argv, FF_ENVIRONMENT *pEnv) {
	THREAD hThread;
	int threadID;

	if(!pEnv) {
		return 0;
	}

	if(argc == 2) {
		sscanf(argv[1], "%d", &threadID);
		hThread = GetThread(threadID);
		if(hThread) {
			
			/*TerminateThread(hThread->hThread, &dwExitCode);*/
			hThread->tKill = FF_TRUE;
			while(!hThread->isDead);
			FF_Close(hThread->pFile);
			CloseHandle(hThread->hThread);
			RemoveThread(hThread);
		}
	} else {
		printf("Usage: %s [Thread ID]\n", argv[0]);
	}

	return 0;
}
const FFT_ERR_TABLE killthreadInfo[] =
{
	{"Generic or Unknown Error!",	-1},
	{"Kills the specified thread.",			FFT_COMMAND_DESCRIPTION},
	{ NULL }
};


