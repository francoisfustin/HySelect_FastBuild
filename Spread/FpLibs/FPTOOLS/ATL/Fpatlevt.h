/****************************************************************************
* FPATLEVT.H : Implementation of "Stock Events"
*
* NOTE: A Majority of this code was taken from Microsoft's CTLEVENT.CPP
*       module.
*
* Copyright (C) 1991-1997 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
****************************************************************************/


//pragma warning(disable: 4706) // assignment within conditional

/////////////////////////////////////////////////////////////////////////////
// Stock event mask

#define STOCKEVENT_CLICK            0x00000001
#define STOCKEVENT_DBLCLICK         0x00000002
#define STOCKEVENT_KEYDOWN          0x00000004
#define STOCKEVENT_KEYPRESS         0x00000008
#define STOCKEVENT_KEYUP            0x00000010
#define STOCKEVENT_MOUSEDOWN        0x00000020
#define STOCKEVENT_MOUSEMOVE        0x00000040
#define STOCKEVENT_MOUSEUP          0x00000080
//#define STOCKEVENT_ERROR            0x00000100
//#define STOCKEVENT_READYSTATECHANGE 0x00000200

#define STOCKEVENTS_MOUSE       0x000000A3  // Click, DblClick, MouseDown, MouseUp

// Button values for mouse events
#define LEFT_BUTTON     0x01
#define RIGHT_BUTTON    0x02
#define MIDDLE_BUTTON   0x04
#define FPATL_KEYDOWN_FLAG  TRUE
#define FPATL_KEYUP_FLAG    FALSE

#define WM_QUEUE_SENTINEL   0x0379

#define FP_DECLARE_SUBCTL_STOCKEVENTS(dwEventsMask) \
static DWORD fpAtlGetStockEventMask() \
{ \
  static DWORD dwSubCtlEventsMask = dwEventsMask; \
  return dwSubCtlEventsMask; \
}

/////////////////////////////////////////////////////////////////////////////
// Stubs for functions not in Subclass
/////////////////////////////////////////////////////////////////////////////
#define FP_DECLARE_STUB_FIRE_KEYDOWN() \
void Fire_KeyDown(SHORT* pnChar, short nShiftState) { return; }

#define FP_DECLARE_STUB_FIRE_KEYUP() \
void Fire_KeyUp(SHORT* pnChar, short nShiftState) { return; } 

#define FP_DECLARE_STUB_FIRE_KEYPRESS() \
void Fire_KeyPress(SHORT* pnChar) { return; }

#define FP_DECLARE_STUB_FIRE_MOUSEDOWN() \
void Fire_MouseDown(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) { return; }

#define FP_DECLARE_STUB_FIRE_MOUSEUP() \
void Fire_MouseUp(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) { return; }

#define FP_DECLARE_STUB_FIRE_MOUSEMOVE() \
void Fire_MouseMove(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y) { return; }

#define FP_DECLARE_STUB_FIRE_CLICK() \
void Fire_Click() { return; }

#define FP_DECLARE_STUB_FIRE_DBLCLICK() \
void Fire_DblClick() { return; }


FP_DECLARE_SUBCTL_STOCKEVENTS(0)  // May be overridden by subclass


/////////////////////////////////////////////////////////////////////////////
// Window message handlers for stock events

LRESULT OnAnyKey(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL fKeyDown)
{
	HWND hWndSave = CTLHWND;
	USHORT nCharShort = (USHORT)wParam;
    if (fKeyDown == FPATL_KEYDOWN_FLAG)
	  KeyDown(&nCharShort);
    else
	  KeyUp(&nCharShort);
	if ((CTLHWND == hWndSave) && (nCharShort != 0))
	  return m_pT->DefWindowProc(nMsg, (WPARAM)nCharShort, lParam);
    else
      return 0;
}

LRESULT OnSysKeyDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  return OnAnyKey(nMsg, wParam, lParam, FPATL_KEYDOWN_FLAG);
}

LRESULT OnSysKeyUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  return OnAnyKey(nMsg, wParam, lParam, FPATL_KEYUP_FLAG);
}

LRESULT OnKeyDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  return OnAnyKey(nMsg, wParam, lParam, FPATL_KEYDOWN_FLAG);
}

LRESULT OnKeyUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  return OnAnyKey(nMsg, wParam, lParam, FPATL_KEYUP_FLAG);
}

void KeyUp(USHORT* pnChar)
{
  if (m_pT->fpAtlGetStockEventMask() & STOCKEVENT_KEYUP)
  {
	USHORT nShiftState = fpGetOleShiftState();
	m_pT->Fire_KeyUp((SHORT *)pnChar, nShiftState);

	// If handler set *pnChar to zero, cancel further processing.
	if (*pnChar != 0)
	  m_pT->fpAtlOnKeyUpEvent(*pnChar, nShiftState);
  }
}

