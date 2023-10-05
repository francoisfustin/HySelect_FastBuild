#pragma once


#include "HMInclude.h"
#include "MyToolTipCtrl.h"
#include "SSheet.h"
#include "XGroupBox.h"

class CDlgSelectActuator : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgSelectActuator )

public:
	enum { IDD = IDD_DLGSELECTACTUATOR };

	CDlgSelectActuator( CTADatabase::FilterSelection eFilterSelection, bool bStartForCombo = false, CWnd *pParent = NULL );
	virtual ~CDlgSelectActuator() {}

public:
	void OpenDlgSelectActr( CDS_HydroMod *pHM,  long lCol, long lRow );
	int InitCompliantActuatorList();
	CDS_HydroMod *GetSelectedHM() { return m_pHM; }

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual LRESULT OnCloseCB( WPARAM wParam, LPARAM lParam );
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage( MSG *pMsg );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedGroupBox();
	afx_msg void OnClickedCheckPackage();
	afx_msg void OnComboSelChangePowerSupply();
	afx_msg void OnComboSelChangeInputSignal();
	afx_msg void OnBnClickedCheckFailSafe();
	afx_msg void OnComboSelChangeDRP();
	afx_msg void OnComboSelChangeActuator();
	afx_msg void OnBnClickedOk();

	afx_msg void OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized );
	afx_msg void OnTimer( UINT_PTR nIDEvent );

// Private methods.
private:
	void _Refresh();
	void _FillCheckBoxPackage();
	void _FillCBPowerSupply();
	void _FillCBInputSignal();
	void _SetCheckFailSafeFct( );
	void _FillCBDefaultReturnPos();
	void _FillCBActuator();
	void _MaptoRankList(std::multimap< double, CDB_Actuator *> *pActrMap, CRank *pRkList);
	int _FindCBLParam(CComboBox *pCB, DWORD_PTR DataMP);

// Private variables.
private:
	CTADatabase::FilterSelection m_eFilterSelection;
	CDS_HydroMod *m_pHM;
	CRankEx m_ActrkList;
	std::multimap< double, CDB_Actuator *> m_ActrFullList;
	std::multimap< double, CDB_Actuator *> m_ActrPackageCompliantList;
	std::multimap< double, CDB_Actuator *>::iterator m_IterActr;

	bool m_bStartForCombo;
	CRect m_DlgPos;
	long m_lRow, m_lCol;
	CSSheet *m_pMainSpreadSheet;
	UINT_PTR m_nTimer;
	bool m_bOpen;
	CPrjParams *m_pTechParams;
	
	CButton m_CheckPackage;
	CXGroupBox m_GroupBox;
	CComboBox m_CBActuator;
	CComboBox m_CBDefaultReturnPosition;
	CComboBox m_CBInputSignal;
	CComboBox m_CBPowerSupply;
	CEdit m_EditSelectedActuator;
	CButton m_CheckboxFailSafe;
	int m_CheckBoxStatus;
	bool m_bDisableActivateMessage;

	CMyToolTipCtrl m_ToolTip;
};
