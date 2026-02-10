#pragma once
#include "../WinApi/WinHeaders.h"
#include <string>

namespace MemX {
	typedef BOOL Status;
	#define XSTATUS_SUCCESS TRUE
	#define XSTATUS_FAILURE FALSE

	class XStatus {
		private:
		Status _success = FALSE;
		std::wstring _message;

		public:
		XStatus() = default;
		explicit XStatus(Status success, std::wstring message = L"") : _success(success), _message(std::move(message)) {}
		
		static XStatus Success(std::wstring message = L"") {
			return XStatus(XSTATUS_SUCCESS, std::move(message));
		}

		static XStatus Fail(std::wstring message) {
			std::wstring location = std::wstring(__FUNCTIONW__) + L":" + std::to_wstring(__LINE__);
			return XStatus(XSTATUS_FAILURE, L"[" + location + L"] " + message);
		}

		const std::wstring& Message() const { return _message; }

		XStatus& operator=(const XStatus&) = default;
		XStatus& operator=(XStatus&&) = default;
		XStatus(const XStatus&) = default;
		XStatus(XStatus&&) = default;
		operator bool() const { return _success; }

	};
}