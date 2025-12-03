#pragma once

#include "../WinApi/WinHeaders.h"
#include "NtCallExt.h"

namespace MemX {

	typedef enum _MEMORY_INFORMATION_CLASS {
		MemoryBasicInformation = 0,
		MemoryWorkingSetInformation = 1,
		MemoryMappedFilenameInformation = 2,
		MemoryRegionInformation = 3,
		MemoryWorkingSetExInformation = 4,
		MemorySharedCommitInformation = 5,
		MemoryImageInformation = 6,
		MemoryRegionInformationEx = 7,
		MemoryPrivilegedBasicInformation = 8
	} MEMORY_INFORMATION_CLASS;

	template<typename T>
	struct _PROCESS_BASIC_INFORMATION_T {
		NTSTATUS ExitStatus;
		UINT32    Reserved0;
		T	     PebBaseAddress;
		T	     AffinityMask;
		LONG	 BasePriority;
		ULONG	 Reserved1;
		T	     uUniqueProcessId;
		T	     uInheritedFromUniqueProcessId;
	};

	typedef _PROCESS_BASIC_INFORMATION_T<DWORD> PROCESS_BASIC_INFORMATION32;
	typedef _PROCESS_BASIC_INFORMATION_T<DWORD64> PROCESS_BASIC_INFORMATION64;

	#pragma pack(push, 1)
	template <class T>
	struct _LIST_ENTRY_T {
		T Flink;
		T Blink;
	};

	template <class T>
	struct _UNICODE_STRING_T {
		union {
			struct {
				WORD Length;
				WORD MaximumLength;
			};
			T dummy;
		};
		T Buffer;
	};

	template <class T>
	struct _NT_TIB_T {
		T ExceptionList;
		T StackBase;
		T StackLimit;
		T SubSystemTib;
		T FiberData;
		T ArbitraryUserPointer;
		T Self;
	};

	template <class T>
	struct _CLIENT_ID_T {
		T UniqueProcess;
		T UniqueThread;
	};

	template <class T>
	struct _TEB_T_ {
		_NT_TIB_T<T> NtTib;
		T EnvironmentPointer;
		_CLIENT_ID_T<T> ClientId;
		T ActiveRpcHandle;
		T ThreadLocalStoragePointer;
		T ProcessEnvironmentBlock;
		DWORD LastErrorValue;
		DWORD CountOfOwnedCriticalSections;
		T CsrClientThread;
		T Win32ThreadInfo;
		DWORD User32Reserved[ 26 ];
		//rest of the structure is not defined for now, as it is not needed
	};

	template <class T>
	struct _LDR_DATA_TABLE_ENTRY_T {
		_LIST_ENTRY_T<T> InLoadOrderLinks;
		_LIST_ENTRY_T<T> InMemoryOrderLinks;
		_LIST_ENTRY_T<T> InInitializationOrderLinks;
		T DllBase;
		T EntryPoint;
		union {
			DWORD SizeOfImage;
			T dummy01;
		};
		_UNICODE_STRING_T<T> FullDllName;
		_UNICODE_STRING_T<T> BaseDllName;
		DWORD Flags;
		WORD LoadCount;
		WORD TlsIndex;
		union {
			_LIST_ENTRY_T<T> HashLinks;
			struct {
				T SectionPointer;
				T CheckSum;
			};
		};
		union {
			T LoadedImports;
			DWORD TimeDateStamp;
		};
		T EntryPointActivationContext;
		T PatchInformation;
		_LIST_ENTRY_T<T> ForwarderLinks;
		_LIST_ENTRY_T<T> ServiceTagLinks;
		_LIST_ENTRY_T<T> StaticLinks;
		T ContextInformation;
		T OriginalBase;
		_LARGE_INTEGER LoadTime;
	};

	template <class T>
	struct _PEB_LDR_DATA_T {
		DWORD Length;
		DWORD Initialized;
		T SsHandle;
		_LIST_ENTRY_T<T> InLoadOrderModuleList;
		_LIST_ENTRY_T<T> InMemoryOrderModuleList;
		_LIST_ENTRY_T<T> InInitializationOrderModuleList;
		T EntryInProgress;
		DWORD ShutdownInProgress;
		T ShutdownThreadId;
	};

