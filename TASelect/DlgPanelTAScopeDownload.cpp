#include "stdafx.h"
#include <algorithm>
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizTAScope.h"
#include "DlgPanelTAScopeDownload.h"


CDlgPanelTAScopeDownload::CDlgPanelTAScopeDownload( CWnd* pParent )
	: CDlgWizard( CDlgPanelTAScopeDownload::IDD, pParent )
{
	m_pParent = NULL;
	m_uiNumberOfQM = 0;
	m_uiNumberOfLog = 0;
	m_strPlantName = _T("");
	m_nTimer = (UINT_PTR)0;
	m_pTree = NULL;
}

BEGIN_MESSAGE_MAP( CDlgPanelTAScopeDownload, CDlgWizard )
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_NOTIFY( NM_CLICK, IDC_TREEQMLOG, OnNMClickTree )
	ON_NOTIFY( NM_CLICK, IDC_TREEPLANT, OnNMClickTree )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREEQMLOG, OnTvnSelChangedTreeQMLog )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREEPLANT, OnTvnSelChangedTreePlant )
	ON_MESSAGE( WM_USER_CHECKSTATECHANGE, OnCheckStateChange )
END_MESSAGE_MAP()

void CDlgPanelTAScopeDownload::DoDataExchange( CDataExchange* pDX )
{
	CDlgWizard::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITCONTENT, m_EditContent );
	DDX_Control( pDX, IDC_STATICVERSION, m_StaticVersionNbr );
	DDX_Control( pDX, IDC_TREEQMLOG, m_TreeQMLog );
	DDX_Control( pDX, IDC_TREEPLANT, m_TreePlant );
	DDX_Control( pDX, IDC_EDITCONTENTPLANT, m_EditPlantContent );
}

BOOL CDlgPanelTAScopeDownload::OnInitDialog()
{
	CDlgWizard::OnInitDialog();
	m_pParent = (CDlgWizTAScope *)GetWizMan();
	
	// Set the white rectangle to the first drawn window.
	GetDlgItem( IDC_STATICRECTANGLE )->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );

	GetDlgItem( IDC_EDITTITLE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELDOWNLOAD_TITLE ) );
	GetDlgItem( IDC_STATICDETECTEDTASCOPE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_VERSIONDETECTED ) );
	GetDlgItem( IDC_STATICQMLOGDESCRITION )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_STATICQMLOGDESCRIPTION ) );
	GetDlgItem( IDC_STATICPLANTDESCRITION )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_STATICPLANTDESCRITION ) );
	m_uiNumberOfQM = 0;
	m_uiNumberOfLog = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPanelTAScopeDownload::OnTimer( UINT_PTR nIDEvent )
{
	CDlgWizard::OnTimer( nIDEvent );
	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
		return;

	CString str, str1;
	m_StaticVersionNbr.GetWindowText( str );
	if( true == m_pParent->IsComForcedOffLine() )
	{
		KillTimer( nIDEvent );
		m_nTimer = (UINT_PTR)0;
		RefreshTreeQMLog();
	}
	else if( str != m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) )
	{
		m_pParent->ActivateRefresh( false );
		m_StaticVersionNbr.SetWindowText( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) );
		m_pParent->WaitMilliSec( 200 );
		BeginWaitCursor();
		RefreshTreeQMLog();
		RefreshTreePlant();
		EndWaitCursor();
		m_pParent->ActivateRefresh( true );
	}
}

HBRUSH CDlgPanelTAScopeDownload::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH hbr;
	if( pWnd->m_hWnd == GetDlgItem( IDC_EDITTITLE)->m_hWnd )
	{
		pDC->SetTextColor( RGB( 0, 0, 0 ) );
		pDC->SetBkColor( RGB( 255, 255, 255 ) );
		return (HBRUSH)m_brWhiteBrush;
	}
	
	hbr = CDlgWizard::OnCtlColor( pDC, pWnd, nCtlColor );
	return hbr;
}

void CDlgPanelTAScopeDownload::OnNMClickTree( NMHDR* pNMHDR, LRESULT* pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state
	// of the item has been changed, you can just determine that the user has clicked the state Icon
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is 
	// post a user defined message as a notification that the check state has changed
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= ((int)(short)LOWORD( dwpos ) );
	ht.pt.y= ((int)(short)HIWORD( dwpos ) );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( ( TVHT_ONITEMSTATEICON & ht.flags ) || ( TVHT_ONITEM & ht.flags ) )
	{
		WPARAM bState = 0;
		m_pTree = NULL;
		if( pNMHDR->hwndFrom == m_TreeQMLog.GetSafeHwnd() )
		{
			bState = m_TreeQMLog.GetCheck( ht.hItem );
			m_pTree = &m_TreeQMLog;
		}
		else if( pNMHDR->hwndFrom == m_TreePlant.GetSafeHwnd() )
		{
			bState = m_TreePlant.GetCheck( ht.hItem );
			m_pTree = &m_TreePlant;
		}
		m_pTree->SelectItem( ht.hItem );
		::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, bState, (LPARAM)ht.hItem );
	}
	if( NULL != pResult )
		*pResult = 0;
}

