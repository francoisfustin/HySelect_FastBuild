#include "stdafx.h"
#include "MainFrm.h"
#include "TASelect.h"
#include "Global.h"
#include "DlgLTtabctrl.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabInfo.h"

#include "RViewStartPage.h"
#include "DlgViscCorr.h"
#include "DlgPipeDp.h"
#include "DlgKvSeries.h"
#include "DlgHydroCalc.h"
#include "DlgUnitConv.h"
#include "DlgTALink.h"
#include "DlgLeftTabProject.h"
#include "DlgWizTAScope.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CRViewStartPage, CScrollView )

CRViewStartPage::CRViewStartPage()
{
	m_pFontTitle = NULL;
	m_pFontLink = NULL;
	_InitLineVector();
}

CRViewStartPage::~CRViewStartPage()
{
	delete m_pFontLink;
	delete m_pFontTitle;
	Stop();
}

void CRViewStartPage::OnInitialUpdate()
{
	CPaintDC dc( this );
	_UpdateFont( dc );
}

BEGIN_MESSAGE_MAP( CRViewStartPage, CScrollView )
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CRViewStartPage::_DrawLine( CDC &dc, startLine &sl, POINT &p )
{
	CRect line = _GetLine( sl.columnNum, sl.lineNum );
	int iconSize = (int)( (double)_GetWorkspace().Height() / m_lines );
	iconSize -= 2; // Leave at least 1 white pixel of spacing
	iconSize -= iconSize % 16;
	int iconOffset = ( line.Height() - iconSize ) / 2;

	dc.SetBkMode( TRANSPARENT );

	if( line.PtInRect( p ) && sl.iconID )
	{
		_UnselectAll();
		sl.lineBoundingBox = line;
		dc.SetTextColor( _TAH_ORANGE );
		line.OffsetRect( 2, 2 );
	}
	else
	{
		if( line.PtInRect( p ) )
		{
			_UnselectAll();
			sl.lineBoundingBox = line;
		}

		dc.SetTextColor( _IMI_GRAY );
	}

	if( sl.title )
	{
		dc.SelectObject( m_pFontTitle );
	}
	else
	{
		dc.SelectObject( m_pFontLink );
	}

	CString str;
	wchar_t drive[MAX_PATH];
	wchar_t dir[MAX_PATH];
	wchar_t fname[MAX_PATH];
	wchar_t ext[MAX_PATH];

	if( sl.stringID >= ID_FILE_MRU_FILE1 && sl.stringID <= ID_FILE_MRU_FILE4 )
	{
		str = GetMRUPath( sl.stringID );
		_wsplitpath_s( str, drive, dir, fname, ext );
		str = fname;
		str += ext;
	}
	else
	{
		str = TASApp.LoadLocalizedString( sl.stringID );
	}

	if( sl.iconID && iconSize && false == str.IsEmpty() )
	{
		line.left += iconSize / 2;
		HICON icon = ( HICON )LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( sl.iconID ), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR | LR_SHARED );
		DrawIconEx( dc, line.left, line.top + iconOffset, icon, iconSize, iconSize, 0, NULL, DI_NORMAL | DI_COMPAT );
		line.left += iconSize * 2;
	}

	dc.DrawText( str, -1, &line, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX );

	//GetProfileString("Recent File List", "File(X)");
	//theApp.m_pRecentFileList->m_arrNames[i];

}

CString CRViewStartPage::GetMRUPath( int id )
{
	switch( id )
	{
		case ID_FILE_MRU_FILE1:
		case ID_FILE_MRU_FILE2:
		case ID_FILE_MRU_FILE3:
		case ID_FILE_MRU_FILE4:
		{
			CString strFileNum;
			strFileNum.Format( L"File%d", id - ID_FILE_MRU_FILE1 + 1 );
			return AfxGetApp()->GetProfileString( L"Recent File List", strFileNum );
			break;
		}
	}

	return _T( "" );
}

