#pragma once


#include "afxwin.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "ExtListCtrl.h"
#include "DlgCtrlPropPage.h"
#include "DlgSearchAndReplaceCircuits.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
// HYS-1324 : All class was reviewed. Combos are deleted, checkboxes and radio button are used.   //
// Functionaly, the algorithm have been reviewed.                                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////
class CDlgSRPageCircuit : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgSRPageCircuit )

public:
	enum { IDD = IDD_TABDLGSEARCHCIRCUITS };

	CDlgSRPageCircuit( CWnd *pParent = NULL );
	virtual ~CDlgSRPageCircuit() {}
	
	// Overrides 'CDlgCtrlPropPage'.
	virtual void Init( bool bRstToDefault = false );
	virtual bool OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams );

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );

	afx_msg void OnClickedCheckCircuits();
	afx_msg void OnClickedCheckModules();
	afx_msg void OnBnClickedCheckApplylocked();

	// When one checkbox receives a mouse click in the balancing type group.
	afx_msg void OnBnClickedCheckBalancingType();

	// When one radio button receives a mouse click in the replace balancing type group.
	afx_msg void OnBnClickedRadioManBalancingType();
	
	// When the listctrl receives a mouse click.
	afx_msg void OnSelchangeListCircuitTypeSrc( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnSelchangeListCircuitTypeDest( NMHDR *pNMHDR, LRESULT *pResult );
	
	// When one checkbox receives a mouse click in the valve type group.
	afx_msg void OnBnClickedCheckIMIValveType();
	afx_msg void OnBnClickedCheckKvsValveType();
	
	// When one radio button receives a mouse click in the replace valve type group.
	afx_msg void OnBnClickedRadioValveTypeDest();
	
	// When one checkbox receives a mouse click in the control type group.
	afx_msg void OnBnClickedCheckControlType();
	
	// When one radio button receives a mouse click in the replace control type group.
	afx_msg void OnBnClickedRadioControlType();
	
	// When one checkbox receives a mouse click in the control valve type group.
	afx_msg void OnBnClickedCheckControlValveType();
	
	// When one radio button receives a mouse click in the replace control valvetype group.
	afx_msg void OnBnClickedRadioControlValveType();

	afx_msg LRESULT OnTreeItemCheckStatusChanged( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnFindNextCircuit( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnReplaceCircuit( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnReplaceAllCircuit( WPARAM wParam=0, LPARAM lParam = 0 );
	afx_msg LRESULT OnClosePopup( WPARAM wParam = 0, LPARAM lParam = 0 );

private:
	// Init the listctrl to list all circuits available in Hydromod.
	void _InitListsControlCircuit();
	
	// Update group, checkboxes, radio button availability.
	void _FillInBalancingTypeSrc();
	void _FillInBalancingTypeDest();
	
	// Update group, checkboxes, radio button availability.
	void _FillInValveTypeSrc();
	void _FillInValveTypeDest();
	
	// Update group, checkboxes, radio button availability.
	void _FillInCtrlTypeSrc();
	void _FillInCtrlTypeDest();
	
	// Update group, checkboxes, radio button availability.
	void _FillInCtrlValveTypeSrc();
	void _FillInCtrlValveTypeDest();
	
	// Update list by disabling, enabling ciruits.
	void _FillInListCircuitTypeSrc();
	void _FillInListCircuitTypeDest();
	
	// Look if HM is compatible with each find element state
	bool _CheckCompatibilityHMvsCB( CDS_HydroMod *pHM, CString strCurrentGroup = _T("") );
	
	void _UpdateGroupMBType();
	void _UpdateGroupValveType();
	void _UpdateGroupCtrlType();
	void _UpdateGroupCVType();
	void _UpdateStartButton();
	
	// Update list appearance.
	void _UpdateListCtrl( CExtListCtrl *pListCtrl, std::vector<int> &vecSelectedIndex, std::map<int, bool> mapCircuitFound, bool bNoCircuitFound );

	// For the left part.
	bool _IsAtLeastOneValveTypeChecked();
	bool _IsAtLeastOneControlTypeChecked();
	bool _IsAtLeastOneControlValveTypeChecked();

	void _GetBalancingTypeSelectedSrc( std::vector<CDB_CircuitScheme::eBALTYPE> &vecBalancingTypeSelected );
	void _GetCircuitTypeSelectedSrc( std::vector<CDB_CircSchemeCateg *> &vecCircuitTypeSelected );
	void _GetValveTypeSelectedSrc( std::vector<_eWorkForCV> &vecValveTypeSelected );
	void _GetControlTypeSelectedSrc( std::vector<CDB_ControlProperties::CvCtrlType> &vecControlTypeSelected );
	void _GetControlValveTypeSelectedSrc( std::vector<CDB_ControlProperties::eCVFUNC> &vecControlValveTypeSelected );

	// For the right part.
	CDB_CircSchemeCateg *_GetCircuitTypeSelectedDest();
	void _UpdateBalancingTypeValueDest();
	void _UpdateValveTypeValueDest();
	void _UpdateControlTypeValueDest();
	void _UpdateControlValveTypeValueDest();

// Internal variables.
private:
	CXGroupBox m_GroupFind;
	CXGroupBox m_GroupReplace;

	CXGroupBox m_GroupWhereSrc;
	CButton m_CheckWhereModules;
	CButton m_CheckWhereCircuits;
	CButton m_CheckWhereApplyToLockedCircuit;

	CXGroupBox m_GroupBalancingTypeSrc;
	CButton m_CheckBTNoneSrc;
	CButton m_CheckBTManualBalancingSrc;
	CButton m_CheckBTDiffPressureControlSrc;
	CButton m_CheckBTElectronicSrc;
	CButton m_CheckBTElectronicDpCSrc;
	
	CXGroupBox m_GroupBalTypeDest;
	CDB_CircuitScheme::eBALTYPE m_eRadioBalancingTypeDest;
	int m_iRadioBalancingTypeDest;

	CExtListCtrl m_ListBoxCircuitTypeSrc;
	bool m_bAtLeastOneCircuitTypeWithCVSrc;
	CExtListCtrl m_ListBoxCircuitTypeDest;

	CXGroupBox m_GroupValveTypeSrc;
	CButton m_CheckValveTypeIMISrc;
	CButton m_CheckValveTypeKvsSrc;

	CXGroupBox m_GroupValveTypeDest;
	_eWorkForCV m_eRadioValveTypeDest;
	int m_iRadioValveTypeDest;

	CXGroupBox m_GroupControlTypeSrc;
	CButton m_CheckControlTypeProportionalSrc;
	CButton m_CheckControlType3PointsSrc;
	CButton m_CheckControlTypeOnOffSrc;

	CXGroupBox m_GroupControlTypeDest;
	CDB_ControlProperties::CvCtrlType m_eRadioControlTypeDest;
	int m_iRadioControlTypeDest;

	CXGroupBox m_GroupControlValveTypeSrc;
	CButton m_CheckControlValveTypeStandardCVSrc;
	CButton m_CheckControlValveTypeCombBalCVSrc;
	CButton m_CheckControlValveTypePresetCvSrc;
	CButton m_CheckControlValveTypePIBCVSrc;
	CButton m_CheckControlValveTypeSmartSrc;
	
	CXGroupBox m_GroupControlValveTypeDest;
	CDB_ControlProperties::eCVFUNC m_eRadioControlValveTypeDest;
	int m_iRadioControlValveTypeDest;

	bool m_bApplyToAll;

	CDlgSearchAndReplaceCircuits *m_pDlgSearchAndReplaceCircuits;
	std::map<int, CDS_HydroMod *> m_mapSelectedCicuits;
	std::map<int, CDS_HydroMod *>::iterator m_mapSelCirIt;

	std::map<WPARAM, CDS_HydroMod *> m_mapModifiedHM;		// Keep a list of modified HM pointer, needed to refresh Tree content
	std::map<WPARAM, CDS_HydroMod *>::iterator m_mapModifiedHMIt;

	bool m_bOnSelchangeListCircuitTypeEnabled;
};
