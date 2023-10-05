#include "stdafx.h"
#include "TASelect.h"
#include "Global.h"
#include "Utilities.h"
#include "DlgHMDirSel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgHMDirSel::CDlgHMDirSel( CWnd* pParent )
	: CDialogEx( CDlgHMDirSel::IDD, pParent )
{
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_SelType = _T("");
	m_SelThingID = _T("");
	m_SelectedID = _T("");
	m_pProd = NULL;
	m_dPresetting = -1.0;
	m_dMinOpening = -1.0;
	m_dMaxOpening = -1.0;
}

int CDlgHMDirSel::Display(CString SelectedID, double dPresetting )
{
	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();

	m_dPresetting = dPresetting;
	m_SelectedID = SelectedID;

	return DoModal();
}

int CDlgHMDirSel::Display( CString CBIType, CString CBISize, double dPresetting )
{
	m_pTADB = TASApp.GetpTADB();
	CRank TAPList;
	CString SelectedID;
	LPARAM dummy;

	if( true == m_pTADB->GetTAPListFromCBIDef( &TAPList, CBIType, CBISize ) )
		TAPList.GetFirst( SelectedID, dummy );

	return Display( SelectedID, dPresetting );
}

BEGIN_MESSAGE_MAP( CDlgHMDirSel, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOEXTYPE, OnSelChangeComboExType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnSelChangeComboFamily )
	ON_CBN_SELCHANGE( IDC_COMBODEVICE, OnSelChangeComboDevice )
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED( IDC_CHECKPRESETTING, OnCheckPresetting )
	ON_EN_CHANGE( IDC_EDITPRESETTING, OnChangeEditPresetting )
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_LISTDEVICE, OnItemChangedListDevice )
END_MESSAGE_MAP()

void CDlgHMDirSel::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LISTDEVICE, m_ListDevice );
	DDX_Control( pDX, IDC_EDITPRESETTING, m_EditPresetting );
	DDX_Control( pDX, IDC_CHECKPRESETTING, m_CheckPresetting );
	DDX_Control( pDX, IDC_COMBOEXTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBODEVICE, m_ComboDevice );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
}

