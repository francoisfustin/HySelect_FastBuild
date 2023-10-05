#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "TASelectDoc.h"
#include "Select.h"
#include "DlgNoDevFound.h"
#include "ProductSelectionParameters.h"

CDlgNoDevFoundParams::CDlgNoDevFoundParams()
{
	m_pList = NULL;
	m_strTypeID = _T("");
	m_strFamilyID = _T("");
	m_strMaterialID = _T("");
	m_strConnectID = _T("");
	m_strVersionID = _T("");
	m_strPNID = _T("");
}

CDlgNoDevFoundParams::CDlgNoDevFoundParams( CRankEx *pList, CString strTypeID, CString strFamilyID, CString strMaterialID, CString strConnectID, 
		CString strVersionID, CString strPNID )
{
	m_pList = pList;
	m_strTypeID = strTypeID;
	m_strFamilyID = strFamilyID;
	m_strMaterialID = strMaterialID;
	m_strConnectID = strConnectID;
	m_strVersionID = strVersionID;
	m_strPNID = strPNID;
}

CDlgNoDevFoundParams &CDlgNoDevFoundParams::operator=( CDlgNoDevFoundParams &rNoDevFoundParams )
{
	m_pList = rNoDevFoundParams.m_pList;
	m_strTypeID = rNoDevFoundParams.m_strTypeID;
	m_strFamilyID = rNoDevFoundParams.m_strFamilyID;
	m_strMaterialID = rNoDevFoundParams.m_strMaterialID;
	m_strConnectID = rNoDevFoundParams.m_strConnectID;
	m_strVersionID = rNoDevFoundParams.m_strVersionID;
	m_strPNID = rNoDevFoundParams.m_strPNID;
	return *this;
}

CDlgNoDevFound::CDlgNoDevFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevFoundParams *pclNoDevFoundParams, CWnd *pParent )
	: CDialogEx( CDlgNoDevFound::IDD, pParent )
{
	m_pclIndividualSelectionParams = pclIndividualSelectionParams;

	if( NULL != pclNoDevFoundParams )
	{
		m_clNoDevFoundParams = *pclNoDevFoundParams;
	}
}

BEGIN_MESSAGE_MAP( CDlgNoDevFound, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY,OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL,OnCbnSelChangeMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
END_MESSAGE_MAP()

void CDlgNoDevFound::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOMATERIAL, m_ComboMaterial );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
}

