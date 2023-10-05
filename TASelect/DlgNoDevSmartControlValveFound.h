#pragma once


#include "ExtComboBox.h"

class CDlgNoDevSmartControlValveFoundParams
{
// Public methods.
public:
	CDlgNoDevSmartControlValveFoundParams();
	CDlgNoDevSmartControlValveFoundParams( CRankEx *pList, CString strBodyMaterialID, CString strConnectID, CString strPNID );

	virtual ~CDlgNoDevSmartControlValveFoundParams() {}

	CDlgNoDevSmartControlValveFoundParams &operator=( CDlgNoDevSmartControlValveFoundParams &rNoDevSmartControlValveFoundParams );

// Public variables.
public:
	CRankEx *m_pList;
	CString m_strBodyMaterialID;
	CString m_strConnectID;
	CString m_strPNID;
};

class CIndividualSelectionParameters;
class CDlgNoDevSmartControlValveFound : public CDialogEx
{
public:
	enum { IDD = IDD_DLGNODEVSMARTCONTROLVALVEFOUND };

	CDlgNoDevSmartControlValveFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevSmartControlValveFoundParams *pclNoDevSmartControlValveFoundParams,
			CWnd *pParent = NULL );

	virtual ~CDlgNoDevSmartControlValveFound() {}

	// Opens the dialog window.
	CDlgNoDevSmartControlValveFoundParams *GetNoDevFoundSmartControlValveParams( void ) { return &m_clNoDevSmartControlValveFoundParams; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	
	virtual afx_msg void OnCbnSelChangeBodyMaterial();
	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangePN();


// Private methods.
private:
	void _FillComboBodyMaterial( CString strBodyMaterialID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboPN( CString strPNID = _T("") );

// Private variables.
private:
	CIndividualSelectionParameters *m_pclIndividualSelectionParams;
	CDlgNoDevSmartControlValveFoundParams m_clNoDevSmartControlValveFoundParams;

	CExtNumEditComboBox m_ComboBodyMaterial;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboPN;
};
