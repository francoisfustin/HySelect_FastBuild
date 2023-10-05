#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"

#include "TeeChartDefines.h"
#include "Export.h"
#include "Legend.h"
#include "Canvas.h"
#include "ChartFont.h"
#include "Zoom.h"
#include "Printer.h"
#include "DlgExportChart.h"

#include "Utilities.h"
#include "RViewLogData.h"
#include "RViewChart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRViewChart *pRViewChart = NULL;

IMPLEMENT_DYNCREATE(CRViewChart, CRScrollView)

CRViewChart::CRViewChart()
{
	m_pLoggedData = NULL;
	m_lLegendLeft = m_lLegendRight = m_lLegendTop = m_lLegendBottom = 0;
	m_fNeedScroll = false;
	pRViewChart = this;
}

CRViewChart::~CRViewChart()
{
	m_brWhiteBrush.DeleteObject();
	pRViewChart = NULL;
}

BEGIN_MESSAGE_MAP(CRViewChart, CRScrollView)
	ON_COMMAND( ID_RBN_LD_CO_TRACKING, OnChartTrack )
	ON_COMMAND( ID_RBN_LD_Z_2D, OnChart2dzoom )
	ON_COMMAND( ID_RBN_LD_CO_DATEAXIS, OnChartDateaxis )
	ON_COMMAND( ID_RBN_LD_CO_EXPORT, OnFileExportChart )
	ON_COMMAND( ID_RBN_LD_CO_LEGEND, OnChartLegend )
	ON_COMMAND( ID_RBN_LD_C_FLOW, OnCheckBoxFlow )
	ON_COMMAND( ID_RBN_LD_C_DP, OnCheckBoxDP )
	ON_COMMAND( ID_RBN_LD_C_T1, OnCheckBoxT1 )
	ON_COMMAND( ID_RBN_LD_C_T2, OnCheckBoxT2 )
	ON_COMMAND( ID_RBN_LD_C_DT, OnCheckBoxDT )
	ON_COMMAND( ID_RBN_LD_C_POWER, OnCheckBoxPower )
	ON_COMMAND( ID_RBN_LD_CO_MARKERS, OnChartSymbols )

	ON_UPDATE_COMMAND_UI( ID_RBN_LD_CO_TRACKING, OnUpdateChartTrack )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_Z_2D, OnUpdateChart2dzoom )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_CO_DATEAXIS, OnUpdateChartDateaxis )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_CO_LEGEND, OnUpdateChartLegend )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_C_FLOW, OnUpdateCheckBoxFlow )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_C_DP, OnUpdateCheckBoxDP )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_C_T1, OnUpdateCheckBoxT1 )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_C_T2, OnUpdateCheckBoxT2 )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_C_DT, OnUpdateCheckBoxDT )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_C_POWER, OnUpdateCheckBoxPower )
	ON_UPDATE_COMMAND_UI( ID_RBN_LD_CO_MARKERS, OnUpdateCheckBoxSymbols )
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_COMMAND( ID_EDIT_COPY, OnEditCopy )
	ON_COMMAND( ID_RBN_LD_Z_UNDO, OnChartRescale )
	ON_COMMAND( ID_RBN_LD_Z_IN, OnChartZoomIn )
	ON_COMMAND( ID_RBN_LD_Z_OUT, OnChartZoomOut )
	
	ON_MESSAGE(WM_USER_NEWDOCUMENT, OnNewDocument )

END_MESSAGE_MAP()

LRESULT CRViewChart::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	m_pTADS = TASApp.GetpTADS();
	
	// Retrieve last selected Page Setup from TADS.
	m_pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup );
	
	m_ptSizeScroll = GetTotalSize();
	{
		m_ptSizeScroll.cx += 100;
		m_ptSizeScroll.cy += 100;
	}
	m_Xfactor = 100;
	return 0;
}