BOOL CDlgHMDirSel::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_STATICDEVICE );
	GetDlgItem( IDC_STATICDEVICE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_CHECKPRESETTING );
	GetDlgItem( IDC_CHECKPRESETTING )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_TURNSPOS );
	GetDlgItem( IDC_STATICPRESETUNIT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DELETEDARTICLENUMBER );
	GetDlgItem( IDC_STATICDELETED )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str.Empty();

	// Define the properties of the CEdit box.
	m_EditPresetting.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );

	// Define the width of each column depending on the width of the ListCtrl.
	CRect rect;
	m_ListDevice.GetClientRect( (LPRECT)&rect );

	// Add columns.
	// Size of columns is defined by the size of columns header, it must be formatted with a sufficient number of space.  
	CSize size;
	int col = 0;
	if( false == TASApp.IsTAArtNumberHidden() )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_ARTICLENR );
		m_ListDevice.InsertColumn( col, str, LVCFMT_LEFT, (int)( rect.Width() * 0.4 ), col );
		col++;
	}
	
	if( true == TASApp.IsLocalArtNumberUsed() )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_LOCARTNR );
		m_ListDevice.InsertColumn( col, str, LVCFMT_LEFT, (int)( rect.Width() * 0.4 ), col );
		col++;
	}

	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_CONNECTION );
	
	m_ListDevice.InsertColumn( col, str, LVCFMT_LEFT, (int)( rect.Width() * 0.3 ), col );
	
	col ++;
	str = TASApp.LoadLocalizedString( IDS_DLGHMDIRSEL_VERSION );
	
	m_ListDevice.InsertColumn( col, str, LVCFMT_LEFT, (int)( rect.Width() * 0.3 ), col );

	_FillComboexType();

	if( m_dPresetting <= 0.0 )
	{
		m_CheckPresetting.SetCheck( BST_UNCHECKED );
		OnCheckPresetting();
	}
	else
	{
		m_CheckPresetting.SetCheck( BST_CHECKED );

		str = _T("-");
		CDB_TAProduct *pTAP = (CDB_TAProduct *)( m_pTADB->Get( (LPCTSTR)m_SelectedID ).MP );

		if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
		{
			str = pTAP->GetValveCharacteristic()->GetSettingString( m_dPresetting );
		}

		m_EditPresetting.SetWindowText( str );
	}

	// if the dialog box is opened with an ID != "", select the correct type, familly and device.
	if( false == m_SelectedID.IsEmpty() )
	{
		CDB_TAProduct *pTAP = (CDB_TAProduct *)( m_pTADB->Get( (LPCTSTR)m_SelectedID ).MP );
		if( NULL == pTAP )
			m_SelectedID = _T("");
		else
		{
			// Find type.
			int index = m_ComboType.FindStringExact( 0, pTAP->GetType() );
			if( index >= 0 )
			{
				m_ComboType.SetCurSel( index );
				OnSelChangeComboExType();
				
				// Find family.
				index = m_ComboFamily.FindStringExact( 0, pTAP->GetFamily() );
				if( index >= 0 )
				{
					m_ComboFamily.SetCurSel( index );
					OnSelChangeComboFamily();
					
					// Find device.
					index = m_ComboDevice.FindStringExact( 0, pTAP->GetName() );
					if( index >= 0 )
					{
						m_ComboDevice.SetCurSel( index );
						OnSelChangeComboDevice();
					}
					
					// Find valve ... in List Box.
					LVFINDINFO info;
					info.flags = LVFI_PARAM;
					info.lParam = (LPARAM)pTAP;
					index = m_ListDevice.FindItem( &info );
					if( index >= 0 )
					{
						m_ListDevice.SetFocus();
						m_ListDevice.SetItemState( index , LVIS_SELECTED | LVIS_FOCUSED , LVIS_SELECTED | LVIS_FOCUSED );
					}
				}
			}
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgHMDirSel::OnSelChangeComboExType() 
{
	CString SelTypeID = _GetSelTypeID();
	ASSERT( false == SelTypeID.IsEmpty() );

	// Set unchecked the check box for pre-setting.
	if( 0 == _tcscmp( SelTypeID, _T("RVTYPE_FO") ) )
	{
		m_CheckPresetting.SetCheck( BST_UNCHECKED );
		m_CheckPresetting.EnableWindow( FALSE );
		OnCheckPresetting();
	}

	m_ComboFamily.ResetContent();

	// Fill the family combo according to selected type.
	CRank SortList;
	CTable* pTab = (CTable*)( m_pTADB->Get( _T("FAMILY_TAB") ).MP );										ASSERT( NULL != pTab );
	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_StringID ) ); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( ( (CDB_StringID*)IDPtr.MP )->GetIDstr2() == SelTypeID )
		{
			TCHAR* endptr;
			double Key = _tcstod( ( (CDB_StringID*)IDPtr.MP )->GetIDstr(), &endptr );						ASSERT(!*endptr);
			SortList.Add( ( (CDB_StringID*)IDPtr.MP )->GetString(), Key, (LPARAM)(void *)IDPtr.MP );
			
			// Still need to insert directly in the Combo.
			int index = m_ComboFamily.InsertString( 0, ( (CDB_StringID*)IDPtr.MP )->GetString() );
			m_ComboFamily.SetItemDataPtr( index, (void *)IDPtr.MP );
		}
	}
			
	// Sort items in the combo and reinsert them.
	SortList.Transfer( &m_ComboFamily );

	// Try to find at least one Regulating valve available for this type and this family
	// Find the reg. valve type tables.
	CDB_StringID *pTypeID = dynamic_cast<CDB_StringID*>( m_pTADB->Get( SelTypeID ).MP );
	if( NULL == pTypeID )
	{
		ASSERT_RETURN
	}
	
	pTab = dynamic_cast<CTable*>( m_pTADB->Get( pTypeID->GetIDstr2() ).MP );
	if( NULL == pTab )
	{
		ASSERT_RETURN
	}

	for( int i = 0; i < m_ComboFamily.GetCount(); )
	{
		CString strFamily;
		
		// Take family ID from stored MP.
		strFamily = ( (CDB_StringID*)m_ComboFamily.GetItemDataPtr( i ) )->GetIDPtr().ID;
		bool fSelectable = false;
		for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Scan all balancing valve with the same type.
			if( _tcscmp( ( (CDB_TAProduct*)IDPtr.MP )->GetTypeIDPtr().ID, (LPCTSTR)SelTypeID ) ) 
				continue;
			
			// and the same family.
			if( _tcscmp( ( (CDB_TAProduct*)IDPtr.MP )->GetFamilyIDPtr().ID, (LPCTSTR)strFamily ) )
				continue;
			
			// HYS-1701: Use isSelectable function to consider deleted product
			if( false == ((CDB_Thing*)IDPtr.MP)->IsSelectable( true, false, true ) ||
				false == ((CDB_Thing*)IDPtr.MP)->InCbi() )
			{
				continue;
			}
			
			fSelectable = true;
			// Loop aborted one BV of this type is enough to display the type.
			break;					
		}
			
		if( false == fSelectable )
			m_ComboFamily.DeleteString( i );
		else 
			i++;
	}

	// Check the number of families found to enable or disable the combo box.
	m_ComboFamily.EnableWindow( m_ComboFamily.GetCount() != 1 ? TRUE : FALSE );

	m_ComboFamily.SetCurSel( 0 );
	OnSelChangeComboFamily();
}

