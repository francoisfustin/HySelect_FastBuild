// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CValueList;
class CErrorsFormat;

/////////////////////////////////////////////////////////////////////////////
// CErrors wrapper class

class CErrors : public COleDispatchDriver
{
public:
	CErrors() {}		// Calls COleDispatchDriver default constructor
	CErrors(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CErrors(const CErrors& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	CValueList GetLeft();
	CValueList GetRight();
	CValueList GetTop();
	CValueList GetBottom();
	BOOL GetSeriesColor();
	void SetSeriesColor(BOOL bNewValue);
	long GetSize();
	void SetSize(long nNewValue);
	long GetSizeUnits();
	void SetSizeUnits(long nNewValue);
	CErrorsFormat GetFormat();
};