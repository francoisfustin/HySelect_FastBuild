#include "stdafx.h"
#include "stdafx.h"
#include "TASelect.h"
#include "DlgPMTChartHelper.h"
#include "SelectPM.h"
#include "afxdialogex.h"

// For TChart
#include "TeeChartDefines.h"
#include "annotationtool.h"
#include "antialiastool.h"
#include "aspect.h"
#include "axes.h"
#include "axis.h"
#include "axislabels.h"
#include "axislabelsitem.h"
#include "axistitle.h"
#include "backimage.h"
#include "bubbleseries.h"
#include "brush.h"
#include "canvas.h"
#include "chartaxispen.h"
#include "cursortool.h"
#include "environment.h"
#include "fastlineseries.h"
#include "gradient.h"
#include "hover.h"
#include "legend.h"
#include "legenditem.h"
#include "legendsymbol.h"
#include "legendtitle.h"
#include "lineseries.h"
#include "marks.h"
#include "markscallout.h"
#include "panel.h"
#include "pen.h"
#include "pointer.h"
#include "pointseries.h"
#include "scroll.h"
#include "series.h"
#include "serieslist.h"
#include "seriespointeritems.h"
#include "strings.h"
#include "teeemboss.h"
#include "teefont.h"
#include "teefunction.h"
#include "teeshadow.h"
#include "textshape.h"
#include "titles.h"
#include "toollist.h"
#include "tools.h"
#include "valuelist.h"
#include "wall.h"
#include "walls.h"
#include "zoom.h"

IMPLEMENT_DYNAMIC( CDlgPMTChartHelper, CDialogEx )

CDlgPMTChartHelper::CDlgPMTChartHelper( CWnd *pParent )
	: CDialogEx( IDD_DLGPMTCHARTHELPER, pParent )
{
	m_rLegendPosInfo.m_mapTopPos.clear();
	m_rLegendPosInfo.m_mapIndexOrder.clear();

	m_bAnnotationTextDisplayed = false;
	m_eCurrentDeviceType = Undefined;
	m_bStaticoLeftAxisInitialized = false;
	m_dYMinPressSI = 0.0;
	m_dYMinPressCU = 0.0;
	m_dYMaxPressSI = 0.0;
	m_dYMaxPressCU = 0.0;

	m_dWorkingPointqNSI = 0.0;
	m_dWorkingPointPmanSI = 0.0;
	
	m_lLegendLeft = 0;
	m_iNextLegendItemYPos = 0;
}

CDlgPMTChartHelper::~CDlgPMTChartHelper()
{
	if( NULL != m_TChart.GetSafeHwnd() )
	{
		m_TChart.DestroyWindow();
	}
}

