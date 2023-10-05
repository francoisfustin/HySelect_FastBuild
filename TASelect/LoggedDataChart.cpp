//
// LoggedDataChart.cpp: implementation of the CLoggedDataChart class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "comdef.h"

#include "TeeChartDefines.h"
#include "Aspect.h"
#include "Panel.h"

#include "Axes.h"
#include "Axis.h"
#include "Chartaxispen.h"
#include "AxisTitle.h"
#include "AxisLabels.h"
#include "CursorTool.h"
#include "Legend.h"
#include "LegendSymbol.h"
#include "LineSeries.h"
#include "MarksTipTool.h"
#include "NearestTool.h"
#include "Pen.h"
#include "Pointer.h"
#include "Scroll.h"
#include "Series.h"
#include "seriespointeritems.h"
#include "Strings.h"
#include "TeeFont.h"
#include "TeeOpenGL.h"
#include "Titles.h"
#include "ToolList.h"
#include "Tools.h"
#include "ValueList.h"
#include "Wall.h"
#include "Walls.h"
#include "Zoom.h"
#include "Math.h"

#include "LoggedDataChart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoggedDataChart::CLoggedDataChart()
{
	m_pUnitDB = NULL;
	m_pLD = NULL;
	m_iLdMode =	CLog/*::LogType*/::LOGTYPE_LAST;
	m_iCursorTool = 0;
	m_bCursorTool = false;
	m_b2DZoom = false;
	m_bLegend = true;
	m_bDateonAxis = true;
	m_bFlowCurve = false;
	m_bDpCurve = false;
	m_bT1Curve = false;
	m_bT2Curve = false;
	m_bDTCurve = false;
	m_bPowerCurve = false;
	m_bSymbols = false;
	m_bT1Show = false;
	m_bT2Show = false;
	m_dAlpha = 50.0;
	m_dOff7 = 0.0;
	m_dMinPower = 999999.0;
	m_dMinFlow = 999999.0;
	m_dMinDp = 999999.0;
	m_dMinT1 = 999999.0;
	m_dMinT2 = 999999.0;
	m_dMinDT = 999999.0;
	m_dMaxPower = -999999.0;
	m_dMaxFlow = -999999.0;
	m_dMaxDp = -999999.0;
	m_dMaxT1 = -999999.0;
	m_dMaxT2 = -999999.0;
	m_dMaxDT = -999999.0;
}

BOOL CLoggedDataChart::Create( LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID )
{
	if( FALSE == CTChart::Create( lpszWindowName, dwStyle ^ WS_BORDER, rect, pParentWnd, nID ) )
	{
		return FALSE;
	}

	m_pUnitDB = CDimValue::AccessUDB();

	return TRUE;
}

void CLoggedDataChart::SetMainParam()
{
	// Remove the thin default bevelled border
	CTChart::GetPanel().SetBevelOuter(bvNone);

	// Set background color of the chart panel to white
	// and background color of the chart to std light gray
	CTChart::GetPanel().SetColor(RGB(255,255,255));
	CTChart::GetWalls().SetBackColor(RGB(192,192,192));
	CTChart::GetWalls().GetBack().SetTransparent(false);

	// Set to 2D
	CTChart::GetAspect().SetView3D(false);

	// Set the zooming and scrolling direction to horizontal only 
	// (zooming direction options interfaced in toolbar)
	CTChart::GetZoom().SetDirection(m_b2DZoom?tzdBoth:tzdHorizontal);
	CTChart::GetScroll().SetEnable(m_b2DZoom?pmBoth:pmHorizontal);

	//=================
	// Parametrize axes
	//=================
	
	// Add Custom Vertical axes
	// the bigger Logg is for the Power
	// with 3 verticals axes on the left
	// and 2 on the right.
	// So : 2 exit and 3 must be created
	CTChart::GetAxis().AddCustom(FALSE);
	CTChart::GetAxis().AddCustom(FALSE);
	CTChart::GetAxis().AddCustom(FALSE);
	CTChart::GetAxis().AddCustom(FALSE);
	m_CustAxis1 = CTChart::GetAxis().GetCustom(0);
	m_CustAxis2 = CTChart::GetAxis().GetCustom(1);
	m_CustAxis3 = CTChart::GetAxis().GetCustom(2);
	m_CustAxis4 = CTChart::GetAxis().GetCustom(3);
	
	m_CustAxis1.SetOtherside(TRUE);
	m_CustAxis1.GetTitle().GetFont().SetBold(true);
	m_CustAxis1.GetGridPen().SetVisible(false);
	m_CustAxis1.GetMinorGrid().SetVisible(false);
	m_CustAxis1.SetPositionPercent(99.8);
	m_CustAxis1.GetTitle().SetAngle(90);
	
	m_CustAxis2.GetTitle().GetFont().SetBold(true);
	m_CustAxis2.GetGridPen().SetVisible(false);
	m_CustAxis2.GetMinorGrid().SetVisible(false);
	m_CustAxis2.SetPositionPercent(99.8);
	m_CustAxis2.GetTitle().SetAngle(90);
	
	m_CustAxis3.GetTitle().GetFont().SetBold(true);
	m_CustAxis3.GetGridPen().SetVisible(false);
	m_CustAxis3.GetMinorGrid().SetVisible(false);
	m_CustAxis3.GetTitle().SetAngle(90);

	m_CustAxis4.SetOtherside(TRUE);
	m_CustAxis4.GetTitle().GetFont().SetBold(true);
	m_CustAxis4.GetGridPen().SetVisible(false);
	m_CustAxis4.GetMinorGrid().SetVisible(false);
	m_CustAxis4.GetTitle().SetAngle(90);
	
	

	CTChart::GetAxis().GetLeft().GetTitle().GetFont().SetBold(true);
	CTChart::GetAxis().GetRight().GetTitle().GetFont().SetBold(true);
	CTChart::GetAxis().GetRight().GetTitle().SetAngle(90);

	CTChart::GetAxis().GetRight().GetGridPen().SetVisible(false);
	CTChart::GetAxis().GetRight().GetMinorGrid().SetVisible(false);

//	CTChart::GetAxis().GetRight().GetLabels().SetAlign(1); // Orientate right axis labels vertically
//	CTChart::GetAxis().GetRight().GetLabels().SetAngle(90);
//	CTChart::GetAxis().GetRight().GetTicks().SetVisible(false);

//	CTChart::GetAxis().GetBottom().SetIncrement(CTChart::GetDateTimeStep(dtTenSeconds));
	CTChart::GetAxis().GetBottom().GetLabels().SetDateTimeFormat(m_bDateonAxis?_T("dd/mm/yyyy hh:mm:ss"):_T("hh:mm:ss"));
	CTChart::GetAxis().GetBottom().GetLabels().SetMultiline(true);

	//===================
	// Parametrize Legend
	//===================
	CTChart::GetLegend().SetLegendStyle(lsSeries);
	CTChart::GetLegend().GetSymbol().SetWidthUnits(lcsPixels);
	CTChart::GetLegend().GetSymbol().SetWidth(24);
	CTChart::GetLegend().SetAlignment(laBottom);
	CTChart::GetLegend().SetVisible(m_bLegend);
}

int CLoggedDataChart::AddLDSeries(int iVerticalAxis)
{
	// Add series as Line
	int iSeries = CTChart::AddSeries(scLine);

	// Variables
	unsigned long color;
	
	// Set series color
	switch(m_iLdMode)
	{
	case CLog/*::LogType*/::LOGTYPE_DP:
		color = _LOGCOLOR4;		     // Dp
		break;
	case CLog/*::LogType*/::LOGTYPE_TEMP:
		if (iVerticalAxis == eLT_T1)
			color = _LOGCOLOR5;
		else if (iVerticalAxis == eLT_T2)
			color = _LOGCOLOR6;
		break;
	case CLog/*::LogType*/::LOGTYPE_FLOW:
		if (iVerticalAxis == eLF_Flow)
			color = _LOGCOLOR1;
		else if (iVerticalAxis == eLF_Dp)
			color = _LOGCOLOR4;
		break;
	case CLog/*::LogType*/::LOGTYPE_DPTEMP:
		if (iVerticalAxis == eLDPT_Dp)
			color = _LOGCOLOR4;
		else if (iVerticalAxis == eLDPT_T1)
			color = _LOGCOLOR5;
		else if (iVerticalAxis == eLDPT_T2)
			color = _LOGCOLOR6;
		break;		
	case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
		if (iVerticalAxis == eLFT_Flow)
			color = _LOGCOLOR1;
		else if (iVerticalAxis == eLFT_T1)
			color = _LOGCOLOR5;
		else if (iVerticalAxis == eLFT_T2)
			color = _LOGCOLOR6;
		else if (iVerticalAxis == eLFT_Dp)
			color = _LOGCOLOR4;
		break;
	case CLog/*::LogType*/::LOGTYPE_DT:
		if (iVerticalAxis == eLDT_DT)
			color = _LOGCOLOR3;
		else if (iVerticalAxis == eLDT_T1)
			color = _LOGCOLOR5;
		else if (iVerticalAxis == eLDT_T2)
			color = _LOGCOLOR6;
		break;
	case CLog/*::LogType*/::LOGTYPE_POWER:
		if (iVerticalAxis == eLP_Power)
			color = _LOGCOLOR2;
		else if (iVerticalAxis == eLP_Flow)
			color = _LOGCOLOR1;
		else if (iVerticalAxis == eLP_Dp)
			color = _LOGCOLOR4;
		else if (iVerticalAxis == eLP_DT)
			color = _LOGCOLOR3;
		else if (iVerticalAxis == eLP_T1)
			color = _LOGCOLOR5;
		else if (iVerticalAxis == eLP_T2)
			color = _LOGCOLOR6;
		break;
	};
	CTChart::Series(iSeries).SetColor(color);

	// Add symbols to the series
	CTChart::Series(iSeries).GetAsLine().GetPointer().SetVisible(m_bSymbols);
	CTChart::Series(iSeries).GetAsLine().GetPointer().SetStyle(iSeries+1); // Pointer style shifted by one
	CTChart::Series(iSeries).GetAsLine().GetPointer().SetHorizontalSize(2);
	CTChart::Series(iSeries).GetAsLine().GetPointer().SetVerticalSize(2);
	CTChart::Series(iSeries).GetAsLine().SetClickableLine(false);

	return iSeries;
}

