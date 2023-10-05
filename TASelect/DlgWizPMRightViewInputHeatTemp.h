#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "ExtComboBox.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_HeatingTemperature : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATTEMP };

	CDlgWizardPM_RightViewInput_HeatingTemperature( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' public virtual method.
	virtual void ApplyPMInputUserUpdated( bool bWaterCharUpdated = false, bool bShowErrorMsg = true );
	virtual int IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced );
	//////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnPaint();
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	virtual afx_msg void OnCbnSelChangeNorm();
	virtual afx_msg void OnEnSetFocusSafetyTempLimiter();
	virtual afx_msg void OnKillFocusEditSafetyTempLimiter();

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	
	// Called by 'CDlgWizardPM_RightViewInput_Base::ResetToolTipAndErrorBorder'.
	virtual void GetExtEditList( CArray<CExtNumEdit*> &arExtEditList );
	//////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _VerifyInputValues( bool bShowErrorMsg = true );

// Private variables.
private:
	CExtStatic m_StaticNorm;
	CExtStatic m_StaticSafetyTempLimiter;

	CExtNumEditComboBox m_ComboNorm;
	CExtNumEdit m_ExtEditSafetyTempLimiter;

	CExtStatic m_StaticSafetyTempLimiterUnit;

	CStatic m_ButtonGoAdvancedMode;
	CRect m_ButtonGoAdanvedModePos;
	
	bool m_bGoToAdvModeMsgSent;
};
