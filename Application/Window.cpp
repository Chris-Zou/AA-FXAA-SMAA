#include "Window.h"
#include <stdexcept>

Window::Window(WNDPROC wndProc, HINSTANCE hInstance, DirectX::XMINT2 screenSize, int nCmdShow)
{
	mScreenSize = screenSize;

	//Window info
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(NULL, IDC_HAND);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"Application";
	wc.hIconSm = 0;

	if (!RegisterClassEx(&wc))
		throw std::runtime_error("Register window class");

	//Adjust and create the window
	RECT rc = { 0, 0, (long)mScreenSize.x, (long)mScreenSize.y };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);


	mHWND = CreateWindow(L"Application",
		L"Application",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!(mHWND))
		throw std::runtime_error("Create Window");

	ShowWindow(mHWND, nCmdShow);
	ShowCursor(TRUE);
}

Window::~Window() {}

HWND Window::GetHWND()
{
	return mHWND;
}