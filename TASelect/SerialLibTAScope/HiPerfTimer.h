#pragma once

class CHiPerfTimer
{
public:
	// Constructor
	CHiPerfTimer();

	bool Start();
	bool Stop();

	double Duration();
    
private:
	LARGE_INTEGER	m_lStartTime, m_lStopTime;
	LARGE_INTEGER	m_lFreq;
	bool			m_fSupported;
};
