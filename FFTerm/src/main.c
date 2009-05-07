#include <stdio.h>
#include "FFTerm.h"


int hello(int argc, char**argv) {
	printf("Hello World\n");
	return 0;
}


int main(void) {
	FFT_CONSOLE *pConsole;
	FF_T_SINT32 Error = FFT_ERR_NONE;

	pConsole = FFTerm_CreateConsole("FullFAT", stdin, stdout, &Error);

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