#pragma once

// CG: This file added by 'Tip of the Day' component.

#include "DialogExt.h"


#define MAX_BUFLEN 1000
#define _TIPS_SECTIONNAME		_T("Tip")
#define _TIPS_SHOWSTARTUP		_T("StartUp")
#define _TIPS_CURRENTTIP		_T("CurrentTip")

/////////////////////////////////////////////////////////////////////////////
// CDlgTip dialog

class CFileTxt;
class CDlgTip : public CDialogExt
{
public:
	CDlgTip( CWnd* pParent = NULL );
	virtual ~CDlgTip();
	BOOL CanShowToopTip( void ) { return m_fShowStartup; }

	enum { IDD = IDD_DLGTIP };

protected:
	DECLARE_MESSAGE_MAP()
	
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	afx_msg void OnNextTip();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	void GetNextTipString( void );

// Protected variables.
protected:
	BOOL		m_fShowStartup;
	UINT		m_uiCurrentTip;
	UINT		m_uiIDCount;
	CEnBitmap	m_BulbBmp;
	CString		m_strTip;
};