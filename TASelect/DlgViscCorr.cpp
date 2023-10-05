#include "stdafx.h"

#include "TASelect.h"
#include "Global.h"
#include "Utilities.h"
#include "Hydronic.h"
#include "DlgWaterChar.h"

#include "DlgViscCorr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString NoResultStr = _T(" *******");


CDlgViscCorr::CDlgViscCorr( CWnd *pParent )
	: CDialogEx( CDlgViscCorr::IDD, pParent )
{
	m_HICONAddit = NULL;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_strSelType = _T("");
	m_pPushed1 = NULL;
	m_pPushed2 = NULL;
	m_pEditIn1 = NULL;
	m_pEditIn2 = NULL;
	m_pEditOut1 = NULL;
	m_pEditOut2 = NULL;
	m_dSetting = -1.0;
	m_dDp = -1.0;
	m_dChartFlow = -1.0;
	m_dTrueFlow = -1.0;
	m_pDev = NULL;
	m_dMinOpening = -1.0;
	m_dMaxOpening = -1.0;
}

int CDlgViscCorr::Display()
{
	m_pTADB = TASApp.GetpTADB();
	
	if( NULL == m_pTADB )
	{
		ASSERTA_RETURN( IDCANCEL );
	}	
	
	m_pTADS = TASApp.GetpTADS();

	if( NULL == m_pTADS || NULL == m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( IDCANCEL );
	}	

	return DoModal();
}

BEGIN_MESSAGE_MAP( CDlgViscCorr, CDialogEx )
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED( IDC_BUTTONMODWATER, OnButtonModWater )
	ON_CBN_SELCHANGE( IDC_COMBOEXTYPE, OnSelChangeComboExType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnSelChangeComboFamily )
	ON_CBN_SELCHANGE( IDC_COMBODEVICE, OnSelChangeComboDevice )
	ON_BN_CLICKED( IDC_RADIOSETTING, OnRadioSetting )
	ON_BN_CLICKED( IDC_RADIODP, OnRadioDp )
	ON_BN_CLICKED( IDC_RADIOCHARTFLOW, OnRadioChartFlow )
	ON_BN_CLICKED( IDC_RADIOTRUEFLOW, OnRadioTrueFlow )
	ON_EN_CHANGE( IDC_EDITSETTING, OnChangeEditSetting )
	ON_EN_CHANGE( IDC_EDITDP, OnChangeEditDp )
	ON_EN_CHANGE( IDC_EDITCHARTFLOW, OnChangeEditChartFlow )
	ON_EN_CHANGE( IDC_EDITTRUEFLOW, OnChangeEditTrueFlow )
	ON_EN_KILLFOCUS( IDC_EDITSETTING, OnKillFocusEditSetting )
	ON_EN_KILLFOCUS( IDC_EDITDP, OnKillFocusEditDp )
	ON_EN_KILLFOCUS( IDC_EDITCHARTFLOW, OnKillFocusEditChartFlow )
	ON_EN_KILLFOCUS( IDC_EDITTRUEFLOW, OnKillFocusEditTrueFlow )
END_MESSAGE_MAP()

void CDlgViscCorr::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_RADIOTRUEFLOW, m_RadioTrueFlow );
	DDX_Control( pDX, IDC_RADIODP, m_RadioDp );
	DDX_Control( pDX, IDC_RADIOCHARTFLOW, m_RadioChartFlow );
	DDX_Control( pDX, IDC_RADIOSETTING, m_RadioSetting );
	DDX_Control( pDX, IDC_EDITTRUEFLOW, m_EditTrueFlow );
	DDX_Control( pDX, IDC_EDITSETTING, m_EditSetting );
	DDX_Control( pDX, IDC_EDITDP, m_EditDp );
	DDX_Control( pDX, IDC_EDITCHARTFLOW, m_EditChartFlow );
	DDX_Control( pDX, IDC_COMBOEXTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBODEVICE, m_ComboDevice );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_GROUPWATER, m_GroupWater );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPRES, m_GroupRes );
	DDX_Control( pDX, IDC_BUTTONMODWATER, m_ButtonModWater );
}

BOOL CDlgViscCorr::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_STATICDEVICE );
	GetDlgItem( IDC_STATICDEVICE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_STATICSETTING );
	GetDlgItem( IDC_STATICSETTING )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_STATICDP );
	GetDlgItem( IDC_STATICDP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_STATICCHARTFLOW );
	GetDlgItem( IDC_STATICCHARTFLOW )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_STATICTRUEFLOW );
	GetDlgItem( IDC_STATICTRUEFLOW )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGVISCCORR_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str.Empty();

	// Add icons to Q, pipe and water groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupWater.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Water );
		m_GroupValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
		m_GroupRes.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Note );
	}

	// Add bitmap to "Water" button.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_HICONAddit = (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ModifyWater );
	}

	ASSERT( NULL != m_HICONAddit );
	m_ButtonModWater.SetIcon( m_HICONAddit );

	// LoadMappedBitmap for the push-like radio buttons.
	m_BitmapIn.LoadBitmap( IDB_GREENRARROW );
	m_BitmapOut.LoadBitmap( IDB_REDBULLET );

	// Set initial state for radio buttons.
	m_pEditIn1 = &m_EditSetting;
	m_pEditIn2 = &m_EditDp;
	m_EditChartFlow.SetReadOnly( TRUE );
	m_EditTrueFlow.SetReadOnly( TRUE );
	m_pPushed1 = &m_RadioSetting;
	m_pPushed2 = &m_RadioDp;
	m_pEditOut1 = &m_EditChartFlow;
	m_pEditOut2 = &m_EditTrueFlow;

	m_pEditIn1->SetWindowText( _T("") );
	m_pEditIn2->SetWindowText( _T("") );
	m_pEditOut1->SetWindowText( _T("") );
	m_pEditOut2->SetWindowText( _T("") );
	
	m_RadioDp.SetCheck( BST_CHECKED );
	m_RadioDp.SetImage( IDB_GREENRARROW );
	m_RadioSetting.SetCheck( BST_CHECKED );
	m_RadioSetting.SetImage( IDB_GREENRARROW );
	
	HBITMAP hBitmapOut = (HBITMAP)m_BitmapOut.GetSafeHandle();
	m_RadioTrueFlow.SetCheck( BST_UNCHECKED );
	m_RadioTrueFlow.SetImage( IDB_REDBULLET );
	m_RadioChartFlow.SetCheck( BST_UNCHECKED );
	m_RadioChartFlow.SetImage( IDB_REDBULLET );
	
	m_RadioDp.Invalidate();
	m_RadioChartFlow.Invalidate();
	m_RadioSetting.Invalidate();
	m_RadioTrueFlow.Invalidate();

	// Create a ToolTipCtrl and add a tool tip for "Water" button.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTMODWAT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONMODWATER ), TTstr );
	
	// Set the text for the static controls displaying units.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Set the text for the setting units.
	str = TASApp.LoadLocalizedString( IDS_SHEETHDR_TURNS );
	SetDlgItemText( IDC_STATICSETUNIT, str );
	
	// Set the text for the Dp units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetDlgItemText( IDC_STATICDPUNIT, tcName );
	
	// Set the text for the flow units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetDlgItemText( IDC_STATICCFLOWUNIT, tcName );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetDlgItemText( IDC_STATICTFLOWUNIT, tcName );

	// Can not calculate for Change-over
	if( ChangeOver == TASApp.GetpTADS()->GetpTechParams()->GetProductSelectionApplicationType() )
	{
		TASApp.GetpTADS()->GetpTechParams()->SetProductSelectionApplicationType( Heating );
		CWaterChar *pWC = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
		*pWC = *( TASApp.GetpTADS()->GetpTechParams()->GetDefaultISHeatingWC() );
	}
	// Save what is the current project type.
	m_eCurrentProjectType = m_pTADS->GetpTechParams()->GetProductSelectionApplicationType();
	
	// Build new water characteristic strings.
	m_clWaterChar = *m_pTADS->GetpTechParams()->GetDefaultISWC();
	CString str1, str2;
	m_clWaterChar.BuildWaterStrings( str1, str2 );
	
	// Set the text for the first static.
	SetDlgItemText( IDC_STATICFLUID1, str1 );
	
	// Set the text for the second static.
	SetDlgItemText( IDC_STATICFLUID2, str2 );

	// Fill ComboBoxEx ComboexType & other valve's combo boxes.
	_FillComboexType();
	
	return TRUE;
}

