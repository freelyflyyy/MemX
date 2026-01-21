#include <windows.h>
#pragma warning(disable: 28251)


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_ERASEBKGND:

		return 0;
	case WM_PAINT:

		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"MainWindowClass";
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	if ( !RegisterClassEx(&wc) ) {
		MessageBox(NULL, L"窗口注册失败！", L"错误", MB_ICONERROR);
		return 0;
	}
	HWND hwnd = CreateWindowEx(
		0,                       
		L"MainWindowClass",       
		L"这是我的第一个窗口",    
		WS_OVERLAPPEDWINDOW,    
		CW_USEDEFAULT, CW_USEDEFAULT,  
		800, 600,     
		NULL,        
		NULL,                     
		hInstance,         
		NULL  
	);

	if ( hwnd == NULL ) {
		MessageBox(NULL, L"窗口创建失败！", L"错误", MB_ICONERROR);
		return 0;
	}
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	MSG msg;
	while ( GetMessage(&msg, NULL, 0, 0) ) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int) msg.wParam;
}