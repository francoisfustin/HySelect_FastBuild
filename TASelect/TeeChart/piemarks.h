// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CPieMarks wrapper class

class CPieMarks : public COleDispatchDriver
{
public:
	CPieMarks() {}		// Calls COleDispatchDriver default constructor
	CPieMarks(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CPieMarks(const CPieMarks& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	BOOL GetVertCenter();
	void SetVertCenter(BOOL bNewValue);
	long GetLegSize();
	void SetLegSize(long nNewValue);
	BOOL GetInsideSlice();
	void SetInsideSlice(BOOL bNewValue);
	BOOL GetRotated();
	void SetRotated(BOOL bNewValue);
	BOOL GetEmptySlice();
	void SetEmptySlice(BOOL bNewValue);
};
