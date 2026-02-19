#include <iostream>
#include <MemX/Process/XContext.h>
#include <MemX/Process/XMemory.h> 
#include <MemX/Process/XModule.h>

int main() {
    MemX::XContext ctx;
	MemX::XStatus xs = ctx.Attach(L"PlantsVsZombies.exe", PROCESS_ALL_ACCESS);
	if ( !xs.Status() ) {
		std::wcout << xs.Message() << std::endl;
		return -1;
	}
	MemX::XModule mod(ctx);
	MemX::XModulePtr modPtr = mod.GetModule(L"ntdll.dll", MemX::ModuleArch::x64);
	if ( !modPtr ) {
		std::wcout << modPtr.Message() << std::endl;
		return -1;
	}
	std::wcout << L"Module Base: 0x" << std::hex << modPtr.Value()->BaseAddress << L"\nModule Size: " << std::dec << modPtr.Value()->Size << L"\nModule Name: " << modPtr.Value()->FullName << std::endl;
	return 0;
}