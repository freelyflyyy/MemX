#pragma once

#include "../WinApi/WinHeaders.h"
#include "../NtApi/NtResult.h"
#include <string>
#include <memory>

namespace MemX {
	template <typename T>
	using result = NtResult<T>;

	using PTR_T = UINT64;

	struct ARCHITECHURE {
		bool sourceWow64 = false;
		bool targetWow64 = false;
		bool mismatch = false;
	};

	struct Module{
		PTR_T BaseAddress;
		std::wstring FullName;
		std::wstring FullPath;
		PTR_T LdrNode;
		BOOL isManual;
		UINT32 Size;
		BOOL IsX86;

		bool operator==(const Module& other) const{
			return this->BaseAddress == other.BaseAddress;
		}

		bool isVild() const {
			return BaseAddress != 0;
		}
	};

	using ModulePtr = std::shared_ptr<const Module>;

	enum MODULE_SEARCH_MODE {
		SCAN_LDR,
		SCAN_SECTION,
		SCAN_PEHEADER
	};


	//Basic Types with NtResult wrapper
	using MxByte = NtResult<BYTE>;
	using MxBool = NtResult<BOOL>;
	using MxWord = NtResult<UINT16>;
	using MxDword = NtResult<UINT32>;
	using MxQword = NtResult<UINT64>;
	using MxFloat = NtResult<FLOAT>;
	using MxDouble = NtResult<DOUBLE>;

	using MxPtr = NtResult<PTR_T>;
	using MxModulePtr = NtResult<ModulePtr>;
}