#include <string>
#include <chrono>
#include "AppMain.hpp"

// constants
WCHAR szWindowTitle[] = L"Direct3D11 window";
WCHAR szWindowClass[] = L"app_con_d3d11";

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

	// get time stamps
	auto prevTimePoint = std::chrono::high_resolution_clock::now();
	auto nextTimePoint = std::chrono::high_resolution_clock::now();

	// main loop
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		// get current time stamps
		prevTimePoint = nextTimePoint;
		nextTimePoint = std::chrono::high_resolution_clock::now();
		float delta = std::chrono::duration_cast<std::chrono::duration<float>>(nextTimePoint - prevTimePoint).count();

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			appMain.Update(delta);
			appMain.Render();
		}

		SetWindowTextA(hWnd, std::to_string(1.0f / delta).c_str());
	}

	// destroy window and class
	appMain.Destroy();
	DestroyWindow(hWnd);
	UnregisterClass(wcex.lpszClassName, wcex.hInstance);

	return S_OK;
}
