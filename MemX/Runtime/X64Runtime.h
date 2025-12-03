#pragma once

#include "Runtime.h"

namespace MemX {
	class X64Runtime : public Runtime {
	public:
		X64Runtime(HANDLE hProcess);
		virtual ~X64Runtime();

		virtual NTSTATUS GetTargetPeb(PEB32* peb) override;

		virtual NTSTATUS GetTargetPeb(PEB64* peb) override;

		virtual NTSTATUS ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) override;

		virtual NTSTATUS WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes = NULL) override;

		virtual NTSTATUS VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) override;
	};
}
