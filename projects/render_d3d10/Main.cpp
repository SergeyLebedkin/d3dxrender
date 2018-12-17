#include <ctime>
#include <string> 
#include "AppMain.hpp"

// constants
WCHAR szWindowTitle[] = L"Direct3D10 window";
WCHAR szWindowClass[] = L"app_con_d3d10";

// application main class
CAppMain appMain;

// WndProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		appMain.Init(hWnd);
		break;
	}
	case WM_SIZE:
		appMain.SetViewportSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// win main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// register class
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = nullptr;
	RegisterClassExW(&wcex);

	// create window
	HWND hWnd = CreateWindowW(szWindowClass, szWindowTitle, (WS_OVERLAPPEDWINDOW),
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wcex.hInstance, nullptr);

	// show and update window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// main loop
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		double t1 = (double)clock() / CLOCKS_PER_SEC;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			appMain.Update(0.0f);
			appMain.Render();
		}

		double t2 = (double)clock() / CLOCKS_PER_SEC;
		SetWindowTextA(hWnd, std::to_string(1.0 / (t2 - t1)).c_str());
	}

	// destroy window and class
	appMain.Destroy();
	DestroyWindow(hWnd);
	UnregisterClass(wcex.lpszClassName, wcex.hInstance);

	return S_OK;
}