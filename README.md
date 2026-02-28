# MemX - Advanced Windows Processes and Memory Library

## Project Introduction
**MemX** is a statically linked library written in modern C++ (C++17), specifically designed for advanced Windows process control, memory read/write operations, and low-level system interactions. Unlike ordinary WinAPI wrapper libraries, MemX dives deep into the operating system's lowest levels by directly invoking undocumented **NTAPI (System Calls)**, providing more powerful and stealthy operational capabilities.

**Core Highlights**:
* **Wow64 Bypass**: Normally, a 32-bit program cannot directly interact with a 64-bit target program. MemX breaks this limitation, allowing 32-bit applications to safely and seamlessly execute native 64-bit system calls, bypassing standard WoW64 subsystem interception.
* **Multi-level Pointer Addressing**: In reverse engineering, data is often hidden behind multiple layers of pointers. This library has built-in pointer chain resolution capabilities. By simply providing an array of offsets, you can complete complex deep dereferencing with a single line of code.
* **Modern C++ Architecture**: Fully embraces C++17. It discards traditional `try-catch` exception handling and cumbersome `BOOL` return values in favor of custom `XResult<T>` and `XStatus`. This makes error handling not only extremely safe but also makes the code highly intuitive to read.
* **Stealthy Module Enumeration (PEB/LDR Parsing)**: Abandons the `CreateToolhelp32Snapshot` function, which is easily monitored by antivirus software or anti-cheat systems. Instead, it directly reads the "Process Environment Block (PEB)" and "Loader Data (Ldr)" in the target process's memory to achieve highly stealthy module list retrieval.

---

## Build and Installation Requirements

To compile and use this library in your project, you must meet the following environmental requirements:

* **Build Tool**: CMake (Minimum version 3.10 required)
* **Compiler**: Must support the **C++17** standard (Highly recommend using MSVC / Visual Studio 2019 or later)
* **Operating System**: Windows 10 or higher (The code includes specific optimizations for Windows 10 pseudo-handle privilege escalation)

---

## Detailed Usage Guide (Step-by-Step Tutorial)

Below, we will teach you how to use MemX step by step. Each step comes with detailed explanations to ensure you not only know how to do it but also understand why.

### Step 1: Attach to the Target Process (`XContext`)
The prerequisite for any operation is establishing a connection with the target process. `XContext` is the "brain" and "passport" of the entire library. You need to create a context object first and then "attach" it to the program you want to manipulate.

```cpp
#include "MemX/Process/XContext.h"
#include <iostream>

using namespace MemX;

int main() {
    // 1. Create the context object. It will manage the process handle and architecture information.
    XContext context;

    // 2. Attach to the target process.
    // Here we attach using the process name (e.g., TargetProcess.exe).
    // PROCESS_ALL_ACCESS means we request the highest control privileges for this process from the system.
    XStatus status = context.Attach(L"TargetProcess.exe", PROCESS_ALL_ACCESS);
    
    // 3. Check if the attachment was successful.
    // XStatus overloads the bool operator, so you can use 'if' directly for evaluation.
    if (!status) {
        // If it fails, status.Message() will tell you the specific system error reason.
        std::wcout << L"Failed to attach. Reason: " << status.Message() << std::endl;
        return -1;
    }

    // Once successfully attached, you can retrieve detailed information about the target process.
    std::wcout << L"Successfully attached! Target PID: " << context.GetPid() << std::endl;
    std::wcout << L"Is the target process 32-bit (Wow64)? " << (context.IsWow64() ? L"Yes" : L"No") << std::endl;

    return 0;
}
```

### Step 2: Read and Write Memory Safely (XMemory)
After attaching to the process, you use the XMemory class to read or modify data. MemX uses XResult<T> to wrap both the execution status and the actual data. This guarantees that your program will never crash due to dereferencing invalid memory.

```cpp
#include "MemX/Process/XMemory.h"

// 1. Initialize the memory manager using the active context created in Step 1.
XMemory memory(context);

// 2. Define a target base address (example address).
PTR_T targetAddress = 0x7FF0000000;

// --- Scenario A: Reading a standard variable ---
auto readResult = memory.Read<int>(targetAddress);
if (readResult) {
    // Use .Value() to safely extract the data if the read operation succeeded.
    std::cout << "Value read: " << readResult.Value() << std::endl;
} else {
    std::wcout << L"Read error: " << readResult.Message() << std::endl;
}

// --- Scenario B: Resolving a Pointer Chain ---
// In reverse engineering, data is often deeply nested like: [[[Base] + 0x10] + 0x20]
// MemX resolves this automatically. Just pass a vector of the base address and offsets.
std::vector<PTR_T> offsetChain = { targetAddress, 0x10, 0x20 };
auto chainResult = memory.Read<float>(offsetChain);
if (chainResult) {
    std::cout << "Float value from pointer chain: " << chainResult.Value() << std::endl;
}

// --- Scenario C: Writing Data ---
int newHealth = 9999;
// Overwrite the memory at targetAddress with the new value.
XStatus writeStatus = memory.Write(targetAddress, newHealth);
if (writeStatus) {
    std::cout << "Data successfully written!" << std::endl;
}
```

### Step 3: Stealthy Module Enumeration (XModule)
Antivirus and anti-cheat systems heavily monitor standard Windows APIs like EnumProcessModules. XModule bypasses these hooks entirely by manually reading the target's internal Process Environment Block (PEB) to locate where DLLs are loaded.

```cpp
#include "MemX/Process/XModule.h"

// 1. Initialize the module manager with your active context.
XModule moduleManager(context);

// 2. Fetch the Main Module (the executable file itself).
auto mainModule = moduleManager.GetMain();
if (mainModule) {
    std::wcout << L"Main Executable Name: " << mainModule.Value()->FullName << std::endl;
    std::wcout << L"Main Base Address: 0x" << std::hex << mainModule.Value()->BaseAddress << std::endl;
}

// 3. Search for a specific DLL (e.g., ntdll.dll). 
// This search is case-insensitive and utilizes internal caching to make future lookups instant.
auto ntdllModule = moduleManager.GetModule(L"ntdll.dll");
if (ntdllModule) {
    std::wcout << L"NTDLL Base Address: 0x" << std::hex << ntdllModule.Value()->BaseAddress << std::endl;
}
```

### Step 4: Window Handling and Coordinate Retrieval (XWindow)
When you are building an external overlay (a transparent window drawn over a game or application), you need exact screen coordinates. XWindow locates windows exclusively owned by your target process and calculates their precise dimensions.

```cpp
#include "MemX/Process/XWindow.h"

// 1. Initialize the window manager.
XWindow winManager(context);

// 2. Automatically find the main visible window of the target process.
WindowInfo mainWin = winManager.GetMainWindow();

if (mainWin.IsValid()) {
    std::wcout << L"Target Window Title: " << mainWin.Title << std::endl;
    std::cout << "Total Window Size: " << mainWin.Width << "x" << mainWin.Height << std::endl;
    
    // The 'ClientRect' holds the coordinates of the inner working area (excluding title bars and borders).
    // MemX automatically maps this client area to absolute screen coordinates.
    std::cout << "Client Area Top-Left (X, Y): " 
              << mainWin.ClientRect.left << ", " << mainWin.ClientRect.top << std::endl;
}
```