#pragma once
#include "../Common/WinApi/WinHeaders.h"
#include "../Runtime/Runtime.h"

namespace MemX {
	class ProcessCore {
		public:
		BOOL isWow64() const { return _arch.targetWow64; }
		BOOL isWow64Self() const { return _arch.sourceWow64; }
		DWORD getTargetPeb(PEB32* peb) const { return _runtime->GetTargetPeb(peb); }
		DWORD64 getTargetPeb(PEB64* peb) const { return _runtime->GetTargetPeb(peb); }
		DWORD getPid() const { return this->_pid; }
		HANDLE getHandle() const { return this->_hProcess; }
		Runtime* getRuntime() const { return _runtime.get(); }
		BOOL isActive() const { return _active; }


		private:
		friend class Process;

		private:
		ProcessCore();
		ProcessCore(const Process&) = delete;
		ProcessCore& operator=(const Process&) = delete;
		~ProcessCore();

		NTSTATUS Open(DWORD pid, DWORD access);

		NTSTATUS Open(HANDLE proHandle);

		NTSTATUS Init();

		void Close();

		private:
		HANDLE _hProcess = nullptr;
		DWORD _pid = 0;
		BOOL _active = FALSE;
		ARCHITECHURE _arch = { 0 };
		std::unique_ptr<Runtime> _runtime = nullptr;
	};
}