void CDlgViscCorr::OnOK() 
{
	PREVENT_ENTER_KEY
	
	CDialogEx::OnOK();
}

void CDlgViscCorr::OnDestroy()
{
	m_ImageListComboType.DeleteImageList();

	CDialogEx::OnDestroy();
}

HBRUSH CDlgViscCorr::OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor ) 
{
	HBRUSH hbr;
	hbr = CDialogEx::OnCtlColor( pDC, pWnd, nCtlColor );
	
	if( pWnd->m_hWnd == ( (CStatic *)GetDlgItem( IDC_STATICSETTING ) )->m_hWnd )
	{
		TRACE( _T("m_Setting = %f\n"), m_dSetting );

		if( -999.0 == m_dSetting )
		{
			pDC->SetTextColor( RGB( 255, 0, 0 ) );
		}
		else
		{
			pDC->SetTextColor( RGB( 0, 0, 0 ) );
		}
	}
	
	return hbr;
}

void CDlgViscCorr::OnButtonModWater() 
{
	// Display the dialog.
	CDlgWaterChar dlg;
	dlg.Display( NULL, CDlgWaterChar::DlgWaterChar_ForTools, true, m_eCurrentProjectType );

	// If the user has simply changed the application type, or has changed something in the current application type.
	if( m_eCurrentProjectType != dlg.GetCurrentApplicationType() || true == dlg.IsSomethingChanged( dlg.GetCurrentApplicationType() ) )
	{
		// Save the new user choice to force it if we call again the 'CDlgWaterChar' dialog.
		m_eCurrentProjectType = dlg.GetCurrentApplicationType();

		// Update the 2 Water statics because 'WM_USER_WATERCHANGE' message does not reach the dialog.
		double dDT = 0.0;
		dlg.GetWaterChar( dlg.GetCurrentApplicationType(), m_clWaterChar, dDT );
	
		CString str1, str2;
		m_clWaterChar.BuildWaterStrings( str1, str2 );
	
		// Set the text for the first static.
		SetDlgItemText( IDC_STATICFLUID1, str1 );
	
		// Set the text for the second static.
		SetDlgItemText( IDC_STATICFLUID2, str2 );

		// Update all fields.
		_UpdateAll();
	}
}

void CDlgViscCorr::OnSelChangeComboExType() 
{
	CString SelTypeID = _GetSelTypeID();
	ASSERT( false == SelTypeID.IsEmpty() );

	m_ComboFamily.ResetContent();

	// Fill the family combo according to selected type.
	CRank SortList;
	CTable *pTab = (CTable *)( m_pTADB->Get( _T("FAMILY_TAB") ).MP );
	ASSERT( NULL != pTab );

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( ( (CDB_StringID *)IDPtr.MP )->GetIDstr2() == SelTypeID )
		{
			TCHAR *ptcEndPtr;
			double dKey = _tcstod( ( (CDB_StringID *)IDPtr.MP )->GetIDstr(), &ptcEndPtr );
			ASSERT( _T('\0') == *ptcEndPtr );

			SortList.Add( ( (CDB_StringID *)IDPtr.MP )->GetString(), dKey, (LPARAM)(void *)IDPtr.MP );
			
			// Still need to insert directly in the combo.
			int index = m_ComboFamily.InsertString( 0, ( (CDB_StringID *)IDPtr.MP )->GetString() );
			m_ComboFamily.SetItemDataPtr( index, (void *)IDPtr.MP );
		}
	}
			
	// Sort items in the combo and reinsert them.
	SortList.Transfer( &m_ComboFamily );
		
	// Try to find at least one Regulating valve available for this type and this family.
	// Find the reg. valve type table.
	CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( (CData *)m_ComboType.GetItemDataPtr( m_ComboType.GetCurSel() ) );

	if( NULL == pStrID )
	{
		ASSERT_RETURN;
	}
	
	pTab = (CTable *)( TASApp.GetpTADB()->Get( pStrID->GetIDstr2() ).MP );

	if( NULL == pTab )
	{
		ASSERT_RETURN;
	}
	
	for( int i = 0; i < m_ComboFamily.GetCount(); )
	{
		CString strFamily;
		
		// Take family ID from stored MP.
		strFamily = ( (CDB_StringID *)m_ComboFamily.GetItemDataPtr( i ) )->GetIDPtr().ID;
		bool bSelectable = false;
		
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Scan all balancing valves with the same type.
			if( 0 != _tcscmp( ( (CDB_TAProduct *)IDPtr.MP )->GetTypeIDPtr().ID, (LPCTSTR)SelTypeID ) ) 
			{
				continue;
			}
			
			// And the same family.
			if( 0 != _tcscmp( ( (CDB_TAProduct *)IDPtr.MP )->GetFamilyIDPtr().ID, (LPCTSTR)strFamily ) )
			{
				continue;
			}
			
			if( false == ( (CDB_Thing *)IDPtr.MP )->IsSelectable( true ) )
			{
				continue;
			}
			
			bSelectable = true;
			
			// Loop aborted one BV of this type is enough to display the type.
			break;					
		}
			
		if( false == bSelectable )
		{
			m_ComboFamily.DeleteString( i );
		}
		else 
		{
			i++;
		}
	}

	// Check the number of families found to enable or disable the combo box.
	m_ComboFamily.EnableWindow( m_ComboFamily.GetCount() != 1 ? TRUE : FALSE );

	m_ComboFamily.SetCurSel( 0 );
	OnSelChangeComboFamily();
}

