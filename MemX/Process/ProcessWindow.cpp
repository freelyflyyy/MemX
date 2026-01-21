#include "ProcessWindow.h"
#include "Process.h"
#include "ProcessCore.h"

namespace MemX {
	ProcessWindow::ProcessWindow(Process& process)
		:_process(process),
		_core(process.Core()),
		_memory(process.Memory()),
		_module(process.Module()) {
	}

	ProcessWindow::~ProcessWindow() {
	}

	std::vector<WindowInfo> ProcessWindow::GetAllWindows() {
		std::vector<WindowInfo> windows;
		_core.getRuntime()->GetProcessWindows(windows);
		return windows;
	}

	WindowInfo ProcessWindow::GetMainWindow() {
		auto windows = GetAllWindows();
		WindowInfo candidate = { 0 };

		for ( const auto& win : windows ) {
			if ( win.IsVisible && !win.Title.empty() ) {
				if ( candidate.hWindow == NULL ) {
					candidate = win;
				} else {
					LONG_PTR exStyle = GetWindowLongPtrW(win.hWindow, GWL_EXSTYLE);
					LONG_PTR candExStyle = GetWindowLongPtrW(candidate.hWindow, GWL_EXSTYLE);

					// 优先选择任务栏窗口 (WS_EX_APPWINDOW)
					if ( (exStyle & WS_EX_APPWINDOW) && !(candExStyle & WS_EX_APPWINDOW) ) {
						candidate = win;
					}
				}
			}
		}
		return candidate;
	}

	WindowInfo ProcessWindow::GetWindowInfo(HWND hWnd) {
		WindowInfo info = { 0 };
		// 直接调用统一的 Runtime 抽象层
		_core.getRuntime()->GetWindowDetail(hWnd, info);
		return info;
	}

	WindowInfo ProcessWindow::FindWindowByTitle(const std::wstring& partialTitle) {
		auto windows = GetAllWindows();
		for ( const auto& win : windows ) {
			if ( win.Title.find(partialTitle) != std::wstring::npos ) {
				return win;
			}
		}
		return WindowInfo{ 0 };
	}
}