void CRViewChart::OnDraw( CDC* pDC )
{
	if( NULL == m_Chart.GetSafeHwnd() || TRUE == pDC->IsPrinting() )
		return;
	
	CTASelectDoc* pDoc = GetDocument();
	pDC->SetMapMode( MM_TEXT );
	int y = m_iTopMargin;
	m_iLeftMargin = 3;	
	int x = m_iLeftMargin;

	m_Xfactor = 100;
	if( true == m_fRedraw )
		ScrollToPosition( CPoint( 0, 0 ) );			// Set position 0,0

	if( NULL != m_pLoggedData )
	{
		CString str;
		str = TASApp.LoadLocalizedString( IDS_LOGGEDDATA );
		str += (CString)_T(": ") + m_pLoggedData->GetName(); 

		// Remark: title can be drawn now.
		y = DrawTitle( pDC, m_iLeftMargin, y, str, m_TitleBGColor, true );

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// Compute needed space under chart!
		int yNeeded = 0;
		// Adjust informations to the page Center
		if( RIGHTVIEWWIDTH < m_PrintRect.Width() )
			x = ( m_PrintRect.Width() - RIGHTVIEWWIDTH ) / 2;

		// Draw Cursor tracking info line
		if( true == m_Chart.IsCursorToolActive() )
		{
			// Remark: 'false' to just compute!
			yNeeded = DrawCursorToolInfo( pDC, m_pLoggedData, x, yNeeded, RIGHTVIEWWIDTH, false );
		}
		else
		{
			// Hide all texts relative to cursor info.
			for( int i = 0; i < MAXSTATICCURSOR; i++ )
				m_StaticCursor[i].ShowWindow( SW_HIDE );
		}

		yNeeded += m_iLineSpace;

		// Draw Log data info
		// Remark: 'false' to just compute!
		yNeeded = DrawLogInfo( pDC, m_pLoggedData, x, yNeeded, RIGHTVIEWWIDTH, false );
		//
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		if( true == m_fRedraw )
		{
			if( m_Chart.GetSafeHwnd() )
			{
				// Remark: we remove 50 because TeeChart draw outside its borders for legend below available space.
				m_Chart.SetWindowPos( NULL, m_iLeftMargin, y, m_PrintRect.Width(), m_PrintRect.Height() - y - yNeeded - 50, SWP_SHOWWINDOW | SWP_NOZORDER );
			}
		}
		y = m_PrintRect.Height() - y - yNeeded;

		// Draw Cursor tracking info line
		if( true == m_Chart.IsCursorToolActive() )
			y = DrawCursorToolInfo( pDC, m_pLoggedData, x, y, RIGHTVIEWWIDTH, true );

		y += m_iLineSpace;

		// Draw Log data info
		y = DrawLogInfo( pDC, m_pLoggedData, x, y, RIGHTVIEWWIDTH, true );
	}

	// Add scrolling only if we are below RIGHTVIEWWIDTH or RIGHTVIEWHEIGHT!
	CRect clientRect;
	GetClientRect( &clientRect );
	if( clientRect.Width() < RIGHTVIEWWIDTH || clientRect.Height() < RIGHTVIEWHEIGHT )
	{
		CSize size = GetTotalSize();
		if( y != size.cy || m_PrintRect.right != size.cx )
		{
			m_ptSizeScroll.cx = m_PrintRect.right;
			m_ptSizeScroll.cy = y;

			m_fNeedScroll = true;
	
			// Remark: 'NULL' to force updating all views (included this one)
			pDoc->UpdateAllViews( NULL );
		}
	}

	m_fRedraw = false;
}

void CRViewChart::SetRedraw( CLog* pLoggedData )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_fRedraw = true;
	m_pLoggedData = pLoggedData;
	if( NULL != pLoggedData )
	{
		BeginWaitCursor();
		CRect PageRect;
		GetClientRect( &PageRect );
		m_PrintRect = PageRect;
		if( PageRect.Width() < RIGHTVIEWWIDTH )
			m_PrintRect.right = RIGHTVIEWWIDTH;
		if( PageRect.Height() < RIGHTVIEWHEIGHT )
			m_PrintRect.bottom = m_PrintRect.top + RIGHTVIEWHEIGHT;
		m_Chart.DisplayLoggedData( pLoggedData );
		Invalidate();
		EndWaitCursor();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRViewChart diagnostics

#ifdef _DEBUG
void CRViewChart::AssertValid() const
{
	CRScrollView::AssertValid();
}

void CRViewChart::Dump(CDumpContext& dc) const
{
	CRScrollView::Dump(dc);
}
CTASelectDoc* CRViewChart::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTASelectDoc)));
	return (CTASelectDoc*)m_pDocument;
}

