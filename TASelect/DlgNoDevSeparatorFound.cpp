#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "TASelectDoc.h"
#include "Select.h"
#include "DlgNoDevSeparatorFound.h"
#include "ProductSelectionParameters.h"

CDlgNoDevSeparatorFoundParams::CDlgNoDevSeparatorFoundParams()
{
	m_pList = NULL;
	m_strTypeID = _T("");
	m_strFamilyID = _T("");
	m_strConnectID = _T("");
	m_strVersionID = _T("");
}

CDlgNoDevSeparatorFoundParams::CDlgNoDevSeparatorFoundParams( CRankEx *pList, CString strTypeID, CString strFamilyID, CString strConnectID, CString strVersionID )
{
	m_pList = pList;
	m_strTypeID = strTypeID;
	m_strFamilyID = strFamilyID;
	m_strConnectID = strConnectID;
	m_strVersionID = strVersionID;
}

CDlgNoDevSeparatorFoundParams &CDlgNoDevSeparatorFoundParams::operator=( CDlgNoDevSeparatorFoundParams &rNoDevSeparatorFoundParams )
{
	m_pList = rNoDevSeparatorFoundParams.m_pList;
	m_strTypeID = rNoDevSeparatorFoundParams.m_strTypeID;
	m_strFamilyID = rNoDevSeparatorFoundParams.m_strFamilyID;
	m_strConnectID = rNoDevSeparatorFoundParams.m_strConnectID;
	m_strVersionID = rNoDevSeparatorFoundParams.m_strVersionID;
	return *this;
}

CDlgNoDevSeparatorFound::CDlgNoDevSeparatorFound( CIndividualSelectionParameters *pclIndividualSelectionParams, CDlgNoDevSeparatorFoundParams *pclNoDevSeparatorFoundParams,
		CWnd *pParent )
	: CDialogEx( CDlgNoDevSeparatorFound::IDD, pParent )
{
	m_pclIndividualSelectionParams = pclIndividualSelectionParams;

	if( NULL != pclNoDevSeparatorFoundParams )
	{
		m_clNoDevSeparatorFoundParams = *pclNoDevSeparatorFoundParams;
	}
}

BEGIN_MESSAGE_MAP( CDlgNoDevSeparatorFound, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
END_MESSAGE_MAP()

void CDlgNoDevSeparatorFound::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
}

BOOL CDlgNoDevSeparatorFound::OnInitDialog() 
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
	
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSEPFOUND_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSEPFOUND_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSEPFOUND_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGNODEVSEPFOUND_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Fill the combo boxes.
	_FillComboType( m_clNoDevSeparatorFoundParams.m_strTypeID );
	_FillComboFamily( m_clNoDevSeparatorFoundParams.m_strFamilyID );
	_FillComboConnect( m_clNoDevSeparatorFoundParams.m_strConnectID );
	_FillComboVersion( m_clNoDevSeparatorFoundParams.m_strVersionID );
	
	return TRUE; 
}

void CDlgNoDevSeparatorFound::OnOK() 
{
	PREVENT_ENTER_KEY
	CDialogEx::OnOK();
}

void CDlgNoDevSeparatorFound::OnCbnSelChangeType()
{
	m_clNoDevSeparatorFoundParams.m_strTypeID = m_ComboType.GetCBCurSelIDPtr().ID;

	_FillComboFamily();
	OnCbnSelChangeFamily();
}

void CDlgNoDevSeparatorFound::OnCbnSelChangeFamily()
{
	m_clNoDevSeparatorFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;

	_FillComboConnect();
	OnCbnSelChangeConnect();
}

void CDlgNoDevSeparatorFound::OnCbnSelChangeConnect()
{
	m_clNoDevSeparatorFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;

	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgNoDevSeparatorFound::OnCbnSelChangeVersion()
{
	m_clNoDevSeparatorFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSeparatorFound::_FillComboType( CString strTypeID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx TypeList;

	m_pclIndividualSelectionParams->m_pTADB->GetSeparatorTypeList( m_clNoDevSeparatorFoundParams.m_pList, &TypeList );

	m_ComboType.FillInCombo( &TypeList, strTypeID, IDS_COMBOTEXT_ALL_TYPES );

	if( false == m_pclIndividualSelectionParams->m_strComboTypeID.IsEmpty() && m_ComboType.GetCount() > 1 && 0 == m_ComboType.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboType.SetCurSel( 1 );
	}

	m_clNoDevSeparatorFoundParams.m_strTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSeparatorFound::_FillComboFamily( CString strFamilyID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx FamilyList;

	m_pclIndividualSelectionParams->m_pTADB->GetSeparatorFamilyList( m_clNoDevSeparatorFoundParams.m_pList, &FamilyList, 
			(LPCTSTR)m_clNoDevSeparatorFoundParams.m_strTypeID );

	m_ComboFamily.FillInCombo( &FamilyList, strFamilyID, IDS_COMBOTEXT_ALL_FAMILIES );

	if( false == m_pclIndividualSelectionParams->m_strComboFamilyID.IsEmpty() && m_ComboFamily.GetCount() > 1 && 0 == m_ComboFamily.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboFamily.SetCurSel( 1 );
	}

	m_clNoDevSeparatorFoundParams.m_strFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSeparatorFound::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx ConnectList;

	m_pclIndividualSelectionParams->m_pTADB->GetSeparatorConnList( m_clNoDevSeparatorFoundParams.m_pList, &ConnectList, 
			(LPCTSTR)m_clNoDevSeparatorFoundParams.m_strTypeID, (LPCTSTR)m_clNoDevSeparatorFoundParams.m_strFamilyID );

	m_ComboConnect.FillInCombo( &ConnectList, strConnectID, IDS_COMBOTEXT_ALL_CONNECTIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboConnectID.IsEmpty() && m_ComboConnect.GetCount() > 1 && 0 == m_ComboConnect.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboConnect.SetCurSel( 1 );
	}

	m_clNoDevSeparatorFoundParams.m_strConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgNoDevSeparatorFound::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_pclIndividualSelectionParams || NULL == m_pclIndividualSelectionParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx VersionList;

	m_pclIndividualSelectionParams->m_pTADB->GetSeparatorVersList( m_clNoDevSeparatorFoundParams.m_pList, &VersionList, 
			(LPCTSTR)m_clNoDevSeparatorFoundParams.m_strTypeID, (LPCTSTR)m_clNoDevSeparatorFoundParams.m_strFamilyID, 
			(LPCTSTR)m_clNoDevSeparatorFoundParams.m_strConnectID );

	m_ComboVersion.FillInCombo( &VersionList, strVersionID, IDS_COMBOTEXT_ALL_VERSIONS );

	if( false == m_pclIndividualSelectionParams->m_strComboVersionID.IsEmpty() && m_ComboVersion.GetCount() > 1 && 0 == m_ComboVersion.GetCurSel() )
	{
		// If there is more than one item in the combo and the current selection is on the "*** All xxx ***" item, 
		// we force the selection to be on the second item.
		m_ComboVersion.SetCurSel( 1 );
	}

	m_clNoDevSeparatorFoundParams.m_strVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}
