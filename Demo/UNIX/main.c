/******************************************************************************
 *   FullFAT - Embedded FAT File-System
 *
 *   Provides a full, thread-safe, implementation of the FAT file-system
 *   suitable for low-power embedded systems.
 *
 *   Written by James Walmsley, james@worm.me.uk, www.worm.me.uk/fullfat/
 *
 *   Copyright 2009 James Walmsley
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   Commercial support is available for FullFAT, for more information
 *   please contact the author, james@worm.me.uk
 *
 *   Removing the above notice is illegal and will invalidate this license.
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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <termios.h>
#include <string.h>

#include "../../src/ff_ioman.h"
#include "../../src/ff_fat.h"

#define PARTITION_NUMBER	0		///< Change this to the primary partition to be mounted (0 to 3)
#define COPY_BUFFER_SIZE	8096	// Increase This for Faster File Copies

int mygetch( ) {
	struct termios oldt,newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

void test(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
void test2(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
void test_ipod(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);

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

	printf("%s %12u %s\n", attr, pDirent->Filesize, pDirent->FileName);
}

int main(void) {
	clock_t startTick, endTick; 
	FILE *fDev,*fDest;
	int f;
	FF_FILE *fSource;
	FF_IOMAN *pIoman = FF_CreateIOMAN(NULL, 4096, 512);
	
	char buffer[COPY_BUFFER_SIZE];
	char commandLine[1024];
	char commandShadow[2600];
	char source[260], destination[260];
	char workingDir[2600] = "\\";
	char tester;
	unsigned long BytesRead;
	FF_T_UINT32 i;
	FF_DIRENT mydir;
	unsigned long long time;
	float transferRate;
	fDev = fopen("/dev/sdb", "rb");		// fopen seems to be more reliable, and its valid.
//f = open(argv[1], O_RDONLY);
	//f = open("/dev/sdb2", O_RDONLY);
	printf("Main Handle %d\n", f);


	printf("FullFAT by James Walmsley - Windows Demonstration\n");
	printf("Use the command help for more information\n\n");
	
	if(fDev) {
		FF_RegisterBlkDevice(pIoman, 512,(FF_WRITE_BLOCKS) test_512, (FF_READ_BLOCKS) test_512, fDev);
		if(FF_MountPartition(pIoman, PARTITION_NUMBER)) {
			fclose(f);
			printf("FullFAT couldn't mount the specified partition\n");
			getchar();
			return -1;
		}

		while(1) {
			printf("FullFAT:%s>",workingDir);
			for(i = 0; i < 1024; i++) {
				commandLine[i] = mygetch();
				putchar(commandLine[i]);
				if(commandLine[i] == '\n') {
					putchar('\n');
					commandLine[i] = '\0';
					break;
				}
			}
			if(strstr(commandLine, "cd")) {
				
				if(commandLine[3] != '\\' && commandLine[3] != '/') {
					if(strlen(workingDir) == 1) {
						sprintf(commandShadow, "\\%s", (commandLine + 3));
					} else {
						sprintf(commandShadow, "%s\\%s", workingDir, (commandLine + 3));
					}
				} else {
					sprintf(commandShadow, "%s", (commandLine + 3));
				}

				if(FF_FindDir(pIoman, commandShadow, strlen(commandShadow))) {
					sprintf(workingDir, "%s", commandShadow);
				} else {
					printf("Path %s Not Found\n", commandShadow);
				}
			}

			if(strstr(commandLine, "ls") || strstr(commandLine, "dir")) {
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

			if(strstr(commandLine, "view")) {
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (commandLine+5)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (commandLine+5));
				}
				
				fSource = FF_Open(pIoman, buffer, FF_MODE_READ);
				if(fSource) {
					for(i = 0; i < fSource->Filesize; i++) {
						printf("%c", FF_GetC(fSource));
					}

					FF_Close(fSource);
				}else {
					printf("File Not Found!\n");
				}
			}

			if(strstr(commandLine, "pwd")) {
				printf("%s\n", workingDir);
			}

			if(strstr(commandLine, "help")) {
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

			if(strstr(commandLine, "info")) {
				switch(pIoman->pPartition->Type) {
					case FF_T_FAT32:
						printf("FAT32 Formatted Drive\n"); break;
					case FF_T_FAT16:
						printf("FAT16 Formatted Drive\n"); break;
				}
				printf("Block Size: %d\n", pIoman->pPartition->BlkSize);
			}

			if(strstr(commandLine, "cp")) {
				sscanf((commandLine + 3), "%s %s", source, destination);
				
				if(strlen(workingDir) == 1) {
					sprintf(buffer, "\\%s", (source)); 
				} else {
					sprintf(buffer, "%s\\%s", workingDir, (source));
				}
				
				fDest = fopen(destination, "wb");
				if(fDest) {
					fSource = FF_Open(pIoman, buffer, FF_MODE_READ);
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
						printf("Error Opening Source\n");
					}
				} else {
					printf("Error Opening Destination\n");
				}
				strcpy(source, "");
				strcpy(destination, "");
			}

			if(strstr(commandLine, "exit") || strstr(commandLine, "quit")) {
				close(f);
				return 0;
			}
		}
		
	} else {
		
		printf("Couldn't Open Block Device\n");
		printf("Run FullFAT as Root (sudo ./FullFAT)\n");
		getchar();
	}

}
