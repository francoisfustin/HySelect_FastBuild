#pragma once


#include "SpreadComboBox.h"
#include "DialogExt.h"

class CDlgSpreadCB : public CDialogExt
{
	DECLARE_DYNAMIC(CDlgSpreadCB)

public:
	CDlgSpreadCB( int iIDD, CWnd *pParent = NULL );
	virtual ~CDlgSpreadCB();
	virtual void Refresh() {}

	void SetLParam( LPARAM lparam ){ m_lparam = lparam; }
	LPARAM GetLParam(){ return m_lparam; }

	void SetMainSheetPos( CSSheet *pSheet, long lColumn, long lRow );
	BOOL Create();
	bool IsOpened() { return m_bOpen; }
	void InvalidateParentWnd();

// Protected members
protected:
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT OnCloseCB( WPARAM wParam, LPARAM lParam );

	CRect GetComboPos( CRect rect );

	afx_msg virtual void OnDestroy();
	afx_msg virtual void OnActivate( UINT nState, CWnd *pWndOther, BOOL fMinimized );
	DECLARE_MESSAGE_MAP()

// Protected variables
protected:
	int m_iIDD;
	CWnd *m_pParent;
	UINT_PTR m_nTimer;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CUserDatabase *m_pUSERDB;
	CUnitDatabase *m_pUnitDB;
	CWaterChar *m_pWC;
	CTableDN *m_pclTableDN;
	CDS_TechnicalParameter *m_pTechParam;
	CDS_HydroMod *m_pHM;
	
	// Main spread sheet col and row
	long m_lMainSpreadColumn, m_lMainSpreadRow;
	bool m_bModified;
	bool m_bOpen;
	CSSheet *m_pMainSheet;
	LPARAM	m_lparam;
};

void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime );
extern CDlgSpreadCB *pDlgSpreadCB;
