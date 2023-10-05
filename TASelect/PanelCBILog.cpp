#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include "TASelect.h"

#include "Utilities.h"
#include "HydroMod.h"
#include "WizardCBI.h"
#include "PanelCBILog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPanelCBILog::CPanelCBILog( CWnd* pParent )
	: CDlgWizard( CPanelCBILog::IDD, pParent )
{
	m_pTADS = NULL;
	m_pParent = NULL;
}

CPanelCBILog::~CPanelCBILog()
{
	m_brWhiteBrush.DeleteObject();	
}

BEGIN_MESSAGE_MAP( CPanelCBILog, CDlgWizard )
	ON_WM_CTLCOLOR()
	ON_NOTIFY( LVN_ENDLABELEDIT, IDC_LIST, OnEndlabeleditList )
	ON_BN_CLICKED( IDC_BUTSELECT, OnButselect )
	ON_BN_CLICKED( IDC_BUTUNSELECT, OnButunselect )
	ON_NOTIFY( NM_CLICK, IDC_LIST, OnClickList )
END_MESSAGE_MAP()

void CPanelCBILog::DoDataExchange( CDataExchange* pDX )
{
	CDlgWizard::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST, m_List );
	DDX_Control( pDX, IDC_EDITTEXT, m_EditText );
}

BOOL CPanelCBILog::OnInitDialog() 
{
	CDlgWizard::OnInitDialog();

	// Set the white rectangle to the first drawn window.
	GetDlgItem( IDC_STATICRECTANGLE )->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );

	m_pParent = (CWizardCBI *)GetWizMan();
	m_pTADS = TASApp.GetpTADS();
	
	m_ImageList.Create( IDB_CHECKEDUNCHECKED, 16, 1, _BLACK );
	m_List.SetImageList( &m_ImageList, LVSIL_STATE );

	CString str = TASApp.LoadLocalizedString( IDS_SELECTALL );
	( (CButton*)GetDlgItem( IDC_BUTSELECT ) )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_UNSELECTALL );
	( (CButton*)GetDlgItem( IDC_BUTUNSELECT ) )->SetWindowText( str );
	
	return TRUE;
}

HBRUSH CPanelCBILog::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor ) 
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

void CPanelCBILog::OnEndlabeleditList( NMHDR* pNMHDR, LRESULT* pResult ) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CDS_LoggedData* pLD = (CDS_LoggedData *)pDispInfo->item.lParam;
	if( NULL != pLD )
	{
		if( pDispInfo->item.pszText )
		{
			pLD->SetName( pDispInfo->item.pszText );
			m_List.SetItem( pDispInfo->item.iItem, 0, LVIF_TEXT, pDispInfo->item.pszText, 0, 0, 0, 0 );
		}
	}
	if( NULL != pResult )
		*pResult = 0;
}

void CPanelCBILog::OnButselect() 
{
	for( int i = 0; i < m_List.GetItemCount(); i++ )
		m_List.SetCheck( i, TRUE );
}

void CPanelCBILog::OnButunselect() 
{
	for( int i = 0; i < m_List.GetItemCount(); i++ )
		m_List.SetCheck( i, FALSE );
}

void CPanelCBILog::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Retrieve where the user clicked.
	LVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= GET_X_LPARAM( dwpos );
	ht.pt.y= GET_Y_LPARAM( dwpos );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	m_List.HitTest( &ht );

	// If the user click on the State Image, change selected to unselected and inversely.
	CRect Rect, LabelRect;
	m_List.GetItemRect( ht.iItem, &Rect, LVIR_BOUNDS );
	m_List.GetItemRect( ht.iItem, &LabelRect, LVIR_LABEL );
	Rect.right = LabelRect.left - 1;
	if( TRUE == Rect.PtInRect( ht.pt ) )
		m_List.SetCheck( ht.iItem, !m_List.GetCheck( ht.iItem ) );

	if( NULL != pResult )
		*pResult = 0;
}

