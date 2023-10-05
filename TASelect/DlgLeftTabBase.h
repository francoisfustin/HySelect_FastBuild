#pragma once


#include "DialogExt.h"
#include "EnBitmap.h"

class CDlgLeftTabBase :	public CDialogExt, public CDlgLTtabctrl::INotificationHandler
{
	DECLARE_DYNCREATE( CDlgLeftTabBase )

	#define _DLGLEFTTABBASE_LOGOYSHIFT		6
	#define _DLGLEFTTABBASE_LOGOXSHIFT		4

public:
	CDlgLeftTabBase( CMyMFCTabCtrl::TabIndex eTabIndex = CMyMFCTabCtrl::TabIndex::etiUndefined, UINT nID = 0, CWnd* pParent = NULL );
	virtual ~CDlgLeftTabBase();

	virtual void OnApplicationLook( COLORREF cBackColor );
	
	// Overrides 'CDlgLTtabctrl::INotificationHandler'.
	virtual void OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex );
	virtual void OnLTTabCtrlLeaveTab( CMyMFCTabCtrl::TabIndex eTabIndex ) {}
	
	virtual void InitializeToolsDockablePaneContextID( void ) {};
	virtual bool GetToolsDockablePaneContextID( int &iContextID );

	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	CRect GetTALogoSize();
	DECLARE_MESSAGE_MAP()

protected:
	int m_iContextID;
	CEnBitmap m_Bmp;
	CStatic m_BmpContainer;
	CWnd *m_pParent;
	bool m_bToolsDockablePaneContextInitialized;
	CMyMFCTabCtrl::TabIndex m_eTabIndex;

public:
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
};
