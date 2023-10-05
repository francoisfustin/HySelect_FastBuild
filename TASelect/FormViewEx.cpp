#include "stdafx.h"
#include "MainFrm.h"
#include "TASelect.h"
#include "MFCRibbonPanelEx.h"
#include "FormViewEx.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CFormViewEx, CFormView )

CFormViewEx::CFormViewEx()
	: CFormView( (UINT)0 )
{
	m_cBkgr = RGB( 255, 255, 255 );
	m_hBkgrBitmap = NULL;
	m_sizeBkgrBitmap = CSize(0, 0);
	m_BkgrLocation = BackgroundLocation::BACKGR_UNDEFINED;
	m_fAutoDestroyBmp = FALSE;
	m_pNotificationHandler = NULL;
}

CFormViewEx::CFormViewEx( UINT nIDTemplate )
	: CFormView( nIDTemplate )
{
	m_cBkgr = RGB( 255, 255, 255 );
	m_hBkgrBitmap = NULL;
	m_sizeBkgrBitmap = CSize(0, 0);
	m_BkgrLocation = BackgroundLocation::BACKGR_UNDEFINED;
	m_fAutoDestroyBmp = FALSE;
	m_pNotificationHandler = NULL;
}

void CFormViewEx::SetBackgroundColor( COLORREF color, BOOL fRepaint )
{
	m_cBkgr = color;
	if( NULL != m_brBkgr.GetSafeHandle() )
	{
		m_brBkgr.DeleteObject();
	}

	if( (COLORREF)-1 != color )
	{
		m_brBkgr.CreateSolidBrush( color );
	}
}

void CFormViewEx::SetBackgroundImage( HBITMAP hBitmap, BackgroundLocation eLocation, BOOL fAutoDestroy, BOOL fRepaint )
{
	if( TRUE == m_fAutoDestroyBmp && NULL != m_hBkgrBitmap )
	{
		::DeleteObject( m_hBkgrBitmap );
	}

	m_hBkgrBitmap = hBitmap;
	m_BkgrLocation = eLocation;
	m_fAutoDestroyBmp = fAutoDestroy;

	if( NULL != hBitmap )
	{
		BITMAP bmp;
		::GetObject( hBitmap, sizeof( BITMAP ), (LPVOID)&bmp );

		m_sizeBkgrBitmap = CSize( bmp.bmWidth, bmp.bmHeight );
	}
	else
	{
		m_sizeBkgrBitmap = CSize( 0, 0 );
	}

	if( TRUE == fRepaint && NULL != GetSafeHwnd() )
	{
		
		Invalidate();
		UpdateWindow();
	}
}

BOOL CFormViewEx::SetBackgroundImage( UINT uiBmpResId, BackgroundLocation eLocation, BOOL fRepaint )
{
	HBITMAP hBitmap = NULL;

	if( 0 != uiBmpResId )
	{
		hBitmap = ::LoadBitmap( AfxFindResourceHandle( MAKEINTRESOURCE( uiBmpResId ), RT_BITMAP ), MAKEINTRESOURCE( uiBmpResId ) );
		if( NULL == hBitmap )
		{
			ASSERT( FALSE );
			return FALSE;
		}
	}

	SetBackgroundImage( hBitmap, eLocation, TRUE, fRepaint );
	return TRUE;
}

void  CFormViewEx::OnApplicationLook( UINT uiID )
{
	SetBackgroundColor(	TASApp.GetBckColor() );	
}

void CFormViewEx::SetNotificationHandler( IFormViewPrintNotification* pINotificationHandler )
{
	if( NULL == pINotificationHandler )
		return;

	m_pNotificationHandler = pINotificationHandler; 
}

void CFormViewEx::ResetNotificationHandler()
{
	m_pNotificationHandler = NULL;
}

void CFormViewEx::OnFilePrintPreview()
{
	CPrintPreviewState *pState = new CPrintPreviewState;

	if( FALSE == DoPrintPreview( IDD_AFXBAR_RES_PRINT_PREVIEW, this, RUNTIME_CLASS( CMyPreviewViewEx ), pState ) )
	{
		TRACE0("Error: OnFilePrintPreview failed.\n");
		AfxMessageBox( AFX_IDP_COMMAND_FAILURE );
		delete pState;      // preview failed to initialize, delete State now
	}
}

BEGIN_MESSAGE_MAP(CFormViewEx, CFormView )
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CFormViewEx::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	OnApplicationLook( 0 );
}

