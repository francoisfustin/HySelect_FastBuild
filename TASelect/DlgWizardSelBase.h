#pragma once


#include "RViewWizardSelBase.h"
#include "DlgSelectionBase.h"
#include "ProductSelectionParameters.h"

class CMainFrame;
class CDlgWizardSelBase : public CDlgSelectionBase
{
public:
	CDlgWizardSelBase( CWizardSelectionParameters &clWizardSelParams, UINT nID = 0, CWnd *pParent = NULL );

	virtual ~CDlgWizardSelBase() {}

	void VerifyModificationMode( void );
	void SetModificationMode( bool bFlag = true );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual	void SaveSelectionParameters( void );
	virtual void ActivateLeftTabDialog( void );
	virtual void ClearAll( void );

	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg void OnDestroy() {}
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual CRViewWizardSelBase *GetLinkedRightViewWizardSel( void ) { return NULL; }

// Protected variables.
protected:
	CWizardSelectionParameters *m_pclWizardSelParams;
};
