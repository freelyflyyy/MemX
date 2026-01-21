#include "XModule.h"
#include <iostream>

namespace MemX {
	XModule::XModule(XContext& context) : _context(context) {}
	XModule::~XModule() {}

	ModulePtr XModule::GetMainModule() {
		if ( !_context.IsActive() ) return std::make_shared<Module>();

		BOOL isWow64 = _context.IsWow64();
		PTR_T imageBase = 0;

		if ( isWow64 ) {
			PEB32 peb32 = { 0 };
			_context.GetRuntime()->GetTargetPeb(&peb32);
			imageBase = peb32.ImageBaseAddress;
		} else {
			PEB64 peb64 = { 0 };
			_context.GetRuntime()->GetTargetPeb(&peb64);
			imageBase = peb64.ImageBaseAddress;
		}
		std::shared_ptr<Module> mainModule = std::make_shared<Module>();
		mainModule->BaseAddress = imageBase;
		mainModule->Size = 0;
		return mainModule;
	}

	ModulePtr XModule::GetModule(WCHAR* moduleName) {
		return GetModule(moduleName, SCAN_LDR);
	}

	ModulePtr XModule::GetModule(WCHAR* moduleName, MODULE_SEARCH_MODE moduleSearchMode) {
		if ( !_context.IsActive() ) return ModulePtr();

		ModulePtr module;
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		switch ( moduleSearchMode ) {
		case SCAN_LDR:
			if ( !_context.IsWow64() ) {
				status = _context.GetRuntime()->FindModuleByLdrList64(moduleName, module);
			} else {
				status = _context.GetRuntime()->FindModuleByLdrList32(moduleName, module);
			}
			break;
		case SCAN_SECTION:
		case SCAN_PEHEADER:
			break;
		default:
			break;
		}
		if ( !NT_SUCCESS(status) ) {
			std::cerr << "GetModuleByNameT failed, NTSTATUS: 0x" << std::hex << status << std::dec << std::endl;
			return ModulePtr();
		}
		return module;
	}
}