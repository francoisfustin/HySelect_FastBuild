#pragma once


#include "DialogExt.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "MyToolTipCtrl.h"


class CDlgStorageTankMaxTemperature : public CDialogExt
{
public:
	enum { IDD = IDD_DLGSTORAGETANKMAXTEMPERATURE };
	CDlgStorageTankMaxTemperature( CPMInputUser *pclPMInputUser, CWnd *pParent = NULL );
	virtual ~CDlgStorageTankMaxTemperature() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	virtual afx_msg void OnEnSetFocusMaxTemperature();
	virtual afx_msg void OnKillFocusMaxTemperature();
	virtual afx_msg void OnEditEnterMaxTemperature( NMHDR *pNMHDR, LRESULT *pResult );

// Protected variables.
protected:
	double m_dEditMaxTemperatureSaved;

	CPMInputUser *m_pclPMInputUser;
	CExtStatic m_StaticMaxTemperature;
	CExtNumEdit m_EditMaxTemperature;
	double m_dMaxTemperatureSaved;

	CMyToolTipCtrl m_ToolTip;
};
