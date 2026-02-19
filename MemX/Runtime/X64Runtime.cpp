#include "X64Runtime.h"
#include "../Common/Utils/StringUtils.h"
#include "../Common/NtApi/NtApi.h"

namespace MemX {
	X64Runtime::X64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	X64Runtime::X64Runtime(DWORD pid, HANDLE hProcess) : Runtime(pid, hProcess) {
	}

	X64Runtime::~X64Runtime() {
	}

	NTSTATUS X64Runtime::GetTargetPeb(PEB32* peb) {
		if ( !peb ) {
			return STATUS_INVALID_PARAMETER;
		}
		PTR_T pbi = 0;
		ULONG bytes = 0;
		static DWORD64 pNtQueryInfo = GET_NTFUNC("NtQueryInformationProcess");
		if ( !pNtQueryInfo ) return STATUS_ACCESS_VIOLATION;
		NTSTATUS queryInfoRet = CALL64_FUNC(pNtQueryInfo,
											(DWORD64) _hProcess,
											(DWORD64) ProcessWow64Information,
											(DWORD64) &pbi,
											(DWORD64) sizeof(pbi),
											(DWORD64) &bytes);
		if ( !NT_SUCCESS(queryInfoRet) ) {
			return queryInfoRet;
		}
		return ReadProcessMemoryT(pbi, peb, sizeof(PEB32), NULL);
	}

	NTSTATUS X64Runtime::GetTargetPeb(PEB64* peb) {
		if ( !peb ) {
			return STATUS_INVALID_PARAMETER;
		}
		PROCESS_BASIC_INFORMATION64 pbi = { 0 };
		ULONG bytes = 0;
		static DWORD64 pNtQueryInfo = GET_NTFUNC("NtQueryInformationProcess");
		if ( !pNtQueryInfo ) return STATUS_ACCESS_VIOLATION;
		NTSTATUS queryInfoRet = CALL64_FUNC(pNtQueryInfo,
											(DWORD64) _hProcess,
											(DWORD64) ProcessBasicInformation,
											(DWORD64) &pbi,
											(DWORD64) sizeof(pbi),
											(DWORD64) &bytes);
		if ( !NT_SUCCESS(queryInfoRet) ) {
			return queryInfoRet;
		}
		return ReadProcessMemoryT(pbi.PebBaseAddress, peb, sizeof(PEB64), NULL);
	}

	NTSTATUS X64Runtime::GetTargetLdr(PEB_LDR_DATA32* ldrData) {
		PEB32 peb32 = { 0 };
		NTSTATUS status = GetTargetPeb(&peb32);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}

