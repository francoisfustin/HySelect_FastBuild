#pragma once


#include "ExtComboBox.h"

class CDlgNoDevSeparatorFoundParams
{
// Public methods.
public:
	CDlgNoDevSeparatorFoundParams();
	CDlgNoDevSeparatorFoundParams( CRankEx *pList, CString strTypeID, CString strFamilyID, CString strConnectID, CString strVersionID );

	virtual ~CDlgNoDevSeparatorFoundParams() {}

	CDlgNoDevSeparatorFoundParams &operator=( CDlgNoDevSeparatorFoundParams &rNoDevSeparatorFoundParams );

// Public variables.
public:
	CRankEx *m_pList;
	CString m_strTypeID;
	CString m_strFamilyID;
	CString m_strConnectID;
	CString m_strVersionID;
};

class CIndividualSelectionParameters;
class CDlgNoDevSeparatorFound : public CDialogEx
{
public:
	enum { IDD = IDD_DLGNODEVSEPARATORFOUND };

	CDlgNoDevSeparatorFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevSeparatorFoundParams *pclNoDevSeparatorFoundParams,
			CWnd *pParent = NULL );

	virtual ~CDlgNoDevSeparatorFound() {}

	// Opens the dialog window.
	CDlgNoDevSeparatorFoundParams *GetNoDevSeparatorFoundParams( void ) { return &m_clNoDevSeparatorFoundParams; }

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	
	virtual afx_msg void OnCbnSelChangeType();
	virtual afx_msg void OnCbnSelChangeFamily();
	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangeVersion();


// Private methods.
private:
	void _FillComboType( CString strTypeID = _T("") );
	void _FillComboFamily( CString strFamilyID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );

// Private variables.
private:
	CIndividualSelectionParameters *m_pclIndividualSelectionParams;
	CDlgNoDevSeparatorFoundParams m_clNoDevSeparatorFoundParams;

	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
};
