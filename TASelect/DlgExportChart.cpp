//
// DlgExportChart.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"
#include "LoggedDataChart.h"
#include "TeeChartDefines.h"
#include "Export.h"
#include "MetafileExport.h"
#include "BMPExport.h"
#include "JpegExport.h"
#include "PNGExport.h"

#include "DlgExportChart.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportChart dialog


CDlgExportChart::CDlgExportChart(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgExportChart::IDD, pParent)
{
	m_CheckedRadio = enum_ExportChartFormat::eef_Wmf;
	m_pChart = NULL;
	m_pLD = NULL;
	m_CheckedRadio = eef_Bmp;
}


void CDlgExportChart::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIOFORMAT1, m_RadioFormat1);
	DDX_Control(pDX, IDC_CHECKGRAYSCALE, m_CheckGrayScale);
	DDX_Control(pDX, IDC_CHECKENHANCED, m_CheckEnhanced);
	DDX_Control(pDX, IDC_GROUPFORMAT, m_GroupFormat);
	DDX_Control(pDX, IDC_GROUPOPTIONS, m_GroupOptions);
}


BEGIN_MESSAGE_MAP(CDlgExportChart, CDialogEx)
	ON_BN_CLICKED(IDC_RADIOFORMAT1, OnRadioformat1)
	ON_BN_CLICKED(IDC_RADIOFORMAT2, OnRadioformat2)
	ON_BN_CLICKED(IDC_RADIOFORMAT3, OnRadioformat3)
	ON_BN_CLICKED(IDC_RADIOFORMAT4, OnRadioformat4)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExportChart member functions

int CDlgExportChart::Display(CLoggedDataChart* pChart, CLog* pLD)
{
	m_pChart = pChart;
	m_pLD = pLD;

	return DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgExportChart message handlers

BOOL CDlgExportChart::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_GROUPFORMAT);
	m_GroupFormat.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_RADIOFORMAT1);
	GetDlgItem(IDC_RADIOFORMAT1)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_RADIOFORMAT2);
	GetDlgItem(IDC_RADIOFORMAT2)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_RADIOFORMAT3);
	GetDlgItem(IDC_RADIOFORMAT3)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_RADIOFORMAT4);
	GetDlgItem(IDC_RADIOFORMAT4)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_GROUPOPTIONS);
	m_GroupOptions.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_CHECKENHANCED);
	GetDlgItem(IDC_CHECKENHANCED)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTCHART_CHECKGRAYSCALE);
	GetDlgItem(IDC_CHECKGRAYSCALE)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);

	m_RadioFormat1.SetCheck(1);
	m_CheckedRadio = eef_Wmf;
	OnRadioformat1();

	m_CheckEnhanced.SetCheck(1);
	m_CheckGrayScale.SetCheck(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExportChart::OnRadioformat1() 
{
	m_CheckEnhanced.ShowWindow(SW_SHOW);
	m_CheckGrayScale.ShowWindow(SW_HIDE);
	m_CheckedRadio = eef_Wmf;

}

void CDlgExportChart::OnRadioformat2() 
{
	m_CheckEnhanced.ShowWindow(SW_HIDE);
	m_CheckGrayScale.ShowWindow(SW_HIDE);
	m_CheckedRadio = eef_Bmp;

}

void CDlgExportChart::OnRadioformat3() 
{
	m_CheckEnhanced.ShowWindow(SW_HIDE);
	m_CheckGrayScale.ShowWindow(SW_SHOW);
	m_CheckedRadio = eef_Jpg;

}

void CDlgExportChart::OnRadioformat4() 
{
	m_CheckEnhanced.ShowWindow(SW_HIDE);
	m_CheckGrayScale.ShowWindow(SW_HIDE);
	m_CheckedRadio = eef_Png;

}

void CDlgExportChart::OnOK() 
{
	CString PrjDir = GetProjectDirectory();

	// Load file filter and initialize CFileDialog
	CString str,strExt;
	switch (m_CheckedRadio)
	{
	case eef_Wmf:
		if (m_CheckEnhanced.GetCheck())
		{
			str=TASApp.LoadLocalizedString(IDS_EXPORTEMFFILTER);
			strExt = _T("emf");
		}
		else
		{
			str=TASApp.LoadLocalizedString(IDS_EXPORTWMFFILTER);
			strExt = _T("wmf");
		}
		break;
	case eef_Bmp:
		str=TASApp.LoadLocalizedString(IDS_EXPORTBMPFILTER);
		strExt = _T("bmp");
		break;
	case eef_Jpg:
		str=TASApp.LoadLocalizedString(IDS_EXPORTJPGFILTER);
		strExt = _T("jpg");
		break;
	case eef_Png:
		str=TASApp.LoadLocalizedString(IDS_EXPORTPNGFILTER);
		strExt = _T("png");
		break;
	};

	CDialogEx::OnOK();

	CFileDialog dlg(false,strExt,m_pLD->GetName(),OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,(LPCTSTR)str, NULL );
	dlg.m_ofn.lpstrInitialDir=(LPCTSTR)PrjDir;
	if (dlg.DoModal()==IDOK)
	{
		// Save Project directory if modified
		if (dlg.GetPathName()!=PrjDir)
		{
			CPath path(dlg.GetPathName());
			CString str = path.SplitPath((CPath::ePathFields)(CPath::ePathFields::epfDrive+CPath::ePathFields::epfDir));
			AfxGetApp()->WriteProfileString(_T("Files"),_T("Project Directory"),str);
		}
		BeginWaitCursor();

		// Write export file on disk
		switch (m_CheckedRadio)
		{
		case eef_Wmf:
			m_pChart->GetExport().GetAsMetafile().SetEnhanced(m_CheckEnhanced.GetCheck());
			m_pChart->GetExport().GetAsMetafile().SaveToFile(dlg.GetPathName());
			break;
		case eef_Bmp:
			m_pChart->GetExport().GetAsBMP().SaveToFile(dlg.GetPathName());
			break;
		case eef_Jpg:
			m_pChart->GetExport().GetAsJPEG().SetGrayScale(m_CheckGrayScale.GetCheck());
			m_pChart->GetExport().GetAsJPEG().SaveToFile(dlg.GetPathName());
			break;
		case eef_Png:
			m_pChart->GetExport().GetAsPNG().SaveToFile(dlg.GetPathName());
			break;
		};

		EndWaitCursor();
	}

}
