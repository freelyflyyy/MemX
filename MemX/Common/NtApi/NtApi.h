#pragma once

#include "NtStructures.h"
#include "NtApiResolver.h"

// This header provides global access to NT API functions loaded at runtime.
// Include this file in any .cpp file where you need to call these functions.
// You must call Bootstrap() once at program startup before using any of these.

namespace MemX {
	// NtDll Functions
	typedef NTSTATUS(NTAPI* fnNtQuerySystemInformation)(
		_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
		_Out_ PVOID SystemInformation,
		_In_ ULONG SystemInformationLength,
		_Out_opt_ PULONG ReturnLength
		);

	typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(
		_In_ HANDLE ProcessHandle,
		_In_ PROCESS_INFORMATION_CLASS ProcessInformationClass,
		_Out_ PVOID ProcessInformation,
		_In_ ULONG ProcessInformationLength,
		_Out_opt_ PULONG ReturnLength
		);

	typedef NTSTATUS(NTAPI* fnRtlGetLastNtStatus)();

	typedef ULONG(NTAPI* fnRtlSetLastWin32ErrorAndNtStatusFromNtStatus)(
		_In_ NTSTATUS Status
		);


	// Global pointers to the dynamically loaded NT API functions.

	inline fnNtQuerySystemInformation pfnNtQuerySystemInformation = nullptr;
	inline fnNtQueryInformationProcess pfnNtQueryInformationProcess = nullptr;
	inline fnRtlGetLastNtStatus pfnRtlGetLastNtStatus = nullptr;
	inline fnRtlSetLastWin32ErrorAndNtStatusFromNtStatus pfnRtlSetLastWin32ErrorAndNtStatusFromNtStatus = nullptr;

	//Macro definitions to allow calling NT API functions directly by their name.
	//If so, it would be better to determine whether the function pointer exists

	#define NtQuerySystemInformation pfnNtQuerySystemInformation
	#define NtQueryInformationProcess pfnNtQueryInformationProcess
	#define RtlGetLastNtStatus pfnRtlGetLastNtStatus
	#define RtlSetLastWin32ErrorAndNtStatusFromNtStatus pfnRtlSetLastWin32ErrorAndNtStatusFromNtStatus


	//NTSTATUS offset in TEB structure 
	//for different architectures
	#ifdef _WIN64
		constexpr size_t NT_STATUS_OFFSET = 0x1250;
	#else
		constexpr size_t NT_STATUS_OFFSET 0x00B4
	#endif

	inline NTSTATUS GetLastNtStatus() {
		return *(NTSTATUS*)((BYTE*)NtCurrentTeb() + NT_STATUS_OFFSET);
	}

	inline VOID SetLastNtStatus(NTSTATUS status) {
		*(NTSTATUS*)((BYTE*)NtCurrentTeb() + NT_STATUS_OFFSET) = status;
	}
}