#endif //_DEBUG

int CRViewChart::DrawCursorToolInfo( CDC* pDC, CLog *pLD , int x, int y, int iWidth, bool fDrawNow ) 
{
	if( NULL == pLD )
		return y;

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	CPen pen;
	pen.CreatePen( PS_SOLID, 1, RGB( 0, 0, 0 ) );
	CPen *pOldPen = pDC->SelectObject( &pen );
	CFont *pOldFont;
	if( TRUE == pDC->IsPrinting() )
		pOldFont = pDC->SelectObject( &m_PrintText11 );
	else
		pOldFont = pDC->SelectObject( &m_Text11 );

	y += m_iLineSpace;
	if( true == fDrawNow )
	{
		// Draw upper line.
		pDC->MoveTo( CPoint( x, y ) );
		pDC->LineTo( CPoint( x + iWidth, y ) );
		y += m_iLineSpace;

		// Draw the text that stays static and locates the static boxes
		//=============================================================
		int xtmp, xcur;
		int ytmp = y;
		int staticHeight = 14;
		CString str, strUnit;

		// Draw the "Time:" string
		xcur = xtmp = x;// + 10;
		// Locate the first static box here.
		int deltaX = 150;			// Size is 150 pixels for date & time.
		CPoint point = GetScrollPosition();
		m_StaticCursor[0].SetWindowPos( &wndTop, xcur - point.x, ytmp - point.y, deltaX, staticHeight, SWP_SHOWWINDOW );
		xcur += deltaX;
		deltaX = 120;					// Size is 120 pixel for each static.
		for( int i = 1; i < MAXSTATICCURSOR; i++ )
		{
			m_StaticCursor[i].SetWindowPos( &wndTop, xcur - point.x, ytmp - point.y, deltaX, staticHeight, SWP_SHOWWINDOW | SWP_NOZORDER );
			xcur += deltaX;
		}
	}
	pDC->SelectObject( pOldFont );
	pDC->SelectObject( pOldPen );
	pen.DeleteObject();
	
	y += m_iLineSpace;
	return y;
}

/////////////////////////////////////////////////////////////////////////////
// CRViewChart message handlers

BEGIN_EVENTSINK_MAP( CRViewChart, CView )
	//{{AFX_EVENTSINK_MAP(CRViewChart)
	ON_EVENT( CRViewChart, IDC_LOGGEDDATACHART, 40 /* OnCursorToolChange */, OnCursorToolChangeTchart, VTS_I4 VTS_I4 VTS_I4 VTS_R8 VTS_R8 VTS_I4 VTS_I4 )
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CRViewChart::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext ) 
{
	if( FALSE == CWnd::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext ) )
		return FALSE ;
	
	// Create a TeeChart window.
	// Remark: first parameter is set to NULL because TEEchart activeX doesn't support the property DISPID_CAPTION and DISPID_TEXT to set
	//         the window text. In debug mode we have a COleException message printed in the output and we don't want it any more.
	if( FALSE == m_Chart.Create( NULL, ( dwStyle | WS_CHILD ) ^ WS_VISIBLE, rect, this, IDC_LOGGEDDATACHART ) ) 
		return FALSE;

	// Create three static controls for Cursor tool info.
	CRect staticRect( 0, 0, 10, 40 );
	for( int i = 0; i < MAXSTATICCURSOR; i++ )
	{
		if( FALSE == m_StaticCursor[i].Create( _T("Static cursor1"), ( dwStyle | WS_CHILD ) ^ WS_VISIBLE ^ WS_BORDER, staticRect, this, IDC_LDCURSORSTATIC1 + i ) )
			return FALSE;
		m_StaticCursor[i].SetFont( &m_Text11 );
	}

	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );
	
	return TRUE;
}