void CDlgHMDirSel::OnSelChangeComboFamily() 
{
	m_ComboDevice.ResetContent();

	// Fill the device combo according to the selected type and family.
	CRank SortList;
	CString SelTypeID = _GetSelTypeID();
	CString SelFamilyID = _GetSelFamilyID();
	CDB_StringID *pTypeID = dynamic_cast<CDB_StringID*>( m_pTADB->Get( SelTypeID ).MP );
	if( NULL == pTypeID )
	{
		ASSERT_RETURN
	}
	
	CTable *pTab=  dynamic_cast<CTable*>( m_pTADB->Get( pTypeID->GetIDstr2() ).MP );
	if( NULL == pTab )
	{
		ASSERT_RETURN
	}

	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( _T("RVTYPE_FO") == SelTypeID && false == IDPtr.MP->IsClass( CLASS( CDB_FixedOrifice ) ) )
			continue;
		if( _T("RVTYPE_CS") == SelTypeID && false == IDPtr.MP->IsClass( CLASS( CDB_CommissioningSet ) ) )
			continue;
		if( _T("RVTYPE_VV") == SelTypeID && false == IDPtr.MP->IsClass( CLASS( CDB_VenturiValve ) ) )
			continue;

		// HYS-1701: Use isSelectable function to consider deleted product
		if( ( SelFamilyID == ( (CDB_TAProduct*)IDPtr.MP )->GetFamilyIDPtr().ID ) &&
			( true == ((CDB_Thing*)IDPtr.MP)->IsSelectable( true, false, true ) ) &&
			true == ( (CDB_Thing*)IDPtr.MP )->InCbi() )
		{
			// Add only one entry in Combo box for devices with same name.
			if( m_ComboDevice.FindStringExact( -1, ( (CDB_TAProduct*)IDPtr.MP )->GetName() ) < 0 )
			{
				CDB_TAProduct *pclTAProduct = (CDB_TAProduct *)( IDPtr.MP );
				double dKey = (double)pclTAProduct->GetSizeKey();
				SortList.Add( ( (CDB_TAProduct*)IDPtr.MP )->GetName(), dKey, (LPARAM)IDPtr.MP );

				// Still need to insert directly in the Combo.
				int iItem = m_ComboDevice.InsertString( 0, ( (CDB_TAProduct *)IDPtr.MP )->GetName() );
				m_ComboDevice.SetItemDataPtr( iItem, (void*)IDPtr.MP );
			}
		}
	}
	
	// Sort items in the Combo and reinsert them.
	SortList.Transfer( &m_ComboDevice) ;

	// Check the number of devices found to enable or disable the combo box
//	m_ComboDevice.EnableWindow(m_ComboDevice.GetCount()!=1 ? TRUE:FALSE);

	m_ComboDevice.SetCurSel( 0 );
	OnSelChangeComboDevice();
}

