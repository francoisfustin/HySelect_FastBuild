#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "RViewHMSumm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRViewHMSumm *pRViewHMSumm = NULL;

IMPLEMENT_DYNCREATE( CRViewHMSumm, CFormViewEx )

CRViewHMSumm::CRViewHMSumm()
	: CFormViewEx( CRViewHMSumm::IDD )
{
	m_fRedraw = true;
	pRViewHMSumm = this;
}

CRViewHMSumm::~CRViewHMSumm()
{
	pRViewHMSumm = NULL;
}

#ifdef _DEBUG
void CRViewHMSumm::AssertValid() const
{
	CScrollView::AssertValid();
}

void CRViewHMSumm::Dump( CDumpContext& dc ) const
{
	CScrollView::Dump(dc);
}
#endif

BOOL CRViewHMSumm::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext ) 
{
	if( FALSE == CFormViewEx::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext ) )
		return FALSE;

	// Create a spread sheet.
	if( FALSE == m_SheetHMSumm.Create( ( dwStyle | WS_CHILD ) ^ WS_VISIBLE, rect, this, IDC_SHEETHMSUMM ) ) 
		return FALSE;

	return TRUE;
}

void CRViewHMSumm::SetRedraw()
{
	if( NULL == m_SheetHMSumm.GetSafeHwnd() )
		return;

	m_fRedraw = true;

	m_SheetHMSumm.SetRedraw();
	_ResizeColumnSheet();
	
	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;

	Invalidate();
}

BEGIN_MESSAGE_MAP( CRViewHMSumm, CFormViewEx )
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
END_MESSAGE_MAP()

void CRViewHMSumm::OnSize( UINT nType, int cx, int cy ) 
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

void CRViewHMSumm::OnPaint()
{
	if( NULL == m_SheetHMSumm.GetSafeHwnd() )
		return;

	CPaintDC dc( this );

	if( true == m_SheetHMSumm.IsSheetHasData() )
	{
		if( true == m_fRedraw && NULL != m_SheetHMSumm.GetSafeHwnd() )
		{
			CRect PageRect;
			GetClientRect( &PageRect );
			m_SheetHMSumm.MoveWindow( 0, 0, PageRect.Width(), PageRect.Height(), TRUE );
		}
	}
	else
	{
		// Force hiding the spread sheet.
		if( NULL != m_SheetHMSumm.GetSafeHwnd() )
		{
			m_SheetHMSumm.MoveWindow( 0, 0, 0, 0, FALSE );
		}

		// Variables.
		CDC *pDC = GetDC();
		int y = m_iTopMargin;
		int x = m_iLeftMargin;
		m_iLeftMargin = 3;

		// No log -> fill the right view with white color.
		CRect clientRect;
		GetClientRect( &clientRect );
		CBrush Brush( RGB( 255,255,255 ) );
		pDC->FillRect( &clientRect, &Brush );
		
		// Set the "No hydronic module defined in the project." text in an orange ribbon.
		CString str = TASApp.LoadLocalizedString( IDS_NOHM );
		y += 3 * m_iLineSpace;
		y = DrawTitle( pDC, m_iLeftMargin, y, str, _TAH_ORANGE );
		
		// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
		ReleaseDC( pDC );
	}

	m_fRedraw = false;
}

BOOL CRViewHMSumm::OnEraseBkgnd( CDC* pDC )
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

LRESULT CRViewHMSumm::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	m_pTADS = TASApp.GetpTADS();

	// Retrieve last selected page setup from TADS.
	m_pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup );

	return 0;
}

void CRViewHMSumm::_ResizeColumnSheet()
{
	if( NULL == m_SheetHMSumm.GetSafeHwnd() )
		return;
	
	CRect rect;
	GetClientRect( &rect );
	int	iWidth = rect.Width();
	iWidth = max( iWidth, RIGHTVIEWWIDTH );
	rect.right = iWidth;
	m_SheetHMSumm.ResizeColumns( iWidth );
}
