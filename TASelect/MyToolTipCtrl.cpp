#include "stdafx.h"
#include "MyToolTipCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// FF: 2019-05-29: this class has been modified to support tooltip on disabled controls.
// https://www.codeguru.com/cpp/controls/controls/tooltipcontrols/article.php/c2277/Tooltip-for-disabled-controls.htm
// See Vladimir Belinkis answers in the replies.

CMyToolTipCtrl::CMyToolTipCtrl()
{
	m_pParent = NULL;
}

CMyToolTipCtrl::~CMyToolTipCtrl()
{
}

BOOL CMyToolTipCtrl::Create( CWnd *pParentWnd, DWORD dwStyle ) 
{ 
	m_pParent = pParentWnd;

	// Call base class.
	return CToolTipCtrl::Create( pParentWnd, dwStyle );
}

BOOL CMyToolTipCtrl::AddToolWindow( CWnd *pWnd, LPCTSTR pszText )
{
	m_ToolList.AddTail( pWnd );

	TOOLINFO ti;
	ti.cbSize = sizeof( TOOLINFO );
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = pWnd->GetParent()->GetSafeHwnd();
	ti.uId = (UINT)pWnd->GetSafeHwnd();
	ti.hinst = AfxGetInstanceHandle();
	ti.lpszText = (LPTSTR)pszText;

	return (BOOL)SendMessage( TTM_ADDTOOL, 0, (LPARAM)&ti );
}

BOOL CMyToolTipCtrl::AddToolRect( CWnd *pWnd, LPCTSTR pszText, LPCRECT lpRect, UINT nIDTool )
{
	TOOLINFO ti;
	ti.cbSize = sizeof( TOOLINFO );
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = pWnd->GetSafeHwnd();
	ti.uId = nIDTool;
	ti.hinst = AfxGetInstanceHandle();
	ti.lpszText = (LPTSTR)pszText;
	::CopyRect( &ti.rect, lpRect );

	return (BOOL)SendMessage( TTM_ADDTOOL, 0, (LPARAM)&ti );
}

void CMyToolTipCtrl::RelayEvent( LPMSG pMsg ) 
{ 
	// To be able to show tooltip even if the control is disabled, you need to override the 'CWnd::PreTranslateMessage' method
	// and manually call this method (Example in 'CDlgSelectionBase::PreTranslateMessage' method).

	if( pMsg->message != WM_MOUSEMOVE || NULL == m_pParent || pMsg->hwnd != m_pParent->m_hWnd )
	{ 
		// Call base class.
		CToolTipCtrl::RelayEvent( pMsg ); 
		return; 
	}

	// Mouse moved over parent.
	// If control is disabled show tool tip anyway.

	// Pass changed message to a tool tip control for processing.
	MSG myMsg;
	::CopyMemory( &myMsg, pMsg, sizeof( MSG ) ); 

	CPoint pt; 
	CRect rect;

	pt.x = LOWORD( pMsg->lParam ); // horizontal position of cursor 
	pt.y = HIWORD( pMsg->lParam ); // vertical position of cursor

	POSITION pos = m_ToolList.GetHeadPosition(); 

	while( NULL != pos )
	{
		CWnd *pWndIter = m_ToolList.GetNext( pos );

		pWndIter->GetWindowRect( &rect );
		m_pParent->ScreenToClient( &rect );

		if( TRUE == rect.PtInRect( pt ) )
		{ 
			//---------------------------------------------------------------- 
			// The mouse is inside the control 
			// 
			// 1. We change the Msg hwnd to the controls hWnd 
			// 2. We change the Msg lParam to the controls client coordinates 
			// 
			//----------------------------------------------------------------

			myMsg.hwnd = pWndIter->m_hWnd; 

			m_pParent->ClientToScreen( &pt );
			pWndIter->ScreenToClient( &pt );
			myMsg.lParam = MAKELPARAM( pt.x, pt.y );

			// Pass the changed message to the base class.
			CToolTipCtrl::RelayEvent( &myMsg );
			return;
		}
	}

	// Pass the changed message to the base class.
	CToolTipCtrl::RelayEvent( pMsg );
} 

BEGIN_MESSAGE_MAP( CMyToolTipCtrl, CToolTipCtrl )
END_MESSAGE_MAP()