void CDlgHMDirSel::OnSelChangeComboDevice() 
{
	int iItem = m_ComboDevice.GetCurSel();

	if( CB_ERR == iItem )
	{
		m_pProd = NULL;
		return;
	}

	void* lp = m_ComboDevice.GetItemDataPtr( iItem );

	if( NULL == lp )
	{
		return;
	}

	m_pProd = (CDB_TAProduct*)lp;

	CString SelTypeID = _GetSelTypeID();

	// Look for the min and max opening of the selected device.
	CString str = TASApp.LoadLocalizedString( IDS_SHEETHDR_SETTING );

	if( 0 != _tcscmp( SelTypeID, _T("RVTYPE_FO") ) )
	{
		CDB_ValveCharacteristic *pclValveCharacteristic = NULL;

		if( 0 == _tcscmp( SelTypeID, _T("RVTYPE_CS") ) )
		{
			CDB_CommissioningSet *pCSDev = (CDB_CommissioningSet *)m_pProd;

			if( NULL == pCSDev->GetDrvCurve() )
			{
				return;
			}

			pclValveCharacteristic = pCSDev->GetDrvCurve();
		}
		else if( 0 == _tcscmp( SelTypeID, _T("RVTYPE_VV") ) )
		{
			CDB_VenturiValve *pVVDev = (CDB_VenturiValve *)m_pProd;

			if( NULL != pVVDev->GetVvCurve() )
			{
				pclValveCharacteristic = pVVDev->GetVvCurve();
			}
		}
		else 
		{
			CDB_RegulatingValve *pRVDev = dynamic_cast<CDB_RegulatingValve *>( m_pProd );

			if( NULL != pRVDev && NULL != pRVDev->GetValveCharacteristic() )
			{
				pclValveCharacteristic = pRVDev->GetValveCharacteristic();
			}
		}

		// Set the limits for the CExtNumEdit box.
		m_dMinOpening = pclValveCharacteristic->GetOpeningMin();
		m_dMaxOpening = pclValveCharacteristic->GetOpeningMax();
		m_EditPresetting.SetMinDblValue( m_dMinOpening );
		m_EditPresetting.SetMaxDblValue( m_dMaxOpening );
    
		// Set the min max values in the static.
		str += _T(" ( ") + pclValveCharacteristic->GetSettingString( m_dMinOpening );
		str +=  _T(" - ") + pclValveCharacteristic->GetSettingString( m_dMaxOpening ) + _T(" )");
		GetDlgItem( IDC_CHECKPRESETTING  )->SetWindowText( str );
	}
	else
	{
		GetDlgItem( IDC_CHECKPRESETTING  )->SetWindowText( str );
	}

	// Fill the device list control according to the selected valve name and associated type and family.
	m_ListDevice.DeleteAllItems();

	LVITEM lvItem;	
	lvItem.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT; 
	lvItem.iItem = 0; 
	lvItem.iSubItem = 0; 
	lvItem.state = 0;
	lvItem.stateMask = 0; 
	lvItem.pszText = NULL; 
	lvItem.cchTextMax = _STRING_LENGTH; 
	lvItem.iImage = 0; 
	lvItem.lParam = NULL;
	lvItem.iIndent = 1;

	CString SelFamilyID = _GetSelFamilyID();
	CString SelDevice = m_pProd->GetName();
	bool bOneDeleted = false;
	CRank SortList;
	CDB_StringID *pTypeID = dynamic_cast<CDB_StringID*>( m_pTADB->Get( SelTypeID ).MP );

	if( NULL == pTypeID )
	{
		ASSERT_RETURN
	}
	
	CTable *pTab = dynamic_cast<CTable*>( m_pTADB->Get( pTypeID->GetIDstr2() ).MP );

	if( NULL == pTab )
	{
		ASSERT_RETURN
	}
	
	for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( SelTypeID == _T("RVTYPE_FO") && false == IDPtr.MP->IsClass( CLASS( CDB_FixedOrifice ) ) )
		{
			continue;
		}
		
		if( SelTypeID == _T("RVTYPE_CS") && false == IDPtr.MP->IsClass( CLASS( CDB_CommissioningSet ) ) )
		{
			continue;
		}
		
		if( SelTypeID == _T("RVTYPE_VV") && false == IDPtr.MP->IsClass( CLASS( CDB_VenturiValve ) ) )
		{
			continue;
		}

		// HYS-1701: Use isSelectable function to consider deleted product
		if( ( SelFamilyID == ( (CDB_TAProduct*)IDPtr.MP )->GetFamilyIDPtr().ID ) &&
			( SelDevice == ( (CDB_TAProduct*)IDPtr.MP )->GetName() ) &&
			( true == ((CDB_Thing*)IDPtr.MP)->IsSelectable( true, false, true ) ) &&
			( (CDB_Thing*)IDPtr.MP )->InCbi() )
		{
			SortList.Add( IDPtr.ID, 100 - ( (CDB_TAProduct*)IDPtr.MP )->GetPriorityLevel(), (LPARAM)IDPtr.MP );
		}
	}
	
	CDB_TAProduct* pProd;
	int flag = SortList.GetFirst( str, (LPARAM &)pProd );

	while( flag )
	{
		// Add entry into the list control.
		int col = 0;

		if( false == TASApp.IsTAArtNumberHidden() )
		{
			lvItem.pszText = (TCHAR *)pProd->GetArtNum();
			lvItem.lParam = (LPARAM) pProd;
			m_ListDevice.InsertItem(&lvItem);	
			col++;

			if( true == TASApp.IsLocalArtNumberUsed() )
			{
				CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( pProd->GetArtNum() );

				if( true == strLocArtNum.IsEmpty() )
				{
					m_ListDevice.SetItem( 0, col, LVIF_TEXT, _T("-"), 0, 0, 0, 0 );
				}
				else
				{
					m_ListDevice.SetItem( 0, col, LVIF_TEXT, strLocArtNum, 0, 0, 0, 0 );
				}

				col++;
			}
		}
		else
		{
			CString strLocArtNum = LocArtNumTab.GetLocalArticleNumber( pProd->GetArtNum() );

			if( true == strLocArtNum.IsEmpty() )
			{
				lvItem.pszText = _T("-");
			}
			else
			{
				lvItem.pszText = (TCHAR *)( strLocArtNum.GetBuffer() );
			}

			lvItem.lParam = (LPARAM) pProd;
			m_ListDevice.InsertItem( &lvItem );
			col++;
		}

		m_ListDevice.SetItem( 0, col, LVIF_TEXT, pProd->GetConnect(), 0, 0, 0, 0 );
		col++;
		m_ListDevice.SetItem( 0, col, LVIF_TEXT, pProd->GetVersion(), 0, 0, 0, 0 );

		if( true == pProd->IsDeleted() )
		{
			bOneDeleted = true;
		}

		flag = SortList.GetNext( str, (LPARAM &)pProd );
	};

	m_ListDevice.SetFocus();
	m_ListDevice.SetItemState(0 , LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED  );
	GetDlgItem( IDC_STATICDELETED )->ShowWindow( true == bOneDeleted ? SW_SHOW : SW_HIDE );
}

