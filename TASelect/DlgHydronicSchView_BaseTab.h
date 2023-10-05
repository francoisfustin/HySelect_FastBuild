#pragma once


#define _Abv50			_T("65 and above")
#define _AllSizes		_T("All sizes")
#define _Blw65			_T("50 and below")
#define _No				_T("No")
#define _Yes			_T("Yes")
#define _None			_T("None")
#define _Proportional	_T("Proportional")
#define _OnOff			_T("On-Off")
#define _3points		_T("3-points")
#define _NoControl		_T("No control")
#define _ControlOnly	_T("Standard control valve")
#define _Adjustable		_T("Adjustable")
#define _AdjustableMeas	_T("Adjustable and measurable")
#define _PressIndep		_T("Pressure independent")
#define _Smart			_T("Smart")
#define _Primary		_T("Primary")
#define _Secondary		_T("Secondary")
#define _Branch			_T("Branch")
#define _Cv				_T("Control valve")

class CDlgHydronicSchView;
class CDlgHydronicSchView_BaseTab : public CDialogEx
{
public:
	CDlgHydronicSchView_BaseTab( CDlgHydronicSchView *pclHydronicSchView, UINT uiIDD, CWnd *pParent = NULL );
	virtual ~CDlgHydronicSchView_BaseTab() {}

	virtual bool IsAvailable( CDB_CircuitScheme *m_pCircuitScheme ) = 0;
	virtual void GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme ) = 0;

	virtual void VerifyControlValveType() {}
	virtual void SetAvailable( bool bAvailable );
	virtual void InitComboBox();
	virtual void ResetComboBoxStr();

	virtual void FillComboBoxType() = 0;
	virtual void FillComboBoxFamily() = 0;
	virtual void FillComboBoxBodyMaterial() = 0;
	virtual void FillComboBoxConnect() = 0;
	virtual void FillComboBoxVersion() = 0;
	virtual void FillComboBoxPN() = 0;
	virtual void FillComboBoxName() = 0;

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );

	afx_msg void OnCbnSelChangeProductSize();

	virtual afx_msg void OnCbnSelChangeType();
	virtual afx_msg void OnCbnSelChangeFamily();
	virtual afx_msg void OnCbnSelChangeBodyMaterial();
	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangeVersion();
	virtual afx_msg void OnCbnSelChangePN();
	virtual afx_msg void OnCbnSelChangeComboName();

// Protected members.
protected:
	void FillComboBoxProductSize();
	void FillComboBox( CRankEx *pListEx, CComboBox *pCCombo, int iCount, _string *pCstr = NULL );
	void FillComboBoxTAProduct( CRankEx *pListEx, CComboBox *pCCombo, int iCount );

// Public variables.
public:
	CDlgHydronicSchView *m_pclHydronicSchView;
	CTADatabase *m_pTADB;

	int m_iDNMin;
	int m_iDNMax;

	eMvLoc m_eMvLoc;
	eDpStab m_eDpStab;
	bool m_bBvIn3WBypass;
	SmartValveLocalization m_eSmartValveLocation;
	bool m_bSelectedBySet;

	CComboBox m_ComboProductSize;
	CComboBox m_ComboType;
	CComboBox m_ComboFamily;
	CComboBox m_ComboBodyMaterial;
	CComboBox m_ComboConnect;
	CComboBox m_ComboVersion;
	CComboBox m_ComboPN;
	CComboBox m_ComboName;

	_string m_strType;
	_string m_strFamily;
	_string m_strBodyMaterial;
	_string m_strConnect;
	_string m_strVersion;
	_string m_strPN;

	CDB_TAProduct *m_pProduct;
};

