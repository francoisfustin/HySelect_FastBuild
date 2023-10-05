//
// DlgExportLdlist.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ExtComboBox.h"
#include "SSheetLogData.h"
#include "XGroupBox.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgExportLdlist dialog

class CDlgExportLdlist : public CDialogEx
{
// Export formats
enum enum_ExportLdlistFormat
{
	eef_Txt = 1,	// Text
	eef_Xls,		// Excel
};

// Delimiting characters
enum enum_DelimitingChar
{
	edc_Tab,		// Tab		'\t'
	edc_Space,		// Space	' '
	edc_Comma,		// Comma	','
	edc_Colon,		// Colon	':'
	edc_Semicolon,	// Semicolon';'
	edc_Custom		// Custom
};

// Construction
public:
	CDlgExportLdlist(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DLGEXPORTLDLIST };
	CButton m_RadioFormat1;
	CEdit m_EditDelimiter;
	CExtNumEditComboBox m_ComboDelimiter;
	CButton m_CheckPointIndex;
	CButton m_CheckHeader;
	CXGroupBox m_GroupFormat;
	CXGroupBox m_GroupInclude;

// Overrides
	// ClassWizard generated virtual function overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display(CSSheetLogData* pSheet, CLog* pLD, CString FileName);

protected:
	enum_ExportLdlistFormat m_CheckedRadio;
	TCHAR m_Delimiter;
	CSSheetLogData *m_pSheet;
	CLog *m_pLD;
	CString m_EditFileName;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioformat1();
	afx_msg void OnRadioformat2();
	afx_msg void OnSelchangeCombodelimiter();
	virtual void OnOK();
	afx_msg void OnKillfocusEditdelimiter();
	DECLARE_MESSAGE_MAP()
};