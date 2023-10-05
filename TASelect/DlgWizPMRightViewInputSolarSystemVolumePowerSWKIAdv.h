#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKIAdv : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_SOLARSYSTEMVOLUMEPOWERSWKIADV };

	CDlgWizardPM_RightViewInput_SolarSystemVolumePowerSWKIAdv( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

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

	virtual afx_msg void OnBnClickedSolarContentMultiplierFactor();

	virtual afx_msg void OnEnSetFocusSystemVolume();
	virtual afx_msg void OnEnSetFocusSolarCollector();
	virtual afx_msg void OnEnSetFocusSolarCollectorMultiplierFactor();
	virtual afx_msg void OnEnSetFocusStorageTankVolume();
	virtual afx_msg void OnEnSetFocusStorageTankMaxTemp();
	virtual afx_msg void OnEnSetFocusInstalledPower();

	virtual afx_msg void OnKillFocusSystemVolume();
	virtual afx_msg void OnKillFocusSolarCollector();
	virtual afx_msg void OnKillFocusSolarCollectorMultiplierFactor();
	virtual afx_msg void OnKillFocusStorageTankVolume();
	virtual afx_msg void OnKillFocusStorageTankMaxTemp();
	virtual afx_msg void OnKillFocusInstallPower();

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	
	// Called by 'CDlgWizardPM_RightViewInput_Base::ResetToolTipAndErrorBorder'.
	virtual void GetExtEditList( CArray<CExtNumEdit*> &arExtEditList );
	virtual void InitToolTips( CWnd* pWnd = NULL );
	//////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _VerifyInputValues( void );

// Private variables.
private:
	CButton m_BtnCheckSolarContentMultiplierFactor;

	CExtStatic m_StaticSystemVolume;
	CExtStatic m_StaticSolarContent;
	CExtStatic m_StaticSolarContentMultiplierFactor;
	CExtStatic m_StaticStorageTankVolume;
	CExtStatic m_StaticStorageTankMaxTemp;
	CExtStatic m_StaticInstalledPower;

	CExtNumEdit m_ExtEditSystemVolume;
	CExtNumEdit m_ExtEditSolarContent;
	CExtNumEdit m_ExtEditSolarContentMultiplierFactor;
	CExtNumEdit m_ExtEditStorageTankVolume;
	CExtNumEdit m_ExtEditStorageTankMaxTemp;
	CExtNumEdit m_ExtEditInstalledPower;

	CExtStatic m_StaticSystemVolumeUnit;
	CExtStatic m_StaticSolarContentUnit;
	CExtStatic m_StaticStorageTankVolumeUnit;
	CExtStatic m_StaticStorageTankMaxTempUnit;
	CExtStatic m_StaticInstalledPowerUnit;

	double m_dSolarContentSaved;
	double m_dSolarContentMultiplerFactorSaved;

	CExtStatic m_ButtonGoNormalMode;
	CRect m_ButtonGoNormalModePos;
	bool m_bGoToNormalModeMsgSent;
};
