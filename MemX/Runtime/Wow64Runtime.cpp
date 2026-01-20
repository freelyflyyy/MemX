#include "Wow64Runtime.h"
#include "MemX/Common/Utils/Wow64Utils.h"
#include "MemX/Common/Utils/StringUtils.h"
#include "../Common/NtApi/NtApi.h"
#include "../Common/NtApi/NtCallExt.h"

namespace MemX {
	Wow64Runtime::Wow64Runtime(HANDLE hProcess) : Runtime(hProcess) {
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

	NTSTATUS Wow64Runtime::FindModuleByLdrList32(LPWSTR lpModuleName, ModuleInfoPtr& pModule) {
		std::vector<ModuleInfoPtr> pModulesEntry;
		NTSTATUS status = GetAllModulesByLdrList32(&pModulesEntry);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}
		for ( const auto& module : pModulesEntry ) {
			if ( module->fullName.find(ToLower(lpModuleName)) != std::wstring::npos ) {
				pModule = module;
				return STATUS_SUCCESS;
			}
		}
		return STATUS_NOT_FOUND;
	}

	NTSTATUS Wow64Runtime::FindModuleByLdrList64(LPWSTR lpModuleName, ModuleInfoPtr& pModule) {
		std::vector<ModuleInfoPtr> pModulesEntry;
		NTSTATUS status = GetAllModulesByLdrList64(&pModulesEntry);
		if ( !NT_SUCCESS(status) ) {
			return status;
		}
		for ( const auto& module : pModulesEntry ) {
			if ( module->fullName.find(ToLower(lpModuleName)) != std::wstring::npos ) {
				pModule = module;
				return STATUS_SUCCESS;
			}
		}
		return STATUS_NOT_FOUND;
	}

	NTSTATUS Wow64Runtime::GetAllModulesByLdrList32(std::vector<ModuleInfoPtr>* pModulesEntry) {
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

			ModuleInfo module;
			module.baseAddr = ldrEntry32.DllBase;
			module.uSize = ldrEntry32.SizeOfImage;
			module.fullPath = ToLower(moduleFullPath);
			module.fullName = ToLower(getPathName(module.fullPath));
			module.isX86 = TRUE; 
			module.isManual = FALSE;
			module.ldrPoint = head;

			pModulesEntry->emplace_back(std::make_shared<ModuleInfo>(module));

			// 往后挪：读取 Flink
			head = ldrEntry32.InLoadOrderLinks.Flink;
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS Wow64Runtime::GetAllModulesByLdrList64(std::vector<ModuleInfoPtr>* pModulesEntry) {
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

			ModuleInfo module;
			module.baseAddr = ldrEntry64.DllBase; 
			module.uSize = ldrEntry64.SizeOfImage;
			module.fullPath = ToLower(moduleFullPath);
			module.fullName = ToLower(getPathName(module.fullPath));
			module.isX86 = FALSE;   
			module.isManual = FALSE;
			module.ldrPoint = head;

			pModulesEntry->emplace_back(std::make_shared<ModuleInfo>(module));

			// 往后挪
			head = ldrEntry64.InLoadOrderLinks.Flink;
		}

		return STATUS_SUCCESS;
	}
	NTSTATUS Wow64Runtime::GetAllModulesByPEHeaders32(std::vector<ModuleInfoPtr>* pModulesEntry) {
		return NTSTATUS();
	}
	NTSTATUS Wow64Runtime::GetAllModulesByPEHeaders64(std::vector<ModuleInfoPtr>* pModulesEntry) {
		return NTSTATUS();
	}
	NTSTATUS Wow64Runtime::GetAllModulesBySections32(std::vector<ModuleInfoPtr>* pModulesEntry) {
		return NTSTATUS();
	}
	NTSTATUS Wow64Runtime::GetAllModulesBySections64(std::vector<ModuleInfoPtr>* pModulesEntry) {
		return NTSTATUS();
	}

	NTSTATUS Wow64Runtime::GetAllModules32(std::vector<ModuleInfoPtr>* pModulesEntry, MODULE_SEARCH_MODE& moduleSearchMode) {
		switch ( moduleSearchMode ) {
		case SCAN_LDR:
			return GetAllModulesByLdrList32(pModulesEntry);

		default:
			break;
		}
		return STATUS_UNSUCCESSFUL;
	}

}
