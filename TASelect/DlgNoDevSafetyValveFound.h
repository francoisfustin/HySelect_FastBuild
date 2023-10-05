#pragma once


#include "ExtComboBox.h"
#include "XGroupBox.h"

class CDlgNoDevSafetyValveFoundParams
{
// Public methods.
public:
	CDlgNoDevSafetyValveFoundParams();
	CDlgNoDevSafetyValveFoundParams( CRankEx *pList, ProjectType eSystemType, CString strSystemHeatGeneratorTypeID, CString strNormID, CString strSafetyValveFamilyID, 
			CString strSafetyValveConnectID, double dSafetyValveSetPressure );

	virtual ~CDlgNoDevSafetyValveFoundParams() {}

	CDlgNoDevSafetyValveFoundParams &operator=( CDlgNoDevSafetyValveFoundParams &rNoDevSeparatorFoundParams );

// Public variables.
public:
	CRankEx *m_pList;
	ProjectType m_eSystemType;
	CString m_strSystemHeatGeneratorTypeID;
	CString m_strNormID;
	CString m_strSafetyValveFamilyID;
	CString m_strSafetyValveConnectID;
	double m_dSafetyValveSetPressure;
};

class CDlgNoDevSafetyValveFound : public CDialogEx
{
public:
	enum { IDD = IDD_DLGNODEVSAFETYVALVEFOUND };

	CDlgNoDevSafetyValveFound( CProductSelelectionParameters *pclProductSelParams, CDlgNoDevSafetyValveFoundParams *pclNoDevSafetyValveFoundParams, 
			CWnd *pParent = NULL );

	virtual ~CDlgNoDevSafetyValveFound() {}

	// Opens the dialog window.
	CDlgNoDevSafetyValveFoundParams *GetNoDevSafetyValveFoundParams( void ) { return &m_clNoDevSafetyValveFoundParams; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	
	virtual afx_msg void OnCbnSelChangeSystemType();
	virtual afx_msg void OnCbnSelChangeSystemHeatGeneratorTypeID();
	virtual afx_msg void OnCbnSelChangeNormID();
	virtual afx_msg void OnCbnSelChangeSafetyValveFamilyID();
	virtual afx_msg void OnCbnSelChangeSafetyValveConnectID();
	virtual afx_msg void OnCbnSelChangeSafetyValveSetPressure();

// Private methods.
private:
	void _FillComboSystemType( ProjectType eApplicationType = ProjectType::InvalidProjectType );
	void _FillComboSystemHeatGeneratorTypeID( CString strSystemHeatGeneratorTypeID = _T("") );
	void _FillComboNormID( CString strNormID = _T( "" ) );
	void _FillComboSafetyValveFamilyID( CString strSafetyValveFamilyID = _T("") );
	void _FillComboSafetyValveConnectID( CString strSafetyValveConnectID = _T("") );
	void _FillComboSafetyValveSetPressure( double dSetPressure = 0.0 );

	void _UpdateLayout();
	ProjectType _GetSytemType();
	CString _GetNormID();
	double _GetSafetyValveSetPressure();

// Private variables.
private:
	CProductSelelectionParameters *m_pclProductSelParams;
	CDlgNoDevSafetyValveFoundParams m_clNoDevSafetyValveFoundParams;

	CXGroupBox m_clGroupSystem;
	CXGroupBox m_clGroupNorm;
	CXGroupBox m_clGroupSafetyValve;

	CExtNumEditComboBox m_ComboSystemType;
	CExtNumEditComboBox m_ComboSystemHeatGeneratorType;
	CExtNumEditComboBox m_ComboNorm;
	CExtNumEditComboBox m_ComboSafetyValveFamily;
	CExtNumEditComboBox m_ComboSafetyValveConnect;
	CExtNumEditComboBox m_ComboSafetyValveSetPressure;
};