void CRViewStartPage::_UpdateFont( CDC &dc )
{
	// initialize fonts
	LOGFONT LogFont;
	LogFont.lfHeight = -( _GetWorkspace().Height() / 25 );
	LogFont.lfWidth = 0;
	LogFont.lfEscapement = 0;
	LogFont.lfOrientation = 0;
	LogFont.lfWeight = 700;
	LogFont.lfItalic = 0;
	LogFont.lfUnderline = 0;
	LogFont.lfStrikeOut = 0;
	LogFont.lfCharSet = 0;
	LogFont.lfOutPrecision = 0;
	LogFont.lfClipPrecision = 0;
	LogFont.lfQuality = 0;
	LogFont.lfPitchAndFamily = 0;
	wcscpy_s( LogFont.lfFaceName, _T("Arial") );

	if( m_pFontTitle )
	{
		delete m_pFontTitle;
	}

	m_pFontTitle = new CFont;
	m_pFontTitle->CreateFontIndirect( &LogFont );

	if( m_pFontLink )
	{
		delete m_pFontLink;
	}

	m_pFontLink = new CFont;
	LogFont.lfHeight = -( _GetWorkspace().Height() / 35 );
	LogFont.lfWeight = 400;
	m_pFontLink->CreateFontIndirect( &LogFont );

	CFont *pOldFont = dc.SelectObject( m_pFontTitle );
}

