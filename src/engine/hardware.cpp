//
// hardware information getter implementation.
// Author:yeyouqun@163.com
// 2010-5-2
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/util.h>
#include <engine/hardware.h>

#ifdef _WIN32
#include <winioctl.h>
#include <Iphlpapi.h>
#include <Ntsecapi.h>
#include <Windows.h>
#endif

#pragma pack(1)
#define  IDENTIFY_BUFFER_SIZE  512

// IOCTL commands
#define  DFP_GET_VERSION			0x00074080
#define  DFP_SEND_DRIVE_COMMAND		0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA		0x0007c088

#define  FILE_DEVICE_SCSI				0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY	((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT			0x0004D008 //  see NTDDSCSI.H for definition

#define  IDE_ATAPI_IDENTIFY  0xA1  // Returns ID sector for ATAPI.    
#define  IDE_ATA_IDENTIFY    0xEC  // Returns ID sector for ATA.    

// GETVERSIONOUTPARAMS contains the data returned from the
// Get Driver Version function.
typedef struct _GETVERSIONOUTPARAMS
{
	BYTE bVersion;  // Binary driver version.
	BYTE bRevision; // Binary driver revision.
	BYTE bReserved; // Not used.
	BYTE bIDEDeviceMap; // Bit map of IDE devices.
	DWORD fCapabilities; // Bit mask of driver capabilities.
	DWORD dwReserved[4]; // For future use.
}GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

// Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS
#define  CAP_IDE_ID_FUNCTION 1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID 2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported
// Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  // Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY0xEC  // Returns ID sector for ATA.
// The following struct defines the interesting part of the IDENTIFY
// buffer:
typedef struct _IDSECTOR
{
	USHORT  wGenConfig;
	USHORT  wNumCyls;
	USHORT  wReserved;
	USHORT  wNumHeads;
	USHORT  wBytesPerTrack;
	USHORT  wBytesPerSector;
	USHORT  wSectorsPerTrack;
	USHORT  wVendorUnique[3];
	CHAR	sSerialNumber[20];
	USHORT  wBufferType;
	USHORT  wBufferSize;
	USHORT  wECCSize;
	CHAR	sFirmwareRev[8];
	CHAR	sModelNumber[40];
	USHORT  wMoreVendorUnique;
	USHORT  wDoubleWordIO;
	USHORT  wCapabilities;
	USHORT  wReserved1;
	USHORT  wPIOTiming;
	USHORT  wDMATiming;
	USHORT	 wBS;
	USHORT  wNumCurrentCyls;
	USHORT  wNumCurrentHeads;
	USHORT  wNumCurrentSectorsPerTrack;
	ULONG   ulCurrentSectorCapacity;
	USHORT  wMultSectorStuff;
	ULONG   ulTotalAddressableSectors;
	USHORT  wSingleWordDMA;
	USHORT  wMultiWordDMA;
	BYTE	bReserved[128];
}IDSECTOR, *PIDSECTOR;

typedef struct _SRB_IO_CONTROL
{
	ULONG HeaderLength;
	UCHAR Signature[8];
	ULONG Timeout;
	ULONG ControlCode;
	ULONG ReturnCode;
	ULONG Length;
}SRB_IO_CONTROL, *PSRB_IO_CONTROL;

#pragma pack()

BOOL DoIDENTIFY(HANDLE hPhysicalDriveIOCTL
				 , PSENDCMDINPARAMS pSCIP
				 , PSENDCMDOUTPARAMS pSCOP
				 , BYTE bIDCmd
				 , BYTE bDriveNum
				 , PDWORD lpcbBytesReturned)
{
	// Set up data structures for IDENTIFY command.
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;
	pSCIP->irDriveRegs.bFeaturesReg = 0;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = 0;
	pSCIP->irDriveRegs.bCylHighReg = 0;

	// Compute the drive number.
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

	// The command can either be IDE identify or ATAPI identify.
	pSCIP->irDriveRegs.bCommandReg = bIDCmd;
	pSCIP->bDriveNumber = bDriveNum;
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

	return DeviceIoControl( hPhysicalDriveIOCTL
						, DFP_RECEIVE_DRIVE_DATA
						, (LPVOID) pSCIP
						, sizeof(SENDCMDINPARAMS) - 1
						, (LPVOID) pSCOP
						, sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1
						, lpcbBytesReturned, NULL);
}

