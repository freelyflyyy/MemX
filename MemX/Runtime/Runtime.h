#pragma once

#include "../Common/WinApi/WinHeaders.h"
#include "../Common/WinApi/ArchitectureApi.h"
#include "../Common/NtApi/NtStructures.h"
#include "../Common/Types/Type.h"
#include "../Config.h"



namespace MemX {
	class Runtime {
		public:
		MEMX_API Runtime(HANDLE hProcess) : _hProcess(hProcess) {
			ArchitectureApi texApi(hProcess);
			_barrier = texApi.GetWow64Barrier();
		};
		MEMX_API ~Runtime() {};

		virtual ptr_t getPEB(PEB32* peb) { return 0; };
		virtual ptr_t getPEB(PEB64* peb) { return 0; };

		virtual NTSTATUS ReadProcessMemoryT(ptr_t lpBaseAddress, LPVOID lpBuffer, size_t dwSize, SIZE_T* readBytes) = 0;

		virtual NTSTATUS WriteProcessMemoryT(ptr_t lpBaseAddress, LPCVOID lpBuffer, size_t dwSize, SIZE_T* writtenBytes = NULL) = 0;

		virtual NTSTATUS VirtualQueryExT(ptr_t lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) = 0;

		protected:
		HANDLE _hProcess;
		Wow64Barrier _barrier;
	};
}

