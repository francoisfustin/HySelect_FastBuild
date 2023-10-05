#pragma once


class CDlgHydronicSchView;
class CDlgHydronicSchView_BVTab : public CDlgHydronicSchView_BaseTab
{
public:
	enum { IDD = IDD_DLGHYDRONICSCHVIEW_BVTAB };

	CDlgHydronicSchView_BVTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent = NULL );
	virtual ~CDlgHydronicSchView_BVTab() {}

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
};