HBRUSH CFormViewEx::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH hbr = CFormView::OnCtlColor( pDC, pWnd, nCtlColor );
	if( NULL != m_brBkgr.GetSafeHandle() || NULL != m_hBkgrBitmap )
	{
		TCHAR lpszClassName[AFX_MAX_CLASS_NAME + 1];
		::GetClassName( pWnd->GetSafeHwnd(), lpszClassName, AFX_MAX_CLASS_NAME );
		CString strClass = lpszClassName;
		if( AFX_STATIC_CLASS == strClass )
		{	
			pDC->SetBkMode( TRANSPARENT );
			hbr = (HBRUSH)m_brBkgr;
		}
		else if( AFX_BUTTON_CLASS == strClass )
		{				
			pDC->SetBkMode( TRANSPARENT );
			hbr = (HBRUSH)m_brBkgr;
		}
		else if( AFX_SLIDER_CLASS == strClass )
		{		
			pDC->SetBkMode( TRANSPARENT );
			hbr = (HBRUSH)m_brBkgr;
		}
	}

	return hbr;
}

BOOL CFormViewEx::OnEraseBkgnd( CDC* pDC )
{
	if( NULL == m_brBkgr.GetSafeHandle() && NULL == m_hBkgrBitmap )
	{
		return CFormView::OnEraseBkgnd( pDC );
	}

	ASSERT_VALID( pDC );

	CRect rectClient;
	GetClientRect( rectClient );

	if( BackgroundLocation::BACKGR_TILE != m_BkgrLocation || NULL == m_hBkgrBitmap )
	{
		if( NULL != m_brBkgr.GetSafeHandle() )
		{
			pDC->FillRect( rectClient, &m_brBkgr );
		}
		else
		{
			CFormView::OnEraseBkgnd( pDC );
		}
	}

	if( NULL == m_hBkgrBitmap )
	{
		return TRUE;
	}

	ASSERT( CSize( 0, 0 ) != m_sizeBkgrBitmap );

	if( BackgroundLocation::BACKGR_TILE != m_BkgrLocation )
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

BOOL CFormViewEx::OnPreparePrinting( CPrintInfo* pInfo )
{
	BOOL fReturn = FALSE;
	if( NULL != m_pNotificationHandler )
		fReturn = m_pNotificationHandler->IFVPN_OnPreparePrinting( pInfo );
	return fReturn;
}

void CFormViewEx::OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo )
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->IFVPN_OnBeginPrinting( pDC, pInfo );
}

void CFormViewEx::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->IFVPN_OnPrint( pDC, pInfo );
}

void CFormViewEx::OnEndPrinting( CDC* pDC, CPrintInfo* pInfo )
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->IFVPN_OnEndPrinting( pDC, pInfo );
}

void CFormViewEx::OnEndPrintPreview( CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView )
{
	CFormView::OnEndPrintPreview( pDC, pInfo, point, pView );
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->IFVPN_OnEndPrintPreview( pDC, pInfo, point, pView );
}

void CFormViewEx::OnFilePrint()
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->IFVPN_OnFilePrint();
}

/////////////////////////////////////////////////////////////////////////////
// CMyPreviewViewEx
IMPLEMENT_DYNCREATE( CMyPreviewViewEx, CPreviewViewEx )

CMyPreviewViewEx::CMyPreviewViewEx()
{
}

BEGIN_MESSAGE_MAP( CMyPreviewViewEx, CPreviewViewEx )
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI( AFX_ID_PREVIEW_NUMPAGE, OnUpdatePreviewNumPage )
END_MESSAGE_MAP()

