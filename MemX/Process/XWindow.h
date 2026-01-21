#pragma once
#include "XContext.h"
#include "../Common/Types/Type.h"
#include <vector>
#include <string>

namespace MemX {
	class XWindow {
		public:
		explicit XWindow(XContext& context);
		~XWindow();

		std::vector<WindowInfo> GetAllWindows();
		WindowInfo GetMainWindow();
		WindowInfo GetWindow(HWND hWnd);
		WindowInfo FindWindowByTitle(const std::wstring& partialTitle);
		std::vector<WindowInfo> GetWindowCache() { return _cache; }

		private:
		XContext& _context;
		std::vector<WindowInfo> _cache;
	};
}