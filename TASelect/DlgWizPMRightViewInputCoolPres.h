#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "ExtComboBox.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_CoolingPressure : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_COOLPRES };

	CDlgWizardPM_RightViewInput_CoolingPressure( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

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

	virtual afx_msg void OnEnSetFocusStaticHeight();
	virtual afx_msg void OnEnSetFocusPz();
	virtual afx_msg void OnEnSetFocusSafetyVRP();

	virtual afx_msg void OnEnSetFocuspSVLocation();
	virtual afx_msg void OnEnChangeStaticHeight();

	virtual afx_msg void OnKillFocusStaticHeight();
	virtual afx_msg void OnKillFocusPz();
	virtual afx_msg void OnKillFocusSafetyVRP();
	virtual afx_msg void OnKillFocuspSVLocation();

	virtual afx_msg void OnBnClickedCheckPz();
	virtual afx_msg void OnBnClickedCheckpSVLocation();
	
	void UpdateP0();

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
	CButton m_BtnCheckPz;
	CButton m_BtnCheckpSVLocation;		// HYS-1083

	CExtStatic m_StaticStaticHeight;
	CExtStatic m_StaticP0;
	CExtStatic m_StaticPz;
	CExtStatic m_StaticSafetyVRP;
	CExtStatic m_StaticSafetyVRPMinimum;
	CExtStatic m_StaticpSVLocation;		// HYS-1083

	CExtNumEdit m_ExtEditStaticHeight;
	CExtNumEdit m_ExtEditPz;
	CExtNumEdit m_ExtEditSafetyVRP;
	CExtNumEdit m_EditpSVLocation;			// HYS-1083.

	CExtStatic m_StaticStaticHeightUnit;
	CExtStatic m_StaticPzUnit;
	CExtStatic m_StaticSafetyVRPUnit;
	CExtStatic m_StaticSVLocationUnit;
	
	CStatic m_ButtonGoAdvancedMode;
	CRect m_ButtonGoAdanvedModePos;

	bool m_bGoToAdvModeMsgSent;
};
