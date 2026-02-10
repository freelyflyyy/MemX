#include "../NtApi/NtApi.h"
#include "StringUtils.h"
#include "Wow64Utils.h"

namespace MemX {
	inline NTSTATUS EnablePrivilege(LPCWSTR lpszPrivilege) {
		HANDLE hToken = nullptr;
		TOKEN_PRIVILEGES tp = { 0 };
		LUID luid;

		if ( !OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, FALSE, &hToken) ) {
			if ( GetLastError() != ERROR_NO_TOKEN ) {
				return GetLastError();
			}

			if ( !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken) ) {
				return GetLastError();
			}
		}

		if ( !LookupPrivilegeValueW(NULL, lpszPrivilege, &luid) ) {
			CloseHandle(hToken);
			return GetLastError();
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[ 0 ].Luid = luid;
		tp.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;
		if ( !AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL) ) {
			CloseHandle(hToken);
			return GetLastError();
		}

		DWORD dwError = GetLastError();
		CloseHandle(hToken);

		if ( dwError == ERROR_NOT_ALL_ASSIGNED ) {
			return STATUS_PRIVILEGE_NOT_HELD;
		}

		return (dwError == ERROR_SUCCESS) ? STATUS_SUCCESS : GetLastNtStatus();
	}

	inline NTSTATUS GetPidByName(const std::wstring& processName, DWORD& outPid) {
		outPid = 0;

		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if ( hSnap == INVALID_HANDLE_VALUE ) {
			return STATUS_UNSUCCESSFUL;
		}

		PROCESSENTRY32W pe32 = { 0 };
		pe32.dwSize = sizeof(PROCESSENTRY32W);

		if ( !Process32FirstW(hSnap, &pe32) ) {
			CloseHandle(hSnap);
			return STATUS_UNSUCCESSFUL;
		}

		do {
			//_wcsicmp 进行不区分大小写比较
			if ( _wcsicmp(pe32.szExeFile, processName.c_str()) == 0 ) {
				outPid = pe32.th32ProcessID;
				CloseHandle(hSnap);
				return STATUS_SUCCESS;
			}
		} while ( Process32NextW(hSnap, &pe32) );

		CloseHandle(hSnap);
		return STATUS_NOT_FOUND;
	}

	inline std::wstring GetSysErrMsg() {
		DWORD error = GetLastError();
		if ( error == 0 ) return L"Unknown Error";

		LPWSTR msgBuf = nullptr;
		size_t size = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &msgBuf, 0, NULL);

		std::wstring msg;
		if ( size > 0 && msgBuf ) {
			msg = msgBuf;
			LocalFree(msgBuf);
			// 去掉末尾换行符
			while ( !msg.empty() && (msg.back() == L'\r' || msg.back() == L'\n') ) {
				msg.pop_back();
			}
		} else {
			msg = L"Error Code: " + std::to_wstring(error);
		}
		return msg;
	}
}