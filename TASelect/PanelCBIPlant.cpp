#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include "math.h"
#include "TASelect.h"

#include "Utilities.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "MainFrm.h"
#include "WizardCBI.h"
#include "DlgResolCBIPlantModif.h"
#include "PanelCBIPlant.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPanelCBIPlant::CPanelCBIPlant( CWnd* pParent )
	: CDlgWizard( CPanelCBIPlant::IDD, pParent )
{
	m_pTADS = NULL;
	m_pTADB = NULL;
	m_pParent = NULL;
}

CPanelCBIPlant::~CPanelCBIPlant()
{
	m_brWhiteBrush.DeleteObject();	
}

BEGIN_MESSAGE_MAP( CPanelCBIPlant, CDialogEx )
	ON_WM_CTLCOLOR()
	ON_NOTIFY( NM_CLICK, IDC_LIST, OnClickList )
END_MESSAGE_MAP()

void CPanelCBIPlant::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST, m_List );
	DDX_Control( pDX, IDC_EDITTEXT, m_EditText );
}

BOOL CPanelCBIPlant::OnInitDialog() 
{
	CDlgWizard::OnInitDialog();
	
	// Set the white rectangle to the first drawn window.
	GetDlgItem( IDC_STATICRECTANGLE )->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );

	m_pParent = (CWizardCBI *)GetWizMan();
	m_pTADS = TASApp.GetpTADS();
	m_pTADB = TASApp.GetpTADB();
	m_ImageList.Create( IDB_RADIOCHECKEDUNCHECKED, 16, 1, RGB( 0, 128, 128 ) );
	m_List.SetImageList( &m_ImageList, LVSIL_STATE );
	
	return TRUE;
}

HBRUSH CPanelCBIPlant::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor ) 
{
	HBRUSH hbr;
	if( pWnd->m_hWnd == GetDlgItem( IDC_EDITTEXT )->m_hWnd )
	{
		pDC->SetTextColor( RGB( 0, 0, 0 ) );
		pDC->SetBkColor( RGB( 255, 255, 255 ) );
		return (HBRUSH)m_brWhiteBrush;
	}
	
	hbr = CDlgWizard::OnCtlColor( pDC, pWnd, nCtlColor );
	return hbr;
}

void CPanelCBIPlant::OnClickList( NMHDR* pNMHDR, LRESULT* pResult )
{
	// Retrieve where the user clicked.
	LVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= GET_X_LPARAM( dwpos );
	ht.pt.y= GET_Y_LPARAM( dwpos );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	m_List.HitTest( &ht );

	// If the user click on the State Image, change the click item to checked and all other items to unchecked.
	CRect Rect, LabelRect;
	m_List.GetItemRect( ht.iItem, &Rect, LVIR_BOUNDS );
	m_List.GetItemRect( ht.iItem, &LabelRect, LVIR_LABEL );
	Rect.right = LabelRect.left - 1;
	if( TRUE == Rect.PtInRect( ht.pt ) )
	{
		for( int i = 0; i < m_List.GetItemCount(); i++ )
			m_List.SetCheck( i, ( i == ht.iItem ) ? ( ( m_List.GetCheck ( i ) ) ? FALSE : TRUE ) : FALSE );
	}

	if( NULL != pResult )
		*pResult = 0;
}

BOOL CPanelCBIPlant::OnWizFinish()
{
	if( false == m_bInitialized )
		return TRUE;

	// Set to be in NoHMCalc mode because valves can be not completely define.
	pMainFrame->SetHMCalcMode( false );

	if( true == ProcessSelectedPlant() )
		return TRUE;

	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	SetFocus();

	// Returns 'FALSE' to stay in this dialog.
	return FALSE;
}

bool CPanelCBIPlant::OnActivate()
{
	// No Plant data present.
	if( FALSE == m_pParent->m_PanelCBI2.GetCheckReadProj() )
		return false;
	
	m_List.DeleteAllItems();

	CTable* pTab = (CTable *)( m_pTADS->Get( _T("TMPSAVEDATA_TAB") ).MP );					ASSERT( NULL != pTab );

	CString str = TASApp.LoadLocalizedString( IDS_PANELCBIPLANT_TEXT );
	m_EditText.SetWindowText( str );
	
	// Define the width of each column depending on the width of the list control.
	CRect rect;
	m_List.GetClientRect( (LPRECT)&rect );
	
	// Add columns.
	// Size of columns is defined by the size of columns header, it must be formatted with a sufficient number of space.
	str = TASApp.LoadLocalizedString( IDS_PANELCBIPLANT_PLANT );
	m_List.InsertColumn( 0, str, LVCFMT_LEFT, (int)( rect.Width() * 0.4 ), 0 );
	str = TASApp.LoadLocalizedString( IDS_PANELCBIPLANT_VALVENBR );
	m_List.InsertColumn( 2, str, LVCFMT_LEFT, (int)( rect.Width() * 0.3 ), 1 );
	str = TASApp.LoadLocalizedString( IDS_PANELCBIPLANT_DATE );
	m_List.InsertColumn( 3, str, LVCFMT_LEFT, (int)( rect.Width() * 0.3 ), 2 );

	// Add all plants in CBI into the list.
	LVITEM	lvItem;	
	lvItem.mask = LVIF_STATE | LVIF_PARAM | LVIF_TEXT; 
	lvItem.iItem = 0; 
	lvItem.iSubItem = 0; 
	lvItem.state = 0; 
	lvItem.stateMask = 0; 
	lvItem.pszText = NULL; 
	lvItem.cchTextMax = _TABLE_NAME_LENGTH; 
	lvItem.iImage = 0; 
	lvItem.lParam = NULL;
	lvItem.iIndent = 1;
	CDB_MultiString* pPlant = NULL;
	CTable* pValveTab = NULL;
	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_MultiString ) ); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		pPlant = (CDB_MultiString*)( IDPtr.MP );										ASSERT( NULL != pPlant );
		pValveTab = (CTable*)( m_pTADS->Get( pPlant->GetString( 0 ) ).MP );				ASSERT( NULL != pValveTab );
		lvItem.pszText = (TCHAR *)pPlant->GetString( 2 );
		lvItem.lParam = (LPARAM)pPlant;
		m_List.InsertItem( &lvItem );

		str.Format( _T("%d"), pValveTab->GetItemCount() );
		m_List.SetItem( 0, 1, LVIF_TEXT, str, 0, 0, 0, 0 );

		int Y, M, D, h, m, s, index, indexp;
		const char point = '.';
		const char colon = ':';

		str = pPlant->GetString ( 3);
		index = str.Find( point, 0 );
		Y = _ttoi( str.Left( index ) );
		indexp = ++index;
		index = str.Find( point, index );
		M = _ttoi( str.Mid( indexp, 2 ) );
		indexp = ++index;
		index = str.Find( point, index );
		D = _ttoi( str.Mid( indexp, 2 ) );

		str = pPlant->GetString( 4 );
		index = str.Find( colon, 0 );
		h = _ttoi( str.Left( index ) );
		indexp = ++index;
		index = str.Find( colon, index );
		m = _ttoi( str.Mid( indexp, 2 ) );
		indexp = ++index;
		index = str.Find( colon, index );
		s = _ttoi( str.Mid( indexp, 2 ) );
		if( Y > 2001 && Y < 3000 && M > 0 && M < 13 && D > 0 && D < 32 && h > -1 && h < 25 && m > -1 && m < 61 && s > -1 && s < 61 )
		{		
			CTimeUnic dtu;
			dtu.CTime::CTime( Y, M, D, h, m, s );			// cfr. C++ Primer page 693
			str = dtu.Format( IDS_LOGDATETIME_FORMAT );
			m_List.SetItem( 0, 2, LVIF_TEXT, str, 0, 0, 0, 0 );
		}
		else
			m_List.SetItem( 0, 2, LVIF_TEXT, _T("-"), 0, 0, 0, 0 );
	}
	
	// Set the check state outside the creation loop otherwise the first list item get no state image.
	for( int i = 0; i < m_List.GetItemCount(); i++ )
		m_List.SetCheck( i, FALSE );
	
	return true;
}

