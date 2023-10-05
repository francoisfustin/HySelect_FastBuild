#pragma once


#include "ExtStatic.h"
#include "XGroupBox.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_HeatingRequirement : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATREQU };

	CDlgWizardPM_RightViewInput_HeatingRequirement( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' public virtual method.
	virtual void ApplyPMInputUserUpdated( bool bWaterCharUpdated = false, bool bShowErrorMsg = true );
	//////////////////////////////////////////////////////////////////////////////////////////////////////;

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnBnClickedGeneralCombi();
	virtual afx_msg void OnBnClickedGeneralRedundancyPump();
	virtual afx_msg void OnBnClickedGeneralRedundancyTecBox();
	virtual afx_msg void OnBnClickedCompBasedCoating();
	virtual afx_msg void OnBnClickedCompBasedCompressedAir();
	virtual afx_msg void OnBnClickedPumpDegasingBasedCoolingInsulation();

// Private methods.
private:
	void _VerifyInputValues( void );

// Private variables.
private:
	CXGroupBox m_GroupGeneral;
	CXGroupBox m_GroupCompressor;
	CXGroupBox m_GroupPumpDegassing;

	CExtStatic m_StaticGeneralCombi;
	CExtStatic m_StaticGeneralRedundancyPump;
	CExtStatic m_StaticGeneralRedundancyTecBox;
	CExtStatic m_StaticCompBasedCoating;
	CExtStatic m_StaticCompBasedCompressedAir;
	CExtStatic m_StaticPumpDegassingBasedCoolingInsulation;

	CButton m_ButtonGeneralCombi;
	CButton m_ButtonGeneralRedundancyPump;
	CButton m_ButtonGeneralRedundancyTecBox;
	CButton m_ButtonCompBasedCoating;
	CButton m_ButtonCompBasedCompressedAir;
	CButton m_ButtonPumpDegassingBasedCoolingInsulation;

	bool m_bButtonGeneralCombiState;
	bool m_bButtonGeneralRedundancyPumpState;
	bool m_bButtonGeneralRedundancyTecBoxState;
	bool m_bButtonCompBasedCoatingState;
	bool m_bButtonCompBasedCompressedAirState;
	bool m_bButtonPumpDegassingBasedCoolingInsulationState;
};
