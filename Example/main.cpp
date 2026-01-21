#include <iostream>
// 1. 引入必要的头文件 (不再需要 Process.h)
#include <MemX/Process/XContext.h>
#include <MemX/Process/XMemory.h> 
#include <MemX/Common/NtApi/NtResult.h> // 注意路径修正

int main() {
    // 2. [核心] 初始化上下文 (替代 MemX::Process)
    MemX::XContext ctx;
    DWORD targetPid = 0x5E78; // 替换为实际 PID

    // 3. [连接] 使用 Open 替代 Catch
    NTSTATUS status = ctx.Open(targetPid, PROCESS_ALL_ACCESS);

    if ( NT_SUCCESS(status) ) {
        std::wcout << L"Successfully attached to process with PID: " << targetPid << std::endl;

        // 4. [状态] 直接检查 Context (替代 process.Core().isActive())
        if ( ctx.IsActive() ) {

            // 5. [组件] 显式创建 Memory 组件并注入 Context (这是最大的不同点)
            MemX::XMemory memory(ctx);

            // 这里的 PTR_T 是 MemX 定义的 (UINT64)
            MemX::PTR_T memoryAddress = 0x5EBF6DD8;

            // Read a DWORD from memory
            // 调用 memory 组件的方法
            auto readResult = memory.Read<DWORD>(memoryAddress);

            if ( readResult.success() ) {
                std::cout << "Read value at " << std::hex << memoryAddress << ": " << std::dec << readResult.result() << std::endl;
            } else {
                std::wcerr << L"Failed to read memory." << std::endl;
            }

            // Write a new DWORD value to memory
            DWORD newValue = 123456789;
            NTSTATUS writeStatus = memory.Write<DWORD>(memoryAddress, newValue);

            if ( NT_SUCCESS(writeStatus) ) {
                std::cout << "Successfully wrote " << newValue << " to " << std::hex << memoryAddress << std::endl;
            } else {
                std::wcerr << L"Failed to write memory. NTSTATUS: " << std::hex << writeStatus << std::endl;
            }

            // Verify the write by reading again
            readResult = memory.Read<DWORD>(memoryAddress);
            if ( readResult.success() ) {
                std::cout << "Verified new value: " << std::dec << readResult.result() << std::endl;
            }
        } else {
            std::wcerr << L"Process is not active." << std::endl;
        }

        // 6. [清理] 使用 Close 替代 Drop (或者让对象析构自动处理)
        ctx.Close();
    } else {
        std::wcerr << L"Failed to attach to process. NTSTATUS: " << std::hex << status << std::endl;
    }
    return 0;
}