void KeyDown(USHORT* pnChar)
{
  if (m_pT->fpAtlGetStockEventMask() & STOCKEVENT_KEYDOWN)
  {
	USHORT nShiftState = fpGetOleShiftState();
	m_pT->Fire_KeyDown((SHORT *)pnChar, nShiftState);

	// If handler set *pnChar to zero, cancel further processing.
	if (*pnChar != 0)
		m_pT->fpAtlOnKeyDownEvent(*pnChar, nShiftState);
  }
}

static void PostTrailByte(HWND hWnd, BYTE bTrailByte)
{
	// Force new trail byte to the front of the queue.
	::PostMessage(hWnd, WM_QUEUE_SENTINEL, 0, 0);
	::PostMessage(hWnd, WM_CHAR, bTrailByte, 0);
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_REMOVE) &&
		(msg.message != WM_QUEUE_SENTINEL))
	{
		::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}

	_ASSERT(msg.message == WM_QUEUE_SENTINEL);
	_ASSERT(msg.hwnd == hWnd);
}

LRESULT OnGetDlgCode(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// If we're firing KeyPress, prevent the container from stealing WM_CHAR.
	return m_pT->DefWindowProc(nMsg, wParam, lParam) |
		((m_pT->fpAtlGetStockEventMask() & STOCKEVENT_KEYPRESS) ? DLGC_WANTCHARS : 0);
}

LRESULT OnChar(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nChar = (UINT)wParam;
    UINT nRepCnt = (UINT)LOWORD(lParam);
    UINT nFlags  = (UINT)HIWORD(lParam);
	USHORT nCharShort = (USHORT)nChar;
	USHORT nCharSave = nCharShort;
	BOOL bLeadByte = IsDBCSLeadByte((BYTE)nCharShort);
	MSG msg;
    LRESULT lResult = 0;

	if (m_pT->fpAtlGetStockEventMask() & STOCKEVENT_KEYPRESS)
	{
		if (bLeadByte)
		{
			// We have the lead-byte of a DBCS character.  Peek for the
			// next WM_CHAR message, which will contain the other byte.

			BOOL bMessage;
			bMessage = ::PeekMessage(&msg, CTLHWND, WM_CHAR, WM_CHAR,
				PM_NOYIELD | PM_NOREMOVE);

			// Combine the bytes to form the DBCS character.

			if (bMessage)
				nCharShort = (USHORT)((nCharShort << 8) | msg.wParam);
		}

		HWND hWndSave = CTLHWND;
		nCharSave = nCharShort;
		m_pT->Fire_KeyPress((SHORT *)&nCharShort);

		// If handler set nCharShort to zero, cancel further processing.
		if (nCharShort != 0)
			fpAtlOnKeyPressEvent(nCharShort);

		if (CTLHWND != hWndSave)
			return 0;
	}

	if (nCharShort != 0)
	{
      BOOL bMsg;
		if (nCharSave != nCharShort)
		{
			nChar = nCharShort;

			// Event handler has changed the character.

			BOOL bNewLeadByte = IsDBCSLeadByte(HIBYTE(nCharShort));

			if (bLeadByte)
			{
				if (bNewLeadByte)
				{
					// Event handler changed character from DBCS to DBCS:
					// Remove the old trail byte and post the new one.
                    bMsg = ::PeekMessage(&msg, CTLHWND, WM_CHAR, WM_CHAR,
						PM_NOYIELD | PM_REMOVE);
                    if (bMsg)
                    {
					  PostTrailByte(CTLHWND, LOBYTE(nCharShort));
					  nChar = HIBYTE(nCharShort);
                    }
				}
				else
				{
					// Event handler changed character from DBCS to SBCS:
					// Remove the second byte from the queue, and forward
					// along the new single-byte character.

					bMsg = ::PeekMessage(&msg, CTLHWND, WM_CHAR, WM_CHAR,
						PM_NOYIELD | PM_REMOVE);
				}
			}
			else
			{
				if (bNewLeadByte)
				{
					// Event handler changed character from SBCS to DBCS:
					// Post the new trail byte.

					PostTrailByte(CTLHWND, LOBYTE(nCharShort));
					nChar = HIBYTE(nCharShort);
				}
			}
		}

		lResult = m_pT->DefWindowProc(WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
	}

	if (bLeadByte)
	{
		// Cleanup after processing a DBCS character:
		// Remove the next WM_CHAR message (containing the second byte) from
		// the message queue, UNLESS we're subclassing an Edit, ListBox, or
		// ComboBox control.
/*SCP
		TCHAR szClassName[10];
		if ((!::GetClassName(m_hWnd, szClassName, 10)) ||  // didn't get class
			(_tcsicmp(szClassName, _T("Edit")) &&           // not Edit
			 _tcsicmp(szClassName, _T("ListBox")) &&        // not ListBox
			 _tcsicmp(szClassName, _T("ComboBox"))))        // not ComboBox
		{
			VERIFY(::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
				PM_NOYIELD | PM_REMOVE));
		}
*/
	}
    return lResult;
}

void fpAtlOnKeyPressEvent(USHORT nChar)
{
	// Can be overridden by subclass
}

void fpAtlOnKeyDownEvent(USHORT nChar, USHORT nShiftState)
{
	// Can be overridden by subclass
}

void fpAtlOnKeyUpEvent(USHORT nChar, USHORT nShiftState)
{
	// Can be overridden by subclass
}

LRESULT OnMouseActivate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
//----------------------------------------
//  This is what COleControl had:
//  if (m_bInPlaceActive && !m_bUIActive)
//      m_bPendingUIActivation = TRUE;
//----------------------------------------
	if (!m_nFreezeEvents)
      m_bPendingUIActivation = TRUE;

	return 0;
}


