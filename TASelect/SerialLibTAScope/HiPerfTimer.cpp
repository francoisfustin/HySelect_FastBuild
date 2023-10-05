#include "stdafx.h"
#include "HiPerfTimer.h"

CHiPerfTimer::CHiPerfTimer()
{
	ZeroMemory((void*)&m_lStartTime, sizeof(LARGE_INTEGER));
	ZeroMemory((void*)&m_lStopTime, sizeof(LARGE_INTEGER));
	BOOL value = QueryPerformanceFrequency(&m_lFreq);
	m_fSupported = ( value == 0) ? false : true;
}

bool CHiPerfTimer::Start()
{
	if(!m_fSupported)
		return false;
	QueryPerformanceCounter(&m_lStartTime);
	return true;
}

bool CHiPerfTimer::Stop()
{
	if(!m_fSupported)
		return false;
	QueryPerformanceCounter(&m_lStopTime);
	return true;
}

// Returns the duration of the timer (in seconds)
double CHiPerfTimer::Duration()
{
	if(!m_fSupported)
		return 0.0;
	double dMircoSecond = (double)(m_lFreq.QuadPart/1000000);	
	return (double)(m_lStopTime.QuadPart - m_lStartTime.QuadPart) / (double)dMircoSecond;
}
