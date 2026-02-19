#include "Wow64Runtime.h"
#include "MemX/Common/Utils/Wow64Utils.h"
#include "MemX/Common/Utils/StringUtils.h"
#include "../Common/NtApi/NtApi.h"
#include "../Common/NtApi/NtCallExt.h"

namespace MemX {
	Wow64Runtime::Wow64Runtime(HANDLE hProcess) : Runtime(hProcess) {
	}

	Wow64Runtime::Wow64Runtime(DWORD pid, HANDLE hProcess) : Runtime(pid, hProcess) {
	}

	Wow64Runtime::~Wow64Runtime() {
	}

	NTSTATUS Wow64Runtime::GetTargetPeb(PEB32* peb) {
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

	NTSTATUS Wow64Runtime::GetTargetPeb(PEB64* peb) {
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

	NTSTATUS Wow64Runtime::GetTargetLdr(PEB_LDR_DATA32* ldrData) {
		PEB32 peb32 = { 0 };
		NTSTATUS status = GetTargetPeb(&peb32);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}

		return ReadProcessMemoryT(peb32.Ldr, ldrData, sizeof(PEB_LDR_DATA32), NULL);
	}

	NTSTATUS Wow64Runtime::GetTargetLdr(PEB_LDR_DATA64* ldrData) {
		PEB64 peb64 = { 0 };
		NTSTATUS status = GetTargetPeb(&peb64);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}

		return ReadProcessMemoryT(peb64.Ldr, ldrData, sizeof(PEB_LDR_DATA32), NULL);
	}

