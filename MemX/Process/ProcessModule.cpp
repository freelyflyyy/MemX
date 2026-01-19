#include "ProcessModule.h"
#include "Process.h"

namespace MemX {
	ProcessModule::ProcessModule(Process& process)
		: _process(process),
		_memory(process.Memory()),
		_core(process.Core()) {
	}
	ProcessModule::~ProcessModule() {
	}

	 ModuleInfoPtr ProcessModule::GetMainModule() {
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
		std::shared_ptr<ModuleInfo> mainModule = std::make_shared<ModuleInfo>();
		mainModule->baseAddr = imageBase;
		mainModule->uSize = 0;
		return mainModule;
	}

}