BOOL ReadPhysicalDriveInNT(DWORD DiskData[256])
{
	// Define global buffers.
	BYTE IdOutCmd [sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
	BOOL bRet = FALSE;
	HANDLE hPhysicalDriveIOCTL = 0;
	int drive = 0;
	// Try to get a handle to PhysicalDrive IOCTL,
	// report failure and exit if can't.
	char_t driveName [256];
	wsprintf(driveName, wxT ("\\\\.\\PhysicalDrive0"));
	// Windows NT, Windows 2000, must have admin rights
	hPhysicalDriveIOCTL = CreateFile(driveName
									 , GENERIC_READ | GENERIC_WRITE
									 , FILE_SHARE_READ | FILE_SHARE_WRITE
									 , NULL
									 , OPEN_EXISTING
									 , 0
									 , NULL);

	if( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE )
	{
		GETVERSIONOUTPARAMS VersionParams;
		DWORD  cbBytesReturned = 0;

		// Get the version, etc of PhysicalDrive IOCTL
		memset((void*)&VersionParams, 0, sizeof(VersionParams));
		if(!DeviceIoControl( hPhysicalDriveIOCTL
							 , DFP_GET_VERSION
							 , NULL
							 , 0
							 , &VersionParams
							 , sizeof(VersionParams)
							 , &cbBytesReturned
							 , NULL))
		{
		}
	
		// If there is a IDE device at number "i" issue commands
		// to the device
		if (VersionParams.bIDEDeviceMap > 0) {
			BYTE  bIDCmd = 0; // IDE or ATAPI IDENTIFY cmd
			SENDCMDINPARAMS  scip;

			// Now, get the ID sector for all IDE devices in the
			// system. If the device is ATAPI use the
			// IDE_ATAPI_IDENTIFY command, otherwise use the
			// IDE_ATA_IDENTIFY command
			bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ?
							 IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

			memset (&scip, 0, sizeof(scip));
			memset (IdOutCmd, 0, sizeof(IdOutCmd));

			if(DoIDENTIFY(hPhysicalDriveIOCTL
						 , &scip
						 , (PSENDCMDOUTPARAMS)&IdOutCmd
						 , (BYTE)bIDCmd
						 , (BYTE)drive
						 , &cbBytesReturned)) {
				memset(DiskData, 0, sizeof (DWORD) * 256);
				int i = 0;

				USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer;

				for( i = 0; i < 256; i++ ) DiskData[i] = pIdSector[i];
				bRet = TRUE;
			}
		}

		CloseHandle(hPhysicalDriveIOCTL);
	}

	return bRet;
}

//=============================================================
// DoIDENTIFY
// FUNCTION: Send an IDENTIFY command to the drive
// bDriveNum = 0-3
// bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
//==============================================================
#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE

BOOL ReadIdeDriveAsScsiDriveInNT(DWORD DiskData[256])
{
	BOOL bRet = FALSE;
	int controller = 0;

	HANDLE hScsiDriveIOCTL = 0;
	char_t   driveName [256];

	// Try to get a handle to PhysicalDrive IOCTL, report failure
	// and exit if can't.
	wsprintf (driveName, wxT ("\\\\.\\Scsi%d:"), controller);

	// Windows NT, Windows 2000, any rights should do
	hScsiDriveIOCTL = CreateFile(driveName
								, GENERIC_READ | GENERIC_WRITE
								, FILE_SHARE_READ | FILE_SHARE_WRITE
								, NULL
								, OPEN_EXISTING
								, 0
								, NULL);

	if (hScsiDriveIOCTL != INVALID_HANDLE_VALUE) {
		int drive = 0;
		char buffer[sizeof (SRB_IO_CONTROL) + SENDIDLENGTH];
		SRB_IO_CONTROL *p = (SRB_IO_CONTROL *) buffer;
		SENDCMDINPARAMS *pin =(SENDCMDINPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
		DWORD dummy;

		memset(buffer, 0, sizeof (buffer));
		p->HeaderLength = sizeof (SRB_IO_CONTROL);
		p->Timeout = 10000;
		p->Length = SENDIDLENGTH;
		p->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
		strncpy((char *) p->Signature, "SCSIDISK", 8);

		pin->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
		pin->bDriveNumber = drive;

		if(DeviceIoControl(hScsiDriveIOCTL
						 , IOCTL_SCSI_MINIPORT
						 , buffer
						 , sizeof(SRB_IO_CONTROL) + sizeof (SENDCMDINPARAMS) - 1
						 , buffer
						 , sizeof (SRB_IO_CONTROL) + SENDIDLENGTH
						 , &dummy
						 , NULL)) {
			SENDCMDOUTPARAMS *pOut =(SENDCMDOUTPARAMS *) (buffer + sizeof (SRB_IO_CONTROL));
			IDSECTOR *pId = (IDSECTOR *)(pOut->bBuffer);
			if( pId->sModelNumber [0]) {
				memset(DiskData, 0, 1024);
				int i = 0;
				USHORT *pIdSector = (USHORT *)pId;
				for( i = 0; i < 256; i++ )
					DiskData[i] = pIdSector[i];
				bRet = TRUE;
			}
		}
		CloseHandle (hScsiDriveIOCTL);
	}

	return bRet;
}

char *ConvertToString(DWORD DiskData[256], int firstIndex, int lastIndex )
{
	static char string [1024];
	int index = 0;
	int position = 0;

	// each integer has two characters stored in it backwards
	for( index = firstIndex; index <= lastIndex; index++ ) {
		//  get high byte for 1st character
		string[position] = (char)(DiskData[index] / 256);
		position++;

		// get low byte for 2nd character
		string[position] = (char)(DiskData[index] % 256);
		position++;
	}
	// end the string
	string[position] = '\0';
	// cut off the trailing blanks
	for(index = position-1; index >= 0; index--) {
		if(string[index]!=' ')
			break;
		else
			string[index]=0;
	}

	for(index = 0; index < position; index++)
		if(string[index]!=' ')
			break;

	if(index!=0) {
		strncpy(string,string + index,position - index);
		string[position-index]=0;
	}

	return string;
}

BOOL GetHDSerialCode(char* lpSerialID)
{
	if (lpSerialID == NULL) return FALSE;

	char strHDSerialNumber[1024];
	memset(strHDSerialNumber, 0, 1024);

	DWORD DiskData[256];
	BOOL bRet = ReadPhysicalDriveInNT(DiskData);

	if(!bRet) bRet = ReadIdeDriveAsScsiDriveInNT(DiskData);

	if (bRet) {
		strcpy(strHDSerialNumber, ConvertToString(DiskData, 10, 19));
		strcpy(lpSerialID, strHDSerialNumber);
	}

	return bRet;
}

 ////////////////////////////////
GmHardwareInfo::GmHardwareInfo ()
{
}

GmHardwareInfo::~GmHardwareInfo ()
{
}

wxString GmHardwareInfo::GetHardDiskSerial (const wxString & disk)
{
	char strSerialNumber[32];
	memset(strSerialNumber, 0, 32);
	BOOL bRet = GetHDSerialCode(strSerialNumber);
	if (bRet) return ToWxString (string (strSerialNumber));
	return wxEmptyString;
}

extern "C" void GetCPUID (char *, unsigned long*, unsigned long*, unsigned long*, unsigned long*);

wxString GmHardwareInfo::GetCPUSerial ()
{
	unsigned long s1, s2, s3, s4;
	s1 = s2 = s3 = s4 = 0;
	char VendorID [32];
	memset (VendorID, 0, 32);
	char_t SerialNo [128];
	/*
	__asm {  
		xor eax,eax
		cpuid
		mov dword ptr VendorID,ebx
		mov dword ptr VendorID[+4],edx
		mov dword ptr VendorID[+8],ecx
	}  

	__asm {  
		mov   eax,01h  
		xor   edx,edx  
		cpuid  
		mov   s1,edx  
		mov   s2,eax  
	}

	__asm{
		mov   eax,03h
		xor   ecx,ecx
		xor   edx,edx
		cpuid
		mov   s3,edx
		mov   s4,ecx
	}
*/
	GetCPUID (VendorID, &s1, &s2, &s3, &s4);
	wxString vendor = ToWxString (VendorID);
	wsprintf (SerialNo, wxT ("%s-%08X-%08X-%08X-%08X"), vendor.c_str (), s1, s2, s3, s4);
	return SerialNo;
}

//
//static UINT FindAwardBios (BYTE** ppBiosAddr)
//{
//    BYTE* pBiosAddr			= *ppBiosAddr + 0xEC71;
//    BYTE szBiosData[128];
//    CopyMemory (szBiosData, pBiosAddr, 127);
//    szBiosData[127]			= 0;
//   
//    int iLen = strlen ((char*)szBiosData);
//    if (iLen > 0 && iLen < 128) {
//		//
//        //AWard:         07/08/2002-i845G-ITE8712-JF69VD0CC-00
//        //Phoenix-Award: 03/12/2002-sis645-p4s333
//		//
//        if (szBiosData[2] == '/' && szBiosData[5] == '/') {
//            BYTE* p = szBiosData;
//            while (*p) {
//                if (*p < ' ' || *p >= 127) break;
//                ++p;
//            }
//
//            if(*p == 0) {
//                *ppBiosAddr = pBiosAddr;
//                return (UINT)iLen;
//            }
//        }
//    }
//
//    return 0;
//}
//
//static UINT FindAmiBios (BYTE** ppBiosAddr)
//{
//	BYTE* pBiosAddr = *ppBiosAddr + 0xF478;
//	BYTE szBiosData[128];
//	CopyMemory( szBiosData, pBiosAddr, 127 );
//	szBiosData[127] = 0;
//
//	int iLen = strlen( ( char* )szBiosData );
//	if (iLen > 0 && iLen < 128) {
//		//
//		// Example: "AMI: 51-2300-000000-00101111-030199-"
//		//
//		if (szBiosData[2] == '-' && szBiosData[7] == '-') {
//			BYTE* p = szBiosData;
//			while (* p) {
//				if( * p < ' ' || * p >= 127 ) break;
//				++ p;
//			}
//
//			if (*p == 0) {
//				*ppBiosAddr = pBiosAddr;
//				return (UINT)iLen;
//			}
//		}
//	}
//
//	return 0;
//}
//
//typedef struct _OBJECT_ATTRIBUTES {
//    ULONG Length;//长度 18h
//    HANDLE RootDirectory;//  00000000
//    PUNICODE_STRING ObjectName;//指向对象名的指针
//    ULONG Attributes;//对象属性00000040h
//    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR，0
//    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE，0
//} OBJECT_ATTRIBUTES;
//
//typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES; 
//
//static UINT FindPhoenixBios (BYTE** ppBiosAddr)
//{
//    UINT uOffset[3] = { 0x6577, 0x7196, 0x7550 };
//    for (UINT i = 0; i < 3; ++i) {
//        BYTE* pBiosAddr = * ppBiosAddr + uOffset[i];
//
//        BYTE szBiosData[128];
//        CopyMemory( szBiosData, pBiosAddr, 127 );
//        szBiosData[127] = 0;
//
//        int iLen = strlen ((char*)szBiosData);
//        if (iLen > 0 && iLen < 128) {
//			//
//            // Example: Phoenix "NITELT0.86B.0044.P11.9910111055"
//			//
//            if (szBiosData[7] == '.' && szBiosData[11] == '.') {
//                BYTE* p = szBiosData;
//                while (*p) {
//                    if (* p < ' ' || * p >= 127) break;
//                    ++ p;
//                }
//                if (*p == 0) {
//                    *ppBiosAddr = pBiosAddr;
//                    return (UINT)iLen;
//                }
//            }
//        }
//    }
//
//    return 0;
//}
//
////函数指针变量类型
//typedef DWORD  (__stdcall *ZWOS )( PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES);
//typedef DWORD  (__stdcall *ZWMV )( HANDLE,HANDLE,PVOID,ULONG,ULONG,PLARGE_INTEGER,PSIZE_T,DWORD,ULONG,ULONG);
//typedef DWORD  (__stdcall *ZWUMV )( HANDLE,PVOID); 
//
//wxString GmHardwareInfo::GetBiosSerial ()
//{
//	SIZE_T ssize;
//	LARGE_INTEGER so;
//	so.LowPart				= 0x000f0000;
//	so.HighPart				= 0x00000000;
//	ssize					= 0xffff;
//	char_t strPH[30]		= wxT ("\\device\\physicalmemory"); 
//	DWORD ba				= 0;
//
//	UNICODE_STRING struniph;
//	struniph.Buffer			= strPH;
//	struniph.Length			= 0x2c;
//	struniph.MaximumLength	= 0x2e; 
//
//	OBJECT_ATTRIBUTES obj_ar;
//	obj_ar.Attributes		= 64;
//	obj_ar.Length			= 24;
//	obj_ar.ObjectName		= &struniph;
//	obj_ar.RootDirectory	= 0;
//	obj_ar.SecurityDescriptor		= 0;
//	obj_ar.SecurityQualityOfService = 0; 
//
//	HMODULE hinstLib	= LoadLibrary(wxT ("ntdll.dll"));
//	ZWOS ZWopenS	= (ZWOS)GetProcAddress(hinstLib,"ZwOpenSection");
//	ZWMV ZWmapV		= (ZWMV)GetProcAddress(hinstLib,"ZwMapViewOfSection");
//	ZWUMV ZWunmapV	= (ZWUMV)GetProcAddress(hinstLib,"ZwUnmapViewOfSection"); 
//
//	//
//	//调用函数，对物理内存进行映射
//	//
//	HANDLE hSection;
//	wxString szSerial;
//	if( 0 == ZWopenS(&hSection,4,&obj_ar) && 0 == ZWmapV((HANDLE)hSection,	//打开Section时得到的句柄
//														(HANDLE)-1, 		//将要映射进程的句柄，
//														&ba,				//映射的基址
//														0,
//														0xFFFF,				//分配的大小
//														&so,				//物理内存的地址
//														&ssize,				//指向读取内存块大小的指针
//														1,					//子进程的可继承性设定
//														0,					//分配类型
//														2					//保护类型
//														)) {
//		//
//		//执行后会在当前进程的空间开辟一段64k的空间，并把f000:0000到f000:ffff处的内容映射到这里
//		//映射的基址由ba返回,如果映射不再有用,应该用ZwUnmapViewOfSection断开映射
//		//
//		BYTE* pBiosSerial = (BYTE*)ba;
//		UINT uBiosSerialLen = FindAwardBios (&pBiosSerial);
//		if (uBiosSerialLen == 0U) {
//			uBiosSerialLen = FindAmiBios (&pBiosSerial);
//			if (uBiosSerialLen == 0U) uBiosSerialLen = FindPhoenixBios (&pBiosSerial);
//		}
//
//		if (uBiosSerialLen != 0U) {
//			string str;
//			str.assign ((char*)pBiosSerial, uBiosSerialLen);
//			szSerial = ToWxString (str);
//		}
//
//		ZWunmapV ((HANDLE)0xFFFFFFFF, (void*)ba);
//	}
//
//	return szSerial;
//}

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x)) 
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

