#include "StdAfx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "ToolsDockablePane.h"
#include "DlgLTtabctrl.h"
#include "DlgLeftTabBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CDlgLeftTabBase, CDialogExt )

CDlgLeftTabBase::CDlgLeftTabBase( CMyMFCTabCtrl::TabIndex eTabIndex, UINT nID, CWnd *pParent )
	: CDialogExt( nID, pParent )
{
	m_iContextID = -1;
	m_eTabIndex = eTabIndex;
	m_pParent = pParent;
	m_bToolsDockablePaneContextInitialized = false;
}

CDlgLeftTabBase::~CDlgLeftTabBase( void )
{
}

void CDlgLeftTabBase::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CDlgLeftTabBase, CDialogExt )
	ON_WM_PAINT()
END_MESSAGE_MAP()

CRect CDlgLeftTabBase::GetTALogoSize()
{
	if( m_Bmp.GetSafeHandle() )
	{
		CRect rect = m_Bmp.GetSizeImage();
		return rect;
	}

	return CRect( 0, 0, 149, 48 );
}

// CDlgLeftTabBase message handlers
void CDlgLeftTabBase::OnApplicationLook( COLORREF cBackColor )
{
 	CDialogExt::SetBackgroundColor( cBackColor );
 	CDialogExt::OnApplicationLook( cBackColor );
}

void CDlgLeftTabBase::OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex )
{
	// If not yet initialized, do it.
	if( false == m_bToolsDockablePaneContextInitialized )
	{
		InitializeToolsDockablePaneContextID();
	}

	// If can't initialize, stop here!
	if( false == m_bToolsDockablePaneContextInitialized )
	{
		return;
	}

	int iContextID = -1;

	if( false == GetToolsDockablePaneContextID( iContextID ) )
	{
		return;
	}

	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->RestoreContext( iContextID );
	}
}

bool CDlgLeftTabBase::GetToolsDockablePaneContextID( int &iContextID )
{
	// If not yet initialized, do it.
	if( false == m_bToolsDockablePaneContextInitialized )
	{
		InitializeToolsDockablePaneContextID();
	}

	if( false == m_bToolsDockablePaneContextInitialized )
	{
		return false;
	}

	return true;
}

void CDlgLeftTabBase::OnPaint()
{
	CPaintDC dc( this ); // device context for painting

	const int YShift = _DLGLEFTTABBASE_LOGOYSHIFT;
	const int XShift = _DLGLEFTTABBASE_LOGOXSHIFT;

	CRect rect = m_Bmp.GetSizeImage();
	CRect rectParent, rectChild;
	GetClientRect( rectParent );

	// if ctrl IDC_STATICGROUP exist top position of logo is limited by bottom of this ctrl
	CWnd *pWnd = GetDlgItem( IDC_STATICGROUP );
	CPoint ptTop = CPoint( 0, 0 );

	if( NULL != pWnd && NULL != pWnd->GetSafeHwnd() )
	{
		pWnd->GetWindowRect( &rectChild );
		ScreenToClient( &rectChild );
		ptTop.x = XShift;
		ptTop.y = rectChild.bottom + YShift;
	}

	CPoint pt;
	pt.y = max( rectParent.bottom - rect.Height() - YShift, ptTop.y );
	pt.x = rectParent.left + XShift;
	//	m_Bmp.DrawOnDC32( &dc, pt, 255 );

	if( NULL == m_BmpContainer.GetSafeHwnd() )
	{
		m_BmpContainer.Create( NULL, WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_BITMAP, rect, this );
		m_BmpContainer.SetBitmap( ( HBITMAP )m_Bmp.GetSafeHandle() );
	}

	m_BmpContainer.MoveWindow( pt.x, pt.y, rect.Width(), rect.Height() );
	m_BmpContainer.ShowWindow( SW_SHOW );
	m_BmpContainer.BringWindowToTop();
}

BOOL CDlgLeftTabBase::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	TASApp.GetCompanyLogo( &m_Bmp, this );

	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->RegisterNotificationHandler( m_eTabIndex, this,
				CDlgLTtabctrl::INotificationHandler::NH_OnEnterTab | CDlgLTtabctrl::INotificationHandler::NH_OnLeaveTab );
	}

	return TRUE;
}