void CDlgViscCorr::OnSelChangeComboFamily() 
{
	m_ComboDevice.ResetContent();

	// Fill the device combo according to selected type and family.
	CRank SortList;
	CString SelTypeID = _GetSelTypeID();
	CString SelFamilyID = _GetSelFamilyID();
	CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( (CData *)m_ComboType.GetItemDataPtr( m_ComboType.GetCurSel() ) );

	if( NULL == pStrID )
	{
		ASSERT_RETURN;
	}
	
	CTable *pTab = (CTable *)( TASApp.GetpTADB()->Get( pStrID->GetIDstr2() ).MP );

	if( NULL == pTab )
	{
		ASSERT_RETURN;
	}
	
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( _T("RVTYPE_FO") == SelTypeID && false == IDPtr.MP->IsClass( CLASS( CDB_FixedOrifice ) ) )
		{
			continue;
		}
		
		if( _T("RVTYPE_CS") == SelTypeID && false == IDPtr.MP->IsClass( CLASS( CDB_CommissioningSet ) ) )
		{
			continue;
		}
		
		if( _T("RVTYPE_VV") == SelTypeID && false == IDPtr.MP->IsClass( CLASS( CDB_VenturiValve ) ) )
		{
			continue;
		}
		
		if( ( SelFamilyID == ( (CDB_TAProduct *)IDPtr.MP )->GetFamilyIDPtr().ID )
				&& ( true == ( (CDB_TAProduct *)IDPtr.MP )->IsSelectable( true ) ) )
		{
			if( m_ComboDevice.FindStringExact( -1, ( (CDB_TAProduct *)IDPtr.MP )->GetName() ) < 0 )
			{
				CDB_TAProduct *pclTAProduct = (CDB_TAProduct *)( IDPtr.MP );
				double dKey = (double)pclTAProduct->GetSizeKey();
				SortList.Add( ( (CDB_TAProduct *)IDPtr.MP )->GetName(), dKey, (LPARAM)IDPtr.MP );
				
				// Still need to insert directly in the combo.
				int iItem = m_ComboDevice.InsertString( 0, ( (CDB_TAProduct *)IDPtr.MP )->GetName() );
				m_ComboDevice.SetItemDataPtr( iItem, (void*)IDPtr.MP );
			}
		}
	}
	
	// Sort items in the combo and reinsert them.
	SortList.Transfer( &m_ComboDevice );

	// Check the number of devices found to enable or disable the combo box.
	m_ComboDevice.EnableWindow( m_ComboDevice.GetCount() != 1 ? TRUE : FALSE );

	m_ComboDevice.SetCurSel( 0 );
	OnSelChangeComboDevice();
}

void CDlgViscCorr::OnSelChangeComboDevice() 
{
	int iItem = m_ComboDevice.GetCurSel();

	if( CB_ERR == iItem )
	{
		m_pDev = NULL;
		return;
	}

	void *lp = m_ComboDevice.GetItemDataPtr( iItem );

	if( NULL == lp )
	{
		return;
	}
	
	m_pDev = (CDB_RegulatingValve *)lp;

	// Look for the min and max opening of the selected device.
	m_dMinOpening = -1.0;
	m_dMaxOpening = -1.0;
	CDB_Characteristic *pCharacteristic = m_pDev->GetValveCharacteristic();

	if( NULL != pCharacteristic )
	{
		m_dMinOpening = pCharacteristic->GetOpeningMin();
		m_dMaxOpening = pCharacteristic->GetOpeningMax();
	
		// Set the min max values in the static.
		CString str = TASApp.LoadLocalizedString( IDS_SHEETHDR_SETTING );
		str += _T(" ( ") + pCharacteristic->GetSettingString( m_dMinOpening ) + _T(" - ");
		str += pCharacteristic->GetSettingString( m_dMaxOpening ) + _T(" )");
		GetDlgItem( IDC_STATICSETTING )->SetWindowText( str ) ;
	}
	else
	{
		GetDlgItem( IDC_STATICSETTING )->SetWindowText( _T("-") );
	}

	// If the item is an STK, hide the SetUnit static.
	CString str = m_pDev->GetFamilyIDPtr().ID;
	( (CWnd *)GetDlgItem( IDC_STATICSETUNIT ) )->ShowWindow( _T("FAM_STK") == str ? SW_HIDE : SW_SHOW );

	m_dSetting = -1.0;

	switch( ReadDouble( m_EditSetting, &m_dSetting ) ) 
	{
		case RD_EMPTY:
			// Update out edits to _T("").
			if( m_EditDp.GetStyle() & ES_READONLY )
			{
				m_EditDp.SetWindowText( _T("") );
			}
			
			if( m_EditChartFlow.GetStyle() & ES_READONLY )
			{
				m_EditChartFlow.SetWindowText( _T("") );
			}
			
			if( m_EditTrueFlow.GetStyle() & ES_READONLY )
			{
				m_EditTrueFlow.SetWindowText( _T("") );
			}
			
			if( m_EditSetting.GetStyle() & ES_READONLY )
			{
				m_EditSetting.SetWindowText( _T("") );
			}

			break;
		
		case RD_NOT_NUMBER:
			m_EditSetting.GetWindowText( str );
			m_EditSetting.SetWindowText( _T("") );
			break;
		
		case RD_OK:
			// Update all fields.
			break;
	};
	
	_UpdateAll();
}

void CDlgViscCorr::OnRadioSetting() 
{
	// Check if button is already checked and in position 1. In this case swap button1/2 and editin1/2.
	if( true == _SwapCtrl( &m_RadioSetting ) )
	{
		return;
	}
	
	m_pPushed1->SetCheck( BST_UNCHECKED );
	m_pPushed1->SetImage( IDB_REDBULLET );
	m_pPushed1 = m_pPushed2;
	m_pPushed2 = &m_RadioSetting;
	m_pPushed2->SetCheck( BST_CHECKED );
	m_pPushed2->SetImage( IDB_GREENRARROW );
	_SetEditvsButton();

	m_RadioDp.Invalidate();
	m_RadioChartFlow.Invalidate();
	m_RadioSetting.Invalidate();
	m_RadioTrueFlow.Invalidate();

	double x;

	if( ReadDouble( m_EditSetting, &x ) != RD_OK )
	{
		m_EditSetting.SetWindowText( _T("") );
		m_pEditOut1->SetWindowText( _T("") );
		m_pEditOut2->SetWindowText( _T("") );
	}
}

