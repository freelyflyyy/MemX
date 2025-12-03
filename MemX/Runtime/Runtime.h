#pragma once

#include "../Common/WinApi/WinHeaders.h"
#include "../Common/WinApi/ArchitectureApi.h"
#include "../Common/NtApi/NtApi.h"
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

		PTR_T maxAddr32() {
			return 0x7FFFFFFF;
		}

		PTR_T maxAddr64() {
			return 0x7FFFFFFFFFFF;
		}

		BOOL isTargetWow64() const {
			return _barrier.targetWow64;
		}

		virtual NTSTATUS GetTargetPeb(PEB32* peb) = 0;
		virtual NTSTATUS GetTargetPeb(PEB64* peb) = 0;

		virtual NTSTATUS ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) = 0;

		virtual NTSTATUS WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes = NULL) = 0;

		virtual NTSTATUS VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) = 0;

		protected:
		HANDLE _hProcess;
		Wow64Barrier _barrier;
	};
}

