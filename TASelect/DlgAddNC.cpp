// DlgAddNC.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "Global.h"
#include "Units.h"
#include "Utilities.h"

#include "DlgAddNC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAddNC dialog


CDlgAddNC::CDlgAddNC(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgAddNC::IDD, pParent)
{
	m_dTs = 0.0;
	m_dTr = 0.0;
	m_dTi = 0.0;
	m_strRadNCID = _T("");
	m_pUSERDB = NULL;
}

void CDlgAddNC::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITTS, m_EditTs);
	DDX_Control(pDX, IDC_EDITTR, m_EditTr);
	DDX_Control(pDX, IDC_EDITTI, m_EditTi);
}


BEGIN_MESSAGE_MAP(CDlgAddNC, CDialogEx)
	ON_EN_CHANGE(IDC_EDITTI, OnChangeEditti)
	ON_EN_CHANGE(IDC_EDITTR, OnChangeEdittr)
	ON_EN_CHANGE(IDC_EDITTS, OnChangeEditts)
END_MESSAGE_MAP()

int CDlgAddNC::Display()
{
	m_pUSERDB = TASApp.GetpUserDB();
	return DoModal();
}

bool CDlgAddNC::CheckTemp(CEdit *pEd,double &value)
{
	CString str;
	pEd->GetWindowText(str);
	switch (ReadDouble(*pEd,&value))
	{
	case RD_EMPTY: 
	case RD_NOT_NUMBER:
		if (str == _T(".") || str == _T(",") || str.Right(1) == _T("e") || 
			str.Right(2) == _T("e+") || str.Right(2) == _T("e-")) 
			return true;
		TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
		pEd->SetSel(0, -1);
		pEd->SetFocus();
		return false;
	case RD_OK:
		if (value <= 0.0)
		{
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
			pEd->SetSel(0, -1);
			pEd->SetFocus();
			return false;
		}
		else
			value = CDimValue::CUtoSI(_U_TEMPERATURE,value);
		return true;
	};
return false;
} 


/////////////////////////////////////////////////////////////////////////////
// CDlgAddNC message handlers

BOOL CDlgAddNC::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGADDNC_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGADDNC_STATICTS);
	GetDlgItem(IDC_STATICTS)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGADDNC_STATICTR);
	GetDlgItem(IDC_STATICTR)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGADDNC_STATICTI);
	GetDlgItem(IDC_STATICTI)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);
	
	// Initialize Unit
	TCHAR unitname[_MAXCHARS];
	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();
	GetNameOf(m_pUnitDB->GetDefaultUnit(_U_TEMPERATURE),unitname);
	GetDlgItem(IDC_STATICTSUNIT)->SetWindowText(unitname);
	GetDlgItem(IDC_STATICTRUNIT)->SetWindowText(unitname);
	GetDlgItem(IDC_STATICTIUNIT)->SetWindowText(unitname);

	m_EditTs.SetWindowText(_T(""));
	m_EditTr.SetWindowText(_T(""));
	m_EditTi.SetWindowText(_T(""));


	m_dTs=m_dTi=m_dTr=0;
	m_strRadNCID=_T("");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAddNC::OnChangeEditti() 
{
	CString str;
	m_EditTi.GetWindowText(str);
	if (!str.IsEmpty())
		CheckTemp(&m_EditTi,m_dTi);	
}

void CDlgAddNC::OnChangeEdittr() 
{
	CString str;
	m_EditTr.GetWindowText(str);
	if (!str.IsEmpty())
		CheckTemp(&m_EditTr,m_dTr);	
}

void CDlgAddNC::OnChangeEditts() 
{
	CString str;
	m_EditTs.GetWindowText(str);
	if (!str.IsEmpty())
		CheckTemp(&m_EditTs,m_dTs);	
}

void CDlgAddNC::OnOK() 
{
	try
	{
		PREVENT_ENTER_KEY
		CString str;
		
		if( m_dTs <= 0.0 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditTs.SetSel( 0, -1 );
			m_EditTs.SetFocus();
			return;
		}
		
		if( m_dTr <= 0.0 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditTr.SetSel( 0, -1 );
			m_EditTr.SetFocus();
			return;
		}
		
		if( m_dTi <= 0.0 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditTi.SetSel( 0, -1 );
			m_EditTi.SetFocus();
			return;
		}

		if( m_dTs < m_dTr )
		{
			CString str1, str2;
			GetDlgItem( IDC_STATICTS )->GetWindowText( str1 );
			GetDlgItem( IDC_STATICTR )->GetWindowText( str2 );
			FormatString( str, IDS_MUSTBEGREATERTHAN, str1, str2 );		
			AfxMessageBox( str );
			m_EditTs.SetSel( 0, -1 );
			m_EditTs.SetFocus();
			return;
		}
		
		if( m_dTr < m_dTi )
		{
			CString str1, str2;
			GetDlgItem( IDC_STATICTR )->GetWindowText( str1 );
			GetDlgItem( IDC_STATICTI )->GetWindowText (str2 );
			FormatString( str, IDS_MUSTBEGREATERTHAN, str1, str2 );
			AfxMessageBox( str );
			m_EditTr.SetSel( 0, -1 );
			m_EditTr.SetFocus();
			return;
		}

		// Check if the object already exist in the USERDB
		bool bFlag = false;
		CTable *pNCTab = (CTable *)( m_pUSERDB->Get( _T("RADNOMCOND_TAB") ).MP );
		
		if( NULL == pNCTab )
		{
			HYSELECT_THROW( _T("Internal error: 'RADNOMCOND_TAB' doesn't exist in the user database.") );
		}

		// Loop on existing CDB_RadNomCond in USERDB radiator nom. Cond. table
		IDPTR RNCIDPtr = _NULL_IDPTR;

		for( RNCIDPtr = pNCTab->GetFirst( CLASS( CDB_RadNomCond ) ); _T('\0') != *RNCIDPtr.ID && false == bFlag; RNCIDPtr = pNCTab->GetNext() )
		{
			if( m_dTs == ( (CDB_RadNomCond *)RNCIDPtr.MP )->GetTs()
					&& m_dTr == ( (CDB_RadNomCond *)RNCIDPtr.MP )->GetTr() 
					&& m_dTi == ( (CDB_RadNomCond *)RNCIDPtr.MP )->GetTi() )
			{
				bFlag = true;
				m_strRadNCID = RNCIDPtr.ID;
			}
		}

		// Object doesn't exist create it.
		if( false == bFlag )
		{	
			m_pUSERDB->CreateObject( RNCIDPtr, CLASS( CDB_RadNomCond ) );
			pNCTab->Insert( RNCIDPtr );

			( (CDB_RadNomCond *)RNCIDPtr.MP )->SetTs( m_dTs );
			( (CDB_RadNomCond *)RNCIDPtr.MP )->SetTr( m_dTr );
			( (CDB_RadNomCond *)RNCIDPtr.MP )->SetTi( m_dTi );
			( (CDB_RadNomCond *)RNCIDPtr.MP )->SetFix( false );
			m_strRadNCID = RNCIDPtr.ID;
		}

		CDialogEx::OnOK();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgAddNC::OnOK'."), __LINE__, __FILE__ );
		throw;
	}
}
