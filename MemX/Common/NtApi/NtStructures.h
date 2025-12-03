#pragma once
#include "../WinApi/WinHeaders.h"

namespace MemX {
	typedef struct PEB_BITFIELD {
		BOOLEAN ImageUsesLargePages : 1;            // ʹ�ô�ҳ
		BOOLEAN IsProtectedProcess : 1;             // �ܱ�������
		BOOLEAN IsImageDynamicallyRelocated : 1;    // ��ַ���ض�λ
		BOOLEAN SkipPatchingUser32Forwarders : 1;   // ���� User32 ת��
		BOOLEAN IsPackagedProcess : 1;              // APPX/MSIX ������
		BOOLEAN IsAppContainer : 1;                 // AppContainer
		BOOLEAN IsProtectedProcessLight : 1;        // PPL ����
		BOOLEAN IsLongPathAwareProcess : 1;         // ֧�ֳ�·��
	} PEB_BITFIELD;

	typedef struct _PEB32 {
		BOOLEAN InheritedAddressSpace;      // 0x00
		BOOLEAN ReadImageFileExecOptions;   // 0x01
		BOOLEAN BeingDebugged;              // 0x02
		union {
			BOOLEAN BitField;               // 0x03
			PEB_BITFIELD Bits;
		};

		DWORD Mutant;                       // 0x04 (HANDLE -> DWORD)
		DWORD ImageBaseAddress;             // 0x08 (PVOID -> DWORD)
		DWORD Ldr;                          // 0x0C (PPEB_LDR_DATA -> DWORD)
		DWORD ProcessParameters;            // 0x10 (PRTL_USER... -> DWORD)
		DWORD SubSystemData;                // 0x14
		DWORD ProcessHeap;                  // 0x18

		BYTE  Padding1[ 0x4C ];
		DWORD NtGlobalFlag;                 // 0x68
		BYTE  Padding2[ 0x38 ];
		DWORD OSMajorVersion;               // 0xA4
		DWORD OSMinorVersion;               // 0xA8
		USHORT OSBuildNumber;               // 0xAC
		USHORT OSCSDVersion;                // 0xAE
	} PEB32, * PPEB32;

	typedef struct _PEB64 {
		BOOLEAN InheritedAddressSpace;      // 0x00
		BOOLEAN ReadImageFileExecOptions;   // 0x01
		BOOLEAN BeingDebugged;              // 0x02
		union {
			BOOLEAN BitField;               // 0x03
			PEB_BITFIELD Bits;
		};

		BYTE Padding0[ 4 ];

		DWORD64 Mutant;                     // 0x08 (HANDLE -> DWORD64)
		DWORD64 ImageBaseAddress;           // 0x10 (PVOID -> DWORD64)
		DWORD64 Ldr;                        // 0x18 (PPEB_LDR_DATA -> DWORD64)
		DWORD64 ProcessParameters;          // 0x20
		DWORD64 SubSystemData;              // 0x28
		DWORD64 ProcessHeap;                // 0x30

		BYTE  Padding1[ 0x84 ];
		DWORD NtGlobalFlag;                 // 0xBC
		BYTE  Padding2[ 0x58 ];
		DWORD OSMajorVersion;               // 0x118
		DWORD OSMinorVersion;               // 0x11C
		USHORT OSBuildNumber;               // 0x120
		USHORT OSCSDVersion;                // 0x122

	} PEB64, * PPEB64;

	#ifdef _WIN64
	#define PEB_T PEB64;
	#define PPEB_T PPEB64;
	#else
	#define PEB_T PEB32;
	#define PPEB_T PPEB32;
	#endif // _WIN64

}