BOOL CDlgNoDevFound::OnInitDialog() 
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
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVDPCFOUND_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICBODYMAT );
	GetDlgItem( IDC_STATICMATERIAL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVFOUND_STATICPN );
	GetDlgItem( IDC_STATICPN )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem(IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem(IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Fill the combo boxes.
	_FillComboType( m_clNoDevFoundParams.m_strTypeID );
	_FillComboFamily( m_clNoDevFoundParams.m_strFamilyID );
	_FillComboBodyMat( m_clNoDevFoundParams.m_strMaterialID );
	_FillComboConnect( m_clNoDevFoundParams.m_strConnectID );
	_FillComboVersion( m_clNoDevFoundParams.m_strVersionID );
	_FillComboPN( m_clNoDevFoundParams.m_strPNID );
	
	return TRUE;
}

void CDlgNoDevFound::OnOK() 
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgNoDevFound::OnCbnSelChangeType()
{
	m_clNoDevFoundParams.m_strTypeID = m_ComboType.GetCBCurSelIDPtr().ID;

	_FillComboFamily();
	OnCbnSelChangeFamily();
}

void CDlgNoDevFound::OnCbnSelChangeFamily()
{
	m_clNoDevFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;

	_FillComboBodyMat();
	OnCbnSelChangeMaterial();
}

void CDlgNoDevFound::OnCbnSelChangeMaterial()
{
	m_clNoDevFoundParams.m_strMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;

	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgNoDevFound::OnCbnSelChangeConnect() 
{
	m_clNoDevFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;

	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgNoDevFound::OnCbnSelChangeVersion() 
{
	m_clNoDevFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;

	_FillComboPN();
}

void CDlgNoDevFound::OnCbnSelChangePN()
{
	m_clNoDevFoundParams.m_strPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevFound::_FillComboType( CString strTypeID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}
	
	CRankEx TypeList;

	m_pclIndividualSelectionParams->m_pTADB->GetBVTypeList( m_clNoDevFoundParams.m_pList, &TypeList );

	m_ComboType.FillInCombo( &TypeList, strTypeID, IDS_COMBOTEXT_ALL_TYPES );

	if( false == m_pclIndividualSelectionParams->m_strComboTypeID.IsEmpty() && m_ComboType.GetCount() > 1 && 0 == m_ComboType.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboType.SetCurSel( 1 );
	}

	m_clNoDevFoundParams.m_strTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevFound::_FillComboFamily( CString strFamilyID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}
	
	CRankEx FamilyList;

	m_pclIndividualSelectionParams->m_pTADB->GetBVFamilyList( m_clNoDevFoundParams.m_pList, &FamilyList, (LPCTSTR)m_clNoDevFoundParams.m_strTypeID );

	m_ComboFamily.FillInCombo( &FamilyList, strFamilyID, IDS_COMBOTEXT_ALL_FAMILIES );

	if( false == m_pclIndividualSelectionParams->m_strComboFamilyID.IsEmpty() && m_ComboFamily.GetCount() > 1 && 0 == m_ComboFamily.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboFamily.SetCurSel( 1 );
	}

	m_clNoDevFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevFound::_FillComboBodyMat( CString strBodyMaterialID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx BodyMaterialList;

	m_pclIndividualSelectionParams->m_pTADB->GetBVBdyMatList( m_clNoDevFoundParams.m_pList, &BodyMaterialList, (LPCTSTR)m_clNoDevFoundParams.m_strTypeID, 
			(LPCTSTR)m_clNoDevFoundParams.m_strFamilyID );

	m_ComboMaterial.FillInCombo( &BodyMaterialList, strBodyMaterialID, IDS_COMBOTEXT_ALL_MATERIALS );

	if( false == m_pclIndividualSelectionParams->m_strComboMaterialID.IsEmpty() && m_ComboMaterial.GetCount() > 1 && 0 == m_ComboMaterial.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboMaterial.SetCurSel( 1 );
	}

	m_clNoDevFoundParams.m_strMaterialID = m_ComboMaterial.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevFound::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx ConnectList;

	m_pclIndividualSelectionParams->m_pTADB->GetBVConnList( m_clNoDevFoundParams.m_pList, &ConnectList, (LPCTSTR)m_clNoDevFoundParams.m_strTypeID, 
			(LPCTSTR)m_clNoDevFoundParams.m_strFamilyID, (LPCTSTR)m_clNoDevFoundParams.m_strMaterialID );

	m_ComboConnect.FillInCombo( &ConnectList, strConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboConnectID.IsEmpty() && m_ComboConnect.GetCount() > 1 && 0 == m_ComboConnect.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboConnect.SetCurSel( 1 );
	}

	m_clNoDevFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevFound::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_pclIndividualSelectionParams->m_pTADB->GetBVVersList( m_clNoDevFoundParams.m_pList, &VersionList, (LPCTSTR)m_clNoDevFoundParams.m_strTypeID, 
			(LPCTSTR)m_clNoDevFoundParams.m_strFamilyID, (LPCTSTR)m_clNoDevFoundParams.m_strMaterialID, (LPCTSTR)m_clNoDevFoundParams.m_strConnectID );

	m_ComboVersion.FillInCombo( &VersionList, strVersionID, IDS_COMBOTEXT_ALL_VERSIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboVersionID.IsEmpty() && m_ComboVersion.GetCount() > 1 && 0 == m_ComboVersion.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboVersion.SetCurSel( 1 );
	}

	m_clNoDevFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevFound::_FillComboPN( CString strPNID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx PNList;

	m_pclIndividualSelectionParams->m_pTADB->GetBVPNList( m_clNoDevFoundParams.m_pList, &PNList, (LPCTSTR)m_clNoDevFoundParams.m_strTypeID, 
			(LPCTSTR)m_clNoDevFoundParams.m_strFamilyID, (LPCTSTR)m_clNoDevFoundParams.m_strMaterialID, (LPCTSTR)m_clNoDevFoundParams.m_strConnectID,
			(LPCTSTR)m_clNoDevFoundParams.m_strVersionID );

	m_ComboPN.FillInCombo( &PNList, strPNID, IDS_COMBOTEXT_ALL );

	if( false == m_pclIndividualSelectionParams->m_strComboPNID.IsEmpty() && m_ComboPN.GetCount() > 1 && 0 == m_ComboPN.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboPN.SetCurSel( 1 );
	}

	m_clNoDevFoundParams.m_strPNID = m_ComboPN.GetCBCurSelIDPtr().ID;
}
