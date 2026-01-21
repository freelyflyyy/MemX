#pragma once
#include "../Common/WinApi/WinHeaders.h"
#include "../Runtime/Runtime.h"
#include <memory>

namespace MemX {
	class XContext {
		public:
		XContext();
		~XContext();

		NTSTATUS Open(DWORD pid, DWORD access);
		NTSTATUS Open(HANDLE proHandle);
		void Close();

		BOOL IsWow64() const { return _arch.targetWow64; }
		BOOL IsActive() const { return _active; }
		DWORD GetPid() const { return _pid; }
		HANDLE GetHandle() const { return _hProcess; }

		Runtime* GetRuntime() const { return _runtime.get(); }

		private:
		NTSTATUS Init();

		private:
		HANDLE _hProcess = nullptr;
		DWORD _pid = 0;
		BOOL _active = FALSE;
		ARCHITECHURE _arch = { 0 };
		std::unique_ptr<Runtime> _runtime = nullptr;
	};
}