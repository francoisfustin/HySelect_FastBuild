#pragma once


#include "ExtButton.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "DialogExt.h"
#include "ExtComboBox.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_CoolingTemperatureAdvance : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLTEMPADV };

	CDlgWizardPM_RightViewInput_CoolingTemperatureAdvance( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

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
	virtual afx_msg void OnEnSetFocusMinTemp();

	virtual afx_msg void OnKillFocusEditMaxTemp();
	virtual afx_msg void OnKillFocusEditSupplyTemp();
	virtual afx_msg void OnKillFocusEditReturnTemp();
	virtual afx_msg void OnKillFocusEditFillTemp();
	virtual afx_msg void OnKillFocusEditMinTemp();

	virtual afx_msg void OnBnClickedCheckMinTemp();
	virtual afx_msg void OnBnClickedWaterChar();

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
	bool _UpdateMinTemperature( void );

// Private variables.
private:
	CExtStatic m_StaticNorm;
	CExtStatic m_StaticMaxTemp;
	CExtStatic m_StaticSupplyTemp;
	CExtStatic m_StaticReturnTemp;
	CExtStatic m_StaticFillTemp;
	CExtStatic m_StaticMinTemp;
	CExtStatic m_StaticWaterChar;

	CExtNumEditComboBox m_ComboNorm;
	CExtNumEdit m_ExtEditMaxTemp;
	CExtNumEdit m_ExtEditSupplyTemp;
	CExtNumEdit m_ExtEditReturnTemp;
	CExtNumEdit m_ExtEditFillTemp;
	CButton m_clCheckMinTemp;
	CExtNumEdit m_ExtEditMinTemp;
	CExtButton m_ButtonWaterChar;

	CExtStatic m_StaticMaxTempUnit;
	CExtStatic m_StaticSupplyTempUnit;
	CExtStatic m_StaticReturnTempUnit;
	CExtStatic m_StaticFillTempUnit;
	CExtStatic m_StaticMinTempUnit;

	CExtStatic m_ButtonGoNormalMode;
	CRect m_ButtonGoNormalModePos;
	bool m_bGoToNormalModeMsgSent;
};
