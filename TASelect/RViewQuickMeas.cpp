#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "RViewQuickMeas.h"
#include "Excel_Workbook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRViewQuickMeas* pRViewQuickMeas = NULL;

IMPLEMENT_DYNCREATE( CRViewQuickMeas, CFormViewEx )

CRViewQuickMeas::CRViewQuickMeas()
	: CFormViewEx( CRViewQuickMeas::IDD )
{
	pRViewQuickMeas = this;
	m_fRedraw = true;
}

#ifdef _DEBUG
void CRViewQuickMeas::AssertValid() const
{
	CFormViewEx::AssertValid();
}

void CRViewQuickMeas::Dump( CDumpContext& dc ) const
{
	CFormViewEx::Dump( dc );
}
#endif

BOOL CRViewQuickMeas::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext ) 
{
	if( FALSE == CFormViewEx::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext ) )
		return FALSE;

	// Create a spread sheet.
	if( FALSE == m_SheetQuickMeas.Create( ( dwStyle | WS_CHILD ) ^ WS_VISIBLE, rect, this, IDC_FPSPREAD ) ) 
		return FALSE;

	return true;
}

void CRViewQuickMeas::SetRedraw()
{
	if( NULL == m_SheetQuickMeas.GetSafeHwnd() )
		return;

	m_fRedraw = true;

	m_SheetQuickMeas.SetRedraw();
	_ResizeColumnSheet();
	
	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;

	Invalidate();
}

BEGIN_MESSAGE_MAP( CRViewQuickMeas, CFormViewEx )
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_COMMAND( ID_APPMENU_EXPORT_QM, OnFileExportQM )
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	// Spread DLL messages.
	ON_MESSAGE( SSM_CLICK , ClickFpspread )
END_MESSAGE_MAP()

void CRViewQuickMeas::OnSize( UINT nType, int cx, int cy ) 
{
	CFormViewEx::OnSize( nType, cx, cy );

	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;
	_ResizeColumnSheet();
	m_fRedraw = true;
}

void CRViewQuickMeas::OnPaint()
{
	if( NULL == m_SheetQuickMeas.GetSafeHwnd() )
		return;

	CPaintDC dc( this );

	if( true == m_SheetQuickMeas.IsSheetHasData() )
	{
		if( true == m_fRedraw && NULL != m_SheetQuickMeas.GetSafeHwnd() )
		{
			CRect PageRect;
			GetClientRect( &PageRect );
			m_SheetQuickMeas.MoveWindow( 0, 0, PageRect.Width(), PageRect.Height(), TRUE );
		}
	}
	else
	{
		// Force hiding the spread sheet.
		if( NULL != m_SheetQuickMeas.GetSafeHwnd() )
		{
			m_SheetQuickMeas.MoveWindow( 0, 0, 0, 0, FALSE );
		}

		// Variables.
		CDC *pDC = GetDC();
		int y = m_iTopMargin;
		int x = m_iLeftMargin;
		m_iLeftMargin = 3;

		// No quick measurement -> fill the right view with white color.
		CRect clientRect;
		GetClientRect( &clientRect );
		CBrush Brush( RGB( 255,255,255 ) );
		pDC->FillRect( &clientRect, &Brush );
		
		// Set the "No quick measurement available in the project." text in an orange ribbon.
		CString str = TASApp.LoadLocalizedString( IDS_NOQUICKMEAS );
		y += 3 * m_iLineSpace;
		y = DrawTitle( pDC, m_iLeftMargin, y, str, _TAH_ORANGE );

		// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
		ReleaseDC(pDC);
	}

	m_fRedraw = false;
}

