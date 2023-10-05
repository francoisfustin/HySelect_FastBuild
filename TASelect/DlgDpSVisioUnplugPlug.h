#pragma once


#include "AfxDialogEx.h"
#include "ExtStatic.h"
#include "afxwin.h"

class CDlgDpSVisioUnplugPlug : public CDialogEx
{
public:
	enum { IDD = IDD_DLGDPSVISIO_UNPLUGPLUG };

	enum WaitMode
	{
		WM_Infinite,
		WM_Hourglass
	};

	CDlgDpSVisioUnplugPlug( WaitMode eWaitMode, CWnd* pParent = NULL );
	virtual ~CDlgDpSVisioUnplugPlug();

// Public methods.
public:
	void SetText( CString strText, COLORREF TextColor, int iSize, bool bBold );
	void SetHourglass( int iStep, int iFinal );
	void StopHourglass( void );

// Protected methods.
protected:
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnBnClickedCancel();

// Private variables.
private:
	WaitMode m_eWaitMode;
	CExtStatic m_StaticInfo;
	UINT_PTR m_nTimer;
	float m_bCurrentAngle;
	float m_bStep;
	float m_bFinal;
	CButton m_ButtonCancel;
	CBitmap m_clHourglassBkgBmp;
	CBitmap m_clHourglassFrgBmp;
	bool m_bBmpAlreadyChanged;
};
