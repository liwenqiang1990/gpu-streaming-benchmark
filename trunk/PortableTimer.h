#ifndef _PORTABLE_TIMER_H
#define _PORTABLE_TIMER_H 

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


//TODO portability 
class PortableTimer
{
public:
	PortableTimer();
	void StartTimer();
	void EndTimer();
	float GetTimeSecond();

private:
#ifdef WIN32
	LARGE_INTEGER _ticksPerSecond;
	LARGE_INTEGER _tic, _toc; 
#else
	timeval _t1, _t2;
	double _elapsedTime;
#endif

};


inline PortableTimer::PortableTimer()
{
#ifdef WIN32
	QueryPerformanceFrequency(&_ticksPerSecond);

#endif
}


inline void PortableTimer::StartTimer()
{
#ifdef WIN32
	QueryPerformanceCounter(&_tic);
#else
	gettimeofday(&_t1,NULL);
#endif
}
inline void PortableTimer::EndTimer()
{
#ifdef WIN32
	QueryPerformanceCounter(&_toc);
#else 
	gettimeofday(&_t2,NULL);
#endif
}

inline float PortableTimer::GetTimeSecond()
{
#ifdef WIN32
	return float(_toc.LowPart-_tic.LowPart)/float(_ticksPerSecond.LowPart);
#else
	return float(_elapsedTime = (_t2.tv_sec - _t1.tv_sec));
#endif
}

#endif