bool CPanelCBIPlant::OnAfterActivate()
{
	CDlgWizard::OnAfterActivate();

	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	SetFocus();
	return true;
}

bool CPanelCBIPlant::OnEscapeKeyPressed( void )
{
	// User presses the [ESCAPE] key or close with the cross button in the title bar.
	CDlgWizard::OnEscapeKeyPressed();
	
	// TODO: if plant exist, display a message to ask user if he wants quit or stay to continue import.
	return TRUE;
}

int CPanelCBIPlant::OnWizButtons() 
{
	// Just Plant Data.
	int iReturn = CWizardManager::WizButFinish;
	if( TRUE == m_pParent->m_PanelCBI2.GetCheckReadProj() )
	{
		if( FALSE == m_pParent->m_PanelCBI2.GetCheckCollectData() )
			iReturn = CWizardManager::WizButFinish;
		else
		{
			// Collect data is selected.
			m_pParent->DisableButtons( CWizardManager::WizButFinish );
			iReturn = ( CWizardManager::WizButNext | CWizardManager::WizButFinish );
		}
	}
	return iReturn;
}

int CPanelCBIPlant::OnWizButFinishTextID()
{
	int iReturn = IDS_WIZBUT_CANCEL;
	if( TRUE == m_pParent->m_PanelCBI2.GetCheckReadProj() && FALSE == m_pParent->m_PanelCBI2.GetCheckCollectData() )
		iReturn = IDS_WIZBUT_FINISH;
	return iReturn;
}

