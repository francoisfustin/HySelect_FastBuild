#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"

#include "RViewGen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRViewGen *pRViewGen = NULL;

IMPLEMENT_DYNCREATE( CRViewGen, CFormViewEx )

CRViewGen::CRViewGen()
	: CFormViewEx( CRViewGen::IDD )
{
	m_fRedraw = true;
	pRViewGen = this;
}

CRViewGen::~CRViewGen()
{
	pRViewGen = NULL;
}

BEGIN_MESSAGE_MAP( CRViewGen, CFormViewEx )
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRViewGen diagnostics

#ifdef _DEBUG
void CRViewGen::AssertValid() const
{
	CFormViewEx::AssertValid();
}

void CRViewGen::Dump( CDumpContext& dc ) const
{
	CFormViewEx::Dump( dc );
}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CRViewGen drawing

void CRViewGen::SetRedraw()
{
	if( NULL == m_SheetGen.GetSafeHwnd() )
		return;

	m_fRedraw = true;

	m_SheetGen.SetRedraw();
	ResizeColumnSheet();
	
	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;

	Invalidate();
}

void CRViewGen::ResizeColumnSheet()
{
	if( NULL == m_SheetGen.GetSafeHwnd() )
		return;
	
	CRect rect;
	GetClientRect( (LPRECT)rect );
	int	iWidth = rect.Width();
	iWidth = max( iWidth, RIGHTVIEWWIDTH );
	m_SheetGen.ResizeColumns( iWidth );
}

/////////////////////////////////////////////////////////////////////////////
// CRViewGen message handlers

BOOL CRViewGen::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext ) 
{
	if( FALSE == CFormViewEx::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext ) )
		return FALSE;

	// Create a Spread sheet 
	if( FALSE == m_SheetGen.Create( ( dwStyle | WS_CHILD ) ^ WS_VISIBLE, rect, this, IDC_SHEETGEN ) ) 
		return FALSE;

	return true;
}

void CRViewGen::OnSize( UINT nType, int cx, int cy ) 
{
	CFormViewEx::OnSize( nType, cx, cy );

	CRect PageRect;
	GetClientRect( &PageRect );
	m_PrintRect = PageRect;
	if( PageRect.Width() < RIGHTVIEWWIDTH )
		m_PrintRect.right = RIGHTVIEWWIDTH;
	ResizeColumnSheet();
	m_fRedraw = true;
}

void CRViewGen::OnPaint()
{
	if( NULL == m_SheetGen.GetSafeHwnd() )
		return;

	CPaintDC dc( this ); // device context for painting

	CString str;
	CRect PageRect;
	GetClientRect( &PageRect );
	if( true == m_fRedraw )
	{
		if( m_SheetGen.GetSafeHwnd() )
		{
			m_SheetGen.MoveWindow( 0, 0, PageRect.Width(), PageRect.Height(), TRUE );
		}
	}

	m_fRedraw = false;

	m_SheetGen.Invalidate();
}

BOOL CRViewGen::OnEraseBkgnd( CDC* pDC )
{
	if( NULL == m_brBkgr.GetSafeHandle() && NULL == m_hBkgrBitmap )
		return CFormView::OnEraseBkgnd( pDC );

	ASSERT_VALID( pDC );

	CRect rectClient;
	GetClientRect( rectClient );

	if( m_BkgrLocation != BACKGR_TILE || NULL == m_hBkgrBitmap )
	{
		if( m_brBkgr.GetSafeHandle() != NULL )
		{
			m_brBkgr.DeleteObject();
			m_brBkgr.CreateSolidBrush( _WHITE );
			pDC->FillRect( rectClient, &m_brBkgr );
		}
		else
			CFormView::OnEraseBkgnd( pDC );
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
		// Tile background image:
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
