#include "XMemory.h"
#include "../Common/WinApi/ArchitectureApi.h"

namespace MemX {
	XMemory::XMemory(XContext& context) : _context(context) {}
	XMemory::~XMemory() {}

	XStatus XMemory::Read(PTR_T baseAddr, PVOID pResult, size_t dwSize, bool skipUncommited) {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XStatus::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		DWORD64 dwRead = 0;

		if ( !skipUncommited ) {
			NTSTATUS status = _context.GetRuntime()->ReadProcessMemoryT(baseAddr, pResult, dwSize, &dwRead);
			if ( NT_SUCCESS(status) ) {
				if ( dwRead != dwSize ) return XStatus::Fail(L"Partial copy: " + std::to_wstring(dwRead) + L"/" + std::to_wstring(dwSize));
				return XStatus::Success();
			}
			return XStatus::Fail(L"Read failed," + GetSysErrMsg());
		} else {
			MEMORY_BASIC_INFORMATION64 mbi = { 0 };
			PTR_T currentAddr = baseAddr;
			while ( currentAddr < baseAddr + dwSize ) {
				if ( _context.GetRuntime()->VirtualQueryExT(currentAddr, &mbi) != STATUS_SUCCESS ) {
					currentAddr += ArchitectureApi().GetPageSize();
					continue;
				}
				if ( mbi.State != MEM_COMMIT || (mbi.Protect & PAGE_NOACCESS) == PAGE_NOACCESS ) {
					currentAddr = mbi.BaseAddress + mbi.RegionSize;
					continue;
				}
				PTR_T readStart = currentAddr;
				size_t readSize = (mbi.RegionSize - (readStart - mbi.BaseAddress)) < ((baseAddr + dwSize) - readStart) ? (mbi.RegionSize - (readStart - mbi.BaseAddress)) : ((baseAddr + dwSize) - readStart);
				PTR_T memoffset = readStart - baseAddr;
				NTSTATUS status = _context.GetRuntime()->ReadProcessMemoryT(readStart, (LPVOID) ((BYTE*) pResult + memoffset), readSize, &dwRead);
				if ( !NT_SUCCESS(status) ) 
				currentAddr = readStart + readSize;
				if ( dwRead != readSize ) {
					return XStatus::Fail(L"Partial copy in the loop");
				}
			}
		}
		return XStatus::Success();
	}

	XStatus XMemory::Read(const std::vector<PTR_T>& addrList, PVOID pResult, size_t dwSize, bool skipUncommited) {
		if ( addrList.empty() ) return XStatus::Fail(L"Empty address list");
		if ( addrList.size() == 1 ) return Read(addrList[ 0 ], pResult, dwSize, skipUncommited);
		PTR_T currentAddr = addrList[ 0 ];
		PTR_T pointerValue = 0;
		for ( size_t i = 1; i < addrList.size(); i++ ) {
			NTSTATUS status = _context.GetRuntime()->ReadProcessMemoryT(currentAddr, &pointerValue, sizeof(PTR_T), nullptr);
			if ( !NT_SUCCESS(status) ) {
				return XStatus::Fail(L"Error in pointer chain at index " + std::to_wstring(i - 1));
			}
			currentAddr = pointerValue + addrList[ i ];
		}
		return Read(currentAddr, pResult, dwSize, skipUncommited);
	}

	XStatus XMemory::Write(PTR_T baseAddr, LPCVOID pData, size_t dwSize) {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XStatus::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		NTSTATUS status = _context.GetRuntime()->WriteProcessMemoryT(baseAddr, pData, dwSize);

		if ( NT_SUCCESS(status) ) {
			return XStatus::Success();
		}
		return XStatus::Fail(L"Write failed, " + GetSysErrMsg());
	}

	XStatus XMemory::Write(const std::vector<PTR_T>& addrList, LPCVOID pData, size_t dwSize) {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XStatus::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		if ( addrList.size() == 1 ) return Write(addrList[ 0 ], pData, dwSize);
		PTR_T currentAddr = addrList[ 0 ];
		PTR_T pointerValue = 0;
		for ( size_t i = 1; i < addrList.size(); i++ ) {
			NTSTATUS status = _context.GetRuntime()->ReadProcessMemoryT(currentAddr, &pointerValue, sizeof(PTR_T), nullptr);
			if ( !NT_SUCCESS(status) ) {
				return XStatus::Fail(L"Error in pointer chain at index " + std::to_wstring(i - 1));
			}
			currentAddr = pointerValue + addrList[ i ];
		}
		return Write(currentAddr, pData, dwSize);
	}
}