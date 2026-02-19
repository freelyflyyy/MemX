#pragma once
#include "../WinApi/WinHeaders.h"
#include "../Types/XTypes.h"

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

	static VOID memcpy64(VOID* dest, DWORD64 src, SIZE_T sz) {
		if ((nullptr == dest) || (0 == src) || (0 == sz))
			return;

	#ifdef _M_IX86
		Reg64 _src = { src };
		__asm {
			x64_start

			push	edi
			push	esi

			mov		edi, dest
	rex_w	mov	    esi, dword ptr[ _src ]
			mov		ecx, sz

			mov		eax, ecx
			and eax, 3
			shr		ecx, 2

			rep		movsd
			test	eax, eax
			je		_remain_0

			cmp		eax, 1
			je		_remain_1

			movsw

			cmp		eax, 2
			je		_remain_0

			_remain_1 :
			movsb

			_remain_0 :
			pop		esi
			pop		edi

			x64_end
		}
	#endif
	}
} // namespace MemX
