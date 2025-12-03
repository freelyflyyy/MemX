#include "Wow64Runtime.h"
#include "../Common/NtApi/NtApi.h"

namespace MemX {
	Wow64Runtime::Wow64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	Wow64Runtime::~Wow64Runtime() {
	}

	ptr_t Wow64Runtime::getPEB(PEB32* peb) {
		if (!peb) {
			return 0; // Invalid parameter
		}

		ULONG_PTR pebAddress32 = 0;
		ULONG returnLength = 0;

		NTSTATUS status = NtQueryInformationProcess(
			_hProcess,
			static_cast<PROCESS_INFORMATION_CLASS>(ProcessWow64Information),
			&pebAddress32,
			sizeof(ULONG_PTR),
			&returnLength
		);

		if (NT_SUCCESS(status) && pebAddress32 != 0) {
			SIZE_T bytesRead;
			status = ReadProcessMemoryT((ptr_t)pebAddress32, peb, sizeof(PEB32), &bytesRead);
			if (NT_SUCCESS(status) && bytesRead == sizeof(PEB32)) {
				return (ptr_t)pebAddress32;
			}
		}
		
		return 0;
	}

	ptr_t Wow64Runtime::getPEB(PEB64* peb) {
		return ptr_t();
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