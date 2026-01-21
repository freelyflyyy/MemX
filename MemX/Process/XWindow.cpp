#include "XWindow.h"

namespace MemX {
	XWindow::XWindow(XContext& context) : _context(context) {}
	XWindow::~XWindow() {}

	std::vector<WindowInfo> XWindow::GetAllWindows() {
		// 如果缓存不为空，直接返回缓存
		if ( !_cache.empty() ) return _cache;
		// 如果未连接或Runtime未初始化，返回空
		if ( !_context.IsActive() || !_context.GetRuntime() ) return {};

		// 1. 定义一个 HWND 列表来接收底层返回的句柄
		std::vector<HWND> handles;
		NTSTATUS nRet = _context.GetRuntime()->GetAllWindow(handles);

		std::vector<WindowInfo> windows;
		if ( STATUS_SUCCESS(nRet) ) {
			// 2. 遍历句柄，逐个获取详细信息
			for ( const auto& hWnd : handles ) {
				WindowInfo info = { 0 };
				// 调用 Runtime 获取详细信息
				if ( STATUS_SUCCESS(_context.GetRuntime()->GetWindowInfo(hWnd, info)) ) {
					windows.push_back(info);
				}
			}
			// 3. 更新缓存
			_cache = windows;
		}
		return windows;
	}

	WindowInfo XWindow::GetMainWindow() {
		auto windows = GetAllWindows();
		WindowInfo candidate = { 0 };
		for ( const auto& win : windows ) {
			if ( win.IsVisible && !win.Title.empty() ) {
				if ( candidate.hWindow == NULL ) {
					candidate = win;
				} else {
					// 简单的启发式算法：优先选择任务栏可见的窗口
					LONG_PTR exStyle = GetWindowLongPtrW(win.hWindow, GWL_EXSTYLE);
					LONG_PTR candExStyle = GetWindowLongPtrW(candidate.hWindow, GWL_EXSTYLE);
					if ( (exStyle & WS_EX_APPWINDOW) && !(candExStyle & WS_EX_APPWINDOW) ) {
						candidate = win;
					}
				}
			}
		}
		return candidate;
	}

	WindowInfo XWindow::GetWindow(HWND hWnd) {
		WindowInfo info = { 0 };
		if ( _context.IsActive() && _context.GetRuntime() ) {
			_context.GetRuntime()->GetWindowInfo(hWnd, info);
		}
		return info;
	}

	WindowInfo XWindow::FindWindowByTitle(const std::wstring& partialTitle) {
		auto windows = GetAllWindows();
		for ( const auto& win : windows ) {
			if ( win.Title.find(partialTitle) != std::wstring::npos ) {
				return win;
			}
		}
		return WindowInfo{ 0 };
	}
}