bool CPanelCBIPlant::ProcessSelectedPlant()
{
	try
	{
		// Check if there is a selection.
		int iSel = -1;

		if( 0 == m_List.GetItemCount() )
		{
			return true;	// do nothing
		}
	
		for( int i = 0; i < m_List.GetItemCount(); i++ )
		{
			if( TRUE == m_List.GetCheck( i ) ) 
			{
				iSel = i;
			}
		}
	
		if( -1 == iSel )
		{
			if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_NOPLANTSELECTED, MB_YESNO | MB_ICONEXCLAMATION ) )
			{
				// Delete the TmpSavedData.
				m_pParent->m_PanelCBI3.DeleteTmpSavedData();
				m_pTADS->Modified();

				return true; // do nothing
			}
			else
			{
				return false; // let the user have a new chance to select one plant
			}
		}

		// Check if the name of the selected plant corresponds to this in the TADS.
		LVITEM lvItem;	
		lvItem.mask = LVIF_PARAM; 
		lvItem.iSubItem = 0; 
		lvItem.iItem = iSel; 
		m_List.GetItem( &lvItem );

		CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();

		if( NULL == pPrjRef )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'PROJECT_REF' object from the datastruct.") );
		}

		CDB_MultiString *pPlant = (CDB_MultiString *)( lvItem.lParam );
		
		if( NULL == pPlant )
		{
			HYSELECT_THROW( _T("Internal error: Object at position %i is NULL."), iSel );
		}

		CString TADSPlantName = pPrjRef->GetString( CDS_ProjectRef::Name );
		CString CBIPlantName = m_List.GetItemText( iSel, 0 );
	
		// If names are different: Ask confirmation, clean PIPING_TAB and load the selected CBI plant.
		if( CBIPlantName != TADSPlantName || m_pTADS->GetpProjectParams()->GetLastUid() <= 1 )
		{
			CString str; 
			FormatString( str, AFXMSG_OVERWRITEPLANTINTAS2, TADSPlantName, CBIPlantName );
			
			if( IDOK == ::AfxMessageBox( str, MB_OKCANCEL | MB_ICONEXCLAMATION ) )
			{
				BeginWaitCursor();
			
				// Clean PIPING_TAB.
				// Scan each table PIPING_TAB, SELHUB_TAB,....
				for( int i = 0; i < m_pParent->m_aTableUID.GetCount(); i++ )
				{
					CTable *pTab = dynamic_cast<CTable *>( m_pParent->m_aTableUID.GetAt( i ).TabIDPtr.MP );
					
					if( NULL == pTab )
					{
						HYSELECT_THROW( _T("Internal error: Object '%s' at position %i is not a 'CTable' object."), m_pParent->m_aTableUID.GetAt( i ).TabIDPtr.ID, i );
					}
					
					m_pTADS->DeleteTADSBranch( pTab );
				}
			
				// Move selected plant from temporary tab to the TADS.
				MoveSelPlantToTADS( pPlant );
				EndWaitCursor();
			}
			else
			{
				return false;
			}
		}
		else
		{
			// Names are identical: Append info from selected temporary tab plant to the TADS.
		
			for( int i = 0; i < m_pParent->m_aTableUID.GetCount(); i++ )
			{
				CTable *pPipTab = dynamic_cast<CTable *>( m_pParent->m_aTableUID.GetAt( i ).TabIDPtr.MP );
				
				if( NULL == pPipTab )
				{
					HYSELECT_THROW( _T("Internal error: Object '%s' at position %i is not a 'CTable' object."), m_pParent->m_aTableUID.GetAt( i ).TabIDPtr.ID, i );
				}

				// Create a tmp table TMPPIPING_TAB as a working copy of PIPING_TAB.
				IDPTR NewTabIDptr;
				m_pTADS->CreateObject( NewTabIDptr, CLASS( CTable ), _T("TMPPIPING_TAB" ) );
				
				CTable *pPrjTab = (CTable *)( m_pTADS->Get( _T("PROJ_TABLE") ).MP );
				pPrjTab->Insert( NewTabIDptr );
			
				CTable *pTmpPipTab = (CTable *)( NewTabIDptr.MP );

				// Duplicate PIPING_TAB in TMPPIPING_TAB.
				m_pTADS->CopyTADSBranch( pPipTab, pTmpPipTab, false );

				bool bWarningOldVersion = false;
				AppendSelPlantToTADS( pPlant, pTmpPipTab, &bWarningOldVersion, pPipTab );

				// !!! TODO !!!
				// Display a WizardPanel to summarize plant modifications and ask confirmation
				// if confirmed delete PIPING_TAB and transfer TMPPIPING_TAB in it,
				// then delete SelP valves marked as deleted. They indeed can no longer be deleted while
				// processing TMPPIPING_TAB if a cancel can be performed!
				// If not, delete TMPPIPING_TAB and reset SelP marking: two possibilities
				//		the user cancels, so do nothing but delete TmpSavedData
				//		the user goes to previous panel => need for a Previous button
				if( true == bWarningOldVersion )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_ATTEMPTTOLOADONOLD, MB_ICONEXCLAMATION );
				}
				else
				{
					// Scan all CDS_HydroMod to delete all valves left with the ett_Delete treatment.
					DeleteTaggedValves( pTmpPipTab );

					// Delete PIPING_TAB and transfer TMPPIPING_TAB in it.
					m_pTADS->DeleteTADSBranch( pPipTab );
				
					for( IDPTR IDPtr = pTmpPipTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTmpPipTab->GetNext() )
					{
						pTmpPipTab->Remove( IDPtr );
						pPipTab->Insert( IDPtr );
					}
				}

				// Delete left TMPPIPING_TAB.
				m_pTADS->DeleteTADSBranch( pTmpPipTab );
				pPrjTab->Remove( NewTabIDptr );
				m_pTADS->DeleteObject( NewTabIDptr );

				if( eBool3::eb3True == m_pTADS->IsHMCalcMode())
				{
					m_pTADS->ComputeAllInstallation();
				}
			}
		}
	
		// Delete the TmpSavedData.
		m_pParent->m_PanelCBI3.DeleteTmpSavedData();
		m_pTADS->Modified();

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CPanelCBIPlant::ProcessSelectedPlant'."), __LINE__, __FILE__ );
		throw;
	}
}

