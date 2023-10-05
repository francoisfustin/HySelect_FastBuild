#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "ExtComboBox.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_CoolingTemperature : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLTEMP };

	CDlgWizardPM_RightViewInput_CoolingTemperature( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

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
	virtual afx_msg void OnEnSetFocusMaxTemp();
	virtual afx_msg void OnEnSetFocusSupplyTemp();
	virtual afx_msg void OnEnSetFocusReturnTemp();
	virtual afx_msg void OnEnSetFocusFillTemp();
	virtual afx_msg void OnKillFocusEditMaxTemp();
	virtual afx_msg void OnKillFocusEditSupplyTemp();
	virtual afx_msg void OnKillFocusEditReturnTemp();
	virtual afx_msg void OnKillFocusEditFillTemp();

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
	CExtStatic m_StaticNorm;
	CExtStatic m_StaticMaxTemp;
	CExtStatic m_StaticSupplyTemp;
	CExtStatic m_StaticReturnTemp;
	CExtStatic m_StaticFillTemp;

	CExtNumEditComboBox m_ComboNorm;
	CExtNumEdit m_ExtEditMaxTemp;
	CExtNumEdit m_ExtEditSupplyTemp;
	CExtNumEdit m_ExtEditReturnTemp;
	CExtNumEdit m_ExtEditFillTemp;

	CExtStatic m_StaticMaxTempUnit;
	CExtStatic m_StaticSupplyTempUnit;
	CExtStatic m_StaticReturnTempUnit;
	CExtStatic m_StaticFillTempUnit;

	CStatic m_ButtonGoAdvancedMode;
	CRect m_ButtonGoAdanvedModePos;
	bool m_bGoToAdvModeMsgSent;
};
