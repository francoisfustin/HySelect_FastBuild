#pragma once

#include "MyToolTipCtrl.h"
#include "afxdialogex.h"
#include "select.h"
#include "Global.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "EnBitmap.h"

class CDlgIndSel6WayValveCircuits : public CDialogEx
{
public:
	CDlgIndSel6WayValveCircuits( bool *bIsModified, e6WayValveSelectionMode eSelectionMode, CWnd *pParent = NULL );
	~CDlgIndSel6WayValveCircuits();
	enum { IDD = IDD_DLGINDSEL6WAYVALVE_CIRCUITS };

	e6WayValveSelectionMode GetSelectionMode6wayValve() { return m_eSavedSelectionMode; }

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedImgA();
	afx_msg void OnBnClickedImgB();
	afx_msg void OnBnClickedImgC();
	afx_msg void OnBnClickedImgD();

	CExtButton* GetDlgCircuitFocus();

	e6WayValveSelectionMode m_eCurrentSelectionMode;
	e6WayValveSelectionMode m_eSavedSelectionMode;
	bool *m_pbIsModified;
	CExtButton	m_ButtonOk;
	CExtButton	m_ButtonCancel;

	CExtStatic m_StaticTextA;
	CExtStatic m_StaticTextB;
	CExtStatic m_StaticTextC;
	CExtStatic m_StaticTextD;

	CEnBitmap m_BmpA;
	CEnBitmap m_BmpB;
	CEnBitmap m_BmpC;
	CEnBitmap m_BmpD;

	CExtButton m_ButtonA;
	CExtButton m_ButtonB;
	CExtButton m_ButtonC;
	CExtButton m_ButtonD;

	// HYS-1429 : Do not show unavailable circuits
	bool m_bCanEQMEnabled;
	bool m_bCanOnoffPIBCVEnabled;
	bool m_bCanOnoffBVEnabled;
	bool m_bCan6WayValveEnabled;
public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
};

