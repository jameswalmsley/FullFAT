#include <stdio.h>
#include <windows.h>
#include "FFTerm.h"


int hello(int argc, char**argv) {
	printf("Hello World\n");
	return 0;
}

void WinConsoleSetup(void){
	DWORD  mode;
	HANDLE hConsole = GetStdHandle( STD_INPUT_HANDLE );
	GetConsoleMode( hConsole, &mode );

	mode &= ~ENABLE_PROCESSED_INPUT;
	//mode &= ~ENABLE_ECHO_INPUT;
	//mode &= ~ENABLE_LINE_INPUT;
	SetConsoleMode( hConsole, mode );
}

int main(void) {
	HANDLE WinConsole;
	DWORD	Mode;

	FFT_CONSOLE *pConsole;
	FF_T_SINT32 Error = FFT_ERR_NONE;

	WinConsole = GetStdHandle(STD_INPUT_HANDLE);
	
	//SetConsoleMode(WinConsole, 0x0000);
	WinConsoleSetup();

	pConsole = FFTerm_CreateConsole("FullFAT", stdin, stdout, &Error);

	setbuf(stdin, NULL);

	if(pConsole) {
		Error = FFTerm_AddCmd(pConsole, "hello1", hello);
		Error = FFTerm_AddCmd(pConsole, "hello", hello);
		Error = FFTerm_AddCmd(pConsole, "hello2", hello);
		Error = FFTerm_RemoveCmd(pConsole, "hello");
		Error = FFTerm_AddCmd(pConsole, "hello", hello);
		
		FFTerm_StartConsole(pConsole);
	}

	

	printf("%s\n", FFTerm_GetErrMessage(Error));

	getchar();

	return 0;

}