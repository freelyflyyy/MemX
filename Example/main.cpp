#include <iostream>
#include <MemX/Common/NtApi/Bootstrap.h>
#include <MemX/Common/NtApi/NtApi.h>
#include <MemX/Common/NtApi/NtStructures.h>

// Define NT_SUCCESS macro if not already defined
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

int main()
{
    // 1. Initialize the NtApi function pointers
    std::cout << "Initializing NtApi..." << std::endl;
    MemX::loadNtFunc();

    // Check if the essential function pointer was loaded
    if (MemX::pfnNtQueryInformationProcess == nullptr)
    {
        std::cerr << "Failed to load NtQueryInformationProcess. Aborting test." << std::endl;
        return 1;
    }
    std::cout << "NtQueryInformationProcess loaded successfully." << std::endl;

    // 2. Get the handle to the current process
    HANDLE hProcess = GetCurrentProcess();
    std::cout << "Testing with current process handle: " << hProcess << std::endl;

    // 3. Call NtQueryInformationProcess
    PROCESS_BASIC_INFORMATION pbi;
    ULONG returnLength = 0;

    std::cout << "Calling NtQueryInformationProcess..." << std::endl;
    NTSTATUS status = MemX::NtQueryInformationProcess(
        hProcess,
        static_cast<PROCESS_INFORMATION_CLASS>(ProcessBasicInformation),
        &pbi,
        sizeof(pbi),
        &returnLength);

    // 4. Validate the result
    if (NT_SUCCESS(status))
    {
        std::cout << "SUCCESS: NtQueryInformationProcess returned success status." << std::endl;
        std::cout << "PEB Base Address: 0x" << pbi.PebBaseAddress << std::endl;
        std::cout << "Process ID: " << pbi.UniqueProcessId << std::endl;
    }
    else
    {
        std::cerr << "FAILURE: NtQueryInformationProcess returned error status: 0x" << std::hex << status << std::endl;
    }

    std::cout << "Test finished. Press Enter to exit." << std::endl;
    std::cin.get();

    return 0;
}