void CDlgViscCorr::OnRadioDp() 
{
	// Check if button is already checked and in position 1. In this case swap button1/2 and editin1/2.
	if( true == _SwapCtrl( &m_RadioDp ) )
	{
		return;
	}

	m_pPushed1->SetCheck( BST_UNCHECKED );
	m_pPushed1->SetImage( IDB_REDBULLET );
	m_pPushed1 = m_pPushed2;
	m_pPushed2 = &m_RadioDp;
	m_pPushed2->SetCheck( BST_CHECKED );
	m_pPushed2->SetImage( IDB_GREENRARROW );
	_SetEditvsButton();

	m_RadioDp.Invalidate();
	m_RadioChartFlow.Invalidate();
	m_RadioSetting.Invalidate();
	m_RadioTrueFlow.Invalidate();

	double x;

	if( ReadDouble( m_EditDp, &x ) != RD_OK )
	{
		m_EditDp.SetWindowText( _T("") );
		m_pEditOut1->SetWindowText( _T("") );
		m_pEditOut2->SetWindowText( _T("") );
	}
}

void CDlgViscCorr::OnRadioChartFlow() 
{
	// Check if button is already checked and in position 1. In this case swap button1/2 and editin1/2.
	if( true == _SwapCtrl( &m_RadioChartFlow ) )
	{
		return;
	}

	m_pPushed1->SetCheck( BST_UNCHECKED );
	m_pPushed1->SetImage( IDB_REDBULLET );
	m_pPushed1 = m_pPushed2;
	m_pPushed2 = &m_RadioChartFlow;
	m_pPushed2->SetCheck( BST_CHECKED );
	m_pPushed2->SetImage( IDB_GREENRARROW );
	_SetEditvsButton();

	m_RadioDp.Invalidate();
	m_RadioChartFlow.Invalidate();
	m_RadioSetting.Invalidate();
	m_RadioTrueFlow.Invalidate();

	double x;

	if( ReadDouble( m_EditChartFlow, &x ) != RD_OK )
	{
		m_EditChartFlow.SetWindowText( _T("") );
		m_pEditOut1->SetWindowText( _T("") );
		m_pEditOut2->SetWindowText( _T("") );
	}

	if( ( m_pEditIn1 == &m_EditTrueFlow ) || ( m_pEditIn2 == &m_EditTrueFlow ) )
	{
		m_EditSetting.SetWindowText( NoResultStr );
		m_EditDp.SetWindowText( NoResultStr );
		m_EditChartFlow.SetWindowText( NoResultStr );
		m_EditTrueFlow.SetWindowText( NoResultStr );
		m_EditChartFlow.SetReadOnly( TRUE );
		m_EditTrueFlow.SetReadOnly( TRUE );
	}
}

void CDlgViscCorr::OnRadioTrueFlow() 
{
	// Check if button is already checked and in position 1. In this case swap button1/2 and editin1/2.
	if( true == _SwapCtrl( &m_RadioTrueFlow ) )
	{
		return;
	}

	m_pPushed1->SetCheck( BST_UNCHECKED );
	m_pPushed1->SetImage( IDB_REDBULLET );
	m_pPushed1 = m_pPushed2;
	m_pPushed2 = &m_RadioTrueFlow;
	m_pPushed2->SetCheck( BST_CHECKED );
	m_pPushed2->SetImage( IDB_GREENRARROW );

	_SetEditvsButton();

	m_RadioDp.Invalidate();
	m_RadioChartFlow.Invalidate();
	m_RadioSetting.Invalidate();
	m_RadioTrueFlow.Invalidate();

	double x;

	if( ReadDouble( m_EditTrueFlow, &x ) != RD_OK )
	{
		m_EditTrueFlow.SetWindowText( _T("") );
		m_pEditOut1->SetWindowText( _T("") );
		m_pEditOut2->SetWindowText( _T("") );
	}

	if( ( m_pEditIn1 == &m_EditChartFlow ) || ( m_pEditIn2 == &m_EditChartFlow ) )
	{
		m_EditSetting.SetWindowText( NoResultStr );
		m_EditDp.SetWindowText( NoResultStr );
		m_EditChartFlow.SetWindowText( NoResultStr );
		m_EditTrueFlow.SetWindowText( NoResultStr );
		m_EditChartFlow.SetReadOnly( TRUE );
		m_EditTrueFlow.SetReadOnly( TRUE );
	}
}

void CDlgViscCorr::OnChangeEditSetting() 
{
	if( GetFocus() != &m_EditSetting || m_EditSetting.GetStyle() & ES_READONLY )
	{
		return; 
	}

	CString str;

	m_dSetting = -1.0;

	switch( ReadDouble( m_EditSetting, &m_dSetting ) )
	{
		case RD_EMPTY:
			
			// Update out edits to _T("").
			if( m_EditDp.GetStyle() & ES_READONLY )
			{
				m_EditDp.SetWindowText( _T("") );
			}
			
			if( m_EditChartFlow.GetStyle() & ES_READONLY )
			{
				m_EditChartFlow.SetWindowText( _T("") );
			}

			if( m_EditTrueFlow.GetStyle() & ES_READONLY )
			{
				m_EditTrueFlow.SetWindowText( _T("") );
			}

			break;
		
		case RD_NOT_NUMBER:
			m_EditSetting.GetWindowText( str );
			
			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) )
			{
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
			}
			else
			{	
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}
			
			break;

		case RD_OK:
			// Update all fields.
			if( 0.0 != m_dSetting )
			{
				if( m_dSetting < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
					m_pEditOut1->SetWindowText( NoResultStr );
					m_pEditOut2->SetWindowText( NoResultStr );
				}
				else
				{
					if( m_dSetting < m_dMinOpening || m_dSetting > m_dMaxOpening )
					{
						m_dSetting = -999.0; // Show setting range in red
					}

					// Update display.
					( (CStatic *)GetDlgItem( IDC_STATICSETTING ) )->Invalidate();
					( (CStatic *)GetDlgItem( IDC_STATICSETTING ) )->UpdateWindow();

					if( -999.0 == m_dSetting )
					{
						m_pEditOut1->SetWindowText( NoResultStr );
						m_pEditOut2->SetWindowText( NoResultStr );
					}
					else
					{
						_UpdateAll();
					}
				}
			}
			break;
	};
}

