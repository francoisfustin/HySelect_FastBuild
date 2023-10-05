// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CTeePoint2D;
class CTeeShadow;
class CBrush1;
class CPen1;
class CPointer;
class CValueList;
class CCircleLabels;
class CGradient;
class CPolarLabels;
class CClockHand;

/////////////////////////////////////////////////////////////////////////////
// CClockSeries wrapper class

class CClockSeries : public COleDispatchDriver
{
public:
	CClockSeries() {}		// Calls COleDispatchDriver default constructor
	CClockSeries(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CClockSeries(const CClockSeries& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetXRadius();
	void SetXRadius(long nNewValue);
	long GetYRadius();
	void SetYRadius(long nNewValue);
	long GetXCenter();
	long GetYCenter();
	long GetCircleWidth();
	long GetCircleHeight();
	unsigned long GetCircleBackColor();
	void SetCircleBackColor(unsigned long newValue);
	BOOL GetCircled();
	void SetCircled(BOOL bNewValue);
	long GetRotationAngle();
	void SetRotationAngle(long nNewValue);
	CTeePoint2D AngleToPoint(double Angle, double AXRadius, double AYRadius);
	double PointToAngle(long XCoord, long YCoord);
	double PointToRadius(long XCoord, long YCoord);
	double GetCustomXRadius();
	void SetCustomXRadius(double newValue);
	double GetCustomYRadius();
	void SetCustomYRadius(double newValue);
	CTeeShadow GetShadow();
	CBrush1 GetCircleBrush();
	CPen1 GetCirclePen();
	BOOL GetColorEachLine();
	void SetColorEachLine(BOOL bNewValue);
	CPointer GetPointer();
	BOOL GetPointerBehind();
	void SetPointerBehind(BOOL bNewValue);
	long GetDrawStyle();
	void SetDrawStyle(long nNewValue);
	long GetTreatNulls();
	void SetTreatNulls(long nNewValue);
	double GetAngleIncrement();
	void SetAngleIncrement(double newValue);
	BOOL GetCloseCircle();
	void SetCloseCircle(BOOL bNewValue);
	CPen1 GetPen();
	double GetRadiusIncrement();
	void SetRadiusIncrement(double newValue);
	CValueList GetAngleValues();
	CValueList GetRadiusValues();
	void DrawRing(double Value, long Z);
	CCircleLabels GetCircleLabels();
	CBrush1 GetBrush();
	void LoadBackImage(LPCTSTR FileName);
	void ClearBackImage();
	BOOL GetClockWiseLabels();
	void SetClockWiseLabels(BOOL bNewValue);
	BOOL GetCircleLabelsInside();
	void SetCircleLabelsInside(BOOL bNewValue);
	long GetTransparency();
	void SetTransparency(long nNewValue);
	CGradient GetCircleGradient();
	void DrawZone(double Min, double Max, long Z);
	long GetLabelsMargin();
	void SetLabelsMargin(long nNewValue);
	CPolarLabels GetAngleLabels();
	BOOL GetRomanNumbers();
	void SetRomanNumbers(BOOL bNewValue);
	CPen1 GetPenHours();
	CPen1 GetPenMinutes();
	CPen1 GetPenSeconds();
	long GetStyle();
	void SetStyle(long nNewValue);
	double GetTime();
	void SetTime(double newValue);
	CClockHand GetHours();
	CClockHand GetMinutes();
	CClockHand GetSeconds();
	BOOL GetCustomTime();
	void SetCustomTime(BOOL bNewValue);
	CPointer GetCenter();
};
