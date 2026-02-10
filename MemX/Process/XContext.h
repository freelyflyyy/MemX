#pragma once
#include "../Common/WinApi/WinHeaders.h"
#include "../Runtime/Runtime.h"
#include "../Common/Types/XStatus.h"
#include <memory>

namespace MemX {
	class XContext {
		public:
		XContext();
		~XContext();

		XStatus Attach(DWORD pid, DWORD access);
		XStatus Attach(std::wstring processName, DWORD access);
		XStatus Attach(HANDLE proHandle);
		VOID Close();

		BOOL IsWow64() const { return _arch.targetWow64; }
		BOOL IsActive() const { return _active; }
		DWORD GetPid() const { return _pid; }
		HANDLE GetHandle() const { return _hProcess; }

		Runtime* GetRuntime() const { return _runtime.get(); }

		private:
		XStatus Init();

		private:
		HANDLE _hProcess = nullptr;
		DWORD _pid = 0;
		BOOL _active = FALSE;
		ARCHITECHURE _arch = { 0 };
		std::unique_ptr<Runtime> _runtime = nullptr;
	};
}