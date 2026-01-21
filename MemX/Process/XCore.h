#pragma once
#include "XContext.h"
#include "../Common/WinApi/WinHeaders.h"

namespace MemX {
	class XCore {
		public:
		explicit XCore(XContext& context);
		~XCore();

		DWORD GetTargetPeb(PEB32* peb) const;
		DWORD64 GetTargetPeb(PEB64* peb) const;

		NTSTATUS GetTargetLdr(PEB_LDR_DATA32* ldrData) const;
		NTSTATUS GetTargetLdr(PEB_LDR_DATA64* ldrData) const;

		private:
		XContext& _context;
	};
}