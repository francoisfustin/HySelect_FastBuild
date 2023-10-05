#include "stdafx.h"
#include "TASelect.h"
#include "HydronicPic.h"
#include "DlgTechParam.h"
#include "DlgTPGeneral.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneral, CDlgCtrlPropPage )

CDlgTPGeneral::CDlgTPGeneral( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHG_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHG_PAGENAME );
}

void CDlgTPGeneral::Init( bool fResetToDefault )
{
	// In case the user click on the default button it's only for CDS_TechParam and not for CDS_PersistData.
	if( true == fResetToDefault )
	{
		return;
	}

	// Verify Local prod pics dll exist in the current folder.
	if( false == TASApp.IsLocalProdPicDllLoaded() )
	{
		m_RadioLocSymb.EnableWindow( FALSE );
	}

	// Set the radio button according to the registry or use TA image by default.
	UINT uiImgType = ::AfxGetApp()->GetProfileInt( REGISTRYSECTIONNAME_DLGTECHPARAM, REGISTRYKEYNAME_VALVEIMAGETYPE, CProdPic::Pic );
	if( CProdPic::Pic == uiImgType )
	{
		m_RadioTAImg.SetCheck( BST_CHECKED );
	}
	else if( CProdPic::PicTASymb == uiImgType )
	{
		m_RadioHVACSymb.SetCheck( BST_CHECKED );
	}
	else if( CProdPic::PicLocSymb == uiImgType && true == TASApp.IsLocalProdPicDllLoaded() )
	{
		m_RadioLocSymb.SetCheck( BST_CHECKED );
	}
	else
	{
		m_RadioTAImg.SetCheck( BST_CHECKED );
	}

}

void CDlgTPGeneral::Save( CString strSectionName )
{
	if ( NULL != GetSafeHwnd() )
	{
		if( BST_CHECKED == m_RadioTAImg.GetCheck() )
		{
			TASApp.SetProdPicType( CProdPic::Pic );
		}
		else if( BST_CHECKED == m_RadioHVACSymb.GetCheck() )
		{
			TASApp.SetProdPicType( CProdPic::PicTASymb );
		}
		else if( BST_CHECKED == m_RadioLocSymb.GetCheck() )
		{
			TASApp.SetProdPicType( CProdPic::PicLocSymb );
		}
	}
}

void CDlgTPGeneral::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_RADIOTAIMG, m_RadioTAImg );
	DDX_Control( pDX, IDC_RADIOHVACSYMB, m_RadioHVACSymb );
	DDX_Control( pDX, IDC_RADIOLOCSYMB, m_RadioLocSymb );
	DDX_Control( pDX, IDC_STATICSCHEMECHOICE, m_GroupSchemeChoice );
}

BOOL CDlgTPGeneral::OnInitDialog() 
{	
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}
	
	// Initialize dialog strings.
	CString str;	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHGEN_SCHEMECHOICE );
	GetDlgItem( IDC_STATICSCHEMECHOICE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHGEN_RADIOIMG );
	GetDlgItem( IDC_RADIOTAIMG )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHGEN_RADIOHVACSYMB );
	GetDlgItem( IDC_RADIOHVACSYMB )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHGEN_RADIOLOCSYMB );
	GetDlgItem( IDC_RADIOLOCSYMB )->SetWindowText( str );

	// Define the control style for the group box.
	m_GroupSchemeChoice.SetControlStyle( CXGroupBox::CONTROL_STYLE::header, FALSE );
	m_GroupSchemeChoice.SetBckgndColor( _WHITE_DLGBOX, FALSE );
	
	Init();

	return TRUE;
}
