#pragma once


#include "afxcmn.h"
#include "XGroupBox.h"
#include "ExtButton.h"
#include "ExtComboBox.h"
#include "ExtNumEdit.h"

class CDlgIndSelSafetyValve : public CDlgIndSelBase
{
public:
	enum { IDD = IDD_DLGINDSELSAFETYVALVE };

	CDlgIndSelSafetyValve( CWnd *pParent = NULL );
	virtual ~CDlgIndSelSafetyValve() {}

	ProjectType GetSytemType();
	CString GetNormID();
	double GetSafetyValveSetPressure();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	virtual void SaveSelectionParameters();
	virtual void ActivateLeftTabDialog();

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	virtual CString GetDlgCaption(){ return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_SAFETYVALVE ); }

	// Remark: This method is called by the 'CDlgWaterChar' dialog to know it it can validate the fluid characteristic that
	// user has defined.
	virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar );
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnCbnSelChangeSystemType();
	virtual afx_msg void OnCbnSelChangeSystemHeatGeneratorTypeID();
	virtual afx_msg void OnCbnSelChangeNormID();
	virtual afx_msg void OnCbnSelChangeSafetyValveFamilyID();
	virtual afx_msg void OnCbnSelChangeSafetyValveConnectID();
	virtual afx_msg void OnCbnSelChangeSafetyValveSetPressure();
	virtual afx_msg LRESULT OnModifySelectedSafetyValve( WPARAM wParam, LPARAM lParam );

	virtual afx_msg void OnEnSetFocusSystemPower();
	virtual afx_msg void OnEnSetFocusSystemCollector();

	virtual afx_msg void OnKillFocusSystemPower();
	virtual afx_msg void OnKillFocusSystemCollector();

	virtual afx_msg void OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedSelect();
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );	
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected methods.
	virtual CButton *GetpBtnSuggest() const { return ( (CButton*)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnCancelModif() const { return ( (CButton*)GetDlgItem( IDC_BUTTONCANCEL ) ); }
	// End of overriding 'CDlgIndSelBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected pure virtual methods.
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgIndSelBase' protected pure virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _ChangeApplicationType();
	void _UpdateLayout();

	bool _SuggestIndividualSelection();
	bool _SelectIndividualSelection();
	
	void _FillComboSystemType( ProjectType eApplicationType = InvalidProjectType );
	void _FillComboSystemHeatGeneratorType( CString strSystemHeatGeneratorTypeID = _T("") );
	void _FillComboNormID( CString strNormID = _T( "" ) );
	void _FillComboSafetyValveFamily( CString strSafetyValveFamilyID = _T("") );
	void _FillComboSafetyValveConnect( CString strSafetyValveConnectID = _T("") );
	void _FillComboSafetyValveSetPressure( double dSetPressure = 0.0 );

	// Launch a suggestion procedure, called from the 'No device found' dialog box.
	void _LaunchSuggestion( ProjectType eSystemType, CString strSystemHeatGeneratorTypeID, CString strSafetyValveTypeID, CString strSafetyValveConnectID, double dSafetyValveSetPressure );

	// Update member variables with the selected data.
	// Return 0 if one (or several) data is (are) invalid.
	bool _VerifySubmittedValues();

	void _VerifyFluidCharacteristics( bool bShowErrorMsg );
	void _VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar = NULL );

// Private variables.
private:
	CIndSelSafetyValveParams m_clIndSelSafetyValveParams;

	CExtNumEditComboBox m_ComboSystemType;
	CExtNumEditComboBox m_ComboSystemHeatGeneratorType;
	CExtNumEditComboBox m_ComboNorm;
	CExtNumEditComboBox m_ComboSafetyValveFamily;
	CExtNumEditComboBox m_ComboSafetyValveConnect;
	CComboBox m_ComboSafetyValveSetPressure;

	CExtNumEdit m_clExtEditInstalledPower;
	CExtNumEdit m_clExtEditInstalledCollector;
	
	CXGroupBox m_clGroupSystem;
	CXGroupBox m_clGroupNorm;
	CXGroupBox m_clGroupInstalledPower;
	CXGroupBox m_clGroupSafetyValve;
	
	CExtButton m_clButtonSuggest;

	double m_dInstalledPowerSaved;
	double m_dInstalledCollectorSaved;

	class CParamsSaved
	{
	public:
		CParamsSaved( ProjectType eSystemType, CString strSystemHeatGeneratorTypeID, CString strNormID, CString m_strSafetyValveFamilyID, CString strSafetyValveConnectID,
			double dUserSetPressureChoice )
		{
			m_eSystemType = eSystemType;
			m_strSystemHeatGeneratorTypeID = strSystemHeatGeneratorTypeID;
			m_strNormID = strNormID;
			m_strSafetyValveFamilyID = m_strSafetyValveFamilyID;
			m_strSafetyValveConnectID = strSafetyValveConnectID;
			m_dUserSetPressureChoice = dUserSetPressureChoice;
		}

	public:
		ProjectType m_eSystemType;
		CString m_strSystemHeatGeneratorTypeID;
		CString m_strNormID;
		CString m_strSafetyValveFamilyID;
		CString m_strSafetyValveConnectID;
		double m_dUserSetPressureChoice;
	};
};
