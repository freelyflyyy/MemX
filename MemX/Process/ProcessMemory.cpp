#include "ProcessMemory.h"
#include "Process.h"
#include "../Common/WinApi/ArchitectureApi.h"

namespace MemX
{

	ProcessMemory::ProcessMemory(Process *process) : _process(process), _core(process->Core())
	{
	}

	ProcessMemory::~ProcessMemory()
	{
	}

	MEMX_API NTSTATUS ProcessMemory::Read(PTR_T baseAddr, PVOID pResult, size_t dwSize, bool skipUncommited)
	{
		DWORD64 dwRead = 0;
		if (!baseAddr)
		{
			return STATUS_INVALID_ADDRESS;
		}

		if (!skipUncommited)
		{
			NTSTATUS status = _core.getRuntime()->ReadProcessMemoryT(baseAddr, pResult, dwSize, &dwRead);
			if (NT_SUCCESS(status) && dwRead != dwSize)
			{
				return STATUS_PARTIAL_COPY;
			}
			return status;
		}
		else
		{
			MEMORY_BASIC_INFORMATION64 mbi = {0};
			PTR_T currentAddr = baseAddr;

			while (currentAddr < baseAddr + dwSize)
			{
				if (_core.getRuntime()->VirtualQueryExT(currentAddr, &mbi) != STATUS_SUCCESS)
				{
					currentAddr += ArchitectureApi().GetPageSize();
					continue;
				}
				if (mbi.State != MEM_COMMIT || (mbi.Protect & PAGE_NOACCESS) == PAGE_NOACCESS)
				{
					currentAddr = mbi.BaseAddress + mbi.RegionSize;
					continue;
				}

				PTR_T readStart = currentAddr;
				size_t readSize = (mbi.RegionSize - (readStart - mbi.BaseAddress)) < ((baseAddr + dwSize) - readStart) ? (mbi.RegionSize - (readStart - mbi.BaseAddress)) : ((baseAddr + dwSize) - readStart);

				PTR_T memoffset = readStart - baseAddr;
				NTSTATUS status = _core.getRuntime()->ReadProcessMemoryT(readStart, (LPVOID)((BYTE *)pResult + memoffset), readSize, &dwRead);

				if (!NT_SUCCESS(status))
				{
					return status;
				}

				currentAddr = readStart + readSize;

				if (dwRead != readSize)
				{
					return STATUS_PARTIAL_COPY;
				}
			}
		}
		return STATUS_SUCCESS;
	}

	// addr list read - pointer chain traversal
	MEMX_API NTSTATUS ProcessMemory::Read(const std::vector<PTR_T> &addrList, PVOID pResult, size_t dwSize, bool skipUncommited)
	{
		if (addrList.empty())
		{
			return STATUS_INVALID_PARAMETER;
		}

		if (addrList.size() == 1)
		{
			return Read(addrList[0], pResult, dwSize, skipUncommited);
		}

		PTR_T currentAddr = addrList[0];
		PTR_T pointerValue = 0;

		for (size_t i = 1; i < addrList.size(); i++)
		{
			NTSTATUS status = _core.getRuntime()->ReadProcessMemoryT(currentAddr, &pointerValue, sizeof(PTR_T), nullptr);
			if (!NT_SUCCESS(status))
			{
				return status;
			}
			currentAddr = pointerValue + addrList[i];
		}
		return Read(currentAddr, pResult, dwSize, skipUncommited);
	}

	MEMX_API NTSTATUS ProcessMemory::Write(PTR_T baseAddr, LPCVOID pData, size_t dwSize)
	{
		if (!baseAddr)
			return STATUS_INVALID_ADDRESS;
		return _core.getRuntime()->WriteProcessMemoryT(baseAddr, pData, dwSize);
	}

	// addr list write - pointer chain traversal
	MEMX_API NTSTATUS ProcessMemory::Write(const std::vector<PTR_T> &addrList, LPCVOID pData, size_t dwSize)
	{
		if (addrList.empty())
		{
			return STATUS_INVALID_PARAMETER;
		}

		if (addrList.size() == 1)
		{
			return Write(addrList[0], pData, dwSize);
		}

		PTR_T currentAddr = addrList[0];
		PTR_T pointerValue = 0;

		for (size_t i = 1; i < addrList.size(); i++)
		{
			NTSTATUS status = _core.getRuntime()->ReadProcessMemoryT(currentAddr, &pointerValue, sizeof(PTR_T), nullptr);
			if (!NT_SUCCESS(status))
			{
				return status;
			}
			currentAddr = pointerValue + addrList[i];
		}
		return Write(currentAddr, pData, dwSize);
	}
}