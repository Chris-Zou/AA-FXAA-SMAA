#include "Application.h"
#include <stdexcept>

Application::Application(HINSTANCE hInstance, int nCmdShow)
{
	//Default values
	wndMessages = this;
	_screenSize = DirectX::XMINT2(1280, 720);
	_window = new Window(WndProc, hInstance, _screenSize, nCmdShow);
	_directX = new DirectXClass(_screenSize, _window->GetHWND());

	_renderModule = new RenderModule(_directX, _screenSize);
	_camera = new Camera(0.1f, 1000.0f, DirectX::XM_PIDIV2, _screenSize);
	_box.push_back(new Box(_directX->GetDevice(), L"Assets/Textures/bright-tiles.png", DirectX::XMFLOAT3(0,-30, 55), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(60, 60, 10)));
	_box.push_back(new Box(_directX->GetDevice(), L"Assets/Textures/cube.png", DirectX::XMFLOAT3(0, -7.5f, 11), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(2, 2, 2)));
	
	_object = new ObjObject(_directX->GetDevice(), _directX->GetDeviceContext());
	_object->LoadFile("bth.obj");
	_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));

	_previousMousePos.x = -1;
	_previousMousePos.y = -1;

	_currentAliasingMethod = AntiAliasingMethod::NOAA;
	ClearResults();
	_test = false;
	_testState = TestingState::NOAA;

	_profiling.OnDeviceAndContextCreated(_directX->GetDevice(), _directX->GetDeviceContext());
	_avgCT = 0;
	_tempAvgCT = 0;
	increment = 0;
}

Application::~Application()
{
	delete _window;
	delete _directX;
	delete _renderModule;
	delete _camera;
	delete _object;
	_profiling.OnDeviceAboutToBeDestroyed();
	for (auto i : _box)
	{
		delete i;
	}
}

bool Application::Update(float deltaTime)
{
	bool run = true;
	_camera->Update();
	_renderModule->Update(_camera);

	return run;
}

