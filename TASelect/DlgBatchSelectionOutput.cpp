#include "stdafx.h"
#include <algorithm>
#include "ATLBase.h"
#include "AtlConv.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "DlgOutput.h"
#include "DlgBatchSelectionOutput.h"
#include "ToolsDockablePane.h"
#include "DlgLeftTabProject.h"
#include "RViewHMCalc.h"

IMPLEMENT_DYNAMIC( CDlgBatchSelectionOutput, CDlgOutput )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgBatchSelectionOutput::CInterfaceBSO class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDlgBatchSelectionOutput::CBSInterface::CBSInterface()
	: CBaseInterface()
{
	m_pDlgBatchSelectionOutput = NULL;
	m_pclBSMessage = NULL;
	Init();
}

CDlgBatchSelectionOutput::CBSInterface::~CBSInterface()
{
	if( m_pclBSMessage != NULL )
	{
		delete m_pclBSMessage;
	}
}

bool CDlgBatchSelectionOutput::CBSInterface::Init( void )
{
	if( m_pDlgBatchSelectionOutput != NULL )
	{
		return true;
	}

	if( NULL == pMainFrame || NULL == pMainFrame->GetpToolsDockablePane() )
	{
		return false;
	}

	m_pDlgBatchSelectionOutput = pMainFrame->GetpToolsDockablePane()->GetpDlgBatchSelectionOutput();

	if( NULL == m_pDlgBatchSelectionOutput )
	{
		return false;
	}

	if( NULL == m_pclBSMessage )
	{
		m_pclBSMessage = new CBSMessage( m_pDlgBatchSelectionOutput );
	}
	else
	{
		m_pclBSMessage->Clear();
	}

	m_pDlgOutput = m_pDlgBatchSelectionOutput;
	return true;
}

bool CDlgBatchSelectionOutput::CBSInterface::EnableCtxtMenuPasteData( bool fEnable, int iOutputID )
{
	bool fReturn = false;

	if( true == Init() )
	{
		fReturn = m_pDlgBatchSelectionOutput->EnableCtxtMenuPasteData( fEnable, iOutputID );
	}

	return fReturn;
}

bool CDlgBatchSelectionOutput::CBSInterface::EnableCtxtMenuAddColumn( bool fEnable, int iOutputID )
{
	bool fReturn = false;

	if( true == Init() )
	{
		fReturn = m_pDlgBatchSelectionOutput->EnableCtxtMenuAddColumn( fEnable, iOutputID );
	}

	return fReturn;
}

CDlgBatchSelectionOutput::CBSMessage *CDlgBatchSelectionOutput::CBSInterface::GetMessage( void )
{
	CBSMessage *pclBSMessage = NULL;

	if( true == Init() )
	{
		pclBSMessage = m_pclBSMessage;
	}

	return pclBSMessage;
}

bool CDlgBatchSelectionOutput::CBSInterface::RegisterNotificationHandler( CDlgBatchSelectionOutput::IBSNotificationHandler *pclHandler,
		int iNotificationHandlerFlags, int iOutputID )
{
	bool fReturn = false;

	if( true == Init() )
	{
		fReturn = m_pDlgBatchSelectionOutput->RegisterNotificationHandler( pclHandler, iNotificationHandlerFlags, iOutputID );
	}

	return fReturn;
}

bool CDlgBatchSelectionOutput::CBSInterface::UnregisterNotificationHandler( CDlgBatchSelectionOutput::IBSNotificationHandler *pclHandler, int iOutputID )
{
	bool fReturn = false;

	if( true == Init() )
	{
		fReturn = m_pDlgBatchSelectionOutput->UnregisterNotificationHandler( pclHandler, iOutputID );
	}

	return fReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgBatchSelectionOutput class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgBatchSelectionOutput::CDlgBatchSelectionOutput()
	: CDlgOutput( false, DLGBSOOID )
{
}

bool CDlgBatchSelectionOutput::Reset( int iOutputID )
{
	if( false == CDlgOutput::Reset( iOutputID ) )
	{
		return false;
	}

	if( -1 == iOutputID )
	{
		for( iterBSOutputContext iter = m_mapBSOutputContextParams.begin(); iter != m_mapBSOutputContextParams.end(); iter++ )
		{
			iter->second.m_fCtxtMenuPasteDataEnabled = true;
			iter->second.m_fCtxtMenuAddColumnEnabled = true;
			iter->second.m_iLastColumnIDRightClicked = -1;
			iter->second.m_lLastRowRightClicked = -1;
		}
	}
	else if( 0 != m_mapBSOutputContextParams.count( iOutputID ) )
	{
		m_mapBSOutputContextParams[iOutputID].m_fCtxtMenuPasteDataEnabled = true;
		m_mapBSOutputContextParams[iOutputID].m_fCtxtMenuAddColumnEnabled = true;
		m_mapBSOutputContextParams[iOutputID].m_iLastColumnIDRightClicked = -1;
		m_mapBSOutputContextParams[iOutputID].m_lLastRowRightClicked = -1;
	}

	return true;
}

bool CDlgBatchSelectionOutput::EnableCtxtMenuPasteData( bool fEnable, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapBSOutputContextParams.count( iOutputID ) )
	{
		return false;
	}

	m_mapBSOutputContextParams[iOutputID].m_fCtxtMenuPasteDataEnabled = fEnable;
	return true;
}

