// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CTeeCorner;

/////////////////////////////////////////////////////////////////////////////
// CTeeCorners wrapper class

class CTeeCorners : public COleDispatchDriver
{
public:
	CTeeCorners() {}		// Calls COleDispatchDriver default constructor
	CTeeCorners(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CTeeCorners(const CTeeCorners& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	CTeeCorner GetLeftBottom();
	CTeeCorner GetLeftTop();
	CTeeCorner GetRightBottom();
	CTeeCorner GetRightTop();
};