void CRViewStartPage::_InitLineVector()
{
	// column 1.
	m_vStartLine.push_back( startLine( slt_projectTitle, true, IDS_DLGREF_STATICPRJ, NULL, 0, 0 ) );
	m_vStartLine.push_back( startLine( slt_newCircuit, false, IDS_APPMENU_NEW, IDI_NEW, 0, 1 ) );
	m_vStartLine.push_back( startLine( slt_browseFile, false, IDS_APPMENU_OPEN, IDI_BROWSE, 0, 2 ) );
	m_vStartLine.push_back( startLine( slt_previousFile1, false, ID_FILE_MRU_FILE1, IDR_TASELECTTYPE, 0, 3 ) );
	m_vStartLine.push_back( startLine( slt_previousFile2, false, ID_FILE_MRU_FILE2, IDR_TASELECTTYPE, 0, 4 ) );
	m_vStartLine.push_back( startLine( slt_previousFile3, false, ID_FILE_MRU_FILE3, IDR_TASELECTTYPE, 0, 5 ) );
	//m_vStartLine.push_back( startLine( slt_previousFile4, false, ID_FILE_MRU_FILE4, IDR_TASELECTTYPE, 0, 6 ) );
	m_vStartLine.push_back( startLine( slt_toolsTitle, true, IDS_RBN_PS_TOOLS, NULL, 0, 6 ) );
	m_vStartLine.push_back( startLine( slt_viscosityCorrection, false, IDS_DLGVISCCORR_CAPTION, IDI_VISCOSITYCORRECTION, 0, 7 ) );
	m_vStartLine.push_back( startLine( slt_pipesPressureDrop, false, IDS_RBN_H_TP_PIPEPRESSDROP, IDI_PIPEPRESSUREDROP, 0, 8 ) );
	m_vStartLine.push_back( startLine( slt_kvValues, false, IDS_RBN_T_H_KVCALC, IDI_KVVALUES, 0, 9 ) );
	m_vStartLine.push_back( startLine( slt_hydronicCalculator, false, IDS_RBN_T_H_CALCULATOR, IDI_CALCULATOR, 0, 10 ) );
	m_vStartLine.push_back( startLine( slt_unitConversion, false, IDS_DLGUNITCONV_CAPTION, IDI_UNITCONVERSION, 0, 11 ) );
	//m_vStartLine.push_back( startLine( slt_taLink, false, IDS_RBN_T_O_TALINK, IDI_TALINK, 0, 13 ) );
	// HYS-1149: Move 'communication' section to the first column to make place for 6-way valve
	m_vStartLine.push_back( startLine( slt_communicationTitle, true, IDS_RBN_COMM, NULL, 0, 12 ) );
	m_vStartLine.push_back( startLine( slt_updateHyselect, false, IDS_RBN_C_U_PRODDATA, IDI_UPDATE, 0, 13 ) );
	m_vStartLine.push_back( startLine( slt_tascopeCommunication, false, IDS_RBN_C_DT_SCOPECOMM, IDI_TASCOPECOM, 0, 14 ) );
	//m_vStartLine.push_back( startLine( slt_tacbiCommunication, false, IDS_RBN_C_DT_CBICOMM, IDI_TACBICOM, 0, 14 ) );
	
	// Column 2.
	int iRowNumber = 0;
	m_vStartLine.push_back( startLine( slt_selectionTitle, true, IDS_RBN_PS_INDBATCHWIZARDDIRECTSEL, NULL, 1, iRowNumber++ ) );
	
	if( true == TASApp.IsAirVentSepDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_separators, false, IDS_COMBOSSELSEPARATOR, IDI_ZEPARO, 1, iRowNumber++ ) );
	}

	if( true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_pressureMaintenance, false, IDS_COMBOSSELPRESSUREMAINTENANCE, IDI_COMPRESSO, 1, iRowNumber++ ) );
	}

	if( true == TASApp.IsSafetyValveDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_SafetyValve, false, IDS_COMBOSSELSAFETYVALVE, IDI_SAFETYVALVE, 1, iRowNumber++ ) );
	}

	m_vStartLine.push_back( startLine( slt_manualBalancing, false, IDS_COMBOSSELBV, IDI_STAD, 1, iRowNumber++ ) );
	
	if( true == TASApp.IsDpCDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_dpControllers, false, IDS_COMBOSSELDPC, IDI_STAP, 1, iRowNumber++ ) );
	}

	if( true == TASApp.IsBCvDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_balancingAndControl, false, IDS_COMBOSSELBCV, IDI_BCV, 1, iRowNumber++ ) );
	}
	
	if( true == TASApp.IsPICvDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_pressIndBalAndControl, false, IDS_COMBOSSELPICV, IDI_PIBCV, 1, iRowNumber++ ) );
	}

	if( true == TASApp.IsDpCBCVDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_DpCBCV, false, IDS_COMBOSSELDPCBCV, IDI_DPCBCV, 1, iRowNumber++ ) );
	}

	// HYS-1149: For 6-way valve.
	if( true == TASApp.Is6WayCVDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_6WayValves, false, IDS_COMBOSSEL6WAY, IDI_6WAYVALVE, 1, iRowNumber++ ) );
	}

	if( true == TASApp.IsCvDisplayed() )
	{
		if( 0 == TASApp.GetCvFamily().CompareNoCase( L"Brazil" ) )
		{
			m_vStartLine.push_back( startLine( slt_controlValves, false, IDS_COMBOSSELCV, IDI_CONTROLVALVE_BRAZIL, 1, iRowNumber++ ) );
		}
		else
		{
			m_vStartLine.push_back( startLine( slt_controlValves, false, IDS_COMBOSSELCV, IDI_CONTROLVALVE, 1, iRowNumber++ ) );
		}
	}

	if( true == TASApp.IsSmartControlValveDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_smartControlValves, false, IDS_COMBOSSELSMARTCONTROLVALVE, IDI_SMARTCONTROLVALVE, 1, iRowNumber++ ) );
	}

	// HYS-1936: TA-Smart Dp - 03 - Add the shortcut in the start page
	if( true == TASApp.IsSmartDpCDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_smartDp, false, IDS_COMBOSSELSMARTDPC, IDI_SMARTDP, 1, iRowNumber++ ) );
	}

	if( true == TASApp.IsTrvDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_radiatorsValves, false, IDS_COMBOSSELTRV, IDI_KHEAD, 1, iRowNumber++ ) );
	}

	if( true == TASApp.IsShutOffValveDisplayed() )
	{
		m_vStartLine.push_back( startLine( slt_shutoffValves, false, IDS_COMBOSSELSV, IDI_SHUTOFFVALVE, 1, iRowNumber++ ) );
	}
}

