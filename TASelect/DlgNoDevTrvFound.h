#pragma once


#include "ExtComboBox.h"

// REMARK: for the moment, this dialog is only for the normal thermostatic valve and not the insert version.

class CDlgNoDevTrvFoundParams
{
// Public methods.
public:
	CDlgNoDevTrvFoundParams();
	CDlgNoDevTrvFoundParams( CRankEx *pList, CString strFamilyID, CString strConnectID, CString strVersionID );

	virtual ~CDlgNoDevTrvFoundParams() {}

	CDlgNoDevTrvFoundParams &operator=( CDlgNoDevTrvFoundParams &rNoDevTrvFoundParams );

// Public variables.
public:
	CRankEx *m_pList;
	CString m_strFamilyID;
	CString m_strConnectID;
	CString m_strVersionID;
};

class CIndividualSelectionParameters;
class CDlgNoDevTrvFound : public CDialogEx
{
public:
	enum { IDD = IDD_DLGNODEVTRVFOUND };

	CDlgNoDevTrvFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevTrvFoundParams *pclNoDevTrvFoundParams, CWnd *pParent = NULL );
	virtual ~CDlgNoDevTrvFound() {}

	CDlgNoDevTrvFoundParams *GetNoDevTrvFoundParams( void ) { return &m_clNoDevTrvFoundParams; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	virtual afx_msg void OnCbnSelChangeFamily();
	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangeVersion();

// Private methods.
private:
	void _FillComboFamily( CString strFamilyID = _T("") );
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );

// Private variables.
public:
	CIndividualSelectionParameters *m_pclIndividualSelectionParams;
	CDlgNoDevTrvFoundParams m_clNoDevTrvFoundParams;

	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
};
