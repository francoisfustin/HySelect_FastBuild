#include "stdafx.h"
#include "TASelect.h"
#include "MFCRibbonBarEx.h"

BOOL CMFCRibbonBarEx::OnShowRibbonContextMenu( CWnd* pWnd, int x, int y, CMFCRibbonBaseElement* pHit )
{
	return FALSE;
}

BOOL CMFCRibbonBarEx::PreTranslateMessage( MSG* pMsg )
{
	// Do not interpret double click on the ribbon tabs.
	// That will avoid refreshing problems.
	if( WM_LBUTTONDBLCLK == pMsg->message && VK_LBUTTON == pMsg->wParam )
	{
		POINT pt = pMsg->pt;
		ScreenToClient( &pt );

		if( pt.y >= m_nCaptionHeight && pt.y <= m_nCaptionHeight + m_nTabsHeight )
		{
			return TRUE;
		}
	}

	return CMFCRibbonBar::PreTranslateMessage( pMsg );
}