void CRViewChart::OnCursorToolChangeTchart( long lTool, long lX, long lY, double dXVal, double dYVal, long lSeries, long lValueIndex ) 
{
	if( NULL == m_Chart.GetSafeHwnd() || lTool != m_Chart.GetCursorTool() )
		return;

	if( 0 == lSeries )
	{
		CTimeUnic dtu;
		CString str, str1;
		str = TASApp.LoadLocalizedString( IDS_TIME );
		str += _T(":");
		m_pLoggedData->GetDateTime( lValueIndex, dtu );
		str1 = dtu.Format( IDS_LOGDATETIME_FORMAT );
		str += str1;
		m_StaticCursor[0].SetWindowText( str );

		double dY0 = 0.0;
		double dY1 = 0.0;
		int index = min( max( lValueIndex, 0 ), m_pLoggedData->GetLength() );

		for( int i = 1; i < MAXSTATICCURSOR; i++ )
			m_StaticCursor[i].SetWindowText( _T("") );
		
		int iStat = 1;
		if( true == m_Chart.IsPowerCurve() )
		{
			m_pLoggedData->GetValue( CLog::ect_Power, index, dY0 );
			str = TASApp.LoadLocalizedString( IDS_CHART_POWER );
			str += _T(":");
			str += WriteCUDouble( _U_TH_POWER, dY0, true );
			m_StaticCursor[iStat++].SetWindowText( str );
		}
		
		if( true == m_Chart.IsFlowCurve() )
		{
			m_pLoggedData->GetValue( CLog::ect_Flow, index, dY0 );
			str = TASApp.LoadLocalizedString( IDS_FLOW );
			str += _T(":");
			str += WriteCUDouble( _U_FLOW, dY0, true );
			m_StaticCursor[iStat++].SetWindowText( str );
		}
		
		if( true == m_Chart.IsDpCurve() )
		{
			m_pLoggedData->GetValue( CLog::ect_Dp, index, dY0 );
			str = TASApp.LoadLocalizedString( IDS_CHART_DIFFPRESS );
			str += _T(":");
			str += WriteCUDouble( _U_DIFFPRESS, dY0, true );
			m_StaticCursor[iStat++].SetWindowText( str );
		}
		
		if( true == m_Chart.IsDTCurve() )
		{
			m_pLoggedData->GetValue( CLog::ect_DeltaT, index, dY0 );
			str = TASApp.LoadLocalizedString(IDS_CHART_DIFFTEMP);
			str += _T(":");
			str += WriteCUDouble( _U_DIFFTEMP, dY0, true );
			m_StaticCursor[iStat++].SetWindowText( str );
		}
		
		if( true == m_Chart.IsT1Curve() )
		{
			m_pLoggedData->GetValue( CLog::ect_Temp1, index, dY0 );
			if( CLog::TOOL_TASCOPE == m_pLoggedData->ToolUsedForMeasure() )
				str = TASApp.LoadLocalizedString( IDS_CHART_T1 );
			else
				str = TASApp.LoadLocalizedString( IDS_TEMPERATURE );
			str += _T(":");
			str += WriteCUDouble( _U_TEMPERATURE, dY0, true );
			m_StaticCursor[iStat++].SetWindowText( str );
		}
		
		if( true == m_Chart.IsT2Curve() )
		{
			m_pLoggedData->GetValue( CLog::ect_Temp2, index, dY0 );
			str = TASApp.LoadLocalizedString(IDS_CHART_T2);
			str += _T(":");
			str += WriteCUDouble( _U_TEMPERATURE, dY0, true );
			m_StaticCursor[iStat++].SetWindowText( str );
		}
	}
}

void CRViewChart::OnChartTrack() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_Chart.ToggleCursorTool();
	Invalidate();
}

void CRViewChart::OnChartRescale() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_Chart.GetZoom().Undo();
	m_Chart.SetRedraw();
}

void CRViewChart::OnChartZoomIn() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	if( true == m_Chart.Is2DZoomActive() )
	{
		double dPercentZoom = 110.0;
		m_Chart.GetZoom().ZoomPercent( dPercentZoom );
		// Use this function because custom axis do not rescale properly.
		m_Chart.SetMinMax2DZoom( dPercentZoom );
	}
	else
		m_Chart.ZoomIn1D();
}

void CRViewChart::OnChartZoomOut() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	if( true == m_Chart.Is2DZoomActive() )
	{
		double dPercentZoom = 90.0;
		m_Chart.GetZoom().ZoomPercent( dPercentZoom );
		// Use this function because custom axis do not rescale properly.
		m_Chart.SetMinMax2DZoom( dPercentZoom );
	}
	else
		m_Chart.ZoomOut1D();
}

