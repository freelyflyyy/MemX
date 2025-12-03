#include "Wow64Runtime.h"
#include "../Common/NtApi/NtApi.h"

namespace MemX {
	Wow64Runtime::Wow64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	Wow64Runtime::~Wow64Runtime() {
	}

	ptr_t Wow64Runtime::getPEB(PEB32* peb) {
		if ( _barrier.targetWow64 == false ) 
			return 0;
		
		PROCESS_BASIC_INFORMATION pbi = { 0 };
		NTSTATUS status = NtQueryInformationProcess(
			_hProcess,
			static_cast<PROCESS_INFORMATION_CLASS>(ProcessWow64Information),
			&pbi,
			sizeof(pbi),
			nullptr
		);

		if ( NT_SUCCESS(status) && peb ) 
			ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(pbi.PebBaseAddress), peb, sizeof(PEB32), nullptr);
		
		return reinterpret_cast<ptr_t>(pbi.PebBaseAddress);
	}

	ptr_t Wow64Runtime::getPEB(PEB64* peb) {
		PROCESS_BASIC_INFORMATION64 pbi = { 0 };
		ULONG returnLength = 0;

		NTSTATUS status = NtWow64QueryInformationProcess64(
			_hProcess,
			static_cast<PROCESS_INFORMATION_CLASS>(ProcessBasicInformation),
			&pbi,
			sizeof(_PROCESS_BASIC_INFORMATION),
			&returnLength
		);

		if ( STATUS_SUCCESS(status) && peb ) {
			NtWow64ReadVirtualMemory64(_hProcess, pbi.PebBaseAddress, peb, sizeof(PEB64), NULL);
		}
		return  reinterpret_cast<ptr_t>(pbi.PebBaseAddress);
	}

	NTSTATUS Wow64Runtime::ReadProcessMemoryT(ptr_t lpBaseAddress, LPVOID lpBuffer, size_t dwSize, SIZE_T* readBytes) {
		// TODO: Implement for WOW64 architecture
		return STATUS_NOT_IMPLEMENTED;
	}

	NTSTATUS Wow64Runtime::WriteProcessMemoryT(ptr_t lpBaseAddress, LPCVOID lpBuffer, size_t dwSize, SIZE_T* writtenBytes) {
		// TODO: Implement for WOW64 architecture
		return STATUS_NOT_IMPLEMENTED;
	}

	NTSTATUS Wow64Runtime::VirtualQueryExT(ptr_t lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) {
		// TODO: Implement for WOW64 architecture
		return STATUS_NOT_IMPLEMENTED;
	}
}