void CDlgPanelTAScopeDownload::OnTvnSelChangedTreeQMLog( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>( pNMHDR );

	// Determine where does click come from.
	if( NULL != pResult )
		*pResult = 0;
	
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = GET_X_LPARAM( dwpos );
	ht.pt.y = GET_Y_LPARAM( dwpos );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	m_EditContent.SetWindowText( _T("") );
	if( NULL == hItem )
		return;
	
	LPARAM lparam= m_TreeQMLog.GetItemData( hItem );
	if( NULL == lparam )
		return;
	
	CData* pData = (CData*)lparam;
	CDS_LogDataX* pLogDataX = dynamic_cast<CDS_LogDataX*>( pData );
	CDS_HydroModX* pHMX = dynamic_cast<CDS_HydroModX*>( pData );
	if( NULL != pLogDataX )
	{
		CString str;
		__time32_t t; 
		pLogDataX->GetStartingDateTime( &t );
		CTimeUnic dtu( (__time64_t)t );
		str = CString( dtu.FormatGmt( IDS_DATE_FORMAT ) + CString( _T(" ") ) + dtu.FormatGmt( IDS_TIME_FORMAT ) + CString( _T("\r\n") ) );
		switch( pLogDataX->GetLogType() )
		{
			case CDS_LogDataX::LogType::LOGTYPE_DP:
				str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_DP) + CString( _T("\r\n") );
				break;
			
			case CDS_LogDataX::LogType::LOGTYPE_FLOW:
				str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_FLOW) + CString( _T("\r\n") );
				break;
			
			case CDS_LogDataX::LogType::LOGTYPE_TEMP:
				str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_TEMP) + CString( _T("\r\n") );
				break;
			
			case CDS_LogDataX::LogType::LOGTYPE_POWER:
				str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_POWER) + CString( _T("\r\n") );
				break;
			
			case CDS_LogDataX::LogType::LOGTYPE_DT:
				str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_DIFTEMP) + CString( _T("\r\n") );
				break;
			
			case CDS_LogDataX::LogType::LOGTYPE_DPTEMP:
				str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_DPANDTEMP) + CString( _T("\r\n") );
				break;
			
			case CDS_LogDataX::LogType::LOGTYPE_FLOWTEMP:
				str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_QANDTEMP) + CString( _T("\r\n") );
				break;

			default:
				break;
		}

		if( false == pLogDataX->GetDescription().empty() )
			str += CString( pLogDataX->GetDescription().c_str() );
		
		m_EditContent.SetWindowText( str );
	}
	else if( NULL != pHMX )
	{
		CString str;
		__time32_t t = pHMX->GetMeasDateTime();
		CTimeUnic dtu( (__time64_t)t );
		str = dtu.FormatGmt( IDS_DATE_FORMAT ) + CString( _T(" ") ) + dtu.FormatGmt( IDS_TIME_FORMAT ) + CString( _T("\r\n") );

		CDS_QMTable* pQMXTab = dynamic_cast<CDS_QMTable *>( pHMX->GetIDPtr().PP );
		for( int k = 0; k < 2; k++ )
		{
			if( k > 0 )
			{
				CDS_HydroModX* pQM2X = pQMXTab->GetAssociatedHM( pHMX );
				if( NULL != pQM2X )
					pHMX = pQM2X;
			}

			if( NULL != pHMX )
			{
				switch (pHMX->GetQMType())
				{
					case CDS_HydroModX::eQMtype::QMDp:
						str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_DP) + CString( _T("\r\n") );
						break;
					
					case CDS_HydroModX::eQMtype::QMFlow:
						str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_FLOW) + CString( _T("\r\n") );
						break;
					
					case CDS_HydroModX::eQMtype::QMTemp:
						str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_TEMP) + CString( _T("\r\n") );
						break;
					
					case CDS_HydroModX::eQMtype::QMPower:
						str += TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_LOGQMTYPE_POWER) + CString( _T("\r\n") );
						break;
					
					default:
						break;
				}
			}
		}
		
		if( false == pHMX->GetDescription()->empty() )
			str += CString( pHMX->GetDescription()->c_str() );

		m_EditContent.SetWindowText( str );
	}
}

void CDlgPanelTAScopeDownload::OnTvnSelChangedTreePlant( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>( pNMHDR );
	
	if( NULL != pResult )
		*pResult = 0;

	m_EditPlantContent.SetWindowText( _T("") );

	// Determine where does click come from.
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= GET_X_LPARAM( dwpos );
	ht.pt.y= GET_Y_LPARAM( dwpos );
	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if( NULL == hItem )
		return;
	
	LPARAM lparam = m_TreePlant.GetItemData( hItem );
	if( NULL == lparam )
		return;
	
	CData* pData = (CData*)lparam;
	CDS_PlantX* pPlant = dynamic_cast<CDS_PlantX*>( pData );
	if( NULL != pPlant )
	{
		CString str = pPlant->GetString( CDS_PlantX::Name );

		CDS_HMXTable::sHMXTab* pHMXTabData = pPlant->GetpHMXTab();
		// If at least one module exist...
		if( NULL != pHMXTabData )
		{
			CString str, Fullstr;
			FormatString( str, IDS_DLGPANELDOWNLOAD_TOTALFLOW, WriteCUDouble( _U_FLOW, pHMXTabData->dHMXTotalQ, true ) );
			Fullstr = str + CString( _T("\r\n") );
			FormatString( str, IDS_DLGPANELDOWNLOAD_BALANCEDMODULES, to_string<uint16_t>( pHMXTabData->HMXBalMod ).c_str(), to_string<uint16_t>( pHMXTabData->HMXNbrMod ).c_str() );
			Fullstr += str + CString( _T("\r\n") );
			FormatString( str, IDS_DLGPANELDOWNLOAD_ADJUSTEDVALVES, to_string<uint16_t>( pHMXTabData->HMXAdjValve ).c_str(), to_string<uint16_t>( pHMXTabData->HMXNbrValve ).c_str() );
			Fullstr += str;
			m_EditPlantContent.SetWindowText( Fullstr );
		}
	}

	if( NULL != pResult )
		*pResult = 0;
}

