#pragma once

#include "../WinApi/WinHeaders.h"
#include "XResult.h"
#include <string>
#include <memory>

namespace MemX {
	template <typename T>
	using result = XResult<T>;

	using PTR_T = UINT64;

	enum class ModuleArch {
		x64,
		x86,
		Default
	};

	typedef struct _ModuleInfo{
		PTR_T BaseAddress;    //Module base address
		std::wstring FullName;//Full name of the module
		std::wstring FullPath;//Full path of the module
		PTR_T LdrNode;        //The address of the module's LDR_DATA_TABLE_ENTRY in the target process, if available. Otherwise, it will be set to 0.
		BOOL isManual;        //Whether the module is manually mapped
		UINT32 Size;          //Module size in bytes
		ModuleArch mArch;           //Whether the module is a 32-bit module

		_ModuleInfo()
			: BaseAddress(0), LdrNode(0), isManual(FALSE), Size(0), mArch(ModuleArch::Default) {
		}

		_ModuleInfo(PTR_T base, UINT32 size, const std::wstring& name, ModuleArch mArch)
			: BaseAddress(base),
			Size(size),
			FullName(name),
			LdrNode(0),
			isManual(FALSE),
			mArch(mArch) {
		}

		_ModuleInfo(PTR_T base, const std::wstring& name, const std::wstring& path, PTR_T ldr, BOOL manual, UINT32 sz, ModuleArch mArch)
			: BaseAddress(base),
			FullName(name),
			FullPath(path),
			LdrNode(ldr),
			isManual(manual),
			Size(sz),
			mArch(mArch) {
		}

		bool operator==(const _ModuleInfo& other) const{
			return this->BaseAddress == other.BaseAddress;
		}

		bool isVild() const {
			return BaseAddress != 0;
		}
	} ModuleInfo;

	typedef struct _WindowInfo {
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
	} WindowInfo;

	typedef std::shared_ptr<ModuleInfo> ModulePtr;
	typedef std::shared_ptr<WindowInfo> WindowPtr;


	//Basic Types with XResult wrapper
	using XByte = XResult<BYTE>;
	using XWord = XResult<UINT16>;
	using XDword = XResult<UINT32>;
	using XQword = XResult<UINT64>;
	using XFloat = XResult<FLOAT>;
	using XDouble = XResult<DOUBLE>;

	using XPtr = XResult<PTR_T>;
	using XModulePtr = XResult<ModulePtr>;
	using XWindowPtr = XResult<WindowPtr>;
}