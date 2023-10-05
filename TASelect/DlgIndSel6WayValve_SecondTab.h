#pragma once


#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "afxwin.h"
#include "DlgIndSelCtrlBase.h"

class ISecondTab_NotificationHandler
{
public:
	virtual void IPIBCvBvNotification_EnableSelectButton( bool bEnable = true ) = 0;
	virtual void IPIBCvBvNotification_OnEditEnterChar( NMHDR *pNMHDR ) = 0;
	virtual void IPIBCvBvNotification_ClearAll() = 0;
	
	// When actuator group is hidden or shown the group Bv must be resized.
	virtual void IPIBCvBvNotification_ResizeSecondTab( long &offset, bool bReduce = true ) = 0;
	virtual void IPIBCvBvNotification_SetFocus() = 0;
	virtual void IPIBCvBvNotification_SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere ePosition ) = 0;
	virtual bool IPIBCvBvNotification_IsPIBCVGroupResized() = 0;
};

class CDlgIndSel6WayValve_SecondTab : public CDlgIndSelCtrlBase
{
public:
	enum { IDD = IDD_DLGINDSEL6WAYVALVE_SECONDTAB };
	
	CDlgIndSel6WayValve_SecondTab( CIndSel6WayValveParams &clIndSel6WayValveParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSel6WayValve_SecondTab();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();
	// Called by 'CDlgLeftTabSelManager' when user change application type.
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual int VerifyDpValue( void );

	enum UpdateFrom
	{
		UF_NewDocument,
		UF_SelectionModeChanged,
		UF_ProductEdited,
		UF_FastEelectricalConnectionChanged,
		UF_Other
	};
	void Update6WayTabCombo( UpdateFrom eUpdateFrom );
	
	void SetGroupDpMaxCheckBox( int iCheck ) { m_GroupDpMax.SetCheck( iCheck ); }
	
	// Called when we need to update the state of the 'Dp max' edit field and unit.
	void UpdateDpMaxFieldState();
	
	// Notification.
	void SetNotificationHandler( ISecondTab_NotificationHandler *pINotificationHandler );
	void ResetNotificationHandler( void );
	
	// In Edition mode this function get all PIBCV or BV parameters values saved from Datastruct.
	void SetSecondTabParameters( CDS_SSel6WayValve *pclCurrent6WayValve );
	
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
	
	// for PIBCv.
	afx_msg void OnBnClickedCheckGroupDpMax();
	afx_msg void OnEnChangeDpMax();
	afx_msg void OnEnKillFocusDpMax();
	
	// For BV.
	afx_msg void OnBnClickedCheckDp();

	afx_msg void OnCbnSelChangeType();
	afx_msg void OnCbnSelChangeCtrlType();
	afx_msg void OnCbnSelChangeFamily();
	afx_msg void OnCbnSelChangeBodyMaterial();
	afx_msg void OnCbnSelChangeConnect();
	afx_msg void OnCbnSelChangeVersion();
	afx_msg void OnCbnSelChangePN();
	afx_msg void OnCbnSelChangeInputSignal();

	virtual void FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvProportional );

	// Notifications
	virtual void EnableSelectButtonNotification( bool bEnable = true );
	virtual void ClearAllNotification();

	// When actuator group is hidden or shown the group Bv must be resized. This function send the notification  to CDlgIndSel6Way.
	virtual void ResizeSecondTabNotification( long &lOffset, bool bReduce = true );
	virtual void SetFocusNotification();
	virtual void SetFocusOnControlNotification( CDlgSelectionTabHelper::SetFocusWhere ePosition );
	virtual bool IsPIBCVGroupResized();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected methods.
	virtual CExtNumEdit *GetpEditDp() const { return ( (CExtNumEdit *)GetDlgItem( IDC_EDITDP ) ); }
	virtual afx_msg void OnEditEnterChar( NMHDR* pNMHDR, LRESULT* pResult );
	virtual afx_msg void OnEnChangeDp();
	virtual afx_msg void OnEnKillFocusDp();
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void UpdateDpFieldState();

// Private methods.
private:
	void _FillComboType( CString strTypeID = _T("") );
	void _FillComboFamily( CString strFamilyID = _T("") );
	void _FillComboBodyMat( CString strBodyMaterialID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );
	void _FillComboPN( CString strPNID = _T("") );
	void _FillComboInputSignal( CString strInputSignalID = _T("") );

	// This function move up or down all objects under valve type group when it is resized.
	void _VerticalMoveForBvDisplay( long &lOffset, bool bUp = true );

	void _SetDpMax( double dDpMax );

// Private variables.
private:
	CIndSel6WayValveParams *m_pclIndSel6WayValveParams;

	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboCtrlType;
	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboMaterial;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
	CExtNumEditComboBox m_ComboPN;
	CExtNumEditComboBox m_ComboInputSignal;
	CXGroupBox m_GroupValveType;
	CRect m_RCGroupValveType;
	CXGroupBox m_GroupValve;
	CXGroupBox m_GroupActuator;
	CXGroupBox m_GroupDpMax;
	CExtNumEdit m_clExtEditDpMax;
	ISecondTab_NotificationHandler *m_pNotificationHandler;
};

