#include <Windows.h>
#include <crtdbg.h>
#include <stdexcept>
#include "Application.h"


int WINAPI wWinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPWSTR _lpCmdLine, int _nCmdShow)
{
	bool run = 0;

	//_CrtSetBreakAlloc(320);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	try
	{
		Application app(_hInstance, _nCmdShow);
		run = app.Run();
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR | MB_OK);
	}

	return run;
}