#pragma once


#include "DialogExt.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "ProductSelectionParameters.h"

class CDlgBatchSelDpC_DpCTab : public CDialogExt
{
public:
	enum { IDD = IDD_DLGBATCHSELDPC_DPCTAB };
	CDlgBatchSelDpC_DpCTab( CBatchSelDpCParams &clBatchSelDpCParams, CWnd *pParent = NULL );
	virtual ~CDlgBatchSelDpC_DpCTab() {}

	void SaveSelectionParameters();
	void EnableCombos( bool bEnable );
	void UpdateCombos( void );
	LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	virtual afx_msg void OnComboSelChangeTypeBelow65();
	virtual afx_msg void OnComboSelChangeFamilyBelow65();
	virtual afx_msg void OnComboSelChangeBdyMatBelow65();
	virtual afx_msg void OnComboSelChangeConnectBelow65();
	virtual afx_msg void OnComboSelChangeVersionBelow65();
	virtual afx_msg void OnComboSelChangePNBelow65();
		
	virtual afx_msg void OnComboSelChangeTypeAbove50();
	virtual afx_msg void OnComboSelChangeFamilyAbove50();
	virtual afx_msg void OnComboSelChangeBdyMatAbove50();
	virtual afx_msg void OnComboSelChangeConnectAbove50();
	virtual afx_msg void OnComboSelChangeVersionAbove50();
	virtual afx_msg void OnComboSelChangePNAbove50();

	void FillComboBoxType( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strTypeID = _T("") );
	void FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strFamilyID = _T("") );
	void FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strBdyMatID = _T("") );
	void FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strConnectID = _T("") );
	void FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strVersionID = _T("") );
	void FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strPNID = _T("") );

// Protected variables.
protected:
	CBatchSelDpCParams *m_pclBatchSelDpCParams;

	CXGroupBox m_clGroupValveBelow65;
	CXGroupBox m_clGroupValveAbove50;

	// Valves below DN 65.
	CExtNumEditComboBox m_ComboTypeBelow65;
	CExtNumEditComboBox m_ComboFamilyBelow65;
	CExtNumEditComboBox m_ComboBodyMaterialBelow65;
	CExtNumEditComboBox m_ComboConnectBelow65;
	CExtNumEditComboBox m_ComboVersionBelow65;
	CExtNumEditComboBox m_ComboPNBelow65;

	// Valves above DN 50.
	CExtNumEditComboBox m_ComboTypeAbove50;
	CExtNumEditComboBox m_ComboFamilyAbove50;
	CExtNumEditComboBox m_ComboBodyMaterialAbove50;
	CExtNumEditComboBox m_ComboConnectAbove50;
	CExtNumEditComboBox m_ComboVersionAbove50;
	CExtNumEditComboBox m_ComboPNAbove50;
};