void ButtonDown(USHORT iButton, UINT nFlags, POINT point)
{
	DWORD dwStockEventMask = m_pT->fpAtlGetStockEventMask();
	if ((dwStockEventMask & STOCKEVENTS_MOUSE) || m_bPendingUIActivation)
	{
		if (m_iButtonState == 0)
		{
			ATLTRACE(_T("Captured ButtonDown\n"));
			SetCapture();
		}

		m_iButtonState |= iButton;

		if (dwStockEventMask & STOCKEVENT_MOUSEDOWN)
			m_pT->Fire_MouseDown(iButton, fpGetOleShiftState(), point.x, point.y);

		m_iDblClkState &= ~iButton;
	}
}

void ButtonUp(USHORT iButton, UINT nFlags, POINT point)
{
	if (m_iButtonState != 0)
	{
		m_iButtonState &= ~iButton;

		if (m_iButtonState == 0)
		{
			ReleaseCapture();
			ATLTRACE(_T("Capture released ButtonUp\n"));
		}

		DWORD dwStockEventMask = m_pT->fpAtlGetStockEventMask();

		if (dwStockEventMask & STOCKEVENT_MOUSEUP)
			m_pT->Fire_MouseUp(iButton, fpGetOleShiftState(), point.x, point.y);

		if ((dwStockEventMask & STOCKEVENT_CLICK) &&
			!(m_iDblClkState & iButton))
		{
			RECT rect;
			::GetClientRect(CTLHWND, &rect);
			if (PtInRect(&rect, point))
				OnClick(iButton);
		}

		m_iDblClkState &= ~iButton;
	}
}

void ButtonDblClk(USHORT iButton, UINT nFlags, POINT point)
{
	DWORD dwStockEventMask = m_pT->fpAtlGetStockEventMask();
	if (dwStockEventMask & STOCKEVENTS_MOUSE)
	{
		SetCapture();
			ATLTRACE(_T("Captured ButtonDblClk \n"));
		m_iButtonState |= iButton;

		if (dwStockEventMask & STOCKEVENT_DBLCLICK)
		{
			m_pT->Fire_DblClick();
			m_iDblClkState |= iButton;
		}
	}
}

void OnClick(USHORT iButton)
{
	// May be overridden by subclass

	if (m_pT->fpAtlGetStockEventMask() & STOCKEVENT_CLICK)
		m_pT->Fire_Click();
}

LRESULT OnMouseMove(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pT->fpAtlGetStockEventMask() & STOCKEVENT_MOUSEMOVE)
	{
		HWND hWndSave = CTLHWND;
        POINT point = {(int)LOWORD(lParam), (int)HIWORD(lParam)};
		m_pT->Fire_MouseMove((short)m_iButtonState, fpGetOleShiftState(), point.x, point.y);
		if (CTLHWND != hWndSave)
			return 0;
	}
    bHandled = FALSE;
    return 0;
}

LRESULT OnLButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	/*
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;
// fix for #8964 -scl
    LRESULT lResult = m_pT->DefWindowProc(nMsg, wParam, lParam);

	SetFocus();
	ButtonDown(LEFT_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
// fix for #8964 -scl
//		return 0;
		return lResult;

// fix for #8964 -scl
//	bHandled = FALSE;  // call default
//		return 0;
		return lResult;
*/		
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;



	if( GetFocus() != CTLHWND )
		SetFocus();

	ButtonDown(LEFT_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return 0;



	bHandled = FALSE;  // call default
    return 0;
}

