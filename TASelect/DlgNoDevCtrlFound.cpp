#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Select.h"
#include "ProductSelectionParameters.h"
#include "DlgNoDevCtrlFound.h"

CDlgNoDevCtrlFoundParams::CDlgNoDevCtrlFoundParams()
{
	m_pList = NULL;
	m_strTypeID = _T("");
	m_eCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvNU;
	m_strFamilyID = _T("");
	m_strMaterialID = _T("");
	m_strConnectID = _T("");
	m_strVersionID = _T("");
	m_strPNID = _T("");
}

CDlgNoDevCtrlFoundParams::CDlgNoDevCtrlFoundParams( CRankEx *pList, CString strTypeID, CDB_ControlProperties::CvCtrlType eCvCtrlType, CString strFamilyID,
	CString strMaterialID, CString strConnectID, CString strVersionID, CString strPNID )
{
	m_pList = pList;
	m_strTypeID = strTypeID;
	m_eCvCtrlType = eCvCtrlType;
	m_strFamilyID = strFamilyID;
	m_strMaterialID = strMaterialID;
	m_strConnectID = strConnectID;
	m_strVersionID = strVersionID;
	m_strPNID = strPNID;
}

CDlgNoDevCtrlFoundParams &CDlgNoDevCtrlFoundParams::operator=( CDlgNoDevCtrlFoundParams &rNoDevCtrlFoundParams )
{
	m_pList = rNoDevCtrlFoundParams.m_pList;
	m_strTypeID = rNoDevCtrlFoundParams.m_strTypeID;
	m_eCvCtrlType = rNoDevCtrlFoundParams.m_eCvCtrlType;
	m_strFamilyID = rNoDevCtrlFoundParams.m_strFamilyID;
	m_strMaterialID = rNoDevCtrlFoundParams.m_strMaterialID;
	m_strConnectID = rNoDevCtrlFoundParams.m_strConnectID;
	m_strVersionID = rNoDevCtrlFoundParams.m_strVersionID;
	m_strPNID = rNoDevCtrlFoundParams.m_strPNID;
	return *this;
}


CDlgNoDevCtrlFound::CDlgNoDevCtrlFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevCtrlFoundParams *pclNoDevCtrlFoundParams, CWnd *pParent )
	: CDialogEx( CDlgNoDevCtrlFound::IDD, pParent )
{
	m_pclIndividualSelectionParams = pclIndividualSelectionParams;
	
	if( NULL != pclNoDevCtrlFoundParams )
	{
		m_clNoDevCtrlFoundParams = *pclNoDevCtrlFoundParams;
	}
}

BEGIN_MESSAGE_MAP( CDlgNoDevCtrlFound, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOCTRLTYPE, OnCbnSelChangeCtrlType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL, OnCbnSelChangeBodyMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
END_MESSAGE_MAP()

void CDlgNoDevCtrlFound::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOCTRLTYPE, m_ComboCVCtrlType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboBodyMaterial );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );	
}

BOOL CDlgNoDevCtrlFound::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICNODEV );
	GetDlgItem( IDC_STATICNODEV )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICDEVEXIST );
	GetDlgItem( IDC_STATICDEVEXIST )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICCTRLTYPE );
	GetDlgItem( IDC_STATICCTRLTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICMATERIAL );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVCTRLFOUND_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Fill the combo boxes.
	_FillComboType( m_clNoDevCtrlFoundParams.m_strTypeID );
	_FillComboCtrlType( m_clNoDevCtrlFoundParams.m_eCvCtrlType );
	_FillComboFamily( m_clNoDevCtrlFoundParams.m_strFamilyID );
	_FillComboBodyMat( m_clNoDevCtrlFoundParams.m_strMaterialID );
	_FillComboConnect( m_clNoDevCtrlFoundParams.m_strConnectID );
	_FillComboVersion( m_clNoDevCtrlFoundParams.m_strVersionID );
	_FillComboPN( m_clNoDevCtrlFoundParams.m_strPNID );

	return TRUE;
}