HBRUSH CDlgHMDirSel::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor ) 
{
	HBRUSH hbr;
	hbr = CDialogEx::OnCtlColor( pDC, pWnd, nCtlColor );
	
	if( pWnd->m_hWnd == ( (CButton*)GetDlgItem( IDC_CHECKPRESETTING ) )->m_hWnd )
	{
		if( -999.0 == m_dPresetting )
			pDC->SetTextColor( RGB( 255, 0, 0 ) );
		else
			pDC->SetTextColor( RGB( 0, 0, 0 ) );
	}
	
	return hbr;
}

void CDlgHMDirSel::OnCheckPresetting() 
{
	if( BST_CHECKED == m_CheckPresetting.GetCheck() )
	{
		m_EditPresetting.SetWindowText( _T("") );
		m_EditPresetting.EnableWindow( TRUE );
		GetDlgItem( IDC_STATICPRESETUNIT )->EnableWindow( TRUE );
	}
	else
	{
		CString str;
		str = TASApp.LoadLocalizedString( IDS_UNKNOWN );
		m_dPresetting = -1.0;
		m_EditPresetting.SetWindowText( str );
		m_EditPresetting.EnableWindow( FALSE );
		GetDlgItem( IDC_STATICPRESETUNIT )->EnableWindow( FALSE );
	}
}

void CDlgHMDirSel::OnChangeEditPresetting() 
{
	if( GetFocus() != &m_EditPresetting || m_EditPresetting.GetStyle() & ES_READONLY )
		return; 

	CString str;

	m_dPresetting = -1.0;

	switch( ReadDouble( m_EditPresetting, &m_dPresetting ) ) 
	{
		case RD_EMPTY:
			return;

		case RD_NOT_NUMBER:
			m_EditPresetting.GetWindowText( str );
			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || 
				_T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) )
				return;
		
		case RD_OK:
			// Update all fields.
			if( 0.0 == m_dPresetting )
				return;
			if( m_dPresetting < m_dMinOpening || m_dPresetting > m_dMaxOpening )
				m_dPresetting = -999.0; // Show setting range in red
			
			// Update display
			( (CStatic*)GetDlgItem( IDC_CHECKPRESETTING ) )->Invalidate();
			( (CStatic*)GetDlgItem( IDC_CHECKPRESETTING ) )->UpdateWindow();
			if( -999.0 == m_dPresetting )
				return;
			break;
	};
}

void CDlgHMDirSel::OnItemChangedListDevice( NMHDR* pNMHDR, LRESULT* pResult ) 
{
	int iItem;
	if( NULL != pNMHDR )
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		iItem = pNMListView->iItem;
	}
	else
		iItem = m_ListDevice.GetNextItem( -1, LVNI_SELECTED );

	// Found selected Item and take its ID.
	LPARAM lp = m_ListDevice.GetItemData( iItem );
	if( NULL == lp )
		return;
	IDPTR IDPtr = ( (CDB_HydroThing*)lp )->GetIDPtr();				ASSERT( '\0' != *IDPtr.ID );
	m_SelThingID = IDPtr.ID;
	
	*pResult = 0;
}