bool CDlgBatchSelectionOutput::EnableCtxtMenuAddColumn( bool fEnable, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapBSOutputContextParams.count( iOutputID ) )
	{
		return false;
	}

	m_mapBSOutputContextParams[iOutputID].m_fCtxtMenuAddColumnEnabled = fEnable;
	return true;
}

bool CDlgBatchSelectionOutput::SendMessage( CBSMessage *pclBSMessage, long lRow, bool fScrollOnTheLastLine, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return false;
	}

	return m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->SendMessage( pclBSMessage, NULL, lRow, fScrollOnTheLastLine );
}

void CDlgBatchSelectionOutput::OnHeaderButtonClicked( int iButtonID, CVButton::State eState )
{
	if( -1 == m_iOutputID || 0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	DlgOutputHelper::CCellFilterList *pclCellFilter = m_mapBSOutputContextParams[m_iOutputID].m_clFilterList.GetCellFilter( iButtonID );

	switch( iButtonID )
	{
		case DLGBSO_BUTTONID_CLEARALL:
			OnFltMenuClearAll();
			break;

		case DLGBSO_BUTTONID_CLEARRESULTS:
			OnFltMenuClearResults();
			break;

		default:
			break;
	}
}

void CDlgBatchSelectionOutput::OnOutputSpreadContainerCellRClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse,
		DlgOutputHelper::vecCCellParam *pVecParamList )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer
		|| 0 == m_mapBSOutputContextParams.count( iOutputID ) )
	{
		return;
	}

	// Save coordinates before calling 'ShowPopupMenu'.
	m_mapBSOutputContextParams[iOutputID].m_iLastColumnIDRightClicked = iColumnID;
	m_mapBSOutputContextParams[iOutputID].m_lLastRowRightClicked = lRow;
	m_mapBSOutputContextParams[iOutputID].m_clLastMousePointRightClicked = ptMouse;

	// Remark: to customize a CMFCPopupMenu the only way to do it it's when framework calls 'OnShowPopupMenu' to allow us to update/create ourself
	//         the menu. For that we add our menu in the 'CContextMenuManager' when application starts (in the 'CTASelectApp:PreLoadState' method).
	//         We thus just need here to call the 'CContextMenuManager::ShowPopupMenu'. This one will prepare the menu (loading the resource in
	//         a 'CMenu' class and importing in a 'CMFCPopupMenu' class) and will call the 'CMainFrame::OnShowPopupMenu' method. We implement
	//         a user message to notify this dialog (see the 'OnShowPopupMenu' method in this dialog).
	//
	//         Keep in mind that MFC internally works with HMENU that is in fact a handle for the 'CMenu' class. It is thus impossible to prepare
	//         here a complete MFC menu with its submenu and so on because their will be converted in a 'CMenu'.
	//         Now, with this method, we can add inherited class from 'CMFCToolBarMenuButton' to customize the drawing of items (we do that in
	//         the 'OnShowPopupMenu'). Because from this point the 'CMFCPopupMenu' is created and it's the one that will be used and shown. Thus
	//         no more internal conversion.
	m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->ClientToScreen( &ptMouse );
	TASApp.GetContextMenuManager()->ShowPopupMenu( IDR_FLTMENU_BATCHSEL, ptMouse.x, ptMouse.y, this, TRUE );
}

void CDlgBatchSelectionOutput::OnOutputSpreadContainerKeyboardShortcut( int iOutputID, int iKeyboardShortcut )
{
	switch( iKeyboardShortcut )
	{
		case KS_CONTROL_A:
			OnFltMenuSelectAll();
			break;

		case KS_CONTROL_C:
			OnFltMenuCopy();
			break;

		case KS_CONTROL_V:
			_PasteData( true );
			break;
	}
}

void CDlgBatchSelectionOutput::OnOutputSpreadContainerKeyboardVirtualKey( int iOutputID, int iKeyboardVirtualKey )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapOutputContextParams.count( iOutputID ) || NULL == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer
		|| 0 == m_mapBSOutputContextParams.count( iOutputID ) )
	{
		return;
	}

	// This variable is set to 'true' is we can execute the key. Otherwise, it is set to 'false'.
	bool fExecuteKey = false;

	if( 0 != m_mapBSOutputContextParams[iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnKeyboardVirtualKey ) )
	{
		fExecuteKey =
			m_mapBSOutputContextParams[iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnKeyboardVirtualKey]->OnBatchSelectionKeyboardVirtualKey(
				iKeyboardVirtualKey );
	}

	if( true == fExecuteKey )
	{
		switch( iKeyboardVirtualKey )
		{
			case VK_DELETE:
				if( true == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->IsBlockSelectionExist() &&
					true == m_mapOutputContextParams[iOutputID].m_pclDlgOutputSpreadContainer->IsDataExistInBlockSelection() )
				{
					OnFltMenuDelete();
				}

				break;
		}
	}
}

bool CDlgBatchSelectionOutput::RegisterNotificationHandler( IBSNotificationHandler *pclHandler, int iNotificationHandlerFlags, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapBSOutputContextParams.count( iOutputID ) )
	{
		return false;
	}

	// Handler for the base class.
	CDlgOutput::RegisterNotificationHandler( ( CDlgOutput::IBaseNotificationHandler * )pclHandler, iNotificationHandlerFlags, iOutputID );

	// Handler for this class.
	if( 0 != m_mapBSOutputContextParams.count( iOutputID ) )
	{
		for( int iLoop = IBSNotificationHandler::BSNH_First; iLoop <= IBSNotificationHandler::BSNH_Last; iLoop <<= 1 )
		{
			if( iLoop == ( iNotificationHandlerFlags & iLoop ) )
			{
				m_mapBSOutputContextParams[iOutputID].m_mapBSNotificationHandler[iLoop] = pclHandler;
			}
		}
	}

	return true;
}

