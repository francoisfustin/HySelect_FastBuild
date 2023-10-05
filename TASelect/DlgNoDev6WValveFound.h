#pragma once


#include "ExtComboBox.h"

// REMARK: for the moment, this dialog is only for the normal thermostatic valve and not the insert version.

class CDlgNoDev6WValveFoundParams
{
// Public methods.
public:
	CDlgNoDev6WValveFoundParams();
	CDlgNoDev6WValveFoundParams( CRankEx *pList, CString strConnectID, CString strVersionID );

	virtual ~CDlgNoDev6WValveFoundParams() {}

	CDlgNoDev6WValveFoundParams &operator=( CDlgNoDev6WValveFoundParams &rNoDev6WValveFoundParams );

// Public variables.
public:
	CRankEx *m_pList;
	CString m_strConnectID;
	CString m_strVersionID;
};

class CIndividualSelectionParameters;
class CDlgNoDev6WValveFound : public CDialogEx
{
public:
	enum { IDD = IDD_DLGNODEV6WVALVEFOUND };

	CDlgNoDev6WValveFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDev6WValveFoundParams *pclNoDev6WValveFoundParams, CWnd *pParent = NULL );
	virtual ~CDlgNoDev6WValveFound() {}

	CDlgNoDev6WValveFoundParams *GetNoDev6WValveFoundParams( void ) { return &m_clNoDev6WValveFoundParams; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	virtual afx_msg void OnCbnSelChangeConnect();
	virtual afx_msg void OnCbnSelChangeVersion();

// Private methods.
private:
	void _FillComboConnect( CString strConnectID = _T("") );
	void _FillComboVersion( CString strVersionID = _T("") );

// Private variables.
public:
	CIndividualSelectionParameters *m_pclIndividualSelectionParams;
	CDlgNoDev6WValveFoundParams m_clNoDev6WValveFoundParams;

	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;
};