void GetAddressOfSpecfiedType (ULONG type, vector<wxString> & addresses)
{
	PIP_ADAPTER_ADDRESSES pAdapterAddresses = NULL;
	ULONG OutBufferLength = 0;
	ULONG RetVal = 0, i; 

	//
    // The size of the buffer can be different between consecutive API calls.
    // In most cases, i < 2 is sufficient;
    // One call to get the size and one call to get the actual parameters.
    // But if one more interface is added or addresses are added, the call again fails with BUFFER_OVERFLOW. 
    // So the number is picked slightly greater than 2. We use i <5 in the example
	//
	for (i = 0; i < 5; i++) {
		RetVal = ::GetAdaptersAddresses(type
										, 0
										, NULL
										, pAdapterAddresses
										, &OutBufferLength);
        
		if (RetVal != ERROR_BUFFER_OVERFLOW) break;
		if (pAdapterAddresses != NULL) FREE(pAdapterAddresses);
        
		pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)MALLOC (OutBufferLength);
		if (pAdapterAddresses == NULL) break;
	}

	// If successful, output some information from the data we received
	PIP_ADAPTER_ADDRESSES pAdapterList = pAdapterAddresses;
	while (pAdapterList) {
		if (pAdapterList->IfType != IF_TYPE_SOFTWARE_LOOPBACK) {
			wxString address;
			for (DWORD index = 0; index < pAdapterList->PhysicalAddressLength; ++index) {
				address += wxString::Format (wxT ("%.2X"), pAdapterList->PhysicalAddress[index] & 0xff);
			}

			addresses.push_back (address);
		}
		pAdapterList = pAdapterList->Next;
	}

	if (pAdapterAddresses != NULL) FREE(pAdapterAddresses);
}

void GmHardwareInfo::GetAdaptersAddresses (vector<wxString> & addresses)
{
	GetAddressOfSpecfiedType (AF_INET, addresses);
	GetAddressOfSpecfiedType (AF_INET6, addresses);
}

int GmHardwareInfo::GetTotalMemory ()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof (status);
	if (!GlobalMemoryStatusEx (&status)) return 0;
	return (int)(status.ullTotalPhys / (1024 * 1024));
}

int GmHardwareInfo::GetFreeMemory ()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof (status);
	if (!GlobalMemoryStatusEx (&status)) return 0;
	return (int)(status.ullAvailPhys / (1024 * 1024));
}

int GmHardwareInfo::GetCPUNumber ()
{
	SYSTEM_INFO siSysInfo;
	GetSystemInfo (&siSysInfo); 
	return siSysInfo.dwNumberOfProcessors;
}

SIGNLETON_IMPL(GmHardwareInfo);