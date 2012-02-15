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
 *	This file calls unit-tests for all modules, and displays the results.
 *	On sucessful completion it will enter a Demonstration Terminal allowing
 *	you to ls through a file-system on a connected device.
 *
 *	You should ensure all Unit-tests are passed on your platform.
 **/

/*
	NOTE THIS DEMO IS HIGHLY SUBJECT TO CHANGE:
	AND MOST UNIT TESTS HAVE NOT BEEN INCLUDED AT THIS POINT:

	SIMPLY USE A BASIC DEMO
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>
#include <winioctl.h>
#include <conio.h>
#include "testdriver_win32.h"
#include "../../../src/fullfat.h"

//#include "C:\Documents and Settings\UserXP\My Documents\Visual Studio 2008\Projects\SmartCACHE\SmartCACHE\SmartCache.h"

#include "md5.h"


#define PARTITION_NUMBER	0				// Change this to the primary partition to be mounted (0 to 3)
#define COPY_BUFFER_SIZE	(2048*16)		// Increase This for Faster File Copies

void FF_PrintCache(FF_IOMAN *pIoman) {
	FF_T_UINT32 i;
	FF_BUFFER *pBuffer;
	FF_T_INT8 mode[8];
	FF_T_INT8 modified[8];
	for(i = 0; i < pIoman->CacheSize; i++) {
		pBuffer = (pIoman->pBuffers + i);
		if(pBuffer->Mode == FF_MODE_WRITE) {
			sprintf(mode, "WRITE");
		} else {
			sprintf(mode, "READ");
		}

		if(pBuffer->Modified) {
			sprintf(modified, "YES");
		} else {
			sprintf(modified, "NO");
		}
		printf("ID: %3d, LBA %10d, Mode: %5s, Hndls: %5d, LRU: %5d, Diff: %3s\n", i, pBuffer->Sector, mode, pBuffer->NumHandles, pBuffer->LRU, modified);
	}
}

void FF_PrintDir(FF_DIRENT *pDirent) {
	unsigned char attr[5] = { '-','-','-','-', '\0' };
	if(pDirent->Attrib & FF_FAT_ATTR_READONLY)
			attr[0] = 'R';
	if(pDirent->Attrib & FF_FAT_ATTR_HIDDEN)
			attr[1] = 'H';
	if(pDirent->Attrib & FF_FAT_ATTR_SYSTEM)
			attr[2] = 'S';
	if(pDirent->Attrib & FF_FAT_ATTR_DIR)
			attr[3] = 'D';

	printf("%s %12lu %s\n", attr, pDirent->Filesize, pDirent->FileName);
}

int main_old(void) {
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER start_ticks, end_ticks, cputime; 
	FILE *f = NULL, *fDest = NULL, *fXSource;
	FF_FILE *fSource;
	FF_BUFFER *pBuffer;
	FF_IOMAN *pIoman = FF_CreateIOMAN(NULL, 8192, 512, NULL);
	char buffer[COPY_BUFFER_SIZE];
	char commandLine[10][1024];
	char commandShadow[2600];
	char source[260], destination[260];
	char workingDir[2600] = "\\";
	char cmdHistory = 0;
	char tester;
	char *Argument;
	char *argv;
	unsigned int nArgs;
	FF_T_SINT8 RetVal;
	unsigned long BytesRead;
	FF_T_INT32 i;
	FF_DIRENT mydir;
	FF_T_SINT8 Error;
//	T_SCACHE *pCache;

	char *mydata;
	char mystring[] = "Hello";

	HANDLE hDev;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

//	char mystring[] = "Hello World!\n";
	float time, transferRate;
	mydata = (char *) malloc(1048576 * 4);
	for(i = 0; i < 1048576 * 4; i++) {
		mydata[i] = mystring[i % 5];
	}
	//hDev = CreateFile(TEXT("\\\\.\\PhysicalDrive1"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
	hDev = 0;
	if(hDev == INVALID_HANDLE_VALUE) {
		printf("Vista!\n");
	}

	SetConsoleTitle(TEXT("FullFAT"));
	
	//f = fopen("c:\\before.img", "rb+");
	f = fopen("\\\\.\\PHYSICALDRIVE1", "rb+");
	//f = fopen("c:\\Before.img", "rb+");
	//f1 = open("\\\\.\\PHYSICALDRIVE1",  O_RDWR | O_BINARY);
	//f1 = open("c:\\ramdisk.dat",  O_RDWR | O_BINARY);
	
	QueryPerformanceFrequency(&ticksPerSecond);

	for(i = 0; i < 10; i++) {
		strcpy(commandLine[i], "\0");
	}
	printf("FullFAT by James Walmsley - Windows Demonstration\n");
	printf("Use the command help for more information\n\n");


	//pCache = SC_CreateCache((SC_WRITE_BLOCKS) fnWrite_512, (SC_READ_BLOCKS) fnRead_512, f);
	

	if(f) {
		FF_RegisterBlkDevice(pIoman, 512, (FF_WRITE_BLOCKS) fnWrite_512, (FF_READ_BLOCKS) fnRead_512, f);
		//FF_RegisterBlkDevice(pIoman, 512, (FF_WRITE_BLOCKS) fnWrite_512, (FF_READ_BLOCKS) SC_fnReadBlocks, pCache);
/*			
		pBuffer = FF_GetBuffer(pIoman, 0, FF_MODE_WRITE);

		memset(pBuffer->pBuffer, 0, 512);

		FF_ReleaseBuffer(pIoman, pBuffer);

		FF_FlushCache(pIoman);
*/
		if(FF_MountPartition(pIoman, PARTITION_NUMBER)) {
			if(f) {
				fclose(f);
			}
			if(hDev) {
				CloseHandle(hDev);
			}
			printf("FullFAT Couldn't mount the specified parition!\n");
			getchar();
			return -1;
		}
		
