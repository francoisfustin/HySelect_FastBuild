#include "stdafx.h"
#include <algorithm>
#include <map>
#include <vector>
#include "MessageManager.h"
#include "Global.h"

CMessageManager::CMessageManager( ClassID eClassID )
{
	m_eClassID = eClassID;
	m_lFirstFlag = SSheetNHFlags::SSheetNHF_First;
	m_lLastFlag = SSheetNHFlags::SSheetNHF_Last;
	m_mapMessageCreators[ClassID::CID_CSSheet][SSheetMST_MouseEvent] = CSSheetMEMsg::NewMessage;
	m_mapMessageCreators[ClassID::CID_CSSheet][SSheetMST_KeyboardEvent] = CSSheetKEMsg::NewMessage;
	m_mapMessageCreators[ClassID::CID_CSSheet][SSheetMST_ColumnWidthChanged] = CSSheetCWCMsg::NewMessage;
	m_mapMessageCreators[ClassID::CID_CSSheet][SSheetMST_SizeChanged] = CSSheetSCMsg::NewMessage;
	m_mapMessageCreators[ClassID::CID_CSSheet][SSheetMST_NavigateOccurs] = CSSheetNOMsg::NewMessage;
}

CMessageManager::~CMessageManager()
{

}

void CMessageManager::MM_RegisterNotificationHandler( CWnd* pWnd, long lNotificationHandlerFlags )
{
	for( long lLoop = m_lFirstFlag; lLoop <= m_lLastFlag; lLoop <<= 1 )
	{
		if( lLoop == ( lNotificationHandlerFlags & lLoop ) )
		{
			if( 0 == m_mapMessageManagerNHList.count( lLoop ) )
			{
				m_mapMessageManagerNHList[lLoop].push_back( pWnd );
			}
			else
			{
				if( m_mapMessageManagerNHList[lLoop].end() == std::find( m_mapMessageManagerNHList[lLoop].begin(), m_mapMessageManagerNHList[lLoop].end(), pWnd ) )
					m_mapMessageManagerNHList[lLoop].push_back( pWnd );
			}
		}
	}
}

void CMessageManager::MM_UnregisterNotificationHandler( CWnd* pWnd, long lNotificationHandlerFlags )
{
	for( long lLoop = m_lFirstFlag; lLoop <= m_lLastFlag; lLoop <<= 1 )
	{
		if( lLoop == ( lNotificationHandlerFlags & lLoop ) )
		{
			if( 0 != m_mapMessageManagerNHList.count( lLoop ) )
			{
				std::vector<CWnd*>::iterator iterFind;
				iterFind = std::find( m_mapMessageManagerNHList[lLoop].begin(), m_mapMessageManagerNHList[lLoop].end(), pWnd );
				if( iterFind != m_mapMessageManagerNHList[lLoop].end() )
					m_mapMessageManagerNHList[lLoop].erase( iterFind );
			}
		}
	}
}

void CMessageManager::MM_UnregisterAllNotificationHandlers( CWnd* pWnd )
{
	std::map<long, std::vector<CWnd*>>::iterator mapIter;
	for( mapIter = m_mapMessageManagerNHList.begin(); mapIter != m_mapMessageManagerNHList.end(); mapIter++ )
	{
		std::vector<CWnd*>::iterator vecIter;
		vecIter = std::find( mapIter->second.begin(), mapIter->second.end(), pWnd );
		if( vecIter != mapIter->second.end() )
		{
			std::vector<CWnd*>::iterator vecIterNext = vecIter;
			vecIterNext++;
			mapIter->second.erase( vecIter );
			vecIter = vecIterNext;
		}
	}
}

bool CMessageManager::MM_IsNotificationHandlerRegistered( long lNotificationHandlerFlags )
{
	bool fReturn = false;
	for( long lLoop = m_lFirstFlag; lLoop <= m_lLastFlag && false == fReturn; lLoop <<= 1 )
	{
		if( lLoop == ( lNotificationHandlerFlags & lLoop ) )
		{
			if( 0 != m_mapMessageManagerNHList.count( lLoop ) )
				fReturn = true;
		}
	}
	return fReturn;
}