void CLoggedDataChart::FillSeries(CLog* pLD)
{
	int nData = pLD->GetLength();
	int LdMode = pLD->GetMode();

	DWORD Array[] = {(DWORD)nData};

	 // Create safe-arrays
	COleSafeArray XValues,YValues;
	XValues.Create(VT_R8, 1 , Array);
	YValues.Create(VT_R8, 1 , Array);

	// Reset the values
	m_dMinPower=m_dMinFlow=m_dMinDp=m_dMinT1=m_dMinT2=m_dMinDT=999999;
	m_dMaxPower=m_dMaxFlow=m_dMaxDp=m_dMaxT1=m_dMaxT2=m_dMaxDT=-999999;

	// Determine physical type to convert
	typedef std::pair<int,CLog::eChannelType> LogType;
	LogType iUnit[eLP_Last];
	int iNbrUnits;
	switch(LdMode)
	{
	case CLog/*::LogType*/::LOGTYPE_DP:
		iNbrUnits=eLDP_Last;
		iUnit[eLDP_Dp]=LogType(_U_DIFFPRESS,CLog::ect_Dp);
		break;
	case CLog/*::LogType*/::LOGTYPE_TEMP:
		iNbrUnits=eLT_Last;
		iUnit[eLT_T1]=LogType(_U_TEMPERATURE,CLog::ect_Temp1);
		iUnit[eLT_T2]=LogType(_U_TEMPERATURE,CLog::ect_Temp2);
		break;
	case CLog/*::LogType*/::LOGTYPE_FLOW:
		iNbrUnits=eLF_Last;
		iUnit[eLF_Flow]=LogType(_U_FLOW,CLog::ect_Flow);
		iUnit[eLF_Dp]=LogType(_U_DIFFPRESS,CLog::ect_Dp);
		break;
	case CLog/*::LogType*/::LOGTYPE_DPTEMP:
		iNbrUnits=eLDPT_Last;
		iUnit[eLDPT_Dp]=LogType(_U_DIFFPRESS,CLog::ect_Dp);
		iUnit[eLDPT_T1]=LogType(_U_TEMPERATURE,CLog::ect_Temp1);
		iUnit[eLDPT_T2]=LogType(_U_TEMPERATURE,CLog::ect_Temp2);
		break;		
	case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
		iNbrUnits=eLFT_Last;
		iUnit[eLFT_Flow]=LogType(_U_FLOW,CLog::ect_Flow);
		iUnit[eLFT_T1]=LogType(_U_TEMPERATURE,CLog::ect_Temp1);
		iUnit[eLFT_T2]=LogType(_U_TEMPERATURE,CLog::ect_Temp2);
		iUnit[eLFT_Dp]=LogType(_U_DIFFPRESS,CLog::ect_Dp);		
		break;
	case CLog/*::LogType*/::LOGTYPE_DT:
		iNbrUnits=eLDT_Last;
		iUnit[eLDT_DT]=LogType(_U_DIFFTEMP,CLog::ect_DeltaT);
		iUnit[eLDT_T1]=LogType(_U_TEMPERATURE,CLog::ect_Temp1);
		iUnit[eLDT_T2]=LogType(_U_TEMPERATURE,CLog::ect_Temp2);
		break;
	case CLog/*::LogType*/::LOGTYPE_POWER:
		iNbrUnits=eLP_Last;
		iUnit[eLP_Power]=LogType(_U_TH_POWER,CLog::ect_Power);
		iUnit[eLP_Flow]=LogType(_U_FLOW,CLog::ect_Flow);
		iUnit[eLP_Dp]=LogType(_U_DIFFPRESS,CLog::ect_Dp);
		iUnit[eLP_DT]=LogType(_U_DIFFTEMP,CLog::ect_DeltaT);
		iUnit[eLP_T1]=LogType(_U_TEMPERATURE,CLog::ect_Temp1);
		iUnit[eLP_T2]=LogType(_U_TEMPERATURE,CLog::ect_Temp2);
		break;
	};
	
	// Do a loop on all value that must be drawn
	for (int i=0; i<iNbrUnits; i++)
	{
		// Initialize XValues array with DateTime
		// and YValues array for series i
		double yVal;
		COleDateTime Oletime;
		CTChart::Series(i).GetXValues().SetDateTime(true);

		// First list element
		long index = 0;
		pLD->GetOleDateTime(index,Oletime);
		XValues.PutElement(&index,&Oletime);

		pLD->GetFirst(iUnit[i].second,yVal);
		yVal = CDimValue::SItoCU(iUnit[i].first,yVal);
		YValues.PutElement(&index,&yVal);
		SetMinMaxValue(yVal,i);

		// Iteration on the list
		for (index=1; index<nData; index++)
		{
			pLD->GetOleDateTime(index,Oletime);
			XValues.PutElement(&index,&Oletime);

			pLD->GetNext(iUnit[i].second,yVal);
			// Convert from SI to CU units
			yVal = CDimValue::SItoCU(iUnit[i].first,yVal);
			YValues.PutElement(&index,&yVal);
			SetMinMaxValue(yVal,i);
		}
		
		// Add arrays for series i
		CTChart::Series(i).AddArray(nData,YValues,XValues);
	}
	
	// Gray values for unused sensor (Special case for TA-SCOPE)
	m_bT1Show=m_bT2Show=true;
	switch(LdMode)
	{
	case CLog/*::LogType*/::LOGTYPE_TEMP:
	case CLog/*::LogType*/::LOGTYPE_DPTEMP:	
	case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
		if (pLD->IsT1SensorUsed())
		{m_bT2Show = false;m_bT2Curve = false;}
		else
		{m_bT1Show = false;m_bT1Curve = false;}
		break;
	};
}
// Define the Min and Max value each log type
void CLoggedDataChart::SetMinMaxValue(double val, int iLogType)
{
	// Set the min max value for T1 and T2
	switch(m_iLdMode)
	{
	case CLog/*::LogType*/::LOGTYPE_DP:
		if (iLogType==eLDP_Dp)
		{
			if (val<m_dMinDp)
				m_dMinDp=val;
			if (val>m_dMaxDp)
				m_dMaxDp=val;
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_TEMP:
		if (iLogType==eLT_T1)
		{
			if (val<m_dMinT1)
				m_dMinT1=val;
			if (val>m_dMaxT1)
				m_dMaxT1=val;
		}
		else if (iLogType==eLT_T2)
		{
			if (val<m_dMinT2)
				m_dMinT2=val;
			if (val>m_dMaxT2)
				m_dMaxT2=val;
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_FLOW:
		if (iLogType==eLF_Flow)
		{
			if (val<m_dMinFlow)
				m_dMinFlow=val;
			if (val>m_dMaxFlow)
				m_dMaxFlow=val;
		}
		else if (iLogType==eLF_Dp)
		{
			if (val<m_dMinDp)
				m_dMinDp=val;
			if (val>m_dMaxDp)
				m_dMaxDp=val;
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_DPTEMP:
		if (iLogType==eLDPT_Dp)
		{
			if (val<m_dMinDp)
				m_dMinDp=val;
			if (val>m_dMaxDp)
				m_dMaxDp=val;
		}
		else if (iLogType==eLDPT_T1)
		{
			if (val<m_dMinT1)
				m_dMinT1=val;
			if (val>m_dMaxT1)
				m_dMaxT1=val;
		}
		else if (iLogType==eLDPT_T2)
		{
			if (val<m_dMinT2)
				m_dMinT2=val;
			if (val>m_dMaxT2)
				m_dMaxT2=val;
		}
		break;		
	case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
		if (iLogType==eLFT_Flow)
		{
			if (val<m_dMinFlow)
				m_dMinFlow=val;
			if (val>m_dMaxFlow)
				m_dMaxFlow=val;
		}
		else if (iLogType==eLFT_T1)
		{
			if (val<m_dMinT1)
				m_dMinT1=val;
			if (val>m_dMaxT1)
				m_dMaxT1=val;
		}
		else if (iLogType==eLFT_T2)
		{
			if (val<m_dMinT2)
				m_dMinT2=val;
			if (val>m_dMaxT2)
				m_dMaxT2=val;
		}
		else if (iLogType==eLFT_Dp)
		{
			if (val<m_dMinDp)
				m_dMinDp=val;
			if (val>m_dMaxDp)
				m_dMaxDp=val;
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_DT:
		if (iLogType==eLDT_DT)
		{
			if (val<m_dMinDT)
				m_dMinDT=val;
			if (val>m_dMaxDT)
				m_dMaxDT=val;
		}
		else if (iLogType==eLDT_T1)
		{
			if (val<m_dMinT1)
				m_dMinT1=val;
			if (val>m_dMaxT1)
				m_dMaxT1=val;
		}
		else if (iLogType==eLDT_T2)
		{
			if (val<m_dMinT2)
				m_dMinT2=val;
			if (val>m_dMaxT2)
				m_dMaxT2=val;
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_POWER:
		if (iLogType==eLP_Power)
		{
			if (val<m_dMinPower)
				m_dMinPower=val;
			if (val>m_dMaxPower)
				m_dMaxPower=val;
		}
		else if (iLogType==eLP_Flow)
		{
			if (val<m_dMinFlow)
				m_dMinFlow=val;
			if (val>m_dMaxFlow)
				m_dMaxFlow=val;
		}
		else if (iLogType==eLP_Dp)
		{
			if (val<m_dMinDp)
				m_dMinDp=val;
			if (val>m_dMaxDp)
				m_dMaxDp=val;
		}
		else if (iLogType==eLP_DT)
		{
			if (val<m_dMinDT)
				m_dMinDT=val;
			if (val>m_dMaxDT)
				m_dMaxDT=val;
		}
		else if (iLogType==eLP_T1)
		{
			if (val<m_dMinT1)
				m_dMinT1=val;
			if (val>m_dMaxT1)
				m_dMaxT1=val;
		}
		else if (iLogType==eLP_T2)
		{
			if (val<m_dMinT2)
				m_dMinT2=val;
			if (val>m_dMaxT2)
				m_dMaxT2=val;
		}
		break;
	};
}

// Define the curves that must be shown by default
void CLoggedDataChart::DefaultCurvesToDisplay()
{
	if (!m_pLD) return;

	switch(m_pLD->GetMode())
	{
	case CLog/*::LogType*/::LOGTYPE_DP:
		m_bDpCurve=true;
		m_bFlowCurve=m_bT1Curve=m_bT2Curve=m_bDTCurve=m_bPowerCurve=false;
		break;
	case CLog/*::LogType*/::LOGTYPE_TEMP:
		m_bT1Curve=m_bT2Curve=true;
		m_bFlowCurve=m_bDpCurve=m_bDTCurve=m_bPowerCurve=false;
		break;
	case CLog/*::LogType*/::LOGTYPE_FLOW:
		m_bFlowCurve=true;
		if (CLog::TOOL_TASCOPE == m_pLD->ToolUsedForMeasure()) m_bDpCurve = true;
		else m_bDpCurve = false;
		m_bT1Curve=m_bT2Curve=m_bDTCurve=m_bPowerCurve=false;
		break;
	case CLog/*::LogType*/::LOGTYPE_DPTEMP:
		m_bDpCurve=m_bT1Curve=m_bT2Curve=true;
		m_bFlowCurve=m_bDTCurve=m_bPowerCurve=false;
		break;		
	case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
		m_bFlowCurve=m_bT1Curve=m_bT2Curve=true;
		if (CLog::TOOL_TASCOPE == m_pLD->ToolUsedForMeasure()) m_bDpCurve = true;
		else m_bDpCurve = false;
		m_bDTCurve=m_bPowerCurve=false;
		break;
	case CLog/*::LogType*/::LOGTYPE_DT:
		m_bT1Curve=m_bT2Curve=m_bDTCurve= true;
		m_bDpCurve =m_bFlowCurve=m_bPowerCurve=false;
		break;
	case CLog/*::LogType*/::LOGTYPE_POWER:
		m_bPowerCurve=m_bFlowCurve=m_bDTCurve= true;
		m_bDpCurve=m_bT1Curve=m_bT2Curve=false;
		break;
	default:
		m_bFlowCurve=m_bDpCurve=m_bT1Curve=m_bT2Curve=m_bDTCurve=m_bPowerCurve=false;
		break;
	};
}

void CLoggedDataChart::DisplayLoggedData(CLog* pLD)
{
	if (!pLD) return;

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	m_pLD = pLD;
	DefaultCurvesToDisplay();

	ClearChart();

	int nData = m_pLD->GetLength();

	// Hide date on x-axis if logging duration is less than one day
	CTime dtBeg,dtEnd;
	m_pLD->GetDateTime(0,dtBeg);
	m_pLD->GetDateTime(nData-1,dtEnd);
	CTimeSpan ts = dtEnd - dtBeg;  // Subtract 2 CTimes
	if (!ts.GetDays())
		m_bDateonAxis = false;

	SetMainParam();

	// Set the header (title) of the chart
//	CString strHeader = "Logged Data: " + CString(m_pLD->GetName()); // !!! Load an IDS here !!!
//	COleVariant var(strHeader);
//	CTChart::GetHeader().GetFont().SetSize(16);
//	CTChart::GetHeader().GetFont().SetBold(true);
//	CTChart::GetHeader().GetText().Add(*(LPCVARIANT)var);


	// Test the logged data mode and set the axis and legend title
	m_iLdMode = m_pLD->GetMode();
	if (!CTChart::GetAspect().GetOpenGL().GetActive())
	{
		CString str;
		switch(m_iLdMode)
		{
		case CLog/*::LogType*/::LOGTYPE_DP:
			AddLDSeries(0); // Dp
			break;
		case CLog/*::LogType*/::LOGTYPE_TEMP:
			AddLDSeries(0); // T1
			AddLDSeries(1); // T2
			break;
		case CLog/*::LogType*/::LOGTYPE_FLOW:
			AddLDSeries(0); // Flow
			AddLDSeries(1); // Dp
			break;
		case CLog/*::LogType*/::LOGTYPE_DPTEMP:
			AddLDSeries(0); // Dp 
			AddLDSeries(1); // T1
			AddLDSeries(2); // T2
			break;		
		case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
			AddLDSeries(0); // Flow
			AddLDSeries(1); // T1
			AddLDSeries(2); // T2
			AddLDSeries(3); // Dp
			break;
		case CLog/*::LogType*/::LOGTYPE_DT:
			AddLDSeries(0); // DT
			AddLDSeries(1); // T1
			AddLDSeries(2); // T2
			break;
		case CLog/*::LogType*/::LOGTYPE_POWER:
			AddLDSeries(0); // Power
			AddLDSeries(1); // Flow
			AddLDSeries(2); // Dp 
			AddLDSeries(3); // DT
			AddLDSeries(4); // T1
			AddLDSeries(5); // T2
			break;
		default:
			break;
		};
	}

	// Draw in the Graph
	SetRedraw();

	// Add a Cursor tool
	VARIANT TheSeries;
	TheSeries.vt=VT_I2;
	TheSeries.intVal=0;

	m_iCursorTool = CTChart::GetTools().Add(tcCursor);
	CTChart::GetTools().GetItems(m_iCursorTool).GetAsTeeCursor().SetSeries(TheSeries);
	CTChart::GetTools().GetItems(m_iCursorTool).GetAsTeeCursor().SetFollowMouse(true);
	CTChart::GetTools().GetItems(m_iCursorTool).GetAsTeeCursor().SetSnap(true);
	CTChart::GetTools().GetItems(m_iCursorTool).GetAsTeeCursor().SetStyle(cssVertical);
	CTChart::GetTools().GetItems(m_iCursorTool).SetActive(m_bCursorTool);
//	CTChart::GetTools().GetItems(m_CursorTool).GetAsTeeCursor().SetXVal(100);	
//	CTChart::GetTools().GetItems(m_CursorTool).GetAsTeeCursor().SetYVal(200);	
//	CTChart::GetTools().GetItems(m_CursorTool).GetAsNearest().SetSeries(TheSeries);
}

// Draw in the Graph
void CLoggedDataChart::SetRedraw()
{
	if (!m_pLD) return;

	// Fill with data
	FillSeries(m_pLD);

	// Show/Hidde curves depending on the check boxes in the Ribbon Bar
	ShowHiddeCurves();

	// In case the Logged data is from a TA-CBI, change the names for the temp
	// because Temp (DpS-T1) does not have any sence for the TA-CBI
	CMFCRibbonCheckBox* pCBLDT1 = pMainFrame->GetRibbonCBLDT1();
	CMFCRibbonCheckBox* pCBLDT2 = pMainFrame->GetRibbonCBLDT2();
	if (CLog::TOOL_TACBI == m_pLD->ToolUsedForMeasure())
	{		
		pCBLDT1->SetText(TASApp.LoadLocalizedString(IDS_RBN_LD_C_T1_CBI));
		pCBLDT2->SetText(TASApp.LoadLocalizedString(IDS_RBN_LD_C_T2_CBI));
		pMainFrame->SetToolTipRibbonElement(pCBLDT1,IDS_RBN_LD_C_T1_CBI_TT);	
		pMainFrame->SetToolTipRibbonElement(pCBLDT2,IDS_RBN_LD_C_T2_CBI_TT);	
	}
	else
	{
		pCBLDT1->SetText(TASApp.LoadLocalizedString(IDS_RBN_LD_C_T1));
		pCBLDT2->SetText(TASApp.LoadLocalizedString(IDS_RBN_LD_C_T2));
		pMainFrame->SetToolTipRibbonElement(pCBLDT1,IDS_RBN_LD_C_T1_TT);	
		pMainFrame->SetToolTipRibbonElement(pCBLDT2,IDS_RBN_LD_C_T2_TT);	
	}
}

void CLoggedDataChart::ToggleCursorTool()
{
	m_bCursorTool = CTChart::GetTools().GetItems(m_iCursorTool).GetActive() ? false : true;
	CTChart::GetTools().GetItems(m_iCursorTool).SetActive(m_bCursorTool);
}

void CLoggedDataChart::Toggle2DZoom()
{
	if (CTChart::GetZoom().GetDirection() == tzdHorizontal)
	{
		m_b2DZoom = true;
		CTChart::GetZoom().SetDirection(tzdBoth);
		CTChart::GetScroll().SetEnable(pmBoth);
	}
	else
	{
		m_b2DZoom = false;
		CTChart::GetZoom().SetDirection(tzdHorizontal);
		CTChart::GetScroll().SetEnable(pmHorizontal);
	};
}

void CLoggedDataChart::ToggleLegend()
{
	m_bLegend = CTChart::GetLegend().GetVisible() ? false : true;
	CTChart::GetLegend().SetVisible(m_bLegend);
}

void CLoggedDataChart::ToggleDateAxis()
{
	if (m_bDateonAxis)
	{
		m_bDateonAxis = false;
		CTChart::GetAxis().GetBottom().GetLabels().SetDateTimeFormat(_T("hh:mm:ss"));
	}
	else
	{
		m_bDateonAxis = true;
		CTChart::GetAxis().GetBottom().GetLabels().SetDateTimeFormat(_T("dd/mm/yyyy hh:mm:ss"));
	};
}

// Check/Uncheck the display of the vertical Flow axis
void CLoggedDataChart::CheckUnCheckFlowAxis()
{
	if (m_bFlowCurve)
		m_bFlowCurve = false;
	else
		m_bFlowCurve = true;

	// Show/Hidde the curve
	ShowHiddeCurves();
}
// Check/Uncheck the display of the vertical Dp axis
void CLoggedDataChart::CheckUnCheckDpAxis()
{
	if (m_bDpCurve)
		m_bDpCurve = false;
	else
		m_bDpCurve = true;

	// Show/Hidde the curve
	ShowHiddeCurves();
}
// Check/Uncheck the display of the vertical T1 axis
void CLoggedDataChart::CheckUnCheckT1Axis()
{
	if (m_bT1Curve)
		m_bT1Curve = false;
	else
		m_bT1Curve = true;

	// Show/Hidde the curve
	ShowHiddeCurves();
}
// Check/Uncheck the display of the vertical T2 axis
void CLoggedDataChart::CheckUnCheckT2Axis()
{
	if (m_bT2Curve)
		m_bT2Curve = false;
	else
		m_bT2Curve = true;

	// Show/Hidde the curve
	ShowHiddeCurves();
}
// Check/Uncheck the display of the vertical DT axis
void CLoggedDataChart::CheckUnCheckDTAxis()
{
	if (m_bDTCurve)
		m_bDTCurve = false;
	else
		m_bDTCurve = true;

	// Show/Hidde the curve
	ShowHiddeCurves();
}
// Check/Uncheck the display of the vertical Power axis
void CLoggedDataChart::CheckUnCheckPowerAxis()
{
	if (m_bPowerCurve)
		m_bPowerCurve = false;
	else
		m_bPowerCurve = true;

	// Show/Hidde the curve
	ShowHiddeCurves();
}

void CLoggedDataChart::ShowHiddeCurves()
{
	// Variables
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	TCHAR UnitName[_MAXCHARS];
	CString str;

	// Define the position of axis to use the all
	// height that exist
	CTChart::GetAxis().GetLeft().SetStartPosition(0);
	CTChart::GetAxis().GetLeft().SetEndPosition(100);
	CTChart::GetAxis().GetLeft().SetAutomatic(TRUE);
	CTChart::GetAxis().GetRight().SetStartPosition(0);
	CTChart::GetAxis().GetRight().SetEndPosition(100);
	CTChart::GetAxis().GetRight().SetAutomatic(TRUE);
	m_CustAxis1.SetStartPosition(0);
	m_CustAxis1.SetEndPosition(100);
	m_CustAxis1.SetAutomatic(TRUE);
	m_CustAxis2.SetStartPosition(0);
	m_CustAxis2.SetEndPosition(100);
	m_CustAxis2.SetAutomatic(TRUE);
	m_CustAxis3.SetStartPosition(0);
	m_CustAxis3.SetEndPosition(100);
	m_CustAxis3.SetAutomatic(TRUE);
	m_CustAxis4.SetStartPosition(0);
	m_CustAxis4.SetEndPosition(100);
	m_CustAxis4.SetAutomatic(TRUE);
	
	// Show/Hidde the curve
	switch(m_iLdMode)
	{
	case CLog/*::LogType*/::LOGTYPE_DP:
		{
			CTChart::Series(eLDP_Dp).SetActive(m_bDpCurve);

			// Define the axis name and the legend name
			str=TASApp.LoadLocalizedString(IDS_LDAXISTITLEDP);
			CTChart::Series(eLDP_Dp).SetTitle(str);
			GetNameOf(pUnitDB->GetDefaultUnit(_U_DIFFPRESS),UnitName);
			str += (CString)_T(" [") + UnitName + _T("]");
			CTChart::GetAxis().GetLeft().GetTitle().SetCaption(str);

			if (m_bDpCurve)
			{
				CTChart::Series(eLDP_Dp).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDP_Dp).GetColor());
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinDp,m_dMaxDp);
			}
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_TEMP:
		{
			CTChart::Series(eLT_T1).SetActive(m_bT1Curve);
			CTChart::Series(eLT_T2).SetActive(m_bT2Curve);

			// Define the axis name and the legend name
			if (CLog::TOOL_TACBI == m_pLD->ToolUsedForMeasure())
				str=TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			else
				str=TASApp.LoadLocalizedString(IDS_CHART_T1);
			CTChart::Series(eLT_T1).SetTitle(str);
			str=TASApp.LoadLocalizedString(IDS_CHART_T2);
			CTChart::Series(eLT_T2).SetTitle(str);
			
			GetNameOf(pUnitDB->GetDefaultUnit(_U_TEMPERATURE),UnitName);
			str = TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			str += (CString)_T(" [") + UnitName + _T("]");
			CTChart::GetAxis().GetLeft().GetTitle().SetCaption(str);

			if (m_bT1Curve && !m_bT2Curve)
			{
				CTChart::Series(eLT_T1).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLT_T1).GetColor());
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT1,m_dMaxT1,true);
			}
			else if (!m_bT1Curve && m_bT2Curve)
			{
				CTChart::Series(eLT_T2).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLT_T2).GetColor());
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT2,m_dMaxT2,true);
			}			
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_FLOW:
		{
			CTChart::Series(eLF_Flow).SetActive(m_bFlowCurve);
			CTChart::Series(eLF_Dp).SetActive(m_bDpCurve);

			// Define the axis name and the legend name
			CString strQ=TASApp.LoadLocalizedString(IDS_LDAXISTITLEFLOW);
			CTChart::Series(eLF_Flow).SetTitle(strQ);
			CString strQU=strQ;
			GetNameOf(pUnitDB->GetDefaultUnit(_U_FLOW),UnitName);
			strQU += (CString)_T(" [") + UnitName + _T("]");
			
			CString strDp=TASApp.LoadLocalizedString(IDS_LDAXISTITLEDP);
			CTChart::Series(eLF_Dp).SetTitle(strDp);
			CString strDpU=strDp;
			GetNameOf(pUnitDB->GetDefaultUnit(_U_DIFFPRESS),UnitName);
			strDpU += (CString)_T(" [") + UnitName + _T("]");
			
			if (m_bFlowCurve && m_bDpCurve)// Flow and Dp
			{
				CTChart::Series(eLF_Flow).SetVerticalAxis(0);
				CTChart::Series(eLF_Dp).SetVerticalAxis(1);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLF_Flow).GetColor());
				CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLF_Dp).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQU);
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strDpU);
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
				RedefineAxisScale(&CTChart::GetAxis().GetRight(),m_dMinDp,m_dMaxDp);
			}
			else if (m_bFlowCurve && !m_bDpCurve) // Flow only
			{
				CTChart::Series(eLF_Flow).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLF_Flow).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQU);
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
			}
			else if (!m_bFlowCurve && m_bDpCurve) // Dp only
			{
				CTChart::Series(eLF_Dp).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLF_Dp).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDpU);
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinDp,m_dMaxDp);
			}
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_DPTEMP:
		{
			CTChart::Series(eLDPT_Dp).SetActive(m_bDpCurve);
			CTChart::Series(eLDPT_T1).SetActive(m_bT1Curve);
			CTChart::Series(eLDPT_T2).SetActive(m_bT2Curve);

			// Define the axis name and the legend name
			CString strDp=TASApp.LoadLocalizedString(IDS_LDAXISTITLEDP);
			CTChart::Series(eLDPT_Dp).SetTitle(strDp);
			if (CLog::TOOL_TACBI == m_pLD->ToolUsedForMeasure())
				str=TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			else
				str=TASApp.LoadLocalizedString(IDS_CHART_T1);
			CTChart::Series(eLDPT_T1).SetTitle(str);
			str=TASApp.LoadLocalizedString(IDS_CHART_T2);
			CTChart::Series(eLDPT_T2).SetTitle(str);

			GetNameOf(pUnitDB->GetDefaultUnit(_U_DIFFPRESS),UnitName);
			strDp += (CString)_T(" [") + UnitName + _T("]");
					
			GetNameOf(pUnitDB->GetDefaultUnit(_U_TEMPERATURE),UnitName);
			CString strTemp = TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			strTemp += (CString)_T(" [") + UnitName + _T("]");
			
			if (m_bDpCurve && m_bT1Curve && !m_bT2Curve) // Dp and T1
			{
				CTChart::Series(eLDPT_Dp).SetVerticalAxis(0);
				CTChart::Series(eLDPT_T1).SetVerticalAxis(1);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDPT_Dp).GetColor());
				CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLDPT_T1).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDp);
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinDp,m_dMaxDp);
				RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT1,m_dMaxT1,true);
			}
			else if (m_bDpCurve && !m_bT1Curve && m_bT2Curve) // Dp and T2
			{
				CTChart::Series(eLDPT_Dp).SetVerticalAxis(0);
				CTChart::Series(eLDPT_T2).SetVerticalAxis(1);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDPT_Dp).GetColor());
				CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLDPT_T2).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDp);
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinDp,m_dMaxDp);
				RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT2,m_dMaxT2,true);
			}
			else if (m_bDpCurve && !(m_bT1Curve||m_bT2Curve)) // Dp only
			{
				CTChart::Series(eLDPT_Dp).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDPT_Dp).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDp);
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinDp,m_dMaxDp);
			}
			else if (!m_bDpCurve && (m_bT1Curve||m_bT2Curve)) // Temp only
			{
				if (m_bT1Curve)
				{
					CTChart::Series(eLDPT_T1).SetVerticalAxis(0);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDPT_T1).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT1,m_dMaxT1,true);
				}
				else
				{
					CTChart::Series(eLDPT_T2).SetVerticalAxis(0);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDPT_T2).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT2,m_dMaxT2,true);
				}
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strTemp);
			}
		}
		break;		
	case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
		{
			CTChart::Series(eLFT_Flow).SetActive(m_bFlowCurve);
			CTChart::Series(eLFT_T1).SetActive(m_bT1Curve);
			CTChart::Series(eLFT_T2).SetActive(m_bT2Curve);
			CTChart::Series(eLFT_Dp).SetActive(m_bDpCurve);

			// Define the axis name and the legend name
			CString strQ=TASApp.LoadLocalizedString(IDS_LDAXISTITLEFLOW);
			CTChart::Series(eLFT_Flow).SetTitle(strQ);
			if (CLog::TOOL_TACBI == m_pLD->ToolUsedForMeasure())
				str=TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			else
				str=TASApp.LoadLocalizedString(IDS_CHART_T1);
			CTChart::Series(eLFT_T1).SetTitle(str);
			str=TASApp.LoadLocalizedString(IDS_CHART_T2);
			CTChart::Series(eLFT_T2).SetTitle(str);
			CString strDp=TASApp.LoadLocalizedString(IDS_LDAXISTITLEDP);
			CTChart::Series(eLFT_Dp).SetTitle(strDp);

			GetNameOf(pUnitDB->GetDefaultUnit(_U_FLOW),UnitName);
			strQ += (CString)_T(" [") + UnitName + _T("]");
					
			GetNameOf(pUnitDB->GetDefaultUnit(_U_DIFFPRESS),UnitName);
			strDp += (CString)_T(" [") + UnitName + _T("]");
			
			GetNameOf(pUnitDB->GetDefaultUnit(_U_TEMPERATURE),UnitName);
			CString strTemp = TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			strTemp += (CString)_T(" [") + UnitName + _T("]");
			
			if (m_bFlowCurve && m_bDpCurve && (m_bT1Curve||m_bT2Curve))
			{
				CTChart::Series(eLFT_Flow).SetVerticalAxis(0);
				CTChart::Series(eLFT_Dp).SetVerticalAxisCustom(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_Flow).GetColor());
				m_CustAxis1.GetAxisPen().SetColor(CTChart::Series(eLFT_Dp).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQ);
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
				RedefineAxisScale(&m_CustAxis1,m_dMinDp,m_dMaxDp);
				
				m_CustAxis1.GetTitle().SetCaption(strDp);
				if (m_bT1Curve)
				{
					CTChart::Series(eLFT_T1).SetVerticalAxis(1);
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLFT_T1).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT1,m_dMaxT1,true);
				}
				else
				{
					CTChart::Series(eLFT_T2).SetVerticalAxis(1);
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLFT_T2).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT2,m_dMaxT2,true);
				}
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
			}
			else if (!m_bFlowCurve && m_bDpCurve && (m_bT1Curve||m_bT2Curve))
			{
				CTChart::Series(eLFT_Dp).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_Dp).GetColor());
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinDp,m_dMaxDp);
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDp);
				if (m_bT1Curve)
				{
					CTChart::Series(eLFT_T1).SetVerticalAxis(1);
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLFT_T1).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT1,m_dMaxT1,true);
				}
				else
				{
					CTChart::Series(eLFT_T2).SetVerticalAxis(1);
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLFT_T2).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT2,m_dMaxT2,true);
				}
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
			}
			else if (m_bFlowCurve && !m_bDpCurve && (m_bT1Curve||m_bT2Curve))
			{
				CTChart::Series(eLFT_Flow).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_Flow).GetColor());
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQ);
				if (m_bT1Curve)
				{
					CTChart::Series(eLFT_T1).SetVerticalAxis(1);
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLFT_T1).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT1,m_dMaxT1,true);
				}
				else
				{
					CTChart::Series(eLFT_T2).SetVerticalAxis(1);
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLFT_T2).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT2,m_dMaxT2,true);
				}
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
			}
			else if (m_bFlowCurve && m_bDpCurve && !(m_bT1Curve||m_bT2Curve))
			{
				CTChart::Series(eLFT_Flow).SetVerticalAxis(0);
				CTChart::Series(eLFT_Dp).SetVerticalAxis(1);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_Flow).GetColor());
				CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLFT_Dp).GetColor());
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
				RedefineAxisScale(&CTChart::GetAxis().GetRight(),m_dMinDp,m_dMaxDp);
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQ);
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strDp);
			}
			else if (!m_bFlowCurve && !m_bDpCurve && (m_bT1Curve||m_bT2Curve))
			{
				if (m_bT1Curve)
				{
					CTChart::Series(eLFT_T1).SetVerticalAxis(0);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_T1).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT1,m_dMaxT1,true);
				}
				else
				{
					CTChart::Series(eLFT_T2).SetVerticalAxis(0);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_T2).GetColor());
					RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT2,m_dMaxT2,true);
				}
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strTemp);
			}
			else if (!m_bFlowCurve && m_bDpCurve && !(m_bT1Curve||m_bT2Curve))
			{
				CTChart::Series(eLFT_Dp).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_Dp).GetColor());
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinDp,m_dMaxDp);
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDp);
			}
			else if (m_bFlowCurve && !m_bDpCurve && !(m_bT1Curve||m_bT2Curve))
			{
				CTChart::Series(eLFT_Flow).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLFT_Flow).GetColor());
				RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQ);
			}
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_DT:
		{
			CTChart::Series(eLDT_DT).SetActive(m_bDTCurve);
			CTChart::Series(eLDT_T1).SetActive(m_bT1Curve);
			CTChart::Series(eLDT_T2).SetActive(m_bT2Curve);
			
			// Define the axis name and the legend name
			CString strDt=TASApp.LoadLocalizedString(IDS_DIFFERENTIALTEMPERATURE);
			CTChart::Series(eLDT_DT).SetTitle(strDt);
			if (CLog::TOOL_TACBI == m_pLD->ToolUsedForMeasure())
				str=TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			else
				str=TASApp.LoadLocalizedString(IDS_CHART_T1);
			CTChart::Series(eLDT_T1).SetTitle(str);
			str=TASApp.LoadLocalizedString(IDS_CHART_T2);
			CTChart::Series(eLDT_T2).SetTitle(str);
			
			GetNameOf(pUnitDB->GetDefaultUnit(_U_DIFFTEMP),UnitName);
			strDt += (CString)_T(" [") + UnitName + _T("]");
			GetNameOf(pUnitDB->GetDefaultUnit(_U_TEMPERATURE),UnitName);
			CString strTemp = TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			strTemp += (CString)_T(" [") + UnitName + _T("]");

			if (m_bDTCurve && m_bT1Curve && m_bT2Curve) // DT and T1 and T2
			{
				CTChart::Series(eLDT_DT).SetVerticalAxis(0);
				CTChart::Series(eLDT_T1).SetVerticalAxis(1);
				CTChart::Series(eLDT_T2).SetVerticalAxisCustom(1);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDT_DT).GetColor());
				CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLDT_T1).GetColor());
				m_CustAxis2.GetAxisPen().SetColor(CTChart::Series(eLDT_T2).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDt);
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
				// Custom the m_CustAxis2 axis to view only the color
				RedefineCustomT1T2Axis(&(CTChart::GetAxis().GetRight()),&m_CustAxis2);
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinDT,m_dMaxDT,true);
			}
			if (!m_bDTCurve && m_bT1Curve && m_bT2Curve) // T1 and T2
			{
				CTChart::Series(eLDT_T1).SetVerticalAxis(0);
				CTChart::Series(eLDT_T2).SetVerticalAxisCustom(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDT_T1).GetColor());
				m_CustAxis1.GetAxisPen().SetColor(CTChart::Series(eLDT_T2).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strTemp);
				// Custom the m_CustAxis1 axis to view only the color
				RedefineCustomT1T2Axis(&(CTChart::GetAxis().GetLeft()),&m_CustAxis1);
			}
			if (m_bDTCurve && !m_bT1Curve && m_bT2Curve) // DT and T2
			{
				CTChart::Series(eLDT_DT).SetVerticalAxis(0);
				CTChart::Series(eLDT_T2).SetVerticalAxis(1);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDT_DT).GetColor());
				CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLDT_T2).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDt);
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
				RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT2,m_dMaxT2,true);
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinDT,m_dMaxDT,true);
			}
			if (m_bDTCurve && m_bT1Curve && !m_bT2Curve) // DT and T1
			{
				CTChart::Series(eLDT_DT).SetVerticalAxis(0);
				CTChart::Series(eLDT_T1).SetVerticalAxis(1);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDT_DT).GetColor());
				CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLDT_T1).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDt);
				CTChart::GetAxis().GetRight().GetTitle().SetCaption(strTemp);
				RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinT1,m_dMaxT1,true);
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinDT,m_dMaxDT,true);
			}
			if (!m_bDTCurve && !m_bT1Curve && m_bT2Curve) // T2
			{
				CTChart::Series(eLDT_T2).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDT_T2).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strTemp);
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT2,m_dMaxT2,true);
			}
			if (!m_bDTCurve && m_bT1Curve && !m_bT2Curve) // T1
			{
				CTChart::Series(eLDT_T1).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDT_T1).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strTemp);
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinT1,m_dMaxT1,true);
			}
			if (m_bDTCurve && !m_bT1Curve && !m_bT2Curve) // DT
			{
				CTChart::Series(eLDT_DT).SetVerticalAxis(0);
				CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLDT_DT).GetColor());
				CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDt);
				RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinDT,m_dMaxDT,true);
			}
		}
		break;
	case CLog/*::LogType*/::LOGTYPE_POWER:
		{
			CTChart::Series(eLP_Power).SetActive(m_bPowerCurve);
			CTChart::Series(eLP_Flow).SetActive(m_bFlowCurve);
			CTChart::Series(eLP_Dp).SetActive(m_bDpCurve);
			CTChart::Series(eLP_DT).SetActive(m_bDTCurve);
			CTChart::Series(eLP_T1).SetActive(m_bT1Curve);
			CTChart::Series(eLP_T2).SetActive(m_bT2Curve);

			// Define the axis name and the legend name
			CString strP=TASApp.LoadLocalizedString(IDS_POWER);
			CTChart::Series(eLP_Power).SetTitle(strP);
			CString strQ=TASApp.LoadLocalizedString(IDS_LDAXISTITLEFLOW);
			CTChart::Series(eLP_Flow).SetTitle(strQ);
			CString strDp=TASApp.LoadLocalizedString(IDS_LDAXISTITLEDP);
			CTChart::Series(eLP_Dp).SetTitle(strDp);
			CString strDt=TASApp.LoadLocalizedString(IDS_DIFFERENTIALTEMPERATURE);
			CTChart::Series(eLP_DT).SetTitle(strDt);
			if (CLog::TOOL_TACBI == m_pLD->ToolUsedForMeasure())
				str=TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			else
				str=TASApp.LoadLocalizedString(IDS_CHART_T1);
			CTChart::Series(eLP_T1).SetTitle(str);
			str=TASApp.LoadLocalizedString(IDS_CHART_T2);
			CTChart::Series(eLP_T2).SetTitle(str);

			GetNameOf(pUnitDB->GetDefaultUnit(_U_TH_POWER),UnitName);
			strP += (CString)_T(" [") + UnitName + _T("]");
			GetNameOf(pUnitDB->GetDefaultUnit(_U_FLOW),UnitName);
			strQ += (CString)_T(" [") + UnitName + _T("]");
			GetNameOf(pUnitDB->GetDefaultUnit(_U_DIFFPRESS),UnitName);
			strDp += (CString)_T(" [") + UnitName + _T("]");
			GetNameOf(pUnitDB->GetDefaultUnit(_U_DIFFTEMP),UnitName);
			strDt += (CString)_T(" [") + UnitName + _T("]");					
			GetNameOf(pUnitDB->GetDefaultUnit(_U_TEMPERATURE),UnitName);
			CString strTemp = TASApp.LoadLocalizedString(IDS_TEMPERATURE);
			strTemp += (CString)_T(" [") + UnitName + _T("]");

			// Two graphs must be displayed
			if ((m_bPowerCurve||m_bFlowCurve||m_bDTCurve) && (m_bDpCurve || m_bT1Curve || m_bT2Curve))
			{
				CTChart::GetAxis().GetLeft().SetStartPosition(0);
				CTChart::GetAxis().GetLeft().SetEndPosition(m_dAlpha);
				CTChart::GetAxis().GetRight().SetStartPosition(0);
				CTChart::GetAxis().GetRight().SetEndPosition(m_dAlpha);
				m_CustAxis1.SetStartPosition(0);
				m_CustAxis1.SetEndPosition(m_dAlpha);
				m_CustAxis2.SetStartPosition(m_dAlpha);
				m_CustAxis2.SetEndPosition(100);
				m_CustAxis3.SetStartPosition(m_dAlpha);
				m_CustAxis3.SetEndPosition(100);
				m_CustAxis4.SetStartPosition(m_dAlpha);
				m_CustAxis4.SetEndPosition(100);
			}

			if (m_bPowerCurve||m_bFlowCurve||m_bDTCurve) // Upper graph must be shown
			{
				if (m_bPowerCurve && m_bFlowCurve && m_bDTCurve) // Power and Flow and DT
				{
					CTChart::Series(eLP_Power).SetVerticalAxis(0);
					CTChart::Series(eLP_Flow).SetVerticalAxisCustom(0);
					CTChart::Series(eLP_DT).SetVerticalAxis(1);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLP_Power).GetColor());
					m_CustAxis1.GetAxisPen().SetColor(CTChart::Series(eLP_Flow).GetColor());
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLP_DT).GetColor());
					RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinPower,m_dMaxPower);
					RedefineAxisScale(&m_CustAxis1,m_dMinFlow,m_dMaxFlow);
					RedefineAxisScale(&CTChart::GetAxis().GetRight(),m_dMinDT,m_dMaxDT,true);
					CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strP);
					m_CustAxis1.GetTitle().SetCaption(strQ);
					CTChart::GetAxis().GetRight().GetTitle().SetCaption(strDt);
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinDT,m_dMaxDT,true);
				}
				else if (!m_bPowerCurve && m_bFlowCurve && m_bDTCurve) // Flow and DT
				{
					CTChart::Series(eLP_Flow).SetVerticalAxis(0);
					CTChart::Series(eLP_DT).SetVerticalAxis(1);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLP_Flow).GetColor());
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLP_DT).GetColor());
					CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQ);
					CTChart::GetAxis().GetRight().GetTitle().SetCaption(strDt);
					RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinDT,m_dMaxDT,true);
				}
				else if (m_bPowerCurve && !m_bFlowCurve && m_bDTCurve) // Power and DT
				{
					CTChart::Series(eLP_Power).SetVerticalAxis(0);
					CTChart::Series(eLP_DT).SetVerticalAxis(1);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLP_Power).GetColor());
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLP_DT).GetColor());
					CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strP);
					CTChart::GetAxis().GetRight().GetTitle().SetCaption(strDt);
					RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinPower,m_dMaxPower);
					RedefineAxisScale(&(CTChart::GetAxis().GetRight()),m_dMinDT,m_dMaxDT,true);
				}
				else if (m_bPowerCurve && m_bFlowCurve && !m_bDTCurve) // Power and Flow
				{
					CTChart::Series(eLP_Power).SetVerticalAxis(0);
					CTChart::Series(eLP_Flow).SetVerticalAxis(1);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLP_Power).GetColor());
					CTChart::GetAxis().GetRight().GetAxisPen().SetColor(CTChart::Series(eLP_Flow).GetColor());
					CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strP);
					CTChart::GetAxis().GetRight().GetTitle().SetCaption(strQ);
					RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinPower,m_dMaxPower);
					RedefineAxisScale(&CTChart::GetAxis().GetRight(),m_dMinFlow,m_dMaxFlow);
				}
				else if (!m_bPowerCurve && !m_bFlowCurve && m_bDTCurve) // DT only
				{
					CTChart::Series(eLP_DT).SetVerticalAxis(0);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLP_DT).GetColor());
					CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strDt);
					RedefineAxisScale(&(CTChart::GetAxis().GetLeft()),m_dMinDT,m_dMaxDT,true);
				}
				else if (!m_bPowerCurve && m_bFlowCurve && !m_bDTCurve) // Flow only
				{
					CTChart::Series(eLP_Flow).SetVerticalAxis(0);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLP_Flow).GetColor());
					CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strQ);
					RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinFlow,m_dMaxFlow);
				}
				else if (m_bPowerCurve && !m_bFlowCurve && !m_bDTCurve) // Power only
				{
					CTChart::Series(eLP_Power).SetVerticalAxis(0);
					CTChart::GetAxis().GetLeft().GetAxisPen().SetColor(CTChart::Series(eLP_Power).GetColor());
					CTChart::GetAxis().GetLeft().GetTitle().SetCaption(strP);
					RedefineAxisScale(&CTChart::GetAxis().GetLeft(),m_dMinPower,m_dMaxPower);
				}

			}
			if (m_bDpCurve || m_bT1Curve || m_bT2Curve) // Lower graph must be shown
			{
				if (m_bDpCurve && m_bT1Curve && m_bT2Curve) // Dp and T1 and T2
				{
					CTChart::Series(eLP_Dp).SetVerticalAxisCustom(2);
					CTChart::Series(eLP_T1).SetVerticalAxisCustom(1);
					CTChart::Series(eLP_T2).SetVerticalAxisCustom(3);
					m_CustAxis3.GetAxisPen().SetColor(CTChart::Series(eLP_Dp).GetColor());
					m_CustAxis2.GetAxisPen().SetColor(CTChart::Series(eLP_T1).GetColor());
					m_CustAxis4.GetAxisPen().SetColor(CTChart::Series(eLP_T2).GetColor());
					m_CustAxis3.GetTitle().SetCaption(strDp);
					m_CustAxis4.GetTitle().SetCaption(strTemp);
					RedefineAxisScale(&m_CustAxis3,m_dMinDp,m_dMaxDp);
					RedefineCustomT1T2Axis(&m_CustAxis4,&m_CustAxis2);
				}
				else if (m_bDpCurve && m_bT1Curve && !m_bT2Curve) // Dp and T1
				{
					CTChart::Series(eLP_Dp).SetVerticalAxisCustom(2);
					CTChart::Series(eLP_T1).SetVerticalAxisCustom(3);
					m_CustAxis3.GetAxisPen().SetColor(CTChart::Series(eLP_Dp).GetColor());
					m_CustAxis4.GetAxisPen().SetColor(CTChart::Series(eLP_T1).GetColor());
					m_CustAxis3.GetTitle().SetCaption(strDp);
					m_CustAxis4.GetTitle().SetCaption(strTemp);
					RedefineAxisScale(&m_CustAxis3,m_dMinDp,m_dMaxDp);
					RedefineAxisScale(&m_CustAxis4,m_dMinT1,m_dMaxT1,true);
				}
				else if (m_bDpCurve && !m_bT1Curve && m_bT2Curve) // Dp and T2
				{
					CTChart::Series(eLP_Dp).SetVerticalAxisCustom(2);
					CTChart::Series(eLP_T2).SetVerticalAxisCustom(3);
					m_CustAxis3.GetAxisPen().SetColor(CTChart::Series(eLP_Dp).GetColor());
					m_CustAxis4.GetAxisPen().SetColor(CTChart::Series(eLP_T2).GetColor());
					m_CustAxis3.GetTitle().SetCaption(strDp);
					m_CustAxis4.GetTitle().SetCaption(strTemp);
					RedefineAxisScale(&m_CustAxis3,m_dMinDp,m_dMaxDp);
					RedefineAxisScale(&m_CustAxis4,m_dMinT2,m_dMaxT2,true);
				}
				else if (m_bDpCurve && !(m_bT1Curve||m_bT2Curve)) // Dp only
				{
					CTChart::Series(eLP_Dp).SetVerticalAxisCustom(2);
					m_CustAxis3.GetAxisPen().SetColor(CTChart::Series(eLP_Dp).GetColor());
					m_CustAxis3.GetTitle().SetCaption(strDp);
					RedefineAxisScale(&m_CustAxis3,m_dMinDp,m_dMaxDp);
				}
				else if (!m_bDpCurve && (m_bT1Curve||m_bT2Curve)) // Temp only
				{
					if (m_bT1Curve && m_bT2Curve)
					{
						CTChart::Series(eLP_T1).SetVerticalAxisCustom(1);
						CTChart::Series(eLP_T2).SetVerticalAxisCustom(3);
						m_CustAxis2.GetAxisPen().SetColor(CTChart::Series(eLP_T1).GetColor());
						m_CustAxis4.GetAxisPen().SetColor(CTChart::Series(eLP_T2).GetColor());
						m_CustAxis4.GetTitle().SetCaption(strTemp);
						// Custom the m_CustAxis2 axis to view only the color
						RedefineCustomT1T2Axis(&m_CustAxis4,&m_CustAxis2);
					}
					else if (m_bT1Curve && !m_bT2Curve)
					{
						CTChart::Series(eLP_T1).SetVerticalAxisCustom(2);
						m_CustAxis3.GetAxisPen().SetColor(CTChart::Series(eLP_T1).GetColor());
						m_CustAxis3.GetTitle().SetCaption(strTemp);
						RedefineAxisScale(&m_CustAxis3,m_dMinT1,m_dMaxT1,true);
					}
					else
					{
						CTChart::Series(eLP_T2).SetVerticalAxisCustom(2);
						m_CustAxis3.GetAxisPen().SetColor(CTChart::Series(eLP_T2).GetColor());
						m_CustAxis3.GetTitle().SetCaption(strTemp);
						RedefineAxisScale(&m_CustAxis3,m_dMinT2,m_dMaxT2,true);
					}					
				}
			}
		}
		break;
	};

	// Define the two main curves to be not Automatic
	// That will avoid rescal problem when we want 
	// change the scale for temperature
	CTChart::GetAxis().GetLeft().SetAutomatic(FALSE);
	CTChart::GetAxis().GetRight().SetAutomatic(FALSE);
}
void CLoggedDataChart::ZoomIn1D()
{
	long lLeft = CTChart::GetLeft();
	long lWidth = CTChart::GetWidth();
	long lTop = CTChart::GetAxis().GetLeft().CalcYPosValue(CTChart::GetAxis().GetLeft().GetMaximum());
	long lBottom = CTChart::GetAxis().GetLeft().CalcYPosValue(CTChart::GetAxis().GetLeft().GetMinimum());
	
	CTChart::GetZoom().ZoomRect(lLeft+(lWidth/5),lTop,lLeft+lWidth-(lWidth/5),lBottom);
}