LRESULT OnLButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;
    LRESULT lResult;

    lResult = m_pT->DefWindowProc(nMsg, wParam, lParam);
	ButtonUp(LEFT_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return lResult;

	if (m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation &&
//rap	if (!m_nFreezeEvents && m_bPendingUIActivation &&
		(GetFocus() == CTLHWND))
	{
		m_bPendingUIActivation = FALSE;
        m_pT->InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
	}

    return lResult;
}

LRESULT OnLButtonDblClk(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;

	ButtonDblClk(LEFT_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return 0;

	bHandled = FALSE;
    return 0;
}

LRESULT OnMButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;

	ButtonDown(MIDDLE_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return 0;

	bHandled = FALSE;
    return 0;
}

LRESULT OnMButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;
    LRESULT lResult;

    lResult = m_pT->DefWindowProc(nMsg, wParam, lParam);
	ButtonUp(MIDDLE_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return lResult;

	if (m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation &&
//rap	if (!m_nFreezeEvents && m_bPendingUIActivation &&
		(GetFocus() == CTLHWND))
	{
		m_bPendingUIActivation = FALSE;
        m_pT->InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
	}
	return lResult;
}

LRESULT OnMButtonDblClk(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;

	ButtonDblClk(MIDDLE_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return 0;

    bHandled = FALSE;
	return 0;
}

LRESULT OnRButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;

	ButtonDown(RIGHT_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return 0;

    bHandled = FALSE;
	return 0;
}

LRESULT OnRButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;
    LRESULT lResult;

    lResult = m_pT->DefWindowProc(nMsg, wParam, lParam);
	ButtonUp(RIGHT_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return lResult;

	if (m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation &&
//rap	if (!m_nFreezeEvents && m_bPendingUIActivation &&
		(GetFocus() == CTLHWND))
	{
		m_bPendingUIActivation = FALSE;
        m_pT->InPlaceActivate(OLEIVERB_INPLACEACTIVATE, NULL);
	}
    return lResult;
}

LRESULT OnRButtonDblClk(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nFlags = (UINT)wParam;
    POINT point = {LOWORD(lParam), HIWORD(lParam)};
	HWND hWndSave = CTLHWND;

	ButtonDblClk(RIGHT_BUTTON, nFlags, point);
	if (CTLHWND != hWndSave)
		return 0;

    bHandled = FALSE;
	return 0;
}

LRESULT OnCancelMode(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lResult = m_pT->DefWindowProc(nMsg, wParam, lParam);

	if ((m_iButtonState != 0) || (m_iDblClkState != 0))
	{
		ReleaseCapture();
		m_iButtonState = 0;
		m_iDblClkState = 0;
	}

    return lResult;
}



/*
/////////////////////////////////////////////////////////////////////////////
// Error event

#define ERROR_PARAMS \
	 EVENT_PARAM(VTS_I2 VTS_PBSTR VTS_SCODE VTS_BSTR VTS_BSTR VTS_I4 VTS_PBOOL)

void FireError(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID)
{
	USES_CONVERSION;

	ExternalAddRef();   // "Insurance" addref -- keeps control alive.

	BSTR bstrDescription = ::SysAllocString(T2COLE(lpszDescription));
	LPCTSTR lpszSource = AfxGetAppName();
	LPCTSTR lpszHelpFile = _T("");

	if (nHelpID != 0)
		lpszHelpFile = AfxGetApp()->m_pszHelpFilePath;

	if (lpszHelpFile == NULL)
		lpszHelpFile = _T("");

	BOOL bCancelDisplay = FALSE;

	FireEvent(DISPID_ERROREVENT, ERROR_PARAMS, (WORD)SCODE_CODE(scode),
		&bstrDescription, scode, lpszSource, lpszHelpFile, (DWORD)nHelpID,
		&bCancelDisplay);

	if (!bCancelDisplay)
		DisplayError(scode, OLE2CT(bstrDescription), lpszSource, lpszHelpFile, nHelpID);

	::SysFreeString(bstrDescription);

	ExternalRelease();
}

void DisplayError(SCODE scode, LPCTSTR lpszDescription,
	LPCTSTR lpszSource, LPCTSTR lpszHelpFile, UINT nHelpID)
{
	// May be overridden by subclass.

	MessageBox(lpszDescription, lpszSource);
}
*/