void CDlgPMTChartHelper::DrawExpansionVesselCurves( CSelectedVssl *pclSelectedVessel, CPMInputUser *pclPMInputUser )
{
	if( NULL == pclSelectedVessel || NULL == pclSelectedVessel->GetProductIDPtr().MP
			|| NULL == dynamic_cast<CDB_Vessel *>( pclSelectedVessel->GetProductIDPtr().MP )
			|| NULL == pclPMInputUser )
	{
		ASSERT_RETURN;
	}

	double dXMinTempSI = 0.0;
	double dXMaxTempSI = 0.0;

	if( ProjectType::Heating == pclPMInputUser->GetApplicationType() || ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		dXMinTempSI = pclPMInputUser->GetMinTemperature();

		if( true == pclPMInputUser->IsNorm( ProjectType::All, PN_SWKIHE301_01 ) )
		{
			// XMax = tps - ( tps - tpr ) / 2 = 2tps/2 - tps/2 + tpr/2 = (tps + tpr) / 2
			dXMaxTempSI = ( pclPMInputUser->GetSupplyTemperature() + pclPMInputUser->GetReturnTemperature() ) / 2;
		}
		else
		{
			dXMaxTempSI = pclPMInputUser->GetSupplyTemperature();
		}
	}
	else
	{
		if( true == pclPMInputUser->IsNorm( ProjectType::Cooling, PressurisationNorm::PN_SWKIHE301_01 ) )
		{
			// HYS-1534: In cooling, where the max. temperature will not exceed 40 °C, the coefficient e is determined 
			// between the lowest possible temperature and 40 °C (standstill of the system). If above 40 °C we take the Tmax.
			dXMaxTempSI = max( 40.0, pclPMInputUser->GetMaxTemperature() );
		}
		else
		{
			dXMinTempSI = pclPMInputUser->GetMinTemperature();
			dXMaxTempSI = pclPMInputUser->GetMaxTemperature();
		}
	}

	if( 0.0 == dXMinTempSI && 0.0 == dXMaxTempSI )
	{
		return;
	}

	m_dYMinPressSI = pclPMInputUser->GetMinimumPressure();
	m_dYMaxPressSI = pclPMInputUser->GetSafetyValveResponsePressure();

	// Convert these values in custom unit.
	double dXMinTempCU = CDimValue::SItoCU( _U_TEMPERATURE, dXMinTempSI );
	double dXMaxTempCU = CDimValue::SItoCU( _U_TEMPERATURE, dXMaxTempSI );

	m_dYMinPressCU = CDimValue::SItoCU( _U_PRESSURE, m_dYMinPressSI );
	m_dYMaxPressCU = CDimValue::SItoCU( _U_PRESSURE, m_dYMaxPressSI );

	m_TChart.GetAspect().SetView3D( FALSE );

	m_TChart.GetWalls().GetBack().GetBrush().GetGradient().SetVisible( FALSE );
	m_TChart.GetWalls().GetBack().SetColor( DLGPMTCHARTHELPER_WALLBACKCOLOR );
	m_TChart.GetWalls().GetBack().SetTransparent( FALSE );

	m_TChart.GetHeader().GetFont().SetCharset( ANSI_CHARSET );
	m_TChart.GetHeader().GetFont().SetColor( DLGPMTCHARTHELPER_HEADERFONTCOLOR );
	m_TChart.GetHeader().GetFont().SetName( _T("Arial") );
	m_TChart.GetHeader().GetFont().SetBold( TRUE );
	m_TChart.GetHeader().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetHeader().GetText().SetText( TASApp.LoadLocalizedString( IDS_SELECTPM_STATICOCURVE ) );

	m_TChart.GetPanel().GetGradient().SetVisible( FALSE );
	m_TChart.GetPanel().SetColor( DLGPMTCHARTHELPER_PANELCOLOR );
	m_TChart.GetPanel().SetBevelOuter( FALSE );
	m_TChart.GetHover().SetVisible( FALSE );

	m_TChart.GetLegend().SetWidth( DLGPMTCHARTHELPER_STATICO_LEGENDWIDTH );
	m_TChart.GetLegend().SetResizeChart( TRUE );
	m_TChart.GetLegend().SetVisible( TRUE );
	m_TChart.GetLegend().GetDividingLines().SetWidth( 0 );
	m_TChart.GetLegend().GetFont().SetName( _T("Arial") );
	m_TChart.GetLegend().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetShadow().SetTransparency( 0 );
	m_TChart.GetLegend().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetSymbol().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().SetTransparent( TRUE );
	m_TChart.GetLegend().GetTitle().SetVisible( FALSE );
	m_TChart.GetLegend().SetColumnWidthAuto( FALSE );
	m_TChart.GetLegend().SetColumnWidths( 0, 20 );
	m_TChart.GetLegend().SetColumnWidths( 1, DLGPMTCHARTHELPER_STATICO_LEGENDWIDTH - 40 );
	m_TChart.GetLegend().GetFrame().SetVisible( FALSE );

	////////////////////////////////////////////////////////////////////////////////////////
	// Bottom axis

	double dIncrement = 5.0;
	m_TChart.GetAxis().GetBottom().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetBottom().GetAxisPen().SetWidth( 1 );
	m_TChart.GetAxis().GetBottom().GetAxisPen().SetColor( DLGPMTCHARTHELPER_AXISPENCOLOR );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetBottom().SetIncrement( ( ( dXMaxTempCU - dXMinTempCU ) / 5.0 >= 18 ) ? 10.0 : 5.0 );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetBottom().GetLabels().SetAlternate( ( ( dXMaxTempCU - dXMinTempCU ) / 5.0 >= 18 ) ? TRUE : FALSE );
	m_TChart.GetAxis().GetBottom().GetLabels().SetOnAxis( FALSE );
	m_TChart.GetAxis().GetBottom().GetLabels().SetSeparation( 0 );
	m_TChart.GetAxis().GetBottom().GetLabels().SetValueFormat( _T("0") );
	m_TChart.GetAxis().GetBottom().GetLabels().SetRoundFirstLabel( FALSE );
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetColor( DLGPMTCHARTHELPER_MINORGRIDCOLOR );
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetVisible( TRUE );
	m_TChart.GetAxis().GetBottom().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetBottom().SetMinorTickLength( 0 );
	m_TChart.GetAxis().GetBottom().GetTicks().SetColor( _BLACK );
	m_TChart.GetAxis().GetBottom().GetTicksInner().SetVisible( FALSE );

	double dOffset = ( dXMaxTempCU - dXMinTempCU ) / 18.0;
	m_TChart.GetAxis().GetBottom().SetMaximum( dXMaxTempCU + dIncrement );
	m_TChart.GetAxis().GetBottom().SetMinimum( dXMinTempCU - dIncrement );

	// Set the X axes unit.
	CString strBottomTitle;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	strBottomTitle.Format( _T("%s [%s]"), TASApp.LoadLocalizedString( IDS_SELECTPM_STATICOCURVETEMP ), pUnitDB->GetNameOfDefaultUnit( _U_TEMPERATURE ).c_str() );
	m_TChart.GetAxis().GetBottom().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetTitle().SetCaption( strBottomTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Left axis.
	m_TChart.GetAxis().GetLeft().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMinimum( FALSE );

	dIncrement = ( m_dYMaxPressCU - m_dYMinPressCU ) / 15.0;
	bool bUseDecimal = false;
	CString strValueFormat = _T("0");

	if( dIncrement < 1.0 )
	{
		bUseDecimal = true;
		strValueFormat = _T("0.00");
	}

	m_TChart.GetAxis().GetLeft().GetAxisPen().SetColor( DLGPMTCHARTHELPER_AXISPENCOLOR );
	m_TChart.GetAxis().GetLeft().GetAxisPen().SetWidth( 1 );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetLeft().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetLeft().SetIncrement( dIncrement );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetLeft().GetLabels().SetOnAxis( FALSE );
	m_TChart.GetAxis().GetLeft().GetLabels().SetValueFormat( strValueFormat );
	m_TChart.GetAxis().GetLeft().GetLabels().SetRoundFirstLabel( FALSE );
	m_TChart.GetAxis().GetLeft().GetTicks().SetColor( _BLACK );

	dOffset = ( m_dYMaxPressCU - m_dYMinPressCU ) / 10.0;
	m_TChart.GetAxis().GetLeft().SetMaximum( m_dYMaxPressCU + dOffset );
	m_TChart.GetAxis().GetLeft().SetMinimum( m_dYMinPressCU - dOffset );

	////////////////////////////////////////////////////////////////////////////////////////
	// Retrieve the good width of label text in regards to the current unit and font.
	CDC *pDC = m_TChart.GetDC();
	CDC dcImage;
	dcImage.CreateCompatibleDC( pDC );

	CBitmap clImage;
	clImage.CreateCompatibleBitmap( pDC, 500, 50 );

	m_TChart.ReleaseDC( pDC );

	LOGFONT rLogFont;
	ZeroMemory( &rLogFont, sizeof( rLogFont ) );
	rLogFont.lfCharSet = (BYTE)m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetCharset();

	CString strFontName = m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetName();
	int iSize = sizeof( rLogFont.lfFaceName ) / sizeof( WCHAR );
	wcscpy_s( rLogFont.lfFaceName, iSize, strFontName.GetBuffer() );

	long lHeight = m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetHeight();
	rLogFont.lfHeight = -MulDiv( lHeight, GetDeviceCaps( pDC->GetSafeHdc(), LOGPIXELSY ), 72 );
	rLogFont.lfItalic = m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetItalic();
	rLogFont.lfStrikeOut = m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetStrikethrough();
	rLogFont.lfUnderline = m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetUnderline();
	rLogFont.lfWeight = ( TRUE == m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetBold() ) ? FW_BOLD : FW_NORMAL;

	CFont clFont;
	clFont.CreateFontIndirect( &rLogFont );

	dcImage.SelectObject( &clImage );
	dcImage.SelectObject( &clFont );

	CRect rectText( 0, 0, 0, 0 );
	CString strLabel = WriteCUDouble( _U_PRESSURE, m_dYMaxPressSI, false, -1, 0 );

	if( true == bUseDecimal && -1 == strLabel.Find( _T("." ) ) )
	{
		strLabel += _T(".0");
	}

	dcImage.DrawText( strLabel, &rectText, DT_CENTER | DT_CALCRECT | DT_NOPREFIX );

	clFont.DeleteObject();
	clImage.DeleteObject();

	m_TChart.GetAxis().GetLeft().GetLabels().SetSize( (long)( rectText.Width() * 1.2 ) );
	////////////////////////////////////////////////////////////////////////////////////////

	// Set the Y axes unit.
	CString strLeftTitle;
	strLeftTitle.Format( _T("%s [%s]"), TASApp.LoadLocalizedString( IDS_SELECTPM_STATICOCURVEPRESSURE ), pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	m_TChart.GetAxis().GetLeft().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetTitle().SetCaption( strLeftTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Right axis.
	m_TChart.GetAxis().GetRight().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetRight().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Top axis.
	m_TChart.GetAxis().GetTop().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetTop().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	int iNumberOfSeries = ( ProjectType::Heating == pclPMInputUser->GetApplicationType() ) ? 7 : 6;
	std::vector<int> vecLegendTitle;
	std::vector<int> vecColors;
	std::vector<long> vecSeries;
	std::vector<long> vecPointerStyle;

	// HYS-1174: no more pa,min in cooling and solar (Pay attention, the series is created but not drawn).
	if( ProjectType::Heating == pclPMInputUser->GetApplicationType() )
	{
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_p0, 0 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pt, 1 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pa, 2 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pamin, 3 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pfill, 4 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pe, 5 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_PSV, 6 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_Last, 7 ) );

		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDP0 );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPT );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPA );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPAMIN );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPFILL );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPE );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPSV );

		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_P0 );				// p0 (minimum pressure) - dark blue
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PT );				// p(t) - black
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PA );				// pa (initial pressure) - light blue
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PAMIN );				// pa,min (minimum initial pressure) - blue
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PFILL );				// pfill (filling pressure) - green
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PE );				// pe (final pressure) - orange
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PSV );				// PSV - light red

		vecSeries.push_back( scLine );
		vecSeries.push_back( scLine );
		vecSeries.push_back( scBubble );
		vecSeries.push_back( scBubble );
		vecSeries.push_back( scBubble );
		vecSeries.push_back( scBubble );
		vecSeries.push_back( scLine );

		vecPointerStyle.push_back( -1 );
		vecPointerStyle.push_back( -1 );
		vecPointerStyle.push_back( psCircle );
		vecPointerStyle.push_back( psCross );
		vecPointerStyle.push_back( psCircle );
		vecPointerStyle.push_back( psCircle );
		vecPointerStyle.push_back( -1 );
	}
	else
	{
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_p0, 0 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pt, 1 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pa, 2 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pfill, 3 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_pe, 4 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_PSV, 5 ) );
		m_mapPCTypeToSeries.insert( std::pair<int, int>( PC_Last, 6 ) );

		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDP0 );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPT );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPA );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPFILL );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPE );
		vecLegendTitle.push_back( IDS_SELECTPM_STATICOCURVELEGENDPSV );

		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_P0 );				// p0 (minimum pressure) - dark blue
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PT );				// p(t) - black
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PA );				// pa (initial pressure) - light blue
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PFILL );				// pfill (filling pressure) - green
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PE );				// pe (final pressure) - orange
		vecColors.push_back( DLGPMTCHARTHELPER_STATICO_PSV );				// PSV - light red

		vecSeries.push_back( scLine );
		vecSeries.push_back( scLine );
		vecSeries.push_back( scBubble );
		vecSeries.push_back( scBubble );
		vecSeries.push_back( scBubble );
		vecSeries.push_back( scLine );

		vecPointerStyle.push_back( -1 );
		vecPointerStyle.push_back( -1 );
		vecPointerStyle.push_back( psCircle );
		vecPointerStyle.push_back( psCircle );
		vecPointerStyle.push_back( psCircle );
		vecPointerStyle.push_back( -1 );
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	// Defines all series.
	for( int iLoopSerie = 0; iLoopSerie < iNumberOfSeries; ++iLoopSerie )
	{
		switch( vecSeries[iLoopSerie ] )
		{
			case scLine:
				m_TChart.AddSeries( scLine );

				// Set width and style of the line between points (the lines in the legend will be the same).
				m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsLine().GetLinePen().SetWidth( 2 );
				m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsLine().GetLinePen().SetEndStyle( esSquare );

				if( PC_pamin == iLoopSerie )
				{
					m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsLine().GetLinePen().SetStyle( psDot );
				}

				break;

			case scBubble:
				m_TChart.AddSeries( scBubble );

				// Set the pointer style.
				m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsBubble().GetPointer().SetHorizontalSize( 16 );
				m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsBubble().GetPointer().SetInflateMargins( TRUE );
				m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsBubble().GetPointer().SetStyle( vecPointerStyle[iLoopSerie] );
				m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsBubble().GetPointer().SetVerticalSize( 16 );
				m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsBubble().GetPointer().SetVisible( TRUE );

				if( psCircle == vecPointerStyle[iLoopSerie] )
				{
					m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetAsBubble().GetPointer().GetPen().SetVisible( FALSE );
				}

				break;
		}

		// Common part to line or bubble series.

		// Change color for each series.
		m_TChart.GetSeriesList().GetItems( iLoopSerie ).SetColor( vecColors[iLoopSerie] );

		m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetMarks().SetVisible( FALSE );
		m_TChart.GetSeriesList().GetItems( iLoopSerie ).SetPercentFormat( _T("##0.##,%") );

		// Set the series title.
		m_TChart.GetSeriesList().GetItems( iLoopSerie ).SetTitle( TASApp.LoadLocalizedString( vecLegendTitle[iLoopSerie] ) );

		m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetXValues().SetName( _T("X") );
		m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetXValues().SetOrder( loAscending );
		m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetYValues().SetName( _T("Y") );
		m_TChart.GetSeriesList().GetItems( iLoopSerie ).GetYValues().SetOrder( loNone );
	}

	////////////////////////////////////////////////////////////////////////////////////////


	// It's only for the Statico curves!!
	if( ProjectType::Heating == pclPMInputUser->GetApplicationType() )
	{
		// To draw, we need this order:      0      1         2      3         4         5      6
		//                                   PC_p0, PC_pt,    PC_pa, PC_pamin, PC_pfill, PC_pe, PC_PSV.
		// But for the legend we reorder:    PC_p0, PC_pamin, PC_pa, PC_pfill, PC_pt,    PC_pe, PC_PSV
		m_rLegendPosInfo.m_mapIndexOrder[0] = 0;
		m_rLegendPosInfo.m_mapIndexOrder[1] = 4;
		m_rLegendPosInfo.m_mapIndexOrder[2] = 2;
		m_rLegendPosInfo.m_mapIndexOrder[3] = 1;
		m_rLegendPosInfo.m_mapIndexOrder[4] = 3;
		m_rLegendPosInfo.m_mapIndexOrder[5] = 5;
		m_rLegendPosInfo.m_mapIndexOrder[6] = 6;
	}
	else
	{
		
		// To draw, we need this order:      0      1      2         3         4      5
		//                                   PC_p0, PC_pt, PC_pa,    PC_pfill, PC_pe, PC_PSV.
		// But for the legend we reorder:    PC_p0, PC_pa, PC_pfill, PC_pt,    PC_pe, PC_PSV
		m_rLegendPosInfo.m_mapIndexOrder[0] = 0;
		m_rLegendPosInfo.m_mapIndexOrder[1] = 3;
		m_rLegendPosInfo.m_mapIndexOrder[2] = 1;
		m_rLegendPosInfo.m_mapIndexOrder[3] = 2;
		m_rLegendPosInfo.m_mapIndexOrder[4] = 4;
		m_rLegendPosInfo.m_mapIndexOrder[5] = 5;
	}


	////////////////////////////////////////////////////////////////////////////////////////
	// Draw all series.

	// p0.
	double dp0CU = CDimValue::SItoCU( _U_PRESSURE, m_dYMinPressSI );
	m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_p0] ).AddXY( dXMinTempCU, dp0CU, _T(""), vecColors[m_mapPCTypeToSeries[PC_p0]] );
	m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_p0] ).AddXY( dXMaxTempCU, dp0CU, _T(""), vecColors[m_mapPCTypeToSeries[PC_p0]] );

	// HYS-1022: 'pa' becomes 'pa,min'.
	// HYS-1174: no more pa,min in cooling and solar.
	double dRadius = ( m_dYMaxPressCU - m_dYMinPressCU ) / 40.0;

	if( ProjectType::Heating == pclPMInputUser->GetApplicationType() )
	{
		// Pa,min.
		double dpaminSI = pclSelectedVessel->GetMinimumInitialPressure();
		double dpaminCU = CDimValue::SItoCU( _U_PRESSURE, dpaminSI );
		m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_pamin] ).GetAsBubble().AddBubble( dXMinTempCU, dpaminCU, dRadius, _T(""), vecColors[m_mapPCTypeToSeries[PC_pamin]] );
	}

	// HYS-1022: 'pa,opt' becomes 'pa'.
	// Pa.
	double dpaSI = pclSelectedVessel->GetInitialPressure();
	double dpaCU = CDimValue::SItoCU( _U_PRESSURE, dpaSI );
	m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_pa] ).GetAsBubble().AddBubble( dXMinTempCU, dpaCU, dRadius, _T(""), vecColors[m_mapPCTypeToSeries[PC_pa]] );

	// Pfill.
	CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclSelectedVessel->GetProductIDPtr().MP );
	double dWaterReserve = pclSelectedVessel->GetWaterReserve();
	double dTotalVolume = pclSelectedVessel->GetNbreOfVsslNeeded() * pclVessel->GetNominalVolume();
	double dXFillTempSI = pclPMInputUser->GetFillTemperature();
	double dXFillTempCU = CDimValue::SItoCU( _U_TEMPERATURE, dXFillTempSI );

	double dYFillPressSI = pclPMInputUser->GetIntermediatePressure( dXFillTempSI, dWaterReserve, dTotalVolume );
	double dYFillPressCU = CDimValue::SItoCU( _U_PRESSURE, dYFillPressSI );

	m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_pfill] ).GetAsBubble().AddBubble( dXFillTempCU, dYFillPressCU, dRadius, _T(""), vecColors[m_mapPCTypeToSeries[PC_pfill]] );

	// P(t).
	// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	int iTempPoints = 50;
	double dTempStep = ( dXMaxTempSI - dXMinTempSI ) / (double)( iTempPoints - 1 );
	double dIntermediateTempSI = dXMinTempSI;

	for( int iLoopTemp = 0; iLoopTemp <= iTempPoints; ++iLoopTemp )
	{
		double dIntermediateTemperatureCU = CDimValue::SItoCU( _U_TEMPERATURE, dIntermediateTempSI );
		double dIntermediatePressureSI = pclPMInputUser->GetIntermediatePressure( dIntermediateTempSI, dWaterReserve, dTotalVolume );

		if( -1.0 == dIntermediatePressureSI )
		{
			// We reach the limit. Vessel volume can't contain water reserve and expanded volume.
			break;
		}

		double dIntermediatePressureCU = CDimValue::SItoCU( _U_PRESSURE, dIntermediatePressureSI );
		m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_pt] ).AddXY( dIntermediateTemperatureCU, dIntermediatePressureCU, _T(""), vecColors[m_mapPCTypeToSeries[PC_pt]] );

		dIntermediateTempSI += dTempStep;

		if( dIntermediateTempSI > dXMaxTempSI )
		{
			dIntermediateTempSI = dXMaxTempSI;
		}
	}

	// Pe.
	double dpeSI = pclPMInputUser->GetFinalPressure();
	double dpeCU = CDimValue::SItoCU( _U_PRESSURE, dpeSI );
	m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_pe] ).GetAsBubble().AddBubble( dXMaxTempCU, dpeCU, dRadius, _T(""), vecColors[m_mapPCTypeToSeries[PC_pe]] );

	// PSV.
	m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_PSV] ).AddXY( dXMinTempCU, m_dYMaxPressCU, _T(""), vecColors[m_mapPCTypeToSeries[PC_PSV]] );
	m_TChart.GetSeriesList().GetItems( m_mapPCTypeToSeries[PC_PSV] ).AddXY( dXMaxTempCU, m_dYMaxPressCU, _T(""), vecColors[m_mapPCTypeToSeries[PC_PSV]] );

	////////////////////////////////////////////////////////////////////////////////////////


	// Prepare the position of legend items.
	int iLoopEnd = m_TChart.GetSeriesCount();
	int iFirstPos = ( m_TChart.GetHeight() - ( iLoopEnd * 14 ) ) / 2;

	for( int iLoopCurve = 0; iLoopCurve < iLoopEnd; iLoopCurve++ )
	{
		m_rLegendPosInfo.m_mapTopPos.insert( std::make_pair( iLoopCurve, iFirstPos ) );
		iFirstPos += 14;
	}

	m_eCurrentDeviceType = Vessel;
}

