cmake_minimum_required (VERSION 3.10)

project ("MemXLibs")

set(CXX_STANDARD 17)

include_directories(..)

############################################################
set(HEADER_INCLUDE "Common/WinApi/WinHeaders.h"
					"Common/WinApi/ArchitectureApi.h"
					"Common/NtApi/NtCallExt.h"
					"Common/NtApi/NtApi.h"
					"Common/NtApi/NtResult.h"
					"Common/Utils/Utils.h"
					"Common/Utils/Wow64Utils.h"
					"Common/Utils/StringUtils.h"
					"Common/Types/Type.h")
set(SOURCE_INCLUDE "Common/NtApi/NtCallExt.cpp")
file(GLOB Inc ${HEADER_INCLUDE} ${SOURCE_INCLUDE})
source_group(Inc FILES ${Inc})
############################################################
set(SOURCE_PROCESS "Process/Process.h"
					"Process/ProcessMemory.h"
					"Process/ProcessCore.h"
					"Process/ProcessModule.h")

set(HEADER_PROCESS "Process/Process.cpp"
					"Process/ProcessMemory.cpp"
					"Process/ProcessCore.cpp"
					"Process/ProcessModule.cpp")

file(GLOB Process ${SOURCE_PROCESS} ${HEADER_PROCESS})
source_group(Process FILES ${Process})
#############################################################
set(HEADER_RUNTIME "Runtime/Runtime.h"
					"Runtime/X64Runtime.h"
					"Runtime/Wow64Runtime.h")
set(SOURCE_RUNTIME  "Runtime/X64Runtime.cpp"
					"Runtime/Wow64Runtime.cpp")
file(GLOB Runtime ${HEADER_RUNTIME} ${SOURCE_RUNTIME})
source_group(Runtime FILES ${Runtime})
#############################################################
set(SOURCE_LIB ${SOURCE_PROCESS}
				${HEADER_SOURCE}
				${HEADER_RUNTIME}
				${SOURCE_INCLUDE})
set(HEADER_LIB ${HEADER_PROCESS}
				${HEADER_INCLUDE}
				${SOURCE_RUNTIME})

add_library(MemX STATIC ${SOURCE_LIB} ${HEADER_LIB})