void CLoggedDataChart::ZoomOut1D()
{
	long lLeft = CTChart::GetLeft();
	long lWidth = CTChart::GetWidth();
	long lTop = CTChart::GetAxis().GetLeft().CalcYPosValue(CTChart::GetAxis().GetLeft().GetMaximum());
	long lBottom = CTChart::GetAxis().GetLeft().CalcYPosValue(CTChart::GetAxis().GetLeft().GetMinimum());
	
	CTChart::GetZoom().ZoomRect(lLeft-(lWidth/5),lTop,lLeft+lWidth+(lWidth/5),lBottom);
}

void CLoggedDataChart::SetMinMax2DZoom(double percentzoom/*=100*/)
{
	// Set series color
	switch(m_iLdMode)
	{
	case CLog/*::LogType*/::LOGTYPE_FLOWTEMP:
		SetMinMax2DZoomCustAxis(&m_CustAxis1,percentzoom);
		SetMinMax2DZoomCustAxis(&m_CustAxis2,percentzoom);
		break;
	case CLog/*::LogType*/::LOGTYPE_DT:
		SetMinMax2DZoomCustAxis(&m_CustAxis1,percentzoom);
		SetMinMax2DZoomCustAxis(&m_CustAxis2,percentzoom);
		break;
	case CLog/*::LogType*/::LOGTYPE_POWER:
		SetMinMax2DZoomCustAxis(&m_CustAxis1,percentzoom);
		SetMinMax2DZoomCustAxis(&m_CustAxis2,percentzoom);
		SetMinMax2DZoomCustAxis(&m_CustAxis3,percentzoom);
		SetMinMax2DZoomCustAxis(&m_CustAxis4,percentzoom);		
		break;
	};
}