	NTSTATUS Wow64Runtime::ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) {
		static DWORD64 pNtReadVM = GET_NTFUNC("NtReadVirtualMemory");
		if ( !pNtReadVM ) {
			return STATUS_ACCESS_VIOLATION;
		}
		NTSTATUS ntRet = CALL64_FUNC(pNtReadVM,
					  (DWORD64) _hProcess,
					  (DWORD64) lpBaseAddress,
					  (DWORD64) lpBuffer,
					  (DWORD64) dwSize,
					  (DWORD64) readBytes);
		return ntRet;
	}

	NTSTATUS Wow64Runtime::WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes) {
		static DWORD64 pNtWriteVM = GET_NTFUNC("NtWriteVirtualMemory");
		if ( !pNtWriteVM ) {
			return STATUS_ACCESS_VIOLATION;
		}
		NTSTATUS ntRet = CALL64_FUNC(pNtWriteVM,
					  (DWORD64) _hProcess,
					  (DWORD64) lpBaseAddress,
					  (DWORD64) lpBuffer,
					  (DWORD64) dwSize,
					  (DWORD64) writtenBytes);
		return ntRet;
	}

	NTSTATUS Wow64Runtime::VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) {
		static DWORD64 pNtQueryEx = GET_NTFUNC("NtQueryVirtualMemory");
		if ( !pNtQueryEx ) {
			return STATUS_ACCESS_VIOLATION;
		}
		NTSTATUS ntRet = CALL64_FUNC(pNtQueryEx, 6, (DWORD64) _hProcess, lpAddress, 0ULL, (DWORD64) lpBuffer, (DWORD64) sizeof(MEMORY_BASIC_INFORMATION64), 0ull);
		return ntRet;
	}

	NTSTATUS Wow64Runtime::EnumLdrModules(const ModuleCallback& callback) {
		if ( !callback ) {
			return STATUS_INVALID_PARAMETER;
		}
		
		//General traversal Ldr table
		auto walkList = [&] (auto ldrTable, PTR_T head, PTR_T end, ModuleArch arch) -> BOOL {
			using LdrType = decltype(ldrTable);
			//Prevent abnormal Ldr
			int safeCount = 0;
			while ( head != end && head != 0 && safeCount++ < 1000) {
				LdrType ldrEntry;

				if ( !NT_SUCCESS(ReadProcessMemoryT(head, &ldrEntry, sizeof(ldrEntry), NULL)) ) {
					break;
				}

				wchar_t moduleFullPath[ 512 ] = { 0 };
				ReadProcessMemoryT(ldrEntry.FullDllName.Buffer, moduleFullPath, ldrEntry.FullDllName.Length, NULL);

				ModuleInfo module;
				module.BaseAddress = ldrEntry.DllBase;
				module.Size = ldrEntry.SizeOfImage;
				module.FullPath = ToLower(moduleFullPath);
				module.FullName = ToLower(getPathName(module.FullPath));
				module.mArch = arch;
				module.isManual = FALSE;
				module.LdrNode = head;

				//Execute callback
				if ( !callback(module) ) return FALSE;

				head = ldrEntry.InLoadOrderLinks.Flink;
			}
			return TRUE;
		};

		//ergodic 64 bit module
		PEB64 peb64 = { 0 };
		if ( NT_SUCCESS(GetTargetPeb(&peb64)) &&  peb64.Ldr != 0) {
			PEB_LDR_DATA64 ldr64 = { 0 };
			if ( NT_SUCCESS(ReadProcessMemoryT(peb64.Ldr, &ldr64, sizeof(PEB_LDR_DATA64), NULL)) ) {
				PTR_T head = ldr64.InLoadOrderModuleList.Flink;
				PTR_T end = peb64.Ldr + offsetof(PEB_LDR_DATA64, InLoadOrderModuleList);

				if ( !walkList(LDR_DATA_TABLE_ENTRY64{}, head, end, ModuleArch::x64) ) {
					return STATUS_SUCCESS;
				}
			}
		}

		PEB32 peb32 = { 0 };
		if ( NT_SUCCESS(GetTargetPeb(&peb32)) && peb32.Ldr != 0 ) {
			PEB_LDR_DATA32 ldr32;
			if ( NT_SUCCESS(ReadProcessMemoryT(peb32.Ldr, &ldr32, sizeof(PEB_LDR_DATA32), NULL)) ) {
				PTR_T head = ldr32.InLoadOrderModuleList.Flink;
				PTR_T end = peb32.Ldr + offsetof(PEB_LDR_DATA32, InLoadOrderModuleList);

				walkList(LDR_DATA_TABLE_ENTRY32{}, head, end, ModuleArch::x86);
			}
		}
		return STATUS_SUCCESS;
	}

	NTSTATUS Wow64Runtime::GetAllWindow(std::vector<HWND>& handles) {
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

	NTSTATUS Wow64Runtime::GetWindow(LPCWSTR lpClassName, LPCWSTR lpWindowName, HWND& handle) {
		if ( lpClassName == nullptr && lpWindowName == nullptr ) {
			return STATUS_INVALID_PARAMETER;
		}

		struct EnumCtx {
			DWORD TargetPID;
			LPCWSTR ClassName;
			LPCWSTR WindowName;
			HWND FoundHandle;
		} ctx = { this->_pid, lpClassName, lpWindowName, NULL };

		EnumWindows([] (HWND hwnd, LPARAM lParam) -> BOOL{
			auto *c = reinterpret_cast<EnumCtx*>(lParam);

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
				if (wcscmp(c->ClassName, className) != 0) {
					return TRUE;
				}
			}

			//Check the window's title matches the target window name (if provided)
			if ( c->WindowName != NULL ) {
				WCHAR windowName[ 256 ] = { 0 };
				GetWindowTextW(hwnd, windowName, 256);
				if (wcscmp(c->WindowName, windowName) != 0) {
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

	NTSTATUS Wow64Runtime::GetWindowInfo(HWND hWnd, WindowInfo& info) {
		if ( !IsWindow(hWnd) ) return STATUS_INVALID_HANDLE;

		info.hWindow = hWnd;
		info.IsVisible = IsWindowVisible(hWnd);
		//Get Title
		WCHAR buf[ 512 ] = { 0 };
		GetWindowTextW(hWnd, buf, 512);
		info.Title = buf;
		//Get class name
		GetClassNameW(hWnd, buf, 512);
		info.ClassName = buf;
		//Get window rectangle (screen coordinates)
		GetWindowRect(hWnd, &info.WindowRect);
		info.Width = info.WindowRect.right - info.WindowRect.left;
		info.Height = info.WindowRect.bottom - info.WindowRect.top;
		//Get client area rectangle
		GetClientRect(hWnd, &info.ClientRect);
		info.ClientWidth = info.ClientRect.right - info.ClientRect.left;
		info.ClientHeight = info.ClientRect.bottom - info.ClientRect.top;
		//Convert client area coordinates to screen coordinates
		POINT pt = { info.ClientRect.left, info.ClientRect.top };
		ClientToScreen(hWnd, &pt);
		info.ClientRect.left = pt.x;
		info.ClientRect.top = pt.y;
		info.ClientRect.right += pt.x;
		info.ClientRect.bottom += pt.y;
		return STATUS_SUCCESS;
	}
}