void CDlgViscCorr::OnChangeEditDp() 
{
	if( GetFocus() != &m_EditDp || m_EditDp.GetStyle() & ES_READONLY )
	{
		return; 
	}

	CString str;
	m_dDp = -1.0;

	switch( ReadDouble( m_EditDp, &m_dDp ) ) 
	{
		case RD_EMPTY:
	
			// Update out edits to _T("").
			if( m_EditSetting.GetStyle() & ES_READONLY )
			{
				m_EditSetting.SetWindowText( _T("") );
			}
			
			if( m_EditChartFlow.GetStyle() & ES_READONLY )
			{
				m_EditChartFlow.SetWindowText( _T("") );
			}
			
			if( m_EditTrueFlow.GetStyle() & ES_READONLY )
			{
				m_EditTrueFlow.SetWindowText( _T("") );
			}

			break;

		case RD_NOT_NUMBER:
			m_EditDp.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
			{
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
			}
			else
			{	
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}
			
			break;

		case RD_OK:
			
			if( 0.0 != m_dDp )
			{
				if( m_dDp < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
					m_pEditOut1->SetWindowText( NoResultStr );
					m_pEditOut2->SetWindowText( NoResultStr );
				}
				else
				{
					m_dDp = CDimValue::CUtoSI( _U_DIFFPRESS, m_dDp );
					// Update all fields.
					_UpdateAll();
				}
			}
			
			break;
	};
}

void CDlgViscCorr::OnChangeEditChartFlow() 
{
	if( GetFocus() != &m_EditChartFlow || m_EditChartFlow.GetStyle() & ES_READONLY )
	{
		return; 
	}

	CString str;
	m_dChartFlow = -1.0;

	switch( ReadDouble( m_EditChartFlow, &m_dChartFlow ) )
	{
		case RD_EMPTY:

			// Update out edits to _T("").
			if( m_EditSetting.GetStyle() & ES_READONLY )
			{
				m_EditSetting.SetWindowText( _T("") );
			}
			
			if( m_EditDp.GetStyle() & ES_READONLY )
			{
				m_EditDp.SetWindowText( _T("") );
			}
			
			if( m_EditTrueFlow.GetStyle() & ES_READONLY )
			{
				m_EditTrueFlow.SetWindowText( _T("") );
			}

			break;

		case RD_NOT_NUMBER:
			m_EditChartFlow.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
			{
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
			}
			else
			{	
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}

			break;

		case RD_OK:

			if( 0.0 != m_dChartFlow )
			{
				if( m_dChartFlow < 0.0 )
				{
					m_pEditOut1->SetWindowText( NoResultStr );
					m_pEditOut2->SetWindowText( NoResultStr );
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				}
				else
				{
					m_dChartFlow = CDimValue::CUtoSI( _U_FLOW, m_dChartFlow );
					// Update all fields.
					_UpdateAll();
				}
			}
			
			break;
	};
}

void CDlgViscCorr::OnChangeEditTrueFlow() 
{
	if( GetFocus() != &m_EditTrueFlow || m_EditTrueFlow.GetStyle() & ES_READONLY )
	{
		return; 
	}

	CString str;
	m_dTrueFlow = -1.0;
	
	switch( ReadDouble( m_EditTrueFlow, &m_dTrueFlow ) ) 
	{
		case RD_EMPTY:
			
			// Update out edits to _T("").
			if( m_EditSetting.GetStyle() & ES_READONLY )
			{
				m_EditSetting.SetWindowText( _T("") );
			}
			
			if( m_EditDp.GetStyle() & ES_READONLY )
			{
				m_EditDp.SetWindowText( _T("") );
			}
			
			if( m_EditChartFlow.GetStyle() & ES_READONLY )
			{
				m_EditChartFlow.SetWindowText( _T("") );
			}
			
			break;
		
		case RD_NOT_NUMBER:
			m_EditTrueFlow.GetWindowText( str );
			
			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
			{
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
			}
			else
			{	
				m_pEditOut1->SetWindowText( NoResultStr );
				m_pEditOut2->SetWindowText( NoResultStr );
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}
			
			break;

		case RD_OK:
			
			if( 0.0 != m_dTrueFlow )
			{
				if( m_dTrueFlow < 0.0 )
				{
					m_pEditOut1->SetWindowText( NoResultStr );
					m_pEditOut2->SetWindowText( NoResultStr );
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				}
				else
				{
					m_dTrueFlow = CDimValue::CUtoSI( _U_FLOW, m_dTrueFlow );
					// Update all fields.
					_UpdateAll();
				}
			}
			
			break;
	};
}

void CDlgViscCorr::OnKillFocusEditSetting() 
{
	if( m_EditSetting.GetStyle() & ES_READONLY )
	{
		return; 
	}

	double dTmp;
	
	switch( ReadDouble( m_EditSetting, &dTmp ) ) 
	{
		case RD_EMPTY:
			break;
		
		case RD_NOT_NUMBER:
			m_EditSetting.SetSel( 0, -1 );
			m_EditSetting.SetFocus();	
			break;
		
		case RD_OK:
			
			// Update all fields.
			if( dTmp < m_dMinOpening || dTmp > m_dMaxOpening )
			{
				m_EditSetting.SetSel( 0, -1 );
				m_EditSetting.SetFocus();	
				return;
			}
			else
			{
				if( 0.0 == dTmp )
				{
					m_EditSetting.SetWindowText( _T("") );
				}
				else if( dTmp < 0.0 )
				{
					m_EditSetting.SetSel( 0, -1 );
					m_EditSetting.SetFocus();	
					return;
				}
			}
			break;
	};
}

void CDlgViscCorr::OnKillFocusEditDp() 
{
	if( m_EditDp.GetStyle() & ES_READONLY )
	{
		return; 
	}
	
	double dTmp;
	
	switch( ReadDouble( m_EditDp, &dTmp ) )
	{
		case RD_EMPTY:
			break;
		
		case RD_NOT_NUMBER:
			m_EditDp.SetSel( 0, -1 );
			m_EditDp.SetFocus();	
			break;

		case RD_OK:
			
			// Update all fields.
			if( 0.0 == dTmp )
			{
				m_EditDp.SetWindowText( _T("") );
			}
			else if( dTmp <= 0.0 )
			{
				m_EditDp.SetSel( 0, -1 );
				m_EditDp.SetFocus();	
			}
			
			break;
	};
}

void CDlgViscCorr::OnKillFocusEditChartFlow() 
{
	if( m_EditChartFlow.GetStyle() & ES_READONLY )
	{
		return; 
	}
	
	double dTmp;
	
	switch( ReadDouble( m_EditChartFlow, &dTmp ) )
	{
		case RD_EMPTY:
			break;
		
		case RD_NOT_NUMBER:
			m_EditChartFlow.SetSel( 0, -1 );
			m_EditChartFlow.SetFocus();	
			break;
		
		case RD_OK:

			if( 0.0 == dTmp )
			{
				m_EditChartFlow.SetWindowText( _T("") );
			}
			else if( dTmp < 0.0 )
			{
				m_EditChartFlow.SetSel( 0, -1 );
				m_EditChartFlow.SetFocus();	
			}

			break;
	};
}

