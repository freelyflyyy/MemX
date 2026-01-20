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


	struct ARCHITECHURE {
		bool sourceWow64 = false;
		bool targetWow64 = false;
		bool mismatch = false;
	};

	struct ModuleInfo{
		PTR_MODULE_T baseAddr;
		std::wstring fullName;
		std::wstring fullPath;
		PTR_T ldrPoint;
		BOOL isManual;
		UINT32 uSize;
		BOOL isX86;

		bool operator==(const ModuleInfo& other) const{
			return this->baseAddr == other.baseAddr;
		}

		bool isVild() const {
			return baseAddr != 0;
		}
	};

	using ModuleInfoPtr = std::shared_ptr<const ModuleInfo>;

	enum MODULE_SEARCH_MODE {
		SCAN_LDR,
		SCAN_SECTION,
		SCAN_FROCE
	};
}