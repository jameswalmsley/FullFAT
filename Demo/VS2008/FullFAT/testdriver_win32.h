
#include <Windows.h>

signed int fnRead_512		(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnVistaRead_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDev);
signed int fnVistaWrite_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, HANDLE hDev);
signed int fnWrite_512		(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnNewRead_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnNewWrite_512	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int test_2048		(char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