void CDlgViscCorr::OnKillFocusEditTrueFlow() 
{
	if( m_EditTrueFlow.GetStyle() & ES_READONLY )
	{
		return; 
	}
	
	double dTmp;
	
	switch( ReadDouble( m_EditTrueFlow, &dTmp ) ) 
	{
		case RD_EMPTY:
			break;

		case RD_NOT_NUMBER:
			m_EditTrueFlow.SetSel( 0, -1 );
			m_EditTrueFlow.SetFocus();	
			break;

		case RD_OK:
			
			// Update all fields.
			if( 0.0 == dTmp )
			{
				m_EditTrueFlow.SetWindowText( _T("") );
			}
			else if( dTmp < 0.0 )
			{
				m_EditTrueFlow.SetSel( 0, -1 );
				m_EditTrueFlow.SetFocus();	
			}
			
			break;
	};
}

void CDlgViscCorr::_FillComboexType( LPCTSTR ptcType )
{
	// Create and attach an image list to 'm_ComboType'.
	
	// Prepare the insertion of items in 'm_ComboType'.
	COMBOBOXEXITEM *cbi;
	CString str;
	
	CStringArray TableTypeID, TableValvTab;
	TableTypeID.Add( _T("RVTYPE_TAB") );
	TableTypeID.Add( _T("BCVTYPE_TAB") );
	int iItemCount = 0;
	IDPTR IDPtr = _NULL_IDPTR;
	
	for( int iLoopTableType = 0; iLoopTableType < TableTypeID.GetSize(); iLoopTableType++ )
	{
		// Find the reg. valve type tables.
		CTable *pTab = (CTable *)( m_pTADB->Get( (LPCTSTR)TableTypeID[iLoopTableType] ).MP );
		ASSERT( NULL != pTab );

		// Item insertion !!! Must be done starting at position 0 and then by increment of 1.
		for( IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			iItemCount++;
			CDB_StringID *pStrID = (CDB_StringID *)( IDPtr.MP );
			bool bFound = false;

			for( int i = 0; i < TableValvTab.GetCount() && false == bFound; i++ )
			{
				if( 0 == TableValvTab.GetAt( i ).Compare( pStrID->GetIDstr2() ) )
				{
					bFound = true;
				}
			}
			
			// Store table associated to the type.
			if( false == bFound )
			{
				TableValvTab.Add( pStrID->GetIDstr2() );
			}
		}
		
		ASSERT( 0 != iItemCount );
		ASSERT( 0 != TableValvTab.GetCount() );
	}
	
	// Prepare vector of comboboxex items.
	cbi = new COMBOBOXEXITEM[iItemCount];
	int i;
	
	for( i = 0; i < iItemCount; i++ )
	{
		cbi[i].iItem = -1;
		cbi[i].mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM;
		cbi[i].cchTextMax = 31;
	}
	
	// Create an array to store pointers of strings that will be deleted just after have filled the CExtComboBoxEx.
	CArray <CString *> arStr;
	int iOffset = 0;
	
	for( int k = 0; k < TableTypeID.GetSize(); k++ )
	{
		CTable *pTab = (CTable *)( m_pTADB->Get( (LPCTSTR)TableTypeID[k] ).MP );
		ASSERT( NULL != pTab );
		
		// Loop on all entries of the table and insert data in vector cbi.
		IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) );
		ASSERT( _T('\0') != *IDPtr.ID );
		CDB_StringID *pstrI; 
		int n = 0;
		
		while( _T('\0') != *IDPtr.ID )
		{
			pstrI = (CDB_StringID *)( IDPtr.MP );
			str = pstrI->GetString();
			i = _ttoi( pstrI->GetIDstr() ) - 1 + iOffset;
			cbi[i].iItem = i;
			
			CString *pStr = new CString( pstrI->GetString() );
			arStr.Add( pStr );
			cbi[i].pszText = (TCHAR*)(LPCTSTR)(*pStr);
			cbi[i].iImage = i;
			cbi[i].iSelectedImage = i;
			cbi[i].lParam = (LPARAM)(void *)( IDPtr.MP );
			IDPtr = pTab->GetNext();
			n++;
		}
		
		iOffset += n;
	}

	// Insert items from vector cbi in the combo
	for( i = 0; i < iItemCount; i++ )
	{
		if( cbi[i].iItem != -1 )
		{
			int j = m_ComboType.AddString( cbi[i].pszText );
			m_ComboType.SetItemData( j, cbi[i].lParam );
		}
	}
	
	for( i = 0; i < arStr.GetCount() ; i++ )
	{
		delete arStr.GetAt( i );
	}

	delete [] cbi;

	CArray<bool> ar;
	ar.SetSize( m_ComboType.GetCount() );

	for( int i = 0; i < ar.GetSize(); i++ )
	{
		ar.SetAt( i, false );
	}

	// Scan valve tab to check if at least one valve of this type is available.
	for( int i = 0; i < TableValvTab.GetCount(); i++ )
	{
		CTable *pTab = (CTable *)( m_pTADB->Get( (LPCTSTR)TableValvTab.GetAt( i ) ).MP );
		ASSERT( NULL != pTab );

		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			CDB_RegulatingValve *pRV = dynamic_cast<CDB_RegulatingValve *>( IDPtr.MP );

			if( NULL == pRV )
			{
				continue;
			}
			
			if( false == pRV->IsSelectable( true ) )
			{
				continue;
			}
			
			CDB_ValveCharacteristic *pValveCharacteristic = pRV->GetValveCharacteristic();
			
			if( NULL == pValveCharacteristic || false == pValveCharacteristic->IsViscCorrExist() )
			{
				continue;
			}
			
			CString Type = pRV->GetTypeIDPtr().ID;
			
			// Try to found type into the combo list.
			bool bAllFound = false;
			
			for( int j = 0; j < m_ComboType.GetCount(); j++ )
			{
				CDB_StringID *pStrID = (CDB_StringID *)( m_ComboType.GetItemDataPtr( j ) );
				CString strType = pStrID->GetIDPtr().ID;

				if( Type == strType )
				{
					ar.SetAt( j, true );
				}
			}
		}
	}

	// Delete entries that are not into the array.
	for( int i = ar.GetSize()-1; i >= 0; i-- )
	{
		if( false == ar.GetAt( i ) )
		{
			m_ComboType.DeleteString( i );
		}
	}

	if( 1 == m_ComboType.GetCount() )
	{
		m_ComboType.EnableWindow( FALSE );
	}

	// Change family.
	m_ComboType.SetCurSel( 0 );
	OnSelChangeComboExType();
	
	if( NULL != ptcType && ( CB_ERR != m_ComboType.FindStringExact( -1, ptcType ) ) )
	{
		m_ComboType.SetCurSel( m_ComboType.FindStringExact( -1, ptcType ) );
	}
	else
	{
		m_ComboType.SetCurSel( 0 );
	}
}