void Application::Render()
{
	_profiling.OnFrameStart();

	

	//Clear Render and Depth buffer
	if (_currentAliasingMethod == AntiAliasingMethod::NOAA)
	{
		//Set deferred RTV to be render target.
		_directX->SetDeferredRenderTargets();
	}
	else if (_currentAliasingMethod == AntiAliasingMethod::SMAA)
	{
		//Set deferred RTV to be render target.
		_directX->SetDeferredRenderTargets();
	}
	else if (_currentAliasingMethod == AntiAliasingMethod::FXAA)
	{
		//Set deferred RTV to be render target.
		_directX->SetFXAARenderTargets();
		
	}
	//Clear them
	_directX->ClearDeferredRenderTargets(DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	
	//
	ID3D11DeviceContext* deviceContext = _directX->GetDeviceContext();
	ID3D11SamplerState* samplerState = _directX->GetSamplerState();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////// Render Scene //////////////////////////////////////////////
	_object->Render();
	_renderModule->RenderVertex(_camera, _object->GetTexture(), samplerState, _object->GetModelSpace(), _object->GetNumVertex());

	//outer box
	//_directX->GetDeviceContext()->RSSetState(_directX->GetRasterState(CullMode::Front));
	_box[0]->Render(_directX->GetDeviceContext());
	_renderModule->RenderIndexed(_camera, _box[0]->GetTexture(), samplerState, _box[0]->GetLocalSpace(), _box[0]->GetNumIndex());
	
	
	//inner box
	_directX->GetDeviceContext()->RSSetState(_directX->GetRasterState(CullMode::None));
	_box[1]->Render(_directX->GetDeviceContext());
	_renderModule->RenderIndexed(_camera, _box[1]->GetTexture(), samplerState, _box[1]->GetLocalSpace(), _box[1]->GetNumIndex());

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////// Render the deferred image //////////////////////////////////////////////
	_directX->GetDeviceContext()->RSSetState(_directX->GetRasterState(CullMode::Back));
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////// Render light //////////////////////////////////////////////
	_renderModule->RenderLight(_camera, _currentAliasingMethod);
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////// Anti-aliasing //////////////////////////////////////////////
	/*
	//Set Render target back to back buffer and clear it
	_directX->SetBackBufferAsRenderTarget();
	
	*/
	_directX->ClearBackBuffer(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
	
	if (_test)
	{
		switch (_testState)
		{
		case TestingState::NOAA:
		{
			_currentAliasingMethod = AntiAliasingMethod::NOAA;
			ResetSampling();

			_currentScene = Scene::A;
			ChangeScene(Scene::A);
			_testState = NOAARUN;
			break;
		}
		case TestingState::NOAARUN:
		{	
			double adt = _avgFPS;
			double act = _avgCT * 1000;
			if (sceneChange)
			{
				switch (_currentScene)
				{
				case Application::A:
				{
					_noaaTestResults[_currentScene][0] = adt;
					_noaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::B);
					ResetSampling();
					break;
				}
				case Application::B:
				{
					_noaaTestResults[_currentScene][0] = adt;
					_noaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::C);
					ResetSampling();
					break;
				}
				case Application::C:
				{
					_noaaTestResults[_currentScene][0] = adt;
					_noaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::D);
					ResetSampling();
					break;
				}
				case Application::D:
				{
					_noaaTestResults[_currentScene][0] = adt;
					_noaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::E);
					ResetSampling();
					break;
				}
				case Application::E:
				{
					_noaaTestResults[_currentScene][0] = adt;
					_noaaTestResults[_currentScene][1] = act;
					_testState = FXAA;
					break;
				}
				}
			}
			_profiling.Start();
			_renderModule->RenderAntiAliasing(_currentAliasingMethod);
			_profiling.Stop();
			break;
		}
		case TestingState::FXAA:
		{
			_currentAliasingMethod = AntiAliasingMethod::FXAA;
			ResetSampling();
			_currentScene = Scene::A;
			ChangeScene(Scene::A);
			_testState = FXAARUN;
			break;
		}
		case TestingState::FXAARUN:
		{
			double adt = _avgFPS;
			double act = _avgCT * 1000;
			if (sceneChange)
			{
				switch (_currentScene)
				{
				case Application::A:
				{
					_fxaaTestResults[_currentScene][0] = adt;
					_fxaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::B);
					ResetSampling();
					break;
				}
				case Application::B:
				{
					_fxaaTestResults[_currentScene][0] = adt;
					_fxaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::C);
					ResetSampling();
					break;
				}
				case Application::C:
				{
					_fxaaTestResults[_currentScene][0] = adt;
					_fxaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::D);
					ResetSampling();
					break;
				}
				case Application::D:
				{
					_fxaaTestResults[_currentScene][0] = adt;
					_fxaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::E);
					ResetSampling();
					break;
				}
				case Application::E:
				{
					_fxaaTestResults[_currentScene][0] = adt;
					_fxaaTestResults[_currentScene][1] = act;
					_testState = SMAA;
					break;
				}
				}
			}
			_profiling.Start();
			_renderModule->RenderAntiAliasing(_currentAliasingMethod);
			_profiling.Stop();
			break;
		}
		case TestingState::SMAA:
		{
			_currentAliasingMethod = AntiAliasingMethod::SMAA;
			ResetSampling();
			_currentScene = Scene::A;
			ChangeScene(Scene::A);
			_testState = SMAARUN;
			break;
		}
		case TestingState::SMAARUN:
		{
			double adt = _avgFPS;
			double act = _avgCT * 1000;
			if (sceneChange)
			{
				switch (_currentScene)
				{
				case Application::A:
				{
					_smaaTestResults[_currentScene][0] = adt;
					_smaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::B);
					ResetSampling();
					break;
				}
				case Application::B:
				{
					_smaaTestResults[_currentScene][0] = adt;
					_smaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::C);
					ResetSampling();
					break;
				}
				case Application::C:
				{
					_smaaTestResults[_currentScene][0] = adt;
					_smaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::D);
					ResetSampling();
					break;
				}
				case Application::D:
				{
					_smaaTestResults[_currentScene][0] = adt;
					_smaaTestResults[_currentScene][1] = act;
					ChangeScene(Scene::E);
					ResetSampling();
					break;
				}
				case Application::E:
				{
					_smaaTestResults[_currentScene][0] = adt;
					_smaaTestResults[_currentScene][1] = act;
					_testState = TESTDONE;
					ResetSampling();
					break;
				}
				}
			}
			_profiling.Start();
			_renderModule->RenderAntiAliasing(_currentAliasingMethod);
			_profiling.Stop();
			break;
		}
		case TESTDONE:
		{
			WriteToFile();
			_test = false;
			break;
		}
		}
	}
	else
	{
		_profiling.Start();
		_renderModule->RenderAntiAliasing(_currentAliasingMethod);
		_profiling.Stop();
	}
	

	_directX->SwapChain();
	_profiling.OnFrameEnd();

	Sampling();
}