int CMyPreviewViewEx::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CPreviewViewEx::OnCreate( lpCreateStruct ) )
		return -1;

	if( NULL == m_pWndRibbonBar )
		return -1;

	m_pWndRibbonBar->EnableKeyTips( false );

	// In the 'CPreviewViewEx::OnCreate' method, the code will retrieve the current main frame and take the current ribbon bar.
	// And then will call the 'CMFCRibbonBar::SetPrintPreviewMode' method. If we look in the source of 'CMFCRibbonBar', we can see that it is already
	// exist the 'm_pPrintPreviewCategory' variable. This one is in fact created when the 'CMFCRibbonBar::RecalcLayout' is called. The first thing
	// that 'SetPrintPreviewMode' does it's to set the shortcut keyboard keys for each panel. After that, it loops on all visible current categories and
	// set their 'm_bIsVisible' property to 'FALSE' (in our case, 'Home', 'Project', ... ). Than set the 'm_bIsVisible' of the 'm_pPrintPreviewCategory' to
	// 'TRUE' and makes it active.
	CMFCRibbonCategory* pclPrintPrevCategory = m_pWndRibbonBar->GetActiveCategory();
	if( NULL == pclPrintPrevCategory )
		return 0;

	// Set the text for the category.
	pclPrintPrevCategory->SetName( TASApp.LoadLocalizedString( IDS_RBN_PRINTPREVIEW ) );

	// Set the text for the panels.
	CMFCRibbonPanel *pPanelPrint = pclPrintPrevCategory->GetPanel( 0 );
	CMFCRibbonBaseElement *pBaseElement = NULL;
	if( NULL != pPanelPrint )
	{
		( (CMFCRibbonPanelEx*)pPanelPrint)->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_PRINT ) );
		
		// Set the text for the print button.
		pBaseElement = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->FindByID( AFX_ID_PREVIEW_PRINT, FALSE, FALSE ) );
		if( NULL != pBaseElement )
		{
			pBaseElement->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_P_PRINT ) );
			SetToolTipRibbonElement( pBaseElement, IDS_RBN_PP_P_PRINT_TT );
		}
	}

	CMFCRibbonPanel *pPanelZoom = pclPrintPrevCategory->GetPanel( 1 );
	if( NULL != pPanelZoom )
	{
		( (CMFCRibbonPanelEx*)pPanelZoom)->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_ZOOM ) );

		// Set the text for the Zoom buttons.
		pBaseElement = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->FindByID( AFX_ID_PREVIEW_ZOOMIN, FALSE, FALSE ) );
		if( NULL != pBaseElement )
		{
			pBaseElement->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_Z_ZOOMIN ) );
			SetToolTipRibbonElement( pBaseElement, IDS_RBN_PP_Z_ZOOMIN_TT );
		}
		
		pBaseElement = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->FindByID( AFX_ID_PREVIEW_ZOOMOUT, FALSE, FALSE ) );
		if( NULL != pBaseElement )
		{
			pBaseElement->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_Z_ZOOMOUT ) );
			SetToolTipRibbonElement( pBaseElement, IDS_RBN_PP_Z_ZOOMOUT_TT );
		}
		
		pBaseElement = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->FindByID( AFX_ID_PREVIEW_NUMPAGE, FALSE, FALSE ) );
		if( NULL != pBaseElement )
		{
			pBaseElement->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_Z_ONEPAGE ) );
			SetToolTipRibbonElement( pBaseElement, IDS_RBN_PP_Z_PAGE_TT );
		}
	}
	
	CMFCRibbonPanel *pPanelPreview = pclPrintPrevCategory->GetPanel( 2 );
	if( NULL != pPanelPreview )
	{
		( (CMFCRibbonPanelEx*)pPanelPreview )->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_PREVIEW ) );

		// Set the text for the Preview buttons.
		pBaseElement = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->FindByID( AFX_ID_PREVIEW_NEXT, FALSE, FALSE ) );
		if( NULL != pBaseElement )
		{
			pBaseElement->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_PREV_NEXTPAGE ) );
			SetToolTipRibbonElement( pBaseElement, IDS_RBN_PP_PREV_NEXTPAGE_TT );
		}
		
		pBaseElement = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->FindByID( AFX_ID_PREVIEW_PREV, FALSE, FALSE ) );
		if( NULL != pBaseElement )
		{
			pBaseElement->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_PREV_PREVIOUSPAGE ) );
			SetToolTipRibbonElement( pBaseElement, IDS_RBN_PP_PREV_PREVIOUSPAGE_TT );
		}
		
		pBaseElement = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->FindByID( AFX_ID_PREVIEW_CLOSE, FALSE, FALSE ) );
		if( NULL != pBaseElement )
		{
			pBaseElement->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_PREV_CLOSE ) );
			SetToolTipRibbonElement( pBaseElement, IDS_RBN_PP_PREV_CLOSE_TT );
		}
	}

	// stackoverflow.com: after adding a ribbon item, or calling a method that updates an item, the ribbon does not refresh correctly.
	// Sometimes it's OK after resizing the window but most of the time the panel is in a collapsed state.
	m_pWndRibbonBar->ForceRecalcLayout();
	
	// Create the status bar.
	if( NULL == m_pWndStatusBar )
		return 0;
	
	// Define the Text according to the indicators and redefine the good indicators.
	int cxWidth = 30;
	m_pWndStatusBar->SetPaneText( 1, TASApp.LoadLocalizedString( IDS_INDICATOR_CAPS ) );
	m_pWndStatusBar->SetPaneInfo( 1, ID_INDICATOR_CAPS, SBPS_NORMAL, cxWidth );
	m_pWndStatusBar->SetPaneText( 2, TASApp.LoadLocalizedString( IDS_INDICATOR_NUM ) );
	m_pWndStatusBar->SetPaneInfo( 2, ID_INDICATOR_NUM,SBPS_NORMAL, cxWidth );
	m_pWndStatusBar->SetPaneText( 3, TASApp.LoadLocalizedString( IDS_INDICATOR_SCRL ) );
	m_pWndStatusBar->SetPaneInfo( 3, ID_INDICATOR_SCRL,SBPS_NORMAL, cxWidth );

	return 0;
}