bool CDlgBatchSelectionOutput::UnregisterNotificationHandler( IBSNotificationHandler *pclHandler, int iOutputID )
{
	if( -1 == iOutputID )
	{
		iOutputID = m_iOutputID;
	}

	if( 0 == m_mapBSOutputContextParams.count( iOutputID ) )
	{
		return false;
	}

	// Handler for the base class.
	CDlgOutput::UnregisterNotificationHandler( ( CDlgOutput::IBaseNotificationHandler * )pclHandler, iOutputID );

	if( 0 != m_mapBSOutputContextParams.count( iOutputID ) )
	{
		for( mapIntBSNHIter iter = m_mapBSOutputContextParams[iOutputID].m_mapBSNotificationHandler.begin();
			 iter != m_mapBSOutputContextParams[iOutputID].m_mapBSNotificationHandler.end(); iter++ )
		{
			if( iter->second == pclHandler )
			{
				m_mapBSOutputContextParams[iOutputID].m_mapBSNotificationHandler.erase( iter );
				break;
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected members
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CDlgBatchSelectionOutput, CDlgOutput )
	ON_UPDATE_COMMAND_UI_RANGE( ID_BATCHSELECTION_COPY, ID_BATCHSELECTION_REMOVECOLUMN, OnUpdateMenuText )
	ON_UPDATE_COMMAND_UI_RANGE( ID_BATCHSELECTION_ADDCOLSTART, ID_BATCHSELECTION_ADDCOLEND, OnUpdateMenuText )
	ON_UPDATE_COMMAND_UI_RANGE( ID_BATCHSELECTION_DELCOLSTART, ID_BATCHSELECTION_DELCOLEND, OnUpdateMenuText )
	ON_COMMAND_RANGE( ID_BATCHSELECTION_ADDCOLSTART, ID_BATCHSELECTION_ADDCOLEND, OnFltMenuAddColumn )
	ON_COMMAND_RANGE( ID_BATCHSELECTION_DELCOLSTART, ID_BATCHSELECTION_DELCOLEND, OnFltMenuRemoveColumn )
	ON_COMMAND( ID_BATCHSELECTION_COPY, OnFltMenuCopy )
	ON_COMMAND( ID_BATCHSELECTION_PASTE, OnFltMenuPaste )
	ON_COMMAND( ID_BATCHSELECTION_DELETE, OnFltMenuDelete )
	ON_COMMAND( ID_BATCHSELECTION_EDIT, OnFltMenuEdit )
	ON_COMMAND( ID_BATCHSELECTION_DELROW, OnFltMenuDelRow )
	ON_COMMAND( ID_BATCHSELECTION_INSROW, OnFltMenuInsRow )
	ON_COMMAND( ID_BATCHSELECTION_SELECTALL, OnFltMenuSelectAll )
	ON_COMMAND( ID_BATCHSELECTION_CLEARALL, OnFltMenuClearAll )
	ON_COMMAND( ID_BATCHSELECTION_SUGGEST, OnFltMenuSuggest )
	ON_COMMAND( ID_BATCHSELECTION_VALIDATE, OnFltMenuValidate )
	ON_COMMAND( ID_BATCHSELECTION_EDITRESULT, OnFltMenuEditResult )
	ON_COMMAND( ID_BATCHSELECTION_CLEARRESULTS, OnFltMenuClearResults )
	ON_COMMAND( ID_BATCHSELECTION_GETFULLCAT, OnFltMenuGetFullCat )
	ON_MESSAGE( WM_USER_SHOWPOPUPMENU, OnShowPopupMenu )
END_MESSAGE_MAP()

BOOL CDlgBatchSelectionOutput::OnInitDialog()
{
	if( FALSE == CDlgOutput::OnInitDialog() )
	{
		return FALSE;
	}

	AddTopPanelButton( DLGBSO_BUTTONID_CLEARALL, IDS_BSCOMPILOUTPUT_CLEARALL, -1, -1, true, CSize( 80, 20 ) );
	AddTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, IDS_BSCOMPILOUTPUT_CLEARRESULT, -1, -1, true, CSize( 80, 20 ) );
	EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, false );

	return TRUE;
}

void CDlgBatchSelectionOutput::OnUpdateMenuText( CCmdUI *pCmdUI )
{
	pMainFrame->UpdateMenuToolTip( pCmdUI );
}

void CDlgBatchSelectionOutput::OnFltMenuCopy()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return;
	}

	if( false == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsBlockSelectionExist() )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;

	if( false == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetBlockSelectionAbsolute( lStartCol, lStartRow, lEndCol, lEndRow ) )
	{
		return;
	}

	CString strClipboard( _T( "" ) );
	HGLOBAL hDataText = NULL;

	if( false == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->CopyDataInCString( lStartCol, lStartRow, lEndCol, lEndRow, strClipboard ) )
	{
		return;
	}

	int iSize = ( strClipboard.GetLength() + 1 ) * sizeof( TCHAR );
	hDataText = GlobalAlloc( GMEM_FIXED, iSize );
	LPVOID lpDest = GlobalLock( hDataText );
	memcpy_s( lpDest, iSize, strClipboard.GetBuffer(), iSize );
	GlobalUnlock( hDataText );

	// Try the XML Spreadsheet format (better for Excel).
	HGLOBAL hDataXML = NULL;
	UINT uiFormat = RegisterClipboardFormat( _T("XML Spreadsheet") );

	if( 0 != uiFormat )
	{
		if( false == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->CopyDataInXMLSpreadsheet( lStartCol, lStartRow, lEndCol, lEndRow,
				strClipboard ) )
		{
			return;
		}

		int iSize = ( strClipboard.GetLength() + 1 ) * sizeof( TCHAR );
		hDataXML = GlobalAlloc( GMEM_FIXED, iSize / 2 );
		LPVOID lpDest = GlobalLock( hDataXML );
		USES_CONVERSION;
		memcpy_s( lpDest, iSize, W2A( strClipboard.GetBuffer() ), iSize / 2 );
		GlobalUnlock( hDataXML );
	}

	if( TRUE == OpenClipboard() )
	{
		EmptyClipboard();
		::SetClipboardData( CF_UNICODETEXT, hDataText );

		if( NULL != hDataXML )
		{
			::SetClipboardData( uiFormat, hDataXML );
		}

		CloseClipboard();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuPaste()
{
	_PasteData( false );
}

void CDlgBatchSelectionOutput::OnFltMenuDelete()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer ||
		0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	bool fIsRedrawBlocked = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsRedrawBlocked();

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->BlockRedraw();
	}

	int iStartColID, iEndColID;
	long lStartRowRelative, lEndRowRelative;
	m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetBlockSelectionRelative( iStartColID, lStartRowRelative, iEndColID, lEndRowRelative );

	m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->EraseDataInBlockSelection();

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnDelete ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnDelete]->OnBatchSelectionDelete( iStartColID, iEndColID,
				lStartRowRelative, lEndRowRelative );
	}

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->ReleaseRedraw();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuEdit()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return;
	}

	m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->EditCurrentCell();
}