LPCTSTR CDlgViscCorr::_GetSelTypeID()
{
	if( CB_ERR == m_ComboType.GetCurSel() )
	{
		return _T("");
	}

	CString str;

	ASSERT( m_ComboType.GetCount() > 0 );

	m_ComboType.GetLBText( m_ComboType.GetCurSel(), str );
	m_strSelType = str;
	
	return ( (CDB_StringID *)m_ComboType.GetItemDataPtr( m_ComboType.GetCurSel() ) )->GetIDPtr().ID;
}

LPCTSTR CDlgViscCorr::_GetSelFamilyID()
{
	ASSERT( m_ComboFamily.GetCount() > 0 );

	return ( (CDB_StringID *)m_ComboFamily.GetItemDataPtr( m_ComboFamily.GetCurSel() ) )->GetIDPtr().ID;
}

void CDlgViscCorr::_UpdateAll()
{
	if( NULL == m_pDev )
	{
		return;
	}

	CDB_ValveCharacteristic *pValveCharacteristic = m_pDev->GetValveCharacteristic();

	if( NULL == pValveCharacteristic )
	{
		return;
	}

	double dValue;
	bool bContinue = true;

	switch( ReadDouble( *m_pEditIn1, &dValue ) ) 
	{
		case RD_EMPTY:

			// Update out edits to _T("").
			if( ES_READONLY == ( m_EditDp.GetStyle() & ES_READONLY ) )
			{
				m_EditDp.SetWindowText( _T("") );
			}
			
			if( ES_READONLY == ( m_EditChartFlow.GetStyle() & ES_READONLY ) )
			{
				m_EditChartFlow.SetWindowText( _T("") );
			}
			
			if( ES_READONLY == ( m_EditTrueFlow.GetStyle() & ES_READONLY ) )
			{
				m_EditTrueFlow.SetWindowText( _T("") );
			}
			
			if( ES_READONLY == ( m_EditSetting.GetStyle() & ES_READONLY ) )
			{
				m_EditSetting.SetWindowText( _T("") );
			}

			bContinue = false;
			break;

		case RD_NOT_NUMBER:
			bContinue = false;
			break;

		case RD_OK:
			break;
	};

	if( false == bContinue )
	{
		return;
	}

	switch( ReadDouble( *m_pEditIn2, &dValue ) ) 
	{
		case RD_EMPTY:

			// Update out edits to _T("").
			if( ES_READONLY == ( m_EditDp.GetStyle() & ES_READONLY ) )
			{
				m_EditDp.SetWindowText( _T("") );
			}
			
			if( ES_READONLY == ( m_EditChartFlow.GetStyle() & ES_READONLY ) )
			{
				m_EditChartFlow.SetWindowText( _T("") );
			}
			
			if( ES_READONLY == ( m_EditTrueFlow.GetStyle() & ES_READONLY ) )
			{
				m_EditTrueFlow.SetWindowText( _T("") );
			}
			
			if( ES_READONLY == ( m_EditSetting.GetStyle() & ES_READONLY ) )
			{
				m_EditSetting.SetWindowText( _T("") );
			}

			bContinue = false;
			break;

		case RD_NOT_NUMBER:
			bContinue = false;
			break;

		case RD_OK:
			break;
	};

	if( false == bContinue )
	{
		return;
	}
	
	int iCaseByte = 0;

	if( ( m_pEditIn1 == &m_EditSetting ) || ( m_pEditIn2 == &m_EditSetting ) )
	{

		if( -1.0 == m_dSetting )
		{
			return;
		}

		iCaseByte += 1;
	}
	
	if( ( m_pEditIn1 == &m_EditDp ) || ( m_pEditIn2 == &m_EditDp ) )
	{

		if( -1.0 == m_dDp )
		{
			return;
		}

		iCaseByte += 2;
	}
	
	if( ( m_pEditIn1 == &m_EditChartFlow ) || ( m_pEditIn2 == &m_EditChartFlow ) )
	{

		if( -1.0 == m_dChartFlow )
		{
			return;
		}

		iCaseByte += 4;
	}
	
	if( ( m_pEditIn1 == &m_EditTrueFlow ) || ( m_pEditIn2 == &m_EditTrueFlow ) )
	{

		if( -1.0 == m_dTrueFlow )
		{
			return;
		}

		iCaseByte += 8;
	}

	// Get water characteristics.
	double dRho = m_clWaterChar.GetDens();
	double dNu = m_clWaterChar.GetKinVisc();

	// Achieve the calculations and update of edit boxes.
	double dKv, dMinOpening, dMaxOpening;

	switch( iCaseByte )
	{
		// Setting & Dp.
		case 3:
			dKv = pValveCharacteristic->GetKv( m_dSetting );
			m_dChartFlow = CalcqT( dKv, m_dDp, _NORMAL_RHO );
			pValveCharacteristic->GetValveQ( &m_dTrueFlow, m_dDp, m_dSetting, dRho, dNu );
			m_EditChartFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dChartFlow ), 3, 1, 0 ) );
			m_EditTrueFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dTrueFlow ), 3, 1, 0 ) );
			break;

		// Setting & Chart flow.
		case 5:
			dKv = pValveCharacteristic->GetKv( m_dSetting );
			m_dDp = CalcDp( m_dChartFlow, dKv, _NORMAL_RHO );
			pValveCharacteristic->GetValveQ( &m_dTrueFlow, m_dDp, m_dSetting, dRho, dNu );
			m_EditDp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, m_dDp ), 3, 1, 0 ) );
			m_EditTrueFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dTrueFlow ), 3, 1, 0 ) );
			break;

		// Dp & Chart flow.
		case 6:
			dKv = CalcKv( m_dChartFlow, m_dDp, _NORMAL_RHO );
			m_dSetting = pValveCharacteristic->GetOpening( dKv );
			dMinOpening = pValveCharacteristic->GetOpeningMin();
			dMaxOpening = pValveCharacteristic->GetOpeningMax();
			
			// Remark: -1.0 is the value returned by GetOpening when Kv is out of valve range.
			if( -1.0 == m_dSetting || -1.0 == dMinOpening || -1.0 == dMaxOpening || ( m_dSetting - dMinOpening ) / ( dMaxOpening - dMinOpening ) < 0.05 )
			{
				m_dSetting = -999.0;
				m_EditSetting.SetWindowText( NoResultStr );
				m_EditTrueFlow.SetWindowText( NoResultStr );
				bContinue = false;
			}
			else
			{
				pValveCharacteristic->GetValveQ( &m_dTrueFlow, m_dDp, m_dSetting, dRho, dNu );
				m_EditSetting.SetWindowText( pValveCharacteristic->GetSettingString( m_dSetting ) );
				m_EditTrueFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dTrueFlow ), 3, 1, 0 ) );
			}
			
			break;

		// Setting & True flow.
		case 9:
			dKv = pValveCharacteristic->GetKv( m_dSetting );

			if( true == pValveCharacteristic->GetValveDp( m_dTrueFlow, &m_dDp, m_dSetting, dRho, dNu ) )
			{
				m_dChartFlow = CalcqT( dKv, m_dDp, _NORMAL_RHO );
				m_EditDp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, m_dDp ), 3, 1, 0 ) );
				m_EditChartFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dChartFlow ), 3, 1, 0 ) );
			}

			break;

		// Dp & True flow.
		case 10:
			dMinOpening = pValveCharacteristic->GetOpeningMin();
			dMaxOpening = pValveCharacteristic->GetOpeningMax();

			if( false == pValveCharacteristic->GetValveOpening( m_dTrueFlow, m_dDp, &m_dSetting, dRho, dNu, 1 ) ||
				-1.0 == dMinOpening || -1.0 == dMaxOpening ||
				( m_dSetting - dMinOpening ) / ( dMaxOpening - dMinOpening ) < 0.05 )
			{
				m_dSetting = -999.0;
				m_EditSetting.SetWindowText( NoResultStr );
				m_EditChartFlow.SetWindowText( NoResultStr );
				bContinue = false;
			}
			else
			{
				dKv = pValveCharacteristic->GetKv( m_dSetting );
				m_dChartFlow = CalcqT( dKv, m_dDp, _NORMAL_RHO );
				m_EditSetting.SetWindowText( pValveCharacteristic->GetSettingString( m_dSetting ) );
				m_EditChartFlow.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_FLOW, m_dChartFlow ), 3, 1, 0 ) );
			}
			
			break;

		// Chart flow & true flow.
		case 12:
			break;
	};

	if( false == bContinue )
	{
		return;
	}

	( (CStatic *)GetDlgItem( IDC_STATICSETTING ) )->Invalidate();
	( (CStatic *)GetDlgItem( IDC_STATICSETTING ) )->UpdateWindow();

}

