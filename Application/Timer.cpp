#include "Timer.h"

using namespace std;

Timer::Timer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&_frequency);

	if (_frequency == 0)
		throw runtime_error("Timer");

	_ticksPerMs = (float)(_frequency / 1000);

	QueryPerformanceCounter((LARGE_INTEGER*)&_startTime);

	_deltaTime = 0;
	_gameTime = 0;

	_avgFPS = 0;
	_avgDeltaTime = 0;
	_tempIncr = 0;
	_tempAvgFPS = 0;
	_tempAvgDT = 0;

	_startClock = 0;
	_stopClock = 0;
	_timeDifference = 0;
	_avgClock = 0;
	_tempAvgClockTime = 0;

}

Timer::~Timer()
{
}

void Timer::Update()
{
	INT64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	double timeDifference = (double)(currentTime - _startTime);
	_tempTime = timeDifference / _ticksPerMs;
	_startTime = currentTime;

	_deltaTime += _tempTime;
	_gameTime += _tempTime;
	_fps = (1 / (_deltaTime / 1000));

	//Average
	const int incrTimer = 5000;
	_tempAvgFPS += _fps;
	_tempAvgDT += _deltaTime;
	_tempAvgClockTime += _timeDifference;
	_tempIncr++;

	/*
	if (_tempIncr > incrTimer)
	{
		_tempIncr = 0;
		_avgFPS = _tempAvgFPS / incrTimer;
		_tempAvgFPS = 0;
		_avgDeltaTime = _tempAvgDT / incrTimer;
		_tempAvgDT = 0;
		_avgClock = static_cast<double>((_tempAvgClockTime / (incrTimer * _ticksPerMs)));
		_tempAvgClockTime = 0;
	}*/

	if (_gameTime > (DBL_MAX - 100000000))
	{
		_gameTime = 0;
	}
	
}

void Timer::ResetAverages()
{
	_tempIncr = 0;
	_tempAvgFPS = 0;
	_tempAvgDT = 0;
	_tempAvgClockTime = 0;
	_avgFPS = 0;
	_avgDeltaTime = 0;
	_avgClock = 0;
}



void Timer::StartClock()
{
	_startClock = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&_startClock);
}

void Timer::StopClock()
{
	_stopClock = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&_stopClock);
	_timeDifference = (_stopClock - _startClock);
}

// Seconds
double Timer::GetDeltaTime()
{
	return _deltaTime;
}

double Timer::GetGameTime()
{
	return _gameTime;
}

double Timer::GetAverageDeltaTime()
{
	return _avgDeltaTime;
}

double Timer::GetAverageFPS()
{
	return _avgFPS;
}

void Timer::Reset()
{
	_deltaTime = 0;
}

int Timer::GetFPS()
{
	return _fps;
}

double Timer::GetCurrentClockTime()
{
	return (double)_timeDifference;
}

double Timer::GetAverageClockTime()
{
	return _avgClock;
}

double Timer::GetAverageFPS2()
{
	double tempFPS = (_tempAvgFPS / _tempIncr);
	_tempAvgFPS = 0;
	_tempIncr = 0;
	return tempFPS;
}
