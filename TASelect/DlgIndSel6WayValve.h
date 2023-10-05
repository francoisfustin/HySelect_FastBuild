#pragma once

#include "ExtComboBox.h"
#include "DialogExt.h"
#include "XGroupBox.h"
#include "afxwin.h"
#include "DlgSelectionBase.h"
#include "DlgIndSel6WayValve_6WayValveTab.h"
#include "DlgIndSel6WayValve_SecondTab.h"


class CDlgIndSel6WayValve_Tabs : public CMFCTabCtrl
{
public:

	// Overrides some 'CMFCBaseTabCtrl' to avoid drag to be moved.
	virtual DROPEFFECT OnDragEnter( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual DROPEFFECT OnDragOver( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual void SwapTabs( int nFisrtTabID, int nSecondTabID ) {}
	virtual void MoveTab( int nSource, int nDest ) {}
	// To avoid that the bottom line of tab group disappears
	virtual BOOL SetActiveTab( int iTab );
};

class CDlgIndSel6WayValve : public CDlgIndSelCtrlBase, public I6WayValveTab_NotificationHandler, public ISecondTab_NotificationHandler
{

public:
	enum { IDD = IDD_DLGINDSEL6WAYVALVE };

	CDlgIndSel6WayValve( CWnd *pParent = NULL );
	virtual ~CDlgIndSel6WayValve();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DialogExt' public virtual methods.
	virtual void SetApplicationBackground( COLORREF cBackColor );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_6W ); }
	// To update all dialog contains during the updateWindow from base class
	virtual void UpdateWindow();
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UpdateTabTitle( int iTabID, e6WayValveSelectionMode eCircuit );

	// Remark: This method is called by the 'CDlgCOWaterChar' dialog to know it it can validate the fluid characteristic that
	// user has defined.
	virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgCOWaterChar *pclDlgWaterChar );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionTabHelper' public virtual methods.
	// This method is called when left tab dialog has no more the focus while user presses TAB or SHIFT+TAB.
	// Param: if 'fNext' is 'true' that means user presses 'TAB', otherwise it's 'SHIFT+TAB'.
	// Remark: for the pressure maintenance panel, it's a particular case. We have the 'CDlgIndSelPM' inherited directly from 'CDlgSelectionBase'.
	//         In this dialog, we have just a few controls common to all application types (heating, cooling and solar). Below these controls, there
	//         is one other dialog (CDlgIndSelPMPanels), one dialog for one application type. 
	//         At start, the focus is set on the control in the 'CDlgIndSelPM'. If user presses and reaches the last control, we don't want to go
	//         in the right view but first in the 'CDlgIndSelPMPanels'. When last control is reached in this dialog while pressing TAB key, then we can go
	//         to the right view.

	virtual bool LeftTabKillFocus( bool bNext );
	// Overrided to go to the first field in 6-way valve left tab instead of the flow field
	virtual void OnRViewSSelLostFocusWithTabKey( bool bShiftPressed );

	// End of overriding 'DlgSelectionTabHelper' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected members.
protected:

	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedSelectionMode();
	afx_msg void OnBnClickedFastConnection();
	// HYS-1877: Add a checkbox for set
	afx_msg void OnBnClickedOnlyForSet();
	afx_msg void OnButtonModWater();

	// This is the way to catch notifications from DlgIndSel6WayValve_6WayValveTab and DlgIndSel6WayValve_SecondTab
	virtual void I6WayNotification_EnableSelectButton( bool bEnable = true );
	virtual void I6WayNotification_OnEditEnterChar( NMHDR *pNMHDR );
	virtual void I6WayNotification_ClearAll();
	virtual void I6WayNotification_SetFocus();
	virtual void I6WayNotification_SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere ePosition );
	virtual void IPIBCvBvNotification_EnableSelectButton( bool bEnable = true );
	virtual void IPIBCvBvNotification_OnEditEnterChar( NMHDR *pNMHDR );
	virtual void IPIBCvBvNotification_ClearAll();
	
	// Allow to change size of Bv group when changing from pibcv to bv or the opposite
	virtual void IPIBCvBvNotification_ResizeSecondTab( long &lOffset, bool bReduce = true );
	virtual void IPIBCvBvNotification_SetFocus();
	virtual void IPIBCvBvNotification_SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere ePosition );
	virtual bool IPIBCvBvNotification_IsPIBCVGroupResized();
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnModifySelected6Way( WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual CButton *GetpBtnSuggest() const { return ( (CButton *)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnCancelModif() const { return ( (CButton *)GetDlgItem( IDC_BUTTONCANCEL ) ); }
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedSelect();
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CString Get6WayValveSelectionModeStr( e6WayValveSelectionMode eCircuitType );

// Private methods.
private:

	void _Update6WayValveWaterChar();
	void _SetStaticFluidTemp();
	void _UpdateCheckFastElectConnection();
	// HYS-1877: Add a checkbox for set
	void _UpdateCheckOnlyForSet();
	// Launch a suggestion procedure, called from the 'No device found' dialog box.
	void _LaunchSuggestion( CString strConnectID, CString strVersionID );

	// If the selection made by the user doesn't work, try to make an alternative selection.
	// The function return true if the user accept to change the criterion.
	bool _TryAlternativeSuggestion( bool bSizeShiftProblem );
	
	// HYS-1429 : This function check the product availability to avoid to enable selection mode
	// which have not a solution.
	// HYS-1877: Add a checkbox for set
	void _CheckProductAvailability( bool &bCanEQMEnabled, bool &bCanOnoffPibcvEnabled, bool &bCanOnoffBvEnabled, 
		bool &bCan6WayValveEnabled, bool &bCanPackageEnable );

	// Verify if we can enable/disable this button and if we can check/uncheck it.
	//void _VerifyCheckboxCircuitStatus();

	// Private variables.
private:
	CIndSel6WayValveParams m_clIndSel6WayValveParams;
	CExtStatic m_StaticSelectionMode;
	CXGroupBox m_Group6WayValveSelection;
	CButton m_BnSelectionMode;
	CButton m_CheckFastConnection;
	// HYS-1877: Add a checkbox for set
	CButton m_CheckOnlyForSet;
	CExtStatic m_StaticWater;
	CExtStatic m_StaticHeating;
	CExtStatic m_StaticCooling;
	CXGroupBox m_GroupWater;
	CMFCButton m_ButtonModWater;
	CRect m_RCGroupPIBCV;
	typedef enum TabIndex
	{
		eti6Way = 0,
		etiSecondaryValve
	};

	CDlgIndSel6WayValve_Tabs m_cl6WayValveCtrlTab;
	CDlgIndSel6WayValve_6WayValveTab *m_pclDlgIndSel6WayValve_6WayValveTab;
	CDlgIndSel6WayValve_SecondTab *m_pclDlgIndSel6WayValve_SecondTab;
};


