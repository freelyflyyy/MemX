#include "XCore.h"
#include "../Common/Utils/Utils.h"

namespace MemX {
	XCore::XCore(XContext& context) : _context(context) {}
	XCore::~XCore() {}

	XStatus XCore::GetTargetPeb(PEB32* peb) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XStatus::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		if ( NT_SUCCESS(_context.GetRuntime()->GetTargetPeb(peb)) ) {
			return XStatus::Success();
		}
		return XStatus::Fail(GetSysErrMsg());
	}

	XStatus XCore::GetTargetPeb(PEB64* peb) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XStatus::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		if ( NT_SUCCESS(_context.GetRuntime()->GetTargetPeb(peb)) ) {
			return XStatus::Success();
		}
		return XStatus::Fail(GetSysErrMsg());
	}

	XStatus XCore::GetTargetLdr(PEB_LDR_DATA32* ldrData) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XStatus::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		if ( NT_SUCCESS(_context.GetRuntime()->GetTargetLdr(ldrData)) ) {
			return XStatus::Success();
		}
		return XStatus::Fail(GetSysErrMsg());
	}

	XStatus XCore::GetTargetLdr(PEB_LDR_DATA64* ldrData) const {
		if ( !_context.IsActive() || !_context.GetRuntime() ) {
			return XStatus::Fail(L"Check whether the incoming XContext is correctly attached to the process");
		}
		if ( NT_SUCCESS(_context.GetRuntime()->GetTargetLdr(ldrData)) ) {
			return XStatus::Success();
		}
		return XStatus::Fail(GetSysErrMsg());
	}
}