bool Application::HandleInput(float deltaTime, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool run = true;
	DirectX::XMFLOAT3 forward(0, 0, 0);
	DirectX::XMFLOAT3 right(0, 0, 0);
	DirectX::XMFLOAT3 position = _camera->GetPosition();
	float speed = 0.01f *deltaTime;

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		return false;

	if (GetAsyncKeyState(VK_F1) & 0x8000 && _test == TestingState::NOTEST)
	{
		_test = true;
		_testState = NOAA;
		_currentAliasingMethod = AntiAliasingMethod::NOAA;
		ClearResults();
	}
	if (!_test)
	{
		if (GetAsyncKeyState('Z') & 0x8000)
		{
			_currentAliasingMethod = AntiAliasingMethod::NOAA;
		}
		if (GetAsyncKeyState('X') & 0x8000)
		{
			_currentAliasingMethod = AntiAliasingMethod::FXAA;
		}
		if (GetAsyncKeyState('C') & 0x8000)//GetAsyncKeyState(0x33) & 0x8000)
		{
			_currentAliasingMethod = AntiAliasingMethod::SMAA;
		}


		//Movement
		if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
		{
			speed *= 3;
		}

		//Rotate BTH
		if (GetAsyncKeyState('Q') & 0x8000)
		{
			_object->Rotate(XMFLOAT3(0, -0.0001f*deltaTime, 0));
		}

		if (GetAsyncKeyState('E') & 0x8000)
		{
			_object->Rotate(XMFLOAT3(0, 0.0001f*deltaTime, 0));
		}

		//Movement
		if (GetAsyncKeyState('W') & 0x8000)
		{
			forward = _camera->GetForwardVector();
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			forward = _camera->GetForwardVector();
			forward.x *= -1;
			forward.y *= -1;
			forward.z *= -1;

		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			right = _camera->GetRightVector();

		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			right = _camera->GetRightVector();
			right.x *= -1;
			right.y *= -1;
			right.z *= -1;
		}
	}

	XMFLOAT3 pos = DirectX::XMFLOAT3(position.x + (forward.x + right.x) * speed, position.y + (forward.y + right.y) * speed, position.z + (forward.z + right.z) * speed);
	_camera->SetPosition(pos);

	if (!_test)
	{
		//Camera Positions
		if (GetAsyncKeyState('T') & 0x8000)
		{
			_camera->SetPosition(XMFLOAT3(0, 0, 0));
			_camera->SetRotation(XMFLOAT3(0, 0, 0));
			_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		}
		if (GetAsyncKeyState('Y') & 0x8000)
		{
			_camera->SetPosition(XMFLOAT3(-2.66877937f, 8.62251472f, 11.5003452f));
			_camera->SetRotation(XMFLOAT3(7, 2.5, 0));
			_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		}
		if (GetAsyncKeyState('U') & 0x8000)
		{
			_camera->SetPosition(XMFLOAT3(-17.0385838f, 13.2552023f, 13.0742922f));
			_camera->SetRotation(XMFLOAT3(-16.5, -6.5, 0));
			_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		}
		if (GetAsyncKeyState('I') & 0x8000)
		{
			_camera->SetPosition(XMFLOAT3(3.40040994f, -2.83563733f, 1.11875713f));
			_camera->SetRotation(XMFLOAT3(7, 2, 0));
			_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0.75f, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		}
		if (GetAsyncKeyState('O') & 0x8000)
		{
			_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 1.0f, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		}
		if (GetAsyncKeyState('P') & 0x8000)
		{
			XMFLOAT3 POS = _camera->GetPosition();
			XMFLOAT3 ROT = _camera->GetRotation();

			//_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 1.25f, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		}

		//Look with Camera
		if (msg == WM_MOUSEMOVE)
		{
			UINT x = GET_X_LPARAM(lParam);
			UINT y = GET_Y_LPARAM(lParam);
			if (_previousMousePos.x == -1)
			{
				_previousMousePos.x = x;
				_previousMousePos.y = y;
			}
			if ((wParam & MK_LBUTTON) != 0)
			{
				DirectX::XMFLOAT3 rotation = _camera->GetRotation();
				float dx = (float)x - (float)_previousMousePos.x;
				float dy = (float)y - (float)_previousMousePos.y;

				rotation.x += dy * 0.5f;
				rotation.y += dx * 0.5f;
				_camera->SetRotation(rotation);
			}

			_previousMousePos.x = x;
			_previousMousePos.y = y;
		}
	}
	
	return run;
}

void Application::ClearResults()
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			_noaaTestResults[i][j] = 0;
			_fxaaTestResults[i][j] = 0;
			_smaaTestResults[i][j] = 0;
		}
	}
}

