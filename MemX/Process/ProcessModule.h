#pragma once
#include "../Common/Types/Type.h"
#include "../Config.h"


namespace MemX {
	class ProcessModule {
		public:
		ProcessModule(class Process& process);
		~ProcessModule();

		MEMX_API PModuleData GetMainModule();


		private:
		ProcessModule(const ProcessModule&) = delete;
		ProcessModule& operator=(const ProcessModule&) = delete;

		private:
		class Process& _Process;
		class ProcessCore& _Core;
		class ProcessMemory& _Memory;
	};
}