		return ReadProcessMemoryT(peb32.Ldr, ldrData, sizeof(PEB_LDR_DATA32), NULL);
	}

	NTSTATUS X64Runtime::GetTargetLdr(PEB_LDR_DATA64* ldrData) {
		PEB64 peb64 = { 0 };
		NTSTATUS status = GetTargetPeb(&peb64);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}
		return ReadProcessMemoryT(peb64.Ldr, ldrData, sizeof(PEB_LDR_DATA32), NULL);
	}

	NTSTATUS X64Runtime::ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) {
		BOOL dwRet = ReadProcessMemory(_hProcess, reinterpret_cast<LPCVOID>(lpBaseAddress), lpBuffer, dwSize, reinterpret_cast<SIZE_T*>(readBytes));
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes) {
		BOOL dwRet = WriteProcessMemory(_hProcess, reinterpret_cast<LPVOID>(lpBaseAddress), lpBuffer, dwSize, reinterpret_cast<SIZE_T*>(writtenBytes));
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) {
		SIZE_T dwRet = VirtualQueryEx(_hProcess, reinterpret_cast<LPCVOID>(lpAddress), (PMEMORY_BASIC_INFORMATION) lpBuffer, sizeof(MEMORY_BASIC_INFORMATION64));
		return dwRet != 0 ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::EnumLdrModules(const ModuleCallback& callback) {
		if ( !callback ) {
			return STATUS_INVALID_PARAMETER;
		}
		PEB64 peb64 = { 0 };
		GetTargetPeb(&peb64);

		PEB_LDR_DATA64 ldr64;
		if ( ReadProcessMemoryT(peb64.Ldr, &ldr64, sizeof(PEB_LDR_DATA64), NULL) != STATUS_SUCCESS ) {
			return STATUS_ACCESS_VIOLATION;
		}

		DWORD64 head = ldr64.InLoadOrderModuleList.Flink;
		DWORD64 end = peb64.Ldr + offsetof(PEB_LDR_DATA64, InLoadOrderModuleList);

		while ( head != end && head != 0 ) {
			LDR_DATA_TABLE_ENTRY64 ldrEntry64;

			if ( ReadProcessMemoryT(head, &ldrEntry64, sizeof(LDR_DATA_TABLE_ENTRY64), NULL) != STATUS_SUCCESS ) {
				break;
			}

			wchar_t moduleFullPath[ 512 ] = { 0 };
			ReadProcessMemoryT(ldrEntry64.FullDllName.Buffer, moduleFullPath, ldrEntry64.FullDllName.Length, NULL);

			ModuleInfo module;
			module.BaseAddress = ldrEntry64.DllBase;
			module.Size = ldrEntry64.SizeOfImage;
			module.FullPath = ToLower(moduleFullPath);
			module.FullName = ToLower(getPathName(module.FullPath));
			module.mArch = ModuleArch::x64;
			module.isManual = FALSE;
			module.LdrNode = head;

			if ( !callback(module) ) break;

			head = ldrEntry64.InLoadOrderLinks.Flink;
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS X64Runtime::GetAllWindow(std::vector<HWND>& handles) {
		handles.clear();
		if ( !_hProcess || !_pid ) {
			return STATUS_INVALID_PARAMETER;
		}

		struct EnumCtx {
			DWORD TargetPID;
			std::vector<HWND>* List;
		} ctx = { this->_pid, &handles };

		EnumWindows([] (HWND hWnd, LPARAM lParam) -> BOOL {
			auto* c = reinterpret_cast<EnumCtx*>(lParam);

			DWORD wPid = 0;
			GetWindowThreadProcessId(hWnd, &wPid);

			if ( wPid == c->TargetPID ) {
				c->List->push_back(hWnd);
			}
			return TRUE;
		}, (LPARAM) &ctx);

		if ( ctx.List->empty() ) {
			return STATUS_NOT_FOUND;
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS X64Runtime::GetWindow(LPCWSTR lpClassName, LPCWSTR lpWindowName, HWND& handle) {
		if ( lpClassName == nullptr && lpWindowName == nullptr ) {
			return STATUS_INVALID_PARAMETER;
		}

		struct EnumCtx {
			DWORD TargetPID;
			LPCWSTR ClassName;
			LPCWSTR WindowName;
			HWND FoundHandle;
		} ctx = { this->_pid, lpClassName, lpWindowName, NULL };

		EnumWindows([] (HWND hwnd, LPARAM lParam) -> BOOL {
			auto* c = reinterpret_cast<EnumCtx*>(lParam);

			//Check the window's HWMD matches the target PID
			DWORD wPid = 0;
			GetWindowThreadProcessId(hwnd, &wPid);
			if ( wPid != c->TargetPID ) {
				return FALSE;
			}

			//Check the window's class name matches the target class name (if provided)
			if ( c->ClassName != NULL ) {
				WCHAR className[ 256 ] = { 0 };
				GetClassNameW(hwnd, className, 256);
				if ( wcscmp(c->ClassName, className) != 0 ) {
					return TRUE;
				}
			}

			//Check the window's title matches the target window name (if provided)
			if ( c->WindowName != NULL ) {
				WCHAR windowName[ 256 ] = { 0 };
				GetWindowTextW(hwnd, windowName, 256);
				if ( wcscmp(c->WindowName, windowName) != 0 ) {
					return TRUE;
				}
			}

			c->FoundHandle = hwnd;
			return FALSE; // Found the window, stop enumeration
		}, (LPARAM) &ctx);

		if ( ctx.FoundHandle != NULL ) {
			handle = ctx.FoundHandle;
			return STATUS_SUCCESS;
		}
		return STATUS_NOT_FOUND;
	}

	NTSTATUS X64Runtime::GetWindowInfo(HWND hWnd, WindowInfo& info) {
		if ( !IsWindow(hWnd) ) return STATUS_INVALID_HANDLE;

		info.hWindow = hWnd;
		info.IsVisible = IsWindowVisible(hWnd);
		// 获取标题
		WCHAR buf[ 512 ] = { 0 };
		GetWindowTextW(hWnd, buf, 512);
		info.Title = buf;
		// 获取类名
		GetClassNameW(hWnd, buf, 512);
		info.ClassName = buf;
		// 获取窗口矩形
		GetWindowRect(hWnd, &info.WindowRect);
		info.Width = info.WindowRect.right - info.WindowRect.left;
		info.Height = info.WindowRect.bottom - info.WindowRect.top;
		// 获取客户区并转换坐标
		GetClientRect(hWnd, &info.ClientRect);
		info.ClientWidth = info.ClientRect.right - info.ClientRect.left;
		info.ClientHeight = info.ClientRect.bottom - info.ClientRect.top;
		// ClientRect 默认是相对坐标 (0,0)，我们需要转换成屏幕绝对坐标
		POINT pt = { info.ClientRect.left, info.ClientRect.top };
		ClientToScreen(hWnd, &pt);
		info.ClientRect.left = pt.x;
		info.ClientRect.top = pt.y;
		info.ClientRect.right += pt.x;
		info.ClientRect.bottom += pt.y;

		return STATUS_SUCCESS;
	}

}