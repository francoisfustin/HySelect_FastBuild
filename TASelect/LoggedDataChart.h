//
// LoggedDataChart.h: interface for the CLoggedDataChart class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "tchart.h"
#include "axis.h"

// Colors define from the following site:
// http://www.tayloredmktg.com/rgb/
#define _LOGCOLOR1	RGB(0,0,255)	 // Blue
#define _LOGCOLOR2	RGB(255,0,0)	 // Red
#define _LOGCOLOR3	RGB(34,139,34)	 // Forest green(Green)
#define _LOGCOLOR4	RGB(0,255,255)	 // Cyan
#define _LOGCOLOR5	RGB(160,32,240)	 // Purple
#define _LOGCOLOR6	RGB(255,140,0)	 // Dark orange
#define _LOGCOLOR7	RGB(238,238,224) // Ivory 2(Gray)

class Clog;

class CLoggedDataChart : public CTChart  
{
// Construction
public:
	CLoggedDataChart();
	virtual ~CLoggedDataChart() {}

	virtual BOOL Create( LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID );

protected:
	// Set the main or global chart parameters
	void SetMainParam();
	// Add one series for logged data to the chart
	int AddLDSeries(int iVerticalAxis);
	// Fill series with data from the CLog object
	void FillSeries(CLog* pLD);
	// Define the Min and Max value each log type
	void SetMinMaxValue(double val, int iLogType);
	// Hide values for the custom axis and show only for first axis
	void RedefineCustomT1T2Axis(CAxis *pAxis, CAxis *pCustAxis);
	// Redefine the upper and lower limit for an axis scale
	void RedefineAxisScale(CAxis *pAxis,double dMin, double dMax, bool bTemp=false);

	typedef enum eLogDp
	{
		eLDP_Dp = 0,
		eLDP_Last
	};
	typedef enum eLogTemp
	{
		eLT_T1 = 0,
		eLT_T2,
		eLT_Last
	};
	typedef enum eLogFlow
	{
		eLF_Flow = 0,
		eLF_Dp,
		eLF_Last
	};
	typedef enum eLogDpTemp
	{
		eLDPT_Dp = 0,
		eLDPT_T1,
		eLDPT_T2,
		eLDPT_Last
	};
	typedef enum eLogFlowTemp
	{
		eLFT_Flow = 0,
		eLFT_T1,
		eLFT_T2,
		eLFT_Dp,
		eLFT_Last
	};

	typedef enum eLogDT
	{
		eLDT_DT = 0,
		eLDT_T1,
		eLDT_T2,
		eLDT_Last
	};
	typedef enum eLogPower
	{
		eLP_Power = 0,
		eLP_Flow,
		eLP_Dp,
		eLP_DT,
		eLP_T1,
		eLP_T2,
		eLP_Last
	};

	CUnitDatabase *m_pUnitDB;
	CLog *m_pLD;
	int m_iLdMode;
	int m_iCursorTool;
	bool m_bCursorTool;
	bool m_b2DZoom;
	bool m_bLegend;
	bool m_bDateonAxis;
	bool m_bFlowCurve;
	bool m_bDpCurve;
	bool m_bT1Curve;
	bool m_bT2Curve;
	bool m_bDTCurve;
	bool m_bPowerCurve;
	bool m_bSymbols;
	bool m_bT1Show;
	bool m_bT2Show;
	double m_dAlpha;
	double m_dOff7;
	double m_dMaxPower;
	double m_dMaxFlow;
	double m_dMaxDp;
	double m_dMaxT1;
	double m_dMaxT2;
	double m_dMaxDT;
	double m_dMinPower;
	double m_dMinFlow;
	double m_dMinDp;
	double m_dMinT1;
	double m_dMinT2;
	double m_dMinDT;
	CAxis m_CustAxis1;
	CAxis m_CustAxis2;
	CAxis m_CustAxis3;
	CAxis m_CustAxis4;

public:
	// Define the curves that must be shown by default
	void DefaultCurvesToDisplay();
	// Load and display in the chart the data series stored in pLD
	void DisplayLoggedData(CLog* pLD);
	void SetRedraw();
	// Toggle on and off the cursor tool (tracking)
	void ToggleCursorTool();
	// Toggle on and off 2D zooming and scrolling
	void Toggle2DZoom();
	// Toggle on and off the legend
	void ToggleLegend();
	// Toggle on and off the display of date in horizontal axis labels
	void ToggleDateAxis();
	// Toggle on and off the display of symbols on curves
	void ToggleSymbols();
	// Check/Uncheck the display of the vertical Flow axis
	void CheckUnCheckFlowAxis();
	// Check/Uncheck the display of the vertical Dp axis
	void CheckUnCheckDpAxis();
	// Check/Uncheck the display of the vertical T1 axis
	void CheckUnCheckT1Axis();
	// Check/Uncheck the display of the vertical T2 axis
	void CheckUnCheckT2Axis();
	// Check/Uncheck the display of the vertical DT axis
	void CheckUnCheckDTAxis();
	// Check/Uncheck the display of the vertical Power axis
	void CheckUnCheckPowerAxis();
	// Show/Hidde the curves
	void ShowHiddeCurves();
	// Return the Cursor Tool integer value
	int GetCursorTool() { return m_iCursorTool; };
	// Function that Zoom-In in 1 dimension (Horizontal)
	void ZoomIn1D();
	// Function that Zoom-Out in 1 dimension (Horizontal)
	void ZoomOut1D();
	// Rescale axis for T1 and T2 in case of 2D zooming to avoid
	// offset problem
	void SetMinMax2DZoom(double percentzoom=100);
	void SetMinMax2DZoomCustAxis(CAxis* pCurAxis, double percentzoom);
	// Return the boolean values for chart toolbar buttons
	bool IsCursorToolActive() { return m_bCursorTool; };
	bool Is2DZoomActive() { return m_b2DZoom; };
	bool IsLegendVisible() { return m_bLegend; };
	bool IsDateonAxis() { return m_bDateonAxis; };
	bool IsFlowCurve() { return m_bFlowCurve; };
	bool IsDpCurve() { return m_bDpCurve; };
	bool IsT1Curve() { return m_bT1Curve; };
	bool IsT2Curve() { return m_bT2Curve; };
	bool IsDTCurve() { return m_bDTCurve; };
	bool IsPowerCurve() { return m_bPowerCurve; };
	bool IsSymbolsActive() { return m_bSymbols; };
	bool IsT1ValidValues() {return m_bT1Show; };
	bool IsT2ValidValues() {return m_bT2Show; };
};
