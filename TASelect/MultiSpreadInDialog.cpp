#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "SSheet.h"
#include "RViewDescription.h"
#include "MultiSpreadInDialog.h"

CMultiSpreadInDialog::CMultiSpreadInDialog( bool fUseOnlyOneSpread, UINT nID )
	:  CDialogEx( nID ), CMultiSpreadBase( fUseOnlyOneSpread )
{
}

BEGIN_MESSAGE_MAP( CMultiSpreadInDialog, CDialogEx )
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
	ON_MESSAGE( WM_USER_MAINFRAMERESIZE, OnMainFrameResize )
	// Spread DLL messages.
	ON_MESSAGE( SSM_TEXTTIPFETCH, TextTipFetch )
	ON_MESSAGE( SSM_LEAVECELL, LeaveCell )
	ON_MESSAGE( SSM_ROWHEIGHTCHANGE, RowHeightChange )
END_MESSAGE_MAP()

BOOL CMultiSpreadInDialog::PreTranslateMessage( MSG *pMsg )
{
	BasePreTranslateMessage( pMsg );
	return __super::PreTranslateMessage( pMsg );
}

void CMultiSpreadInDialog::OnPaint()
{
	SetBackgroundColor( _WHITE, 0 );
	CDialogEx::OnPaint();
}

void CMultiSpreadInDialog::OnSize( UINT nType, int cx, int cy )
{
	bool fCheckScrollbarRunning;
	EnterCriticalSection( &m_CriticalSection );
	fCheckScrollbarRunning = m_bScrollbarRunning;
	LeaveCriticalSection( &m_CriticalSection );

	if( false == fCheckScrollbarRunning )
	{
		CDialogEx::OnSize( nType, cx, cy );
	}

	CMultiSpreadBase::BaseOnSize( nType, cx, cy );
}

void CMultiSpreadInDialog::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	CMultiSpreadBase::BaseOnHScroll( this, nSBCode, nPos, pScrollBar );
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMultiSpreadInDialog::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	CMultiSpreadBase::BaseOnVScroll( this, nSBCode, nPos, pScrollBar );
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

LRESULT CMultiSpreadInDialog::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	return BaseMM_OnMessageReceive( wParam, lParam );
}

LRESULT CMultiSpreadInDialog::OnMainFrameResize( WPARAM wParam, LPARAM lParam )
{
	// Reset the scrolling because in some circumstances when resizing the mainframe it's difficult to correct the scrolling if exist.
	int iVertPos, iHorzPos;
	GetScrollingPos( iVertPos, iHorzPos );

	SCROLLINFO rScrollInfo;
	ZeroMemory( &rScrollInfo, sizeof( SCROLLINFO ) );
	rScrollInfo.cbSize = sizeof( SCROLLINFO );
	rScrollInfo.fMask = SIF_ALL;
	GetScrollInfo( SB_HORZ, &rScrollInfo );
	rScrollInfo.nPos = 0;
	rScrollInfo.nTrackPos = 0;
	SetScrollInfo( SB_HORZ, &rScrollInfo );

	GetScrollInfo( SB_VERT, &rScrollInfo );
	rScrollInfo.nPos = 0;
	rScrollInfo.nTrackPos = 0;
	SetScrollInfo( SB_VERT, &rScrollInfo );

	ScrollWindow( iHorzPos, iVertPos );

	return 0;
}

void CMultiSpreadInDialog::OnOK()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ENTER] key).
}

void CMultiSpreadInDialog::OnCancel()
{
	// Don't call base class! (it's to avoid to loose focus when pressing [ESCAPE] key).
}

LRESULT CMultiSpreadInDialog::TextTipFetch( WPARAM wParam, LPARAM lParam )
{
	return BaseTextTipFetch( wParam, lParam );
}

LRESULT CMultiSpreadInDialog::LeaveCell( WPARAM wParam, LPARAM lParam )
{
	return BaseLeaveCell( wParam, lParam );
}

LRESULT CMultiSpreadInDialog::RowHeightChange( WPARAM wParam, LPARAM lParam )
{
	return BaseRowHeightChange( wParam, lParam );
}

void CMultiSpreadInDialog::OnUpdateMenuText( CCmdUI *pCmdUI )
{
	BaseOnUpdateMenuText( pCmdUI );
}