void CDlgHMDirSel::_FillComboexType( LPCTSTR ptcType )
{
	CString str;
	
	// Find the reg. valve type tables.
	CTable* pTab = NULL;
	CArray<CString> arTab;
	std::map <int, CDB_StringID *> mapType;
	std::map <int, CDB_StringID *>::iterator mapTypeIT;
	
	arTab.Add( _T("RVTYPE_TAB") );
	arTab.Add( _T("BCVTYPE_TAB") );
	arTab.Add( _T("PICVTYPE_TAB") );
	arTab.Add( _T("DPCBCVTYPE_TAB") );
	
	int j = 0;

	for(int i = 0; i < arTab.GetCount(); i++ )
	{
		pTab = (CTable*)( m_pTADB->Get( arTab.GetAt( i ) ).MP );
		ASSERT( NULL != pTab );

		// Loop on all entries of the table and insert data in vector cbi
		for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext( IDPtr.MP ) )
		{
			CDB_StringID *pstrI = dynamic_cast<CDB_StringID*>( IDPtr.MP );

			if( NULL != pstrI )
			{
				j = i * 1000 + _ttoi( pstrI->GetIDstr() );
				mapType[j] = pstrI;
			}
		}
	}
	
	j = 0;
	for( mapTypeIT = mapType.begin(); mapTypeIT != mapType.end(); mapTypeIT++ )
	{
		// Insert sorted Item into the combo.
		CString str = mapTypeIT->second->GetString();
		m_ComboType.InsertString( j, (LPCTSTR)str );
		m_ComboType.SetItemData( j, (DWORD_PTR)mapTypeIT->second );
		j++;
	}
	
	// Try to find at least one Regulating valve available for this type.

	for( int i = 0; i < m_ComboType.GetCount(); )
	{
		CDB_StringID *pTypeID = dynamic_cast<CDB_StringID*>( (CData*)m_ComboType.GetItemDataPtr( i ) );

		if( NULL == pTypeID )
		{
			ASSERT_RETURN
		}

		pTab = dynamic_cast<CTable*>( m_pTADB->Get( pTypeID->GetIDstr2() ).MP );

		if( NULL == pTab )
		{
			ASSERT_RETURN
		}

		CString strType = pTypeID->GetIDPtr().ID;
		bool fSelectable = false;

		for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Scan all balancing valve with the same type.
			if( _tcscmp( ( (CDB_TAProduct*)IDPtr.MP )->GetTypeIDPtr().ID, (LPCTSTR)strType ) ) 
			{
				continue;
			}
			
			// HYS-1701: Use isSelectable function to consider deleted product
			if( false == ( (CDB_Thing*)IDPtr.MP )->IsSelectable( true, false, true ) ||
				false == ( (CDB_Thing*)IDPtr.MP )->InCbi() )
			{
				continue;
			}

			fSelectable = true;
			// Loop aborted one BV of this type is enough to display the type
			break;					
		}

		if( false == fSelectable )
		{
			m_ComboType.DeleteString( i );
		}
		else
		{
			i++;
		}
	}

	// Change family.
	if( NULL != ptcType && ( m_ComboType.FindStringExact( -1, ptcType ) != CB_ERR ) )
	{
		m_ComboType.SetCurSel( m_ComboType.FindStringExact( -1, ptcType ) );
	}
	else
	{
		m_ComboType.SetCurSel( 0 );
	}

	OnSelChangeComboExType();
}

LPCTSTR CDlgHMDirSel::_GetSelTypeID()
{
	if( CB_ERR == m_ComboType.GetCurSel() )
	{
		return _T("");
	}

	ASSERT( 0 != m_ComboType.GetCount() );
	m_ComboType.GetLBText( m_ComboType.GetCurSel(), m_SelType );
	
	return ( (CDB_StringID*)( m_ComboType.GetItemDataPtr( m_ComboType.GetCurSel() ) ) )->GetIDPtr().ID;
}

LPCTSTR CDlgHMDirSel::_GetSelFamilyID()
{
	ASSERT( m_ComboFamily.GetCount() );

	return ( (CDB_StringID*)( m_ComboFamily.GetItemDataPtr( m_ComboFamily.GetCurSel() ) ) )->GetIDPtr().ID;
}
