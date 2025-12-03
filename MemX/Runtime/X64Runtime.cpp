#include "X64Runtime.h"
#include "../Common/NtApi/NtApi.h"

namespace MemX {
	X64Runtime::X64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	X64Runtime::~X64Runtime() {
	}

	NTSTATUS X64Runtime::GetTargetPeb(PEB32* peb) {
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

	NTSTATUS X64Runtime::GetTargetPeb(PEB64* peb) {
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

	NTSTATUS X64Runtime::ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) {
		BOOL dwRet = ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(lpBaseAddress), lpBuffer, dwSize, reinterpret_cast<SIZE_T*>(readBytes));
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes) {
		BOOL dwRet = WriteProcessMemory(_hProcess, reinterpret_cast<LPVOID>(lpBaseAddress), lpBuffer, dwSize, reinterpret_cast<SIZE_T*>(writtenBytes));
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) {
		SIZE_T dwRet = VirtualQueryEx(_hProcess, reinterpret_cast<LPCVOID>(lpAddress), (PMEMORY_BASIC_INFORMATION)lpBuffer, sizeof(MEMORY_BASIC_INFORMATION64));
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}
}