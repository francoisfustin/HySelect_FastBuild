// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CGradientColor wrapper class

class CGradientColor : public COleDispatchDriver
{
public:
	CGradientColor() {}		// Calls COleDispatchDriver default constructor
	CGradientColor(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CGradientColor(const CGradientColor& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	unsigned long GetColor();
	void SetColor(unsigned long newValue);
	double GetOffset();
	void SetOffset(double newValue);
	long GetTransparency();
	void SetTransparency(long nNewValue);
};