LRESULT CDlgPanelTAScopeDownload::OnCheckStateChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pTree )
		return 0;

	HTREEITEM hItem = (HTREEITEM)lParam;
	BOOL bState = wParam & 0x01;
	LPARAM lparam = m_pTree->GetItemData( hItem );
	
	// If user clicks on the parent item...
	if( m_pTree == &m_TreeQMLog )
	{
		if( NULL == lparam && m_TreeQMLog.ItemHasChildren( hItem ) )
		{
			bool fAtLeastOneChildren = false;
			HTREEITEM hNextItem;
			HTREEITEM hChildItem = m_TreeQMLog.GetChildItem( hItem );
			while( NULL != hChildItem )
			{
				hNextItem = m_TreeQMLog.GetNextItem( hChildItem, TVGN_NEXT );
				m_TreeQMLog.SetCheck( hChildItem, !bState );
				hChildItem = hNextItem;
				fAtLeastOneChildren = true;
			}
			
			if( false == fAtLeastOneChildren )
				m_TreeQMLog.SetCheck( hItem, false );
			m_TreeQMLog.Invalidate();
		}
	}
	else // Only one Plant
	{
		// Uncheck all.
		HTREEITEM hNextItem = m_TreePlant.GetRootItem();
		while( NULL != hNextItem )
		{
			m_TreePlant.SetCheck( hNextItem, false );
			hNextItem = m_TreePlant.GetNextItem( hNextItem, TVGN_NEXT );
		}
		
		if( FALSE == bState )	// One Item should be checked
		{
			m_TreePlant.SetCheck( hItem, true );
		}
	}
	_CheckIsAtLeastOneItemSelected();
	return 0;
}

BOOL CDlgPanelTAScopeDownload::OnWizCancel()
{
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
	return TRUE;
}

BOOL CDlgPanelTAScopeDownload::OnWizFinish()
{
	// The 'Finish' button in the 'CDlgWizTAScope' is used as 'Cancel'.
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
	m_pParent->JumpToPanel( CDlgWizTAScope::epanStart );
	return FALSE;
}

BOOL CDlgPanelTAScopeDownload::OnWizNext()
{
	// The 'Next' button in the 'CDlgWizTAScope' is used as 'Download'.
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;

	// This order must be respected (first HMX / after QM and logs).
	DownLoadHMX();
	FillDataFromTADSX();

	if( 0 != m_uiNumberOfQM || 0 != m_uiNumberOfLog|| false == m_strPlantName.IsEmpty() )
	{
		CString str, str1, str2;
		str = TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_DOWNLOADED );
		if( 0 != m_uiNumberOfQM )
		{
			str2.Format( _T("%d"), m_uiNumberOfQM );
			FormatString( str1, IDS_DLGPANELDOWNLOAD_QMNBR, str2 );
			str += _T("\r\n") + str1;
		}
		
		if( 0 != m_uiNumberOfLog )
		{
			str2.Format( _T("%d"), m_uiNumberOfLog );
			FormatString( str1, IDS_DLGPANELDOWNLOAD_DATALOGNBR, str2 );
			str += _T("\r\n") + str1;
		}
		
		if( false == m_strPlantName.IsEmpty() )
		{
			FormatString( str1, IDS_DLGPANELDOWNLOAD_PLANTNAME, m_strPlantName );
			str += _T("\r\n") + str1;
		}
		m_pParent->m_DlgPanelStart.SetBtnInfoText( IDC_STATICINFOBTNDOWNLOAD, str );
	}	
	
	// Reset the upper and lower limits for measurements.
	pMainFrame->ResetMeasurementsLimits();
	
	// UnFreeze the project, reset the tree and pass in Non-HMCalc mode.
	pMainFrame->SetHMCalcMode( false );
	
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	pPrjParam->SetFreeze( false );

	m_pParent->JumpToPanel( CDlgWizTAScope::epanStart );
	return FALSE;
}

bool CDlgPanelTAScopeDownload::OnAfterActivate()
{
	BeginWaitCursor();
	
	// 'Next' and 'Finish' buttons.
	m_pParent->ShowButtons( CWizardManager::WizButNext | CWizardManager::WizButBack, false );

	m_TreeQMLog.DeleteAllItems();
	m_TreePlant.DeleteAllItems();
	m_EditContent.SetWindowText( _T("") );
	m_EditPlantContent.SetWindowText( _T("") );
	m_StaticVersionNbr.SetWindowText( _T("") );
	m_strPlantName = _T("");
	m_uiNumberOfQM = 0;
	m_uiNumberOfLog = 0;

	m_pParent->m_DlgPanelStart.SetBtnInfoText( IDC_STATICINFOBTNDOWNLOAD, _T("") );

	// Discharge the current project.
	m_pParent->ActivateRefresh( false );
	m_pParent->WaitMilliSec( 200 );

	// Just refreshing the connection status.
	m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEDOWNLOAD, 1000, 0 );					
	m_MetaData.Init();
	m_TADSX.Init();

	if( true == m_pParent->IsComForcedOffLine() )
	{
		m_pParent->ShowButtons( CWizardManager::WizButNext, true );
	}

	EndWaitCursor();
	
	// Force first refresh.
	OnTimer( m_nTimer );
	return true;
}

