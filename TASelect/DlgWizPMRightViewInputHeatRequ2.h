#pragma once


#include "ExtStatic.h"
#include "ExtNumEdit.h"
#include "XGroupBox.h"
#include "DialogExt.h"
#include "ExtComboBox.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_HeatingRequirement2 : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_HEATREQU2 };

	CDlgWizardPM_RightViewInput_HeatingRequirement2( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizardPM_RightViewInput_Base' public virtual method.
	virtual void ApplyPMInputUserUpdated( bool bWaterCharUpdated = false, bool bShowErrorMsg = true );
	//////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnBnClickedWaterMakeUpBreakTank();
	virtual afx_msg void OnBnClickedWaterMakeUpDutyStby();

	virtual afx_msg void OnEnSetFocusMaxWidth();
	virtual afx_msg void OnEnSetFocusMaxHeight();

	virtual afx_msg void OnKillFocusMaxWidth();
	virtual afx_msg void OnKillFocusMaxHeight();

	// Overrides 'CDlgWizardPM_RightViewInput_Base' protected virtual methods.
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual void GetExtEditList( CArray<CExtNumEdit*> &arExtEditList );
	virtual void InitToolTips( CWnd* pWnd = NULL );

// Private methods.
private:
	void _VerifyInputValues( void );

// Private variables.
private:
	CXGroupBox m_GroupWaterMakeUp;
	CXGroupBox m_GroupSize;

	CExtStatic m_StaticWaterMakeUpBreakTank;
	CExtStatic m_StaticWaterMakeUpDutyStby;
	CExtStatic m_StaticSizeMaxWidth;
	CExtStatic m_StaticSizeMaxHeight;

	CButton m_clButtonWaterMakeUpBreakTank;
	CButton m_clButtonWaterMakeUpDutyStby;
	CExtNumEdit m_ExtEditSizeMaxWidth;
	CExtNumEdit m_ExtEditSizeMaxHeight;

	CExtStatic m_StaticSizeMaxWidthUnit;
	CExtStatic m_StaticSizeMaxHeightUnit;

	bool m_bButtonWaterMakeUpBreakTankState;
	bool m_bButtonWaterMakeUpDutyStby;
};

