#pragma once
#include <vector>
#include <string>
#include "../Common/Types/Type.h"

namespace MemX {
	class ProcessWindow {
		public:
		ProcessWindow(class Process& process);
		~ProcessWindow();

		std::vector<WindowInfo> GetAllWindows();

		WindowInfo GetMainWindow();

		WindowInfo GetWindowInfo(HWND hWnd);

		WindowInfo FindWindowByTitle(const std::wstring& partialTitle);

		private:
		ProcessWindow(const ProcessWindow&) = delete;
		ProcessWindow& operator=(const ProcessWindow&) = delete;
		
		private:
		class Process& _process;
		class ProcessCore& _core;

		class ProcessMemory& _memory;
		class ProcessModule& _module;
	};
}