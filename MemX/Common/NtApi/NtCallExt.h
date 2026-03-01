#pragma once
#include "../WinApi/WinHeaders.h"
#include "../Types/XTypes.h"

#include <unordered_map>
#include <shared_mutex>
#include <string>

namespace MemX {
	#define EMIT(a) __asm __emit (a)

	// Switch to 64 bit mode
	#define x64_start \
		EMIT(0x6A) EMIT(0x33)                         /* push 0x33             */ \
		EMIT(0xE8) EMIT(0) EMIT(0) EMIT(0) EMIT(0)    /* call $+5              */ \
		EMIT(0x83) EMIT(0x04) EMIT(0x24) EMIT(0x05)   /* add dword [esp], 5    */ \
		EMIT(0xCB)                                    /* retf                  */

	// back to 32 bit mode
	#define x64_end \
		EMIT(0xE8) EMIT(0) EMIT(0) EMIT(0) EMIT(0)                                     /* call $+5              */  \
		EMIT(0xC7) EMIT(0x44) EMIT(0x24) EMIT(0x04) EMIT(0x23) EMIT(0) EMIT(0) EMIT(0) /* mov dword [rsp+4], 0x23*/ \
		EMIT(0x83) EMIT(0x04) EMIT(0x24) EMIT(0x0D)                                    /* add dword [rsp], 0x0D */  \
		EMIT(0xCB)                                                                     /* retf                  */

	#define rex_w EMIT(0x48) __asm

	#define rax  0
	#define rcx  1
	#define rdx  2
	#define rbx  3
	#define rsp  4
	#define rbp  5
	#define rsi  6
	#define rdi  7
	#define r8   8
	#define r9   9
	#define r10  10
	#define r11  11
	#define r12  12
	#define r13  13
	#define r14  14
	#define r15  15

	// push 64 register
	#define x64_push(r) EMIT(0x48 | ((r) >> 3)) EMIT(0x50 | ((r) & 7))

	// pop 64 register
	#define x64_pop(r)  EMIT(0x48 | ((r) >> 3)) EMIT(0x58 | ((r) & 7))

	union Reg64 {
		DWORD64 v;
		DWORD dw[ 2 ];
	};

	class NtCallExt {
		public:
		virtual ~NtCallExt() = default;

		virtual DWORD64 __cdecl GetProcAddress64(DWORD64 hMod, const char* funcName) = 0;
		virtual DWORD64 __cdecl GetModuleBase64(const wchar_t* moduleName) = 0;
		virtual DWORD64 __cdecl GetTeb64() = 0;
		virtual DWORD64 __cdecl GetPeb64() = 0;
		virtual DWORD64 __cdecl GetNtdll64() = 0;

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

	class X64NtCallExt : public NtCallExt {
		public:
		static X64NtCallExt& Instance();

		DWORD64 __cdecl GetProcAddress64(DWORD64 hMod, const char* funcName) override;
		DWORD64 __cdecl GetModuleBase64(const wchar_t* moduleName) override;
		DWORD64 __cdecl GetTeb64() override;
		DWORD64 __cdecl GetPeb64() override;
		DWORD64 __cdecl GetNtdll64() override;

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

	class Wow64NtCallExt : public NtCallExt {
		public:
		static Wow64NtCallExt& Instance();

		DWORD64 __cdecl GetProcAddress64(DWORD64 hMod, const char* funcName) override;
		DWORD64 __cdecl GetModuleBase64(const wchar_t* moduleName) override;
		DWORD64 __cdecl GetTeb64() override;
		DWORD64 __cdecl GetPeb64() override;
		DWORD64 __cdecl GetNtdll64() override;
		DWORD64 __cdecl GetLdrGetProcedureAddress();
		DWORD64 __cdecl X64CallVa(DWORD64 funcAddr, int argCount, ...);
		VOID __cdecl memcpy64(VOID* dest, DWORD64 src, SIZE_T sz);

		template<typename... Args>
		NTSTATUS X64Call(const DWORD64& funcAddr, Args&&... args) {
			if ( !funcAddr ) return ERROR_INVALID_ADDRESS;
			return (NTSTATUS) X64CallVa((DWORD64) funcAddr, (int) sizeof...(Args), (DWORD64) std::forward<Args>(args)...);
		}

		DWORD __cdecl GetProcAddress32(DWORD hMod, const char* funcName);
		DWORD __cdecl GetModuleBase32(const wchar_t* moduleName);
		DWORD __cdecl GetTeb32();
		DWORD __cdecl GetPeb32();
		DWORD __cdecl GetNtdll32();
		DWORD __cdecl GetLdrGetProcedureAddress32();
		DWORD __cdecl LoadLibrary32(const wchar_t* moduleName);

		DWORD IsCached32(const std::string& funcName) {
			std::shared_lock<std::shared_mutex> lock(_mutex32);
			auto it = _cache32.find(funcName);
			if ( it != _cache32.end() ) {
				return it->second;
			}
			return 0;
		}

		DWORD GetFunc32(DWORD hMod, const std::string& funcName) {
			if ( auto addr = IsCached32(funcName) ) return addr;
			if ( hMod == 0 ) return 0;
			DWORD procAddr = GetProcAddress32(hMod, funcName.c_str());
			if ( procAddr ) {
				std::unique_lock<std::shared_mutex> lock(_mutex32);
				_cache32[ funcName ] = procAddr;
			}
			return procAddr;
		}

		DWORD GetFunc32(const std::wstring& moduleName, const std::string& funcName) {
			if ( auto addr = IsCached32(funcName) ) return addr;
			DWORD hMod = GetModuleBase32(moduleName.c_str());
			if ( hMod == 0 ) hMod = LoadLibrary32(moduleName.c_str());
			if ( hMod == 0 ) return 0;
			return GetFunc32(hMod, funcName);
		}

		DWORD GetFunc32(const std::string& funcName) {
			if ( auto addr = IsCached32(funcName) ) return addr;
			return GetFunc32(GetNtdll32(), funcName);
		}

		private:
		Wow64NtCallExt();
		std::unordered_map<std::string, DWORD> _cache32;
		std::shared_mutex _mutex32;
	};

	inline auto& GetNtCallExt() {
		#ifdef _WIN64
		return X64NtCallExt::Instance();
		#else
		return Wow64NtCallExt::Instance();
		#endif
	}

	#define GET_FUNC64(moduleName, funcName) (GetNtCallExt().GetFunc(moduleName, funcName))
	#define GET_NTFUNC64(funcName) (GetNtCallExt().GetFunc(funcName))
	#define CALL64_FUNC(funcName, ...) (GetNtCallExt().X64Call(funcName, __VA_ARGS__))

	#define GET_FUNC(moduleName, funcName) GET_FUNC64(moduleName, funcName)
	#define GET_NTFUNC(funcName) GET_NTFUNC64(funcName)

	#ifndef _WIN64
	#define GET_FUNC32(moduleName, funcName) (Wow64NtCallExt::Instance().GetFunc32(moduleName, funcName))
	#define GET_NTFUNC32(funcName) (Wow64NtCallExt::Instance().GetFunc32(funcName))
	#endif
}