void CDlgBatchSelectionOutput::OnFltMenuDelRow()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return;
	}

	long lStartRowRelative;
	long lEndRowRelative;

	if( false == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetRowSelection( lStartRowRelative, lEndRowRelative ) )
	{
		return;
	}
	
	bool fIsRedrawBlocked = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsRedrawBlocked();

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->BlockRedraw();
	}

	m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->DeleteRowSelection();

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnDeleteRow ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnDeleteRow]->OnBatchSelectionDeleteRow( lStartRowRelative, lEndRowRelative );
	}

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->ReleaseRedraw();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuInsRow()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return;
	}

	// TODO
}

void CDlgBatchSelectionOutput::OnFltMenuSelectAll()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer )
	{
		return;
	}

	bool fIsRedrawBlocked = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsRedrawBlocked();

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->BlockRedraw();
	}

	m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->SetBlockSelectionAll();

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->ReleaseRedraw();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuClearAll()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer ||
		0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	bool fIsRedrawBlocked = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsRedrawBlocked();

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->BlockRedraw();
	}

	ClearOutput();

	if( false == fIsRedrawBlocked )
	{
		m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->ReleaseRedraw();
	}

	EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, false );

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnButtonClearAll ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnButtonClearAll]->OnBatchSelectionButtonClearAll();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuSuggest()
{
	if( -1 == m_iOutputID || 0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnSuggest ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnSuggest]->OnBatchSelectionSuggest();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuValidate()
{
	if( -1 == m_iOutputID || 0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnValidate ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnValidate]->OnBatchSelectionValidate();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuEditResult()
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer ||
		0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	int iStartColID, iEndColID;
	long lStartRowRelative, lEndRowRelative;
	m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetBlockSelectionRelative( iStartColID, lStartRowRelative, iEndColID, lEndRowRelative );

	if( lStartRowRelative != lEndRowRelative )
	{
		return;
	}

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnEdit ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnEdit]->OnBatchSelectionEdit( lStartRowRelative );
	}
}

void CDlgBatchSelectionOutput::OnFltMenuClearResults()
{
	if( -1 == m_iOutputID || 0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, false );

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnButtonClearResults ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnButtonClearResults]->OnBatchSelectionButtonClearResults();
	}
}

void CDlgBatchSelectionOutput::OnFltMenuGetFullCat()
{
	if( NULL == m_mapBSOutputContextParams[m_iOutputID].m_pclProductForQRSCat )
	{
		return;
	}

	LPARAM lParam = ( LPARAM )m_mapBSOutputContextParams[m_iOutputID].m_pclProductForQRSCat->GetIDPtr().ID;
	pMainFrame->PostMessageToDescendants( WM_USER_DISPLAYDOCUMENTATION, 0, lParam );

	m_mapBSOutputContextParams[m_iOutputID].m_pclProductForQRSCat = NULL;
}