void CRViewChart::OnChartSymbols()
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	m_Chart.ToggleSymbols();
}

void CRViewChart::OnChart2dzoom() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	m_Chart.Toggle2DZoom();
}

void CRViewChart::OnChartDateaxis() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	m_Chart.ToggleDateAxis();
}

void CRViewChart::OnFileExportChart() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	CDlgExportChart dlg;
	dlg.Display( &m_Chart,m_pLoggedData );
}

void CRViewChart::OnEditCopy() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	m_Chart.GetExport().CopyToClipboardBitmap();
}

void CRViewChart::OnChartLegend() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	m_Chart.ToggleLegend();
}

void CRViewChart::OnCheckBoxFlow() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	
	m_Chart.CheckUnCheckFlowAxis();
	
	// UnCheck the tracking point when add remove a curve to avoid refreshing problem.
	if( true == m_Chart.IsCursorToolActive() )
		OnChartTrack();
}

void CRViewChart::OnCheckBoxDP() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_Chart.CheckUnCheckDpAxis();
	
	// UnCheck the tracking point when add remove a curve to avoid refreshing problem.
	if( true == m_Chart.IsCursorToolActive() )
		OnChartTrack();	
}

void CRViewChart::OnCheckBoxT1() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_Chart.CheckUnCheckT1Axis();
	
	// UnCheck the tracking point when add remove a curve to avoid refreshing problem.
	if (m_Chart.IsCursorToolActive())
		OnChartTrack();
}

void CRViewChart::OnCheckBoxT2() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_Chart.CheckUnCheckT2Axis();
	
	// UnCheck the tracking point when add remove a curve to avoid refreshing problem.
	if( true == m_Chart.IsCursorToolActive() )
		OnChartTrack();
}

void CRViewChart::OnCheckBoxDT() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_Chart.CheckUnCheckDTAxis();
	
	// UnCheck the tracking point when add remove a curve to avoid refreshing problem.
	if( true == m_Chart.IsCursorToolActive() )
		OnChartTrack();
}

void CRViewChart::OnCheckBoxPower() 
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	m_Chart.CheckUnCheckPowerAxis();
	
	// UnCheck the tracking point when add remove a curve to avoid refreshing problem.
	if( true == m_Chart.IsCursorToolActive() )
		OnChartTrack();
}

void CRViewChart::OnUpdateChartTrack( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	pCmdUI->SetCheck( m_Chart.IsCursorToolActive() );
}

void CRViewChart::OnUpdateChart2dzoom( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	pCmdUI->SetCheck( m_Chart.Is2DZoomActive() );
}

void CRViewChart::OnUpdateChartDateaxis( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	pCmdUI->SetCheck( m_Chart.IsDateonAxis() );
}

void CRViewChart::OnUpdateChartLegend( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;
	pCmdUI->SetCheck( m_Chart.IsLegendVisible() );
}

void CRViewChart::OnUpdateCheckBoxFlow( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	// Special treatment to avoid that no curves appears on the graph.
	if( true == m_Chart.IsFlowCurve() && !( m_Chart.IsDpCurve() || m_Chart.IsDTCurve() || m_Chart.IsT1Curve() || m_Chart.IsT2Curve() || m_Chart.IsPowerCurve() ) )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( TRUE );
		return;
	}
	
	if( CLog::LOGTYPE_FLOW == m_pLoggedData->GetMode() || CLog::LOGTYPE_FLOWTEMP == m_pLoggedData->GetMode() ||
		CLog::LOGTYPE_POWER == m_pLoggedData->GetMode() )
	{
		pCmdUI->Enable( TRUE );
		pCmdUI->SetCheck( m_Chart.IsFlowCurve() );
	}
	else
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
	}
}

