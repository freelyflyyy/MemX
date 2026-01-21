#pragma once
#include "../Common/Types/Type.h"
#include <shared_mutex>
#include <unordered_map>


namespace MemX {

	class ProcessModule {
		public:
		ProcessModule(class Process& process);
		~ProcessModule();

		ModulePtr GetMainModule();

		ModulePtr GetModule(WCHAR* moduleName);

		ModulePtr GetModule(WCHAR* moduleName, MODULE_SEARCH_MODE moduleSearchMode);
		
		private:
		ProcessModule(const ProcessModule&) = delete;
		ProcessModule& operator=(const ProcessModule&) = delete;

		private:
		class Process& _process;
		class ProcessCore& _core;
		class ProcessMemory& _memory;

		protected:
		std::shared_mutex _mutex;
		std::unordered_map<std::wstring, Module> _cache;
	};
}