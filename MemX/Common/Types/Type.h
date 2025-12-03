#pragma once

#include "../WinApi/WinHeaders.h"
#include "../NtApi/NtResult.h"
#include <cstdint>
#include <cassert>
#include <optional>
#include <memory>
#include <string>

namespace MemX {
	template <typename T>
	using result = NtResult<T>;

	using PTR_T = UINT64;
	using PTR_MODULE_T = PTR_T;

	union Reg64 {
		DWORD64 v;
		DWORD dw[ 2 ];
	};

	enum ArchBarrier {
		ARCHITECTURE_32_32,//Both processes are Wow64
		ARCHITECTURE_64_64,//Both processes are x64
		ARCHITECTURE_64_32,//Control Wow64 process from x64;
		ARCHITECTURE_32_64,//Control x64 process from Wow64 
		ARCHITECTURE_UNSUPPORTED
	};

	struct Wow64Barrier {
		ArchBarrier archType = ARCHITECTURE_UNSUPPORTED;
		bool sourceWow64 = false;
		bool targetWow64 = false;
		bool mismatch = false;
	};

	struct ModuleInfo{
		PTR_MODULE_T baseAddr;
		std::wstring fullName;
		std::wstring fullPath;
		UINT32 uSize;
		
		bool operator==(const ModuleInfo& other) const{
			return this->baseAddr == other.baseAddr;
		}
	};
	using PModuleInfo = std::shared_ptr<const ModuleInfo>;
}