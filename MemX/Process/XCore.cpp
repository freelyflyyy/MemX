#include "XCore.h"

namespace MemX {
	XCore::XCore(XContext& context) : _context(context) {}
	XCore::~XCore() {}

	DWORD XCore::GetTargetPeb(PEB32* peb) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) return 0;
		return _context.GetRuntime()->GetTargetPeb(peb);
	}

	DWORD64 XCore::GetTargetPeb(PEB64* peb) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) return 0;
		return _context.GetRuntime()->GetTargetPeb(peb);
	}

	NTSTATUS XCore::GetTargetLdr(PEB_LDR_DATA32* ldrData) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) return STATUS_UNSUCCESSFUL;
		return _context.GetRuntime()->GetTargetLdr(ldrData);
	}

	NTSTATUS XCore::GetTargetLdr(PEB_LDR_DATA64* ldrData) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) return STATUS_UNSUCCESSFUL;
		return _context.GetRuntime()->GetTargetLdr(ldrData);
	}
}