void CDlgBatchSelectionOutput::OnFltMenuAddColumn( UINT nID )
{
	if( -1 == m_iOutputID || 0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	// Verify first if user has clicked on the 'Add all columns' item.
	int iColumnID = 0;

	if( ID_BATCHSELECTION_ADDCOLSTART == nID )
	{
		iColumnID = -1;
	}
	else if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapColumnList.count( nID - ID_BATCHSELECTION_ADDCOLSTART ) )
	{
		iColumnID = ( int )m_mapBSOutputContextParams[m_iOutputID].m_mapColumnList[nID - ID_BATCHSELECTION_ADDCOLSTART].m_lColumnID;
	}
	else
	{
		return;
	}

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnAddColumn ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnAddColumn]->OnBatchSelectionAddColumn( iColumnID );
	}
}

void CDlgBatchSelectionOutput::OnFltMenuRemoveColumn( UINT nID )
{
	if( -1 == m_iOutputID || 0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	// Verify first if user has clicked on the 'Add all columns' item.
	int iColumnID = 0;

	if( ID_BATCHSELECTION_DELCOLSTART == nID )
	{
		iColumnID = -1;
	}
	else if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapColumnList.count( nID - ID_BATCHSELECTION_DELCOLSTART ) )
	{
		iColumnID = ( int )m_mapBSOutputContextParams[m_iOutputID].m_mapColumnList[nID - ID_BATCHSELECTION_DELCOLSTART].m_lColumnID;
	}
	else
	{
		return;
	}

	if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnRemoveColumn ) )
	{
		m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnRemoveColumn]->OnBatchSelectionRemoveColumn( iColumnID );
	}
}

