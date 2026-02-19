#include "XModule.h"
#include "../Common/Utils/StringUtils.h"
#include <iostream>
#include <unordered_set>

namespace MemX {
	XModule::XModule(XContext& context) : _context(context) {}
	XModule::~XModule() {}

	XModulePtr XModule::GetMain() {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XModulePtr::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}

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
		if ( !imageBase ) {
			return XModulePtr::Fail(L"Failed to get main module base address");
		}
		return GetModule(imageBase, ModuleArch::Default, ModuleStrategy::Hybrid);
	}

	XModulePtr XModule::GetModule(const std::wstring& moduleName, ModuleArch mArch, ModuleStrategy strategy) {
		return GetModuleVa(moduleName, 0, mArch, strategy);
	}

	XModulePtr XModule::GetModule(PTR_T moduleAddr, ModuleArch mArch, ModuleStrategy strategy) {
		return GetModuleVa(L"", moduleAddr, mArch, strategy);
	}

	XModulePtr XModule::GetModuleVa(const std::wstring& moduleName, PTR_T moduleAddr, ModuleArch mArch, ModuleStrategy strategy) {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XModulePtr::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		if ( moduleName.empty() && !moduleAddr ) {
			return XModulePtr::Fail(L"Invalid Value");
		}

		std::wstring key = ToLower(moduleName);
		BOOL isWow64 = _context.IsWow64();

		auto findCache = [&] () -> ModulePtr {
			auto searchMap = [&] (const std::unordered_map<std::wstring, ModulePtr>& map) -> ModulePtr {
				if ( !key.empty() ) {
					auto it = map.find(key);
					if ( it != map.end() ) {
						return it->second;
					}
				}
				if ( moduleAddr ) {
					for ( const auto& kv : map ) {
						//Check whether the address is in the module
						if ( moduleAddr >= kv.second->BaseAddress &&
							moduleAddr < (kv.second->BaseAddress + kv.second->Size) ) {
							return kv.second;
						}
					}
				}
				return nullptr;
			};
			if ( mArch == ModuleArch::x86 ) {
				return searchMap(_wow64Cache);
			} else if ( mArch == ModuleArch::x64 ) {
				return searchMap(_nativeCache);
			} else {
				if ( isWow64 ) {
					auto ptr = searchMap(_wow64Cache);
					if ( ptr ) return ptr;
					return searchMap(_nativeCache);
				} else {
					return searchMap(_nativeCache);
				}
			}
		};

		//Search in cache first
		{
			std::shared_lock<std::shared_mutex> lock(_mutex);
			auto ptr = findCache();
			if ( ptr ) { return XModulePtr::Success(ptr); }
		}

		Refresh(strategy);

		//Search in cache again after refresh
		{
			std::shared_lock<std::shared_mutex> lock(_mutex);
			auto ptr = findCache();
			if ( ptr ) return XModulePtr::Success(ptr);
		}
		return XModulePtr::Fail(L"Module not found");
	}


	XModulePtr XModule::GetAllModules(ModuleStrategy strategy) {
		return XModulePtr();
	}

	SIZE_T XModule::Refresh(ModuleStrategy strategy) {
		if ( !_context.IsActive() || !_context.GetRuntime() ) return 0;
		std::unique_lock<std::shared_mutex> lock(_mutex);

		BOOL isWow64 = _context.IsWow64();
		//live module set
		std::unordered_set<std::wstring> observedNative;
		std::unordered_set<std::wstring> observedWow64;

		auto runtime = _context.GetRuntime();
		auto callback = [&] (ModuleInfo& module) -> BOOL {
			std::wstring key = ToLower(module.FullName);
			auto& targetMap = (module.mArch == ModuleArch::x86) ? _wow64Cache : _nativeCache;
			auto& targetObservedSet = (module.mArch == ModuleArch::x86) ? observedWow64 : observedNative;

			targetObservedSet.insert(key);
			auto it = targetMap.find(key);
			if ( it != targetMap.end() ) {
				*it->second = module;
			} else {
				targetMap[ key ] = std::make_shared<ModuleInfo>(std::move(module));
			}
			return TRUE;
		};
		if ( strategy == ModuleStrategy::Fast || strategy == ModuleStrategy::Hybrid ) {
			runtime->EnumLdrModules(callback);
		}

		for ( auto it = _nativeCache.begin(); it != _nativeCache.end(); ) {
			if ( observedNative.find(it->first) == observedNative.end() ) {
				it = _nativeCache.erase(it);
			} else {
				++it;
			}
		}
		if ( isWow64 ) {
			for ( auto it = _wow64Cache.begin(); it != _wow64Cache.end(); ) {
				if ( observedWow64.find(it->first) == observedWow64.end() ) {
					it = _wow64Cache.erase(it);
				} else {
					++it;
				}
			}
		}
		return observedNative.size() + observedWow64.size();
	}
}