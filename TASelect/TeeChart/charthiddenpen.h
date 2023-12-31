// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CBrush1;

/////////////////////////////////////////////////////////////////////////////
// CChartHiddenPen wrapper class

class CChartHiddenPen : public COleDispatchDriver
{
public:
	CChartHiddenPen() {}		// Calls COleDispatchDriver default constructor
	CChartHiddenPen(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CChartHiddenPen(const CChartHiddenPen& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	unsigned long GetColor();
	void SetColor(unsigned long newValue);
	long GetWidth();
	void SetWidth(long nNewValue);
	long GetStyle();
	void SetStyle(long nNewValue);
	BOOL GetVisible();
	void SetVisible(BOOL bNewValue);
	long GetEndStyle();
	void SetEndStyle(long nNewValue);
	BOOL ShowEditor(BOOL HideColor, LPCTSTR Title, long OwnerChart);
	long GetSmallSpace();
	void SetSmallSpace(long nNewValue);
	long GetMode();
	void SetMode(long nNewValue);
	void Show();
	void Hide();
	CBrush1 GetFill();
	long GetJoinStyle();
	void SetJoinStyle(long nNewValue);
	BOOL GetSmallDots();
	void SetSmallDots(BOOL bNewValue);
};