void Application::WriteToFile()
{
	std::ofstream myfile;
	myfile.open("testResults.txt");
	std::string scen;

	myfile << "NO AA: \n";
	for (int i = 0; i < 5; i++)
	{
		if (i == Scene::A)
			scen = "A: ";
		else if (i == Scene::B)
			scen = "B: ";
		else if (i == Scene::C)
			scen = "C: ";
		else if (i == Scene::D)
			scen = "D: ";
		else if (i == Scene::E)
			scen = "E: ";
		for (int j = 0; j < 2; j++)
		{
			if (j == 0)
			{
				myfile << scen << "DT: "<< _noaaTestResults[i][j];
			}
			else
			{
				myfile << scen << "ClockT: " << _noaaTestResults[i][j];
			}
		}
		myfile << "\n";
	}

	myfile << "FXAA: \n";
	for (int i = 0; i < 5; i++)
	{
		if (i == Scene::A)
			scen = "A: ";
		else if (i == Scene::B)
			scen = "B: ";
		else if (i == Scene::C)
			scen = "C: ";
		else if (i == Scene::D)
			scen = "D: ";
		else if (i == Scene::E)
			scen = "E: ";
		for (int j = 0; j < 2; j++)
		{
			if (j == 0)
			{
				myfile << scen << "DT: " << _fxaaTestResults[i][j];
			}
			else
			{
				myfile << scen << "ClockT: " << _fxaaTestResults[i][j];
			}
		}
		myfile << "\n";
	}

	myfile << "SMAA: \n";
	for (int i = 0; i < 5; i++)
	{
		if (i == Scene::A)
			scen = "A: ";
		else if (i == Scene::B)
			scen = "B: ";
		else if (i == Scene::C)
			scen = "C: ";
		else if (i == Scene::D)
			scen = "D: ";
		else if (i == Scene::E)
			scen = "E: ";
		for (int j = 0; j < 2; j++)
		{
			if (j == 0)
			{
				myfile << scen << "DT: " << _smaaTestResults[i][j];
			}
			else
			{
				myfile << scen << "ClockT: " << _smaaTestResults[i][j];
			}
		}
		myfile << "\n";
	}
}