BOOL CRViewQuickMeas::OnEraseBkgnd( CDC* pDC )
{
	if( NULL == m_brBkgr.GetSafeHandle() && NULL == m_hBkgrBitmap )
		return CFormView::OnEraseBkgnd( pDC );

	ASSERT_VALID( pDC );

	CRect rectClient;
	GetClientRect( &rectClient );

	if( m_BkgrLocation != BACKGR_TILE || NULL == m_hBkgrBitmap )
	{
		if( m_brBkgr.GetSafeHandle() != NULL )
		{
			m_brBkgr.DeleteObject();
			m_brBkgr.CreateSolidBrush( _WHITE );
			pDC->FillRect( rectClient, &m_brBkgr );
		}
		else
		{
			CFormView::OnEraseBkgnd( pDC );
		}
	}

	if( NULL == m_hBkgrBitmap )
		return TRUE;

	ASSERT( m_sizeBkgrBitmap != CSize( 0, 0 ) );

	if( m_BkgrLocation != BACKGR_TILE )
	{
		CPoint ptImage = rectClient.TopLeft();

		switch( m_BkgrLocation )
		{
			case BACKGR_TOPLEFT:
				break;

			case BACKGR_TOPRIGHT:
				ptImage.x = rectClient.right - m_sizeBkgrBitmap.cx;
				break;

			case BACKGR_BOTTOMLEFT:
				ptImage.y = rectClient.bottom - m_sizeBkgrBitmap.cy;
				break;

			case BACKGR_BOTTOMRIGHT:
				ptImage.x = rectClient.right - m_sizeBkgrBitmap.cx;
				ptImage.y = rectClient.bottom - m_sizeBkgrBitmap.cy;
				break;
		}

		pDC->DrawState( ptImage, m_sizeBkgrBitmap, m_hBkgrBitmap, DSS_NORMAL );
	}
	else
	{
		// Tile background image.
		for( int x = rectClient.left; x < rectClient.Width(); x += m_sizeBkgrBitmap.cx )
		{
			for( int y = rectClient.top; y < rectClient.Height(); y += m_sizeBkgrBitmap.cy )
			{
				pDC->DrawState( CPoint( x, y ), m_sizeBkgrBitmap, m_hBkgrBitmap, DSS_NORMAL );
			}
		}
	}
	return TRUE;
}

void CRViewQuickMeas::OnFileExportQM()
{
	CString PrjDir = GetProjectDirectory();

	// Load file filter, compose file name and initialize 'CFileDialog'.
	CString str, strExt, strName;

	str = TASApp.LoadLocalizedString( IDS_EXPORTXLSXFILTER );
	strExt = _T("xlsx");
	strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

	CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, (LPCTSTR)str, NULL );
	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;
	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();

		// Create a sheetHMCalc with multiple tabs.
		CSSheetQuickMeas SheetQuickMeas;
		CRect rect( 0, 0, 50, 50 );
		SheetQuickMeas.Create( WS_CHILD ^ !WS_VISIBLE, rect, this, IDC_TMPSSHEET );
		SheetQuickMeas.SetRedraw( true );
		//SheetQuickMeas.ExportExcelBook( dlg.GetPathName(), NULL );
		Excel_Workbook wbQuickMeas;
		wbQuickMeas.AddSheet( &SheetQuickMeas );
		wbQuickMeas.Write( dlg.GetPathName() );
		EndWaitCursor();
	}
}

LRESULT CRViewQuickMeas::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	m_pTADS = TASApp.GetpTADS();

	// Retrieve last selected page setup from TADS.
	m_pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup );

	return 0;
}

LRESULT CRViewQuickMeas::ClickFpspread( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_SheetQuickMeas.GetSafeHwnd() )
		return 0;

	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	m_SheetQuickMeas.CellClicked( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

void CRViewQuickMeas::_ResizeColumnSheet()
{
	if( NULL == m_SheetQuickMeas.GetSafeHwnd() )
		return;
	
	CRect rect;
	GetClientRect( &rect );
	int	iWidth = rect.Width();
	iWidth = max( iWidth, RIGHTVIEWWIDTH );
	rect.right = iWidth;
	m_SheetQuickMeas.ResizeColumns( iWidth );
}
