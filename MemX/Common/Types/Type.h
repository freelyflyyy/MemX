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

	using ptr_t = uintptr_t;
	using ptr_module_t = ptr_t;

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
		ptr_module_t baseAddr;
		std::wstring fullName;
		std::wstring fullPath;
		uint32_t uSize;
		
		bool operator==(const ModuleInfo& other) const{
			return this->baseAddr == other.baseAddr;
		}
	};
	using PModuleData = std::shared_ptr<const ModuleInfo>;
}