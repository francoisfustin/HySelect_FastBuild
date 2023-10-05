#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "MyToolTipCtrl.h"
#include "EnBitmap.h"
#include "HydroMod.h"
#include "XGroupBox.h"


// DlgExportConfigFile dialog

class DlgExportConfigFile : public CDialogEx
{
	DECLARE_DYNAMIC(DlgExportConfigFile)

public:
	DlgExportConfigFile(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgExportConfigFile();
	virtual BOOL OnInitDialog();

	CButton		m_ButtonProjectInfo;
	CButton		m_ButtonUnselectAll;
	CButton		m_ButtonSelectAll;
	CXGroupBox	m_GroupSelForExp;
	CXGroupBox	m_GroupProjInfo;
	CMyToolTipCtrl m_ToolTip;

// Dialog Data
	enum { IDD = IDD_DLGEXPORTCONFIGURATIONFILE };

protected:
	CTADatastruct* m_pTADS;
	CListCtrl m_ListHub;
	CStatic m_Img;
	CEdit m_EditCustomerName;
	CEnBitmap m_BmpHub;
	CImageList m_ImageList;
	CTADatabase *m_pTADB;
	int m_iSelectedRow;
	CStatic m_StaticSelectedHubs;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void FormatStaticSelectedForExport();
	void UpdateEnableOKButton(); // enable the button ok if minimum one hub is selected.

	void SelectRowInTheList(int Row);

	void WriteCString(CFile& outf, LPCTSTR string);	// write a CString in a file.
	void GenerateXML(CFile& outf);					// Generate XML file for selected TA-Hub configurations
	void WriteXmlHeadElt(CFile& outf);				// Write the XML file head element
	void WriteXmlOrderLineElts(CFile& outf);		// Write the order line elements for the selected TA-Hubs
	void WriteXmlConfigurationElt(CFile& outf,CDS_HmHub *currentHUB);	// Write the configuration element for currentHUB
	void WriteXmlConfElt(CFile& outf,CString cIDAttr,CString vIDAttr);	// Write empty conf element with charID and valueID attributes
	void WriteJpgFiles(CString filename);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonselectall();
	afx_msg void OnBnClickedButtonunselectall();
	afx_msg void OnBnClickedButtonprojectinfo();
	afx_msg void OnBnClickedRadioexportall();
	afx_msg void OnBnClickedRadioexportselectedconfigurationonly();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnLvnItemchangedListhub(NMHDR *pNMHDR, LRESULT *pResult);
};
