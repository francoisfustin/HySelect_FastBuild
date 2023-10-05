#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "ExtButton.h"
#include "DialogExt.h"
#include "ExtComboBox.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_CoolingPMType : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLPMTYPE };

	CDlgWizardPM_RightViewInput_CoolingPMType( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' public virtual method.
	virtual void ApplyPMInputUserUpdated( bool bWaterCharUpdated = false, bool bShowErrorMsg = true );
	virtual int IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced );
	//////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnCbnSelChangePressureMaintenanceType();
	virtual afx_msg void OnCbnSelChangeWaterMakeUpType();

	virtual afx_msg void OnEnSetFocusWaterMakeUpNetworkPN();
	virtual afx_msg void OnEnSetFocusWaterHardness();

	virtual afx_msg void OnKillFocusWaterMakeUpNetworkPN();
	virtual afx_msg void OnKillFocusEditWaterHardness();

	virtual afx_msg void OnBnClickedDegassing();

	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual void GetExtEditList( CArray<CExtNumEdit*> &arExtEditList );

// Private methods.
private:
	void _VerifyInputValues( void );

// Private variables.
private:
	CExtStatic m_StaticPMType;
	CExtStatic m_StaticWaterMakeUpType;
	CExtStatic m_StaticWaterMakeUpNetworkPN;
	CExtStatic m_StaticWaterHardness;

	CExtNumEditComboBox m_ComboPMType;
	CExtNumEditComboBox m_ComboWaterMakeUpType;
	CExtNumEdit m_ExtEditWaterMakeUpNetworkPN;
	CExtNumEdit m_ExtEditWaterHardness;
	CExtButton m_clButtonDegassing;

	CExtStatic m_StaticWaterMakeUpNetworkPNUnit;
	CExtStatic m_StaticWaterHardnessUnit;
};
