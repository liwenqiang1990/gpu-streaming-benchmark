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
	double GetTimeSecond();
  double GetAllTimeSecond();
  void Clear(){_time = 0.0;}

private:
#ifdef WIN32
	LARGE_INTEGER _ticksPerSecond;
	LARGE_INTEGER _tic, _toc; 
#else
	timeval _t1, _t2;
	double _elapsedTime;
#endif
  double _time;
};

inline PortableTimer::PortableTimer()
{
  _time = 0.0;
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
  _time += double(_toc.LowPart-_tic.LowPart)/double(_ticksPerSecond.LowPart);
#else 
	gettimeofday(&_t2,NULL);
  _time += (_t2.tv_sec - _t1.tv_sec)+(_t2.tv_usec - _t1.tv_usec)/1000000.0;
#endif
}

inline double PortableTimer::GetTimeSecond()
{
#ifdef WIN32
	return double(_toc.LowPart-_tic.LowPart)/double(_ticksPerSecond.LowPart);
#else
	return double(_elapsedTime = (_t2.tv_sec - _t1.tv_sec)+(_t2.tv_usec - _t1.tv_usec)/1000000.0);
#endif
}

inline double PortableTimer::GetAllTimeSecond()
{
  return _time;
}

#endif
