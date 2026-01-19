#pragma once
#include "ProcessMemory.h"
#include "ProcessCore.h"
#include "ProcessModule.h"
#include "../Common/WinApi/WinHeaders.h"
#include "../Common/NtApi/NtResult.h"

#include <string>
#include <list>


namespace MemX {

	//<summary>
	// Contains information about a process.
	//</summary>
	struct ProcessInfo {
		uint32_t pid = 0;
		std::wstring imageName;

		bool operator < (const ProcessInfo& other) const {
			return this->pid < other.pid;
		}
	};

	#define DEFAULT_NORMAL_PROCESS_ACCESS \
									 PROCESS_VM_READ		   |\
                                	 PROCESS_VM_WRITE          |\
									 PROCESS_VM_OPERATION      |\
									 PROCESS_QUERY_INFORMATION |\
									 PROCESS_CREATE_THREAD
	 

	#define DEFAULT_ALL_PROCESS_ACCESS  \
	                          PROCESS_QUERY_INFORMATION | \
							  PROCESS_VM_READ           | \
                              PROCESS_VM_WRITE          | \
                              PROCESS_VM_OPERATION      | \
                              PROCESS_CREATE_THREAD     | \
                              PROCESS_SET_QUOTA         | \
                              PROCESS_TERMINATE         | \
                              PROCESS_SUSPEND_RESUME    | \
                              PROCESS_DUP_HANDLE

	class Process {
		public:
		 Process();
		 ~Process(void);

		
		 NTSTATUS Catch(DWORD pid, DWORD access = DEFAULT_NORMAL_PROCESS_ACCESS);

		 NTSTATUS Catch(const wchar_t* processName, DWORD access = DEFAULT_NORMAL_PROCESS_ACCESS);

		 NTSTATUS Catch(HANDLE proHandle);

		 NTSTATUS Drop();

		 static NTSTATUS GetPidByName(const std::wstring& processName);

		 ProcessMemory& Memory() { return _memory; }
		 ProcessCore& Core() { return _core; }
		 ProcessModule& Module() { return _module; }

		private:
		Process(const Process&) = delete;
		Process& operator =(const Process&) = delete;


		private:
		// Ensure ProcessCore is declared before ProcessMemory so it is constructed first.
		ProcessCore _core;
		ProcessMemory _memory;
		ProcessModule _module;
	};
}
