#include "stdafx.h" 
#include "resource.h"
#include "global.h"
#include "SplashWindow.h"


CSplashWindow* CSplashWindow::m_pSplashWindow;
int CSplashWindow::m_millisecondsToDisplay;

CSplashWindow::CSplashWindow()
{
	m_nTimer = (UINT_PTR)0;
}

CSplashWindow::~CSplashWindow()
{
	// Clear the static window pointer.
	ASSERT(m_pSplashWindow == this);
	m_pSplashWindow = NULL;
}

BEGIN_MESSAGE_MAP(CSplashWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CSplashWindow::ShowSplashScreen( CWnd* pParentWnd, int millisecondsToDisplay )
{
	m_millisecondsToDisplay = millisecondsToDisplay;

	// Allocate a new splash screen, and create the window.
	if ( m_pSplashWindow == NULL )
	{
		m_pSplashWindow = new CSplashWindow;
		if( FALSE == m_pSplashWindow->Create( pParentWnd ) )
		{
			delete m_pSplashWindow;
			m_pSplashWindow = NULL;
		}
	}

	// Set a timer to destroy the splash screen.
	if( 0 != millisecondsToDisplay )
	{ 
		m_pSplashWindow->m_nTimer = m_pSplashWindow->SetTimer( _TIMERID_SPLASHWINDOW, m_millisecondsToDisplay, NULL );
	}

	m_pSplashWindow->ShowWindow ( SW_SHOW );
	m_pSplashWindow->UpdateWindow();
	m_pSplashWindow->InvalidateRect( NULL, FALSE );
	
	// clear message queue
	MSG msg;
	while( PeekMessage( &msg, m_pSplashWindow->m_hWnd,  0, 0, PM_REMOVE ) )
	{ 
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

}

BOOL CSplashWindow::Create( CWnd* pParentWnd )
{
	if (!m_bitmap.LoadBitmap(IDB_MAIN))
		return FALSE;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	int xPos = 0;
	int yPos = 0;
	int width = bm.bmWidth;
	int height = bm.bmHeight;

	// if parent window, center it on the parent window. otherwise center it on the screen
	CRect parentRect;
	if ( pParentWnd == NULL ) {
		GetDesktopWindow()->GetWindowRect (parentRect );
	} else {
		pParentWnd->GetWindowRect ( parentRect );
	}

	xPos = parentRect.left + (parentRect.right - parentRect.left)/2 - (width/2);
	yPos = parentRect.top + (parentRect.bottom - parentRect.top)/2 - (height/2);


	BOOL result =  CreateEx(0,
		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		NULL, WS_POPUP | WS_VISIBLE, xPos, yPos, width, height, pParentWnd->GetSafeHwnd(), NULL);

	if ( !result )
		return FALSE;

	// if no parent window, make it a topmost, so eventual application window will appear under it
	if ( pParentWnd == NULL ) {
		SetWindowPos( &wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	}
	return TRUE;
}

void CSplashWindow::HideSplashScreen()
{
	// Destroy the window, and update the mainframe.
	if ( m_pSplashWindow != NULL ) {
		m_pSplashWindow->DestroyWindow();
		CWnd* mainWnd = AfxGetMainWnd();
		if ( mainWnd && IsWindow(mainWnd->m_hWnd) )
			mainWnd->UpdateWindow();
	}
}

void CSplashWindow::PostNcDestroy()
{
	// Free the C++ class.
	delete this;
}

int CSplashWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Center the window.
	CenterWindow();

	// Set a timer to destroy the splash screen.
	if( 0 != m_millisecondsToDisplay )
	{
		m_nTimer = SetTimer( _TIMERID_SPLASHWINDOW, m_millisecondsToDisplay, NULL );
	}

	return 0;
}

void CSplashWindow::OnPaint()
{
	CPaintDC dc(this);

	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(&dc))
		return;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	// Paint the image.
	CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
	dcImage.SelectObject(pOldBitmap);
}

void CSplashWindow::OnTimer( UINT_PTR nIDEvent )
{
	CWnd::OnTimer( nIDEvent );
	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
		return;

	// Destroy the splash screen window.
	HideSplashScreen();
}