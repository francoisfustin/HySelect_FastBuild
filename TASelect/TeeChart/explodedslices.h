// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CExplodedSlices wrapper class

class CExplodedSlices : public COleDispatchDriver
{
public:
	CExplodedSlices() {}		// Calls COleDispatchDriver default constructor
	CExplodedSlices(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CExplodedSlices(const CExplodedSlices& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetValue(long Index);
	void SetValue(long Index, long nNewValue);
};
