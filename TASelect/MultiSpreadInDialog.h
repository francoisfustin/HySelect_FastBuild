#pragma once


#include "FormViewEx.h"
#include "MultiSpreadBase.h"

class CMultiSpreadInDialog : public CDialogEx, public CMultiSpreadBase
{
public:
	enum
	{
		IDD = IDV_DIALOG
	};

	CMultiSpreadInDialog( bool fUseOnlyOneSpread = true, UINT nId = IDD );
	virtual ~CMultiSpreadInDialog() {}
	
// Protected members.
protected:
	DECLARE_MESSAGE_MAP()

	BOOL PreTranslateMessage( MSG *pMsg );

	// Windows message handlers.
	afx_msg void OnPaint();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnMainFrameResize( WPARAM wParam, LPARAM lParam );
	virtual void OnOK();
	virtual void OnCancel();

	// Spread DLL message handlers.
	afx_msg LRESULT TextTipFetch( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT LeaveCell( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT RowHeightChange( WPARAM wParam, LPARAM lParam );

	afx_msg void OnUpdateMenuText( CCmdUI* pCmdUI );

	// Overrides 'CMultiSpreadBase' protected pure virtual methods.
	virtual CWnd* GetpWnd( void ) { return this; }
};