void CPanelCBIPlant::MoveSelPlantToTADS(CDB_MultiString *pPlant)
{
	try
	{
		CTable *pPipTab = NULL;
	
		// Copy the general parameters of the plant.
	
		// Plant Name.
		CString Tmp = pPlant->GetString( 2 );
		Tmp.TrimRight();

		CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
		
		if( NULL == pPrjRef )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'PROJECT_REF' object from the datastruct.") );
		}

		pPrjRef->SetString( CDS_ProjectRef::Name, Tmp );
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_PROJECTREFCHANGE );
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PROJECTREFCHANGE );
	
		// Media.
		CopyMediaData( pPlant );

		// Copy the valves and associated modules.
		m_pTADS->GetpProjectParams()->ResetUid();
	
		CTable *pValveTab = (CTable*)( m_pTADS->Get( pPlant->GetString( 0 ) ).MP );
		
		if( NULL == pValveTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the '%s' table from the datastruct."), pPlant->GetString( 0 ) );
		}

		CDS_HydroMod *pHM = NULL;
		CString strPrevRefName, strRefName;
		int iMod = 0;
		IDPTR IDPtr = _NULL_IDPTR;
	
		for( IDPtr = pValveTab->GetFirst( CLASS( CDB_MultiString ) ); _T('\0') != *IDPtr.ID; IDPtr = pValveTab->GetNext() )
		{
			CDB_MultiString *pValve = (CDB_MultiString *)( IDPtr.MP );
			
			if( NULL == pValve )
			{
				HYSELECT_THROW( _T("Internal error: '%s' object is not a 'CDB_MultiString' object."), IDPtr.ID );
			}
		
			long lUID;

			if( RD_OK != ReadLong( pValve->GetString( 2 ), &lUID ) )
			{
				lUID = 0;
			}

			if( lUID < m_pTADS->GetpProjectParams()->GetLastUid() )
			{
				continue;
			}
		
			m_pTADS->GetpProjectParams()->SetLastUid( (unsigned short)lUID );
		}
	
		for( IDPtr = pValveTab->GetFirst( CLASS( CDB_MultiString ) ); _T('\0') != *IDPtr.ID; IDPtr = pValveTab->GetNext() )
		{
			CDB_MultiString *pValve = (CDB_MultiString*)( IDPtr.MP );
			
			if( NULL == pValve )
			{
				HYSELECT_THROW( _T("Internal error: '%s' object is not a 'CDB_MultiString' object."), IDPtr.ID );
			}

			long lUID;

			if( RD_OK != ReadLong( pValve->GetString( 2 ), &lUID ) )
			{
				lUID = 0;
			}
		
			pPipTab = m_pParent->GetPipingTab( lUID );

			strRefName = pValve->GetString( 1 );
			strRefName.TrimRight();
		
			// Error the module name is empty, skip this module .
			if( 0 == strRefName.GetLength() )
			{
				continue;
			}
		
			// If module is not the same as previous one, scan to find if module already exists, create it if necessary.
			if( strRefName != strPrevRefName )
			{
				pHM = m_pTADS->FindHydroMod( strRefName, pPipTab, NULL, CDatastruct::FindMode::OnlyModules );

				// Module does not exist yet, create it.
				if( NULL == pHM )
				{
					// Create a new module as a root module attached to the PIPING_TAB.
					iMod++;
					
					IDPTR ModIDptr;
					m_pTADS->CreateObject( ModIDptr, CLASS( CDS_HydroMod ) );
					pPipTab->Insert( ModIDptr );

					pHM = (CDS_HydroMod*)( ModIDptr.MP );

					pHM->SetHMName( strRefName );
					pHM->SetLevel( 0 );
					pHM->SetPos( iMod );
					pHM->SetFlagModule( true );
					pHM->SetTreatment( ett_Added );
					pHM->SetTreatmentLparam( NULL );
					TRACE( _T("Module :%s added\n"), pHM->GetHMName() );
				}
			}

			strPrevRefName = strRefName;

			// Add a new valve in pHM and fill it.
			IDPTR ChildIDptr;
			m_pTADS->CreateObject( ChildIDptr, CLASS( CDS_HydroMod ) );
			pHM->Insert( ChildIDptr );

			CDS_HydroMod *pNewValve = (CDS_HydroMod *)( ChildIDptr.MP );

			if( RD_OK != ReadLong( pValve->GetString( 2 ), &lUID ) )
			{
				lUID = 0;
			}
		
			CopyValvData( pValve, pNewValve, true, (unsigned short)lUID );
			pNewValve->SetTreatment( ett_Added );
			pNewValve->SetTreatmentLparam( NULL );
			TRACE( _T("Valve :%s added, UID = %d\n"), pNewValve->GetHMName(), pNewValve->GetUid() );
		}

		// Loop over modules, reattribute valves of Ghost "PV_" modules as Partner valves  of existing modules and delete the Ghost "PV_" modules.
		for( int i = 0; i < m_pParent->m_aTableUID.GetCount(); i++ )
		{
			pPipTab = dynamic_cast<CTable *>( m_pParent->m_aTableUID.GetAt( i ).TabIDPtr.MP );
			
			if( NULL == pPipTab )
			{
				HYSELECT_THROW( _T("Internal error: Object '%s' is not a 'CTable' object."), m_pParent->m_aTableUID.GetAt( i ).TabIDPtr );
			}

			CString strPVmod = TASApp.LoadLocalizedString( IDS_PV );

			for( IDPtr = pPipTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pPipTab->GetNext() )
			{
				CDS_HydroMod *pGhostHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );
				
				if( NULL == pGhostHM )
				{
					HYSELECT_THROW( _T("Internal error: Object '%s' is not a 'CDS_HydroMod' object."), IDPtr.ID );
				}

				CString strGhostRefName = pGhostHM->GetHMName();
				strGhostRefName.TrimRight();
			
				// If it's not a ghost "PV_" module...
				if( strGhostRefName.Left( 3 ) != strPVmod )
				{
					continue;
				}
			
				// There is more than 1 valve in the ghost module.
				// The module can thus not be dealt with as a ghost module.
				if( pGhostHM->GetCount() > 1 )
				{
					continue;
				}

				// Find the module to which the ghost module is "associated".
				CString strRefName = strGhostRefName.Mid( 3 );
				pHM = m_pTADS->FindHydroMod( strRefName, pPipTab, NULL, CDatastruct::FindMode::OnlyModules );
			
				if( NULL == pHM ) 
				{
					// Try to find "associated" module by name of Partner valve.
					CDS_HydroMod *pPV = (CDS_HydroMod*)( pGhostHM->GetFirst().MP );

					if( NULL == pPV )
					{
						pPipTab->GetMP( NULL/*CLASS(CDS_HydroMod)*/, IDPtr.ID );
						continue;
					}
				
					strRefName = pPV->GetHMName();
					pHM = m_pTADS->FindHydroMod( strRefName, pPipTab, pPV, CDatastruct::FindMode::Both );
				
					if( NULL == pHM ) 
					{
						pPipTab->GetMP( NULL/*CLASS(CDS_HydroMod)*/, IDPtr.ID );
						continue;
					}
				}
			
				// Copy valve data and Uid.
				CDS_HydroMod *pCopiedValve = (CDS_HydroMod*)( pGhostHM->GetFirst().MP );
				pCopiedValve->Copy( pHM, true );
				pHM->SetUid( pCopiedValve->GetUid() );

				// Delete the valve in the ghost module.
				m_pTADS->DeleteTADSBranch( pGhostHM );

				// Tag the ghost module as deleted.
				pGhostHM->SetTreatment( ett_Delete );
				pPipTab->GetMP( NULL/*CLASS(CDS_HydroMod)*/, IDPtr.ID );
			}

			// Scan all CDS_HydroMod to delete all valves left with the ett_Delete treatment.
			DeleteTaggedValves( pPipTab );

		
			// Scan all root modules to find if a PV can be associated to them search is done by matching PV name and Module name.
			for( IDPtr = pPipTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pPipTab->GetNext() )
			{
				CDS_HydroMod *pCurrentHM = (CDS_HydroMod*)( IDPtr.MP );
				
				if( NULL == pCurrentHM )
				{
					HYSELECT_THROW( _T("Internal error: Object '%s' is not a 'CDS_HydroMod' object."), IDPtr.ID );
				}

				CString strCurrentHMName = pCurrentHM->GetHMName();
				strCurrentHMName.TrimRight();
			
				// Skip current module.
				pHM = m_pTADS->FindHydroMod( strCurrentHMName, pPipTab, pCurrentHM, CDatastruct::FindMode::Both );
			
				if( NULL == pHM )
				{
					pPipTab->GetMP( NULL, IDPtr.ID );
					continue;
				}
			
				// Move the current module to the PV found.
				m_pTADS->MoveHydroMod( pCurrentHM, pHM, true );

				// Structure has moved restart scanning from beginning.
				IDPtr = pPipTab->GetFirst();
			}

			// Scan all root modules.
			for( IDPtr = pPipTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pPipTab->GetNext() )
			{
				// Check UID and update it if necessary.
				if( 0 == ( (CDS_HydroMod*)IDPtr.MP )->GetUid() )
				{
					( (CDS_HydroMod*)IDPtr.MP )->SetUid( m_pTADS->GetpProjectParams()->GetNextUid() );
				}

				// Set correct level for HM.
				m_pTADS->SetHMLevel( (CTable*)IDPtr.MP, 0 );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CPanelCBIPlant::MoveSelPlantToTADS'."), __LINE__, __FILE__ );
		throw;
	}
}

