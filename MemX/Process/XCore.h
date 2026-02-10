#pragma once
#include "XContext.h"
#include "../Common/WinApi/WinHeaders.h"

namespace MemX {
	class XCore {
		public:
		explicit XCore(XContext& context);
		~XCore();

		XStatus GetTargetPeb(PEB32* peb) const;
		XStatus GetTargetPeb(PEB64* peb) const;

		XStatus GetTargetLdr(PEB_LDR_DATA32* ldrData) const;
		XStatus GetTargetLdr(PEB_LDR_DATA64* ldrData) const;

		private:
		XContext& _context;
	};
}