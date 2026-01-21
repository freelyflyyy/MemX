#pragma once

#include "Runtime.h"

namespace MemX {
	class Wow64Runtime : public Runtime {
	public:
		Wow64Runtime(HANDLE hProcess);
		virtual ~Wow64Runtime();

		virtual NTSTATUS GetTargetPeb(PEB32* peb) override;

		virtual NTSTATUS GetTargetPeb(PEB64* peb) override;

		virtual NTSTATUS GetTargetLdr(PEB_LDR_DATA32* ldrData) override;

		virtual NTSTATUS GetTargetLdr(PEB_LDR_DATA64* ldrData) override;

		virtual NTSTATUS ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) override;

		virtual NTSTATUS WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes = NULL) override;

		virtual NTSTATUS VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) override;

		virtual NTSTATUS FindModuleByLdrList32(LPWSTR lpModuleName, ModulePtr& pModule) override;

		virtual NTSTATUS FindModuleByLdrList64(LPWSTR lpModuleName, ModulePtr& pModule) override;

		virtual NTSTATUS GetAllModulesByLdrList32(std::vector<ModulePtr>* pModulesEntry) override;

		virtual NTSTATUS GetAllModulesByLdrList64(std::vector<ModulePtr>* pModulesEntry) override;

		virtual NTSTATUS GetAllModulesByPEHeaders32(std::vector<ModulePtr>* pModulesEntry) override;

		virtual NTSTATUS GetAllModulesByPEHeaders64(std::vector<ModulePtr>* pModulesEntry) override;

		virtual NTSTATUS GetAllModulesBySections32(std::vector<ModulePtr>* pModulesEntry) override;

		virtual NTSTATUS GetAllModulesBySections64(std::vector<ModulePtr>* pModulesEntry) override;

		virtual NTSTATUS GetAllModules32(std::vector<ModulePtr>* pModulesEntry, MODULE_SEARCH_MODE& moduleSearchMode) override;

		virtual NTSTATUS GetAllModules64(std::vector<ModulePtr>* pModulesEntry, MODULE_SEARCH_MODE& moduleSearchMode) override;

	private:
		DWORD64 _ntdll64 = 0;
	};
}
