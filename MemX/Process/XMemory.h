#pragma once
#include "XContext.h"
#include "../Common/WinApi/WinHeaders.h"
#include "../Common/Types/XTypes.h"
#include "../Common/Types/XResult.h"
#include "../Common/Utils/Utils.h"
#include <vector>

namespace MemX {
	class XMemory {
		public:
		explicit XMemory(XContext& context);
		~XMemory();

		XStatus Read(PTR_T baseAddr, PVOID pResult, size_t dwSize, bool skipUncommited);
		XStatus Read(const std::vector<PTR_T>& addrList, PVOID pResult, size_t dwSize, bool skipUncommited);
		XStatus Write(PTR_T baseAddr, LPCVOID pData, size_t dwSize);
		XStatus Write(const std::vector<PTR_T>& addrList, LPCVOID pData, size_t dwSize);

		template<typename T>
		XResult<T> Read(PTR_T baseAddr) {
			if (!baseAddr) {
				return XStatus::Fail(L"Invalid memory address (0)");
			}

			T buffer{};
			NTSTATUS status = Read(baseAddr, &buffer, sizeof(T), false);
			if ( !NT_SUCCESS(status) ) {
				return XResult<T>::Fail(L"Failed to read memory at address: " + std::to_wstring(baseAddr) + L"\n" + GetSysErrMsg());
			}
			return XResult<T>::Success(std::move(buffer));
		}
		template<typename T>
		XResult<T> Read(std::vector<PTR_T> addList) {
			if ( addList.empty() ) {
				return XResult<T>::Fail(L"Pointer chain is empty");
			}

			T buffer = {};
			NTSTATUS status = Read(addList, &buffer, sizeof(T), false);
			if ( !NT_SUCCESS(status) ) {
				return XResult<T>::Fail(L"Failed to read pointer chain\n" + GetSysErrMsg());
			}
			return XResult<T>::Success(std::move(buffer));
		}
		template<typename T>
		XStatus Write(PTR_T baseAddr, const T& data) {
			return Write(baseAddr, reinterpret_cast<LPCVOID>(&data), sizeof(T));
		}
		template<typename T>
		XStatus Write(const std::vector<PTR_T>& addrList, const T& data) {
			return Write(addrList, reinterpret_cast<LPCVOID>(&data), sizeof(T));
		}

		private:
		XContext& _context;
	};
}