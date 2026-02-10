#include <iostream>
#include <MemX/Process/XContext.h>
#include <MemX/Process/XMemory.h> 
#include <MemX/Process/XModule.h>

int main() {
    MemX::XContext ctx;
	ctx.Attach(L"PlantsVsZombies.exe", PROCESS_ALL_ACCESS);

	if ( ctx.IsActive() ) {
		std::cout << "成功打开进程" << std::endl;
	}
	MemX::XMemory mem(ctx);
	mem.Write(0x6128F380, 0x9100); 


	MemX::XModule mod(ctx);
	auto mainModule = mod.GetMainModule();
	std::cout << "主模块基址: 0x" << std::hex << mainModule->BaseAddress << std::dec << std::endl;
	return 0;
}