#pragma once
#include "NtApi.h"
#include <atomic>

namespace MemX {
	std::atomic<bool> done(false);

	//Promote permissions
	MEMX_API NTSTATUS EnablePrivilege(LPCWSTR lpszPrivilege) {
		HANDLE hToken = nullptr;
		TOKEN_PRIVILEGES tp = { 0 };
		LUID luid;

		if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, FALSE, &hToken)) {
			if (GetLastError() != ERROR_NO_TOKEN) {
				return GetLastNtStatus();
			}

			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
				return GetLastNtStatus();
			}
		}

		if (!LookupPrivilegeValueW(NULL, lpszPrivilege, &luid) ) {
			CloseHandle(hToken);
			return GetLastNtStatus();
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
			CloseHandle(hToken);
			return GetLastNtStatus();
		}

		DWORD dwError = GetLastError();
		CloseHandle(hToken);
		
		if (dwError == ERROR_NOT_ALL_ASSIGNED) {
			return STATUS_PRIVILEGE_NOT_HELD;
		}

		return (dwError == ERROR_SUCCESS) ? STATUS_SUCCESS : GetLastNtStatus();
	}


	MEMX_API void Bootstrap() {
		//Ensure that Nt functions are loaded only once
		bool expected = false;
		if ( done.compare_exchange_strong(expected, true) ) {
			EnablePrivilege(SE_DEBUG_NAME);
		}
	}
}
