#pragma once


#include "ExtComboBox.h"

class CDlgNoDevFoundParams
{
// Public methods.
public:
	CDlgNoDevFoundParams();
	CDlgNoDevFoundParams( CRankEx *pList, CString strTypeID, CString strFamilyID, CString strMaterialID, CString strConnectID, 
			CString strVersionID, CString strPNID );

	virtual ~CDlgNoDevFoundParams() {}

	CDlgNoDevFoundParams &operator=( CDlgNoDevFoundParams &pclNoDevSeparatorFoundParams );

// Public variables.
public:
	CRankEx *m_pList;
	CString m_strTypeID;
	CString m_strFamilyID;
	CString m_strMaterialID;
	CString m_strConnectID;
	CString m_strVersionID;
	CString m_strPNID;
};

class CIndividualSelectionParameters;
class CDlgNoDevFound : public CDialogEx
{
public:
	enum { IDD = IDD_DLGNODEVFOUND };

	CDlgNoDevFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevFoundParams *pclNoDevFoundParams, CWnd *pParent = NULL );
	virtual ~CDlgNoDevFound() {}

	CDlgNoDevFoundParams *GetNoDevFoundParams( void ) { return &m_clNoDevFoundParams; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	
	virtual afx_msg void OnCbnSelChangeType();
	virtual afx_msg void OnCbnSelChangeFamily();
	virtual afx_msg void OnCbnSelChangeMaterial();
	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangeVersion();
	virtual afx_msg void OnCbnSelChangePN();

// Private methods.
private:
	void _FillComboType( CString strTypeID = _T("") );
	void _FillComboFamily( CString strFamilyID = _T("") );
	void _FillComboBodyMat( CString strBodyMaterialID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );
	void _FillComboPN( CString strPNID = _T("") );

// Private variables.
private:
	CIndividualSelectionParameters *m_pclIndividualSelectionParams;
	CDlgNoDevFoundParams m_clNoDevFoundParams;

	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboMaterial;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
	CExtNumEditComboBox m_ComboPN;
};