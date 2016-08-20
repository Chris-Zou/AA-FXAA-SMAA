#pragma once
#include <windows.h>
#include <stdexcept>

class Timer
{
private:

	INT64 _frequency;
	double _ticksPerMs;
	INT64 _startTime;

	double _tempTime;
	double _deltaTime;
	double _gameTime;
	double _fps;

	//
	double	_avgFPS;
	double _avgDeltaTime;
	double _tempIncr;
	double _tempAvgFPS;
	double _tempAvgDT;
	//
	__int64 _startClock;
	__int64 _stopClock;
	__int64 _timeDifference;
	double _avgClock;
	__int64 _tempAvgClockTime;

public:

	Timer();
	~Timer();

	void Update();

	void StartClock();
	void StopClock();

	double GetDeltaTime();
	double GetGameTime();
	double GetAverageDeltaTime();
	double GetAverageFPS();
	int GetFPS();
	double GetCurrentClockTime();
	double GetAverageClockTime();
	void Reset();
	void ResetAverages();

	double GetAverageFPS2();
};