bool CMessageManager::MM_PostMessage( long lNotification, CMessageManager::CMessage* pclMessage )
{
	if( NULL == pclMessage || 0 == m_mapMessageManagerNHList.count( lNotification ) )
		return false;

	bool fReturn = true;
	HGLOBAL hMessage = GlobalAlloc( GMEM_FIXED, sizeof( short ) + pclMessage->GetSizeNeeded() );
	LPBYTE pbBuffer = (LPBYTE)GlobalLock( hMessage );
	if( NULL != pbBuffer )
	{
		short nCount = (short)m_mapMessageManagerNHList[lNotification].size();
		LPBYTE pbMessageBuffer = _SetCounter( pbBuffer, nCount );
		pclMessage->WriteBuffer( pbMessageBuffer );

		std::vector<CWnd*>::iterator vecIter;
		for( vecIter = m_mapMessageManagerNHList[lNotification].begin(); vecIter != m_mapMessageManagerNHList[lNotification].end(); vecIter++ )
		{
			if( FALSE == ::PostMessage( (*vecIter)->GetSafeHwnd(), WM_USER_MESSAGEDATA, (WPARAM)hMessage, (LPARAM)0 ) )
				_SetCounter( pbBuffer, --nCount );
		}

		GlobalUnlock( hMessage );
		if( 0 == nCount )
		{
			GlobalFree( hMessage );
			fReturn = false;
		}
	}
	return fReturn;
}