CEdit *CDlgViscCorr::_Get2ndOutEdit( CEdit *pEditOut1 )
{
	if( &m_EditSetting != pEditOut1 && BST_UNCHECKED == m_RadioSetting.GetCheck() )
	{
		return &m_EditSetting;
	}

	if( &m_EditDp != pEditOut1 && BST_UNCHECKED == m_RadioDp.GetCheck() )
	{
		return &m_EditDp;
	}

	if( &m_EditChartFlow != pEditOut1 && BST_UNCHECKED == m_RadioChartFlow.GetCheck() )
	{
		return &m_EditChartFlow;
	}

	if( &m_EditTrueFlow != pEditOut1 && BST_UNCHECKED == m_RadioTrueFlow.GetCheck() )
	{
		return &m_EditTrueFlow;
	}

	return NULL;
}

bool CDlgViscCorr::_SwapCtrl( CButton *pButton )
{
	if( BST_CHECKED == pButton->GetCheck() )
	{
		if( m_pPushed1 == pButton )
		{
			void *pVoid = (void *)m_pPushed1;
			m_pPushed1 = m_pPushed2;
			m_pPushed2 = (CMFCButton *)pVoid;

			pVoid = (void *)m_pEditIn1;
			m_pEditIn1 = m_pEditIn2;
			m_pEditIn2 = (CEdit *)pVoid;
		}
		
		return true;
	}
	
	return false;
}

void CDlgViscCorr::_SetEditvsButton()
{
	CString str;

	int iOut = 0x0F;
	m_pEditIn1->GetWindowText( str );

	if( m_pPushed1 == &m_RadioSetting )
	{
		m_pEditIn1 = &m_EditSetting;
		
		if( true == str.IsEmpty() || str == NoResultStr )
		{
			m_dSetting = -1.0;
			m_pEditIn1->SetWindowText( _T("") );
		}
		
		iOut &= ~0x01;
	}
	else if( m_pPushed1 == &m_RadioDp )
	{
		m_pEditIn1 = &m_EditDp;
		
		if( true == str.IsEmpty() || str == NoResultStr )
		{
			m_dDp = -1.0;
			m_pEditIn1->SetWindowText( _T("") );
		}
		
		iOut &= ~0x02;
	}
	else if( m_pPushed1 == &m_RadioChartFlow )
	{
		m_pEditIn1 = &m_EditChartFlow; 
		
		if( true == str.IsEmpty() || str == NoResultStr )
		{
			m_dChartFlow = -1.0;
			m_pEditIn1->SetWindowText( _T("") );
		}
		
		iOut &= ~0x04;
	}
	else if( m_pPushed1 == &m_RadioTrueFlow )
	{
		m_pEditIn1 = &m_EditTrueFlow; 
		
		if( true == str.IsEmpty() || str == NoResultStr )
		{
			m_dTrueFlow = -1.0;
			m_pEditIn1->SetWindowText( _T("") );
		}
		
		iOut &= ~0x08;
	}

	if( m_pPushed2 == &m_RadioSetting )
	{
		m_pEditIn2 = &m_EditSetting;
		iOut &= ~0x01;
	}
	else if( m_pPushed2 == &m_RadioDp )
	{
		m_pEditIn2 = &m_EditDp; 
		iOut &= ~0x02;
	}
	else if( m_pPushed2 == &m_RadioChartFlow )
	{
		m_pEditIn2 = &m_EditChartFlow;
		iOut &= ~0x04;
	}
	else if( m_pPushed2 == &m_RadioTrueFlow )
	{
		m_pEditIn2 = &m_EditTrueFlow;
		iOut &= ~0x08;
	}
	
	if( 0x01 == ( iOut & 0x01 ) )
	{
		m_pEditOut1 = &m_EditSetting; 
		iOut &= ~0x01;
	}
	else if( 0x02 == ( iOut & 0x02 ) )
	{
		m_pEditOut1 = &m_EditDp; 
		iOut &= ~0x02;
	}	
	else if( 0x04 == ( iOut & 0x04 ) ) 
	{
		m_pEditOut1 = &m_EditChartFlow; 
		iOut &= ~0x04;
	}	
	else if( 0x08 == ( iOut & 0x08 ) ) 
	{
		m_pEditOut1 = &m_EditTrueFlow; 
		iOut &= ~0x04;
	}

	if( 0x01 == ( iOut & 0x01 ) )
	{
		m_pEditOut2 = &m_EditSetting; 
	}
	else if( 0x02 == ( iOut & 0x02 ) ) 
	{
		m_pEditOut2 = &m_EditDp; 
	}
	else if( 0x04 == ( iOut & 0x04 ) ) 
	{
		m_pEditOut2 = &m_EditChartFlow; 
	}
	else if( 0x08 == ( iOut & 0x08 ) ) 
	{
		m_pEditOut2 = &m_EditTrueFlow; 
	}

	m_pEditIn1->SetReadOnly( FALSE );
	m_pEditIn2->SetReadOnly( FALSE );
	m_pEditOut1->SetReadOnly( TRUE );
	m_pEditOut2->SetReadOnly( TRUE );
}