void CPanelCBIPlant::AppendSelPlantToTADS( CDB_MultiString* pPlant, CTable* pTmpPipTab, bool* pfWarningOldVersion, CTable* pOrgTab )
{
	try
	{
		// Reset all Treatment type flags.
		ResetHMTreatment( pTmpPipTab );

		// Copy the general parameters of the plant.
		// TODO!!! To enable a back step in the wizard, copy these parameters later after user confirmed plant transfer.
	
		// Plant name.
		CString strTmp = pPlant->GetString( 2 );
		strTmp.TrimRight();
	
		CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
		
		if( NULL == pPrjRef )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'PROJECT_REF' table from the datastruct.") );
		}
	
		pPrjRef->SetString( CDS_ProjectRef::Name, strTmp );
	
		// Media.
		CopyMediaData( pPlant );

		// Start the recognition process.
		CTable *pValveTab = (CTable *)( m_pTADS->Get( pPlant->GetString( 0 ) ).MP );	
		
		if( NULL == pValveTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the '%s' table from the datastruct."), pPlant->GetString( 0 ) );
		}

		CDS_HydroMod *pHM = NULL;
		CString strPrevRefName, strRefName;
		CString strPVmod = TASApp.LoadLocalizedString( IDS_PV );
		IDPTR IDPtr;
		
		for( IDPtr = pValveTab->GetFirst( CLASS( CDB_MultiString ) ); _T('\0') != *IDPtr.ID; IDPtr = pValveTab->GetNext() )
		{
			CDB_MultiString *pValve = dynamic_cast<CDB_MultiString *>( IDPtr.MP );
			
			if( NULL == pValve )
			{
				HYSELECT_THROW( _T("Internal error: '%s' object is not a 'CDB_MultiString' object."), IDPtr.ID );
			}

			strRefName = pValve->GetString( 1 );
			strRefName.TrimRight();
			long lUID;

			if( RD_OK != ReadLong( pValve->GetString( 2 ), &lUID ) )
			{
				lUID = 0;
			}
		
			if( m_pParent->GetPipingTab( lUID ) != pOrgTab )
			{
				continue;
			}
		
			if( lUID > m_pTADS->GetpProjectParams()->GetLastUid() )
			{
				lUID = 0;
				*pfWarningOldVersion = true;
				return;	// TODO!!! Return after the whole processing (do that when all cases will be handled)
			}
		
			// If module is not the same as previous one, scan to find if module already exists, create it if necessary.
			if( strRefName != strPrevRefName )
			{
				pHM = m_pTADS->FindHydroMod( strRefName, pTmpPipTab, NULL, CDatastruct::FindMode::OnlyModules );

				// Module does not exist yet in TADS and is not a Ghost "PV_" module, create it.
				if( NULL == pHM && strRefName.Left(3) != strPVmod )
				{
					// Create a new module as a root module attached to TMPPIPING_TAB.
					IDPTR ModIDptr;
					m_pTADS->CreateObject( ModIDptr, CLASS( CDS_HydroMod ) );
					pTmpPipTab->Insert( ModIDptr );

					pHM = (CDS_HydroMod *)( ModIDptr.MP );
				
					if( eBool3::eb3True == m_pTADS->IsHMCalcMode() )
					{
						IDPTR IDPtrSch = m_pTADB->Get( _T("SP") );
						pHM->Init( IDPtrSch, pTmpPipTab, true );
						pHM->CreateTermUnit();
						pHM->GetpTermUnit()->SetVirtual( true );
						pHM->SetSchemeID( IDPtrSch.ID );
					}
					
					pHM->SetHMName( strRefName );
					pHM->SetLevel( 0 );
					pHM->SetUid( m_pTADS->GetpProjectParams()->GetNextUid() );
					pHM->SetPos( pTmpPipTab->GetItemCount() );
					pHM->SetFlagModule( true );
					pHM->SetTreatment( ett_Added );
					pHM->SetTreatmentLparam( NULL );
				}
			}
			
			strPrevRefName = strRefName;

			// Check Uid.
			CDS_HydroMod *pTADSValve = NULL;

			if( NULL != pHM && 0 == lUID )	// Valve created with the CBI (or from a later version of the project)
			{
				// Add a new valve in pHM and fill it.
				IDPTR ChildIDptr;
				m_pTADS->CreateObject( ChildIDptr, CLASS( CDS_HydroMod ) );
				pHM->Insert( ChildIDptr );

				CDS_HydroMod *pTADSValve = (CDS_HydroMod *)( ChildIDptr.MP );
				
				if( eBool3::eb3True == m_pTADS->IsHMCalcMode() )
				{
					IDPTR IDPtrSch = m_pTADB->Get( _T("DC_BV") );
					pTADSValve->Init( IDPtrSch, pHM, true );
					pTADSValve->SetDescription( _T("") );
					pTADSValve->GetpBv()->InvalidateSelection();			// Erase Balancing valve, it will be added later 
					pTADSValve->CreateTermUnit();
					pTADSValve->GetpTermUnit()->SetVirtual( false );
					pTADSValve->SetSchemeID( IDPtrSch.ID );
				}
				
				CopyValvData( pValve, pTADSValve, true );
				pTADSValve->SetTreatment( ett_Added );		// Tagged as added
				pTADSValve->SetTreatmentLparam( NULL );
			}
			else			// Valve existing already in the TADS
			{
				// Try to find the valve with same Uid in pHM.
				if( strRefName.Left( 3 ) != strPVmod )
				{
					pTADSValve = m_pTADS->FindHydroMod( (unsigned short)lUID, pHM );
				}
				else
				{
					// This a Ghost "PV_" module created for the partner valve of a root module
					pTADSValve = m_pTADS->FindHydroMod( (unsigned short)lUID, pTmpPipTab );
				}

				// Valve could not be found in its former module. Thus, Module has  been renamed but not moved otherwise Uid would have changed 
				// and thus would have been reset to 0 earlier in this function: We are dealing with an old version of the TAS2 project.
				if( NULL == pTADSValve )
				{
					*pfWarningOldVersion = true;
					return;	// Return after the whole processing (do that when all cases will be handled)
					// Search again the valve, but this time starting from the piping table
					//pTADSValve = m_pTADS->FindHydroMod((unsigned short)Uid, pTmpPipTab);
					// ...
					// If not found, see how to abort and say it is not possible
					//if (!pTADSValve)
					//	return;
				}

				// Check valve index.
				long lIndex;

				if( RD_OK != ReadLong( pValve->GetString( 3 ), &lIndex ) )
				{
					lIndex = 0;
				}
			
				// PV Index in CBI is always 1. In this case keep TAS2 local index.
				if( strRefName.Left( 3 ) != strPVmod && lIndex != pTADSValve->GetPos() )	// Index is different, re-index the valve
				{
					pTADSValve->SetTreatment( ett_Moved );	// Tagged as moved
					// SetTreatmentLparam contains old position.
					pTADSValve->SetTreatmentLparam( pTADSValve->GetPos() );
				}
				else
				{
					pTADSValve->SetTreatment( ett_Kept );		// Tagged as kept
				}

				CopyValvData( pValve, pTADSValve );
			}

		} // end of loop on TmpSavedData valves
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CPanelCBIPlant::AppendSelPlantToTADS'."), __LINE__, __FILE__ );
		throw;
	}
}

