#pragma once
#include "XContext.h"
#include "../Common/Types/Type.h"
#include <shared_mutex>
#include <unordered_map>

namespace MemX {
	class XModule {
		public:
		explicit XModule(XContext& context);
		~XModule();

		ModulePtr GetMainModule();
		ModulePtr GetModule(WCHAR* moduleName);
		ModulePtr GetModule(WCHAR* moduleName, MODULE_SEARCH_MODE moduleSearchMode);

		private:
		XContext& _context;
		std::shared_mutex _mutex;
		std::unordered_map<std::wstring, Module> _cache;
	};
}