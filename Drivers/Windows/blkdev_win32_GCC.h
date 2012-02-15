#include <Windows.h>
#include <stdio.h>
#include "../../src/fullfat.h"			// Include everything required for FullFAT.

#define BLOCK_SIZE	512

FF_T_UINT16 GetBlockSize(HANDLE hDevice);

HANDLE fnOpen(char *strDevName, int nBlockSize);
void fnClose(HANDLE hDevice);
signed int fnRead	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);
signed int fnWrite	(unsigned char *buffer, unsigned long sector, unsigned short sectors, void *pParam);



//---------- The following code is based on the Windows API:
//---------- It is included here to provide compatibilty with GCC (MinGW Windows API Headers)

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#define FILE_ANY_ACCESS                 0
#define METHOD_BUFFERED                 0
#define FILE_DEVICE_DISK                0x00000007
#define IOCTL_DISK_BASE                 FILE_DEVICE_DISK
#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0028, METHOD_BUFFERED, FILE_ANY_ACCESS)



typedef enum _MEDIA_TYPE {
    Unknown,                // Format is unknown
} MEDIA_TYPE, *PMEDIA_TYPE;


typedef struct _DISK_GEOMETRY {
    LARGE_INTEGER Cylinders;
    MEDIA_TYPE MediaType;
    DWORD TracksPerCylinder;
    DWORD SectorsPerTrack;
    DWORD BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;


typedef struct _DISK_GEOMETRY_EX {
        DISK_GEOMETRY Geometry;                                 // Standard disk geometry: may be faked by driver.
        LARGE_INTEGER DiskSize;                                 // Must always be correct
        BYTE  Data[1];                                          // Partition, Detect info
} DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;