void CPanelCBIPlant::CopyValvData( CDB_MultiString* pTmpValv, CDS_HydroMod* pHM, bool fNew, unsigned short Uid )
{
	// Set the name stored in pTmpValv or recompose it.
	IDPTR ChildIDptr = pHM->GetIDPtr();
	long lVal;
	double dVal;
	CString ValvName = pTmpValv->GetString( 4 );
	ValvName.TrimRight();
	if( true == ValvName.IsEmpty() || '\r' == ValvName[0] )
	{
		pHM->SetHMName( _T("*") );
		m_pTADS->ComposeValveName( ChildIDptr );
	}
	else
		pHM->SetHMName( ValvName );

	// Set the CBI type and CBI size.
	CString str = pTmpValv->GetString( 5 );
	str.TrimLeft();
	str.TrimRight();
	pHM->SetCBIType( str );
	
	str = pTmpValv->GetString( 6 );
	str.TrimLeft();
	str.TrimRight();
	pHM->SetCBISize( str );

	// Set the Level and Uid.
	bool fMatch = false;
	
	if( true == fNew )	// Valve is new set level to 1 and New Uid if Uid=0
	{
		if( false == (ChildIDptr.PP)->IsClass( CLASS( CTable ) ) )						// Get Parent's level
		{
			pHM->SetLevel( ( (CDS_HydroMod *)( ChildIDptr.PP ) )->GetLevel() + 1 );
		}
		else																	// Parent is a TABLE
		{
			pHM->SetLevel( 0 );
		}

		if( 0 == Uid )
		{
			pHM->SetUid( m_pTADS->GetpProjectParams()->GetNextUid() );
		}
		else 
		{
			pHM->SetUid( Uid );
		}
	}
	else		// Valve exists, keep the level as is, copy Uid, check Valve type and size
	{
		long lTmp;
		if( RD_OK != ReadLong( pTmpValv->GetString( 2 ), &lTmp ) )
			lTmp = 1;
		
		pHM->SetUid( (unsigned short)lTmp );
		IDPTR idptr = pHM->GetTADBValveIDPtr();
		if( '\0' != *idptr.ID )				// Valve Exist
		{
			CDB_TAProduct *pTAP = (CDB_TAProduct *)( idptr.MP );
			
			// Check valve type and size
			if( '\0' != *pTAP->GetCBIType() && '\0' != *pHM->GetCBIType() )
			{
				if( 0 == _tcscmp(pTAP->GetCBIType(), pHM->GetCBIType()) && '\0' != *pHM->GetCBISize() )	// Type match
				{
					if( '\0' != *pTAP->GetCBISize() )
					{
						if( 0 == _tcscmp( pTAP->GetCBISize(), pHM->GetCBISize() ) )			// Size Match
							fMatch = true;
					}
					
					if( '\0' != *pTAP->GetCBISizeInch() ) 
					{
						if( 0 == _tcscmp( pTAP->GetCBISizeInch(), pHM->GetCBISize() ) )		// Size Match
							fMatch = true;
					}
				}
			}

			if( false == fMatch )		// Valve selected in TADB is not correct
			{
				pHM->SetTreatment( ett_ValveModified );
				pHM->SetTreatmentLparam( (LPARAM)pTAP );
				if( eBool3::eb3True == m_pTADS->IsHMCalcMode() )
				{
					if( NULL != pHM->GetpBv() )
					{
						pHM->GetpBv()->InvalidateSelection();
					}
				}
				else
					m_pTADS->DeleteValveFromTADS( pHM );					// delete it
			}
		}
	}
	
	CString	strRefName = pTmpValv->GetString( 1 );
	
	// Copy valve index only if it is not a Partner Valve of a Root module (always 1 in CBI).
	CString strPVmod = TASApp.LoadLocalizedString( IDS_PV );
	if( strRefName.Left( 3 ) != strPVmod )
	{
		if( RD_OK == ReadLong( pTmpValv->GetString( 3 ), &lVal ) )
			pHM->SetPos( lVal );
	}
	
	if( true == fNew )
		pHM->SetFlagModule( false );

	// For conversion to SI.
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();		
	UnitDesign_struct udQ = pUnitDB->GetUnit( _U_FLOW, 1 );				// Receive m³/h from CBI
	UnitDesign_struct udDp = pUnitDB->GetUnit( _U_DIFFPRESS, 2 );		// Receive bar from CBI
	UnitDesign_struct udCv = pUnitDB->GetUnit( _C_KVCVCOEFF, 1 );		// For when receiving Cv from CBI

	if( RD_OK == ReadDouble( pTmpValv->GetString( 7 ), &dVal ) )
	{
		if( dVal > 0.0 )	
			pHM->SetQDesign( GetConvOf( udQ ) * ( dVal + GetOffsetOf( udQ ) ) );
	}

	// Used for compatibility with previous hydromod that stored only one measurement by circuit.
	double dSettingTABal1 = 0.0;
	double dQTABal1 = 0.0;
	double dDpTABal1 = 0.0;
	double dDpTABal2 = 0.0;
	double dDpPvTABal = 0.0;
	double dSettingf = 0.0;
	double dQf = 0.0;
	double dDpf = 0.0;
	if( RD_OK == ReadDouble( pTmpValv->GetString( 13 ), &dVal ) )
	{
		if( dVal > 0.0 )		// Test DpTABal1 > 0
		{
			dDpTABal1 = GetConvOf( udDp ) * ( dVal + GetOffsetOf( udDp ) );
			if( RD_OK == ReadDouble( pTmpValv->GetString( 11 ), &dVal) )
			{
				if( dVal > 0.0 )
					dSettingTABal1 = dVal;
			}
			
			if( RD_OK == ReadDouble( pTmpValv->GetString( 14 ), &dVal ) )
			{
				if( dVal > 0.0 )
					dQTABal1 = GetConvOf( udQ ) * ( dVal + GetOffsetOf( udQ ) );
			}
			
			if( RD_OK == ReadDouble( pTmpValv->GetString( 10 ), &dVal ) )
			{
				if( dVal > 0.0 )
					dDpTABal2 = GetConvOf( udDp ) * ( dVal + GetOffsetOf( udDp ) );
			}
			
			if( RD_OK == ReadDouble( pTmpValv->GetString( 0 ), &dVal ) )
			{
				if( dVal > 0.0 )
					dDpPvTABal = GetConvOf( udDp ) * ( dVal + GetOffsetOf( udDp ) );
			}
		}
	}

	if( RD_OK == ReadDouble( pTmpValv->GetString( 9 ), &dVal ) )
	{
		if( dVal > 0.0 )		// Test Dpf > 0
		{
			dDpf = GetConvOf( udDp ) * ( dVal + GetOffsetOf( udDp ) );
			if( RD_OK == ReadDouble( pTmpValv->GetString( 8 ), &dVal ) )
				dQf = GetConvOf( udQ ) * ( dVal + GetOffsetOf( udQ ) );
			if( RD_OK == ReadDouble( pTmpValv->GetString( 12 ), &dVal ) )
				dSettingf = dVal;
		}
	}

	if( 0.0 != dSettingTABal1 || 0.0 != dQTABal1 || 0.0 != dDpTABal1 || 0.0 != dDpTABal2 || 0.0 != dDpPvTABal || 0.0 != dSettingf || 0.0 != dQf || 0.0 != dDpf )
	{
		// If CBI measurement already exist for this circuit overwrite it.
		CDS_HydroMod::CMeasData* pMD = pHM->GetpMeasDataForCBI();
		if( NULL == pMD )
		{
			pMD = pHM->CreateMeasData();			// Otherwise create a new entry
		}
		
		if( NULL != pMD )
		{
			pMD->SetDesignQ( pHM->GetQDesign() );
			pMD->SetTaBalOpening_1( dSettingTABal1 );
			pMD->SetTaBalMeasDp_1( dDpTABal1 );
			pMD->SetTaBalMeasDp_2( dDpTABal2 );
			pMD->SetTaBalOpeningResult( dSettingf );
			pMD->SetDpPVTABal( dDpPvTABal );
			pMD->Setqf( dQf );
			pMD->SetqTABal1( dQTABal1 );
			pMD->SetMeasDp( dDpf );

			// Define the measurement type.
			if( 0.0 != dQf )
			{
				pMD->SetQMType( CDS_HydroModX::eQMtype::QMFlow );
			}
			else if( 0.0 == dQf && 0.0 != dDpf )
			{
				pMD->SetQMType( CDS_HydroModX::eQMtype::QMDp );
			}
			else
			{
				pMD->SetQMType( CDS_HydroModX::eQMtype::QMundef );
			}				
		}
	}

	if( false == fNew && true == fMatch && edt_TADBValve == pHM->GetVDescrType() )	// Valve exists in SelP as a true TADB valve
		pHM->SetVDescrType( edt_TADBValve );	// already set
	else if( RD_OK == ReadLong( pTmpValv->GetString( 16 ), &lVal ) )	// Valve is new or reassigned because not 
	{															// matching the existing TADB valve stored in SelP
		if( 0 == lVal )	// edt_CBISizeValve or edt_CBISizeInchValve
		{
			if( true == m_pParent->GetUSflag() )
				pHM->SetVDescrType( edt_CBISizeInchValve );
			else
				pHM->SetVDescrType( edt_CBISizeValve );
		}
		else			// edt_KvCv
		{
			pHM->SetVDescrType( edt_KvCv );
			if( RD_OK == ReadDouble( pTmpValv->GetString( 15 ), &dVal ) )
			{
				if( 1 == lVal )	// Kv has been sent
					pHM->SetKvCv( dVal );
				else			// Cv has been sent
					pHM->SetKvCv( GetConvOf( udCv ) * dVal );
			}
		}
	}
}

