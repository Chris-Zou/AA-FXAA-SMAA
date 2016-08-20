#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

class Window
{
	HWND mHWND;
	DirectX::XMINT2 mScreenSize;

public:
	Window(WNDPROC wndProc, HINSTANCE hInstance, DirectX::XMINT2 screenSize, int nCmdShow);
	~Window();
	
	HWND GetHWND();
};

