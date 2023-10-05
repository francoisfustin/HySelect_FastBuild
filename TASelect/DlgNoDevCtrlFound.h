#pragma once


#include "ExtComboBox.h"

class CDlgNoDevCtrlFoundParams
{
// Public methods.
public:
	CDlgNoDevCtrlFoundParams();
	CDlgNoDevCtrlFoundParams( CRankEx *pList, CString strTypeID, CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID,
			CString strMaterialID, CString strConnectID, CString strVersionID, CString strPNID );

	virtual ~CDlgNoDevCtrlFoundParams() {}

	CDlgNoDevCtrlFoundParams &operator=( CDlgNoDevCtrlFoundParams &rNoDevCtrlFoundParams );

// Public variables.
public:
	CRankEx *m_pList;
	CString m_strTypeID;
	CDB_ControlProperties::CvCtrlType m_eCvCtrlType;
	CString m_strFamilyID;
	CString m_strMaterialID;
	CString m_strConnectID;
	CString m_strVersionID;
	CString m_strPNID;
};

class CIndividualSelectionParameters;
class CDlgNoDevCtrlFound : public CDialogEx
{
public:
	enum { IDD = IDD_DLGNODEVCTRLFOUND };

	CDlgNoDevCtrlFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevCtrlFoundParams *pclNoDevCtrlFoundParams, CWnd *pParent = NULL );
	virtual ~CDlgNoDevCtrlFound() {}

	CDlgNoDevCtrlFoundParams *GetNoDevCtrlFoundParams( void ) { return &m_clNoDevCtrlFoundParams; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	virtual afx_msg void OnCbnSelChangeType();
	virtual afx_msg void OnCbnSelChangeCtrlType();
	virtual afx_msg void OnCbnSelChangeFamily();
	virtual afx_msg void OnCbnSelChangeBodyMaterial();
	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangeVersion();
	virtual afx_msg void OnCbnSelChangePN();

// Private methods.
private:
	void _FillComboType( CString strTypeID = _T("") );
	void _FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvNU );
	void _FillComboFamily( CString strFamilyID = _T("") );
	void _FillComboBodyMat( CString strBodyMaterialID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );
	void _FillComboPN( CString strPNID = _T("") );

// Private variables.
private:
	CIndividualSelectionParameters *m_pclIndividualSelectionParams;
	CDlgNoDevCtrlFoundParams m_clNoDevCtrlFoundParams;

	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboCVCtrlType;
	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboBodyMaterial;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
	CExtNumEditComboBox m_ComboPN;
};
