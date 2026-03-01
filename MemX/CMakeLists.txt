cmake_minimum_required (VERSION 3.10)

project ("MemXLibs")

set(CXX_STANDARD 17)

include_directories(..)

############################################################
set(HEADER_INCLUDE "Common/WinApi/WinHeaders.h"
					"Common/WinApi/ArchitectureApi.h"
					"Common/NtApi/NtCallExt.h"
					"Common/NtApi/NtApi.h"
					"Common/Utils/Utils.h"
					"Common/Utils/StringUtils.h"
					"Common/Types/XTypes.h"
					"Common/Types/XStatus.h"
					"Common/Types/XResult.h")
set(SOURCE_INCLUDE "Common/NtApi/NtCallExt.cpp")
file(GLOB Inc ${HEADER_INCLUDE} ${SOURCE_INCLUDE})
source_group(Inc FILES ${Inc})
############################################################
set(SOURCE_PROCESS "Process/XContext.h"
					"Process/XMemory.h"
					"Process/XCore.h"
					"Process/XModule.h"
					"Process/XWindow.h")

set(HEADER_PROCESS "Process/XContext.cpp"
					"Process/XMemory.cpp"
					"Process/XCore.cpp" 
					"Process/XModule.cpp"
					"Process/XWindow.cpp")

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