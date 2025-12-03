#pragma once
#include "NtApiResolver.h"
#include "NtApi.h"

namespace MemX {
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

		if (!LookupPrivilegeValueW(NULL, lpszPrivilege, &luid)) {
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

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
			CloseHandle(hToken);
			return GetLastNtStatus();
		}

		CloseHandle(hToken);
		return STATUS_SUCCESS;
	}


	void loadNtFunc() {
		HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
		if (hNtdll == nullptr) {
			return;
		}

		//init ntfunc pointers
		pfnNtQuerySystemInformation = reinterpret_cast<fnNtQuerySystemInformation>(NtApiResolver::Instance().loadFunc(hNtdll, L"NtQuerySystemInformation"));
		pfnNtQueryInformationProcess = reinterpret_cast<fnNtQueryInformationProcess>(NtApiResolver::Instance().loadFunc(hNtdll, L"NtQueryInformationProcess"));
		pfnRtlGetLastNtStatus = reinterpret_cast<fnRtlGetLastNtStatus>(NtApiResolver::Instance().loadFunc(hNtdll, L"RtlGetLastNtStatus"));
		pfnRtlSetLastWin32ErrorAndNtStatusFromNtStatus = reinterpret_cast<fnRtlSetLastWin32ErrorAndNtStatusFromNtStatus>(NtApiResolver::Instance().loadFunc(hNtdll, L"RtlSetLastWin32ErrorAndNtStatusFromNtStatus"));
	}

	MEMX_API void Bootstrap() {
		loadNtFunc();
	}
}
