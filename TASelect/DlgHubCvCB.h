#pragma once


#include "DlgSpreadCB.h"
#include "ExtComboBox.h"
#include "afxwin.h"

// CDlgHubCvCB dialog

class CDlgHubCvCB : public CDlgSpreadCB
{
	DECLARE_DYNAMIC(CDlgHubCvCB)

public:
	CDlgHubCvCB(CSSheet *pOwnerSSheet);   // standard constructor
	virtual ~CDlgHubCvCB();

// Dialog Data
	enum { IDD = IDD_DLGHUBCVCB };

protected:
	CExtNumEditComboBox m_ComboCv;
	CExtNumEditComboBox m_ComboCvCtrlType;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
	CExtNumEditComboBox m_ComboPN;

	void FillComboCV();
	void FillComboCtrlType();
	void FillComboConnect();
	void FillComboVersion();
	void FillComboPN();
	void CloseDialogCB();

	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT OnCloseCB(WPARAM wParam, LPARAM lParam) { return (__super::OnCloseCB(wParam,lParam));};
	virtual BOOL OnInitDialog();
	afx_msg virtual void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg virtual void OnDestroy(){__super::OnDestroy();};
	afx_msg void OnCbnSelchangeCombocv();
	afx_msg void OnCbnSelchangeComboctrltype();
	afx_msg void OnCbnSelchangeComboconnect();
	afx_msg void OnCbnSelchangeComboversion();
	afx_msg void OnCbnSelchangeCombopn();
public:
	virtual	void Refresh();
	void OpenDialogSCB(CDS_HydroMod *pHM, long col, long row);
	afx_msg void OnPaint();
};
