#pragma once


#include "afxwin.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"

class CDlgDocs : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgDocs )

public:	
	enum { IDD = IDD_DLGDOCS };

	CDlgDocs( CWnd* pParent = NULL );
	virtual ~CDlgDocs();

	int Display( bool fFirstOpening = false );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnNMClickTreeDocs( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSelChangeComboDftLangDoc();

// Private methods.
private:
	void _FillComboChangeLanguage( void );

	// Fill the Tree box with all available doc languages.
	void _FillTreeBox();

	// Try to find a specific folder in the current path. The function is recursive and will loop on all children's folders.
	// If the we specify bRecursive = false, it will only loop on the current directory.
	bool _FindFolder( CString strCurrentPath, CString strFolderName, bool fRecursive = false );

// Protected variables.
protected:
	CXGroupBox m_GroupDoc;
	CXGroupBox m_GroupTASCOPE;
	CTreeCtrl m_TreeDoc;
	CButton m_BtnTASCOPE;
	bool m_bFirstOpening;

// Private variables.
private:
	CExtNumEditComboBox m_ChangeLanguage;
	CString m_strPrevDocLanguage;
	CString m_strNewDocLanguage;
};
