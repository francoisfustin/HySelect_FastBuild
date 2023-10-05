#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_HeatingSystemVolumePower : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATSYSTEMVOLUMEPOWER };

	CDlgWizardPM_RightViewInput_HeatingSystemVolumePower( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

	DECLARE_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' public virtual method.
	virtual void ApplyPMInputUserUpdated( bool bWaterCharUpdated = false, bool bShowErrorMsg = true );
	virtual int IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced );
	//////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnPaint();
	virtual afx_msg void OnLButtonDown( UINT nFlags, CPoint point );

	virtual afx_msg void OnEnSetFocusSystemVolume();
	virtual afx_msg void OnEnSetFocusInstalledPower();

	virtual afx_msg void OnKillFocusSystemVolume();
	virtual afx_msg void OnKillFocusInstalledPower();

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	
	// Called by 'CDlgWizardPM_RightViewInput_Base::ResetToolTipAndErrorBorder'.
	virtual void GetExtEditList( CArray<CExtNumEdit*> &arExtEditList );
	//////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _VerifyInputValues( void );

// Private variables.
private:
	CExtStatic m_StaticSystemVolume;
	CExtStatic m_StaticInstalledPower;

	CExtNumEdit m_ExtEditSystemVolume;
	CExtNumEdit m_ExtEditInstalledPower;

	CExtStatic m_StaticSystemVolumeUnit;
	CExtStatic m_StaticInstalledPowerUnit;

	CStatic m_ButtonGoAdvancedMode;
	CRect m_ButtonGoAdanvedModePos;
	bool m_bGoToAdvModeMsgSent;
};