//		FF_FindEntry(pIoman, 2, "4m", &mydir);

/*		fSource = FF_Open(pIoman, "\\4m", FF_MODE_WRITE, &Error);
		fDest = fopen("c:\\my1m", "wb");
		FF_Write(fSource, 1, 1048576*4, mydata);
		/*for(i = 0; i < 1048576*4; i++) {
			FF_PutC(fSource, mydata[i]);
		}*/
/*		fwrite(mydata, 1, 1048576*4, fDest);
		fclose(fDest);
		FF_Close(fSource);
*/
		/*for(i = 0; i < 256; i++) {
			sprintf(buffer, "%dthis-is-dir%d", i);
			RetVal = FF_MkDir(pIoman, "\\james", buffer);
			if(RetVal) {
				printf("%s\n", FF_GetErrMessage(RetVal));
			}
		}*/

		//ff1 = FF_Open(pIoman, "\\hello.txt", FF_MODE_WRITE, NULL);

		//ff2 = FF_Open(pIoman, "\\hello.txt", FF_MODE_READ, NULL);
		/*if(ff1) {
			for(i = 0; i < 64000; i++) {
				x = i % strlen(mystring);
				FF_PutC(ff1, mystring[x]);
			}
		//	FF_Close(ff1);
		}*/

		//FF_RmFile(pIoman, "\\talk.mp3");

		while(1 == 1) {
			printf("FullFAT:%s>",workingDir);
			for(i = 0; i < 1024; i++) {
				commandLine[cmdHistory][i] = (char) _getch();
				commandLine[cmdHistory][i + 1] = '\0';
				if(commandLine[cmdHistory][i] == 'à') {
					commandLine[cmdHistory][i] = (char) _getch();
					if(commandLine[cmdHistory][i] == 'K') {
						printf("Left\n");
					}
					if(commandLine[cmdHistory][i] == 'M') {
						printf("Right\n");
					}
					if(commandLine[cmdHistory][i] == 'H') {
						if(cmdHistory > 0) {
							cmdHistory -= 1;
						} else {
							cmdHistory = 9;
						}
						printf("\rFullFAT:%s>%s", workingDir, commandLine[cmdHistory]);
						i = strlen(commandLine[cmdHistory]);

					}
					if(commandLine[cmdHistory][i] == 'P') {
						cmdHistory += 1;
						if(cmdHistory > 9) {
							cmdHistory = 0;
						}
						printf("\rFullFAT:%s>%s", workingDir, commandLine[cmdHistory]);
						i = strlen(commandLine[cmdHistory]);
					}
				} else if(commandLine[cmdHistory][i] == 0x08) {
					commandLine[cmdHistory][--i] = '\0';
					i --;
					//_putch(0x08);
				}else {
					//_putch(commandLine[cmdHistory][i]);
				}
				printf("\r                                                                               ");
				//printf("\x1B[2K");
				printf("\rFullFAT:%s>%s\r",workingDir, commandLine[cmdHistory]);

				if(commandLine[cmdHistory][i] == '\r') {
					_putch('\n');
					commandLine[cmdHistory][i] = '\0';
					break;
				}
			}

			i = 0;
			/*argv = malloc(
			while((Argument = strtok(commandLine[cmdHistory], " ")) != NULL) {
				i++;
												
			}*/

			

			if(strstr(commandLine[cmdHistory], "cd")) {
				
				if(commandLine[cmdHistory][3] != '\\' && commandLine[cmdHistory][3] != '/') {
					if(strlen(workingDir) == 1) {
						sprintf(commandShadow, "\\%s", (commandLine[cmdHistory] + 3));
					} else {
						sprintf(commandShadow, "%s\\%s", workingDir, (commandLine[cmdHistory] + 3));
					}
				} else {
					sprintf(commandShadow, "%s", (commandLine[cmdHistory] + 3));
				}

				if(FF_FindDir(pIoman, commandShadow, (FF_T_UINT16) strlen(commandShadow))) {
					sprintf(workingDir, "%s", commandShadow);
				} else {
					printf("Path %s Not Found\n", commandShadow);
				}
			}

			if(strstr(commandLine[cmdHistory], "md5")) {
				
				if(commandLine[cmdHistory][4] != '\\' && commandLine[cmdHistory][4] != '/') {
					if(strlen(workingDir) == 1) {
						sprintf(commandShadow, "\\%s", (commandLine[cmdHistory] + 4));
					} else {
						sprintf(commandShadow, "%s\\%s", workingDir, (commandLine[cmdHistory] + 4));
					}
				} else {
					sprintf(commandShadow, "%s", (commandLine[cmdHistory] + 4));
				}

				/*if(FF_FindDir(pIoman, commandShadow, (FF_T_UINT16) strlen(commandShadow))) {
					sprintf(workingDir, "%s", commandShadow);
				} else {
					printf("Path %s Not Found\n", commandShadow);
				}*/

				
				fSource = FF_Open(pIoman, commandShadow, FF_MODE_READ, NULL);

				if(fSource) {
					int len;
					md5_state_t state;
					md5_byte_t digest[16];
					int di;

					//unsigned char md5buffer[1024];

					md5_init(&state);

					while (len = FF_Read(fSource, 1, COPY_BUFFER_SIZE, buffer))
						md5_append(&state, (const md5_byte_t *)buffer, len);
					
					md5_finish(&state, digest);

					for (di = 0; di < 16; ++di)
						printf("%02x", digest[di]);

					printf ("\n");

					FF_Close(fSource);
				}

			}

			if(strstr(commandLine[cmdHistory], "ls")) {
				i = 0;
				tester = 0;
				tester = FF_FindFirst(pIoman, &mydir, workingDir);
				while(tester == 0) {
					FF_PrintDir(&mydir);
					i++;
					tester = FF_FindNext(pIoman, &mydir);
				}
				printf("\n%d Items\n", i);
				putchar('\n');
			}



			if(strstr(commandLine[cmdHistory], "mkdir")) {

				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (commandLine[cmdHistory]+6)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (commandLine[cmdHistory]+6));
				}

				tester = FF_MkDir(pIoman, buffer);

				if(tester) {
					printf("%s", FF_GetErrMessage(tester));
				}

				if(tester) {
					switch(tester) {
						case FF_ERR_DIR_OBJECT_EXISTS: {
							printf("An object with the name \"%s\" already exists!\n", commandLine[cmdHistory]+6);
							break;
						}

						case FF_ERR_DIR_DIRECTORY_FULL: {
							printf("The current directory has reached its limit of objects!\n");
							break;
						}

						default: {
							printf("Unknown Error while making a Directory (%d)\n", RetVal);
							break;
						}
					}
				}
			}

			if(strstr(commandLine[cmdHistory], "rm")) {
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (commandLine[cmdHistory]+3)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (commandLine[cmdHistory]+3));
				}

				tester = FF_RmFile(pIoman, buffer);

				if(tester) {
					switch(tester) {
						case FF_ERR_FILE_NOT_FOUND: {
							printf("Could not find \"%s\"\n", commandLine[cmdHistory]+3);
							break;
						}

						case FF_ERR_FILE_OBJECT_IS_A_DIR: {
							printf("Cannot delete a directory (use the rd command)\n");
							break;
						}

						case FF_ERR_FILE_ALREADY_OPEN: {
							printf("Dir is currently in use\n");
						}

						default: {
							printf("Unknown Error while removing a file.\n");
							break;
						}
					}
				}
			}

			if(strstr(commandLine[cmdHistory], "rd")) {
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (commandLine[cmdHistory]+3)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (commandLine[cmdHistory]+3));
				}

				tester = FF_RmDir(pIoman, buffer);

				if(tester) {
					switch(tester) {
						case FF_ERR_FILE_NOT_FOUND: {
							printf("Could not find \"%s\"\n", commandLine[cmdHistory]+3);
							break;
						}

						case FF_ERR_FILE_ALREADY_OPEN: {
							printf("Directory is currently in use\n");
						}

						case FF_ERR_DIR_NOT_EMPTY: {
							printf("Cannot delete this Dir, it contains files!\n");
							break;
						}

						default: {
							printf("Unknown Error while removing a directory.\n");
							break;
						}
					}
				}
			}



			if(strstr(commandLine[cmdHistory], "view")) {
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (commandLine[cmdHistory]+5)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (commandLine[cmdHistory]+5));
				}
				
				fSource = FF_Open(pIoman, buffer, FF_MODE_READ, &Error);
				if(fSource) {
					for(i = 0; (FF_T_UINT32)i < fSource->Filesize; i++) {
						printf("%c", FF_GetC(fSource));
					}

					FF_Close(fSource);
				}else {
					if(Error == FF_ERR_FILE_NOT_FOUND) {
						printf("File Not Found!\n");
					} else if (Error == FF_ERR_FILE_ALREADY_OPEN) {
						printf("File In Use!\n");
					} else {
						printf("Couldn't Open file! Unknown Error \n");
					}
				}
			}

			if(strstr(commandLine[cmdHistory], "pwd")) {
				printf("%s\n", workingDir);
			}

			if(strstr(commandLine[cmdHistory], "help")) {
				printf("The following commands are available:\n\n");
				printf("pwd \t\t- Print the working directory\n");
				printf("ls or dir \t- List the contents of the working directory\n");
				printf("cd \t\t- Change working directory e.g. cd path_to_dir\n");
				printf("cp \t\t- Copy a file to the hard disk.\n");
				printf("\t\t  e.g. cp filename c:\\filename\n");
				printf("exit \t\t- Quits the FullFAT test suite.\n");
				printf("\nFullFAT is developed and maintained by James Walmsley\n");
				printf("\nVisit www.worm.me.uk/fullfat for more information, and contact details\n\n");
			}

			if(strstr(commandLine[cmdHistory], "info")) {
				switch(pIoman->pPartition->Type) {
					case FF_T_FAT32:
						printf("FAT32 Formatted Drive\n"); break;
					case FF_T_FAT16:
						printf("FAT16 Formatted Drive\n"); break;
					case FF_T_FAT12:
						printf("FAT12 Formatted Drive\n"); break;
				}

				printf("Block Size: %d\n", pIoman->pPartition->BlkSize);
				printf("Cluster Size: %dKb\n", (pIoman->pPartition->BlkSize * pIoman->pPartition->SectorsPerCluster) / 1024);
#ifdef FF_64_NUM_SUPPORT
				printf("Volume Size: %llu (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
				printf("Volume Free: %llu (%d MB)\n", FF_GetFreeSize(pIoman), (unsigned int) (FF_GetFreeSize(pIoman) / 1048576));
#else
				printf("Volume Size: %d (%d MB)\n", FF_GetVolumeSize(pIoman), (unsigned int) (FF_GetVolumeSize(pIoman) / 1048576));
#endif
			}

			if(strstr(commandLine[cmdHistory], "cmp")) {
				sscanf((commandLine[cmdHistory] + 4), "%s %s", source, destination);
				i = strlen(source);
				if(i == strlen(destination)) {
					if(FF_StrMatch(source, destination, i)) {
						printf("Strings match!\n");
					} else {
						printf("Not a match!\n");
					}
				} else {
					printf("Not a match!\n");
				}
			}

			if(strstr(commandLine[cmdHistory], "cp")) {
				sscanf((commandLine[cmdHistory] + 3), "%s %s", source, destination);
				
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (source)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (source));
				}
				
				fDest = fopen(destination, "wb");
				if(fDest) {
					fSource = FF_Open(pIoman, buffer, FF_MODE_READ, &Error);
					if(fSource) {
						QueryPerformanceCounter(&start_ticks);  
						do{
							BytesRead = FF_Read(fSource, COPY_BUFFER_SIZE, 1, (FF_T_UINT8 *)buffer);
							fwrite(buffer, BytesRead, 1, fDest);
							QueryPerformanceCounter(&end_ticks); 
							cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
							time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
							transferRate = (fSource->FilePointer / time) / 1024;
							printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
						}while(BytesRead > 0);
						printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
						fclose(fDest);
						FF_Close(fSource);
					} else {
						fclose(fDest);
							printf("Source: ");
					switch(Error) {
						case FF_ERR_FILE_INVALID_PATH:
							printf("Invalid Path (Directory not found)\n");
							break;
						case FF_ERR_FILE_IS_READ_ONLY:
							printf("Destination has Read-Only permissions\n");
							break;
						case FF_ERR_FILE_NOT_FOUND:
							printf("File Not Found!\n");
							break;
						case FF_ERR_FILE_ALREADY_OPEN:
							printf("The file is currently in use by another process!\n");
							break;
						default :
							printf("Couldn't Open file! Unknown Error (%d)\n", Error);
							break;
					}
					}
				} else {
					printf("Error opening destination!\n");
				}
				strcpy(source, "");
				strcpy(destination, "");
			}

			if(strstr(commandLine[cmdHistory], "copy")) {
				sscanf((commandLine[cmdHistory] + 5), "%s %s", source, destination);
				
				if(strlen(workingDir) == 1 && !(destination[0] != '\\' || destination[0] != '/')) {
					sprintf(buffer, "\\%s", (destination)); 
				} else {
					if(destination[0] == '\\' || destination[0] == '/') {
						sprintf(buffer, "%s", (destination));
					} else {
						sprintf(buffer, "%s\\%s", workingDir, (destination));
					}
				}
				
				fDest = fopen(source, "rb");
				if(fDest) {
					fSource = FF_Open(pIoman, buffer, FF_MODE_WRITE, &Error);
					if(fSource) {
						QueryPerformanceCounter(&start_ticks);  
						do{
							BytesRead = fread(buffer, 1, COPY_BUFFER_SIZE, fDest);
							FF_Write(fSource, BytesRead, 1, (FF_T_UINT8 *)buffer);
							QueryPerformanceCounter(&end_ticks); 
							cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
							time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
							transferRate = (fSource->FilePointer / time) / 1024;
							printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\r", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
						}while(BytesRead > 0);
						printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\n", ((float)((float)fSource->FilePointer/(float)fSource->Filesize) * 100), fSource->FilePointer, transferRate);
						fclose(fDest);
						FF_Close(fSource);
					} else {
						fclose(fDest);
						switch(Error) {
							case FF_ERR_FILE_INVALID_PATH:
								printf("Invalid Path (Directory not found)\n");
								break;
							case FF_ERR_FILE_IS_READ_ONLY:
								printf("Destination has Read-Only permissions\n");
								break;
							case FF_ERR_FILE_NOT_FOUND:
								printf("File Not Found!\n");
								break;
							case FF_ERR_FILE_ALREADY_OPEN:
								printf("The file is currently in use by another process!\n");
								break;
							default :
								printf("Couldn't Open file! Unknown Error (%d)\n", Error);
								break;
						}
					}
				} else {
					printf("Error Opening Destination\n");
				}
				strcpy(source, "");
				strcpy(destination, "");
			}

			if(strstr(commandLine[cmdHistory], "xc")) {
				sscanf((commandLine[cmdHistory] + 3), "%s %s", source, destination);
				
				fDest = fopen(destination, "wb");
				if(fDest) {
					fXSource = fopen(source, "rb");
					if(fXSource) {
						QueryPerformanceCounter(&start_ticks);  
						i = 0;
						do{
							BytesRead = fread(buffer, 1, COPY_BUFFER_SIZE, fXSource);
							i += BytesRead;
							fwrite(buffer, BytesRead, 1, fDest);
							QueryPerformanceCounter(&end_ticks); 
							cputime.QuadPart = end_ticks.QuadPart - start_ticks.QuadPart;
							time = ((float)cputime.QuadPart/(float)ticksPerSecond.QuadPart);
							transferRate = ((float)i / time) / 1024;
							printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\r", 1.0, i, transferRate);
						}while(BytesRead > 0);
						printf("%3.0f%% - %10d Bytes Copied, %7.2f Kb/S\n", 1.0, i, transferRate);
						fclose(fDest);
						fclose(fXSource);
					} else {
						fclose(fDest);
						if(Error == FF_ERR_FILE_NOT_FOUND) {
							printf("File Not Found!\n");
						} else if (Error == FF_ERR_FILE_ALREADY_OPEN) {
							printf("File In Use!\n");
						} else {
							printf("Couldn't Open file! Unknown Error \n");
						}
					}
				} else {
					printf("Error Opening Destination\n");
				}
				strcpy(source, "");
				strcpy(destination, "");
			}

			if(strstr(commandLine[cmdHistory], "flush")) {
				FF_FlushCache(pIoman);
			}

			if(strstr(commandLine[cmdHistory], "cache")) {
				FF_PrintCache(pIoman);
			}

			if(strstr(commandLine[cmdHistory], "exit") || strstr(commandLine[cmdHistory], "quit")) {
				RetVal = FF_UnMountPartition(pIoman);
				if(!RetVal) {
					if(f) {
						fclose(f);
					}
					if(hDev) {
						CloseHandle(hDev);
					}
					return 0;
				}

				switch(RetVal) {
					case FF_ERR_IOMAN_ACTIVE_HANDLES: {
						printf("There are currently active handles on the disk.\n");
						break;
					}
					default: {
						printf("Unknown Error\n");
						break;
					}
				}
			}

			cmdHistory++;
			if(cmdHistory >= 10) {
				cmdHistory = 0;
			}
		}
		
	} else {
		
		printf("Couldn't Open Block Device\n");
		printf("Run Visual Studio as an Administrator!\n");
		getchar();
	}
}

