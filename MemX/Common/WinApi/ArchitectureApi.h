#pragma once
#include "../Types/XTypes.h"
#include "../WinApi/WinHeaders.h"

namespace MemX {
	struct ARCHITECHURE {
		bool sourceWow64 = false;
		bool targetWow64 = false;
		bool mismatch = false;
	};

	class ArchitectureApi {
		public:
		ArchitectureApi() {
			SYSTEM_INFO sysInfo = { 0 };
			GetNativeSystemInfo(&sysInfo);
			_dwPageSize = sysInfo.dwPageSize;
		}

		ArchitectureApi(HANDLE _hProcess) : ArchitectureApi() {
			BOOL isSourceWow64 = FALSE, isTargetWow64 = FALSE;
			IsWow64Process(GetCurrentProcess(), (BOOL*) &isSourceWow64);
			IsWow64Process(_hProcess, (BOOL*) &isTargetWow64);

			_arch.sourceWow64 = (isSourceWow64 == TRUE);
			_arch.targetWow64 = (isTargetWow64 == TRUE);
		}
		~ArchitectureApi() {

		}

		const ARCHITECHURE& GetArchitechure() const { return _arch; }
		uint32_t GetPageSize() const { return _dwPageSize; }

		private:
		ARCHITECHURE _arch;
		uint32_t _dwPageSize;
	};
}