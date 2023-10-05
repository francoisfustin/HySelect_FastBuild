#pragma once


#include "ExtComboBox.h"
#include "SelectPM.h"

class CDlgPMWQSelectionPrefs : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgPMWQSelectionPrefs )

public:
	enum { IDD = IDD_DLGPMSSELPREF };
	
	CDlgPMWQSelectionPrefs( CPMWQPrefs* pclPMWQSelectionPreferences, CTADatabase *pTADB, CWnd* pParent = NULL );
	virtual ~CDlgPMWQSelectionPrefs() {}

	bool IsChangesOperate( void ) { return m_bChanges; }

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// Private methods.
private:
	void _FillComboGenRedundancy();

// Private variables.
protected:
	CPMWQPrefs *m_pclPMWQSelectionPreferences;
	CTADatabase *m_pTADB;
	CPMWQPrefs m_clPMWQSelectPrefCopy;
	bool m_bChanges;
	CButton m_clCheckGenOneDevice;
	CExtNumEditComboBox m_ComboGenRedundancy;
	CButton m_clCheckCPInternalCoating;
	CButton m_clCheckCPExternalAir;
	CButton m_clCheckPumpDegCoolingVersion;
	CButton m_clCheckWMBreakTank;
	CButton m_clCheckWMDutyStandBy;
};
