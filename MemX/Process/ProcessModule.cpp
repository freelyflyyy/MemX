#include "Process.h"
#include "ProcessCore.h"
#include "ProcessModule.h"
#include <iostream>

namespace MemX {
	ProcessModule::ProcessModule(Process& process)
		: _process(process),
		_memory(process.Memory()),
		_core(process.Core()) {
	}
	ProcessModule::~ProcessModule() {
	}

	ModulePtr ProcessModule::GetMainModule() {
		BOOL isWow64 = _core.isWow64();
		PTR_T imageBase = 0;
		if ( isWow64 ) {
			PEB32 peb32 = { 0 };
			_core.getTargetPeb(&peb32);
			imageBase = peb32.ImageBaseAddress;
		} else {
			PEB64 peb64 = { 0 };
			_core.getTargetPeb(&peb64);
			imageBase = peb64.ImageBaseAddress;
		}
		std::shared_ptr<Module> mainModule = std::make_shared<Module>();
		mainModule->BaseAddress = imageBase;
		mainModule->Size = 0;
		return mainModule;
	}

	ModulePtr ProcessModule::GetModule(WCHAR* moduleName) {
		return GetModule(moduleName, SCAN_LDR);
	}

	ModulePtr ProcessModule::GetModule(WCHAR* moduleName, MODULE_SEARCH_MODE moduleSearchMode) {
		ModulePtr module;
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		switch ( moduleSearchMode ) {
		case SCAN_LDR:
			if ( !_core.isWow64() ) {
				status = _core.getRuntime()->FindModuleByLdrList64(moduleName, module);
			} else {
				status = _core.getRuntime()->FindModuleByLdrList32(moduleName, module);
			}
			break;
		case SCAN_SECTION:
			break;
		case SCAN_PEHEADER:
			break;
		default:
			break;
		}
		if ( !NT_SUCCESS(status) ) {
			std::cerr << "GetModuleByNameT failed, Please check whether the parameter passed in is null, NTSTATUS: 0x" << std::hex << status << std::dec << std::endl;
			return ModulePtr();
		}
		return module;
	}
}
