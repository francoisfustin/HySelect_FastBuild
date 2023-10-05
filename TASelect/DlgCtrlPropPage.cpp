#include "stdafx.h"
#include "TASelect.h"
#include "DlgCtrlPropPage.h"


IMPLEMENT_DYNAMIC( CDlgCtrlPropPage, CDialogExt )

CDlgCtrlPropPage::CDlgCtrlPropPage( CWnd *pParent )
	: CDialogExt( CDlgCtrlPropPage::IDD, pParent )
{
	m_csPageName = _T("");
	m_csHeaderTitle = _T("");
	m_iPageNumber = 0;
	m_pDlgReplacePopup = NULL;
	SetBackgroundColor( _WHITE_DLGBOX );
}

void CDlgCtrlPropPage::DoDataExchange( CDataExchange* pDX )
{
	CDialogExt::DoDataExchange( pDX );
}

BOOL CDlgCtrlPropPage::OnInitDialog()
{
	CDialogExt::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgCtrlPropPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
	CWnd *pWnd = GetFocus();
	switch( wParam )
	{
		case IDOK:
			if( pWnd != GetDlgItem( IDOK ) )
			{
				return FALSE;
			}
			break;

		case IDCANCEL:
			if( pWnd != GetDlgItem( IDCANCEL ) )
			{
				return FALSE;
			}
			break;
	}

	return CDialogExt::OnCommand( wParam, lParam );
}

bool CDlgCtrlPropPage::VerifyInfo()
{
	// By default if a page doesn't have info to verify,
	// the function will return true.
	return true;
}

void CDlgCtrlPropPage::SetCorrectDblMinMaxBackColor( CExtNumEdit *pclMin, CExtNumEdit *pclMax, bool bReset )
{
	// Reset to a white background.
	if( true == bReset )
	{
		pclMin->SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		pclMax->SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
	}
	
	// If 'pclMin' is upper than 'pclMax', set a background color to red.
	// Otherwise, the background color stay white.
	double dValMin, dValMax;
	ReadDouble( *pclMin, &dValMin );
	ReadDouble( *pclMax, &dValMax );

	if( dValMin > dValMax )
	{
		pclMin->SetBackColor( _RED );
		pclMax->SetBackColor( _RED );
	}
}
