#pragma once


// Base class for the 'CDlgBatchSelBCv' and 'CDlgBatchSelPICv' classes.
class CDlgBatchSelCtrlBase : public CDlgBatchSelBase
{
public:

	CDlgBatchSelCtrlBase( CBatchSelCtrlParamsBase &clBatchSelCtrlParams, UINT nID = 0, CWnd* pParent = NULL );

	virtual ~CDlgBatchSelCtrlBase() {}

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnCbnSelChangePowerSupply();
	afx_msg void OnCbnSelChangeInputSignal();
	afx_msg void OnCbnSelChangeDRPFct();
	afx_msg void OnBnClickedFailSafe();

	// Overrides 'CDlgBatchSelBase' protected virtual methods.
	virtual void GetTypeList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetFamilyList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetMaterialList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetConnectList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetVersionList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetPNList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );

	// Notify the inherited classes that the treatment for the combo change is finished.
	// It allows the inherited classes to do some work (Like change actuator combos for control batch).
	virtual void EndOfComboChange();
	// HYS-1263 : Added to update application type if it is necessary
	virtual void ActivateLeftTabDialog();
	// HYS-1458 : Rename column
	virtual void RenameColHeader() { return; }

	void FillComboPowerSupply( CString strPowerSupplyID = _T("") );
	void FillComboInputSignal( CString strInputSignalID = _T("") );
	void SetCheckFailSafeFct();
	void FillComboDRPFct( CDB_ControlValve::DRPFunction eDRPFct = CDB_ControlValve::drpfUndefined );
	void SetCheckStatus( int iStatus ) { m_CheckStatus = iStatus; }
	void VerifyCheckboxFailsafeStatus();
	
// Protected variables.
protected:
	CExtNumEditComboBox m_ComboPowerSupply;
	CExtNumEditComboBox m_ComboInputSignal;
	CExtNumEditComboBox m_ComboDRPFct;
	CButton m_CheckFailSafeFct;
	int m_CheckStatus;
	CXGroupBox m_GroupActuator;

// Private variables.
private:
	CBatchSelCtrlParamsBase *m_pclBatchSelCtrlParams;
	std::map<ProductSubCategory, CTADatabase::CvTargetTab> m_mapLeftTabID2CvTargetTab;
};
