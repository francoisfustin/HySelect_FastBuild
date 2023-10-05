// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CExpandAnimation;
class CTransformAnimation;
class CSeriesAnimationTool;

/////////////////////////////////////////////////////////////////////////////
// CChartAnimations wrapper class

class CChartAnimations : public COleDispatchDriver
{
public:
	CChartAnimations() {}		// Calls COleDispatchDriver default constructor
	CChartAnimations(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CChartAnimations(const CChartAnimations& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	CExpandAnimation GetAsExpand();
	CTransformAnimation GetAsTransformation();
	CSeriesAnimationTool GetAsSeriesAnimation();
};