void CRViewChart::OnUpdateCheckBoxDP( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	// Special treatment to avoid that no curves appears on the graph.
	if( true == m_Chart.IsDpCurve() && !( m_Chart.IsFlowCurve() || m_Chart.IsDTCurve() || m_Chart.IsT1Curve() || m_Chart.IsT2Curve() || m_Chart.IsPowerCurve() ) )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( TRUE );
		return;
	}
	
	if(   CLog::LOGTYPE_DP == m_pLoggedData->GetMode() ||
		( CLog::LOGTYPE_FLOW == m_pLoggedData->GetMode() && CLog::TOOL_TASCOPE == m_pLoggedData->ToolUsedForMeasure() ) ||
		  CLog::LOGTYPE_DPTEMP == m_pLoggedData->GetMode() ||
		( CLog::LOGTYPE_FLOWTEMP == m_pLoggedData->GetMode() && CLog::TOOL_TASCOPE == m_pLoggedData->ToolUsedForMeasure() ) ||
		  CLog::LOGTYPE_POWER == m_pLoggedData->GetMode() )
	{
		pCmdUI->Enable( TRUE );
		pCmdUI->SetCheck( m_Chart.IsDpCurve() );
	}
	else
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
	}
}

void CRViewChart::OnUpdateCheckBoxT1( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	// Special treatment to avoid that no curves appears on the graph.
	if( true == m_Chart.IsT1Curve() && !( m_Chart.IsDpCurve() || m_Chart.IsDTCurve() || m_Chart.IsFlowCurve() || m_Chart.IsT2Curve() || m_Chart.IsPowerCurve() ) )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( TRUE );
		return;
	}

	if( ( CLog::LOGTYPE_TEMP == m_pLoggedData->GetMode() ||	CLog::LOGTYPE_DPTEMP == m_pLoggedData->GetMode() ||	CLog::LOGTYPE_FLOWTEMP == m_pLoggedData->GetMode() ||
		  CLog::LOGTYPE_DT == m_pLoggedData->GetMode() || CLog::LOGTYPE_POWER == m_pLoggedData->GetMode() ) &&
		true == m_Chart.IsT1ValidValues() )
	{
		pCmdUI->Enable( TRUE );
		pCmdUI->SetCheck( m_Chart.IsT1Curve() );
	}
	else
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
	}
}

void CRViewChart::OnUpdateCheckBoxT2( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	// Special treatment to avoid that no curves appears on the graph.
	if( true == m_Chart.IsT2Curve() && !( m_Chart.IsDpCurve() || m_Chart.IsDTCurve() || m_Chart.IsT1Curve() || m_Chart.IsFlowCurve()|| m_Chart.IsPowerCurve() ) )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( TRUE );
		return;
	}
	
	if( ( CLog::LOGTYPE_TEMP == m_pLoggedData->GetMode() || CLog::LOGTYPE_DPTEMP == m_pLoggedData->GetMode() ||	CLog::LOGTYPE_FLOWTEMP == m_pLoggedData->GetMode() ||
		  CLog::LOGTYPE_DT == m_pLoggedData->GetMode() || CLog::LOGTYPE_POWER == m_pLoggedData->GetMode() ) &&
		true == m_Chart.IsT2ValidValues() )
	{
		pCmdUI->Enable( TRUE );
		pCmdUI->SetCheck( m_Chart.IsT2Curve() );
	}
	else
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
	}
}

void CRViewChart::OnUpdateCheckBoxDT( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	// Special treatment to avoid that no curves appears on the graph.
	if( true == m_Chart.IsDTCurve() && !( m_Chart.IsDpCurve() || m_Chart.IsFlowCurve() || m_Chart.IsT1Curve() || m_Chart.IsT2Curve() || m_Chart.IsPowerCurve() ) )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( TRUE );
		return;
	}
	
	if( CLog::LOGTYPE_POWER == m_pLoggedData->GetMode() || CLog::LOGTYPE_DT == m_pLoggedData->GetMode() )
	{
		pCmdUI->Enable( TRUE );
		pCmdUI->SetCheck( m_Chart.IsDTCurve() );
	}
	else
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
	}
}

void CRViewChart::OnUpdateCheckBoxPower( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	// Special treatment to avoid that no curves appears on the graph.
	if( true == m_Chart.IsPowerCurve() && !( m_Chart.IsDpCurve() || m_Chart.IsDTCurve() || m_Chart.IsT1Curve() || m_Chart.IsT2Curve() || m_Chart.IsFlowCurve() ) )
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( TRUE );
		return;
	}
	
	if( CLog::LOGTYPE_POWER == m_pLoggedData->GetMode() )
	{
		pCmdUI->Enable( TRUE );
		pCmdUI->SetCheck( m_Chart.IsPowerCurve() );
	}
	else
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
	}	
}

