#pragma once


class CDlgHydronicSchView;
class CDlgHydronicSchView_CVTab : public CDlgHydronicSchView_BaseTab
{
public:
	enum { IDD = IDD_DLGHYDRONICSCHVIEW_CVTAB };

	CDlgHydronicSchView_CVTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent = NULL );
	virtual ~CDlgHydronicSchView_CVTab() {}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDlgHydronicSchView_BaseTab' public pure virtual methods.
	virtual bool IsAvailable( CDB_CircuitScheme *m_pCircuitScheme );
	virtual void GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme );
	//////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDlgHydronicSchView_BaseTab' public virtual methods.
	virtual void VerifyControlValveType();
	virtual void SetAvailable( bool bAvailable );
	virtual void InitComboBox();
	//////////////////////////////////////////////////////////////////////////////////////////

	void FillComboBoxBVIn3WBypass();

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDlgHydronicSchView_BaseTab' public pure virtual methods.
	virtual void FillComboBoxType();
	virtual void FillComboBoxFamily();
	virtual void FillComboBoxBodyMaterial();
	virtual void FillComboBoxConnect();
	virtual void FillComboBoxVersion();
	virtual void FillComboBoxPN();
	virtual void FillComboBoxName();
	//////////////////////////////////////////////////////////////////////////////////////////

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );

	afx_msg void OnCbnSelChangeBVIn3wBypass();

// Public variables.
private:
	CComboBox m_ComboBVIn3WBypass;
};

