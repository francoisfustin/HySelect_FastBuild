#pragma once
#include "DialogExt.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelCtrlBase.h"
#include "ProductSelectionParameters.h"

class I6WayValveTab_NotificationHandler
{
	public:
		virtual void I6WayNotification_EnableSelectButton( bool bEnable = true ) = 0;
		virtual void I6WayNotification_ClearAll() = 0;
		virtual void I6WayNotification_OnEditEnterChar( NMHDR *pNMHDR ) = 0;
		virtual void I6WayNotification_SetFocus() = 0;
		virtual void I6WayNotification_SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere ePosition ) = 0;
};

class CDlgIndSel6WayValve_6WayValveTab : public CDlgIndSelCtrlBase
{
public:
	enum { IDD = IDD_DLGINDSEL6WAYVALVE_6WAYVALVETAB };

	CDlgIndSel6WayValve_6WayValveTab( CIndSel6WayValveParams &clIndSel6WayValveParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSel6WayValve_6WayValveTab();

	virtual void SaveSelectionParameters();

	// Notification
	void SetNotificationHandler( I6WayValveTab_NotificationHandler *pINotificationHandler );
	void ResetNotificationHandler( void );

	void FillComboConnection( CString strConnectID = _T("") );
	void FillComboVersion( CString strVersionID = _T("") );
	
	// In Edition mode this function get all 6-way valve parameters values saved from Datastruct.
	void Set6WayValveTabParameters( CDS_SSel6WayValve *pclCurrent6WayValve );
	
	// Update PDT DT field.
	void Update6WayValveTabDTField();

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

	virtual bool LeftTabKillFocus( bool fNext );
	// Overrided to go to the first field in 6-way valve left tab instead of the flow field
	virtual void OnRViewSSelLostFocusWithTabKey( bool bShiftPressed );
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );

	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnBnClickedRadioFlowOrPowerDT();
	virtual afx_msg void OnComboSelChangeConnect();
	virtual afx_msg void OnComboSelChangeVersion();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected methods.
	virtual CButton *GetpRadioFlow() const { return ( (CButton *)GetDlgItem( IDC_RADIOFLOW ) ); }
	virtual CButton *GetpRadioPower() const { return ( (CButton *)GetDlgItem( IDC_RADIOPOWER ) ); }
	virtual CStatic *GetpStaticDT() const { return ( (CStatic *)GetDlgItem( IDC_STATICDT_H ) ); }
	virtual CStatic *GetpStaticQUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICUNITFLOW_H ) ); }
	virtual CStatic *GetpStaticPUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICUNITPOWER_H ) ); }
	virtual CStatic *GetpStaticDTUnit() const { return ( (CStatic *)GetDlgItem( IDC_STATICUNITDT_H ) ); }

	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected methods.

	afx_msg void OnEnKillFocusFlowCooling();
	afx_msg void OnEnKillFocusPowerCooling();
	afx_msg void OnEnKillFocusDTCooling();
	afx_msg void OnEnKillFocusFlowHeating();
	afx_msg void OnEnKillFocusPowerHeating();
	afx_msg void OnEnKillFocusDTHeating();
	afx_msg void OnEnChangeFlowCooling();
	afx_msg void OnEnChangePowerCooling();
	afx_msg void OnEnChangeDTCooling();
	afx_msg void OnEnChangeFlowHeating();
	afx_msg void OnEnChangePowerHeating();
	afx_msg void OnEnChangeDTHeating();
	virtual afx_msg void OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult );
	// For notifications.
	virtual void EnableSelectButtonNotification( bool bEnable = true );
	virtual void OnEditEnterCharNotification( NMHDR *pNMHDR );
	virtual void ClearAllNotification();
	virtual void SetFocusNotification();
	virtual void SetFocusOnControlNotification( CDlgSelectionTabHelper::SetFocusWhere ePosition );

	void Update6WayFlowOrPowerDTState();
	void InitNumericalEdit( CExtNumEdit *pEdit, ePHYSTYPE phystype );

// Protected variables.
protected:
	CIndSel6WayValveParams *m_pclIndSel6WayValveParams;
	CXGroupBox m_clGroupHeating;
	CXGroupBox m_clGroupCooling;

	// Group heating.
	CExtNumEdit m_EditFlowHeating;
	CExtNumEdit m_EditPowerHeating;
	CExtNumEdit m_EditDTHeating;

	// Group cooling.
	CExtNumEdit m_EditFlowCooling;
	CExtNumEdit m_EditPowerCooling;
	CExtNumEdit m_EditDTCooling;

	bool m_bIsDTCoolingEdited;
	bool m_bIsDTHeatingEdited;
	double m_dPreviousCoolingDT;
	double m_dPreviousHeatingDT;

	// Valves .
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;

	I6WayValveTab_NotificationHandler *m_pNotificationHandler;

// Private methods.
private:
	void _UpdateFlowCooling();
	void _UpdateFlowHeating();
};

