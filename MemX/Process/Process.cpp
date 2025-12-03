#include "Process.h"
#include "../Common/NtApi/Bootstrap.h"

namespace MemX {
    Process::Process() 
        :_core(),
        _memory(this) {
        //Ensure weight lifting and successful initialization of kernel functions
        //Bootstrap();
    }
    Process::~Process(void) {
    }

    MEMX_API NTSTATUS Process::Catch(DWORD pid, DWORD access) {
        Drop();
        return _core.Open(pid, access);
    }

    MEMX_API NTSTATUS Process::Catch(const wchar_t* processName, DWORD access) {
		DWORD pid = GetPidByName(processName);
		return pid >= 0 ? Catch(pid, access) : STATUS_NOT_FOUND;
    }

    MEMX_API NTSTATUS Process::Catch(HANDLE proHandle) {
        Drop();
        return _core.Open(proHandle);
    }

    MEMX_API NTSTATUS Process::Drop() {
		_core.Close();
		return STATUS_SUCCESS;
    }

    MEMX_API NTSTATUS Process::GetPidByName(const std::wstring& processName) {
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if ( hSnap == INVALID_HANDLE_VALUE ) {
			return GetLastNtStatus();
        }
		PROCESSENTRY32W pe32 = { 0 };
		pe32.dwSize = sizeof(PROCESSENTRY32W);

        if ( !Process32FirstW(hSnap, &pe32) ) {
            CloseHandle(hSnap);
            return STATUS_UNSUCCESSFUL;
        }
        do {
            if ( _wcsicmp(pe32.szExeFile, processName.data()) == 0 ) {
                CloseHandle(hSnap);
                return pe32.th32ProcessID;
            }
        } while ( Process32Next(hSnap, &pe32) );
        CloseHandle(hSnap);
        return pe32.th32ProcessID;
    }
}
