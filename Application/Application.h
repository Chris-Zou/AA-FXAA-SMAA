#pragma once
#include <Windows.h>
#include <vector>
#include "Window.h"
#include "DirectXClass.h"
#include "Camera.h"
#include "Box.h"
#include "RenderModule.h"
#include "ObjObject.h"
#include "Timer.h"
#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <string>
#include "CPUTGPUTimerDX11.h"

#define SAMPLERATE 2000

__declspec(align(16))class Application
{
private:
	DirectX::XMINT2 _screenSize;
	Window* _window;
	DirectXClass* _directX;
	RenderModule* _renderModule;
	Camera* _camera;
	std::vector<Box*> _box;
	ObjObject* _object;
	//Timer
	Timer _timer;

	//
	POINT _previousMousePos;
	int _currentAliasingMethod;
	
	bool _test;
	int _testState;
	double _noaaTestResults[5][2];
	double _fxaaTestResults[5][2];
	double _smaaTestResults[5][2];

	CPUTGPUTimerDX11 _profiling;
	double _tempAvgCT;
	double _avgCT;
	double _avgFPS;
	double _prevAvg;
	int increment;
	bool sceneChange;
	void Sampling();
	void ResetSampling();

private:
	enum TestingState
	{
		NOTEST,
		NOAA,
		NOAARUN,
		FXAA,
		FXAARUN,
		SMAA,
		SMAARUN,
		TESTDONE
	};
	enum Scene
	{
		A,
		B,
		C,
		D,
		E
	};
	Scene _currentScene;
	void WriteToFile();
	void ChangeScene(Scene scene);
	void ClearResults();

private:
	bool HandleInput(float deltaTime, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	Application(HINSTANCE hInstance, int nCmdShow);
	~Application();

	bool Update(float deltaTime);
	void Render();
	bool Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	//Overloading these guarantees 16B alignment of XMMATRIX
	void* operator new(size_t i);
	void operator delete(void* p);
};

static Application* wndMessages;