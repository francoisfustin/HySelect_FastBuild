// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CLightTool wrapper class

class CLightTool : public COleDispatchDriver
{
public:
	CLightTool() {}		// Calls COleDispatchDriver default constructor
	CLightTool(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CLightTool(const CLightTool& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetFactor();
	void SetFactor(long nNewValue);
	long GetLeft();
	void SetLeft(long nNewValue);
	long GetStyle();
	void SetStyle(long nNewValue);
	long GetTop();
	void SetTop(long nNewValue);
	BOOL GetFollowMouse();
	void SetFollowMouse(BOOL bNewValue);
};
