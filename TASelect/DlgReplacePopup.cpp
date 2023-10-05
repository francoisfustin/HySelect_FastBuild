// DlgReplacePopup.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "DlgReplacePopup.h"

// CDlgReplacePopup dialog


IMPLEMENT_DYNAMIC(CDlgReplacePopup, CDialogEx)

CDlgReplacePopup::CDlgReplacePopup( CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgReplacePopup::IDD, pParent)
{
	m_pParent = pParent;
}

CDlgReplacePopup::~CDlgReplacePopup()
{
}

void CDlgReplacePopup::UpdateData()
{
	m_StaticFind1.SetWindowText(m_strStaticFind1);
	m_StaticFind2.SetWindowText(m_strStaticFind2);
	m_StaticReplace.SetWindowText(m_strStaticReplace);
}

void CDlgReplacePopup::DisableFindNextBtn()
{
	m_BtnFindNext.EnableWindow(false);
}

void CDlgReplacePopup::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FIND1, m_StaticFind1);
	DDX_Control(pDX, IDC_STATIC_FIND2, m_StaticFind2);
	DDX_Control(pDX, IDC_STATIC_REPLACE, m_StaticReplace);
	DDX_Control(pDX, IDC_GROUP_FIND, m_GroupFind);
	DDX_Control(pDX, IDC_GROUP_REPLACE, m_GroupReplace);
	DDX_Control(pDX, IDC_BTN_FINDNEXT, m_BtnFindNext);
	DDX_Control(pDX, IDC_BTN_REPLACE, m_BtnReplace);
	DDX_Control(pDX, IDC_BTN_REPLACEALL, m_BtnReplaceAll);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_BtnClose);
}

BOOL CDlgReplacePopup::OnInitDialog()
{
	BOOL Breturn = CDialogEx::OnInitDialog();

	// Init strings
	CString str;
	m_GroupFind.SetWindowText(m_strGroupFind);
	m_GroupReplace.SetWindowText(m_strGroupReplace);
	str = TASApp.LoadLocalizedString(IDS_DLGREPLACEPOPUP_FINDNEXT);
	m_BtnFindNext.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGREPLACEPOPUP_REPLACE);
	m_BtnReplace.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGREPLACEPOPUP_REPLACEALL);
	m_BtnReplaceAll.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_CLOSE);
	m_BtnClose.SetWindowText(str);
	
	// Define the Ctrl style for the group box
	m_GroupFind.SetControlStyle(CXGroupBox::CONTROL_STYLE::header,FALSE);
	m_GroupReplace.SetControlStyle(CXGroupBox::CONTROL_STYLE::header,FALSE);

	// Send message to notify the first item
	m_pParent->SendMessage(WM_USER_REPLACEPOPUPFINDNEXT);
	// Update the data info
	UpdateData();

	return Breturn;
}

BEGIN_MESSAGE_MAP(CDlgReplacePopup, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_FINDNEXT, OnBnClickedBtnFindnext)
	ON_BN_CLICKED(IDC_BTN_REPLACE, OnBnClickedBtnReplace)
	ON_BN_CLICKED(IDC_BTN_REPLACEALL, OnBnClickedBtnReplaceall)
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBnClickedBtnClose)
END_MESSAGE_MAP()


// CDlgReplacePopup message handlers

void CDlgReplacePopup::OnBnClickedBtnFindnext()
{
	m_pParent->SendMessage(WM_USER_REPLACEPOPUPFINDNEXT);
}

void CDlgReplacePopup::OnBnClickedBtnReplace()
{
	m_pParent->SendMessage(WM_USER_REPLACEPOPUPREPLACE);
}

void CDlgReplacePopup::OnBnClickedBtnReplaceall()
{
	m_pParent->SendMessage(WM_USER_REPLACEPOPUPREPLACEALL);
}

void CDlgReplacePopup::OnBnClickedBtnClose()
{
	m_pParent->SendMessage(WM_USER_CLOSEPOPUPWND);
	CDialogEx::OnOK();
}