bool CMessageManager::MM_PM_SSheet_MouseMove( HWND hSSheet, long lColumn, long lRow, CPoint point )
{
	CMessageManager::CSSheetMEMsg clMessage;
	SSheetMEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eMouseEventType = SSheetMET::MET_MouseMove;
	prParams->m_lColumn = lColumn;
	prParams->m_lRow = lRow;
	prParams->m_ptMouse = point;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_MouseMove, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_MouseLButtonDown( HWND hSSheet, long lColumn, long lRow, CPoint point )
{
	CMessageManager::CSSheetMEMsg clMessage;
	SSheetMEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eMouseEventType = SSheetMET::MET_LButtonDown;
	prParams->m_lColumn = lColumn;
	prParams->m_lRow = lRow;
	prParams->m_ptMouse = point;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_MouseLButtonDown, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_MouseLButtonDblClk( HWND hSSheet, long lColumn, long lRow, CPoint point )
{
	CMessageManager::CSSheetMEMsg clMessage;
	SSheetMEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eMouseEventType = SSheetMET::MET_LButtonDblClk;
	prParams->m_lColumn = lColumn;
	prParams->m_lRow = lRow;
	prParams->m_ptMouse = point;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_MouseLButtonDblClk, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_MouseLButtonUp( HWND hSSheet, long lColumn, long lRow, CPoint point )
{
	CMessageManager::CSSheetMEMsg clMessage;
	SSheetMEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eMouseEventType = SSheetMET::MET_LButtonUp;
	prParams->m_lColumn = lColumn;
	prParams->m_lRow = lRow;
	prParams->m_ptMouse = point;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_MouseLButtonUp, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_MouseRButtonDown( HWND hSSheet, long lColumn, long lRow, CPoint point )
{
	CMessageManager::CSSheetMEMsg clMessage;
	SSheetMEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eMouseEventType = SSheetMET::MET_RButtonDown;
	prParams->m_lColumn = lColumn;
	prParams->m_lRow = lRow;
	prParams->m_ptMouse = point;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_MouseRButtonDown, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_MouseWheel( HWND hSSheet, int iDelta )
{
	CMessageManager::CSSheetMEMsg clMessage;
	SSheetMEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eMouseEventType = SSheetMET::MET_MouseWheel;
	prParams->m_iDelta = iDelta;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_MouseWheel, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_MouseCaptureChanged( HWND hSSheet, HWND hWnd )
{
	CMessageManager::CSSheetMEMsg clMessage;
	SSheetMEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eMouseEventType = SSheetMET::MET_MouseCaptured;
	prParams->m_hWnd = hWnd;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_MouseCaptureChanged, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_KeyboardShortcut( HWND hSSheet, int iKeyCode )
{
	CMessageManager::CSSheetKEMsg clMessage;
	SSheetKEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eKeyboardEventType = SSheetKET::KET_Shortcut;
	prParams->m_iKeyCode = iKeyCode;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_KeyboardShortcut, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_KeyboardVirtualKeyDown( HWND hSSheet, int iKeyCode )
{
	CMessageManager::CSSheetKEMsg clMessage;
	SSheetKEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eKeyboardEventType = SSheetKET::KET_VirtualKeyDown;
	prParams->m_iKeyCode = iKeyCode;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyDown, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_KeyboardVirtualKeyUp( HWND hSSheet, int iKeyCode )
{
	CMessageManager::CSSheetKEMsg clMessage;
	SSheetKEParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_eKeyboardEventType = SSheetKET::KET_VirtualKeyUp;
	prParams->m_iKeyCode = iKeyCode;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyUp, &clMessage );
}
bool CMessageManager::MM_PM_SSheet_ColWidthChanged( HWND hSSheet, long lColumn )
{
	CMessageManager::CSSheetCWCMsg clMessage;
	SSheetCWCParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_lColumn = lColumn;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_ColWidthChanged, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_SizeChanged( HWND hSSheet, UINT uiType, int iCx, int iCy )
{
	CMessageManager::CSSheetSCMsg clMessage;
	SSheetSCParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_nType = uiType;
	prParams->m_cx = iCx;
	prParams->m_cy = iCy;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_SizeChanged, &clMessage );
}

bool CMessageManager::MM_PM_SSheet_NavigationOccurs( HWND hSSheet, long lOldCol, long lOldRow, long lNewCol, long lNewRow )
{
	CMessageManager::CSSheetNOMsg clMessage;
	SSheetNOParams* prParams = clMessage.GetParams();
	prParams->m_hSheet = hSSheet;
	prParams->m_lOldCol = lOldCol;
	prParams->m_lOldRow = lOldRow;
	prParams->m_lNewCol = lNewCol;
	prParams->m_lNewRow = lNewRow;
	return MM_PostMessage( SSheetNHFlags::SSheetNHF_NavigateOccurs, &clMessage );
}

CMessageManager::CMessage* CMessageManager::MM_ReadMessage( WPARAM wParam )
{
	if( (WPARAM)0 == wParam )
		return NULL;

	HGLOBAL hMessage = (HGLOBAL)wParam;
	LPBYTE pbBuffer = (LPBYTE)GlobalLock( hMessage );
	if( NULL == pbBuffer )
		return NULL;

	short nCounter;
	pbBuffer = _GetCounter( pbBuffer, nCounter );

	int iClassID;
	CMessage::GetClassID( pbBuffer, &iClassID );
	int iMessageType;
	CMessage::GetMessageType( pbBuffer, &iMessageType );

	CMessage* pclMessage = NULL;
	if( 0 != m_mapMessageCreators.count( iClassID ) && 0 != m_mapMessageCreators[iClassID].count( iMessageType ) )
	{
		pclMessage = m_mapMessageCreators[iClassID][iMessageType]();
		if( NULL != pclMessage )
			pclMessage->ReadBuffer( pbBuffer );
	}
	
	nCounter--;
	if( nCounter > 0 && NULL != pclMessage )
	{
		_SetCounter( pbBuffer, nCounter );
		GlobalUnlock( hMessage );
	}
	else
	{
		GlobalUnlock( hMessage );
		GlobalFree( hMessage );
	}
	
	return pclMessage;
}

WPARAM CMessageManager::MM_CopyMessage( WPARAM wParam )
{
	if( (WPARAM)0 == wParam )
		return (WPARAM)0;

	HGLOBAL hMessageSrc = (HGLOBAL)wParam;
	LPBYTE pbBuffer = (LPBYTE)GlobalLock( hMessageSrc );
	if( NULL == pbBuffer )
		return (WPARAM)0;

	SIZE_T BufferSize = GlobalSize( hMessageSrc );
	if( BufferSize <= 0 )
		return (WPARAM)0;

	HGLOBAL hMessageDst = GlobalAlloc( GMEM_FIXED, BufferSize );
	if( NULL == hMessageDst )
		return (WPARAM)0;
	
	LPBYTE pbBufferDst = (LPBYTE)GlobalLock( hMessageDst );
	CopyMemory( pbBufferDst, pbBuffer, BufferSize );
	GlobalUnlock( hMessageDst );
	GlobalUnlock( hMessageSrc );

	return (WPARAM)hMessageDst;
}

void CMessageManager::MM_DestroyMessage( WPARAM wParam )
{
	if( (WPARAM)0 == wParam )
		return;

	HGLOBAL hMessage = (HGLOBAL)wParam;
	GlobalUnlock( hMessage );
	GlobalFree( hMessage );
}

LPBYTE CMessageManager::_SetCounter( LPBYTE pbBuffer, short& nCounter, bool fIncrementPointer )
{
	if( NULL == pbBuffer )
		return pbBuffer;
	CopyMemory( (void*)pbBuffer, (void*)&nCounter, sizeof( short ) );
	if( true == fIncrementPointer )
		pbBuffer += sizeof( short );
	return pbBuffer;
}

LPBYTE CMessageManager::_GetCounter( LPBYTE pbBuffer, short& nCounter, bool fIncrementPointer )
{
	if( NULL == pbBuffer )
		return pbBuffer;
	CopyMemory( (void*)&nCounter, (void*)pbBuffer, sizeof( short ) );
	if( true == fIncrementPointer )
		pbBuffer += sizeof( short );
	return pbBuffer;
}
