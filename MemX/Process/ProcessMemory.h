#pragma once

#include "../Common/WinApi/WinHeaders.h"
#include "../Common/Types/Type.h"
#include "../Common/NtApi/NtResult.h"

#include <vector>

namespace MemX {


	class ProcessMemory {
		public:
		 ProcessMemory(class Process* process);
		 ~ProcessMemory();

		 NTSTATUS Read(PTR_T baseAddr, PVOID pResult, size_t dwSize, bool skipUncommited);

		 NTSTATUS Read(const std::vector<PTR_T>& addrList, PVOID pResult, size_t dwSize, bool skipUncommited);

		 NTSTATUS Write(PTR_T baseAddr, LPCVOID pData, size_t dwSize);

		 NTSTATUS Write(const std::vector<PTR_T>& addrList, LPCVOID pData, size_t dwSize);

		template<typename T>
		NtResult<T> Read(PTR_T baseAddr) {
			auto pResult = std::make_unique<T>();
			NTSTATUS status = Read(baseAddr, pResult.get(), sizeof(T), false);
			return NtResult<T>(*pResult, status);
		}

		template<typename T>
		NtResult<T> Read(std::vector<PTR_T>&& addList) {
			auto pResult = std::make_unique<T>();
			NTSTATUS status = Read(std::forward<std::vector<PTR_T>>(addList), pResult.get(), sizeof(T), false);
			return NtResult<T>(*pResult, status);
		}


		template<typename T>
		NTSTATUS Write(PTR_T baseAddr, const T& data) {
			return Write(baseAddr, reinterpret_cast<LPCVOID>(&data), sizeof(T));
		}

		template<typename T>
		NTSTATUS Write(const std::vector<PTR_T>&& addrList, const T& data) {
			return Write(std::forward<std::vector<ptr_t>>(addrList), reinterpret_cast<LPCVOID>(&data), sizeof(T));
		}


		private:
		ProcessMemory(const ProcessMemory&) = delete;
		ProcessMemory& operator=(const ProcessMemory&) = delete;

		private:
		class Process* _process;
		class ProcessCore& _core;
	};
}