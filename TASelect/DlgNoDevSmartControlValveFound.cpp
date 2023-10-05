#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "TASelectDoc.h"
#include "Select.h"
#include "DlgNoDevSmartControlValveFound.h"
#include "ProductSelectionParameters.h"

CDlgNoDevSmartControlValveFoundParams::CDlgNoDevSmartControlValveFoundParams()
{
	m_pList = NULL;
	m_strBodyMaterialID = _T("");
	m_strConnectID = _T("");
	m_strPNID = _T("");
}

CDlgNoDevSmartControlValveFoundParams::CDlgNoDevSmartControlValveFoundParams( CRankEx *pList, CString strBodyMaterialID, CString strConnectID, CString strPNID )
{
	m_pList = pList;
	m_strBodyMaterialID = strBodyMaterialID;
	m_strConnectID = strConnectID;
	m_strPNID = strPNID;
}

CDlgNoDevSmartControlValveFoundParams &CDlgNoDevSmartControlValveFoundParams::operator=( CDlgNoDevSmartControlValveFoundParams &rNoDevSmartControlValveFoundParams )
{
	m_pList = rNoDevSmartControlValveFoundParams.m_pList;
	m_strBodyMaterialID = rNoDevSmartControlValveFoundParams.m_strBodyMaterialID;
	m_strConnectID = rNoDevSmartControlValveFoundParams.m_strConnectID;
	m_strPNID = rNoDevSmartControlValveFoundParams.m_strPNID;
	return *this;
}

CDlgNoDevSmartControlValveFound::CDlgNoDevSmartControlValveFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevSmartControlValveFoundParams *pclNoDevSeparatorFoundParams,
		CWnd *pParent )
	: CDialogEx( CDlgNoDevSmartControlValveFound::IDD, pParent )
{
	m_pclIndividualSelectionParams = pclIndividualSelectionParams;

	if( NULL != pclNoDevSeparatorFoundParams )
	{
		m_clNoDevSmartControlValveFoundParams = *pclNoDevSeparatorFoundParams;
	}
}

BEGIN_MESSAGE_MAP( CDlgNoDevSmartControlValveFound, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOBODYMATERIAL, OnCbnSelChangeBodyMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
END_MESSAGE_MAP()

void CDlgNoDevSmartControlValveFound::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOBODYMATERIAL, m_ComboBodyMaterial );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN);
}

BOOL CDlgNoDevSmartControlValveFound::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICNODEV );
	GetDlgItem( IDC_STATICNODEV )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICDEVEXIST );
	GetDlgItem( IDC_STATICDEVEXIST )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSMARTCONTROLVALVEFOUND_STATICBODYMATERIAL );
	GetDlgItem( IDC_STATICBODYMATERIAL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSMARTCONTROLVALVEFOUND_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSMARTCONTROLVALVEFOUND_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Fill the combo boxes.
	_FillComboBodyMaterial( m_clNoDevSmartControlValveFoundParams.m_strBodyMaterialID );
	_FillComboConnect( m_clNoDevSmartControlValveFoundParams.m_strConnectID );
	_FillComboPN( m_clNoDevSmartControlValveFoundParams.m_strPNID );
	
	return TRUE; 
}

void CDlgNoDevSmartControlValveFound::OnOK() 
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgNoDevSmartControlValveFound::OnCbnSelChangeBodyMaterial()
{
	m_clNoDevSmartControlValveFoundParams.m_strBodyMaterialID = m_ComboBodyMaterial.GetCBCurSelIDPtr().ID;

	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgNoDevSmartControlValveFound::OnCbnSelChangeConnect()
{
	m_clNoDevSmartControlValveFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;

	_FillComboPN();
	OnCbnSelChangePN();
}

void CDlgNoDevSmartControlValveFound::OnCbnSelChangePN()
{
	m_clNoDevSmartControlValveFoundParams.m_strPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSmartControlValveFound::_FillComboBodyMaterial( CString strBodyMaterialID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx BodyMaterialList;

	m_pclIndividualSelectionParams->m_pTADB->GetSmartControlValveBdyMatList( m_clNoDevSmartControlValveFoundParams.m_pList, &BodyMaterialList, _T(""), _T("") );

	m_ComboBodyMaterial.FillInCombo( &BodyMaterialList, strBodyMaterialID, IDS_COMBOTEXT_ALL_MATERIALS );

	if( false == m_pclIndividualSelectionParams->m_strComboMaterialID.IsEmpty() && m_ComboBodyMaterial.GetCount() > 1 && 0 == m_ComboBodyMaterial.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboBodyMaterial.SetCurSel( 1 );
	}

	m_clNoDevSmartControlValveFoundParams.m_strBodyMaterialID = m_ComboBodyMaterial.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSmartControlValveFound::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx ConnectList;

	m_pclIndividualSelectionParams->m_pTADB->GetSmartControlValveConnList( m_clNoDevSmartControlValveFoundParams.m_pList, &ConnectList, 
			_T(""), _T(""), (LPCTSTR)m_clNoDevSmartControlValveFoundParams.m_strBodyMaterialID );

	m_ComboConnect.FillInCombo( &ConnectList, strConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboConnectID.IsEmpty() && m_ComboConnect.GetCount() > 1 && 0 == m_ComboConnect.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboConnect.SetCurSel( 1 );
	}

	m_clNoDevSmartControlValveFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSmartControlValveFound::_FillComboPN( CString strPNID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_pclIndividualSelectionParams->m_pTADB->GetSmartControlValvePNList( m_clNoDevSmartControlValveFoundParams.m_pList, &VersionList, 
			_T(""), _T(""), (LPCTSTR)m_clNoDevSmartControlValveFoundParams.m_strBodyMaterialID, (LPCTSTR)m_clNoDevSmartControlValveFoundParams.m_strConnectID, _T("") );

	m_ComboPN.FillInCombo( &VersionList, strPNID, IDS_COMBOTEXT_ALL );

	if( false == m_pclIndividualSelectionParams->m_strComboPNID.IsEmpty() && m_ComboPN.GetCount() > 1 && 0 == m_ComboPN.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboPN.SetCurSel( 1 );
	}

	m_clNoDevSmartControlValveFoundParams.m_strPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}
