#pragma once


#include "afxcmn.h"
#include "afxwin.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "HMInclude.h"

class CHubHandler;
class CDlgIndSelHub : public CDlgIndSelBase
{
public:
	enum { IDD = IDD_DLGINDSELHUB };
	CDlgIndSelHub( CWnd* pParent = NULL );   // standard constructor
	virtual ~CDlgIndSelHub();

	// Refresh the Number of stations displayed into the combo according to the Hub displayed into the right page
	// Number of stations can be modified by Delete, Paste functionalities
	void RefreshNumberOfStations();	
	
	// When Application is not chosen all must be lock
	bool IsLocked() { return m_bLocked; }
	
	// Enable or Disable Select button
	void CheckSelectButton( CDS_HmHub *pHmHub = NULL );
	
	// Fill Combo MvLoc according to the measuring valve position
	void FillComboMvLoc();

	// Update the data in the right part
	void UpdateValues( bool fNew = false );
	
	bool IsNewDoc() { return m_fNewDoc; }
	void SetNewDoc(bool fNewDoc ) { m_fNewDoc = fNewDoc; }

// Protected members
protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	// By default, set the focus on the Suggest button
	virtual void OnDisplay(){ GetDlgItem( IDC_BUTTONNEW )->SetFocus(); }

	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonmodwater();
	afx_msg void OnButtonselect();
	afx_msg void OnBnClickedRadioMainSupplyReturnPipe();
	afx_msg void OnBnClickedRadioSelectionMode();
	afx_msg void OnBnClickedButtonnew();
	afx_msg void OnCbnSelchangeCombopartnertype();
	afx_msg void OnCbnSelchangeCombooptions();
	afx_msg void OnCbnSelchangeCombonumberofstations();						
	afx_msg void OnCbnSelchangeComboapplication();
	afx_msg void OnCbnSelchangeCombopartnerconnect();
	afx_msg void OnCbnSelchangeCombostationconnect();
	afx_msg void OnCbnSelchangeCombomvloc();

	afx_msg LRESULT OnChangefocusRViewssel( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnModifySelectedHub( WPARAM wParam, LPARAM lParam );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FillComboApplication();
	void FillComboConnect();
	void FillComboExPartner();			// Procedure to fill the combobox for the partner type
	void FillComboNumberOfStation();

	void ResetToDefault();

// Protected variables
protected:
	CIndSelHUBParams m_clIndSelHUBParams;

	CButton			m_radioFlow;				// Radio button for the flow
	CButton			m_radioFlowLoopDP;			// Radio button for the flow and loop DP 
	CButton			m_radioDirectSelection;		// Radio button for the direct selection.
	CButton			m_radioFlowLoopDetails;		// Radio button for the flow and loop details
	CButton			m_radioLeft;				// Radio button for "on the left"
	CButton			m_radioRight;				// Radio button for "on the right"
	CButton			m_ButtonSelect;
	CXGroupBox		m_GroupTAHub;
	CXGroupBox		m_GroupSelectionMode;
	CExtNumEditComboBox	m_ComboAppli;
	CExtNumEditComboBox	m_ComboStationConnect;
	CExtNumEditComboBox	m_ComboMvLoc;
	CExtNumEditComboBox	m_ComboPartnerType;			// Comboxbox for the partner type of TA Hub
	CExtNumEditComboBox	m_ComboNumberOfStation;		// number of station (linked with the combo box)

	CFont m_font;
	CToolTipCtrl m_TTSelectBut;
	bool m_bLocked;
	bool m_fNewDoc;

	CHubHandler	*m_pHubHandler;
};

extern CDlgIndSelHub *pDlgIndSelHub;
