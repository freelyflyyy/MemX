#pragma once
#include "../WinApi/WinHeaders.h"
#include "../Types/Type.h"
#include "../../Config.h"

#include <unordered_map>
#include <shared_mutex>
#include <string>

namespace MemX {


	//////////////////////////////////////////////////////////////////////////
	// NtCallExt: NT函数调用扩展
	//////////////////////////////////////////////////////////////////////////

	class NtCallExt {
		public:
		virtual ~NtCallExt() = default;

		//TODO:有时间需要统一参数
		MEMX_API virtual DWORD64 __cdecl GetProcAddress64(DWORD64 hMod, const char* funcName) = 0;
		MEMX_API virtual DWORD64 __cdecl GetModuleBase64(const wchar_t* moduleName) = 0;
		MEMX_API virtual DWORD64 __cdecl GetTeb64() = 0;
		MEMX_API virtual DWORD64 __cdecl GetPeb64() = 0;
		MEMX_API virtual DWORD64 __cdecl GetNtdll64() = 0;

		DWORD64 IsCached(const std::string& funcName) {
			std::shared_lock<std::shared_mutex> lock(_mutex);
			auto it = _cache.find(funcName);
			if ( it != _cache.end() ) {
				return it->second;
			}
			return 0;
		}

		DWORD64 GetFunc(DWORD64 hMod, const std::string& funcName) {
			if ( auto addr = IsCached(funcName) ) return addr;

			if ( hMod == 0 ) return 0;

			DWORD64 procAddr = GetProcAddress64(hMod, funcName.c_str());

			if ( procAddr ) {
				std::unique_lock<std::shared_mutex> lock(_mutex);
				_cache[ funcName ] = procAddr;
			}
			return procAddr;
		}

		DWORD64 GetFunc(const std::wstring& moduleName, const std::string& funcName) {
			if ( auto addr = IsCached(funcName) ) return addr;

			DWORD64 hMod = GetModuleBase64(moduleName.c_str());
			if ( hMod == 0 ) return 0;

			return GetFunc(hMod, funcName);
		}

		DWORD64 GetFunc(const std::string& funcName) {
			if ( auto addr = IsCached(funcName) ) return addr;
			return GetFunc(GetNtdll64(), funcName);
		}

		protected:
		std::unordered_map<std::string, DWORD64> _cache;
		std::shared_mutex _mutex;
	};

	//////////////////////////////////////////////////////////////////////////
	// X64NtCallExt: 原生64位实现
	//////////////////////////////////////////////////////////////////////////
	class X64NtCallExt : public NtCallExt {
		public:
		static X64NtCallExt& Instance();

		MEMX_API DWORD64 __cdecl GetProcAddress64(DWORD64 hMod, const char* funcName) override;
		MEMX_API DWORD64 __cdecl GetModuleBase64(const wchar_t* moduleName) override;
		MEMX_API DWORD64 __cdecl GetTeb64() override;
		MEMX_API DWORD64 __cdecl GetPeb64() override;
		MEMX_API DWORD64 __cdecl GetNtdll64() override;

		template<typename... Args>
		NTSTATUS X64Call(const DWORD64& funcAddr, Args&&... args) {
			if ( !funcAddr ) {
				return ERROR_INVALID_ADDRESS;
			}
			return ((NTSTATUS(NTAPI*)(Args...))funcAddr)(std::forward<Args>(args)...);
		}

		private:
		X64NtCallExt();
	};

	//////////////////////////////////////////////////////////////////////////
	// Wow64NtCallExt: WoW64环境实现 (Heaven's Gate)
	//////////////////////////////////////////////////////////////////////////
	class Wow64NtCallExt : public NtCallExt {
		public:
		static Wow64NtCallExt& Instance();

		MEMX_API DWORD64 __cdecl GetProcAddress64(DWORD64 hMod, const char* funcName) override;
		MEMX_API DWORD64 __cdecl GetModuleBase64(const wchar_t* moduleName) override;
		MEMX_API DWORD64 __cdecl GetTeb64() override;
		MEMX_API DWORD64 __cdecl GetPeb64() override;
		MEMX_API DWORD64 __cdecl GetNtdll64() override;
		MEMX_API DWORD64 __cdecl GetLdrGetProcedureAddress();
		MEMX_API DWORD64 __cdecl X64CallVa(DWORD64 funcAddr, int argCount, ...);

		template<typename... Args>
		NTSTATUS X64Call(const DWORD64& funcAddr, Args&&... args) {
			if ( !funcAddr ) {
				return ERROR_INVALID_ADDRESS;
			}
			DWORD64 result = X64CallVa(
				(DWORD64) funcAddr,
				(int) sizeof...(Args),
				(DWORD64) std::forward<Args>(args)...
			);
			return result;
		}

		private:
		Wow64NtCallExt();
	};

	inline auto& GetNtCallExt() {
		#ifdef _WIN64
		return X64NtCallExt::Instance();
		#else
		return Wow64NtCallExt::Instance();
		#endif
	}

	#define GET_FUNC(moduleName, funcName) (GetNtCallExt().GetFunc(moduleName, funcName))
	#define GET_NTFUNC(funcName) (GetNtCallExt().GetFunc(funcName))
	#define CALL64_FUNC(funcName, ...) (GetNtCallExt().X64Call(funcName, __VA_ARGS__))
}
