#include "stdafx.h"
#include "TASelect.h"
#include "Utilities.h"
#include "MyToolTipCtrl.h"
#include "HydroMod.h"

#include "DlgPasteHM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgPasteHM::CDlgPasteHM( CWnd *pParent )
	: CDialogEx(CDlgPasteHM::IDD, pParent)
{
	m_pTADS = NULL;
	m_pHM = NULL;
	m_pTabTo = NULL;
	m_pIndex = NULL;
}

int	CDlgPasteHM::Display(CDS_HydroMod *pHM,	CTable **pTabTo, int *pIndex)
{
	m_pHM = pHM;
	m_pTabTo = pTabTo;
	m_pIndex = pIndex;
	return DoModal();
}

BEGIN_MESSAGE_MAP( CDlgPasteHM, CDialogEx )
	ON_BN_CLICKED( IDC_RADIOPOS, OnRadioClickedInsertInPosition )
	ON_BN_CLICKED( IDC_RADIOPV, OnRadioClickedPartnerValve )
	ON_CBN_SELCHANGE( IDC_COMBO, OnCbnSelChangePartnerValve )
END_MESSAGE_MAP()

void CDlgPasteHM::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_SPIN, m_Spin );
	DDX_Control( pDX, IDC_EDITPOS, m_EditPos );
	DDX_Control( pDX, IDC_COMBO, m_Combo );
}

BOOL CDlgPasteHM::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	m_pTADS = TASApp.GetpTADS();

	// Initialize all strings.
	CString str; 
	str = TASApp.LoadLocalizedString( IDS_DLGPASTEHM_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPASTEHM_STATIC1 );
	
	CString str1;
	
	if( false == (*m_pTabTo)->IsClass( CLASS( CTableHM ) ) )
	{
		str1 = ((CDS_HydroMod *)*m_pTabTo)->GetHMName();
	}
	else
	{
		str1 = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
	}

	FormatString( str, IDS_DLGPASTEHM_STATIC1, m_pHM->GetHMName(), str1 );
	SetDlgItemText( IDC_STATIC1, str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPASTEHM_RADIOPOS );
	SetDlgItemText( IDC_RADIOPOS, str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPASTEHM_RADIOPV );
	SetDlgItemText( IDC_RADIOPV, str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	SetDlgItemText( IDOK, str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	SetDlgItemText( IDCANCEL, str );

	m_ToolTip.DestroyWindow();
	m_ToolTip.Create( this, TTS_NOPREFIX );
	
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPASTEHM_TT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_RADIOPV ), TTstr );
	
	TTstr = TASApp.LoadLocalizedString( IDS_DLGPASTEHM_TT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_COMBO ), TTstr );
	
	( (CButton *)GetDlgItem( IDC_RADIOPOS ) )->SetCheck( 1 );
	m_Spin.SetRange( 1, *m_pIndex );
	m_Spin.SetPos( *m_pIndex );
	OnRadioClickedInsertInPosition();

	CRank PVList;

	// If the copied module contains a PV you can't select a valve to attach him.
	if( _T('\0') == *m_pHM->GetCBIValveID() )
	{
		for( IDPTR IDPtr = (*m_pTabTo)->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = (*m_pTabTo)->GetNext() )
		{
			if( false == ( (CDS_HydroMod *)IDPtr.MP )->IsaModule() )
			{
				PVList.Add( ( (CDS_HydroMod *)IDPtr.MP )->GetHMName(), ( (CDS_HydroMod *)IDPtr.MP )->GetPos(), (LPARAM)IDPtr.MP );
			}
		}
	}

	PVList.Transfer( &m_Combo );

	if( m_Combo.GetCount() > 0 )
	{
		m_Combo.SetCurSel( 0 );
	}
	else
	{
		( (CButton *)GetDlgItem( IDC_RADIOPV ) )->EnableWindow( FALSE );
	}

	return TRUE;
}

void CDlgPasteHM::OnOK() 
{
	*m_pIndex = m_Spin.GetPos();
	
	if( BST_CHECKED == ( (CButton *)GetDlgItem( IDC_RADIOPV ) )->GetCheck() )
	{	
		// A valve will be converted into a partner valve keep the same index.
		if( m_Combo.GetCurSel() >= 0 )
		{
			*m_pTabTo = (CTable *)m_Combo.GetItemDataPtr( m_Combo.GetCurSel() );
		}
	}

	CDialogEx::OnOK();
}

void CDlgPasteHM::OnRadioClickedInsertInPosition() 
{
	if( BST_CHECKED == ( (CButton *)GetDlgItem( IDC_RADIOPOS ) )->GetCheck() )
	{
		m_EditPos.EnableWindow( TRUE );
		m_Spin.EnableWindow( TRUE );
		m_Combo.EnableWindow( FALSE );
	}
	else
	{
		if( 0 == m_Combo.GetCount() )
		{
			return;
		}

		m_EditPos.EnableWindow( FALSE );
		m_Spin.EnableWindow( FALSE );
		m_Combo.EnableWindow( TRUE );
	}
}

void CDlgPasteHM::OnRadioClickedPartnerValve() 
{
	if( BST_CHECKED == ( (CButton *)GetDlgItem( IDC_RADIOPOS ) )->GetCheck() )
	{
		m_EditPos.EnableWindow( TRUE );
		m_Spin.EnableWindow( TRUE );
		m_Combo.EnableWindow( FALSE );
	}
	else
	{
		m_EditPos.EnableWindow( FALSE );
		m_Spin.EnableWindow( FALSE );
		m_Combo.EnableWindow( TRUE );
	}
}

void CDlgPasteHM::OnCbnSelChangePartnerValve() 
{
	if( m_Combo.GetCurSel() < 0 )
	{
		return;
	}
	
	// Update Spin position in function of valve's index.
	CDS_HydroMod *pHM = (CDS_HydroMod *)( m_Combo.GetItemDataPtr( m_Combo.GetCurSel() ) );

	if( NULL != pHM )
	{
		m_Spin.SetPos( pHM->GetPos() );
	}
}
