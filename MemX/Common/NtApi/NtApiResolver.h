#pragma once

#include "../WinApi/WinHeaders.h"
#include "../../Config.h"

#include <unordered_map>
#include <mutex>
#include <string>

namespace MemX {
	class NtApiResolver {
		public:
		MEMX_API static NtApiResolver& Instance() {
			static NtApiResolver instance;
			return instance;
		}

		template<typename T>
		MEMX_API T getFunc(const std::wstring& name) {
			std::lock_guard<std::mutex> lockGuard(_mutex);

			auto iterator = _apiMap.find(name);
			if ( iterator != _apiMap.end() ) {
				return reinterpret_cast<T>(iterator->second);
			}
		}

		template<typename T, typename... Arges>
		MEMX_API auto callFunc(const std::wstring& name, Arges&&... arges) {
			std::lock_guard<std::mutex> lockGuard(_mutex);
			auto iterator = _apiMap.find(name);
			if ( iterator != _apiMap.end() ) {
				T func = reinterpret_cast<T>(iterator->second);
				return func(std::forward(arges));
			} else {
				return std::invoke_result_t<T, Arges...>();
			}
		}

		MEMX_API FARPROC loadFunc(HMODULE hMod, const std::wstring& funName) {
			std::lock_guard<std::mutex> lockGuard(_mutex);
			FARPROC procAddress = GetProcAddress(hMod, std::string(funName.begin(), funName.end()).c_str());
			if ( procAddress ) {
				_apiMap.insert({ funName, procAddress });
				return procAddress;
			}
			return nullptr;
		}

		MEMX_API FARPROC loadFunc(const std::wstring& moduleName, const std::wstring& funName) {
			HMODULE hMod = GetModuleHandleW(moduleName.c_str());
			if ( hMod ) {
				return loadFunc(hMod, funName);
			}
			return nullptr;
		}

		private:
		std::unordered_map<std::wstring, FARPROC> _apiMap;
		std::mutex _mutex;

		NtApiResolver() = default;
		NtApiResolver(const NtApiResolver&) = delete;
	};
}