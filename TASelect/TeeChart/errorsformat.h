// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CPen1;

/////////////////////////////////////////////////////////////////////////////
// CErrorsFormat wrapper class

class CErrorsFormat : public COleDispatchDriver
{
public:
	CErrorsFormat() {}		// Calls COleDispatchDriver default constructor
	CErrorsFormat(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CErrorsFormat(const CErrorsFormat& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	CPen1 GetLeft();
	CPen1 GetRight();
	CPen1 GetTop();
	CPen1 GetBottom();
};
