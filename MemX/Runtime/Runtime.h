#pragma once

#include "../Common/WinApi/WinHeaders.h"
#include "../Common/WinApi/ArchitectureApi.h"
#include "../Common/NtApi/NtApi.h"
#include "../Common/Types/Type.h"



namespace MemX {
	class Runtime {
		public:
		 Runtime(HANDLE hProcess) : _hProcess(hProcess) {
			ArchitectureApi texApi(hProcess);
			_barrier = texApi.GetWow64Barrier();
		};
		 ~Runtime() {};

		PTR_T maxAddr32() {
			return 0x7FFFFFFF;
		}

		PTR_T maxAddr64() {
			return 0x7FFFFFFFFFFF;
		}

		BOOL isTargetWow64() const {
			return _barrier.targetWow64;
		}

		/// <summary>
		/// 获取目标进程的32位PEB结构
		/// </summary>
		/// <param name="peb">[Out] 回传指针</param>
		/// <returns>NTSTATUS 状态码，表示操作结果（例如成功或失败的具体状态）。</returns>
		/// <note>注意：如果目标进程不是Wow64进程则返回空PEB32结构</note>
		virtual NTSTATUS GetTargetPeb(PEB32* peb) = 0;

		/// <summary>
		/// 获取目标进程的64位PEB结构
		/// </summary>
		/// <param name="peb">[Out] 回传指针</param>
		/// <returns>NTSTATUS 状态码，表示操作结果（例如成功或失败的具体状态）。</returns>
		/// <note>注意：如果目标进程是32位，调用该函数得到的是64位DWORD，将不可按照常规的方法进行值传递</note>
		virtual NTSTATUS GetTargetPeb(PEB64* peb) = 0;

		virtual NTSTATUS GetTargetLdr(PEB_LDR_DATA32* ldrData) = 0;

		virtual NTSTATUS GetTargetLdr(PEB_LDR_DATA64* ldrData) = 0;

		/// <summary>
		/// 纯虚方法：将目标进程（或指定地址空间）中从 lpBaseAddress 起始的内存内容读取到提供的缓冲区，需由派生类实现。
		/// </summary>
		/// <param name="lpBaseAddress">源地址，表示要读取的起始内存地址（目标进程地址空间中的指针或地址）。</param>
		/// <param name="lpBuffer">指向接收数据的本地缓冲区。调用者必须保证该缓冲区至少可写 dwSize 字节。</param>
		/// <param name="dwSize">要读取的字节数。</param>
		/// <param name="readBytes">指向 DWORD64 的输出参数，用于接收实际读取的字节数；调用返回时写入已读取的字节数。是否允许为 nullptr 由具体实现决定。</param>
		/// <returns>NTSTATUS 状态码，表示操作结果（例如成功或失败的具体状态）。</returns>
		virtual NTSTATUS ReadProcessMemoryT(PTR_T lpBaseAddress, LPVOID lpBuffer, SIZE_T dwSize, DWORD64* readBytes) = 0;

		virtual NTSTATUS WriteProcessMemoryT(PTR_T lpBaseAddress, LPCVOID lpBuffer, SIZE_T dwSize, DWORD64* writtenBytes = NULL) = 0;

		virtual NTSTATUS VirtualQueryExT(PTR_T lpAddress, PMEMORY_BASIC_INFORMATION64 lpBuffer) = 0;

		virtual NTSTATUS GetAllModulesByLdrList32(std::vector<ModuleInfoPtr>* pModulesEntry) = 0;

		virtual NTSTATUS GetAllModulesByLdrList64(std::vector<ModuleInfoPtr>* pModulesEntry) = 0;

		virtual NTSTATUS GetAllModulesByPEHeaders32(std::vector<ModuleInfoPtr>* pModulesEntry) = 0;

		virtual NTSTATUS GetAllModulesByPEHeaders64(std::vector<ModuleInfoPtr>* pModulesEntry) = 0;

		virtual NTSTATUS GetAllModulesBySections32(std::vector<ModuleInfoPtr>* pModulesEntry) = 0;

		virtual NTSTATUS GetAllModulesBySections64(std::vector<ModuleInfoPtr>* pModulesEntry) = 0;

		virtual NTSTATUS GetAllModulesT(std::vector<ModuleInfoPtr>* pModulesEntry, ModuleSearchMode& moduleSearchMode) = 0;

		protected:
		HANDLE _hProcess;
		Wow64Barrier _barrier;
	};
}

