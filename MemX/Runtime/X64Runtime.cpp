#include "X64Runtime.h"
#include "../Common/Utils/StringUtils.h"
#include "../Common/NtApi/NtApi.h"

namespace MemX {
	X64Runtime::X64Runtime(HANDLE hProcess) : Runtime(hProcess) {
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

	NTSTATUS X64Runtime::FindModuleByLdrList32(LPWSTR lpModuleName, ModulePtr& pModule) {
		std::vector<ModulePtr> pModulesEntry;
		NTSTATUS status = GetAllModulesByLdrList32(&pModulesEntry);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}
		for ( const auto& module : pModulesEntry ) {
			if ( module->FullName.find(ToLower(lpModuleName)) != std::wstring::npos ) {
				pModule = module;
				return STATUS_SUCCESS;
			}
		}
		return STATUS_NOT_FOUND;
	}

	NTSTATUS X64Runtime::FindModuleByLdrList64(LPWSTR lpModuleName, ModulePtr& pModule) {
		std::vector<ModulePtr> pModulesEntry;
		NTSTATUS status = GetAllModulesByLdrList64(&pModulesEntry);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}
		for ( const auto& module : pModulesEntry ) {
			if ( module->FullName.find(ToLower(lpModuleName)) != std::wstring::npos ) {
				pModule = module;
				return STATUS_SUCCESS;
			}
		}
		return STATUS_NOT_FOUND;
	}


	NTSTATUS X64Runtime::GetAllModulesByLdrList32(std::vector<ModulePtr>* pModulesEntry) {
		NTSTATUS status = STATUS_SUCCESS;
		if ( !pModulesEntry ) return STATUS_INVALID_PARAMETER;
		pModulesEntry->clear();

		PEB32 peb32 = { 0 };
		GetTargetPeb(&peb32);

		PEB_LDR_DATA32 ldr32;
		if ( ReadProcessMemoryT(peb32.Ldr, &ldr32, sizeof(PEB_LDR_DATA32), NULL) != STATUS_SUCCESS ) {
			return STATUS_ACCESS_VIOLATION;
		}

		DWORD head = ldr32.InLoadOrderModuleList.Flink;
		DWORD end = peb32.Ldr + offsetof(PEB_LDR_DATA32, InLoadOrderModuleList);

		while ( head != end && head != 0 ) {
			LDR_DATA_TABLE_ENTRY32 ldrEntry32;

			if ( ReadProcessMemoryT(head, &ldrEntry32, sizeof(LDR_DATA_TABLE_ENTRY32), NULL) != STATUS_SUCCESS ) {
				break;
			}

			wchar_t moduleFullPath[ 512 ] = { 0 };
			ReadProcessMemoryT(ldrEntry32.FullDllName.Buffer, moduleFullPath, ldrEntry32.FullDllName.Length, NULL);

			Module module;
			module.BaseAddress = ldrEntry32.DllBase;
			module.Size = ldrEntry32.SizeOfImage;
			module.FullPath = ToLower(moduleFullPath);
			module.FullName = ToLower(getPathName(module.FullPath));
			module.IsX86 = FALSE;
			module.isManual = FALSE;
			module.LdrNode = head;

			pModulesEntry->emplace_back(std::make_shared<Module>(module));

			// 往后挪：读取 Flink
			head = ldrEntry32.InLoadOrderLinks.Flink;
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS X64Runtime::GetAllModulesByLdrList64(std::vector<ModulePtr>* pModulesEntry) {
		NTSTATUS status = STATUS_SUCCESS;
		if ( !pModulesEntry ) return STATUS_INVALID_PARAMETER;
		pModulesEntry->clear();

		// 获取 64位 PEB
		PEB64 peb64 = { 0 };
		GetTargetPeb(&peb64);

		//读取 LDR 数据
		PEB_LDR_DATA64 ldr64;
		if ( ReadProcessMemoryT(peb64.Ldr, &ldr64, sizeof(PEB_LDR_DATA64), NULL) != STATUS_SUCCESS ) {
			return STATUS_ACCESS_VIOLATION;
		}

		DWORD64 head = ldr64.InLoadOrderModuleList.Flink;
		DWORD64 end = peb64.Ldr + offsetof(PEB_LDR_DATA64, InLoadOrderModuleList);

		// 开始遍历
		while ( head != end && head != 0 ) {
			LDR_DATA_TABLE_ENTRY64 ldrEntry64;
			if ( ReadProcessMemoryT(head, &ldrEntry64, sizeof(LDR_DATA_TABLE_ENTRY64), NULL) != STATUS_SUCCESS ) {
				break;
			}
			wchar_t moduleFullPath[ 512 ] = { 0 };
			ReadProcessMemoryT(ldrEntry64.FullDllName.Buffer, moduleFullPath, ldrEntry64.FullDllName.Length, NULL);

			Module module;
			module.BaseAddress = ldrEntry64.DllBase;
			module.Size = ldrEntry64.SizeOfImage;
			module.FullPath = ToLower(moduleFullPath);
			module.FullName = ToLower(getPathName(module.FullPath));
			module.IsX86 = FALSE;
			module.isManual = FALSE;
			module.LdrNode = head;

			pModulesEntry->emplace_back(std::make_shared<Module>(module));

			// 往后挪
			head = ldrEntry64.InLoadOrderLinks.Flink;
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS X64Runtime::GetAllModulesByPEHeaders32(std::vector<ModulePtr>* pModulesEntry) {
		return NTSTATUS();
	}

	NTSTATUS X64Runtime::GetAllModulesByPEHeaders64(std::vector<ModulePtr>* pModulesEntry) {
		return NTSTATUS();
	}

	NTSTATUS X64Runtime::GetAllModulesBySections32(std::vector<ModulePtr>* pModulesEntry) {
		return NTSTATUS();
	}

	NTSTATUS X64Runtime::GetAllModulesBySections64(std::vector<ModulePtr>* pModulesEntry) {
		return NTSTATUS();
	}

	NTSTATUS X64Runtime::GetAllModules32(std::vector<ModulePtr>* pModulesEntry, MODULE_SEARCH_MODE& moduleSearchMode) {
		switch ( moduleSearchMode ) {
		case SCAN_LDR:
			return GetAllModulesByLdrList32(pModulesEntry);
		case SCAN_SECTION:
			return GetAllModulesBySections32(pModulesEntry);
		case SCAN_PEHEADER:
			return GetAllModulesByPEHeaders32(pModulesEntry);
		default:
			break;
		}
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS X64Runtime::GetAllModules64(std::vector<ModulePtr>* pModulesEntry, MODULE_SEARCH_MODE& moduleSearchMode) {
		switch ( moduleSearchMode ) {
		case SCAN_LDR:
			return GetAllModulesByLdrList64(pModulesEntry);
		case SCAN_SECTION:
			return GetAllModulesBySections64(pModulesEntry);
		case SCAN_PEHEADER:
			return GetAllModulesByPEHeaders64(pModulesEntry);
		default:
			break;
		}
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS X64Runtime::GetProcessWindows(std::vector<WindowInfo>& windows) {
		windows.clear();
		if ( !_hProcess || !_pid ) {
			return STATUS_INVALID_PARAMETER;
		}
		struct EnumCtx {
			DWORD TargetPID;
			std::vector<WindowInfo>* List;
			X64Runtime* Runtime;
		} ctx = { this->_pid, &windows, this };

		BOOL ret = EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
				auto* pCtx = reinterpret_cast<EnumCtx*>(lParam);
				DWORD wPid = 0;
				GetWindowThreadProcessId(hWnd, &wPid);
				if ( wPid == pCtx->TargetPID ) {
					WindowInfo info;
					if ( NT_SUCCESS(pCtx->Runtime->GetWindowDetail(hWnd, info)) ) {
						pCtx->List->emplace_back(info);
					}
				}
				return TRUE;
			},
			reinterpret_cast<LPARAM>(&ctx)
		);
		return ret ? STATUS_SUCCESS : GetLastNtStatus();
	}

	NTSTATUS X64Runtime::GetWindowDetail(HWND hWnd, WindowInfo& info) {
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