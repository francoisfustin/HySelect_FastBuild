//
// DlgExportLdlist.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"

#include "DlgExportLdlist.h"
#include "Excel_Workbook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportLdlist dialog


CDlgExportLdlist::CDlgExportLdlist(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgExportLdlist::IDD, pParent)
{
	m_CheckedRadio = enum_ExportLdlistFormat::eef_Txt;
	m_pSheet = NULL;
	m_pLD = NULL;
	m_EditFileName = _T("");
	m_Delimiter = '\0';
}


void CDlgExportLdlist::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIOFORMAT1, m_RadioFormat1);
	DDX_Control(pDX, IDC_EDITDELIMITER, m_EditDelimiter);
	DDX_Control(pDX, IDC_COMBODELIMITER, m_ComboDelimiter);
	DDX_Control(pDX, IDC_CHECKPOINTINDEX, m_CheckPointIndex);
	DDX_Control(pDX, IDC_CHECKHEADER, m_CheckHeader);
	DDX_Control(pDX, IDC_GROUPFORMAT, m_GroupFormat);
	DDX_Control(pDX, IDC_GROUPINCLUDE, m_GroupInclude);
}


BEGIN_MESSAGE_MAP(CDlgExportLdlist, CDialogEx)
	ON_BN_CLICKED(IDC_RADIOFORMAT1, OnRadioformat1)
	ON_BN_CLICKED(IDC_RADIOFORMAT2, OnRadioformat2)
	ON_CBN_SELCHANGE(IDC_COMBODELIMITER, OnSelchangeCombodelimiter)
	ON_EN_KILLFOCUS(IDC_EDITDELIMITER, OnKillfocusEditdelimiter)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExportLdlist member functions

int CDlgExportLdlist::Display(CSSheetLogData* pSheet, CLog* pLD, CString FileName)
{
	m_pSheet = pSheet;
	m_pLD = pLD;
	m_EditFileName = FileName;

	return DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgExportLdlist message handlers

BOOL CDlgExportLdlist::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_GROUPFORMAT);
	m_GroupFormat.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_RADIOFORMAT1);
	GetDlgItem(IDC_RADIOFORMAT1)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_RADIOFORMAT2);
	GetDlgItem(IDC_RADIOFORMAT2)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_GROUPINCLUDE);
	m_GroupInclude.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_CHECKPOINTINDEX);
	GetDlgItem(IDC_CHECKPOINTINDEX)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_CHECKHEADER);
	GetDlgItem(IDC_CHECKHEADER)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGEXPORTLDLIST_STATICDELIMITER);
	GetDlgItem(IDC_STATICDELIMITER)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);
	str.Empty();

	m_RadioFormat1.SetCheck(1);
	m_CheckedRadio = eef_Txt;
	OnRadioformat1();

	m_CheckPointIndex.SetCheck(1);
	m_CheckHeader.SetCheck(1);

	// Fill Delimiter Combo box
	int index;
	str=TASApp.LoadLocalizedString(IDS_TABCHARACTER);
	index = m_ComboDelimiter.AddString(str);
	m_ComboDelimiter.SetItemData(index, edc_Tab);
	str=TASApp.LoadLocalizedString(IDS_SPACECHARACTER);
	index = m_ComboDelimiter.AddString(str);
	m_ComboDelimiter.SetItemData(index, edc_Space);
	str=TASApp.LoadLocalizedString(IDS_COMMA);
	index = m_ComboDelimiter.AddString(str);
	m_ComboDelimiter.SetItemData(index, edc_Comma);
	str=TASApp.LoadLocalizedString(IDS_COLON);
	index = m_ComboDelimiter.AddString(str);
	m_ComboDelimiter.SetItemData(index, edc_Colon);
	str=TASApp.LoadLocalizedString(IDS_SEMICOLON);
	index = m_ComboDelimiter.AddString(str);
	m_ComboDelimiter.SetItemData(index, edc_Semicolon);
	str=TASApp.LoadLocalizedString(IDS_CUSTOMCHARACTER);
	index = m_ComboDelimiter.AddString(str);
	m_ComboDelimiter.SetItemData(index, edc_Custom);

	m_EditDelimiter.EnableWindow(FALSE);
	m_EditDelimiter.LimitText(1);

	m_ComboDelimiter.SetCurSel(0);
	OnSelchangeCombodelimiter();


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExportLdlist::OnRadioformat1() 
{
	GetDlgItem(IDC_GROUPINCLUDE)->EnableWindow(TRUE);
	m_CheckPointIndex.EnableWindow(TRUE);
	m_CheckHeader.EnableWindow(TRUE);
	m_ComboDelimiter.EnableWindow(TRUE);
	GetDlgItem(IDC_STATICDELIMITER)->EnableWindow(TRUE);
	m_CheckedRadio = eef_Txt;
}