void CRViewStartPage::_UnselectAll()
{
	for( unsigned int i = 0; i < m_vStartLine.size(); ++i )
	{
		m_vStartLine[i].lineBoundingBox.SetRectEmpty();
	}
}

CRect CRViewStartPage::_GetWorkspace()
{
	m_xmargin = m_rect.Width() / 10;
//	m_ymargin = m_rect.Height() / 40;
	m_ymargin = 10;

	CRect workspace = m_rect;
	workspace.DeflateRect( m_xmargin, m_ymargin );
	return workspace;
}

CRect CRViewStartPage::_GetColumn( unsigned int colNum )
{
	CRect col = _GetWorkspace();

	if( colNum >= m_columns )
	{
		return col;
	}

	switch( colNum )
	{
		case 0:
			col.right = ( ( col.Width() - m_xmargin ) / 2 ) + col.left;
			break;

		case m_columns-1:
			col.left = col.right - ( ( col.Width() - m_xmargin ) / 2 );
			break;

		default:
			break;
	}

	return col;
}

CRect CRViewStartPage::_GetLine( unsigned int colNum, unsigned int lineNum )
{
	CRect line = _GetColumn( colNum );

	if( lineNum >= m_lines )
	{
		return line;
	}

	unsigned int lineHeight = line.Height() / m_lines;
	line.top = line.top + ( lineHeight * lineNum );
	line.bottom = line.bottom - ( lineHeight * ( m_lines - lineNum - 1 ) );
	return line;
}

CRect CRViewStartPage::_GetLine( startLineType sltType )
{
	CRect line;

	for( unsigned int i = 0; i < m_vStartLine.size(); ++i )
	{
		if( sltType == m_vStartLine[i].lineType )
		{
			return _GetLine( m_vStartLine[i].columnNum, m_vStartLine[i].lineNum );
		}
	}

	return line;
}

CRect CRViewStartPage::_GetSelectedLine()
{

	for( unsigned int i = 0; i < m_vStartLine.size(); ++i )
	{
		if( FALSE == m_vStartLine[i].lineBoundingBox.IsRectEmpty() )
		{
			return m_vStartLine[i].lineBoundingBox;
		}
	}

	return m_vStartLine[0].lineBoundingBox;
}

CRViewStartPage::startLineType CRViewStartPage::_GetClickedLine( CPoint point )
{
	for( unsigned int i = 0; i < m_vStartLine.size(); ++i )
	{

		if( TRUE == _GetLine( m_vStartLine[i].columnNum, m_vStartLine[i].lineNum ).PtInRect( point ) )
		{
			return m_vStartLine[i].lineType;
		}
	}

	return slt_unknown;
}

