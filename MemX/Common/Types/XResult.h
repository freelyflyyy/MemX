#pragma once

#include "../WinApi/WinHeaders.h"
#include "XStatus.h"
#include <cassert>
#include <optional>

namespace MemX {
	template <typename T>
	class XResult : XStatus {
		private:
		std::optional<T> _data;

		public:
		XResult() : XStatus(XSTATUS_FAILURE, L"Default"), _data(std::nullopt){}
		XResult(const T& data, std::wstring msg = L"")
			: XStatus(true, std::move(msg)), _data(data) {
		}

		XResult(T&& data, std::wstring msg = L"")
			: XStatus(true, std::move(msg)), _data(std::move(data)) {
		}

		XResult(const XStatus& status) : XStatus(status), _data(std::nullopt) {
		}
		static XResult<T> Success(const T& data, std::wstring msg = L"") {
			return XResult<T>(data, std::move(msg));
		}

		static XResult<T> Success(T&& data, std::wstring msg = L"") {
			return XResult<T>(std::move(data), std::move(msg));
		}

		static XResult<T> Fail(std::wstring message) {
			return XStatus::Fail(std::move(message));
		}

		T& Value() { return _data.value(); }
		const T& Value() const { return _data.value(); }

		T ValueOr(const T& defaultValue) const {
			return _data.value_or(defaultValue);
		}

		T* operator->() { return &_data.value(); }
		const T* operator->() const { return &_data.value(); }

		T& operator*() { return _data.value(); }
		const T& operator*() const { return _data.value(); }

		explicit operator bool() const {
			return XStatus::operator bool() && _data.has_value();
		}
	};
}

