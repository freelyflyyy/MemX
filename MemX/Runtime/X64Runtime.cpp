#include "X64Runtime.h"
#include "../Common/NtApi/NtApi.h"

namespace MemX {
	X64Runtime::X64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	X64Runtime::~X64Runtime() {
	}

	ptr_t X64Runtime::getPEB(PEB32* peb) {
		if(_barrier.targetWow64 == false)
			return 0;

		ptr_t pebAddress = 0;
		NTSTATUS status = NtQueryInformationProcess(
			_hProcess,
			static_cast<PROCESS_INFORMATION_CLASS>(ProcessWow64Information),
			&pebAddress,
			sizeof(ptr_t),
			nullptr
		);

		if ( NT_SUCCESS(status) && peb )
			ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(pebAddress), peb, sizeof(PEB32), nullptr);

		return pebAddress;
	}

	ptr_t X64Runtime::getPEB(PEB64* peb) {
		PROCESS_BASIC_INFORMATION pbi = { 0 };
		ULONG returnLength = 0;

		NTSTATUS status = NtQueryInformationProcess(
			_hProcess,
			static_cast<PROCESS_INFORMATION_CLASS>(ProcessBasicInformation),
			&pbi,
			sizeof(PROCESS_BASIC_INFORMATION),
			&returnLength
		);

		if ( STATUS_SUCCESS(status) && peb ) {
			ReadProcessMemory(_hProcess, pbi.PebBaseAddress, peb, sizeof(PEB64), NULL);
		}
		return  reinterpret_cast<ptr_t>(pbi.PebBaseAddress);
	}

	NTSTATUS X64Runtime::ReadProcessMemoryT(ptr_t lpBaseAddress, LPVOID lpBuffer, size_t dwSize, SIZE_T* readBytes) {
		DWORD dwRet = ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(lpBaseAddress), lpBuffer, dwSize, readBytes);
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::WriteProcessMemoryT(ptr_t lpBaseAddress, LPCVOID lpBuffer, size_t dwSize, SIZE_T* writtenBytes) {
		DWORD dwRet = WriteProcessMemory(_hProcess, reinterpret_cast<LPVOID>(lpBaseAddress), lpBuffer, dwSize, writtenBytes);
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::VirtualQueryExT(ptr_t lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) {
		DWORD64 dwRet = VirtualQueryEx(_hProcess, reinterpret_cast<LPCVOID>(lpAddress), (PMEMORY_BASIC_INFORMATION) lpBuffer, sizeof(MEMORY_BASIC_INFORMATION64));
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}
}