void CPanelCBIPlant::ResetHMTreatment( CTable *pTab )
{
	if( false == pTab->IsClass( CLASS( CTable ) ) )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)pTab;
		if( '\0' != *pHM->GetCBIValveID() )
			pHM->SetTreatment( ett_Delete );
		else
			pHM->SetTreatment(ett_None );
		pHM->SetTreatmentLparam( NULL );
	}
	
	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
		ResetHMTreatment( ( (CTable *)IDPtr.MP ) );
}

void CPanelCBIPlant::DeleteTaggedValves( CTable* pHM )
{
	if( NULL == pHM )
		return;

	// Loop on children and recursively delete tagged valves.
	CRank IndexList;
	for( IDPTR IDPtr = pHM->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pHM->GetNext() )
	{
		CDS_HydroMod* pChild = (CDS_HydroMod*)( IDPtr.MP );
		DeleteTaggedValves( pChild );

		if( ett_Delete == pChild->GetTreatment() )
		{
			if( false == pChild->IsaModule() || 0 == pChild->GetItemCount() )
			{
				// In the case of GhostModule Keep index of deleted pChild in an ordered list for later re indexing.
				CString strPVmod = TASApp.LoadLocalizedString( IDS_PV );
				CString strGhostRefName = pChild->GetHMName();
				strGhostRefName.TrimRight();
				if( strGhostRefName.Left( 3 ) == strPVmod )
				{
					int iChildIndex = ( (CDS_HydroMod*)pChild )->GetPos();
					IndexList.Add( _T("Index"), (double)( INT_MAX - iChildIndex ), iChildIndex );
				}
				
				// Delete the CDS_HydroMod.
				TRACE( _T("Delete HM :%s %s\n"), IDPtr.ID, pChild->GetHMName() );
				pHM->Remove( IDPtr );
				m_pTADS->DeleteObject( IDPtr );
			}
			else if( true == pChild->IsaModule() && pChild->GetItemCount() > 0 ) //Remove Module's PV, Keep CDS_Hydromod
			{
				// Delete associated SelP valve in TADS.
				if( *( pChild->GetCBIValveID() ) )
					m_pTADS->DeleteValveFromTADS( pChild );
			}
		}
	}

	// Reindex the modules of pHM if the deletion of pChild has created hole(s) in the indexing.
	CString str;
	LPARAM itemdata;
	for( BOOL flag = IndexList.GetFirst( str, itemdata ); TRUE == flag; flag = IndexList.GetNext( str, itemdata ) )
		m_pTADS->RemoveIndexHole( pHM, (int)itemdata );
}