void CLoggedDataChart::SetMinMax2DZoomCustAxis(CAxis* pCurAxis, double percentzoom)
{
	// OffSet
	double dOf7=percentzoom-100;
	
	// Define the % of the full scale to be shown
	double dPercentFullScale = (100-(percentzoom+dOf7-100))/100;

	// Define the height
	double dMax = pCurAxis->GetMaximum();
	double dMin = pCurAxis->GetMinimum();
	double dHeight = dMax-dMin;
	
	// Define the new Height
	double dNewHeight = dHeight;
		
	// Set the new scale
	dNewHeight *= dPercentFullScale;
	
	// Define the Scale factor
	double dFactor = (dHeight-dNewHeight)/2;

	// Do not use automatic scale
	pCurAxis->SetAutomatic(FALSE);
	
	// Generate default min/max value 
	// to avoid afxMessage that are not correct
	pCurAxis->SetMinimum(-999999);
	pCurAxis->SetMaximum(999999);

	pCurAxis->SetMinimum(dMin+dFactor);
	pCurAxis->SetMaximum(dMax-dFactor);
}

// Hide values for the custom axis and show only for first axis
void CLoggedDataChart::RedefineCustomT1T2Axis(CAxis *pAxis, CAxis *pCustAxis)
{
	// Variables
	double dCeilAxis = 0.0;
	double dFloorAxis = 0.0;

	// Redefine the Offset
	m_dOff7 = 0.05*(max(m_dMaxT1,m_dMaxT2)-min(m_dMinT1,m_dMinT2));

	// Hide redondant information
	pCustAxis->GetTicks().SetVisible(FALSE);
	pCustAxis->GetLabels().SetVisible(FALSE);

	// Do not use automatic scale
	pAxis->SetAutomatic(FALSE);
	pCustAxis->SetAutomatic(FALSE);

	// Generate default min/max value 
	// to avoid afxMessage that are not correct
	pAxis->SetMinimum(-999999);
	pAxis->SetMaximum(999999);
	pCustAxis->SetMinimum(-999999);
	pCustAxis->SetMaximum(999999);

	// Verify if all values are positive.
	// In that case, do not show axis below 0
	// Otherwhize accept it.
	if (m_dMinT1>=0 && m_dMinT2>=0 && (m_dMinT1-m_dOff7<0 || m_dMinT2-m_dOff7<0)) dFloorAxis = 0.0;
	else dFloorAxis = min(m_dMinT1-m_dOff7,m_dMinT2-m_dOff7);
	dCeilAxis = max(m_dMaxT1+m_dOff7,m_dMaxT2+m_dOff7);

	dFloorAxis = floor(dFloorAxis);
	dCeilAxis = ceil(dCeilAxis);

	pAxis->SetMinimum(dFloorAxis);
	pCustAxis->SetMinimum(dFloorAxis);
	pAxis->SetMaximum(dCeilAxis);
	pCustAxis->SetMaximum(dCeilAxis);
}