void Application::ChangeScene(Scene scene)
{
	_currentScene = scene;
	switch (scene)
	{
	case Application::A:
	{
		_camera->SetPosition(XMFLOAT3(0, 0, 0));
		_camera->SetRotation(XMFLOAT3(0, 0, 0));
		_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		break;
	}
	case Application::B:
	{
		_camera->SetPosition(XMFLOAT3(-2.66877937f, 8.62251472f, 11.5003452f));
		_camera->SetRotation(XMFLOAT3(7, 2.5, 0));
		_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		break;
	}
	case Application::C:
	{
		_camera->SetPosition(XMFLOAT3(-17.0385838f, 13.2552023f, 13.0742922f));
		_camera->SetRotation(XMFLOAT3(-16.5, -6.5, 0));
		_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		break;
	}
	case Application::D:
	{
		_camera->SetPosition(XMFLOAT3(3.40040994f, -2.83563733f, 1.11875713f));
		_camera->SetRotation(XMFLOAT3(7, 2, 0));
		_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 0.75f, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		break;
	}
	case Application::E:
	{
		_object->SetObject(DirectX::XMFLOAT3(0, -5, 15), DirectX::XMFLOAT3(0, 1.0f, 0), DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
		break;
	}
	default:
		break;
	}
}

bool Application::Run()
{
	bool run = true;

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (run)
	{
		//Messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				run = false;
			}
		}
		else
		{
			_timer.Update();
			run = HandleInput((float)_timer.GetDeltaTime(), msg.message, msg.wParam, msg.lParam);
			
			if (run)
			{
				Update((float)_timer.GetDeltaTime());
				Render();

				std::wstring aaMethod;
				if (_currentAliasingMethod == AntiAliasingMethod::NOAA)
					aaMethod = L" NO AA";
				else if (_currentAliasingMethod == AntiAliasingMethod::FXAA)
					aaMethod = L" FXAA";
				else
					aaMethod = L" SMAA";

				std::wstring s;
				if (!_test)
				{
					s = L"Delta time :" + std::to_wstring(_timer.GetDeltaTime())
						+ L" FPS: " + std::to_wstring(_timer.GetFPS())
						+ L" --- AA METHOD: " + aaMethod
						+ L" Average FPS: " + std::to_wstring(_timer.GetAverageFPS())
						+ L" Average Time: " + std::to_wstring(_avgCT * 1000);
				}
				else
				{
					s = L"TEST RUNNING! - AA METHOD: " + aaMethod + L" - SCENE: " + std::to_wstring((int)_currentScene);
				}
				SetWindowText(_window->GetHWND(), s.c_str());

				_timer.Reset();
			}
		}
	}

	return false;
}

LRESULT CALLBACK Application::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	case WM_QUIT:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
	{
		return wndMessages->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}

void Application::Sampling()
{
	double temp = _profiling.GetAvgTime();
	if (temp > 0 && temp != _prevAvg)
	{
		_tempAvgCT += _profiling.GetAvgTime();
		if (increment > SAMPLERATE)
		{
			_avgCT = _tempAvgCT / SAMPLERATE;
			increment = 0;
			_tempAvgCT = 0;
			sceneChange = true;
			_avgFPS = _timer.GetAverageFPS2();
		}
		else
		{
			increment++;
		}
		_prevAvg = temp;
	}
}

void Application::ResetSampling()
{
	_tempAvgCT = 0;
	_avgCT = 0;
	_prevAvg = 0;
	increment = 0;
	sceneChange = false;
}

void Application::operator delete(void* p)
{
	_mm_free(p);
}

void* Application::operator new(size_t i)
{
	return _mm_malloc(i, 16);
}