LRESULT CDlgBatchSelectionOutput::OnShowPopupMenu( WPARAM wParam, LPARAM lParam )
{
	// See remark in the 'OnOutputSpreadContainerCellRClicked' method for a complete description of the process.

	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer ||
		0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return -1;
	}

	CMFCPopupMenu *pclMFCPopupMenu = ( CMFCPopupMenu * )wParam;

	if( NULL == pclMFCPopupMenu )
	{
		return -1;
	}

	// Check first what is the concerned popup menu (can be the main, the 'Add column' or 'Remove column').
	enum { PopupMenuMain, PopupMenuAddCol, PopupMenuDelCol };
	int iPopupMenu;
	CMFCToolBarMenuButton *pclParentButton = pclMFCPopupMenu->GetParentButton();

	if( NULL == pclParentButton )
	{
		iPopupMenu = PopupMenuMain;
	}
	else
	{
		CString str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_ADDCOL );
		iPopupMenu = ( 0 == str.Compare( CString( pclParentButton->m_strText ) ) ) ? PopupMenuAddCol : PopupMenuDelCol;
	}

	// Common code for the three cases. It's to determine what to do with 'Add column' and 'Remove column' item.
	bool fAddColumnItem = false;
	bool fAtLeastOneAddColumnEnabled = false;
	bool fAtLeastOneRemoveColumnEnabled = false;
	mapColData mapColumnList;

	if( true == m_mapBSOutputContextParams[m_iOutputID].m_fCtxtMenuAddColumnEnabled )
	{
		if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnButtonClearResults ) )
		{
			m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnButtonClearResults]->OnBatchSelectionGetColumnList(
				mapColumnList );
		}

		if( mapColumnList.size() > 0 )
		{
			fAddColumnItem = true;

			for( mapColDataIter iter = mapColumnList.begin(); iter != mapColumnList.end(); iter++ )
			{
				if( -1 != iter->second.m_lColumnID )
				{
					if( false == iter->second.m_bAlreadyDisplayed && true == iter->second.m_bEnable )
					{
						fAtLeastOneAddColumnEnabled = true;
					}

					if( true == iter->second.m_bAlreadyDisplayed && true == iter->second.m_bEnable )
					{
						fAtLeastOneRemoveColumnEnabled = true;
					}
				}
			}
		}
	}

	long lNumberRows = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetRows();

	switch( iPopupMenu )
	{
		case PopupMenuMain:
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Prepare some needed variables.

			// Check if data exist and if data has been well validated.
			bool fDataExist = false;
			bool fDataValidated = false;

			if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnIsDataExist ) )
			{
				m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnIsDataExist]->OnBatchSelectionIsDataExist( fDataExist,
						fDataValidated );
			}

			// Check if result exist and if validate has been executed.
			bool fResultExist = false;
			bool fResultValidated = false;

			if( 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnIsResultExist ) )
			{
				m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnIsResultExist]->OnBatchSelectionIsResultExist( fResultExist,
						fResultValidated );
			}

			int iStartColID, iEndColID;
			long lStartRowRelative, lEndRowRelative;
			m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetBlockSelectionRelative( iStartColID, lStartRowRelative, iEndColID, lEndRowRelative );

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			// Check if there are data that can be copied in the clipboard.
			bool fCopyItem = false;

			if( true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsBlockSelectionExist() &&
				true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsDataExistInBlockSelection() )
			{
				// A block selection exist and there is data in it. Check now if user clicks in this area.
				if( true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsMousePointerInBlockSelection(
						m_mapBSOutputContextParams[m_iOutputID].m_clLastMousePointRightClicked ) )
				{
					fCopyItem = true;
				}
			}

			bool fPasteItem = false;

			if( true == m_mapBSOutputContextParams[m_iOutputID].m_fCtxtMenuPasteDataEnabled && m_mapBSOutputContextParams[m_iOutputID].m_iLastColumnIDRightClicked >= 0
				&& m_mapBSOutputContextParams[m_iOutputID].m_lLastRowRightClicked > 0 )
			{
				long lRowRelative = m_mapBSOutputContextParams[m_iOutputID].m_lLastRowRightClicked;
				m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetRelativeRow( lRowRelative );

				if( -1 != lRowRelative )
				{
					bool fCanPasteData = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsCellCanPasteData(
											 m_mapBSOutputContextParams[m_iOutputID].m_iLastColumnIDRightClicked );

					if( true == fCanPasteData )
					{
						// Verify first if there is some data in the clipboard.
						if( TRUE == OpenClipboard() )
						{
							if( TRUE == IsClipboardFormatAvailable( CF_UNICODETEXT ) )
							{
								fPasteItem = true;
							}

							CloseClipboard();
						}
					}
				}
			}

			bool fDeleteItem = false;

			// Remark: we can use 'm_fCtxtMenuPasteDateEnabled' variable because if we can't paste, we can't delete.
			if( true == m_mapBSOutputContextParams[m_iOutputID].m_fCtxtMenuPasteDataEnabled &&
				true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsBlockSelectionExist() &&
				true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsDataExistInBlockSelection() )
			{
				// A block selection exist and there is data in it. Check now if user clicks in this area.
				if( true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsMousePointerInBlockSelection(
						m_mapBSOutputContextParams[m_iOutputID].m_clLastMousePointRightClicked ) )
				{
					fDeleteItem = true;
				}
			}

			// If suggestion is not yet done, check if we can edit the cell.
			bool fEditItem = false;

			if( true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsBlockSelectionExist() )
			{
				// If suggestion is not yet done, we must verify now if cell is editable.
				if( false == fResultExist )
				{
					if( iStartColID == iEndColID && lStartRowRelative == lEndRowRelative )
					{
						m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsCellEditable( iStartColID, lStartRowRelative, fEditItem );
					}
				}
			}

			// If row are selected, we can delete them.
			bool fDelRowItem = false;

			if( false == fResultExist && true == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsRowSelectionExist() )
			{
				fDelRowItem = true;
			}

			// If row are selected, we can insert a new one.
			bool fInsRowItem = false;
			// TODO

			// If data exist, we can select all of them.
			bool fSelectAllItem = fDataExist;

			// If there is at least one row, we can clear all.
			bool fClearAllItem = false;

			if( lNumberRows > 1 )
			{
				fClearAllItem = true;
			}

			// Check if we can launch a suggestion (if not yet already done).
			bool fSuggestItem = ( true == fDataValidated && false == fResultExist );

			// Check if we can launch a validate (only if result exist).
			bool fValidateItem = ( true == fResultExist && false == fResultValidated );

			// Check if suggestion is done and if we can edit result.
			bool fEditResultItem = fResultExist;

			bool fClearResultsItem = fResultExist;

			// Check if catalog is available for the current product.
			bool fCatItem = false;

			if( iStartColID == iEndColID && lStartRowRelative == lEndRowRelative
				&& 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnIsCatalogExist ) )
			{
				m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnIsCatalogExist]->OnBatchSelectionIsCatalogExist(
					iStartColID, lStartRowRelative, fCatItem, m_mapBSOutputContextParams[m_iOutputID].m_pclProductForQRSCat );
			}

			CMFCPopupMenuBar *pclMFCPopupMenuBar = pclMFCPopupMenu->GetMenuBar();
			ASSERT( NULL != pclMFCPopupMenuBar );
			pclMFCPopupMenuBar->AdjustSizeImmediate( TRUE );

			// Load correct strings.
			CString str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_COPY );
			int iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_COPY );
			ASSERT( -1 != iIndex );
			CMFCToolBarMenuButton *pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_PASTE );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_PASTE );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_DELETE );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_DELETE );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_EDIT );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_EDIT );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_DELROW );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_DELROW );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_INSROW );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_INSROW );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_SELECTALL );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_SELECTALL );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_CLEARALL );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_CLEARALL );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_SUGGEST );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_SUGGEST );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_VALIDATE );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_VALIDATE );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_EDITRESULT );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_EDITRESULT );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_CLEARRESULTS );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_CLEARRESULTS );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_GETFULLCAT );
			iIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_GETFULLCAT );
			ASSERT( -1 != iIndex );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( iIndex );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			// To set the correct value, open the 'IDR_FLTMENU_BATCHSEL' menu in the resource view and count from 0.
			enum { AddColPos = 19, DelColPos = 20 }; // 0-indexed!
			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_ADDCOL );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( AddColPos );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			str = pMainFrame->GetSplitStringFromResID( IDS_FLTMENUBATCHSELECTION_DELCOL );
			pclMFCContextMenuItem = pclMFCPopupMenu->GetMenuItem( DelColPos );
			ASSERT( NULL != pclMFCContextMenuItem );
			pclMFCContextMenuItem->m_strText = str;

			// Check for 'Add column' and 'Remove column' must be done before all other. Because we don't have ID for these columns
			// but just position (for item that is a popup menu we don't have ID!). If we remove some item before, the position is no more
			// good.
			if( false == fAddColumnItem )
			{
				// Remark: It's intentional to remove twice the index 'AddColPos'. Because after removing the first one ('Add column' item),
				//         the position for 'Remove column' has decreased.
				pclMFCPopupMenu->RemoveItem( AddColPos );
				pclMFCPopupMenu->RemoveItem( AddColPos );
			}

			if( true == fCopyItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_COPY );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_COPY );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fPasteItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_PASTE );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_PASTE );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fDeleteItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_DELETE );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_DELETE );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fEditItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_EDIT );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_EDIT );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fDelRowItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_DELROW );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_DELROW );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fInsRowItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_INSROW );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_INSROW );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fSelectAllItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_SELECTALL );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_SELECTALL );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fClearAllItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_CLEARALL );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_CLEARALL );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			// 'Suggest' and 'Validate' item always visible.
			if( true == fSuggestItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_SUGGEST );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_BATCHSELECTION_SUGGEST );
			}

			if( true == fValidateItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_VALIDATE );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_BATCHSELECTION_VALIDATE );
			}

			if( true == fEditResultItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_EDITRESULT );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_EDITRESULT );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fClearResultsItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_CLEARRESULTS );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_CLEARRESULTS );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}

			if( true == fCatItem )
			{
				pMainFrame->EnableMenuResID( ID_BATCHSELECTION_GETFULLCAT );
			}
			else
			{
				int iItemIndex = pclMFCPopupMenuBar->CommandToIndex( ID_BATCHSELECTION_GETFULLCAT );

				if( -1 != iItemIndex )
				{
					pclMFCPopupMenu->RemoveItem( iItemIndex );
				}
			}
		}
		break;

		case PopupMenuAddCol:
		{
			// Remove the place holder.
			pclMFCPopupMenu->RemoveItem( 0 );

			int iItemID = ID_BATCHSELECTION_ADDCOLSTART;

			// 'Add all columns' for the first item.
			CMFCToolBarMenuButton clAddAllColumnSubItem( iItemID, NULL, -1, ( LPCTSTR )TASApp.LoadLocalizedString( IDS_BATCHSELECTION_ADDALLCOL ) );
			pclMFCPopupMenu->InsertItem( clAddAllColumnSubItem );

			if( true == fAtLeastOneAddColumnEnabled )
			{
				pMainFrame->EnableMenuResID( iItemID );
			}
			else
			{
				pMainFrame->DisableMenuResID( iItemID );
			}

			pclMFCPopupMenu->InsertSeparator();

			iItemID++;

			for( mapColDataIter iter = mapColumnList.begin(); iter != mapColumnList.end(); iter++ )
			{
				// If special column (title or separator).
				if( -1 == iter->second.m_lColumnID )
				{
					if( false == iter->second.m_bSeparator )
					{
						CMFCToolBarMenuButton clTitleSubItem( iItemID, NULL, -1, ( LPCTSTR )iter->second.m_strName );
						pclMFCPopupMenu->InsertItem( clTitleSubItem );
						pMainFrame->DisableMenuResID( iItemID );
					}
					else
					{
						pclMFCPopupMenu->InsertSeparator();
					}
				}
				else
				{
					CMFCToolBarMenuButton clColumnSubItem( iItemID, NULL, -1, ( LPCTSTR )iter->second.m_strName );
					pclMFCPopupMenu->InsertItem( clColumnSubItem );

					if( false == iter->second.m_bAlreadyDisplayed && true == iter->second.m_bEnable )
					{
						pMainFrame->EnableMenuResID( iItemID );
					}
					else
					{
						pMainFrame->DisableMenuResID( iItemID );
					}

					m_mapBSOutputContextParams[m_iOutputID].m_mapColumnList[iItemID - ID_BATCHSELECTION_ADDCOLSTART] = iter->second;

				}

				iItemID++;
			}
		}
		break;

		case PopupMenuDelCol:
		{
			// Remove the place holder.
			pclMFCPopupMenu->RemoveItem( 0 );

			int iItemID = ID_BATCHSELECTION_DELCOLSTART;

			// 'Remove all columns' for the first item.
			CMFCToolBarMenuButton clAddAllColumnSubItem( iItemID, NULL, -1, ( LPCTSTR )TASApp.LoadLocalizedString( IDS_BATCHSELECTION_REMOVEALLCOL ) );
			pclMFCPopupMenu->InsertItem( clAddAllColumnSubItem );

			if( true == fAtLeastOneRemoveColumnEnabled )
			{
				pMainFrame->EnableMenuResID( iItemID );
			}
			else
			{
				pMainFrame->DisableMenuResID( iItemID );
			}

			pclMFCPopupMenu->InsertSeparator();

			iItemID++;

			for( mapColDataIter iter = mapColumnList.begin(); iter != mapColumnList.end(); iter++ )
			{
				// If special column (title or separator).
				if( -1 == iter->second.m_lColumnID )
				{
					if( false == iter->second.m_bSeparator )
					{
						CMFCToolBarMenuButton clTitleSubItem( iItemID, NULL, -1, ( LPCTSTR )iter->second.m_strName );
						pclMFCPopupMenu->InsertItem( clTitleSubItem );
						pMainFrame->DisableMenuResID( iItemID );
					}
					else
					{
						pclMFCPopupMenu->InsertSeparator();
					}
				}
				else
				{
					CMFCToolBarMenuButton clColumnSubItem( iItemID, NULL, -1, ( LPCTSTR )iter->second.m_strName );
					pclMFCPopupMenu->InsertItem( clColumnSubItem );

					if( true == iter->second.m_bAlreadyDisplayed && true == iter->second.m_bEnable )
					{
						pMainFrame->EnableMenuResID( iItemID );
					}
					else
					{
						pMainFrame->DisableMenuResID( iItemID );
					}

					m_mapBSOutputContextParams[m_iOutputID].m_mapColumnList[iItemID - ID_BATCHSELECTION_DELCOLSTART] = iter->second;

				}

				iItemID++;
			}
		}
		break;
	}

	return 0;
}

