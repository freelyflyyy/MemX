#include "NtCallExt.h"
#include "NtApi.h"
#include "../Utils/Wow64Utils.h"
#include <cstdarg>
#include <vector>

namespace MemX {

	//////////////////////////////////////////////////////////////////////////
	// X64NtCallExt: 64位原生实现
	//////////////////////////////////////////////////////////////////////////

	X64NtCallExt& X64NtCallExt::Instance() {
		static X64NtCallExt instance;
		return instance;
	}

	X64NtCallExt::X64NtCallExt() {
	}

	MEMX_API DWORD64 __cdecl X64NtCallExt::GetProcAddress64(DWORD64 hMod, const char* funcName) {
		if (!hMod || !funcName) {
			return 0;
		}
		return (DWORD64)GetProcAddress((HMODULE)hMod, funcName);
	}

	MEMX_API DWORD64 __cdecl X64NtCallExt::GetModuleBase64(const wchar_t* moduleName) {
		if (!moduleName) {
			return 0;
		}
		return (DWORD64)GetModuleHandleW(moduleName);
	}

	MEMX_API DWORD64 __cdecl X64NtCallExt::GetNtdll64() {
		static DWORD64 _ntdll64 = 0;
		if (_ntdll64 != 0) {
			return _ntdll64;
		}
		_ntdll64 = (DWORD64)GetModuleHandleW(L"ntdll.dll");
		return _ntdll64;
	}


	MEMX_API DWORD64 __cdecl X64NtCallExt::GetTeb64() {
		Reg64 _teb64 = { 0 };
		#ifdef _WIN64
			_teb64.v = __readgsqword(FIELD_OFFSET(NT_TIB, Self));
		#endif
		return _teb64.v;
	}

