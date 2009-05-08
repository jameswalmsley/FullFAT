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


#include <stdio.h>
#include <windows.h>
#include "FFTerm.h"


int hello(int argc, char**argv) {
	
	if(argc == 0) {
		printf("Corrupt Arguments\n");
		return -2;
	}

	if(argc == 1) {
		printf("Hello World\n");
	}

	if(argc == 2) {
		printf("%s\n", argv[1]);
	}

	if(argc > 2) {
		printf("Usage: hello \"My Message\"\n");
		return -1;
	}

	return 0;
}

void WinConsoleSetup(void){
	DWORD  mode;
	HANDLE hConsole = GetStdHandle( STD_INPUT_HANDLE );
	GetConsoleMode( hConsole, &mode );

	//mode &= ~ENABLE_PROCESSED_INPUT;
	//mode &= ~ENABLE_ECHO_INPUT;
	//mode &= ~ENABLE_LINE_INPUT;
	SetConsoleMode( hConsole, mode );
}

int main(void) {

	FFT_CONSOLE *pConsole;
	FF_T_SINT32 Error = FFT_ERR_NONE;

	WinConsoleSetup();	// Let the Windows Terminal do the hardwork!

	pConsole = FFTerm_CreateConsole("FullFAT>", stdin, stdout, &Error);

	setbuf(stdin, NULL);

	if(pConsole) {
		Error = FFTerm_AddCmd(pConsole, "hello1", hello, NULL);
		Error = FFTerm_AddCmd(pConsole, "hello", hello, NULL);
		Error = FFTerm_AddCmd(pConsole, "hello2", hello, NULL);
		Error = FFTerm_RemoveCmd(pConsole, "hello");
		Error = FFTerm_AddCmd(pConsole, "hello", hello, NULL);
		
		FFTerm_StartConsole(pConsole);
	}

	

	printf("%s\n", FFTerm_GetErrMessage(Error));

	getchar();

	return 0;

}