void CRViewChart::OnUpdateCheckBoxSymbols( CCmdUI* pCmdUI )
{
	if( NULL == m_Chart.GetSafeHwnd() )
		return;

	pCmdUI->SetCheck( m_Chart.IsSymbolsActive() );
}

void CRViewChart::OnSize( UINT nType, int cx, int cy )
{
	CRScrollView::OnSize( nType, cx, cy );
	
	// Update drawable zone.
	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
	{
		m_PrintRect.right = RIGHTVIEWWIDTH;
	}
	if( PageRect.Height() < ( RIGHTVIEWHEIGHT + 20 ) )
	{
		m_PrintRect.bottom = m_PrintRect.top + RIGHTVIEWHEIGHT;
	}
	m_ptSizeScroll.cx = cx;
	m_ptSizeScroll.cy = cy;

	m_fRedraw = true;
}

HBRUSH CRViewChart::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor ) 
{
	HBRUSH hbr;
	for( int i = 0; i < MAXSTATICCURSOR; i++ )
	{
		if( pWnd->m_hWnd == m_StaticCursor[i].m_hWnd )
		{
			pDC->SetTextColor( RGB( 0, 0, 0 ) );
			pDC->SetBkColor( RGB( 255, 255, 255 ) );
			return (HBRUSH)m_brWhiteBrush;
		}
	}	
	hbr = CRScrollView::OnCtlColor( pDC, pWnd, nCtlColor );
	return hbr;
}

///////////////////////////////////////////////////////////////////////////
// Printing functions

BOOL CRViewChart::OnPreparePrinting( CPrintInfo* pInfo )
{
	return CRScrollView::OnPreparePrinting( pInfo );
}

void CRViewChart::OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo )
{
	CRScrollView::OnBeginPrinting( pDC, pInfo );
}

void CRViewChart::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo )
{
	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;
 
	CRScrollView::OnEndPrinting( pDC, pInfo );
	SetRedraw( m_pLoggedData );
}

