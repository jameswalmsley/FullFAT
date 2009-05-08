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