void CDlgPMTChartHelper::DrawTransferoCurves( CDB_TecBox *pclTransferoReference, double dPmanSI, double dqNSI, CRankEx &clTransferoTecBoxList, CPMInputUser *pclPMInputUser )
{
	if( NULL == pclTransferoReference || 0 == clTransferoTecBoxList.GetCount() || NULL == pclPMInputUser )
	{
		ASSERT_RETURN;
	}

	// Scan all Transfero TecBox to extract min. and max. values.
	std::wstring str;
	LPARAM itemdata;
	double dXMinqNSI = DBL_MAX;
	double dXMaxqNSI = 0;
	double dYMinPmanSI = DBL_MAX;
	double dYMaxPmanSI = 0;
	int iNbrTransferoTecBox = clTransferoTecBoxList.GetCount();
	CDB_TecBox **parTransferoTecBoxList = (CDB_TecBox **)malloc( iNbrTransferoTecBox * sizeof( CDB_TecBox *) );
	CDB_TBCurve **parTransferoTecBoxCurves = (CDB_TBCurve **)malloc( iNbrTransferoTecBox * sizeof( CDB_TBCurve *) );

	if( NULL == parTransferoTecBoxList || NULL == parTransferoTecBoxCurves )
	{
		if( NULL != parTransferoTecBoxList )
		{
			free( parTransferoTecBoxList );
		}

		if( NULL != parTransferoTecBoxCurves )
		{
			free( parTransferoTecBoxCurves );
		}

		return;
	}

	int iCount = 0;

	for( BOOL bContinue = clTransferoTecBoxList.GetFirst( str, itemdata); TRUE == bContinue; bContinue = clTransferoTecBoxList.GetNext( str, itemdata ) )
	{
		CDB_TecBox *pclCurrentTransferoTecBox = dynamic_cast<CDB_TecBox *>( ( CData *)itemdata );

		if( NULL == pclCurrentTransferoTecBox )
		{
			continue;
		}

		if( _NULL_IDPTR == pclCurrentTransferoTecBox->GetPumpComprCurveIDPtr() || NULL == pclCurrentTransferoTecBox->GetPumpComprCurveIDPtr().MP
			|| NULL == dynamic_cast<CDB_TBCurve *>( pclCurrentTransferoTecBox->GetPumpComprCurveIDPtr().MP ) )
		{
			continue;
		}

		parTransferoTecBoxList[iCount] = pclCurrentTransferoTecBox;

		CDB_TBCurve *pclTBCurve = dynamic_cast<CDB_TBCurve *>( pclCurrentTransferoTecBox->GetPumpComprCurveIDPtr().MP );
		parTransferoTecBoxCurves[iCount] = pclTBCurve;

		if( pclTBCurve->GetqNmin() < dXMinqNSI )
		{
			dXMinqNSI = pclTBCurve->GetqNmin();
		}

		if( pclTBCurve->GetqNmax() > dXMaxqNSI )
		{
			dXMaxqNSI = pclTBCurve->GetqNmax();
		}

		if( pclTBCurve->GetPmin() < dYMinPmanSI )
		{
			dYMinPmanSI = pclTBCurve->GetPmin();
		}

		if( pclTBCurve->GetPmax() > dYMaxPmanSI )
		{
			dYMaxPmanSI = pclTBCurve->GetPmax();
		}

		iCount++;

	}

	if( DBL_MAX == dXMinqNSI || 0 == dXMaxqNSI || DBL_MAX == dYMinPmanSI || 0 == dYMaxPmanSI )
	{
		free( parTransferoTecBoxList );
		free( parTransferoTecBoxCurves );
		return;
	}

	// Convert these values in custom unit.
	double dXMinqNCU = CDimValue::SItoCU( _U_FLOW, dXMinqNSI );
	double dXMaxqNCU = CDimValue::SItoCU( _U_FLOW, dXMaxqNSI );
	double dYMinPmanCU = CDimValue::SItoCU( _U_PRESSURE, dYMinPmanSI );
	double dYMaxPmanCU = CDimValue::SItoCU( _U_PRESSURE, dYMaxPmanSI );

	m_TChart.GetAspect().SetView3D( FALSE );

	m_TChart.GetWalls().GetBack().GetBrush().GetGradient().SetVisible( FALSE );
	m_TChart.GetWalls().GetBack().SetColor( DLGPMTCHARTHELPER_WALLBACKCOLOR );
	m_TChart.GetWalls().GetBack().SetTransparent( FALSE );

	// Set the graph name.
	m_TChart.GetHeader().GetFont().SetCharset( ANSI_CHARSET );
	m_TChart.GetHeader().GetFont().SetColor( _BLACK );
	m_TChart.GetHeader().GetFont().SetName( _T("Arial") );
	m_TChart.GetHeader().GetFont().SetBold( TRUE );
	m_TChart.GetHeader().GetFont().GetShadow().SetVisible( FALSE );

	CString strFamilyName = pclTransferoReference->GetFamily();
	CString strTecBoxGraphName;
	strTecBoxGraphName.Format( _T("%s...%i"), strFamilyName, pclTransferoReference->GetNbrPumpCompressor() );
	m_TChart.GetHeader().GetText().SetText( strTecBoxGraphName );

	m_TChart.GetPanel().GetGradient().SetVisible( FALSE );
	m_TChart.GetPanel().SetColor( DLGPMTCHARTHELPER_PANELCOLOR );
	m_TChart.GetPanel().SetBevelOuter( FALSE );
	m_TChart.GetHover().SetVisible( FALSE );

	m_TChart.GetLegend().SetWidth( DLGPMTCHARTHELPER_TRANSFERO_LEGENDWIDTH );
	m_TChart.GetLegend().SetResizeChart( TRUE );
	m_TChart.GetLegend().SetVisible( TRUE );
	m_TChart.GetLegend().GetDividingLines().SetWidth( 0 );
	m_TChart.GetLegend().GetFont().SetName( _T("Arial") );
	m_TChart.GetLegend().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetShadow().SetTransparency( 0 );
	m_TChart.GetLegend().GetSymbol().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetTitle().SetVisible( FALSE );
	m_TChart.GetLegend().SetColumnWidthAuto( FALSE );
	m_TChart.GetLegend().SetColumnWidths( 0, 20 );
	m_TChart.GetLegend().SetColumnWidths( 1, DLGPMTCHARTHELPER_TRANSFERO_LEGENDWIDTH - 40 );
	m_TChart.GetLegend().GetFrame().SetVisible( FALSE );

	m_TChart.GetZoom().SetEnable( TRUE );
	m_TChart.GetZoom().SetAnimated( TRUE );
	m_TChart.GetZoom().SetAnimatedSteps( 5 );
	m_TChart.GetZoom().SetDirection( tzdBoth );

	////////////////////////////////////////////////////////////////////////////////////////
	// Bottom axis
	m_TChart.GetAxis().GetBottom().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetBottom().GetAxisPen().SetVisible( FALSE );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetBottom().SetMaximum( dXMaxqNCU );
	m_TChart.GetAxis().GetBottom().SetMaximumOffset( 20 );
	m_TChart.GetAxis().GetBottom().SetMinimum( 0 );
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetColor( DLGPMTCHARTHELPER_MINORGRIDCOLOR );	// clSilver
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetVisible( TRUE );
	m_TChart.GetAxis().GetBottom().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetBottom().SetMinorTickLength( 0 );
	m_TChart.GetAxis().GetBottom().GetTicks().SetColor( _BLACK );
	m_TChart.GetAxis().GetBottom().GetTicksInner().SetVisible( FALSE );

	// Set the X axes unit.
	CString strBottomTitle;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	strBottomTitle.Format( _T("qN [%s]"), pUnitDB->GetNameOfDefaultUnit( _U_FLOW ).c_str() );
	m_TChart.GetAxis().GetBottom().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetTitle().SetCaption( strBottomTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Left axis.
	UnitDesign_struct rUNitDesign = CDimValue::AccessUDB()->GetDefaultUnit( _U_PRESSURE );
	CString strFormat = ( rUNitDesign.Conv >= 1e5 ) ? _T("#,##0.00" ) : _T("#,##0." );
	
	m_TChart.GetAxis().GetLeft().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetLeft().GetAxisPen().SetVisible( FALSE );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetLeft().SetMaximum( dYMaxPmanCU );
	m_TChart.GetAxis().GetLeft().SetMaximumOffset( 16 );
	m_TChart.GetAxis().GetLeft().SetMinimum( dYMinPmanCU );
	m_TChart.GetAxis().GetLeft().SetMinimumOffset( 16 );
	m_TChart.GetAxis().GetLeft().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetLeft().SetIncrement( 1.0 );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetLeft().GetLabels().SetValueFormat( strFormat );
	m_TChart.GetAxis().GetLeft().GetLabels().SetSeparation( 2 );

	// Set the Y axes unit.
	CString strLeftTitle;
	strLeftTitle.Format( _T("Pman [%s]"), pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	m_TChart.GetAxis().GetLeft().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetTitle().SetCaption( strLeftTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Right axis.
	m_TChart.GetAxis().GetRight().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetRight().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Top axis.
	m_TChart.GetAxis().GetTop().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetTop().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Defines colors.
	COLORREF arColors[5] = { DLGPMTCHARTHELPER_TRANSFERO_CURVE1, DLGPMTCHARTHELPER_TRANSFERO_CURVE2, DLGPMTCHARTHELPER_TRANSFERO_CURVE3,
			DLGPMTCHARTHELPER_TRANSFERO_CURVE4, DLGPMTCHARTHELPER_TRANSFERO_CURVE5 };

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Defines all curves.
	int iLoopSeries = 0;
	int iLoopColor = 0;
	int iLegendNumber = 0;

	for( int iLoopCurve = 0; iLoopCurve < iNbrTransferoTecBox; iLoopCurve++ )
	{
		CCurveFitter *pclCurveFitter = parTransferoTecBoxCurves[iLoopCurve]->GetpCurve();

		if( NULL == pclCurveFitter )
		{
			continue;
		}

		m_TChart.AddSeries( scLine );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetMarks().GetArrow().SetVisible( TRUE );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetMarks().GetCallout().GetBrush().SetColor( _BLACK );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetMarks().GetCallout().GetArrow().SetVisible( TRUE );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetMarks().SetVisible( FALSE );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).SetPercentFormat( _T("##0.##,%") );

		// Set the series title.
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).SetTitle( parTransferoTecBoxList[iLoopCurve]->GetName() );

		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetXValues().SetName( _T("X") );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetXValues().SetOrder( loAscending );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetYValues().SetName( _T("Y") );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetYValues().SetOrder( loNone );

		// Change color for each curve.
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).SetColor( arColors[iLoopColor] );

		// Set width and style of the line between points (the lines in the legend will be the same).
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetAsLine().GetLinePen().SetWidth( 2 );
		m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetAsLine().GetLinePen().SetEndStyle( esSquare );
		iLegendNumber++;

		// If 'pmam_valve' exist...
		if( 2 == pclCurveFitter->GetFuncNumber() )
		{
			iLoopSeries++;
			m_TChart.AddSeries( scLine );
			m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetMarks().SetVisible( FALSE );
			m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetXValues().SetOrder( loAscending );
			m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetYValues().SetOrder( loNone );
			m_TChart.GetSeriesList().GetItems( iLoopSeries ).SetColor( arColors[iLoopColor] );
			m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetAsLine().GetLinePen().SetWidth( 2 );
			m_TChart.GetSeriesList().GetItems( iLoopSeries ).GetAsLine().GetLinePen().SetEndStyle( esSquare );
			m_TChart.GetSeriesList().GetItems( iLoopSeries ).SetShowInLegend( FALSE );
		}

		iLoopSeries++;
		iLoopColor++;

		if( iLoopColor > 4 )
		{
			iLoopColor = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Draw all curves.
	iLoopSeries = 0;
	iLoopColor = 0;

	for( int iLoopCurve = 0; iLoopCurve < iNbrTransferoTecBox; ++iLoopCurve )
	{
		CCurveFitter *pclCurveFitter = parTransferoTecBoxCurves[iLoopCurve]->GetpCurve();

		if( NULL == pclCurveFitter )
		{
			continue;
		}

		double dPmanMaxSI = parTransferoTecBoxCurves[iLoopCurve]->GetPmax();
		double dPmanMinSI = parTransferoTecBoxCurves[iLoopCurve]->GetPmin();
		double dPmanMaxCU = CDimValue::SItoCU( _U_PRESSURE, dPmanMaxSI );
		double dPmanMinCU = CDimValue::SItoCU( _U_PRESSURE, dPmanMinSI );

		bool bPmanValveExist = ( 2 == pclCurveFitter->GetFuncNumber() ) ? true : false;

		int iNbrOfPtToDraw = 15;
		double dXSI = parTransferoTecBoxCurves[iLoopCurve]->GetqNmin();
		double dStep = ( parTransferoTecBoxCurves[iLoopCurve]->GetqNmax() - dXSI ) / (double)( iNbrOfPtToDraw );

		double dLastXCU = DBL_MAX;
		double dLastPmamPumpCU = DBL_MAX;

		// Pman_pump [& pman_valve if exist].
		for( int iLoopPoint = 0; iLoopPoint < iNbrOfPtToDraw; iLoopPoint++ )
		{
			double dPmanPumpSI = pclCurveFitter->GetValue( dXSI );
			double dPmanValveSI = ( true == bPmanValveExist ) ? pclCurveFitter->GetValue( dXSI, 1 ) : 0.0;

			double dXCU = CDimValue::SItoCU( _U_FLOW, dXSI );
			double dPmanPumpCU = CDimValue::SItoCU( _U_PRESSURE, dPmanPumpSI );
			double dPmanValveCU = ( true == bPmanValveExist ) ? CDimValue::SItoCU( _U_PRESSURE, dPmanValveSI ) : 0.0;

			if( dPmanPumpCU > dPmanMaxCU )
			{
				dPmanPumpCU = dPmanMaxCU;
			}

			if( dPmanPumpCU < dPmanMinCU )
			{
				dPmanPumpCU = dPmanMinCU;
			}

			if( true == bPmanValveExist && dPmanValveCU < dPmanMinCU )
			{
				dPmanValveCU = dPmanMinCU;
			}

			dLastXCU = dXCU;
			dLastPmamPumpCU = dPmanPumpCU;

			if( true == bPmanValveExist && dPmanPumpCU < dPmanValveCU )
			{
				// Don't draw curve if pman_pump and pmap_valve reach the same value.
				dLastXCU = DBL_MAX;
				dLastPmamPumpCU = DBL_MAX;
				break;
			}

			if( 0 == iLoopPoint )
			{
				// First horizontal line.
				m_TChart.GetSeriesList().GetItems( iLoopSeries ).AddXY( 0, dPmanPumpCU, _T(""), arColors[iLoopColor] );

				if( true == bPmanValveExist )
				{
					m_TChart.GetSeriesList().GetItems( iLoopSeries + 1 ).AddXY( 0, dPmanValveCU, _T(""), arColors[iLoopColor] );
				}
			}

			m_TChart.GetSeriesList().GetItems( iLoopSeries ).AddXY( dXCU, dPmanPumpCU, _T(""), arColors[iLoopColor] );

			if( true == bPmanValveExist )
			{
				m_TChart.GetSeriesList().GetItems( iLoopSeries + 1 ).AddXY( dXCU, dPmanValveCU, _T(""), arColors[iLoopColor] );
			}

			if( iLoopPoint < ( iNbrOfPtToDraw - 2 ) )
			{
				dXSI += dStep;
			}
			else
			{
				dXSI = parTransferoTecBoxCurves[iLoopCurve]->GetqNmax();
			}
		}

		if( true == bPmanValveExist && DBL_MAX != dLastXCU && DBL_MAX != dLastPmamPumpCU )
		{
			m_TChart.GetSeriesList().GetItems( iLoopSeries + 1 ).AddXY( dLastXCU, dLastPmamPumpCU, _T(""), arColors[iLoopColor] );
		}

		iLoopSeries += ( false == bPmanValveExist ) ? 1 : 2;
		iLoopColor++;

		if( iLoopColor > 4 )
		{
			iLoopColor = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Add the working point.
	int iLastSeries = m_TChart.GetSeriesCount();
	m_TChart.AddSeries( scBubble );

	m_dWorkingPointqNSI = dqNSI;
	double dqNCU = CDimValue::SItoCU( _U_FLOW, dqNSI );

	m_dWorkingPointPmanSI = dPmanSI;
	double dPmanCU = CDimValue::SItoCU( _U_PRESSURE, dPmanSI );
	double dRadius = ( dYMaxPmanCU - dYMinPmanCU ) / 40.0;

	m_TChart.GetSeriesList().GetItems( iLastSeries ).SetTitle( TASApp.LoadLocalizedString( IDS_SELECTPM_WORKPOINT ) );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).SetColor( DLGPMTCHARTHELPER_WORKPOINTLEGENDITEMCOLOR );

	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetMarks().SetVisible( FALSE );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).SetPercentFormat( _T("##0.##,%") );

	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().GetPointer().SetHorizontalSize( 16 );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().GetPointer().SetInflateMargins( TRUE );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().GetPointer().GetPen().SetVisible( FALSE );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().GetPointer().SetStyle( psCircle );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().GetPointer().SetVerticalSize( 16 );

	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetXValues().SetName( _T("X") );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetXValues().SetOrder( loNone );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetYValues().SetName( _T("Y") );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetYValues().SetOrder( loNone );

	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().GetRadiusValues().SetName( _T("Radius") );
	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().GetRadiusValues().SetOrder( loNone );

	m_TChart.GetSeriesList().GetItems( iLastSeries ).GetAsBubble().AddBubble( dqNCU, dPmanCU, dRadius, _T(""), DLGPMTCHARTHELPER_WORKPOINTLEGENDSYMBOLCOLOR );
	iLegendNumber++;

	////////////////////////////////////////////////////////////////////////////////////////

	// We add 2 positions for the 'qN' and 'Pman' values just below the working point.
	int iFirstPos = ( m_TChart.GetHeight() - ( ( iLegendNumber + 2 ) * 14 ) ) / 2;
	int iNextPos = iFirstPos;

	// We invert the legend for the curves but we let the 'Workpoint' at the good position.
	//   Ex: Legend index: 0; TV 4.1 EH		-> TV 14.1 EH
	//       Legend index: 1; TV 6.1 EH		-> TV 10.1 EH
	//       Legend index: 2; TV 8.1 EH		-> TV 8.1 EH
	//       Legend index: 3; TV 10.1 EH	-> TV 6.1 EH
	//       Legend index: 4; TV 14.1 EH	-> TV 4.1 EH
	//       Legend index: 5; Workpoint		-> Workpoint

	for( int iLoopLegend = 0; iLoopLegend < iLegendNumber; iLoopLegend++ )
	{
		if( iLoopLegend < iLegendNumber - 1 )
		{
			// Permute curve legend.
			m_rLegendPosInfo.m_mapIndexOrder.insert( std::make_pair( iLoopLegend, iLegendNumber - 2 - iLoopLegend ) );
		}
		else
		{
			// Do not change the 'Workpoint'.
			m_rLegendPosInfo.m_mapIndexOrder.insert( std::make_pair( iLoopLegend, iLoopLegend ) );
		}

		// Prepare the position of legend items.
		m_rLegendPosInfo.m_mapTopPos.insert( std::make_pair( iLoopLegend, iNextPos ) );
		iNextPos += 14;
	}
	
	m_iNextLegendItemYPos = iNextPos;
	m_eCurrentDeviceType = Transfero;

	free( parTransferoTecBoxList );
	free( parTransferoTecBoxCurves );
}

void CDlgPMTChartHelper::DrawCompressoCurves( CDB_TecBox *pclCompressoReference, double dPmanSI, double dqNSI, CRankEx &clCompressoTecBoxList, CPMInputUser *pclPMInputUser )
{
	if( NULL == pclCompressoReference || 0 == clCompressoTecBoxList.GetCount() || NULL == pclPMInputUser )
	{
		ASSERT_RETURN;
	}

	// Scan all curves to extract min. and max. values.
	std::wstring str;
	LPARAM itemdata;
	double dXMinqNSI = DBL_MAX;
	double dXMaxqNSI = 0;
	double dYMinPmanSI = DBL_MAX;
	double dYMaxPmanSI = 0;
	int iNbrCompressoCurve = clCompressoTecBoxList.GetCount();
	CDB_TBCurve **parCompressoCurves = (CDB_TBCurve **)malloc( iNbrCompressoCurve * sizeof( CDB_TBCurve *) );

	if( NULL == parCompressoCurves )
	{
		return;
	}

	int iCount = 0;

	for( BOOL bContinue = clCompressoTecBoxList.GetFirst( str, itemdata); TRUE == bContinue; bContinue = clCompressoTecBoxList.GetNext( str, itemdata ) )
	{
		CDB_TBCurve *pclCurrentCompressoCurve = dynamic_cast<CDB_TBCurve *>( ( CData *)itemdata );

		if( NULL == pclCurrentCompressoCurve )
		{
			continue;
		}

		parCompressoCurves[iCount] = pclCurrentCompressoCurve;

		if( pclCurrentCompressoCurve->GetqNmin() < dXMinqNSI )
		{
			dXMinqNSI = pclCurrentCompressoCurve->GetqNmin();
		}

		if( pclCurrentCompressoCurve->GetqNmax() > dXMaxqNSI )
		{
			dXMaxqNSI = pclCurrentCompressoCurve->GetqNmax();
		}

		if( pclCurrentCompressoCurve->GetPmin() < dYMinPmanSI )
		{
			dYMinPmanSI = pclCurrentCompressoCurve->GetPmin();
		}

		if( pclCurrentCompressoCurve->GetPmax() > dYMaxPmanSI )
		{
			dYMaxPmanSI = pclCurrentCompressoCurve->GetPmax();
		}

		iCount++;

	}

	if( DBL_MAX == dXMinqNSI || 0 == dXMaxqNSI || DBL_MAX == dYMinPmanSI || 0 == dYMaxPmanSI )
	{
		free( parCompressoCurves );
		return;
	}

	// Convert these values in custom unit.
	double dXMinqNCU = CDimValue::SItoCU( _U_FLOW, dXMinqNSI );
	double dXMaxqNCU = CDimValue::SItoCU( _U_FLOW, dXMaxqNSI );
	double dYMinPmanCU = CDimValue::SItoCU( _U_PRESSURE, dYMinPmanSI );
	double dYMaxPmanCU = CDimValue::SItoCU( _U_PRESSURE, dYMaxPmanSI );

	m_TChart.GetAspect().SetView3D( FALSE );

	m_TChart.GetWalls().GetBack().GetBrush().GetGradient().SetVisible( FALSE );
	m_TChart.GetWalls().GetBack().SetColor( DLGPMTCHARTHELPER_WALLBACKCOLOR );
	m_TChart.GetWalls().GetBack().SetTransparent( FALSE );

	// Set the graph name.
	m_TChart.GetHeader().GetFont().SetCharset( ANSI_CHARSET );
	m_TChart.GetHeader().GetFont().SetColor( _BLACK );
	m_TChart.GetHeader().GetFont().SetName( _T("Arial") );
	m_TChart.GetHeader().GetFont().SetBold( TRUE );
	m_TChart.GetHeader().GetFont().GetShadow().SetVisible( FALSE );

	CString strTecBoxGraphName = pclCompressoReference->GetFamily();
	m_TChart.GetHeader().GetText().SetText( strTecBoxGraphName );

	m_TChart.GetPanel().GetGradient().SetVisible( FALSE );
	m_TChart.GetPanel().SetColor( DLGPMTCHARTHELPER_PANELCOLOR );
	m_TChart.GetPanel().SetBevelOuter( FALSE );
	m_TChart.GetHover().SetVisible( FALSE );

	m_TChart.GetLegend().SetWidth( DLGPMTCHARTHELPER_COMPRESSO_LEGENDWIDTH );
	m_TChart.GetLegend().SetResizeChart( TRUE );
	m_TChart.GetLegend().SetVisible( TRUE );
	m_TChart.GetLegend().GetDividingLines().SetWidth( 0 );
	m_TChart.GetLegend().GetFont().SetName( _T("Arial") );
	m_TChart.GetLegend().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetShadow().SetTransparency( 0 );
	m_TChart.GetLegend().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetSymbol().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetTitle().SetVisible( FALSE );
	m_TChart.GetLegend().SetColumnWidthAuto( FALSE );
	m_TChart.GetLegend().SetColumnWidths( 0, 20 );
	m_TChart.GetLegend().SetColumnWidths( 1, DLGPMTCHARTHELPER_COMPRESSO_LEGENDWIDTH - 40 );
	m_TChart.GetLegend().GetFrame().SetVisible( FALSE );

	m_TChart.GetZoom().SetEnable( TRUE );
	m_TChart.GetZoom().SetAnimated( TRUE );
	m_TChart.GetZoom().SetAnimatedSteps( 5 );
	m_TChart.GetZoom().SetDirection( tzdBoth );

	m_TChart.GetScroll().SetEnable( pmBoth );

	////////////////////////////////////////////////////////////////////////////////////////
	// Bottom axis
	m_TChart.GetAxis().GetBottom().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetBottom().GetAxisPen().SetVisible( FALSE );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetBottom().SetMaximum( dXMaxqNCU );
	m_TChart.GetAxis().GetBottom().SetMaximumOffset( 20 );
	m_TChart.GetAxis().GetBottom().SetMinimum( 0 );
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetColor( DLGPMTCHARTHELPER_MINORGRIDCOLOR );	// clSilver
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetVisible( TRUE );
	m_TChart.GetAxis().GetBottom().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetBottom().SetMinorTickLength( 0 );
	m_TChart.GetAxis().GetBottom().GetTicks().SetColor( _BLACK );
	m_TChart.GetAxis().GetBottom().GetTicksInner().SetVisible( FALSE );

	// Set the X axes unit.
	CString strBottomTitle;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	strBottomTitle.Format( _T("qN [%s]"), pUnitDB->GetNameOfDefaultUnit( _U_FLOW ).c_str() );
	m_TChart.GetAxis().GetBottom().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetTitle().SetCaption( strBottomTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Left axis.
	UnitDesign_struct rUNitDesign = CDimValue::AccessUDB()->GetDefaultUnit( _U_PRESSURE );
	CString strFormat = ( rUNitDesign.Conv >= 1e5 ) ? _T("#,##0.00" ) : _T("#,##0." );

	m_TChart.GetAxis().GetLeft().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetLeft().GetAxisPen().SetVisible( FALSE );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetLeft().SetMaximum( dYMaxPmanCU );
	m_TChart.GetAxis().GetLeft().SetMaximumOffset( 16 );
	m_TChart.GetAxis().GetLeft().SetMinimum( dYMinPmanCU );
	m_TChart.GetAxis().GetLeft().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetLeft().SetIncrement( 1.0 );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetLeft().GetLabels().SetValueFormat( strFormat );
	m_TChart.GetAxis().GetLeft().GetLabels().SetSeparation( 2 );
	m_TChart.GetAxis().GetLeft().GetLabels();

	// Set the Y axes unit.
	CString strLeftTitle;
	strLeftTitle.Format( _T("Pman [%s]"), pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	m_TChart.GetAxis().GetLeft().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetTitle().SetCaption( strLeftTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Right axis.
	m_TChart.GetAxis().GetRight().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetRight().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Top axis.
	m_TChart.GetAxis().GetTop().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetTop().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Defines colors (light blue, yellow, green, orange, dark blue).
	COLORREF arColors[5] = { DLGPMTCHARTHELPER_COMPRESSO_CURVE1, DLGPMTCHARTHELPER_COMPRESSO_CURVE2, DLGPMTCHARTHELPER_COMPRESSO_CURVE3,
			DLGPMTCHARTHELPER_COMPRESSO_CURVE4, DLGPMTCHARTHELPER_COMPRESSO_CURVE5 };

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Defines all curves.
	int iLoopColor = 0;

	for( int iLoopCurve = 0; iLoopCurve < iNbrCompressoCurve; ++iLoopCurve )
	{
		CDB_TBCurve *pclCurrentCompressoCurve = parCompressoCurves[iLoopCurve];
		CCurveFitter *pclCurveFitter = pclCurrentCompressoCurve->GetpCurve();

		if( NULL == pclCurveFitter )
		{
			continue;
		}

		m_TChart.AddSeries( scLine );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetMarks().GetArrow().SetVisible( TRUE );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetMarks().GetCallout().GetBrush().SetColor( _BLACK );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetMarks().GetCallout().GetArrow().SetVisible( TRUE );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetMarks().SetVisible( FALSE );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).SetPercentFormat( _T("##0.##,%") );

		// Set the series title.
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).SetTitle( pclCurrentCompressoCurve->GetCurveName() );

		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetXValues().SetName( _T("X") );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetXValues().SetOrder( loAscending );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetYValues().SetName( _T("Y") );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetYValues().SetOrder( loNone );

		// Change color for each curve.
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).SetColor( arColors[iLoopColor] );

		// Set width and style of the line between points (the lines in the legend will be the same).
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetAsLine().GetLinePen().SetWidth( 2 );
		m_TChart.GetSeriesList().GetItems( iLoopCurve ).GetAsLine().GetLinePen().SetEndStyle( esSquare );

		iLoopColor++;

		if( iLoopColor > 4 )
		{
			iLoopColor = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Draw all curves.
	for( int iLoopCurve = iNbrCompressoCurve; iLoopCurve > 0; iLoopCurve-- )
	{
		// The last series is painted the last. If we want that the first curve overlapped all the others, we must draw it in the last
		// series.
		int iSeries = iLoopCurve - 1;
		CDB_TBCurve *pclCurrentCompressoCurve = parCompressoCurves[iSeries];
		CCurveFitter *pclCurveFitter = pclCurrentCompressoCurve->GetpCurve();

		if( NULL == pclCurveFitter )
		{
			continue;
		}

		int iNbrOfPtToDraw = 20;
		double dXSI = pclCurrentCompressoCurve->GetqNmin();
		double dStep = ( pclCurrentCompressoCurve->GetqNmax() - dXSI ) / (double)( iNbrOfPtToDraw );

		unsigned long ulColor = m_TChart.GetSeriesList().GetItems( iSeries ).GetColor();

		for( int iLoopPoint = 0; iLoopPoint < iNbrOfPtToDraw; iLoopPoint++ )
		{
			double dYSI = pclCurveFitter->GetValue( dXSI );

			double dXCU = CDimValue::SItoCU( _U_FLOW, dXSI );
			double dYCU = CDimValue::SItoCU( _U_PRESSURE, dYSI );

			if( 0 == iLoopPoint )
			{
				// First horizontal line.
				m_TChart.GetSeriesList().GetItems( iSeries ).AddXY( 0, dYCU, _T(""), ulColor );
			}

			m_TChart.GetSeriesList().GetItems( iSeries ).AddXY( dXCU, dYCU, _T(""), ulColor );

			if( iLoopPoint < ( iNbrOfPtToDraw - 2 ) )
			{
				dXSI += dStep;
			}
			else if( iLoopPoint < ( iNbrOfPtToDraw - 1 ) )
			{
				dXSI = pclCurrentCompressoCurve->GetqNmax();
			}
			else
			{
				// Add final vertical line.
				m_TChart.GetSeriesList().GetItems( iSeries ).AddXY( dXCU, 0, _T(""), ulColor );
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Add the working point.
	m_TChart.AddSeries( scBubble );
	
	m_dWorkingPointqNSI = dqNSI;
	double dqNCU = CDimValue::SItoCU( _U_FLOW, dqNSI );

	m_dWorkingPointPmanSI = dPmanSI;
	double dPmanCU = CDimValue::SItoCU( _U_PRESSURE, dPmanSI );
	double dRadius = ( dYMaxPmanCU - dYMinPmanCU ) / 40.0;

	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).SetTitle( TASApp.LoadLocalizedString( IDS_SELECTPM_WORKPOINT ) );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).SetColor( DLGPMTCHARTHELPER_WORKPOINTLEGENDITEMCOLOR );

	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetMarks().SetVisible( FALSE );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).SetPercentFormat( _T("##0.##,%") );

	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().GetPointer().SetHorizontalSize( 16 );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().GetPointer().SetInflateMargins( TRUE );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().GetPointer().GetPen().SetVisible( FALSE );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().GetPointer().SetStyle( psCircle );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().GetPointer().SetVerticalSize( 16 );

	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetXValues().SetName( _T("X") );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetXValues().SetOrder( loNone );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetYValues().SetName( _T("Y") );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetYValues().SetOrder( loNone );

	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().GetRadiusValues().SetName( _T("Radius") );
	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().GetRadiusValues().SetOrder( loNone );

	m_TChart.GetSeriesList().GetItems( iNbrCompressoCurve ).GetAsBubble().AddBubble( dqNCU, dPmanCU, dRadius, _T(""), DLGPMTCHARTHELPER_WORKPOINTLEGENDSYMBOLCOLOR );

	////////////////////////////////////////////////////////////////////////////////////////

	// Prepare the position of legend items.
	int iLoopEnd = iNbrCompressoCurve + 1;

	// We add 2 positions for the 'qN' and 'Pman' values just below the working point.
	int iFirstPos = ( m_TChart.GetHeight() - ( ( iLoopEnd + 2 ) * 14 ) ) / 2;
	int iNextPos = iFirstPos;

	// We invert the legend for the curves but we let the 'Workpoint' at the good position.
	//   Ex: Legend index: 0; C 10.1 Connect	-> C 15.2 Connect
	//       Legend index: 1; C 10.2 Connect	-> C 15.1 Connect
	//       Legend index: 2; C 15.1 Connect	-> C 10.2 Connect
	//       Legend index: 3; C 15.2 Connect	-> C 10.1 Connect
	//       Legend index: 4; Workpoint			-> Workpoint

	for( int iLoopCurve = 0; iLoopCurve < iLoopEnd; iLoopCurve++ )
	{
		if( iLoopCurve < iLoopEnd - 1 )
		{
			// Permute curve legend.
			m_rLegendPosInfo.m_mapIndexOrder.insert( std::make_pair( iLoopCurve, iLoopEnd - 2 - iLoopCurve ) );
		}
		else
		{
			// Do not change the 'Workpoint'.
			m_rLegendPosInfo.m_mapIndexOrder.insert( std::make_pair( iLoopCurve, iLoopCurve ) );
		}

		// Prepare the position of legend items.
		m_rLegendPosInfo.m_mapTopPos.insert( std::make_pair( iLoopCurve, iNextPos ) );
		iNextPos += 14;
	}

	m_iNextLegendItemYPos = iNextPos;
	m_eCurrentDeviceType = Compresso;

	free( parCompressoCurves );
}

// HYS-599: Now we draw also curves for Compresso CX.
void CDlgPMTChartHelper::DrawCompressoCXCurves( CDB_TecBox *pclCompressoReference, double dPmanSI, double dqNSI, CRankEx &clCompressoTecBoxList, CPMInputUser *pclPMInputUser, int iNbrOfDevices )
{
	if( NULL == pclCompressoReference || 0 == clCompressoTecBoxList.GetCount() || NULL == pclPMInputUser || iNbrOfDevices <= 0 )
	{
		ASSERT_RETURN;
	}

	// Scan all curves to extract min. and max. values.
	std::wstring str;
	LPARAM itemdata;
	double dXMinqNSI = DBL_MAX;
	double dXMaxqNSI = 0;
	double dYMinPmanSI = DBL_MAX;
	double dYMaxPmanSI = 0;
	int iNbrCompressoCurve = clCompressoTecBoxList.GetCount();
	CDB_TBCurve **parCompressoCurves = (CDB_TBCurve **)malloc( iNbrCompressoCurve * sizeof( CDB_TBCurve *) );

	if( NULL == parCompressoCurves )
	{
		return;
	}

	int iCount = 0;

	for( BOOL bContinue = clCompressoTecBoxList.GetFirst( str, itemdata); TRUE == bContinue; bContinue = clCompressoTecBoxList.GetNext( str, itemdata ) )
	{
		CDB_TBCurve *pclCurrentCompressoCurve = dynamic_cast<CDB_TBCurve *>( ( CData *)itemdata );

		if( NULL == pclCurrentCompressoCurve )
		{
			continue;
		}

		parCompressoCurves[iCount] = pclCurrentCompressoCurve;

		if( pclCurrentCompressoCurve->GetqNmin() < dXMinqNSI )
		{
			dXMinqNSI = pclCurrentCompressoCurve->GetqNmin();
		}

		if( pclCurrentCompressoCurve->GetqNmax() > dXMaxqNSI )
		{
			dXMaxqNSI = pclCurrentCompressoCurve->GetqNmax();
		}

		if( pclCurrentCompressoCurve->GetPmin() < dYMinPmanSI )
		{
			dYMinPmanSI = pclCurrentCompressoCurve->GetPmin();
		}

		// For Compresso CX we need to check the first curve to get the Pmax to display.
		if( pclCurrentCompressoCurve->GetBoundedY( 0.0, 0 ) > dYMaxPmanSI )
		{
			dYMaxPmanSI = pclCurrentCompressoCurve->GetBoundedY( 0.0, 0 );
		}

		iCount++;

	}

	if( DBL_MAX == dXMinqNSI || 0 == dXMaxqNSI || DBL_MAX == dYMinPmanSI || 0 == dYMaxPmanSI )
	{
		free( parCompressoCurves );
		return;
	}

	// Convert these values in custom unit.
	double dXMinqNCU = CDimValue::SItoCU( _U_FLOW, dXMinqNSI );
	double dXMaxqNCU = CDimValue::SItoCU( _U_FLOW, dXMaxqNSI );
	double dYMinPmanCU = CDimValue::SItoCU( _U_PRESSURE, dYMinPmanSI );
	double dYMaxPmanCU = CDimValue::SItoCU( _U_PRESSURE, dYMaxPmanSI );

	m_TChart.GetAspect().SetView3D( FALSE );

	m_TChart.GetWalls().GetBack().GetBrush().GetGradient().SetVisible( FALSE );
	m_TChart.GetWalls().GetBack().SetColor( DLGPMTCHARTHELPER_WALLBACKCOLOR );
	m_TChart.GetWalls().GetBack().SetTransparent( FALSE );

	// Set the graph name.
	m_TChart.GetHeader().GetFont().SetCharset( ANSI_CHARSET );
	m_TChart.GetHeader().GetFont().SetColor( _BLACK );
	m_TChart.GetHeader().GetFont().SetName( _T("Arial") );
	m_TChart.GetHeader().GetFont().SetBold( TRUE );
	m_TChart.GetHeader().GetFont().GetShadow().SetVisible( FALSE );

	CString strTecBoxGraphName = pclCompressoReference->GetFamily();
	m_TChart.GetHeader().GetText().SetText( strTecBoxGraphName );

	m_TChart.GetPanel().GetGradient().SetVisible( FALSE );
	m_TChart.GetPanel().SetColor( DLGPMTCHARTHELPER_PANELCOLOR );
	m_TChart.GetPanel().SetBevelOuter( FALSE );
	m_TChart.GetHover().SetVisible( FALSE );

	m_TChart.GetLegend().SetWidth( DLGPMTCHARTHELPER_COMPRESSOCX_LEGENDWIDTH );
	m_TChart.GetLegend().SetResizeChart( TRUE );
	m_TChart.GetLegend().SetVisible( TRUE );
	m_TChart.GetLegend().GetDividingLines().SetWidth( 0 );
	m_TChart.GetLegend().GetFont().SetName( _T("Arial") );
	m_TChart.GetLegend().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetShadow().SetTransparency( 0 );
	m_TChart.GetLegend().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().GetSymbol().GetShadow().SetVisible( FALSE );
	m_TChart.GetLegend().SetTransparent( TRUE );
	m_TChart.GetLegend().GetTitle().SetVisible( FALSE );
	m_TChart.GetLegend().SetColumnWidthAuto( FALSE );
	m_TChart.GetLegend().SetColumnWidths( 0, 20 );
	m_TChart.GetLegend().SetColumnWidths( 1, DLGPMTCHARTHELPER_COMPRESSOCX_LEGENDWIDTH - 40 );
	m_TChart.GetLegend().GetFrame().SetVisible( FALSE );

	m_TChart.GetZoom().SetEnable( TRUE );
	m_TChart.GetZoom().SetAnimated( TRUE );
	m_TChart.GetZoom().SetAnimatedSteps( 5 );
	m_TChart.GetZoom().SetDirection( tzdBoth );

	m_TChart.GetScroll().SetEnable( pmBoth );

	////////////////////////////////////////////////////////////////////////////////////////
	// Bottom axis
	m_TChart.GetAxis().GetBottom().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetBottom().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetBottom().GetAxisPen().SetVisible( FALSE );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetBottom().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetBottom().SetMaximum( dXMaxqNCU );
	m_TChart.GetAxis().GetBottom().SetMaximumOffset( 20 );
	m_TChart.GetAxis().GetBottom().SetMinimum( 0 );
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetColor( DLGPMTCHARTHELPER_MINORGRIDCOLOR );	// clSilver
	m_TChart.GetAxis().GetBottom().GetMinorGrid().SetVisible( TRUE );
	m_TChart.GetAxis().GetBottom().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetBottom().SetMinorTickLength( 0 );
	m_TChart.GetAxis().GetBottom().GetTicks().SetColor( _BLACK );
	m_TChart.GetAxis().GetBottom().GetTicksInner().SetVisible( FALSE );

	// Set the X axes unit.
	CString strBottomTitle;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	strBottomTitle.Format( _T("qN [%s]"), pUnitDB->GetNameOfDefaultUnit( _U_FLOW ).c_str() );
	m_TChart.GetAxis().GetBottom().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetBottom().GetTitle().SetCaption( strBottomTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Left axis.
	UnitDesign_struct rUNitDesign = CDimValue::AccessUDB()->GetDefaultUnit( _U_PRESSURE );
	CString strFormat = ( rUNitDesign.Conv >= 1e5 ) ? _T("#,##0.00" ) : _T("#,##0." );

	m_TChart.GetAxis().GetLeft().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetLeft().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetLeft().GetAxisPen().SetVisible( FALSE );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetColor( DLGPMTCHARTHELPER_GRIDPENCOLOR );
	m_TChart.GetAxis().GetLeft().GetGridPen().SetStyle( psSolid );
	m_TChart.GetAxis().GetLeft().SetMaximum( dYMaxPmanCU );
	m_TChart.GetAxis().GetLeft().SetMaximumOffset( 16 );
	m_TChart.GetAxis().GetLeft().SetMinimum( dYMinPmanCU );
	m_TChart.GetAxis().GetLeft().SetMinorTickCount( 0 );
	m_TChart.GetAxis().GetLeft().SetIncrement( 1.0 );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetLabels().GetFont().GetShadow().SetVisible( FALSE );
	m_TChart.GetAxis().GetLeft().GetLabels().SetValueFormat( strFormat );
	m_TChart.GetAxis().GetLeft().GetLabels().SetSeparation( 2 );
	m_TChart.GetAxis().GetLeft().GetLabels();

	// Set the Y axes unit.
	CString strLeftTitle;
	strLeftTitle.Format( _T("Pman [%s]"), pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str() );
	m_TChart.GetAxis().GetLeft().GetTitle().GetFont().SetName( _T("Arial") );
	m_TChart.GetAxis().GetLeft().GetTitle().SetCaption( strLeftTitle );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Right axis.
	m_TChart.GetAxis().GetRight().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetRight().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetRight().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Top axis.
	m_TChart.GetAxis().GetTop().SetAutomatic( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMaximum( FALSE );
	m_TChart.GetAxis().GetTop().SetAutomaticMinimum( FALSE );
	m_TChart.GetAxis().GetTop().SetVisible( FALSE );
	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Defines colors (dark blue, orange, light blue)
	COLORREF arColors[3] = { DLGPMTCHARTHELPER_COMPRESSOCX_CURVE1, DLGPMTCHARTHELPER_COMPRESSOCX_CURVE2, DLGPMTCHARTHELPER_COMPRESSOCX_CURVE3 };

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Defines all curves.
	int iLoopColor = 0;
	int iTotalCurve = 0;
	int iTotalColor = 0;

	for( int iLoopCurve = 0; iLoopCurve < iNbrCompressoCurve; ++iLoopCurve )
	{
		CDB_TBCurve *pclCurrentCompressoCurve = parCompressoCurves[iLoopCurve];
		
		// To display CX PXmax curve before PX in the legend, we need to inverse here.
		for( int iLoopSubCurve = pclCurrentCompressoCurve->GetFuncNumber(); iLoopSubCurve > 0 ; iLoopSubCurve-- )
		{
			m_TChart.AddSeries( scFastLine );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetMarks().GetArrow().SetVisible( TRUE );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetMarks().GetCallout().GetBrush().SetColor( _BLACK );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetMarks().GetCallout().GetArrow().SetVisible( TRUE );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetMarks().SetVisible( FALSE );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).SetPercentFormat( _T("##0.##,%") );

			// Set the series title.
			CString strSeriesTitle;

			if( 0 == ( iLoopSubCurve - 1 ) )
			{
				strSeriesTitle = pclCurrentCompressoCurve->GetCurveName();
				double dPXmax = pclCurrentCompressoCurve->GetBoundedY( 0.0, iLoopSubCurve - 1 );
				strSeriesTitle += _T(", PXmax = ") + CString( WriteCUDouble( _U_PRESSURE, dPXmax, true ) );
			}
			else
			{
				double dPX = pclCurrentCompressoCurve->GetBoundedY( 0.0, iLoopSubCurve - 1 );
				CString strPX = ( 0 == iLoopCurve ) ? _T("PX* = ") : _T("PX = ");
				strSeriesTitle = strPX + CString( WriteCUDouble( _U_PRESSURE, dPX, true ) );
			}
			
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).SetTitle( strSeriesTitle );

			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetXValues().SetName( _T("X") );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetXValues().SetOrder( loAscending );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetYValues().SetName( _T("Y") );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetYValues().SetOrder( loNone );

			// Change color for each curve.
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).SetColor( arColors[iLoopColor] );

			// Set width and style of the line between points (the lines in the legend will be the same).
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsFastLine().GetLinePen().SetWidth( 2 );
			m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsFastLine().GetLinePen().SetEndStyle( esRound );

			if( 0 == ( iLoopSubCurve - 1 ) )
			{
				m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsFastLine().GetLinePen().SetStyle( psSolid );
			}
			else if( 1 == ( iLoopSubCurve - 1 ) )
			{
				m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsFastLine().GetLinePen().SetStyle( psDash );
			}
			else if( 2 == ( iLoopSubCurve - 1 ) )
			{
				m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsFastLine().GetLinePen().SetStyle( psDot );
			}

			iTotalCurve++;
		}

		iLoopColor++;
		iTotalColor++;

		if( iLoopColor > 2 )
		{
			iLoopColor = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Draw all curves.
	iLoopColor = ( iTotalColor - 1 ) % 3;

	// The last series is painted the last. If we want that the first curve overlapped all the others, we must draw it in the last
	// series.
	int iSeries = iTotalCurve - 1;

	for( int iLoopCurve = iNbrCompressoCurve; iLoopCurve > 0 ; iLoopCurve-- )
	{
		CDB_TBCurve *pclCurrentCompressoCurve = parCompressoCurves[iLoopCurve - 1];
		CCurveFitter *pclCurveFitter = pclCurrentCompressoCurve->GetpCurve();

		if( NULL == pclCurveFitter )
		{
			continue;
		}

		for( int iLoopSubCurve = 0; iLoopSubCurve < pclCurrentCompressoCurve->GetFuncNumber(); iLoopSubCurve++ )
		{
			int iNbrOfPtToDraw = 10;
			double dXSI = pclCurrentCompressoCurve->GetqNmin();
			double dStep = ( pclCurrentCompressoCurve->GetqNmax() - dXSI ) / (double)( iNbrOfPtToDraw );

			for( int iLoopPoint = 0; iLoopPoint < iNbrOfPtToDraw; iLoopPoint++ )
			{
				double dYSI = pclCurveFitter->GetValue( dXSI, iLoopSubCurve );

				double dXCU = CDimValue::SItoCU( _U_FLOW, dXSI );
				double dYCU = CDimValue::SItoCU( _U_PRESSURE, dYSI );

				if( 0 == iLoopPoint )
				{
					// First horizontal line.
					m_TChart.GetSeriesList().GetItems( iSeries ).AddXY( 0, dYCU, _T(""), arColors[iLoopColor] );
				}

				m_TChart.GetSeriesList().GetItems( iSeries ).AddXY( dXCU, dYCU, _T(""), arColors[iLoopColor] );

				if( iLoopPoint < ( iNbrOfPtToDraw - 2 ) )
				{
					dXSI += dStep;
				}
				else if( iLoopPoint < ( iNbrOfPtToDraw - 1 ) )
				{
					dXSI = pclCurrentCompressoCurve->GetqNmax();
				}
				else
				{
					// Add final vertical line.
					m_TChart.GetSeriesList().GetItems( iSeries ).AddXY( dXCU, 0, _T(""), arColors[iLoopColor] );
				}
			}

			iSeries--;
		}

		iLoopColor--;

		if( -1 == iLoopColor )
		{
			iLoopColor = 2;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////
	// Add the working point.
	m_TChart.AddSeries( scBubble );

	// Pay attention: We can have more than one device in parallel.
	m_dWorkingPointqNSI = dqNSI / iNbrOfDevices;
	m_dWorkingPointPmanSI = dPmanSI;
	double dPmanCU = CDimValue::SItoCU( _U_PRESSURE, dPmanSI );
	double dRadius = ( dYMaxPmanCU - dYMinPmanCU ) / 40.0;
	long lTemp = m_TChart.GetHeight();

	m_TChart.GetSeriesList().GetItems( iTotalCurve ).SetTitle( TASApp.LoadLocalizedString( IDS_SELECTPM_WORKPOINT ) );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).SetColor( DLGPMTCHARTHELPER_WORKPOINTLEGENDITEMCOLOR );

	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetMarks().SetVisible( FALSE );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).SetPercentFormat( _T("##0.##,%") );

	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().GetPointer().SetHorizontalSize( 16 );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().GetPointer().SetInflateMargins( TRUE );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().GetPointer().GetPen().SetVisible( FALSE );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().GetPointer().SetStyle( psCircle );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().GetPointer().SetVerticalSize( 16 );

	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetXValues().SetName( _T("X") );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetXValues().SetOrder( loNone );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetYValues().SetName( _T("Y") );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetYValues().SetOrder( loNone );

	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().GetRadiusValues().SetName( _T("Radius") );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().GetRadiusValues().SetOrder( loNone );

	double dqNCU = CDimValue::SItoCU( _U_FLOW, m_dWorkingPointqNSI );
	m_TChart.GetSeriesList().GetItems( iTotalCurve ).GetAsBubble().AddBubble( dqNCU, dPmanCU, dRadius, _T(""), DLGPMTCHARTHELPER_WORKPOINTLEGENDSYMBOLCOLOR );

	////////////////////////////////////////////////////////////////////////////////////////

	// +1 because we have also the working point series that will be added below.
	int iLoopEnd = iTotalCurve + 1;
	
	// We add 2 positions for the 'qN' and 'Pman' values just below the working point.
	int iFirstPos = ( m_TChart.GetHeight() - ( ( iLoopEnd + 2 ) * 14 ) ) / 2;
	int iNextPos = iFirstPos;

	for( int iLoopCurve = 0; iLoopCurve < iLoopEnd; iLoopCurve++ )
	{
		// Change the index order for the legend to show the 'Working point' at the lowest position.
		if( iLoopCurve < iLoopEnd - 1 )
		{
			m_rLegendPosInfo.m_mapIndexOrder.insert( std::make_pair( iLoopCurve, iLoopCurve + 1 ) );
		}
		else
		{
			m_rLegendPosInfo.m_mapIndexOrder.insert( std::make_pair( iLoopCurve, 0 ) );
		}

		// Prepare the position of legend items.
		m_rLegendPosInfo.m_mapTopPos.insert( std::make_pair( iLoopEnd - iLoopCurve - 1, iNextPos ) );
		iNextPos += 14;
	}

	m_iNextLegendItemYPos = iNextPos;
	m_eCurrentDeviceType = CompressoCX;

	free( parCompressoCurves );
}

// To get this event, I have created a project where I have a instance of a TChart object in a dialog.
// And when right-clicking on this object you can click on 'Add Event Handler...'.
BEGIN_EVENTSINK_MAP( CDlgPMTChartHelper, CDialogEx )
	ON_EVENT( CDlgPMTChartHelper, IDC_TCHART1, 15, CDlgPMTChartHelper::OnGetLegendPosTchart1, VTS_I4 VTS_PI4 VTS_PI4 VTS_PI4 )
END_EVENTSINK_MAP()

BOOL CDlgPMTChartHelper::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rect;
	GetClientRect( &rect );

	if( FALSE == m_TChart.Create( _T("TeeChart Window"), ( GetStyle() | WS_CHILD ) ^ WS_VISIBLE, rect, this, IDC_TCHART1 ) )
	{
		return FALSE;
	}

	return TRUE;
}

void CDlgPMTChartHelper::OnGetLegendPosTchart1( long ItemIndex, long *X, long *Y, long *XColor )
{
	CString strHeader = m_TChart.GetHeader().GetText().GetText();
	strHeader.Trim();

	if( Vessel == m_eCurrentDeviceType )
	{
		// Here we change the values of left axis to have min and max value fixed with the correct values.
		if( false == m_bStaticoLeftAxisInitialized )
		{
			int iLeftCount = m_TChart.GetAxis().GetLeft().GetLabels().Count();
			std::vector<double> vecValue;
			std::vector<CString> vecText;

			for( int i = 0; i < iLeftCount; i++ )
			{
				vecValue.push_back( m_TChart.GetAxis().GetLeft().GetLabels().GetItem( i ).GetValue() );
				vecText.push_back( m_TChart.GetAxis().GetLeft().GetLabels().GetItem( i ).GetText() );
			}

			m_TChart.GetAxis().GetLeft().GetLabels().Clear();

			CString strValue = WriteCUDouble( _U_PRESSURE, m_dYMinPressSI );
			m_TChart.GetAxis().GetLeft().GetLabels().Add( m_dYMinPressCU, strValue );

			for( int i = 1; i < iLeftCount - 1; i++ )
			{
				m_TChart.GetAxis().GetLeft().GetLabels().Add( vecValue.at( i ), vecText.at( i ) );
			}

			strValue = WriteCUDouble( _U_PRESSURE, m_dYMaxPressSI );
			m_TChart.GetAxis().GetLeft().GetLabels().Add( m_dYMaxPressCU, strValue );

			m_bStaticoLeftAxisInitialized = true;

			Invalidate();
			UpdateWindow();
		}
	}

	if( false == m_bAnnotationTextDisplayed )
	{
		int iAnnotationNbr = 0;

		if( CompressoCX == m_eCurrentDeviceType )
		{
			// Set footnote text.
			m_TChart.GetTools().Add( tcAnnotate );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).SetActive( TRUE );

			long lLeft = 0;
			long lTop = m_TChart.GetCanvas().GetHeight() - 18;
			long lWidth = m_TChart.GetCanvas().GetWidth() - 5;
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetAutoSize( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetAutoSize( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetCustomPosition( TRUE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetLeft( lLeft );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTop( lTop );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetHeight( 15 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetWidth( lWidth );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFrame().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetShadow().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTextAlignment( taRightJustify );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetSize( 7 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTransparent( TRUE );
			CString str = TASApp.LoadLocalizedString( IDS_DLGPMTCHARHELPER_FOOTNOTETEXT );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetText( str );

			iAnnotationNbr++;
		}

		if( Vessel != m_eCurrentDeviceType )
		{
			long lLeft = m_TChart.GetLegend().GetLeft() + 30;

			// Set 'qN' value.
			m_TChart.GetTools().Add( tcAnnotate );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).SetActive( TRUE );

			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetAutoSize( TRUE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetAutoSize( TRUE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetCustomPosition( TRUE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetLeft( lLeft );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTop( m_iNextLegendItemYPos );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFrame().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetShadow().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTextAlignment( taLeftJustify );

			CTeeFont cLegendItemFont = m_TChart.GetLegend().GetItem( 0 ).GetFont();
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetBold( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetCharset( 2 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetColor( 0 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetDepth( 0 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetInterCharSize( 0 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetItalic( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetName( _T("Arial") );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetPixelsPerInch( cLegendItemFont.GetPixelsPerInch() );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetQuality( 2 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetSize( 8 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetStrikethrough( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetUnderline( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetShadow().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetEmboss().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetGradient().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetOutline().SetVisible( FALSE );
			
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTransparent( TRUE );
			CString str = _T("qN= ") + CString(  WriteCUDouble( _U_FLOW, m_dWorkingPointqNSI, true ) );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetText( str );

			iAnnotationNbr++;
			m_iNextLegendItemYPos += 14;

			// Set 'Pman' value.
			m_TChart.GetTools().Add( tcAnnotate );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).SetActive( TRUE );

			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetAutoSize( TRUE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetAutoSize( TRUE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetCustomPosition( TRUE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetLeft( lLeft );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTop( m_iNextLegendItemYPos );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFrame().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetShadow().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTextAlignment( taLeftJustify );

			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetBold( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetCharset( 2 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetColor( 0 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetDepth( 0 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetInterCharSize( 0 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetItalic( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetName( _T("Arial") );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetPixelsPerInch( cLegendItemFont.GetPixelsPerInch() );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetQuality( 2 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetSize( 8 );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetStrikethrough( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().SetUnderline( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetShadow().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetEmboss().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetGradient().SetVisible( FALSE );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().GetFont().GetOutline().SetVisible( FALSE );
			
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().GetShape().SetTransparent( TRUE );
			str = _T("Pman= ") + CString(  WriteCUDouble( _U_PRESSURE, m_dWorkingPointPmanSI, true ) );
			m_TChart.GetTools().GetItems( iAnnotationNbr ).GetAsAnnotation().SetText( str );
		}

		m_bAnnotationTextDisplayed = true;
	}

	*Y = m_rLegendPosInfo.m_mapTopPos[m_rLegendPosInfo.m_mapIndexOrder[ItemIndex]];
}
