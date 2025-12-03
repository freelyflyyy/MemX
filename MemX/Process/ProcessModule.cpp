#include "ProcessModule.h"
#include "Process.h"

namespace MemX {
	ProcessModule::ProcessModule(Process& process)
		: _Process(process),
		_Memory(process.Memory()),
		_Core(process.Core()) {
	}
	ProcessModule::~ProcessModule() {
	}

	MEMX_API PModuleData ProcessModule::GetMainModule() {
		return MEMX_API PModuleData();
	}
}