void CPanelCBIPlant::CopyMediaData( CDB_MultiString *pPlant )
{
	// Project WC.
	CDS_WaterCharacteristic* pWC = m_pTADS->GetpWCForProject();
	ASSERT( NULL != pWC );

	// Media type.
	CString MediaType = pPlant->GetString( 5 );
	MediaType.TrimRight();

	// For conversion to SI.
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();		
	UnitDesign_struct ud;

	// Act differently depending on Media type.
	const double dTol = 1.0e-3;
	bool fModified = false;
	if( _T("WATER") == MediaType )
	{
		double dTemp;
		if( RD_OK != ReadDouble( pPlant->GetString( 6 ), &dTemp ) )
			return;
		
		ud = pUnitDB->GetUnit( _U_TEMPERATURE, 0 );			// Receive °C from CBI
		dTemp = GetConvOf( ud ) * ( dTemp + GetOffsetOf( ud ) );

		double dDiff = fabs( dTemp - pWC->GetTemp() );
		if( 0.0 != dTemp )
			dDiff/=dTemp;
		
		if( MediaType != pWC->GetAdditID() || dDiff > dTol )	// Media is not the same
		{
			pWC->SetAdditFamID( _T("WATER_ADDIT") );
			pWC->SetAdditID( _T("WATER") );
			pWC->SetTemp( dTemp );
			fModified = true;
		}
	}
	else if( _T("PROPYL_GLYC") == MediaType || _T("ETHYL_GLYC") == MediaType )
	{
		double dTemp, dConc;
		
		// Media temperature.
		if( RD_OK != ReadDouble( pPlant->GetString( 6 ), &dTemp ) )
			return;
		
		ud = pUnitDB->GetUnit( _U_TEMPERATURE, 0 );			// Receive °C from CBI
		dTemp = GetConvOf( ud ) * ( dTemp + GetOffsetOf( ud ) );
		
		// Media concentration.
		if( RD_OK != ReadDouble( pPlant->GetString( 7 ), &dConc ) )
			return;

		double dDiff1 = fabs( dTemp - pWC->GetTemp() );
		if( 0.0 != dTemp )
			dDiff1 /= dTemp;
		
		double dDiff2 = fabs( dConc - pWC->GetPcWeight() );
		if( 0.0 != dConc )
			dDiff2 /= dConc;
		
		if( MediaType != pWC->GetAdditID() || dDiff1 > dTol || dDiff2 > dTol )	// Media is not the same
		{
			pWC->SetAdditFamID( _T("GLYCOL_ADDIT") );
			pWC->SetAdditID( MediaType );
			pWC->SetTemp( dTemp );
			pWC->SetPcWeight( dConc );
			fModified = true;
		}
	}
	else if( _T("OTHER_FLUID") == MediaType )
	{
		double dDens;
		double dVisc;
		
		// Media density.
		if( RD_OK != ReadDouble( pPlant->GetString( 6 ), &dDens ) )
			return;
		
		ud = pUnitDB->GetUnit( _U_DENSITY, 0 );				// Receive kg/m³ from CBI
		dDens = GetConvOf( ud ) * ( dDens + GetOffsetOf( ud ) );
		
		// Media viscosity.
		if( RD_OK != ReadDouble( pPlant->GetString( 7 ), &dVisc ) )
			return;
		
		ud = pUnitDB->GetUnit( _U_KINVISCOSITY, 2 );			// Receive mm²/s from CBI
		dVisc = GetConvOf( ud ) * ( dVisc + GetOffsetOf( ud ) );

		double dDiff1 = fabs( dDens - pWC->GetDens() );
		if( 0.0 != dDens )
			dDiff1 /= dDens;
		
		double dDiff2 = fabs( dVisc - pWC->GetKinVisc() );
		if( 0.0 != dVisc )
			dDiff2 /= dVisc;
		
		if( MediaType != pWC->GetAdditID() || dDiff1 > dTol || dDiff2 > dTol )	// Media is not the same
		{
			pWC->SetAdditFamID( _T("OTHER_ADDIT") );
			pWC->SetAdditID( _T("OTHER_FLUID") );
			pWC->SetDens( dDens );
			pWC->SetKinVisc( dVisc );
			fModified = true;
		}
	}
	else
		return;
	
	// Update parent Water Char.
	*m_pParent->GetpWaterChar() = *pWC->GetpWCData();

	// Send a Proj Water change message if needed.
	if( true == fModified )
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_WATERCHANGE, (WPARAM)WMUserWaterCharWParam::WM_UWC_WP_ForProject );
}
