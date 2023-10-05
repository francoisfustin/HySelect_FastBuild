// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CSeriesAnimationTool wrapper class

class CSeriesAnimationTool : public COleDispatchDriver
{
public:
	CSeriesAnimationTool() {}		// Calls COleDispatchDriver default constructor
	CSeriesAnimationTool(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CSeriesAnimationTool(const CSeriesAnimationTool& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetDrawEvery();
	void SetDrawEvery(long nNewValue);
	BOOL GetStartAtMin();
	void SetStartAtMin(BOOL bNewValue);
	double GetStartValue();
	void SetStartValue(double newValue);
	long GetSteps();
	void SetSteps(long nNewValue);
	void Execute();
	BOOL GetRunning();
	void Stop();
	long GetDelay();
	void SetDelay(long nNewValue);
	long GetLoop();
	void SetLoop(long nNewValue);
	long GetDuration();
	void SetDuration(long nNewValue);
	VARIANT GetSeries();
	void SetSeries(const VARIANT& newValue);
};
