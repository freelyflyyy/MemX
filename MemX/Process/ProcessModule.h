#pragma once
#include "../Common/Types/Type.h"
#include <shared_mutex>
#include <unordered_map>


namespace MemX {

	class ProcessModule {
		public:
		ProcessModule(class Process& process);
		~ProcessModule();

		 BOOL IsCached(const std::wstring& moduleName);

		 ModuleInfoPtr GetMainModule();

		private:
		ProcessModule(const ProcessModule&) = delete;
		ProcessModule& operator=(const ProcessModule&) = delete;

		private:
		class Process& _process;
		class ProcessCore& _core;
		class ProcessMemory& _memory;

		protected:
		std::shared_mutex _mutex;
	};
}