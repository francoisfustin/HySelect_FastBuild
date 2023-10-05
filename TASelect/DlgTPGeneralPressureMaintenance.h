#pragma once


#include "DlgCtrlPropPage.h"

class CDlgTPGeneralPressureMaintenance : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralPressureMaintenance )

public:
	enum { IDD = IDD_TABDIALOGTECHPM };
	
	CDlgTPGeneralPressureMaintenance( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedPressureVolumeLimit();

// Protected variables.
protected:
	CButton m_clCheckPressureVolumeLimit;
	CExtNumEdit m_clEditPressureVolumeLimit;
	CExtNumEdit m_clEditMaxAdditionalVesselsInParallel;
	CExtNumEdit m_clEditMaxVentoInParallel;
	CExtNumEdit m_clEditVesselSelectionRangePercentage;
	// HYS-1126 : Simplify pst
	CButton m_clCheckSimplifyPst;
};