// Redefine the upper and lower limit for an axis scale
void CLoggedDataChart::RedefineAxisScale(CAxis *pAxis,double dMin, double dMax, bool bTemp/*=false*/)
{
	// Variables
	double dCeilAxis = 0.0;
	double dFloorAxis = 0.0;

	// Redefine the Offset
	m_dOff7 = 0.05*(dMax-dMin);
	
	// Do not use automatic scale
	pAxis->SetAutomatic(FALSE);

	// Generate default min/max value 
	// to avoid afxMessage that are not correct
	pAxis->SetMinimum(-999999);
	pAxis->SetMaximum(999999);

	// Verify if all values are positive.
	// In that case, do not show axis below 0
	// Otherwhize accept it.
	if (dMin-m_dOff7<0 && dMin>=0) dFloorAxis = 0.0;
	else dFloorAxis = dMin-m_dOff7;
	dCeilAxis = dMax+m_dOff7;

	// In case we redefine a temperature scale
	// Increase the scale to avoid little variation
	// that takes all the graph
	if (bTemp)
	{
		dFloorAxis = floor(dFloorAxis);
		dCeilAxis = ceil(dCeilAxis);
	}

	pAxis->SetMinimum(dFloorAxis);
	pAxis->SetMaximum(dCeilAxis);
}

