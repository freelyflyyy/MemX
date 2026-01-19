#include <iostream>
#include <MemX/Process/Process.h>
#include <MemX/Common/NtApi/NtApi.h>
#include <MemX/Common/NtApi/NtCallExt.h>

using namespace MemX;

int main()
{
	Process process;
	process.Catch(L"PlantsVsZombies.exe"); // Replace 1234 with a valid PID
	MemX::NtResult<DWORD> readResult = process.Memory().Read<DWORD>(0x5F5625F8);
	if ( readResult.success() ) {
		std::cout << "Value at address 0x5EAE3430: " << std::dec << readResult.result() << std::dec << std::endl;
		process.Memory().Write(0x5F5625F8, 9999);
	} else {
		std::cout << "Failed to read memory. NTSTATUS: " << std::hex
			<< readResult.success() << std::dec << std::endl;
	}

	PEB32 peb32 = { 0 };
	process.Core().getTargetPeb(&peb32);
	std::cout << "PEB32 Ldr: 0x" << std::hex << peb32.Ldr << std::endl;
	DWORD imageBase = process.Memory().Read<DWORD>(peb32.ImageBaseAddress).result(imageBase);
	std::cout << "Image Base Address: 0x" << std::hex << imageBase << std::dec << std::endl;


	PEB64 peb64 = { 0 };
	process.Core().getTargetPeb(&peb64);
	std::cout << "PEB64 Ldr: 0x" << std::hex << peb64.Ldr << std::dec << std::endl;

	ModuleInfoPtr mainModule = process.Module().GetMainModule();
	std::cout << "Image Base Address: 0x" << std::hex << mainModule->baseAddr << std::dec << std::endl;

	std::vector<ModuleInfoPtr> allModules;
	process.Core().getRuntime()->GetAllModulesByLdrList32(&allModules);

	for ( size_t i = 0; i < allModules.size(); i++ ) {
		std::wcout << allModules[ i ]->fullName << " "
				<< allModules[i]->isX86 << " " << std::endl;
	}
	return 0;
}