void CMyPreviewViewEx::OnUpdatePreviewNumPage( CCmdUI *pCmdUI )
{
	// Code comes from CPreviewViewEx::OnUpdatePreviewNumPage(CCmdUI *pCmdUI)
	// in the afxpreviewviewex.cpp file and I comment the base function to avoid string flashing.

	// Change the Icon of AFX_ID_PREVIEW_NUMPAGE button:.
	UINT nPages = ( m_nZoomState == ZOOM_OUT ) ? m_nPages : m_nZoomOutPages;

	if( m_pWndRibbonBar != NULL )
	{
		ASSERT_VALID( m_pWndRibbonBar );

		if( NULL == m_pNumPageButton )
			m_pNumPageButton = DYNAMIC_DOWNCAST( CMFCRibbonButton, m_pWndRibbonBar->GetActiveCategory()->FindByID( AFX_ID_PREVIEW_NUMPAGE ) );

		if( m_pNumPageButton != NULL )
		{
			ASSERT_VALID( m_pNumPageButton );

			int nImageIndex = ( nPages == 1 ) ? 5 : 4;
			
			// HYS-1218 : We also have to update m_pNumPageButton when the text is not the same. For example, the case
			// just after "OnCreate" which initialize the text to IDS_RBN_PP_Z_ONEPAGE.
			CString strMenuText = ( 5 == nImageIndex ) ? TASApp.LoadLocalizedString( IDS_RBN_PP_Z_TWOPAGE ) : TASApp.LoadLocalizedString( IDS_RBN_PP_Z_ONEPAGE );
			CMFCRibbonCmdUI* pclRibbonCmdUi = NULL;
			pclRibbonCmdUi = dynamic_cast<CMFCRibbonCmdUI*>( pCmdUI );

			if ( ( m_pNumPageButton->GetImageIndex( TRUE ) != nImageIndex ) 
				|| ( NULL != pclRibbonCmdUi
				&& ( 0 != strMenuText.Compare( pclRibbonCmdUi->m_pUpdated->GetText() ) ) ) )
			{
				m_pNumPageButton->SetImageIndex( nImageIndex, TRUE );
				m_pNumPageButton->SetKeys( nPages == 1 ? _T("2") : _T("1") );
				m_pNumPageButton->Redraw();

				if( 5 == nImageIndex )
				{
					pCmdUI->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_Z_TWOPAGE ) );
				}
				else if( 4 == nImageIndex )
				{
					pCmdUI->SetText( TASApp.LoadLocalizedString( IDS_RBN_PP_Z_ONEPAGE ) );
				}
				m_pWndRibbonBar->ForceRecalcLayout();
			}
		}
	}
	else if( m_wndToolBar.GetSafeHwnd() != NULL )
	{
		CMFCToolBarButton* pButton = m_wndToolBar.GetButton( m_iPagesBtnIndex );
		ASSERT_VALID( pButton );

		pButton->SetImage( ( nPages == 1 ) ? m_iTwoPageImageIndex : m_iOnePageImageIndex );

		m_wndToolBar.InvalidateRect( pButton->Rect() );
	}
}

void CMyPreviewViewEx::SetToolTipRibbonElement( CMFCRibbonBaseElement *pRbnBaseElement, UINT ID )
{
	if( NULL == pRbnBaseElement )
		return;
	if( 0 == ID )
		return;

	// Variables.
	CString str1, str2;
	int pos = -1;

	// Load the current string.
	str1 = TASApp.LoadLocalizedString( ID );
	str2 = str1;

	// Find the position of the last "\n".
	for( int i = str1.Find( _T("\n") ); i != -1; i = str1.Find( _T("\n" ), pos + 2 ) )
		pos = i;	

	// Split the string in two.
	// One part for the header of the tooltip and one part for the description.
	if( pos != -1 )
	{
		str2 = str1.Right( str1.GetLength() - pos );
		str1 = str1.Left( pos );
	}

	// Fill the tooltip with the header.
	pRbnBaseElement->SetToolTipText( (LPCTSTR)str2 );

	// Fill the tooltip with the description.
	if( pos != -1 )
		pRbnBaseElement->SetDescription( (LPCTSTR)str1 );
}
