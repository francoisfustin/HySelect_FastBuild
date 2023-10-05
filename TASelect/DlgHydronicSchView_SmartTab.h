#pragma once


class CDlgHydronicSchView;
class CDlgHydronicSchView_SmartTab : public CDlgHydronicSchView_BaseTab
{
public:
	enum { IDD = IDD_DLGHYDRONICSCHVIEW_SMARTTAB };

	CDlgHydronicSchView_SmartTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent = NULL );
	virtual ~CDlgHydronicSchView_SmartTab() {}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDlgHydronicSchView_BaseTab' public virtual methods.
	virtual void SetAvailable( bool bAvailable );
	virtual void InitComboBox();
	//////////////////////////////////////////////////////////////////////////////////////////

	void FillComboBoxLocation();

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDlgHydronicSchView_BaseTab' public pure virtual methods.
	virtual bool IsAvailable( CDB_CircuitScheme *m_pCircuitScheme );
	virtual void GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme );

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

	afx_msg void OnCbnSelChangeLocation();

// Public variables.
private:
	CComboBox m_ComboLocation;
};

