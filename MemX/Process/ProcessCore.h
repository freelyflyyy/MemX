#pragma once
#include "../Common/WinApi/WinHeaders.h"
#include "../Runtime/Runtime.h"

#include <memory>


namespace MemX {
	class ProcessCore {
		public:
		MEMX_API BOOL isWow64() const { return _runtime->isTargetWow64(); }// TODO: Implement isWow64 check
		MEMX_API DWORD getTargetPeb(PEB32* peb) const { return _runtime->GetTargetPeb(peb); }
		MEMX_API DWORD64 getTargetPeb(PEB64* peb) const { return _runtime->GetTargetPeb(peb); }
		MEMX_API DWORD getPid() const { return this->_pid; }
		MEMX_API HANDLE getHandle() const { return this->_hProcess; }
		MEMX_API Runtime* getRuntime() const { return _runtime.get(); }
		MEMX_API BOOL isActive() const { return _active; }
		

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
		std::unique_ptr<Runtime> _runtime = nullptr;
	};
}