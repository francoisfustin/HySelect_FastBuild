#include "stdafx.h"


#include "TASelect.h"
#include "DlgHydronicSchViewLarge.h"

CDlgHydronicSchViewLarge::CDlgHydronicSchViewLarge( CEnBitmap *pBitmap, CWnd *pParent )
	: CDialogEx( CDlgHydronicSchViewLarge::IDD, pParent )
{
	m_pBitmap = pBitmap;
}

void CDlgHydronicSchViewLarge::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICPICDYNSCH, m_stDynSch );
}

BEGIN_MESSAGE_MAP( CDlgHydronicSchViewLarge, CDialogEx )
	ON_BN_CLICKED( IDC_BUTTONREFRESH, OnBnClickedButtonRefresh )
END_MESSAGE_MAP()

void CDlgHydronicSchViewLarge::OnBnClickedButtonRefresh()
{
	this->RedrawWindow();
}

BOOL CDlgHydronicSchViewLarge::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetDlgItem( IDC_STATICSEPARATOR )->EnableWindow( FALSE );
	GetDlgItem( IDC_STATICSEPARATOR )->ShowWindow( SW_HIDE );

	if( NULL != m_pBitmap && NULL != m_pBitmap->GetSafeHandle() )
	{
		// Retrieve the window size.
		CRect rectWindow;
		GetWindowRect( &rectWindow );

		// Retrieve the client size.
		CRect rectClient;
		GetClientRect( &rectClient );

		CPoint ptDiff( rectWindow.Width() - rectClient.Width(), rectWindow.Height() - rectClient.Height() );

		// Retrieve space area for the close button.
		CRect rectStaticSeparator;
		GetDlgItem( IDC_STATICSEPARATOR )->GetWindowRect( &rectStaticSeparator );
		ScreenToClient( &rectStaticSeparator );
		long lCloseSpaceHeight = rectClient.Height() - rectStaticSeparator.top;

		// Retrieve image size.
		BITMAP bitmapInfo;
		GetObject( m_pBitmap->GetSafeHandle(), sizeof( BITMAP ), &bitmapInfo );

		// New window size.
		CRect rectNewSize = CRect( 0, 0, bitmapInfo.bmWidth, bitmapInfo.bmHeight );

		// Add difference between window and client size.
		rectNewSize.right += ptDiff.x;
		rectNewSize.bottom += ptDiff.y;

		// Add space area for the close button.
		rectNewSize.bottom += lCloseSpaceHeight;

		SetWindowPos( NULL, 0, 0, rectNewSize.Width(), rectNewSize.Height(), SWP_NOMOVE | SWP_NOZORDER );
		m_stDynSch.SetWindowPos( NULL, 0, 0, bitmapInfo.bmWidth, bitmapInfo.bmHeight, SWP_NOMOVE | SWP_NOZORDER );

		// Retrieve size of the 'close' button.
		CRect rectBtnCloseSize;
		GetDlgItem( IDOK )->GetClientRect( &rectBtnCloseSize );

		GetClientRect( &rectClient );
		
		CPoint ptClosePos = CPoint( ( rectClient.Width() - rectBtnCloseSize.Width() ) / 2, bitmapInfo.bmHeight + ( lCloseSpaceHeight - rectBtnCloseSize.Height() ) / 2 );
		GetDlgItem( IDOK )->SetWindowPos( NULL, ptClosePos.x, ptClosePos.y, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

		m_pBitmap->SaveIntoCStatic( &m_stDynSch );
	}

	return TRUE;
}