int CDlgPanelTAScopeDownload::OnWizButFinishTextID()
{
	return IDS_DLGPANELDOWNLOAD_CANCEL;
}

int CDlgPanelTAScopeDownload::OnWizButNextTextID()
{
	return IDS_DLGPANELDOWNLOAD_DOWNLOAD;
}

void CDlgPanelTAScopeDownload::FillDataFromTADSX()
{
	try
	{
		HTREEITEM hNextItem, hSibbling;
		HTREEITEM hChildItem = m_TreeQMLog.GetRootItem();
	
		CDS_LogTable *pLDTab = dynamic_cast<CDS_LogTable *>( TASApp.GetpTADS()->GetpLogDataTable() );
		
		if( NULL == pLDTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'LOGDATA_TAB' table from the datastruct.") );
		}

		CDS_QMTable *pQMTab = dynamic_cast<CDS_QMTable *>( TASApp.GetpTADS()->GetpQuickMeasureTable() );
		
		if( NULL == pQMTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'QUICKMEAS_TAB' table from the datastruct.") );
		}

		int iMeasDateTime = 1;
		m_uiNumberOfQM = 0;
		m_uiNumberOfLog = 0;

		hSibbling = m_TreeQMLog.GetRootItem();

		while( NULL != hSibbling )
		{
			CString str = m_TreeQMLog.GetItemText( hSibbling ); 
			hChildItem = m_TreeQMLog.GetChildItem( hSibbling );
		
			while( NULL != hChildItem )
			{
				CString str = m_TreeQMLog.GetItemText( hChildItem );
				hNextItem = m_TreeQMLog.GetNextItem( hChildItem, TVGN_NEXT );
			
				if( TRUE == m_TreeQMLog.GetCheck( hChildItem ) )
				{
					LPARAM lparam= m_TreeQMLog.GetItemData( hChildItem );
				
					if( NULL != lparam )
					{
						CData *pData = (CData *)lparam;
						CDS_LogDataX *pLogX = dynamic_cast<CDS_LogDataX *>( pData );

						if( NULL != pLogX )
						{
							// Data logging.
							IDPTR idptr;
						
							// Verify if the log is not already inserted.
							if( true == pLDTab->VerifyLogExist( pLogX, &idptr ) )
							{
								CDS_LogDataX *pLog = dynamic_cast<CDS_LogDataX *>( idptr.MP );

								if( NULL == pLog )
								{
									HYSELECT_THROW( _T("Internal error: Object '%s' is not a 'CDS_LogDataX' object."), idptr.ID );
								}

								pLogX->Copy( pLog );
								m_uiNumberOfLog;
							}
							else
							{
								TASApp.GetpTADS()->CreateObject( idptr, CLASS( CDS_LogDataX ) );
								CDS_LogDataX *pLog = dynamic_cast<CDS_LogDataX *>( idptr.MP );

								pLogX->Copy( pLog );
								pLDTab->Insert( (IDPTR&)pLog->GetIDPtr() );
								m_uiNumberOfLog++;
							}
						}
						else
						{	
							// Quick measurement.
							CDS_HydroModX *pQMX = dynamic_cast<CDS_HydroModX *>( pData );
							CTable *pTabWC = dynamic_cast<CTable *>( TASApp.GetpTADS()->Get( _T("WC_TAB") ).MP );
							CDS_QMTable *pQMXTab = dynamic_cast<CDS_QMTable *>( pQMX->GetIDPtr().PP );

							for( int k = 0; k < 2; k++ )
							{
								if( k > 0 )
								{
									pQMX = pQMXTab->GetAssociatedHM( pQMX );
								}
							
								if( NULL != pQMX )
								{
									// Before importing quick measurements, we verify bad measures (Sometimes the quick measurements imported from the TA 
									// Scope were bad (negative values)).
									__time32_t time = pQMX->GetMeasDateTime();

									if( time < 0 && pQMX->GetQMType() != CDS_HydroModX::QMundef )
									{
										pQMX->SetMeasDateTime( iMeasDateTime++ );
									}

									// For quick measure, TABalDateTime must be always set to 0.
									pQMX->SetTABalDateTime( 0 );

									IDPTR idptr;
									// Verify the QM is not already inserted.
									if( true == pQMTab->VerifyQMExist( pQMX, &idptr ) )
									{
										CDS_HydroModX *pQM = dynamic_cast<CDS_HydroModX *>( idptr.MP );
										
										if( NULL == pQM )
										{
											HYSELECT_THROW( _T("Internal error: Object '%s' is not a 'CDS_HydroModX' object."), idptr.ID );
										}

										m_uiNumberOfQM++;
										pQMX->Copy( pQM );
										
										// Verify if the waterchar already exist in the table. If not, added it with the same id.
										if( NULL == pQM->GetWaterCharIDPtr().MP )
										{
											CDS_WaterCharacteristic *pWC = dynamic_cast<CDS_WaterCharacteristic *>( pQMX->GetWaterCharIDPtr().MP );
											
											if( NULL == pWC )
											{
												HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CDS_WaterCharacteristic' object from the object '%s'."), pQMX->GetIDPtr().ID );
											}

											IDPTR IDPtr;
											TASApp.GetpTADS()->CreateObject( IDPtr, CLASS( CDS_WaterCharacteristic ), pQMX->GetWaterCharIDPtr().ID );
												
											CDS_WaterCharacteristic *pQMWC = dynamic_cast<CDS_WaterCharacteristic *>( IDPtr.MP );
											*pQMWC->GetpWCData() = *pWC->GetpWCData();

											if( NULL != pTabWC )
											{
												pTabWC->Insert( IDPtr );
											}
										}
									}
									else
									{
										TASApp.GetpTADS()->CreateObject( idptr, CLASS( CDS_HydroModX ) );
										CDS_HydroModX *pQM = dynamic_cast<CDS_HydroModX *>( idptr.MP );
										
										if( NULL == pQM )
										{
											HYSELECT_THROW( _T("Internal error: Object '%s' is not a 'CDS_HydroModX' object."), idptr.ID );
										}

										m_uiNumberOfQM++;
										pQMX->Copy( pQM );
										
										// Verify if the waterchar already exist in the table. If not, added it with the same id.
										if( NULL == pQM->GetWaterCharIDPtr().MP )
										{
											CDS_WaterCharacteristic *pWC = dynamic_cast<CDS_WaterCharacteristic *>( pQMX->GetWaterCharIDPtr().MP );
											
											if( NULL == pWC )
											{
												HYSELECT_THROW( _T("Internal error: Can't retrieve the 'CDS_WaterCharacteristic' object from the object '%s'."), pQMX->GetIDPtr().ID );
											}
											
											IDPTR IDPtr;
											TASApp.GetpTADS()->CreateObject( IDPtr, CLASS( CDS_WaterCharacteristic ), pQMX->GetWaterCharIDPtr().ID );
												
											CDS_WaterCharacteristic* pQMWC = dynamic_cast<CDS_WaterCharacteristic *>( IDPtr.MP );
											*pQMWC->GetpWCData() = *pWC->GetpWCData();
											
											if( NULL != pTabWC )
											{
												pTabWC->Insert( IDPtr );
											}
										}
										
										pQMTab->Insert( (IDPTR &)pQM->GetIDPtr() );
									}
								}
							}
						}
					}
				}
			
				hChildItem = hNextItem;
			}
		
			hSibbling = m_TreeQMLog.GetNextSiblingItem( hSibbling );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgPanelTAScopeDownload::FillDataFromTADSX'."), __LINE__, __FILE__ );
		throw;
	}
}

bool CDlgPanelTAScopeDownload::DownLoadHMX( bool bTestMode, CString strOfflineTSCFileName )
{
	try
	{
		CString strTargetFileName;
		bool bReturn = false;
		CDS_PlantX *pPlant = NULL;

		// 2017-01-13: 'bTestMode' can be true if this method is called by 'CTabDlgSpecActTesting::_InterpretCmd'.
		if( true == bTestMode || true == m_pParent->IsComForcedOffLine() )
		{
			if( true == strOfflineTSCFileName.IsEmpty() )
			{
				CString str = L"TA Scope files (*.tsc)|*.tsc|All files (*.*)|*.*||";
				CFileDialog dlg( true, _T("tsc"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, (LPCTSTR)str, NULL );

				if( IDOK != dlg.DoModal() )
				{
					return false;
				}

				strTargetFileName = dlg.GetPathName();
			}
			else
			{
				strTargetFileName = strOfflineTSCFileName;
			}
					
			bReturn = true;
		}
		else
		{
			// Found checked item.
			HTREEITEM hNextItem = m_TreePlant.GetRootItem();
			
			while( NULL != hNextItem )
			{
				if( TRUE == m_TreePlant.GetCheck( hNextItem ) )
				{
					break;
				}

				hNextItem = m_TreePlant.GetNextItem( hNextItem, TVGN_NEXT );
			}
			
			if( NULL != hNextItem )
			{
				LPARAM lparam = m_TreePlant.GetItemData( hNextItem );

				if( NULL == lparam )
				{
					return false;
				}
				
				CData *pData = (CData *)lparam;
				pPlant = dynamic_cast<CDS_PlantX *>( pData );

				if( NULL != pPlant )
				{
					// Plant name.
					CString TAScopePlantName = pPlant->GetString( CDS_PlantX::Name );
					m_strPlantName = TAScopePlantName;
					
					// Get file.
					CString Srcfn = CString( CStringA( pPlant->GetTSCFilename() ) );
					
					// Remove 'old' metadata file from Maintenance folder.
					strTargetFileName = CTAScopeUtil::GetTAScopeDataFolder() + _T("\\") + Srcfn;
					RemoveFile( strTargetFileName );
					
					// Download 'Datastruct'.
					bReturn = m_pParent->ReadFile( Srcfn, strTargetFileName );
					// File downloaded incorporate it.
				}
			}
		}
		
		if( true == bReturn )
		{
			CTADataStructX TADSX;
			TADSX.Init();

			// TASCOPE-59. Force this variable to be sure to well read TA-Scope version.
			TADSX.SetDSType( CTADataStructX::DSXType_TADSX );
			
			TADSX.Read( (wchar_t *)(LPCTSTR)strTargetFileName );

			// HYS-1699: we don't have heating/cooling notion in the TA-Scope. So for injection we can put either 6/12 or 65/55.
			// By chance, the formula to compute primary flow will delete the sign and it will always be positive.
			// But in HySelect we need to know what is the application type by checking these temperatures if exist.
			ProjectType eProjectType = _RetrieveProjectType( &TADSX );

			// Check unique ID of datastructX with the current datastruct UID.
			// 2017-01-13: 'bTestMode' can be true if this method is called by 'CTabDlgSpecActTesting::_InterpretCmd'.
			if( true == bTestMode || true == m_pParent->IsComForcedOffLine() )
			{
				pPlant = (CDS_PlantX *)( TADSX.Get( _T("PLANT_INFO") ).MP );
				m_strPlantName = pPlant->GetString( CDS_PlantX::Name );
			}
			
			CString str1 = pPlant->GetString( CDS_PlantX::ProjectUID );
			CString str2 = TASApp.GetpTADS()->GetUID().c_str();
			bool bImportTADSX = false;
			bool bFromScratch = true;
			
			// Projects are different ask to close current datastruct it first.
			if( str1 != str2 )
			{
				if( false == bTestMode )
				{
					CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
					ASSERT( NULL != pPrjRef );

					CString TADSPlantName = pPrjRef->GetString( CDS_ProjectRef::Name );
					CString str; 
					FormatString( str, IDS_TASCOPE_PANELDOWNLOAD_EXISINGPLANTWILLBEERASED, TADSPlantName, m_strPlantName );
						
					if( IDOK == ::AfxMessageBox( str, MB_OKCANCEL | MB_ICONEXCLAMATION ) )
					{
						bImportTADSX = true;
						BeginWaitCursor();
						pTASelectDoc->OnFileNew();
					}
				}
				else
				{
					// Overwrite by default in debug mode.
					bImportTADSX = true;
					BeginWaitCursor();
					pTASelectDoc->OnFileNew();
				}
			}
			else	// Projects match
			{
				bFromScratch = false;
				bImportTADSX = true;
			}

			if( true == bImportTADSX )
			{
				// Copy the piping tab into the.
				CTableHM *pHNTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
				ASSERT( NULL != pHNTab );

				CDS_HMXTable *pHNTabX = (CDS_HMXTable *)( TADSX.Get( _T("PIPING_TAB") ).MP );
				
				if( NULL != pHNTabX && 0 != pHNTabX->GetItemCount() ) 
				{
					// First of all, verify if all pipes coming from TAScope are well defined in TASelect.
					if( false == pHNTabX->IsAllPipeExistsInUserDB( pHNTabX, TASApp.GetpPipeDB() ) )
					{
						CString str;
						FormatString( str, IDS_TASCOPE_PANELDOWNLOAD_PIPENOTEXISTINTASELECT, m_strPlantName );
				
						if( IDOK == ::AfxMessageBox( str, MB_OKCANCEL | MB_ICONEXCLAMATION ) )
						{
							BeginWaitCursor();
									
							// Find closest pipe coming from default pipes series.
							pHNTabX->ReplaceUserPipeByGenericPipe( pHNTabX, TASApp.GetpPipeDB() );
						}
						else
						{
							return false;
						}
					}

					// Load HMX Water char.
					CDS_WaterCharacteristic *pTADSWC = TASApp.GetpTADS()->GetpWCForProject();
					ASSERT( NULL != pTADSWC );

					CDS_WaterCharacteristic *pHMXWC = dynamic_cast<CDS_WaterCharacteristic*>( TADSX.Get( L"PARAM_HMXWC" ).MP );
					
					if( NULL != pTADSWC && NULL != pHMXWC )
					{
						*pTADSWC->GetpWCData() = *pHMXWC->GetpWCData();						
					}

					// HYS-1699: in the TA-Scope we don't have DT, so no default return temperature. But we need one to 
					// import when we will call the 'CDS_HydroMod::InitAllWaterCharWithParent' method.
					TASApp.GetpTADS()->GetpTechParams()->SetProjectApplicationType( eProjectType );
					TASApp.GetpTADS()->GetpTechParams()->SetDefaultPrjTps( pHMXWC->GetpWCData()->GetTemp() );
					TASApp.GetpTADS()->GetpTechParams()->SetDefaultPrjDT( 10.0 );

					// Sometimes there is a bug with the date & time of measures coming from TA Scope (Values are negative).
					// In this case, we reset the date & time.
					pHNTabX->VerifyMeasTABalDateTime();

					if( true == pHNTab->LoadFromHMXTable( bFromScratch, pHNTabX ) )
					{
					    // Position correctly all module.
						TASApp.GetpTADS()->VerifyAndCorrectPosition();

						// Compute the total flow.
						TASApp.GetpTADS()->ComputeFlow( pHNTab );
							
						CDS_PlantX *pPlantX = (CDS_PlantX *)( TADSX.Get( _T("PLANT_INFO") ).MP );
						LPCTSTR lpProjectName = pPlantX->GetString( CDS_PlantX::ePlant::Name );
						LPCTSTR lpProjectDesc = pPlantX->GetString( CDS_PlantX::ePlant::Description );
						CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
						
						if( NULL != pPrjRef )
						{
							pPrjRef->SetString( CDS_ProjectRef::enPrjRef::Name, lpProjectName );
							pPrjRef->SetString( CDS_ProjectRef::enPrjRef::Comment, lpProjectDesc );
						}

						// HYS-1060: Once the project from the TA-Scope has been imported, we can overwrite the UID of the tsp project
						// with the one from the TA-Scope. 2 cases:
						//   1) if project has been created in HySelect, than exported to the TA-Scope and imported back in HySelect on:
						//		1a) empty project: UID of the tsp takes the UID from the TA-Scope -> OK.
						//      1b) same original project: UID of the tsp is replaced by the one from the TA-Scope but it's the same -> OK.
						//   2) if project has been created in TA-Scope, than imported in HY-Select on:
						//		1a) empty project: UID of the tsp takes the UID from the TA-Scope -> OK.
						TASApp.GetpTADS()->SetUID( pPlantX->GetString( CDS_PlantX::ePlant::ProjectUID ) );

						// Change the project application type.
						pMainFrame->ChangeRbnProjectApplicationType( eProjectType );

						// Build new water characteristic strings.
						pMainFrame->UpdateRibbonWaterCharacteristic( WMUserWaterCharWParam::WM_UWC_WP_ForProject );

						// Reset the upper and lower limits for measurements.
						pMainFrame->ResetMeasurementsLimits();

						// Set a message that the project references have changed.
						::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_PROJECTREFCHANGE );
					}
				}
			}
			else
			{
				return false;
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgPanelTAScopeDownload::DownLoadHMX'."), __LINE__, __FILE__ );
		throw;
	}

	return true;
}

bool CDlgPanelTAScopeDownload::RefreshTreeQMLog()
{
	BeginWaitCursor();
	m_TreeQMLog.DeleteAllItems();
	CString str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELDOWNLOAD_TREELABEL_QM );
	HTREEITEM hParentItemQM = m_TreeQMLog.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, NULL, TVI_ROOT, TVI_ROOT );
	
	str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELDOWNLOAD_TREELABEL_LOG );
	HTREEITEM hParentItemLog = m_TreeQMLog.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, NULL, TVI_ROOT, TVI_ROOT );

	if( false == m_pParent->IsComForcedOffLine() )
	{
		// Load tadsx.tsc file from TA-SCOPE.
		m_pParent->GetTADSFile();
	}
	BeginWaitCursor();

	// Process data.
	CString fn = CTAScopeUtil::GetTAScopeDataFolder() + _T("\\") + TADSXFILE;
	std::ifstream inpf( fn, std::ifstream::in | std::ifstream::binary );
	if( std::ifstream::failbit == ( inpf.rdstate() & std::ifstream::failbit ) )
	{
		ASSERT( 0 );
		return false;
	}

	m_TADSX.Init();

	// TASCOPE-59. Force this variable to be sure to well read TA-Scope version.
	m_TADSX.SetDSType( CTADataStructX::DSXType_TADSX );

	m_TADSX.Read( inpf );
	inpf.close();
	
	// Copy LogDataX into the LOGGEDDATA_TAB in the DATASTRUCT.
	CTable *pLDTab = TASApp.GetpTADS()->GetpLogDataTable();
	ASSERT( NULL != pLDTab );

	CTable *pLDTabX = (CTable *)( m_TADSX.Get( _T("LOGGEDDATA_TAB") ).MP );
	
	int iLog = 0;
	
	if( NULL != pLDTabX )
	{
		for( IDPTR IDPtr = pLDTabX->GetFirst( CLASS( CDS_LogDataX ) ); _T('\0') != *IDPtr.ID; IDPtr = pLDTabX->GetNext() )
		{
			CDS_LogDataX *pLogDataX = dynamic_cast<CDS_LogDataX*>( IDPtr.MP );

			if( NULL == pLogDataX )
			{
				continue;
			}

			str = pLogDataX->GetName();
			HTREEITEM ItemLog = m_TreeQMLog.InsertItem( TVIF_TEXT | TVIF_PARAM,	(LPCTSTR)pLogDataX->GetName(), 0, 0, 0, 0, (LPARAM)pLogDataX, hParentItemLog, TVI_LAST );
			iLog++;
		}
	}

	// Copy QuickMeas into the QUICKMEAS_TAB in the DATASTRUCT.
	CDS_QMTable *pTab = dynamic_cast<CDS_QMTable *>( TASApp.GetpTADS()->GetpQuickMeasureTable() );
	ASSERT( NULL != pTab );

	CDS_QMTable *pTabX = (CDS_QMTable *)( m_TADSX.Get( _T("QUICKMEAS_TAB") ).MP );
	int iQM = 0;
	std::vector <CDS_HydroModX*> m_vectorAlreadyPrinted;
	
	if( NULL != pTabX )
	{
		for( IDPTR IDPtr = pTabX->GetFirst( CLASS( CDS_HydroModX ) ); _T('\0') != *IDPtr.ID; IDPtr = pTabX->GetNext( IDPtr.MP ) )
		{
			CDS_HydroModX *pHMX = dynamic_cast<CDS_HydroModX*>( IDPtr.MP );

			if( NULL == pHMX )
			{
				continue;
			}
			
			// Associated measurement.
			CDS_HydroModX *pAssHMX = pTabX->GetAssociatedHM( pHMX );

			if( m_vectorAlreadyPrinted.end() != std::find( m_vectorAlreadyPrinted.begin(), m_vectorAlreadyPrinted.end(), pAssHMX ) )
			{
				continue;
			}
			
			m_vectorAlreadyPrinted.push_back( pHMX );
			str = pHMX->GetHMName()->c_str();
			HTREEITEM ItemLog = m_TreeQMLog.InsertItem( TVIF_TEXT | TVIF_PARAM,	(LPCTSTR)str, 0, 0, 0, 0, (LPARAM)pHMX, hParentItemQM, TVI_LAST );
			iQM++;
		}
	}
	
	if( 0 == iLog )
	{
		CString str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELDOWNLOAD_TREELABEL_NOLOG );
		m_TreeQMLog.DeleteItem( hParentItemLog );
		HTREEITEM ItemLog = m_TreeQMLog.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, NULL, TVI_ROOT, TVI_LAST );
	}

	if( 0 == iQM )
	{
		CString str = TASApp.LoadLocalizedString( IDS_TASCOPE_PANELDOWNLOAD_TREELABEL_NOQM );
		m_TreeQMLog.DeleteItem( hParentItemQM );
		HTREEITEM ItemLog = m_TreeQMLog.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, NULL, TVI_ROOT, TVI_LAST );
	}

	m_TreeQMLog.Expand( hParentItemQM, TVE_EXPAND );
	m_TreeQMLog.Expand( hParentItemLog, TVE_EXPAND );

	return true;
}

