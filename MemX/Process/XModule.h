#pragma once
#include "XContext.h"
#include "../Common/Types/XTypes.h"
#include <shared_mutex>
#include <unordered_map>

namespace MemX {
	enum class ModuleStrategy {
		Fast,		//Ldr link
		BruteForce,
		Hybrid
	};

	class XModule {
		public:
		explicit XModule(XContext& context);
		~XModule();

		XModulePtr GetMain();
		XModulePtr GetModule(const std::wstring& moduleName,
							 ModuleArch mArch = ModuleArch::Default,
							 ModuleStrategy strategy = ModuleStrategy::Fast);

		XModulePtr GetModule(PTR_T moduleAddr,
							 ModuleArch mArch = ModuleArch::Default,
							 ModuleStrategy strategy = ModuleStrategy::Fast);
		XModulePtr GetAllModules(ModuleStrategy strategy = ModuleStrategy::Hybrid);
		SIZE_T Refresh(ModuleStrategy strategy = ModuleStrategy::Fast);

		private:
		XModulePtr GetModuleVa(const std::wstring& moduleName = L"",
							   PTR_T moduleAddr = 0,
							   ModuleArch mArch = ModuleArch::Default,
							   ModuleStrategy strategy = ModuleStrategy::Fast);
		private:
		XContext& _context;
		mutable std::shared_mutex _mutex;
		std::unordered_map<std::wstring, ModulePtr> _nativeCache;
		std::unordered_map<std::wstring, ModulePtr> _wow64Cache;
	};
}