void CDlgBatchSelectionOutput::OnDlgOutputAdded( int iOutputID, CDlgOutputSpreadContainer *pclDlgOutputSpreadContainer )
{
	if( NULL != pclDlgOutputSpreadContainer )
	{
		// Remarks: other notifications are already registered in the base class 'CDlgOutput'.
		pclDlgOutputSpreadContainer->RegisterNotificationHandler( this,
				CDlgOutputSpreadContainer::INotificationHandler::NH_OnCellRClicked |
				CDlgOutputSpreadContainer::INotificationHandler::NH_OnKeyboardShortcut |
				CDlgOutputSpreadContainer::INotificationHandler::NH_OnKeyboardVirtualKey );

		if( 0 == m_mapBSOutputContextParams.count( iOutputID ) )
		{
			BSOutputContextParams rOutputContextParams;
			m_mapBSOutputContextParams[iOutputID] = rOutputContextParams;
		}
	}
}

void CDlgBatchSelectionOutput::OnDlgOutputActivated( int iOutputID )
{
	m_iOutputID = -1;

	if( 0 != m_mapBSOutputContextParams.count( iOutputID ) )
	{
		m_iOutputID = iOutputID;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgBatchSelectionOutput::CMessage class.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDlgBatchSelectionOutput::CBSMessage::CBSMessage( CDlgBatchSelectionOutput *pDlgBatchSelectionOutput )
{
	m_pDlgBatchSelectionOutput = pDlgBatchSelectionOutput;
}

CDlgBatchSelectionOutput::CBSMessage &CDlgBatchSelectionOutput::CBSMessage::operator=( CDlgBatchSelectionOutput::CBSMessage &clHMessage )
{
	Copy( &clHMessage );
	return *this;
}

void CDlgBatchSelectionOutput::CBSMessage::Copy( CBSMessage *pclBSMessage )
{
	if( NULL == pclBSMessage )
	{
		return;
	}

	CMessageBase::Copy( pclBSMessage );
	m_pDlgBatchSelectionOutput = pclBSMessage->GetpDlgBatchSelectionOutput();
}

bool CDlgBatchSelectionOutput::CBSMessage::SendMessage( long lRow, bool fScrollOnTheLastLine, int iOutputID )
{
	bool fReturn = false;

	if( m_pDlgBatchSelectionOutput != NULL )
	{
		m_pDlgBatchSelectionOutput->SendMessage( this, lRow, fScrollOnTheLastLine, iOutputID );
		fReturn = true;
	}

	return fReturn;
}

void CDlgBatchSelectionOutput::_PasteData( bool fWithKeyboard )
{
	if( -1 == m_iOutputID || 0 == m_mapOutputContextParams.count( m_iOutputID ) || NULL == m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer ||
		0 == m_mapBSOutputContextParams.count( m_iOutputID ) )
	{
		return;
	}

	if( false == fWithKeyboard && ( -1 == m_mapBSOutputContextParams[m_iOutputID].m_iLastColumnIDRightClicked
									|| -1 == m_mapBSOutputContextParams[m_iOutputID]. m_lLastRowRightClicked ) )
	{
		return;
	}

	if( TRUE == OpenClipboard() )
	{
		if( TRUE == IsClipboardFormatAvailable( CF_UNICODETEXT ) )
		{
			HANDLE hglb = GetClipboardData( CF_UNICODETEXT );
			LPSTR lpstr = ( LPSTR )GlobalLock( hglb );
			CString strData = ( LPTSTR )lpstr;
			GlobalUnlock( hglb );

			int iPasteDataColID;
			long lPasteDataRow;

			if( false == fWithKeyboard )
			{
				iPasteDataColID = m_mapBSOutputContextParams[m_iOutputID].m_iLastColumnIDRightClicked;
				lPasteDataRow = m_mapBSOutputContextParams[m_iOutputID].m_lLastRowRightClicked;
				m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetRelativeRow( lPasteDataRow );
			}
			else
			{
				int iStartColID, iEndColID;
				long lStartRow, lEndRow;
				m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->GetBlockSelectionRelative( iStartColID, lStartRow, iEndColID, lEndRow );
				iPasteDataColID = iStartColID;
				lPasteDataRow = lStartRow;
			}

			bool fIsRedrawBlocked = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->IsRedrawBlocked();

			if( false == fIsRedrawBlocked )
			{
				m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->BlockRedraw();
			}

			int iError = m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->PasteData( strData, iPasteDataColID, lPasteDataRow );

			if( false == fIsRedrawBlocked )
			{
				m_mapOutputContextParams[m_iOutputID].m_pclDlgOutputSpreadContainer->ReleaseRedraw();
			}

			if( DlgOutputHelper::PasteDataError::PDE_Ok == iError
				&& 0 != m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler.count( IBSNotificationHandler::BSNH_OnPasteData ) )
			{
				m_mapBSOutputContextParams[m_iOutputID].m_mapBSNotificationHandler[IBSNotificationHandler::BSNH_OnPasteData]->OnBatchSelectionPasteData();
			}

			EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, true );
		}

		CloseClipboard();
	}
}