void CRViewChart::OnPrint( CDC* pDC, CPrintInfo* pInfo )
{
	pDC->SetMapMode( MM_TEXT );
	
	// 'm_PrintRect' space available on sheet.
	m_PrintRect = pInfo->m_rectDraw;
	
	// Compute left and right margin.
	int iMargin, x;
	iMargin = ( pDC->GetDeviceCaps( HORZRES ) * 100 ) / pDC->GetDeviceCaps( HORZSIZE );		// Horizontal resolution pt/mm
	
	// Margin is fixed to 10 mm.
	iMargin *= 10;
	m_PrintRect.right -= iMargin / 100;
	m_PrintRect.left += iMargin / 100;
	x=m_iLeftMargin = iMargin / 100;

	CRect SaveRect( m_PrintRect );

	if( m_PrintRect.Width() < m_PrintRect.Height() )
		m_Xfactor = ( m_PrintRect.Width() * 80 ) / RIGHTVIEWWIDTH;	
	else
		m_Xfactor = ( m_PrintRect.Height() * 80 ) / RIGHTVIEWWIDTH;	

	int y;

	// TODO to increase the speed of print preview,  on the first page create a table witch contains 
// a CurrentPrintPos and CurrentPrintIndex for each page, this table is created one times on the beginning
// of print preview.  Afterwards when the user click on next/preview buttons use this table to find out
// CurrentPrintPos and CurrentPrintIndex
	for( UINT page = 1; page <= pInfo->m_nCurPage; page++ )
	{
		if( 1 == page )
		{
			m_CurPrintPos = ecppHeader;					// Start with header page.
			m_CurPrintIndex = 0;
			m_CurrentPage = 1;
		}
		
		m_PrintRect = SaveRect;
		y = m_PrintRect.top;	
		if( page == pInfo->m_nCurPage )
			m_bPrintThisPage = true;
		else
			m_bPrintThisPage = false;

		// Y current position in window
		
		/////////////
		//HEADER	
		/////////////
		// always Draw Header and footer
		y = DrawHeader( pDC, m_iLeftMargin, y, page, m_bPrintThisPage );
		y = DrawFooter( pDC, m_iLeftMargin, y, page, m_bPrintThisPage );

		if( 1 == page )
			m_CurPrintPos = ecppChartTitle;
		
		//////////////////////
		//CHART TITLE
		//////////////////////
		if( ecppChartTitle == m_CurPrintPos )
		{
			y += ( m_iLineSpace * m_Xfactor ) / 100;
			CString str;
			str = TASApp.LoadLocalizedString( IDS_LOGGEDDATA );
			str += (CString)_T(": ") + m_pLoggedData->GetName(); 
			y = DrawTitle( pDC, m_iLeftMargin, y, str, m_TitleBGColor, m_bPrintThisPage );
		
			m_CurPrintPos = ecppChartInfo;
		}
		
		//////////////////////
		//CHART INFORMATIONS
		//////////////////////
		int yTop = y;						// save the current position.
		int yBot = y;
		if( ecppChartInfo == m_CurPrintPos )
		{
			// Compute the height required by chart informations.
			y += ( m_iLineSpace * m_Xfactor ) / 100;
			
			// Adjust to the page center.
			if( ( RIGHTVIEWWIDTH * m_Xfactor / 100 ) < m_PrintRect.Width() )
			{
				x = ( m_PrintRect.Width() - RIGHTVIEWWIDTH * m_Xfactor / 100 ) / 2 + m_iLeftMargin;
				
				// Draw log data info.
				y = DrawLogInfo( pDC, m_pLoggedData, x, y, RIGHTVIEWWIDTH * m_Xfactor / 100, false );
			}
			else
				y = DrawLogInfo( pDC, m_pLoggedData, x, y, m_PrintRect.Width(), false );

			// Draw now the chart informations.
			yBot = y = m_PrintRect.bottom - ( y - yTop ) - ( m_iLineSpace * m_Xfactor) /100;
			if( ( RIGHTVIEWWIDTH * m_Xfactor / 100 ) < m_PrintRect.Width() )
			{
				x = ( m_PrintRect.Width() - RIGHTVIEWWIDTH * m_Xfactor / 100 ) /  2 + m_iLeftMargin;
				// Draw log data info.
				DrawLogInfo( pDC, m_pLoggedData, x, y, RIGHTVIEWWIDTH * m_Xfactor / 100, m_bPrintThisPage );
			}
			else
				y = DrawLogInfo( pDC, m_pLoggedData, x, y, m_PrintRect.Width(), m_bPrintThisPage );
			
			m_CurPrintPos = ecppChart;
		}

		//////////////////////
		//CHART
		//////////////////////
		if( ecppChart == m_CurPrintPos )
		{
			y = yTop;
			
			// Fill rows.
			y += ( m_iLineSpace * m_Xfactor ) / 100;

			if( NULL != m_Chart.GetSafeHwnd() )
			{
				if( FALSE == pInfo->m_bPreview )
				{
					m_Chart.GetPrinter().PrintPartialHandle( COleVariant( LONG( pDC->m_hDC ) ),	m_iLeftMargin, y, m_PrintRect.right - m_iLeftMargin, yBot );
				}
				else
				{
					VARIANT vHDC;
					vHDC.lVal = (LONG)pDC->m_hDC;
					m_Chart.Draw( vHDC, m_iLeftMargin, y, m_PrintRect.right - m_iLeftMargin, yBot );
				}
			}
		}
	}

	if( y < 0 ) //one of them didn't had enough space to write informations so next page....
	{
		if( true == m_bPrintThisPage )
			pInfo->SetMaxPage( pInfo->m_nCurPage + 2 );
		m_CurrentPage++;
	}
	else 
	{
		if( true == m_bPrintThisPage )
			pInfo->SetMaxPage( pInfo->m_nCurPage );
		m_bContinuePrinting = false;
	}
	
	CRScrollView::OnPrint(pDC, pInfo);
}

void CRViewChart::OnPrepareDC( CDC* pDC, CPrintInfo* pInfo ) 
{
	CRScrollView::OnPrepareDC( pDC, pInfo );
}

void CRViewChart::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
	CRScrollView::OnUpdate( pSender, lHint, pHint );
	if( true == m_fNeedScroll )
		SetScrollSizes( MM_TEXT, m_ptSizeScroll );
}