	template <class T, class NGF, int A>
	struct _PEB_T {
		union {
			struct {
				BYTE InheritedAddressSpace;
				BYTE ReadImageFileExecOptions;
				BYTE BeingDebugged;
				BYTE BitField;
			};
			T dummy01;
		};
		T Mutant;
		T ImageBaseAddress;
		T Ldr;
		T ProcessParameters;
		T SubSystemData;
		T ProcessHeap;
		T FastPebLock;
		T AtlThunkSListPtr;
		T IFEOKey;
		T CrossProcessFlags;
		T UserSharedInfoPtr;
		DWORD SystemReserved;
		DWORD AtlThunkSListPtr32;
		T ApiSetMap;
		T TlsExpansionCounter;
		T TlsBitmap;
		DWORD TlsBitmapBits[ 2 ];
		T ReadOnlySharedMemoryBase;
		T HotpatchInformation;
		T ReadOnlyStaticServerData;
		T AnsiCodePageData;
		T OemCodePageData;
		T UnicodeCaseTableData;
		DWORD NumberOfProcessors;
		union {
			DWORD NtGlobalFlag;
			NGF dummy02;
		};
		LARGE_INTEGER CriticalSectionTimeout;
		T HeapSegmentReserve;
		T HeapSegmentCommit;
		T HeapDeCommitTotalFreeThreshold;
		T HeapDeCommitFreeBlockThreshold;
		DWORD NumberOfHeaps;
		DWORD MaximumNumberOfHeaps;
		T ProcessHeaps;
		T GdiSharedHandleTable;
		T ProcessStarterHelper;
		T GdiDCAttributeList;
		T LoaderLock;
		DWORD OSMajorVersion;
		DWORD OSMinorVersion;
		WORD OSBuildNumber;
		WORD OSCSDVersion;
		DWORD OSPlatformId;
		DWORD ImageSubsystem;
		DWORD ImageSubsystemMajorVersion;
		T ImageSubsystemMinorVersion;
		T ActiveProcessAffinityMask;
		T GdiHandleBuffer[ A ];
		T PostProcessInitRoutine;
		T TlsExpansionBitmap;
		DWORD TlsExpansionBitmapBits[ 32 ];
		T SessionId;
		ULARGE_INTEGER AppCompatFlags;
		ULARGE_INTEGER AppCompatFlagsUser;
		T pShimData;
		T AppCompatInfo;
		_UNICODE_STRING_T<T> CSDVersion;
		T ActivationContextData;
		T ProcessAssemblyStorageMap;
		T SystemDefaultActivationContextData;
		T SystemAssemblyStorageMap;
		T MinimumStackCommit;
		T FlsCallback;
		_LIST_ENTRY_T<T> FlsListHead;
		T FlsBitmap;
		DWORD FlsBitmapBits[ 4 ];
		T FlsHighIndex;
		T WerRegistrationData;
		T WerShipAssertPtr;
		T pContextData;
		T pImageHeaderHash;
		T TracingFlags;
	};
	#pragma pack(pop)

	typedef _LDR_DATA_TABLE_ENTRY_T<DWORD> LDR_DATA_TABLE_ENTRY32;
	typedef _LDR_DATA_TABLE_ENTRY_T<DWORD64> LDR_DATA_TABLE_ENTRY64;

	typedef _TEB_T_<DWORD> TEB32;
	typedef _TEB_T_<DWORD64> TEB64;

	typedef _PEB_LDR_DATA_T<DWORD> PEB_LDR_DATA32;
	typedef _PEB_LDR_DATA_T<DWORD64> PEB_LDR_DATA64;

	typedef _PEB_T<DWORD, DWORD64, 34> PEB32;
	typedef _PEB_T<DWORD64, DWORD, 30> PEB64;

	// NTSTATUS offset in TEB structure for different architectures
	#ifdef _WIN64
	constexpr size_t NT_STATUS_OFFSET = 0x1250;
	#else
	constexpr size_t NT_STATUS_OFFSET = 0x00B4;
	#endif

	inline NTSTATUS GetLastNtStatus() {
		return *(NTSTATUS*) ((BYTE*) NtCurrentTeb() + NT_STATUS_OFFSET);
	}

	inline void SetLastNtStatus(NTSTATUS status) {
		*(NTSTATUS*) ((BYTE*) NtCurrentTeb() + NT_STATUS_OFFSET) = status;
	}

}