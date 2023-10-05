// DlgRibbonDateTime.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "DlgRibbonDateTime.h"


// CDlgRibbonDateTime dialog

IMPLEMENT_DYNAMIC(CDlgRibbonDateTime, CDialogEx)

CDlgRibbonDateTime::CDlgRibbonDateTime(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgRibbonDateTime::IDD, pParent)
{

}

CDlgRibbonDateTime::~CDlgRibbonDateTime()
{
}

void CDlgRibbonDateTime::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATEPICKERUP, m_DatePickStart);
	DDX_Control(pDX, IDC_DATEPICKERDOWN, m_DatePickEnd);
	DDX_Control(pDX, IDC_TIMEPICKERUP, m_TimePickStart);
	DDX_Control(pDX, IDC_TIMEPICKERDOWN, m_TimePickEnd);
	
}

BOOL CDlgRibbonDateTime::OnInitDialog() 
{
	BOOL bReturn = CDialogEx::OnInitDialog();

	// Set a string for the Dialog
	CString str=TASApp.LoadLocalizedString(IDS_DLGRIBBONDATETIME_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGRIBBONDATETIME_RESET);
	GetDlgItem(IDC_BUTTONRESET)->SetWindowText(str);

	// Fill the Upper DateTimePicker
	m_DatePickStart.SetTime(pMainFrame->GetLowerDateTime());
	m_TimePickStart.SetTime(pMainFrame->GetLowerDateTime());
	
	// Fill the Lower DateTimePicker
	m_DatePickEnd.SetTime(pMainFrame->GetUpperDateTime());
	m_TimePickEnd.SetTime(pMainFrame->GetUpperDateTime());

	return bReturn;
}


BEGIN_MESSAGE_MAP(CDlgRibbonDateTime, CDialogEx)
	ON_BN_CLICKED(IDOK, OnBnClickedOK)
	ON_BN_CLICKED(IDC_BUTTONRESET, OnBnClickedReset)
END_MESSAGE_MAP()


// CDlgRibbonDateTime message handlers
void CDlgRibbonDateTime::OnBnClickedOK()
{
	// Variables
	COleDateTime LowerDate,UpperDate,LowerTime,UpperTime;

	// Recuperate the date and time from the DateTimePickers
	m_DatePickStart.GetTime(LowerDate);
	m_DatePickEnd.GetTime(UpperDate);
	m_TimePickStart.GetTime(LowerTime);
	m_TimePickEnd.GetTime(UpperTime);
	
	// Set the Time in the Date picker
	UpperDate.SetDateTime(UpperDate.GetYear(),UpperDate.GetMonth(),UpperDate.GetDay(),UpperTime.GetHour(),UpperTime.GetMinute(),UpperTime.GetSecond());
	LowerDate.SetDateTime(LowerDate.GetYear(),LowerDate.GetMonth(),LowerDate.GetDay(),LowerTime.GetHour(),LowerTime.GetMinute(),LowerTime.GetSecond());

	// Verify the values inserted
	if (LowerDate < UpperDate)
	{
		// Verify if the user go outside the maximum scale
		// Save the values in the member variable in the MainFrame for the Ribbon bar
		if (LowerDate > pMainFrame->GetMaxLowerDateTime())
			pMainFrame->SetLowerDateTime(LowerDate);
		else
			pMainFrame->SetLowerDateTime(pMainFrame->GetMaxLowerDateTime());
		if (UpperDate < pMainFrame->GetMaxUpperDateTime())
			pMainFrame->SetUpperDateTime(UpperDate);
		else
			pMainFrame->SetUpperDateTime(pMainFrame->GetMaxUpperDateTime());
	}
	
	OnOK();
}

// Reset to the Min Max value
void CDlgRibbonDateTime::OnBnClickedReset()
{
	// Fill the Upper DateTimePicker
	m_DatePickStart.SetTime(pMainFrame->GetMaxLowerDateTime());
	m_TimePickStart.SetTime(pMainFrame->GetMaxLowerDateTime());
	
	// Fill the Lower DateTimePicker
	m_DatePickEnd.SetTime(pMainFrame->GetMaxUpperDateTime());
	m_TimePickEnd.SetTime(pMainFrame->GetMaxUpperDateTime());

}