void CRViewStartPage::OnPaint()
{
	SCROLLINFO rVScrollInfo;
	ZeroMemory( &rVScrollInfo, sizeof( SCROLLINFO ) );
	rVScrollInfo.cbSize = sizeof( SCROLLINFO );
	rVScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_VERT, &rVScrollInfo );

	SCROLLINFO rHScrollInfo;
	ZeroMemory( &rHScrollInfo, sizeof( SCROLLINFO ) );
	rHScrollInfo.cbSize = sizeof( SCROLLINFO );
	rHScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_HORZ, &rHScrollInfo );

	// Retrieve current position
	int nCurrentVPos = GetScrollPos( SBS_VERT );
	int nCurrentHPos = GetScrollPos( SBS_HORZ );
	int nMaxVPos = rVScrollInfo.nMax;
	int nMaxHPos = rHScrollInfo.nMax;

	// vertical scroll offset
	if( GetStyle() & WS_VSCROLL )
	{
		m_rect.top = -nCurrentVPos;
		m_rect.bottom = nMaxVPos + 1 - nCurrentVPos;
	}

	// horizontal scroll offset
	if( GetStyle() & WS_HSCROLL )
	{
		m_rect.left = -nCurrentHPos;
		m_rect.right = nMaxHPos + 1 - nCurrentHPos;
	}

	// update the client rectangle
	CPaintDC dc( this );
	CDC memDC; // buffer context
	CBitmap bmpDC;
	memDC.CreateCompatibleDC( &dc );
	bmpDC.CreateCompatibleBitmap( &dc, m_rect.Width(), m_rect.Height() );
	memDC.SelectObject( &bmpDC );
	_UpdateFont( memDC );

	// Fill background with white
	memDC.FillSolidRect( m_rect, RGB( 0xff, 0xff, 0xff ) );

	// Add Logo
	CBitmap logo;
	logo.LoadBitmap( IDB_LOGOTA_PRINT );
	CDC logoDC;
	logoDC.CreateCompatibleDC( &dc );
	logoDC.SelectObject( &logo );

	BITMAP bm;
	logo.GetObject( sizeof( BITMAP ), &bm );
	int logoW = bm.bmWidth;
	int logoH = bm.bmHeight;

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 10;
	bf.AlphaFormat = AC_SRC_OVER;

	::AlphaBlend( memDC,
				  ( m_rect.Width() - logoW ) / 3 - nCurrentHPos,
				  ( m_rect.Height() - logoH ) / 3 - nCurrentVPos,
				  logoW * 2,
				  logoH * 2,
				  logoDC,
				  0,
				  0,
				  logoW,
				  logoH,
				  bf );

	POINT p;
	GetCursorPos( &p );
	ScreenToClient( &p );

	for( unsigned int i = 0; i < m_vStartLine.size(); ++i )
	{
		_DrawLine( memDC, m_vStartLine[i], p );
	}

	dc.BitBlt( 0, 0, m_rect.Width(), m_rect.Height(), &memDC, 0, 0, SRCCOPY ); // blit buffer to screen
}

void CRViewStartPage::OnMouseMove( UINT nFlags, CPoint point )
{
	CRect lineSelected = _GetSelectedLine();

	if( TRUE == lineSelected.PtInRect( point ) ) // always on the selected line : no need to refresh
	{
		return;
	}
	else if( TRUE == lineSelected.IsRectEmpty() ) // no selected line
	{
		startLineType sltType = _GetClickedLine( point );

		if( slt_unknown == sltType )
		{
			return; // not in a line : no need to refresh
		}
		else
		{
			this->Invalidate( FALSE );
		}
	}
	else // move outside of the selected line
	{
		_UnselectAll();
		this->Invalidate( FALSE );
	}
}