bool CDlgPanelTAScopeDownload::RefreshTreePlant()
{
	try
	{
		BeginWaitCursor();
		m_TreePlant.DeleteAllItems();

		// Prepare and load metadata file file.
		bool bReturn = m_pParent->GetMetaData();

		if( false == bReturn )
		{
			return false;
		}
	
		BeginWaitCursor();
		m_MetaData.Init();

		CString fn = CTAScopeUtil::GetTAScopeMaintenanceFolder() + _T("\\") + METAFILE;
		CFileStatus fs;

		if( FALSE == CFile::GetStatus( fn, fs ) ) // check if the file exists and get the status.
		{
			return false;
		}
	
		std::ifstream inpf( fn, std::ifstream::in | std::ifstream::binary );
		m_MetaData.Read( inpf );

		for( IDPTR IDPtr = m_MetaData.Access().GetFirst(); NULL != IDPtr.MP; IDPtr = m_MetaData.Access().GetNext() )
		{
			CDS_PlantX *pPlant = dynamic_cast<CDS_PlantX*>( IDPtr.MP );

			if( NULL != pPlant )
			{
				// Skip tadsx.tsc.
				CString fn = (CString)CStringA( pPlant->GetTSCFilename() );

				if( fn.Find( TADSXFILE ) > -1 )
				{
					continue;
				}
			
				CString str = pPlant->GetString( CDS_PlantX::Name );
				HTREEITEM hItem = m_TreePlant.InsertItem( TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 0, 0, 0, 0, (LPARAM)pPlant, TVI_ROOT, TVI_ROOT );
			}	
		}
	
		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgPanelTAScopeDownload::RefreshTreePlant'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgPanelTAScopeDownload::_CheckIsAtLeastOneItemSelected()
{
	bool fDownLoadAvailable = false;
	
	// Tree 'Plant'.
	for( int i = 0; i < 2; i++ )
	{
		CTreeCtrl *pTree = ( 0 == i ) ? &m_TreeQMLog : &m_TreePlant;
		
		HTREEITEM hItem = pTree->GetRootItem();
		HTREEITEM hNextItem = pTree->GetRootItem();
		while( NULL != hItem )
		{
			if( TRUE == pTree->ItemHasChildren(hItem ) )
				hNextItem = pTree->GetChildItem( hItem );
			
			while( NULL != hNextItem && false == fDownLoadAvailable )
			{
				LPARAM lparam = pTree->GetItemData( hNextItem );
				if( NULL != lparam )
				{
					CData* pData = dynamic_cast<CData *>( (CData*)lparam );
					if( NULL != pData && TRUE == pTree->GetCheck( hNextItem ) )
						fDownLoadAvailable = true;
				}
				hNextItem = pTree->GetNextItem( hNextItem, TVGN_NEXT );
			}
			
			if( TRUE == pTree->ItemHasChildren( hItem ) )
				hItem = pTree->GetNextItem( hItem, TVGN_NEXT );
			else
				hItem = NULL;
		}
	}
	
	if( true == fDownLoadAvailable )
		m_pParent->ShowButtons( CWizardManager::WizButNext, true );
	else
		m_pParent->ShowButtons( CWizardManager::WizButNext, false );
}

ProjectType CDlgPanelTAScopeDownload::_RetrieveProjectType( CTADataStructX *pclTADataStructX )
{
	if( NULL == pclTADataStructX )
	{
		return ProjectType::InvalidProjectType;
	}

	for( IDPTR IDPtr = pclTADataStructX->GetpDataList()->GetFirst(); NULL != IDPtr.MP; IDPtr = pclTADataStructX->GetpDataList()->GetNext() )
	{
		CDS_HydroModX *pclHMX = dynamic_cast<CDS_HydroModX *>( IDPtr.MP );

		if( NULL == pclHMX )
		{
			continue;
		}

		CDB_CircSchemeCateg *pclCircSchemeCateg = pclHMX->GetpSchcat();

		if( NULL != pclCircSchemeCateg && true == pclCircSchemeCateg->IsSecondarySideExist() )
		{
			if( pclHMX->GetTempSecondary() > 0.0 && 0.0 != pclHMX->GetTempReturn() )
			{
				if( pclHMX->GetTempSecondary() > pclHMX->GetTempReturn() )
				{
					return ProjectType::Heating;
				}
				else if( pclHMX->GetTempSecondary() < pclHMX->GetTempReturn() )
				{
					return ProjectType::Cooling;
				}
			}
		}
	}

	return ProjectType::Heating;
}