// Toggle on and off the display of symbols on curves
void CLoggedDataChart::ToggleSymbols()
{
	// Invert the Selection
	if (m_bSymbols)	m_bSymbols=false;
	else m_bSymbols=true;
	
	// Count the Curves number and refresh them
	long lCount = CTChart::GetSeriesCount();
	for (int i=0; i<lCount; i++)
		CTChart::Series(i).GetAsLine().GetPointer().SetVisible(m_bSymbols);
}

//****************************************************************
//	To be removed!!!  Test some interesting functions of TeeChart
//****************************************************************
/*void CLoggedDataChart::TestTeeChartFunctionnalities()
{
	// Link TeeCommander with the chart
//	m_Commander1.SetChartLink(CTChart::GetChartLink());

	//=====================
	// Add a mark tips tool
	//=====================
	VARIANT TheSeries;
	TheSeries.vt=VT_I2;
	TheSeries.intVal=1;

	int m_MarkTipsTool = CTChart::GetTools().Add(tcMarksTip);
	CTChart::GetTools().GetItems(m_MarkTipsTool).SetActive(true);
	CTChart::GetTools().GetItems(m_MarkTipsTool).GetAsMarksTip().SetDelay(100);
	CTChart::GetTools().GetItems(m_MarkTipsTool).GetAsMarksTip().SetMouseAction(mtmMove);
//	CTChart::GetTools().GetItems(m_MarkTipsTool).GetAsMarksTip().SetMouseAction(mtmClick);
	CTChart::GetTools().GetItems(m_MarkTipsTool).GetAsMarksTip().SetSeries(TheSeries);
	CTChart::GetTools().GetItems(m_MarkTipsTool).GetAsMarksTip().SetStyle(smsXY);

	// Disable Chart Paging
	//	CTChart::GetPage().SetMaxPointsPerPage(0);

	  // Drop an Excel file
	  CTChart::GetExport().GetAsXLS().SetIncludeHeader(true);
	  CTChart::GetExport().GetAsXLS().SetIncludeLabels(true);
	  CTChart::GetExport().GetAsXLS().SetIncludeIndex(true);
	  CTChart::GetExport().GetAsXLS().SaveToFile("chart.xls");

}
*/