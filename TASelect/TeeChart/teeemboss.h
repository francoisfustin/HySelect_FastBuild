// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CTeeEmboss wrapper class

class CTeeEmboss : public COleDispatchDriver
{
public:
	CTeeEmboss() {}		// Calls COleDispatchDriver default constructor
	CTeeEmboss(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CTeeEmboss(const CTeeEmboss& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetHorizSize();
	void SetHorizSize(long nNewValue);
	long GetVertSize();
	void SetVertSize(long nNewValue);
	unsigned long GetColor();
	void SetColor(unsigned long newValue);
	long GetTransparency();
	void SetTransparency(long nNewValue);
	void ShowEditor(long OwnerChart);
	BOOL GetVisible();
	void SetVisible(BOOL bNewValue);
	BOOL GetSmooth();
	void SetSmooth(BOOL bNewValue);
	long GetSmoothBlur();
	void SetSmoothBlur(long nNewValue);
	BOOL GetClip();
	void SetClip(BOOL bNewValue);
};