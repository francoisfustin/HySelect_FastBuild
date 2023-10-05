#pragma once


#include "DlgCtrlPropPage.h"

class CDlgCtrlPropPageMng : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgCtrlPropPageMng )

public:
	CDlgCtrlPropPageMng( UINT nIDTemplate, CWnd *pParent = NULL );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();

	void DisplayPage( CDlgCtrlPropPage* pPage );
	void SetCurPage( CDlgCtrlPropPage* pPage ) { m_pCurPage = pPage; }

// Protected variables.
protected:
	CDlgCtrlPropPage*	m_pCurPage;		// Current page object
	CRect				m_PageRect;
	int					m_iHeaderHeight;
};
