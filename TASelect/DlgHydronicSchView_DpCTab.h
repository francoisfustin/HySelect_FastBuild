#pragma once


class CDlgHydronicSchView;
class CDlgHydronicSchView_DpCTab : public CDlgHydronicSchView_BaseTab
{
public:
	enum { IDD = IDD_DLGHYDRONICSCHVIEW_DPCTAB };

	CDlgHydronicSchView_DpCTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent = NULL );
	virtual ~CDlgHydronicSchView_DpCTab() {}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDlgHydronicSchView_BaseTab' public pure virtual methods.
	virtual bool IsAvailable( CDB_CircuitScheme *m_pCircuitScheme );
	virtual void GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme );
	//////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDlgHydronicSchView_BaseTab' public virtual methods.
	virtual void SetAvailable( bool bAvailable );
	virtual void InitComboBox();
	//////////////////////////////////////////////////////////////////////////////////////////

	void FillComboBoxMeasuringValvePosition();
	void FillComboBoxDpStabilizedOn();

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

	afx_msg void OnCbnSelChangeMeasuringValvePosition();
	afx_msg void OnCbnSelChangeDpStabilizedOn();

// Public variables.
public:
	CComboBox m_ComboMeasuringValvePosition;
	CComboBox m_ComboDpStabilizedOn;
};