void CDlgExportLdlist::OnRadioformat2() 
{
	GetDlgItem(IDC_GROUPINCLUDE)->EnableWindow(FALSE);
	m_CheckPointIndex.EnableWindow(FALSE);
	m_CheckHeader.EnableWindow(FALSE);
	m_CheckPointIndex.SetCheck(1);
	m_CheckHeader.SetCheck(1);
	m_ComboDelimiter.EnableWindow(FALSE);
	GetDlgItem(IDC_STATICDELIMITER)->EnableWindow(FALSE);
	m_CheckedRadio = eef_Xls;
}

void CDlgExportLdlist::OnSelchangeCombodelimiter() 
{
	int Delimiter = m_ComboDelimiter.GetItemData(m_ComboDelimiter.GetCurSel());
	
	switch (Delimiter)
	{
	case edc_Tab:
		m_EditDelimiter.EnableWindow(FALSE);
		m_EditDelimiter.SetWindowText(_T(""));
		m_Delimiter = '\t';
		break;
	case edc_Space:
		m_EditDelimiter.EnableWindow(FALSE);
		m_EditDelimiter.SetWindowText(_T(""));
		m_Delimiter = ' ';
		break;
	case edc_Comma:
		m_EditDelimiter.EnableWindow(FALSE);
		m_EditDelimiter.SetWindowText(_T(""));
		m_Delimiter = ',';
		break;
	case edc_Colon:
		m_EditDelimiter.EnableWindow(FALSE);
		m_EditDelimiter.SetWindowText(_T(""));
		m_Delimiter = ':';
		break;
	case edc_Semicolon:
		m_EditDelimiter.EnableWindow(FALSE);
		m_EditDelimiter.SetWindowText(_T(""));
		m_Delimiter = ';';
		break;
	case edc_Custom:
		m_EditDelimiter.EnableWindow(TRUE);
		m_Delimiter = '\0';	
		break;
	};
}

void CDlgExportLdlist::OnKillfocusEditdelimiter() 
{
	if (m_ComboDelimiter.GetItemData(m_ComboDelimiter.GetCurSel()) != edc_Custom)
		return;

	CString str;
	m_EditDelimiter.GetWindowText(str);
	if (str.IsEmpty())
		m_Delimiter = '\0';
	else
		m_Delimiter = str.GetAt(0);
}

void CDlgExportLdlist::OnOK() 
{
	if (m_CheckedRadio == eef_Txt && m_Delimiter == '\0')
	{
		TASApp.AfxLocalizeMessageBox(AFXMSG_ONE_DELIM_CHAR);
		m_EditDelimiter.SetFocus();
		return;
	};

	CString PrjDir = GetProjectDirectory();

	// Load file filter and initialize CFileDialog
	CString str,strExt;
	switch (m_CheckedRadio)
	{
	case eef_Txt:
		str=TASApp.LoadLocalizedString(IDS_EXPORTTXTFILTER);
		strExt = _T("txt");
		break;
	case eef_Xls:
		str=TASApp.LoadLocalizedString(IDS_EXPORTXLSXFILTER);
		strExt = _T("xlsx");
		break;
	};

	// Modify default file name for Txt export (all parts are printed in one file)
	if (m_CheckedRadio == eef_Txt)
	{
		CString strTmp;
		strTmp=TASApp.LoadLocalizedString(IDS_PART);
		int index = m_EditFileName.Find(strTmp,0);
		if (index > -1)
			m_EditFileName.Delete(index-3,m_EditFileName.GetLength()-index+3);
	};

	CDialogEx::OnOK();

	CFileDialog dlg(false,strExt,m_EditFileName,OFN_HIDEREADONLY| OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,(LPCTSTR)str, NULL );
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
			case eef_Txt:
				{
					CString strChar = CString (m_Delimiter);
					m_pLD->WriteAsText(dlg.GetPathName(),m_CheckPointIndex.GetCheck()==1,m_CheckHeader.GetCheck()==1,strChar);
				}
				break;
			case eef_Xls:
// 				m_pSheet->SetGridShowHoriz(true); 
// 				m_pSheet->SetGridShowVert(true);
				m_pSheet->SetBool( SSB_HORZSCROLLBAR , TRUE );
				m_pSheet->SetBool( SSB_VERTSCROLLBAR , TRUE );
				//m_pSheet->ExportExcelBook(dlg.GetPathName(), NULL);
				Excel_Workbook wbSheet;
				wbSheet.AddSheet( m_pSheet );
				wbSheet.Write( dlg.GetPathName() );
//				m_pSheet->SetGridShowHoriz(false); 
//				m_pSheet->SetGridShowVert(false);
				m_pSheet->SetBool( SSB_HORZSCROLLBAR , FALSE );
				m_pSheet->SetBool( SSB_VERTSCROLLBAR , FALSE );
				break;
		}

		EndWaitCursor();
	}
}

