#include "XContext.h"
#include "../Common/NtApi/NtApi.h"
#include "../Common/WinApi/ArchitectureApi.h"
#include "../Runtime/X64Runtime.h"
#include "../Runtime/Wow64Runtime.h"
#include "../Common/Utils/Utils.h"
#include <VersionHelpers.h>

namespace MemX {
	XContext::XContext() : _runtime(nullptr) {}
	XContext::~XContext() { Close(); }

	XStatus XContext::Attach(DWORD pid, DWORD access) {
		Close();
		if ( pid == 0 || access == 0 ) {
			return XStatus::Fail(L"Please check invalid pid and Access");
		}

		if ( pid == GetCurrentProcessId() ) {
			_hProcess = GetCurrentProcess();
			_active = TRUE;
		} else {
			_hProcess = OpenProcess(access, FALSE, pid);
		}

		if ( IsWindows10OrGreater() && pid == GetCurrentProcessId() ) {
			HANDLE enhancedHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
			if ( enhancedHandle ) {
				//If it is a pseudo handle, it will not be closed
				if ( _hProcess && _hProcess != GetCurrentProcess() ) {
					CloseHandle(_hProcess);
				}
				_hProcess = enhancedHandle;
			}
		}

		if ( _hProcess ) {
			_pid = pid;
			_active = TRUE;
			return Init();
		}
		return XStatus::Fail(GetSysErrMsg());
	}

	XStatus XContext::Attach(std::wstring processName, DWORD access) {
		Close();

		if ( processName.empty() || !access ) {
			return XStatus::Fail(L"Please check invalid process name and access");
		}
		DWORD pid = 0;
		if ( !NT_SUCCESS(GetPidByName(processName, pid)) ) {
			return XStatus::Fail(L"Failed to find process by name: " + processName);
		}
		return Attach(pid, access);
	}

	XStatus XContext::Attach(HANDLE proHandle) {
		Close();

		if ( !proHandle || proHandle == INVALID_HANDLE_VALUE ) {
			return XStatus::Fail(L"Try to attach an invalid handle");
		}
		_hProcess = proHandle;
		_pid = GetProcessId(_hProcess);

		if ( IsWindows10OrGreater() && _pid == GetCurrentProcessId() ) {
			HANDLE enhancedHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _pid);
			/*
				If you see this, please note that 
			the previously passed handle will not be closed here
			*/
			if ( enhancedHandle ) {
				_hProcess = enhancedHandle;
			}
		}

		if ( _hProcess ) {
			_active = TRUE;
			return Init();
		}
		return XStatus::Fail(L"Failed to attach the process. Please check the handle and process privileges");
	}

	XStatus XContext::Init() {
		ArchitectureApi texApi(_hProcess);
		_arch = texApi.GetArchitechure();

		if ( !_arch.sourceWow64 ) {
			_runtime = std::make_unique<X64Runtime>(_pid, _hProcess);
		} else {
			_runtime = std::make_unique<Wow64Runtime>(_pid, _hProcess);
		}
		return XStatus::Success();
	}

	VOID XContext::Close() {
		if ( _hProcess && _hProcess != GetCurrentProcess() ) {
			CloseHandle(_hProcess);
		}
		_hProcess = nullptr;
		_runtime.reset();
		_pid = 0;
		_active = FALSE;
	}
}