#include <string>
#include <algorithm>


namespace MemX {
	inline std::wstring getPathName(std::wstring fullPath) {
		if ( fullPath.empty() ) {
			return fullPath;
		}
		auto pos = fullPath.find_last_of(L"\\/");
		if ( pos != std::wstring::npos ) {
			return fullPath.substr(pos + 1);
		}
		return fullPath;
	}

	inline std::wstring getPathDir(std::wstring fullPath) {
		if ( fullPath.empty() ) {
			return fullPath;
		}
		auto pos = fullPath.find_last_of(L"\\/");
		if ( pos != std::wstring::npos ) {
			return fullPath.substr(0, pos);
		}
		return fullPath;
	}
	inline std::wstring ToLower(std::wstring str) {
		std::transform(str.begin(), str.end(), str.begin(), ::towlower);
		return str;
	}
}