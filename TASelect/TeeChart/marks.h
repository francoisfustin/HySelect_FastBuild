// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CTeeFont;
class CGradient;
class CTeeRect;
class CTeeShadow;
class CPen1;
class CBackImage;
class CTeeEmboss;
class CBrush1;
class CShapeCallout;
class CTeeCorners;
class CTeeShapes;
class CSeriesMarksPositions;
class CMarksCallout;
class CMarksItem;
class CMarksSymbol;
class CMargins;
class CAutomaticText;
class CMarksTail;

/////////////////////////////////////////////////////////////////////////////
// CMarks wrapper class

class CMarks : public COleDispatchDriver
{
public:
	CMarks() {}		// Calls COleDispatchDriver default constructor
	CMarks(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CMarks(const CMarks& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	unsigned long GetColor();
	void SetColor(unsigned long newValue);
	CTeeFont GetFont();
	CGradient GetGradient();
	long GetShadowSize();
	void SetShadowSize(long nNewValue);
	unsigned long GetShadowColor();
	void SetShadowColor(unsigned long newValue);
	BOOL GetTransparent();
	void SetTransparent(BOOL bNewValue);
	long GetShapeStyle();
	void SetShapeStyle(long nNewValue);
	long GetTransparency();
	void SetTransparency(long nNewValue);
	CTeeRect GetShapeBounds();
	long GetBevel();
	void SetBevel(long nNewValue);
	long GetBevelWidth();
	void SetBevelWidth(long nNewValue);
	void ShowEditorShape(BOOL HideTexts, LPCTSTR Caption);
	CTeeShadow GetShadow();
	long GetRoundSize();
	void SetRoundSize(long nNewValue);
	CPen1 GetPen();
	CBackImage GetPicture();
	void Show();
	void Hide();
	CTeeEmboss GetEmboss();
	long GetWidth();
	void SetWidth(long nNewValue);
	long GetHeight();
	void SetHeight(long nNewValue);
	CPen1 GetFrame();
	CBrush1 GetBrush();
	CShapeCallout GetShapeCallout();
	CTeeCorners GetCorners();
	CTeeShapes GetChildren();
	long GetChildLayout();
	void SetChildLayout(long nNewValue);
	long GetAngle();
	void SetAngle(long nNewValue);
	BOOL GetShapeVisible();
	void SetShapeVisible(BOOL bNewValue);
	long GetTextFormat();
	void SetTextFormat(long nNewValue);
	CPen1 GetArrow();
	long GetArrowLength();
	void SetArrowLength(long nNewValue);
	unsigned long GetBackColor();
	void SetBackColor(unsigned long newValue);
	BOOL GetClip();
	void SetClip(BOOL bNewValue);
	long GetStyle();
	void SetStyle(long nNewValue);
	BOOL GetVisible();
	void SetVisible(BOOL bNewValue);
	CSeriesMarksPositions GetPositions();
	long Clicked(long X, long Y);
	long GetZPosition();
	void SetZPosition(long nNewValue);
	long GetDrawEvery();
	void SetDrawEvery(long nNewValue);
	BOOL GetMultiline();
	void SetMultiline(BOOL bNewValue);
	void ResetPositions();
	CMarksCallout GetCallout();
	void ShowEditorMarks();
	CMarksItem GetItem(long Index);
	void Clear();
	CMarksSymbol GetSymbol();
	long GetTextAlign();
	void SetTextAlign(long nNewValue);
	CMargins GetMargins();
	BOOL GetFontSeriesColor();
	void SetFontSeriesColor(BOOL bNewValue);
	BOOL GetOnTop();
	void SetOnTop(BOOL bNewValue);
	BOOL GetAutoPosition();
	void SetAutoPosition(BOOL bNewValue);
	BOOL GetSoftClip();
	void SetSoftClip(BOOL bNewValue);
	BOOL GetUseSeriesTransparency();
	void SetUseSeriesTransparency(BOOL bNewValue);
	CAutomaticText GetAutomatic();
	CMarksTail GetTail();
};