BOOL CPanelCBILog::OnWizFinish()
{
	try
	{
		LVITEM lvItem;	
		lvItem.mask = LVIF_PARAM; 
		lvItem.iSubItem = 0; 
	
		// Check if the selected logged data name already exist into the LogDataTab.
		CTable *pTab = m_pTADS->GetpLogDataTable();
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'LOGDATA_TAB' table from the datastruct.") );
		}
	
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// The table can have CDS_LoggedData, CDS_LogDataX and TMPLOGDATA_TAB.
			// Only the two first are CLog.
			if( true == IDPtr.MP->IsaTable() )
			{
				continue;
			}

			// Check each selected item.
			for( int i = 0; i < m_List.GetItemCount(); i++ )
			{
				// Not selected.
				if( FALSE == m_List.GetCheck( i ) )
				{
					continue; 
				}

				if( 0 == _tcscmp( ( (CDS_LoggedData*)IDPtr.MP )->GetName(), (LPCTSTR)m_List.GetItemText( i, 0 ) ) )
				{
					CString str; 
					FormatString( str, IDS_LOGGEDDATA_EXIST, ( (CDS_LoggedData*)IDPtr.MP )->GetName() );
					AfxMessageBox( str );
					return FALSE;
				}
			}
		}		
	
		// Move selected logged data from temporary tab to the log data tab.
		for( int i = 0; i < m_List.GetItemCount(); i++ )
		{
			CDS_LoggedData *pLoggedData = NULL;
			CTable *pTmpTab = (CTable *)( m_pTADS->Get( _T("TMPLOGDATA_TAB") ).MP );
			
			if( NULL == pTmpTab )
			{
				HYSELECT_THROW( _T("Internal error: can't retrieve the 'TMPLOGDATA_TAB' table from the datastruct.") );
			}

			// Selected.
			if( TRUE == m_List.GetCheck( i ) ) 
			{
				lvItem.iItem = i; 
				m_List.GetItem( &lvItem );
				pLoggedData = (CDS_LoggedData *)( lvItem.lParam );
				
				if( NULL == pLoggedData )
				{
					HYSELECT_THROW( _T("Internal error: parameter for the item at postion %i is NULL."), i );
				}

				IDPTR IDPtr = pLoggedData->GetIDPtr();
			
				// Remove from temporary LogData.
				pTmpTab->Remove( IDPtr );
			
				// Insert into LogData Tab.
				pTab->Insert( IDPtr );
			
				m_pTADS->Modified();
			}
		}	
	
		return TRUE;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CPanelCBILog::OnWizFinish'."), __LINE__, __FILE__ );
		throw;
	}
}

bool CPanelCBILog::OnActivate()
{
	m_List.DeleteAllItems();
	CDS_LoggedData *pLD;
	CTable* pTab = (CTable *)( m_pTADS->Get( _T("TMPLOGDATA_TAB") ).MP );
	ASSERT( NULL != pTab );

	CString str;
	if( pTab->GetItemCount( CLASS( CDS_LoggedData ) ) > 0 )
		str = TASApp.LoadLocalizedString( IDS_PANELCBILOG_TEXT );
	else
		str = TASApp.LoadLocalizedString( IDS_PANELCBILOG_TEXTNOLD );
	m_EditText.SetWindowText( str );

	// Define the width of each column depending on the width of the list control.
	CRect rect;
	m_List.GetClientRect( (LPRECT)&rect );
	
	// Add columns.
	// Size of columns is defined by the size of columns header, it must be formatted with a sufficient number of space.
	str = TASApp.LoadLocalizedString( IDS_PANELCBILOG_NAME );
	
	m_List.InsertColumn( 0, str, LVCFMT_LEFT, (int)( rect.Width() * 0.2 ), 0 );
	str = TASApp.LoadLocalizedString( IDS_PANELCBILOG_DATE );
	
	m_List.InsertColumn( 1, str, LVCFMT_LEFT, (int)( rect.Width() * 0.2 ), 1 );
	str = TASApp.LoadLocalizedString(IDS_PANELCBILOG_PLANT);
	
	m_List.InsertColumn( 2, str, LVCFMT_LEFT, (int)( rect.Width() * 0.2 ), 2 );
	str = TASApp.LoadLocalizedString( IDS_PANELCBILOG_REF );
	
	m_List.InsertColumn( 3, str, LVCFMT_LEFT, (int)( rect.Width() * 0.15 ), 3 );
	str = TASApp.LoadLocalizedString( IDS_PANELCBILOG_VALVE );
	
	m_List.InsertColumn( 4, str, LVCFMT_LEFT, (int)( rect.Width() * 0.15 ), 4 );
	str = TASApp.LoadLocalizedString( IDS_PANELCBILOG_DATA );
	
	m_List.InsertColumn( 5, str, LVCFMT_LEFT, (int)( rect.Width() * 0.1 ), 5 );

	// Add all 'CDS_LoggedData' into the list.
	LVITEM lvItem;	
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
	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		pLD = (CDS_LoggedData *)( IDPtr.MP );
		lvItem.pszText = (TCHAR *)pLD->GetName();
		lvItem.lParam = (LPARAM) pLD;
		m_List.InsertItem( &lvItem );

		CTimeUnic dtu;
		pLD->GetDateTime( 0, dtu );
		CString strDT = dtu.Format( IDS_LOGDATETIME_FORMAT );
		m_List.SetItem( 0, 1, LVIF_TEXT, strDT, 0, 0, 0, 0 );
		m_List.SetItem( 0, 2, LVIF_TEXT, pLD->GetSite(), 0, 0, 0, 0 );
		m_List.SetItem( 0, 3, LVIF_TEXT, pLD->GetRef(), 0, 0, 0, 0 );
	
		str.Format( _T("%d"), pLD->GetValveIndex() );
		m_List.SetItem( 0, 4, LVIF_TEXT, str, 0, 0, 0, 0 );
		str.Format( _T("%d"), pLD->GetLength() );
		m_List.SetItem( 0, 5, LVIF_TEXT, str, 0, 0, 0, 0 );
	}
	
	// Set the check state outside the creation loop otherwise the first list item get no state image.
	for( int i = 0; i < m_List.GetItemCount(); i++ )
		m_List.SetCheck( i, TRUE );
	
	return true;
}

bool CPanelCBILog::OnAfterActivate()
{
	CDlgWizard::OnAfterActivate();

	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	SetFocus();
	return true;
}