	MEMX_API DWORD64 __cdecl X64NtCallExt::GetPeb64() {
		Reg64 _peb64 = { 0 };
		#ifdef _WIN64
			_peb64.v = __readgsqword(FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
		#endif
		return _peb64.v;
	}

	//////////////////////////////////////////////////////////////////////////
	// Wow64NtCallExt: WoW64实现 (Heaven's Gate)
	//////////////////////////////////////////////////////////////////////////

	Wow64NtCallExt& Wow64NtCallExt::Instance() {
		static Wow64NtCallExt instance;
		return instance;
	}

	Wow64NtCallExt::Wow64NtCallExt() {
	}

	MEMX_API DWORD64 __cdecl Wow64NtCallExt::GetProcAddress64(DWORD64 hMod, const char* funcName) {
		static DWORD64 ldrGetProcedureAddress = 0;
		if ( !ldrGetProcedureAddress ) {
			ldrGetProcedureAddress = GetLdrGetProcedureAddress();

			if ( ldrGetProcedureAddress == 0 ) return 0;
		}
		_UNICODE_STRING_T<DWORD64> fName = { 0 };
		fName.Buffer = (DWORD64) funcName;
		fName.Length = (WORD) strlen(funcName);
		fName.MaximumLength = fName.Length + 1;

		DWORD64 rect = 0;
		X64CallVa(ldrGetProcedureAddress, 4, (DWORD64) hMod, (DWORD64) &fName, (DWORD64) 0, (DWORD64) &rect);
		return rect;
	}

	MEMX_API DWORD64 __cdecl Wow64NtCallExt::GetModuleBase64(const wchar_t* moduleName) {
		DWORD64 teb64Addr = GetTeb64();
		if (teb64Addr == 0) {
			return 0;
		}

		TEB64 _teb64 = { 0 };
		memcpy64(&_teb64, teb64Addr, sizeof(TEB64));

		if (_teb64.ProcessEnvironmentBlock == 0) {
			return 0;
		}

		PEB64 _peb64 = { 0 };
		memcpy64(&_peb64, _teb64.ProcessEnvironmentBlock, sizeof(PEB64));

		if (_peb64.Ldr == 0) {
			return 0;
		}

		PEB_LDR_DATA64 _ldr64;
		memcpy64(&_ldr64, _peb64.Ldr, sizeof(PEB_LDR_DATA64));
		LDR_DATA_TABLE_ENTRY64 head;

		DWORD64 LastEntry = _peb64.Ldr + offsetof(PEB_LDR_DATA64, InLoadOrderModuleList);
		head.InLoadOrderLinks.Flink = _ldr64.InLoadOrderModuleList.Flink;
		do {
			memcpy64(&head, head.InLoadOrderLinks.Flink, sizeof(LDR_DATA_TABLE_ENTRY64));

			std::vector<WCHAR> tempbuf(head.BaseDllName.MaximumLength / sizeof(WCHAR) + 1, 0);
			if (tempbuf.empty())
				return 0;
			memcpy64(tempbuf.data(), head.BaseDllName.Buffer, head.BaseDllName.Length);

			tempbuf[head.BaseDllName.Length / sizeof(WCHAR)] = L'\0';

			if (0 == _wcsicmp(tempbuf.data(), moduleName))
				return head.DllBase;

		} while (head.InLoadOrderLinks.Flink != LastEntry);

		return 0;
	}

	#pragma warning(push)
	#pragma warning(disable : 4409)
	MEMX_API DWORD64 __cdecl Wow64NtCallExt::X64CallVa(DWORD64 func, int argC, ...) {
		if (func == 0) return 0;

		va_list args;
		va_start(args, argC);
		Reg64 _rcx = { (argC > 0) ? argC--, va_arg(args, DWORD64) : 0 };
		Reg64 _rdx = { (argC > 0) ? argC--, va_arg(args, DWORD64) : 0 };
		Reg64 _r8 = { (argC > 0) ? argC--, va_arg(args, DWORD64) : 0 };
		Reg64 _r9 = { (argC > 0) ? argC--, va_arg(args, DWORD64) : 0 };
		Reg64 _rax = { 0 };

		Reg64 restArgs = { (DWORD64)&va_arg(args, DWORD64) };
		Reg64 _argC = { (DWORD64)argC };
		DWORD back_esp = 0;
		WORD back_fs = 0;
	#ifdef _M_IX86
		__asm
		{
			mov    back_fs, fs
			mov    eax, 0x2B
			mov    fs, ax

			mov    back_esp, esp

			and    esp, 0xFFFFFFF0

			x64_start

	  rex_w mov    ecx, _rcx.dw[0]
	  rex_w mov    edx, _rdx.dw[0]
			
			push   _r8.v
			x64_pop(r8)
			
			push   _r9.v
			x64_pop(r9)
			
	  rex_w mov    eax, _argC.dw[0]

			test   al, 1
			jnz    _no_adjust
			sub    esp, 8
			
		_no_adjust:
			push   edi
	  rex_w mov    edi, restArgs.dw[0]
	  rex_w test   eax, eax
			jz     _ls_e
	  rex_w lea    edi, dword ptr[edi + 8 * eax - 8]
			
		_ls:
	  rex_w test   eax, eax
			jz     _ls_e
			push   dword ptr[edi]
	  rex_w sub    edi, 8
	  rex_w sub    eax, 1
			jmp    _ls
			
		_ls_e:
	  rex_w sub    esp, 0x20
			call   func

	  rex_w mov    ecx, _argC.dw[0]
	  rex_w lea    esp, dword ptr[esp + 8 * ecx + 0x20]
			pop    edi
	  rex_w mov    _rax.dw[0], eax

			x64_end

			mov    ax, ds
			mov    ss, ax
			mov    esp, back_esp
			mov    ax, back_fs
			mov    fs, ax
		}
	#endif
		va_end(args);
		return _rax.v;
	}
	#pragma warning(pop)

	MEMX_API DWORD64 __cdecl Wow64NtCallExt::GetNtdll64() {
		static DWORD64 _ntdll64 = 0;
		if ( _ntdll64 != 0 ) {
			return _ntdll64;
		}
		_ntdll64 = GetModuleBase64(L"ntdll.dll");
		return _ntdll64;
	}

	DWORD64 __cdecl Wow64NtCallExt::GetLdrGetProcedureAddress() {
		DWORD64 dllBase = GetNtdll64();
		if (!dllBase) {
			return 0;
		}
		IMAGE_DOS_HEADER idh;
		memcpy64(&idh, dllBase, sizeof(IMAGE_DOS_HEADER));

		IMAGE_NT_HEADERS64 inth;
		memcpy64(&inth, dllBase + idh.e_lfanew, sizeof(IMAGE_NT_HEADERS64));

		IMAGE_EXPORT_DIRECTORY ied;
		memcpy64(&ied, dllBase + inth.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, sizeof(IMAGE_EXPORT_DIRECTORY));

		std::vector<DWORD> rvaTable(ied.NumberOfFunctions, 0);
		std::vector<DWORD> nameTable(ied.NumberOfNames, 0);
		std::vector<WORD> ordTable(ied.NumberOfNames, 0);

		memcpy64(rvaTable.data(), dllBase + ied.AddressOfFunctions, ied.NumberOfFunctions * sizeof(DWORD));
		memcpy64(nameTable.data(), dllBase + ied.AddressOfNames, ied.NumberOfNames * sizeof(DWORD));
		memcpy64(ordTable.data(), dllBase + ied.AddressOfNameOrdinals, ied.NumberOfNames * sizeof(WORD));

		for (DWORD i = 0; i < ied.NumberOfNames; i++) {
			char funcName[256] = { 0 };
			memcpy64(funcName, dllBase + nameTable[i], sizeof(funcName) - 1);
			if (strcmp(funcName, "LdrGetProcedureAddress") != 0)
				continue;
			else
				return dllBase + rvaTable[ordTable[i]];
		}
		return 0;
	}

	MEMX_API DWORD64 __cdecl Wow64NtCallExt::GetTeb64() {
		Reg64 _teb64 = { 0 };
		#ifdef _M_IX86
		__asm {
			x64_start
			x64_push(r12);
	        pop    _teb64.dw[0] 
			x64_end
		}
		#endif
		return _teb64.v;
	}

	MEMX_API DWORD64 __cdecl Wow64NtCallExt::GetPeb64() {
		Reg64 _peb64 = { 0 };
	#ifdef _M_IX86
		__asm {
			// 保存栈指针
			x64_start
			x64_push(r12);
			x64_pop(rax);
	  rex_w mov    eax, [ eax + 0x60 ]
	  rex_w mov    _peb64.dw[ 0 ], eax

			x64_end
		}
	#endif // _M_IX86
		return _peb64.v;
	}
} // namespace MemX