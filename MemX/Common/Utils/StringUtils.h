#include <string>
#include <algorithm>


namespace MemX {

	static std::wstring getPathName(std::wstring fullPath) {
		if ( fullPath.empty() ) {
			return fullPath;
		}
		auto pos = fullPath.find_last_of(L"\\/");
		if ( pos != std::wstring::npos ) {
			return fullPath.substr(pos + 1);
		}
		return fullPath;
	}

	static std::wstring getPathDir(std::wstring fullPath) {
		if ( fullPath.empty() ) {
			return fullPath;
		}
		auto pos = fullPath.find_last_of(L"\\/");
		if ( pos != std::wstring::npos ) {
			return fullPath.substr(0, pos);
		}
		return fullPath;
	}

	static std::wstring ToLower(std::wstring str) {
		std::transform(str.begin(), str.end(), str.begin(), ::towlower);
		return str;
	}
}