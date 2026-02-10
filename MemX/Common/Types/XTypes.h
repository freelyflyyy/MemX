#pragma once

#include "../WinApi/WinHeaders.h"
#include "XResult.h"
#include <string>
#include <memory>

namespace MemX {
	template <typename T>
	using result = XResult<T>;

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

	enum MODULE_SEARCH_MODE {
		SCAN_LDR,
		SCAN_SECTION,
		SCAN_PEHEADER
	};

	struct WindowInfo {
		HWND hWindow;           //Window handle
		std::wstring Title;     //Window Title
		std::wstring ClassName; //Window class name
		RECT WindowRect;        //Window rectangle (screen coordinates)
		RECT ClientRect;        //Client area rectangle (screen coordinates)
		int Width;              //Total window width
		int Height;             //Total height of window
		int ClientWidth;        //Width of customer area
		int ClientHeight;       //Height of customer area
		BOOL IsVisible;         //Visible or not

		bool IsValid() const { return hWindow != NULL; }
	};


	//Basic Types with XResult wrapper
	using XByte = XResult<BYTE>;
	using XWord = XResult<UINT16>;
	using XDword = XResult<UINT32>;
	using XQword = XResult<UINT64>;
	using XFloat = XResult<FLOAT>;
	using XDouble = XResult<DOUBLE>;

	using XPtr = XResult<PTR_T>;
	using XModule = XResult<Module>;
}