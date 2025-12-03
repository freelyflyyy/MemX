#include "Wow64Runtime.h"
#include "MemX/Common/Utils/Wow64Utils.h"
#include "../Common/NtApi/NtApi.h"
#include "../Common/NtApi/NtCallExt.h"
#include <iostream>

namespace MemX {
	Wow64Runtime::Wow64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	Wow64Runtime::~Wow64Runtime() {
	}

	NTSTATUS Wow64Runtime::GetTargetPeb(PEB32* peb) {
		if ( !peb ) {
			return STATUS_INVALID_PARAMETER;
		}
		if ( _barrier.targetWow64 == false ) {
			return 0;
		}
		PTR_T pbi = 0;
		ULONG bytes = 0;
		static DWORD64 pNtQueryInfo = GET_NTFUNC("NtQueryInformationProcess");
		if ( !pNtQueryInfo ) return STATUS_ACCESS_VIOLATION;
		NTSTATUS queryInfoRet = CALL64_FUNC(pNtQueryInfo,
									  (DWORD64) _hProcess,
									  (DWORD64) ProcessWow64Information,
									  (DWORD64) &pbi,
									  (DWORD64) sizeof(pbi),
									  (DWORD64) &bytes);
		if ( !NT_SUCCESS(queryInfoRet) ) {
			return queryInfoRet;
		}
		return ReadProcessMemoryT(pbi, peb, sizeof(PEB32), NULL);
	}

	NTSTATUS Wow64Runtime::GetTargetPeb(PEB64* peb) {
		if ( !peb ) {
			return STATUS_INVALID_PARAMETER;
		}
		PROCESS_BASIC_INFORMATION64 pbi = { 0 };
		ULONG bytes = 0;
		static DWORD64 pNtQueryInfo = GET_NTFUNC("NtQueryInformationProcess");
		if ( !pNtQueryInfo ) return STATUS_ACCESS_VIOLATION;
		NTSTATUS queryInfoRet = CALL64_FUNC(pNtQueryInfo,
									  (DWORD64) _hProcess,
									  (DWORD64) ProcessBasicInformation,
									  (DWORD64) &pbi,
									  (DWORD64) sizeof(pbi),
									  (DWORD64) &bytes);
		if ( !NT_SUCCESS(queryInfoRet) ) {
			return queryInfoRet;
		}
		return ReadProcessMemoryT(pbi.PebBaseAddress, peb, sizeof(PEB64), NULL);
	}

	NTSTATUS Wow64Runtime::ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) {
		static DWORD64 pNtReadVM = GET_NTFUNC("NtReadVirtualMemory");
		if ( !pNtReadVM ) {
			return STATUS_ACCESS_VIOLATION;
		}
		NTSTATUS ntRet = CALL64_FUNC(pNtReadVM,
					  (DWORD64) _hProcess,
					  (DWORD64) lpBaseAddress,
					  (DWORD64) lpBuffer,
					  (DWORD64) dwSize,
					  (DWORD64) readBytes);
		return ntRet;
	}

	NTSTATUS Wow64Runtime::WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes) {
		static DWORD64 pNtWriteVM = GET_NTFUNC("NtWriteVirtualMemory");
		if ( !pNtWriteVM ) {
			return STATUS_ACCESS_VIOLATION;
		}
		NTSTATUS ntRet = CALL64_FUNC(pNtWriteVM,
					  (DWORD64) _hProcess,
					  (DWORD64) lpBaseAddress,
					  (DWORD64) lpBuffer,
					  (DWORD64) dwSize,
					  (DWORD64) writtenBytes);
		return ntRet;
	}

	NTSTATUS Wow64Runtime::VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) {
		DWORD64 returnLength = 0;
		return STATUS_NOT_SUPPORTED;
	}
}