void CRViewStartPage::OnLButtonUp( UINT nFlags, CPoint point )
{
	switch( _GetClickedLine( point ) )
	{
		case slt_projectTitle:
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonProjectCategory() );
			break;

		case slt_newCircuit:
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonProjectCategory() );
			pTASelectDoc->OnFileNew();
			break;

		case slt_browseFile:
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonProjectCategory() );
			/*TASApp.FileOpen(TASApp.GetpTADS(), NULL);*/
			pTASelectDoc->OnFileOpen();
			break;


		case slt_previousFile1:
		{
			if( true == _OpenPreviousFile( ID_FILE_MRU_FILE1 ) )
			{
				break;
			}

			return;
			break;
		}

		case slt_previousFile2:
		{
			if( true == _OpenPreviousFile( ID_FILE_MRU_FILE2 ) )
			{
				break;
			}

			return;
			break;
		}

		case slt_previousFile3:
		{
			if( true == _OpenPreviousFile( ID_FILE_MRU_FILE3 ) )
			{
				break;
			}

			return;
			break;
		}

		case slt_toolsTitle:
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonToolsCategory() );
			break;

		case slt_viscosityCorrection:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonToolsCategory() );
			CDlgViscCorr dlg;
			dlg.Display();
		}
		break;

		case slt_pipesPressureDrop:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonToolsCategory() );
			CDlgPipeDp dlg;
			dlg.Display();
			dlg.DoModal();
		}
		break;

		case slt_kvValues:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonToolsCategory() );
			CDlgKvSeries dlg;
			dlg.Display();
		}
		break;

		case slt_hydronicCalculator:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonToolsCategory() );
			CDlgHydroCalc dlg;
			dlg.DoModal();
		}
		break;

		case slt_unitConversion:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonToolsCategory() );
			CDlgUnitConv dlg;
			dlg.Display();
		}
		break;

		//case slt_taLink:
		//{
		//	pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonToolsCategory() );
		//	CDlgTALink dlg;
		//	dlg.DoModal();
		//}
		//break;

		case slt_selectionTitle:
		case slt_separators:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_Separator );
			break;
		}

		case slt_pressureMaintenance:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_ExpansionVessel );
			break;
		}

		case slt_SafetyValve:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_SafetyValve );
			break;
		}

		case slt_manualBalancing:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_RegulatingValve );
			break;
		}

		case slt_dpControllers:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_DpController );
			break;
		}

		case slt_balancingAndControl:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_BalAndCtrlValve );
			break;
		}

		case slt_pressIndBalAndControl:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_PressureIndepCtrlValve );
			break;
		}

		case slt_DpCBCV:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve );
			break;
		}

		case slt_controlValves:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_ControlValve );
			break;
		}

		case slt_smartControlValves:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_SmartControlValve );
			break;
		}

		case slt_smartDp:
		{
			// HYS-1937: TA - Smart Dp - 04 - Individual selection : left tab.
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_SmartDpC );
			break;
		}

		// HYS-1149		
		case slt_6WayValves:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_BC_6WayControlValve );
			break;
		}

		case slt_radiatorsValves:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_TC_ThermostaticValve );
			break;
		}

		case slt_shutoffValves:
		{
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );
			}

			pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_ESC_ShutoffValve );
			break;
		}

		case slt_communicationTitle:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonCommunicationCategory() );
		}
		break;

		case slt_updateHyselect:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonCommunicationCategory() );

			if( true == TASApp.GetHyUpdateInstDir().IsEmpty() && false == TASApp.IsStartingWOInstal() )
			{
				return;
			}

			CString HyUpdate = TASApp.GetHyUpdateInstDir() + L"HyUpdate.exe";
			CString strParam;

			if( true == TASApp.IsAutoUpdate() )
			{
				strParam += CString( L"*Auto" );
				TASApp.SetAutoUpdateflag( false );
				TASApp.StartProcess( HyUpdate, 2, ::AfxGetAppName(), L"*Auto" );
			}

			TASApp.StartProcess( HyUpdate, 1, ::AfxGetAppName() );
		}
		break;

		case slt_tascopeCommunication:
		{
			pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonCommunicationCategory() );
			
			// Send message to notify the display of the project tab and the selection of the root item to avoid refreshing problems.
			if( NULL != pDlgLTtabctrl )
			{
				pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiProj );
			}

			if( NULL != pDlgLeftTabProject )
			{
				pDlgLeftTabProject->m_Tree.SelectItem( pDlgLeftTabProject->m_Tree.GetRootItem() );
			}

			CDlgWizTAScope dlgWiz;
			dlgWiz.DisplayWizard();

			BeginWaitCursor();
			
			if( NULL != pDlgLeftTabProject )
			{
				pDlgLeftTabProject->ResetTree();
			}

			EndWaitCursor();
		}
		break;

		case slt_unknown:
		{
			return;
			break;
		}

		default:
			break;
	}

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->ResetTree();
	}
}

bool CRViewStartPage::_OpenPreviousFile( int fileID )
{
	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiProj );
	}

	pMainFrame->GetRibbonBar()->SetActiveCategory( pMainFrame->GetRibbonProjectCategory() );
	CString file1 = GetMRUPath( fileID );

	if( !file1.IsEmpty() )
	{
		pTASelectDoc->OnOpenDocument( file1 );
		pTASelectDoc->SetPathName( file1, FALSE );
		return true;
	}

	return false;
}

void CRViewStartPage::OnSize( UINT nType, int cx, int cy )
{
	m_rect.top = 0;
	m_rect.left = 0;
	m_rect.bottom = ( cy < 500 ? 500 : cy );
	m_rect.right = ( cx < 700 ? 700 : cx );

	SetScrollSizes( MM_TEXT, m_rect.Size() );
}