void CDlgNoDevCtrlFound::OnOK() 
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgNoDevCtrlFound::OnCbnSelChangeType()
{
	m_clNoDevCtrlFoundParams.m_strTypeID = m_ComboType.GetCBCurSelIDPtr().ID;

	_FillComboCtrlType();
	OnCbnSelChangeCtrlType();
}

void CDlgNoDevCtrlFound::OnCbnSelChangeCtrlType()
{
    m_clNoDevCtrlFoundParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)(LPARAM)m_ComboCVCtrlType.GetItemDataPtr( m_ComboCVCtrlType.GetCurSel() );

	_FillComboFamily();
	OnCbnSelChangeFamily();
}

void CDlgNoDevCtrlFound::OnCbnSelChangeFamily()
{
	m_clNoDevCtrlFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;

	_FillComboBodyMat();
	OnCbnSelChangeBodyMaterial();
}

void CDlgNoDevCtrlFound::OnCbnSelChangeBodyMaterial()
{
	m_clNoDevCtrlFoundParams.m_strMaterialID = m_ComboBodyMaterial.GetCBCurSelIDPtr().ID;

	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgNoDevCtrlFound::OnCbnSelChangeConnect()
{
	m_clNoDevCtrlFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;

	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgNoDevCtrlFound::OnCbnSelChangeVersion()
{
	m_clNoDevCtrlFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;

	_FillComboPN();
}

void CDlgNoDevCtrlFound::OnCbnSelChangePN()
{
	m_clNoDevCtrlFoundParams.m_strPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevCtrlFound::_FillComboType( CString strTypeID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx TypeList;

	m_pclIndividualSelectionParams->m_pTADB->GetTaCVTypeList( m_clNoDevCtrlFoundParams.m_pList, &TypeList );
	m_ComboType.FillInCombo( &TypeList, strTypeID, IDS_COMBOTEXT_ALL_TYPES );

	if( false == m_pclIndividualSelectionParams->m_strComboTypeID.IsEmpty() && m_ComboType.GetCount() > 1 && 0 == m_ComboType.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboType.SetCurSel( 1 );
	}

	m_clNoDevCtrlFoundParams.m_strTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevCtrlFound::_FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx CtrlTypeList;
	
	m_pclIndividualSelectionParams->m_pTADB->GetTACVCtrlTypeList( m_clNoDevCtrlFoundParams.m_pList, &CtrlTypeList, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strTypeID );
	
	m_ComboCVCtrlType.ResetContent();
	CtrlTypeList.Transfer( &m_ComboCVCtrlType );
	ASSERT( 0 != m_ComboCVCtrlType.GetCount() );

	int iSelPos = 0;
	CDB_ControlProperties::CvCtrlType eSelectedCvCtrlType = CDB_ControlProperties::CvCtrlType::eCvNU;

	for( int i = 0; i < m_ComboCVCtrlType.GetCount(); i++ )
	{
        if( eCvCtrlType == ( CDB_ControlProperties::CvCtrlType )m_ComboCVCtrlType.GetItemData( i ) )
		{
			eSelectedCvCtrlType = eCvCtrlType;
			iSelPos = i;
			break;
		}
	}

	m_ComboCVCtrlType.SetCurSel( iSelPos );

	if( m_ComboCVCtrlType.GetCount() <= 1 )
	{
		m_ComboCVCtrlType.EnableWindow( false );
	}
	else
	{
		m_ComboCVCtrlType.EnableWindow( true );
	}

    m_clNoDevCtrlFoundParams.m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)(LPARAM)m_ComboCVCtrlType.GetItemDataPtr( m_ComboCVCtrlType.GetCurSel() );
}

void CDlgNoDevCtrlFound::_FillComboFamily( CString strFamilyID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx FamilyList;

	m_pclIndividualSelectionParams->m_pTADB->GetTACVFamList( m_clNoDevCtrlFoundParams.m_pList, &FamilyList, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strTypeID, 
			m_clNoDevCtrlFoundParams.m_eCvCtrlType );

	m_ComboFamily.FillInCombo( &FamilyList, strFamilyID, IDS_COMBOTEXT_ALL_FAMILIES );

	if( false == m_pclIndividualSelectionParams->m_strComboFamilyID.IsEmpty() && m_ComboFamily.GetCount() > 1 && 0 == m_ComboFamily.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboFamily.SetCurSel( 1 );
	}

	m_clNoDevCtrlFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevCtrlFound::_FillComboBodyMat( CString strBodyMaterialID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx BodyMaterialList;

	m_pclIndividualSelectionParams->m_pTADB->GetTaCVBdyMatList( m_clNoDevCtrlFoundParams.m_pList, &BodyMaterialList, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strTypeID, 
			(LPCTSTR)m_clNoDevCtrlFoundParams.m_strFamilyID, m_clNoDevCtrlFoundParams.m_eCvCtrlType );

	m_ComboBodyMaterial.FillInCombo( &BodyMaterialList, strBodyMaterialID, IDS_COMBOTEXT_ALL_MATERIALS );

	if( false == m_pclIndividualSelectionParams->m_strComboMaterialID.IsEmpty() && m_ComboBodyMaterial.GetCount() > 1 && 0 == m_ComboBodyMaterial.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboBodyMaterial.SetCurSel( 1 );
	}

	m_clNoDevCtrlFoundParams.m_strMaterialID = m_ComboBodyMaterial.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevCtrlFound::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{

		ASSERT_RETURN;
	}

	CRankEx ConnectList;

	m_pclIndividualSelectionParams->m_pTADB->GetTaCVConnList( m_clNoDevCtrlFoundParams.m_pList, &ConnectList, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strTypeID,
		(LPCTSTR)m_clNoDevCtrlFoundParams.m_strFamilyID, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strMaterialID, m_clNoDevCtrlFoundParams.m_eCvCtrlType );

	m_ComboConnect.FillInCombo( &ConnectList, strConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboConnectID.IsEmpty() && m_ComboConnect.GetCount() > 1 && 0 == m_ComboConnect.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboConnect.SetCurSel( 1 );
	}

	m_clNoDevCtrlFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevCtrlFound::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_pclIndividualSelectionParams->m_pTADB->GetTaCVVersList( m_clNoDevCtrlFoundParams.m_pList, &VersionList, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strTypeID, 
			(LPCTSTR)m_clNoDevCtrlFoundParams.m_strFamilyID, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strMaterialID, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strConnectID, 
			m_clNoDevCtrlFoundParams.m_eCvCtrlType );

	m_ComboVersion.FillInCombo( &VersionList, strVersionID, IDS_COMBOTEXT_ALL_VERSIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboVersionID.IsEmpty() && m_ComboVersion.GetCount() > 1 && 0 == m_ComboVersion.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboVersion.SetCurSel( 1 );
	}

	m_clNoDevCtrlFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevCtrlFound::_FillComboPN( CString strPNID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx PNList;

	m_pclIndividualSelectionParams->m_pTADB->GetTaCVPNList( m_clNoDevCtrlFoundParams.m_pList, &PNList, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strTypeID, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strFamilyID, 
			(LPCTSTR)m_clNoDevCtrlFoundParams.m_strMaterialID, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strConnectID, (LPCTSTR)m_clNoDevCtrlFoundParams.m_strVersionID, m_clNoDevCtrlFoundParams.m_eCvCtrlType );

	m_ComboPN.FillInCombo( &PNList, strPNID, IDS_COMBOTEXT_ALL );

	if( false == m_pclIndividualSelectionParams->m_strComboPNID.IsEmpty() && m_ComboPN.GetCount() > 1 && 0 == m_ComboPN.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboPN.SetCurSel( 1 );
	}

	m_clNoDevCtrlFoundParams.m_strPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}
