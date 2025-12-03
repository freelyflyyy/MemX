#include "X64Runtime.h"
#include "../Common/NtApi/NtApi.h"

namespace MemX {
	X64Runtime::X64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	X64Runtime::~X64Runtime() {
	}

	ptr_t X64Runtime::getPEB(PEB32* peb) {
		if ( _barrier.targetWow64 == false ) {
			return 0;
		} else {
			
		}
		return ptr_t();
	}

	ptr_t X64Runtime::getPEB(PEB64* peb) {
		if (!peb) {
			return 0; // Invalid parameter
		}

		PROCESS_BASIC_INFORMATION pbi;
		ULONG returnLength;
		NTSTATUS status = NtQueryInformationProcess(
			_hProcess,
			static_cast<PROCESS_INFORMATION_CLASS>(ProcessBasicInformation),
			&pbi,
			sizeof(PROCESS_BASIC_INFORMATION),
			&returnLength
		);

		if (NT_SUCCESS(status)) {
			ptr_t pebAddress = (ptr_t)pbi.PebBaseAddress;
			SIZE_T bytesRead;
			status = ReadProcessMemoryT(pebAddress, peb, sizeof(PEB64), &bytesRead);
			if (NT_SUCCESS(status) && bytesRead == sizeof(PEB64)) {
				return pebAddress;
			}
		}
		return 0;
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