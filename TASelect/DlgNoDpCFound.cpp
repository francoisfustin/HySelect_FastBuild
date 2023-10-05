// DlgNoDpCFound.cpp : implementation file
//

#include "stdafx.h"

#include "TASelect.h"

#include "DlgNoDpCFound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgNoDpCFound::CDlgNoDpCFound( CWnd *pParent )
	: CDialogEx( CDlgNoDpCFound::IDD, pParent )
{
	m_dDpToStab = 0.0;
	m_iDpStab = 0;
	m_iMvLoc = 0;
}

void CDlgNoDpCFound::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
}


BEGIN_MESSAGE_MAP( CDlgNoDpCFound, CDialogEx )
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogTechParam member functions

int CDlgNoDpCFound::Display( double dDpToStab, int iDpStab, int iMvLoc )
{
	m_dDpToStab = dDpToStab;
	m_iDpStab = iDpStab;
	m_iMvLoc = iMvLoc;

	return DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgNoDpCFound message handlers

BOOL CDlgNoDpCFound::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Set the text for the Dp units.
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	
	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_DLGNODPCFOUND_CAPTION );
	SetWindowText( str );

	CString tmp = (CString)WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, m_dDpToStab ), 2, 0, true ) + _T(" ") + name;
	FormatString( str, IDS_DLGNODPCFOUND_STATICNODPC, tmp );
	GetDlgItem( IDC_STATICNODPC )->SetWindowText( str );

	if( !m_iMvLoc )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGNODPCFOUND_STATICDPCEXISTSEC );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGNODPCFOUND_STATICDPCEXISTPRIM );
	}
	
	GetDlgItem( IDC_STATICDPCEXIST )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	return TRUE;
}

void CDlgNoDpCFound::OnOK() 
{
	PREVENT_ENTER_KEY
		
	CDialogEx::OnOK();
}

void CDlgNoDpCFound::OnDestroy() 
{
	CDialogEx::OnDestroy();
}
