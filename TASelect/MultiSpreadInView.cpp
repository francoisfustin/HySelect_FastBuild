#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "SSheet.h"
#include "RViewDescription.h"
#include "MultiSpreadInView.h"

CMultiSpreadInView::CMultiSpreadInView( CMainFrame::RightViewList eRightViewID, bool fUseOnlyOneSpread, UINT nID )
	:  CFormViewEx( nID ), CMultiSpreadBase( fUseOnlyOneSpread )
{
	m_eRViewID = eRightViewID;
}

BOOL CMultiSpreadInView::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext )
{
	// Create the form view.
	if( FALSE == CFormViewEx::Create( lpszClassName, lpszWindowName, dwStyle | WS_HSCROLL | WS_VSCROLL , rect, pParentWnd, nID, pContext ) )
		return FALSE;

	return TRUE;
}

BEGIN_MESSAGE_MAP( CMultiSpreadInView, CFormViewEx )
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
	// Spread DLL messages.
	ON_MESSAGE( SSM_TEXTTIPFETCH, TextTipFetch )
	ON_MESSAGE( SSM_LEAVECELL, LeaveCell )
	ON_MESSAGE( SSM_ROWHEIGHTCHANGE, RowHeightChange )
END_MESSAGE_MAP()

BOOL CMultiSpreadInView::PreTranslateMessage( MSG *pMsg )
{
	BasePreTranslateMessage( pMsg );
	return __super::PreTranslateMessage( pMsg );
}

void CMultiSpreadInView::OnPaint()
{
	SetBackgroundColor( _WHITE, 0 );
	CFormViewEx::OnPaint();
}

void CMultiSpreadInView::OnSize( UINT nType, int cx, int cy )
{
	CFormViewEx::OnSize( nType, cx, cy );

	CRect rectVirtual = CMultiSpreadBase::GetTotalSize();
	SIZE rSize;
	rSize.cx = cx;
	rSize.cy = cy;

	if( rectVirtual.Width() > rSize.cx )
	{
		rSize.cx = rectVirtual.Width();
	}

	if( rectVirtual.Height() > rSize.cy )
	{
		rSize.cy = rectVirtual.Height();
	}

	SetScrollSizes( MM_TEXT, rSize );
}

void CMultiSpreadInView::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	CMultiSpreadBase::BaseOnHScroll( this, nSBCode, nPos, pScrollBar );
	CFormViewEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMultiSpreadInView::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	CMultiSpreadBase::BaseOnVScroll( this, nSBCode, nPos, pScrollBar );
	CFormViewEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CMultiSpreadInView::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( this != GetFocus() )
	{
		SetFocus();
	}

	CFormViewEx::OnLButtonDown( nFlags, point );
}

LRESULT CMultiSpreadInView::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	return BaseMM_OnMessageReceive( wParam, lParam );
}

LRESULT CMultiSpreadInView::TextTipFetch( WPARAM wParam, LPARAM lParam )
{
	return BaseTextTipFetch( wParam, lParam );
}

LRESULT CMultiSpreadInView::LeaveCell( WPARAM wParam, LPARAM lParam )
{
	return BaseLeaveCell( wParam, lParam );
}

LRESULT CMultiSpreadInView::RowHeightChange( WPARAM wParam, LPARAM lParam )
{
	return BaseRowHeightChange( wParam, lParam );
}

void CMultiSpreadInView::OnUpdateMenuText( CCmdUI *pCmdUI )
{
	BaseOnUpdateMenuText( pCmdUI );
}

void CMultiSpreadInView::OnInitialUpdate( )
{
	CRect rectView;
	GetClientRect( &rectView );
	CRect rectVirtual = CMultiSpreadBase::GetTotalSize();
	SIZE rSize;
	rSize.cx = rectView.Width();
	rSize.cy = rectView.Height();
	
	if( rectVirtual.Width() > rSize.cx )
	{
		rSize.cx = rectVirtual.Width();
	}

	if( rectVirtual.Height() > rSize.cy )
	{
		rSize.cy = rectVirtual.Height();
	}

	SetScrollSizes( MM_TEXT, rSize );
}
