#include "XContext.h"
#include "../Common/NtApi/NtApi.h"
#include "../Common/WinApi/ArchitectureApi.h"
#include "../Runtime/X64Runtime.h"
#include "../Runtime/Wow64Runtime.h"
#include <VersionHelpers.h>

namespace MemX {
	XContext::XContext() : _runtime(nullptr) {}
	XContext::~XContext() { Close(); }

	NTSTATUS XContext::Open(DWORD pid, DWORD access) {
		Close();

		if ( pid == GetCurrentProcessId() ) {
			_hProcess = GetCurrentProcess();
			_active = TRUE;
		} else {
			_hProcess = OpenProcess(access, FALSE, pid);
		}

		if ( IsWindows10OrGreater() && pid == GetCurrentProcessId() ) {
			_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		}

		if ( _hProcess ) {
			_pid = pid;
			_active = TRUE;
			return Init();
		}
		return GetLastNtStatus();
	}

	NTSTATUS XContext::Open(HANDLE proHandle) {
		Close();
		_hProcess = proHandle;
		_pid = GetProcessId(_hProcess);

		if ( IsWindows10OrGreater() && _pid == GetCurrentProcessId() )
			_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _pid);

		if ( _hProcess ) {
			_active = TRUE;
		}
		return Init();
	}

	NTSTATUS XContext::Init() {
		ArchitectureApi texApi(_hProcess);
		_arch = texApi.GetArchitechure();

		if ( !_arch.sourceWow64 ) {
			_runtime = std::make_unique<X64Runtime>(_hProcess);
		} else {
			_runtime = std::make_unique<Wow64Runtime>(_hProcess);
		}
		return STATUS_SUCCESS;
	}

	void XContext::Close() {
		if ( _hProcess && _hProcess != GetCurrentProcess() ) {
			CloseHandle(_hProcess);
		}
		_hProcess = nullptr;
		_runtime.reset();
		_pid = 0;
		_active = FALSE;
	}
}