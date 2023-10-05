#include "stdafx.h"


#include "TASelect.h"
#include "TabDlgSpecActUser.h"

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

/////////////////////////////////////////////////////////////////////////////
// CTabDlgSpecActUser dialog


CTabDlgSpecActUser::CTabDlgSpecActUser(CWnd* pParent /*=NULL*/)
	: CDialogExt(CTabDlgSpecActUser::IDD, pParent)
{
	SetBackgroundColor(_WHITE_DLGBOX);
}


void CTabDlgSpecActUser::DoDataExchange(CDataExchange* pDX)
{
	CDialogExt::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECKDROPSERIAL, m_CheckDropSerial);
}


BEGIN_MESSAGE_MAP(CTabDlgSpecActUser, CDialogExt)
	ON_BN_CLICKED(IDC_CHECKDROPSERIAL, OnCheckdropserial)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabDlgSpecActUser message handlers

void CTabDlgSpecActUser::OnCheckdropserial() 
{
	TASApp.SetDebugCom(m_CheckDropSerial.GetCheck()?1:0);	
}

BOOL CTabDlgSpecActUser::OnInitDialog() 
{
	CDialogExt::OnInitDialog();
	
	m_CheckDropSerial.SetCheck(TASApp.IsDebugComActif());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
