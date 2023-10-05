//
// DialogExportChart.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "XGroupBox.h"

class CLoggedDataChart;

/////////////////////////////////////////////////////////////////////////////
// CDlgExportChart dialog

class CDlgExportChart : public CDialogEx
{
// Export formats
enum enum_ExportChartFormat
{
	eef_Wmf = 1,	// Metafile
	eef_Bmp,		// Bitmap
	eef_Jpg,		// Jpeg
	eef_Png			// Png (Portable Network Graphics)
};

// Construction
public:
	CDlgExportChart(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DLGEXPORTCHART };
	CButton		m_RadioFormat1;
	CButton		m_CheckGrayScale;
	CButton		m_CheckEnhanced;
	CXGroupBox	m_GroupFormat;
	CXGroupBox	m_GroupOptions;


// Overrides
	// ClassWizard generated virtual function overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display(CLoggedDataChart* pChart, CLog* pLD);

protected:
	enum_ExportChartFormat m_CheckedRadio;
	CLoggedDataChart* m_pChart;
	CLog* m_pLD;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioformat1();
	afx_msg void OnRadioformat2();
	afx_msg void OnRadioformat3();
	afx_msg void OnRadioformat4();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};