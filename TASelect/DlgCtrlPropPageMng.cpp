#include "stdafx.h"
#include "TASelect.h"
#include "DlgCtrlPropPageMng.h"


IMPLEMENT_DYNAMIC( CDlgCtrlPropPageMng, CDialogEx )

CDlgCtrlPropPageMng::CDlgCtrlPropPageMng( UINT nIDTemplate, CWnd *pParent )
	: CDialogEx( nIDTemplate, pParent )
{
	m_iHeaderHeight = 32;
	m_pCurPage = NULL;
}

BEGIN_MESSAGE_MAP( CDlgCtrlPropPageMng, CDialogEx )
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CDlgCtrlPropPageMng::OnPaint()
{
	// Painting is used here to draw the header.
	
	CPaintDC dc( this );
	if( m_pCurPage != NULL && m_pCurPage->GetHeaderTitle() != _T("") )
	{
		CString strTitle = m_pCurPage->GetHeaderTitle();
		
		// Test to know if a header exist.
		if( _T("") == strTitle )
			return;
		
		// Set the background color dimensions.
		CRect rectBckgrndColor( m_PageRect.left, m_PageRect.top, m_PageRect.right, m_PageRect.top + m_iHeaderHeight );
		
		// Set background color corresponding to Office 2007 Word default options settings.
		dc.FillSolidRect( rectBckgrndColor, TASApp.GetBckColor() );

		CFont* pOldFont = dc.SelectObject ( &afxGlobalData.fontBold );
		dc.SetBkMode( TRANSPARENT );
		if( ID_VIEW_APPLOOK_OFF_2007_BLUE == TASApp.GetAppLook() || ID_VIEW_APPLOOK_OFF_2007_AQUA == TASApp.GetAppLook() )
			dc.SetTextColor( RGB( 0, 21, 110 ) );
		else if( ID_VIEW_APPLOOK_OFF_2007_BLACK == TASApp.GetAppLook() || ID_VIEW_APPLOOK_OFF_2007_SILVER == TASApp.GetAppLook() )
			dc.SetTextColor( RGB( 70, 70, 70 ) );
		else
			dc.SetTextColor( _WHITE_DLGBOX );
		
		CRect rectTitleText( rectBckgrndColor.left + 3, rectBckgrndColor.top, rectBckgrndColor.right, rectBckgrndColor.bottom );
		dc.DrawText( strTitle, rectTitleText, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_EXPANDTABS );
		dc.SelectObject( pOldFont );
	}
}

void CDlgCtrlPropPageMng::DisplayPage( CDlgCtrlPropPage* pPage )
{
	int iOffset = 0;
	
	// Set the height of the header.
	if( pPage->GetHeaderTitle() != _T("") )
		iOffset = m_iHeaderHeight;
	
	// Calculate correct size and position for 'pPage'.
	CRect rect( m_PageRect.left, m_PageRect.top + iOffset, m_PageRect.right, m_PageRect.bottom );
	
	// Display 'pPage' and hide previous child dlg.
	pPage->SetWindowPos( &CWnd::wndTop, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW );
	if( NULL != m_pCurPage && m_pCurPage != pPage )
	{
		m_pCurPage->ShowWindow( SW_HIDE );
	}
	m_pCurPage = pPage;

	// Useful to change data into the header.
	Invalidate();
}
