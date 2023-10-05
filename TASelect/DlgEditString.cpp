#include "stdafx.h"
#include "TASelect.h"
#include "DlgEditString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgEditString::CDlgEditString( CWnd* pParent )
	: CDialogEx( CDlgEditString::IDD, pParent )
{
	m_strTitle = _T("");
	m_strStatic = _T("");
	m_pstrName = NULL;
}

int CDlgEditString::Display( CString &strTitle, CString &strStatic, CString *pstrName )
{
	m_strTitle = strTitle;
	m_strStatic = strStatic;
	m_pstrName = pstrName;
	return DoModal();
}

void CDlgEditString::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICNAME, m_strStaticName );
	DDX_Control( pDX, IDC_EDIT, m_Edit );
}

BOOL CDlgEditString::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	SetWindowText( m_strTitle );
	m_strStaticName.SetWindowText( m_strStatic );
	m_Edit.SetWindowText( *m_pstrName );
	m_Edit.SetSel( 0, -1 );
	m_Edit.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgEditString::OnOK() 
{
	m_Edit.GetWindowText( *m_pstrName );
	CDialogEx::OnOK();
}
