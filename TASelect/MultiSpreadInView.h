#pragma once


#include "FormViewEx.h"
#include "MultiSpreadBase.h"

class CMultiSpreadInView : public CFormViewEx, public CMultiSpreadBase
{
public:
	enum
	{
		IDD = IDV_RVIEW
	};

	CMultiSpreadInView( CMainFrame::RightViewList eRightViewID, bool fUseOnlyOneSpread = true, UINT nId = IDD );
	virtual ~CMultiSpreadInView() {}
	
	CMainFrame::RightViewList GetRViewID( void ) { return m_eRViewID; }
	BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	
	BOOL PreTranslateMessage( MSG *pMsg );

	// Windows message handlers.
	afx_msg void OnPaint();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );

	// Spread DLL message handlers.
	afx_msg LRESULT TextTipFetch( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT LeaveCell( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT RowHeightChange( WPARAM wParam, LPARAM lParam );

	afx_msg void OnUpdateMenuText( CCmdUI* pCmdUI );

	// Overrides 'CFormViewEx' virtual methods.
	virtual void OnInitialUpdate( );

	// Overrides 'CMultiSpreadBase' protected pure virtual methods.
	virtual CWnd* GetpWnd( void ) { return this; }

// Protected variables.
protected:
	CMainFrame::RightViewList m_eRViewID;
};
