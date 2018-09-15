#pragma once
#include <Windows.h>

namespace bpd{
	class Time {
	public:
		Time();
		~Time();

		void StartTimer();
		double GetTime();
		double GetFrameTime();
	private:
		double countsPerSecond;
		__int64 CounterStart;
		__int64 frameTimeOld;
	};
}