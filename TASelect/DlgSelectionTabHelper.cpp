#include "stdafx.h"
#include "DlgSelectionTabHelper.h"


void CDlgSelectionTabHelper::SetFocusOnControl( int iWhere )
{
	if( NULL == m_pWndOwner || NULL == m_pWndOwner->GetWindow( GW_CHILD ) )
	{
		return;
	}

	CWnd *pControlToFocus = NULL;
	UCHAR ucWhere = iWhere & SetFocusWhere::WhereMask;
	UCHAR ucType = iWhere & SetFocusWhere::TypeMask;

	CWnd *pChild = m_pWndOwner->GetWindow( GW_CHILD );
	CWnd *pCurrent = pChild->GetWindow( ( (int)SetFocusWhere::First == ucWhere ) ? GW_HWNDFIRST : GW_HWNDLAST );

	while( NULL != pCurrent && NULL == pControlToFocus )
	{
		DWORD dwStyle = pCurrent->GetStyle();

		if( WS_TABSTOP == ( WS_TABSTOP & dwStyle ) && WS_VISIBLE == ( WS_VISIBLE & dwStyle ) && WS_DISABLED != ( WS_DISABLED & dwStyle ) )
		{
			if( SetFocusWhere::Edit == ucType )
			{
				if( NULL != dynamic_cast<CEdit*>( pCurrent ) )
				{
					pControlToFocus = pCurrent;
				}
			}
			else if( SetFocusWhere::Combo == ucType )
			{
				if( NULL != dynamic_cast<CComboBox*>( pCurrent ) )
				{
					pControlToFocus = pCurrent;
				}
			}
			else
			{
				pControlToFocus = pCurrent;
			}
		}

		if( NULL == pControlToFocus )
		{
			pCurrent = pCurrent->GetWindow( ( (int)SetFocusWhere::First == ucWhere ) ? GW_HWNDNEXT : GW_HWNDPREV );
		}
	}

	if( NULL != pControlToFocus )
	{
		pControlToFocus->SetFocus();
	}
	else
	{
		m_pWndOwner->SetFocus();
	}
}

BOOL CDlgSelectionTabHelper::OnPreTranslateMessage( MSG* pMsg )
{
	if( NULL == m_pWndOwner )
	{
		return FALSE;
	}
	
	BOOL fReturn = FALSE;

	if( TRUE == m_pWndOwner->IsWindowVisible() && pMsg->message == WM_KEYDOWN && GetKeyState( VK_TAB ) < 0 && NULL != m_pWndOwner->GetFocus() )
	{
		CWnd *pChild = m_pWndOwner->GetWindow( GW_CHILD );

		if( NULL == pChild )
		{
			return FALSE;
		}

		if( NULL == m_pWndOwner->GetFocus() )
		{
			return FALSE;
		}

		HWND hWndCurrentFocus = m_pWndOwner->GetFocus()->GetSafeHwnd();

		if( GetKeyState( VK_SHIFT ) < 0 )
		{
			HWND hWndFirst = _GetActiveControl( pChild, GW_HWNDFIRST );
			
			if( NULL == hWndFirst )
			{
				return FALSE;
			}
			
			if( hWndCurrentFocus == hWndFirst )
			{
				if( true == LeftTabKillFocus( false ) )
				{
					// It means that a control outside the current window (dialog) has taken the focus.
					// To signal to not dispatch the message.
					fReturn = TRUE;
				}
			}
			else
			{
				// TAB -> go normally to the next control.
				// Do nothing.
			}
		}
		else
		{
			HWND hWndLast = _GetActiveControl( pChild, GW_HWNDLAST );

			if( NULL == hWndLast )
			{
				return FALSE;
			}
			
			if( hWndCurrentFocus == hWndLast )
			{
				if( true == LeftTabKillFocus( true ) )
				{
					// It means that a control outside the current window (dialog) has taken the focus.
					// To signal to not dispatch the message.
					fReturn = TRUE;
				}
			}
			else
			{
				// TAB -> go normally to the next control.
				// Do nothing.
			}
 		}
	}

	return fReturn;
}

HWND CDlgSelectionTabHelper::_GetActiveControl( CWnd *pWnd, UINT uiGetWhat )
{
	HWND hWnd = NULL;
	CWnd *pCurrent = pWnd->GetWindow( uiGetWhat );

	while( NULL != pCurrent && NULL == hWnd )
	{
		DWORD dwStyle = pCurrent->GetStyle();

		if( WS_TABSTOP == ( WS_TABSTOP & dwStyle ) && WS_VISIBLE == ( WS_VISIBLE & dwStyle ) && WS_DISABLED != ( WS_DISABLED & dwStyle ) )
		{
			hWnd = pCurrent->GetSafeHwnd();
		}

		if( NULL == hWnd )
		{
			pCurrent = pCurrent->GetWindow( ( GW_HWNDLAST == uiGetWhat ) ? GW_HWNDPREV : GW_HWNDNEXT );
		}
	}

	return hWnd;
}

