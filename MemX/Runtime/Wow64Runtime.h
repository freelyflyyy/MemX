#pragma once

#include "Runtime.h"

namespace MemX {
	class Wow64Runtime : public Runtime {
	public:
		Wow64Runtime(HANDLE hProcess);
		virtual ~Wow64Runtime();

		virtual ptr_t getPEB(PEB32* peb) override;

		virtual ptr_t getPEB(PEB64* peb) override;

		virtual NTSTATUS ReadProcessMemoryT(ptr_t lpBaseAddress, LPVOID lpBuffer, size_t dwSize, SIZE_T* readBytes) override;

		virtual NTSTATUS WriteProcessMemoryT(ptr_t lpBaseAddress, LPCVOID lpBuffer, size_t dwSize, SIZE_T* writtenBytes = NULL) override;

		virtual NTSTATUS VirtualQueryExT(ptr_t lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) override;
	};
}
