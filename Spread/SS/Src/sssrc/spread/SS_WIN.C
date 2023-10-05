/*********************************************************
* SS_WIN.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#define OEMRESOURCE

#include <windows.h>
#include <windowsx.h>
#include <fphdc.h>
#include "spread.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The SS_InvalidateRect() function adds a rectangle to the
//  spread sheet's update region.  The function is similiar to
//  InvalidateRect(), but can be used when the spread sheet has no
//  associated HWND.
//
//  Note: OCX controls may or may not have an associated HWND.
//

void SS_InvalidateRect(LPSS_BOOK lpBook, const RECT FAR* lprc, BOOL fErase)
{
if (lpBook->hWnd)
   InvalidateRect(lpBook->hWnd, lprc, fErase);
else
   lpBook->fInvalidated = TRUE;
}


//--------------------------------------------------------------------
//
//  The SS_UpdateWindow() function updates the client area of the
//  spread sheet.  The function is similiar to UpdateWindow(), but
//  can be used when the spread sheet has no associated HWND.
//
//  Note: OCX controls may or may not have an associated HWND.
//

void SS_UpdateWindow(LPSS_BOOK lpBook)
{
if (lpBook->hWnd)
   UpdateWindow(lpBook->hWnd);
}


//--------------------------------------------------------------------
//
//  The SS_GetWinClientRect() function retrieves the coordinates of
//  a spread sheet's client area.  The function is similiar to
//  GetClientRect(), but can be used when the spread sheet has no
//  associated HWND.
//
//  Note: OCX controls may or may not have an associated HWND.
//

void SS_GetWinClientRect(LPSS_BOOK lpBook, LPRECT lpRect)
{
if (lpBook->hWnd)
   GetClientRect(lpBook->hWnd, lpRect);
}


//--------------------------------------------------------------------
//
//  The SS_GetWindowRect() function retrieves the coordinates of
//  a spread sheet's control area.  The function is similiar to
//  GetWindowRect(), but can be used when the spread sheet has no
//  associated HWND.
//
//  Note: OCX controls may or may not have an associated HWND.
//

void SS_GetWindowRect(LPSS_BOOK lpBook, LPRECT lpRect)
{
if (lpBook->hWnd)
   GetWindowRect(lpBook->hWnd, lpRect);
}


//--------------------------------------------------------------------
//
//  The SS_GetDC() function retrieves a handle of a display device
//  context for the client area of the specifed spread sheet.  The
//  function is similiar to GetDC(), but can be used when the spread
//  sheet has no associated HWND.
//
//  Note: OCX controls may or may not have an associated HWND.
//

HDC SS_GetDC(LPSS_BOOK lpBook)
{
HDC hRet = 0;

if (lpBook->hWnd)
   hRet = fpGetDC(lpBook->hWnd);
else
   hRet = fpGetDC(GetDesktopWindow());
return hRet;
}


//--------------------------------------------------------------------
//
//  The SS_ReleaseDC() function retrieves a handle of a display
//  device context for the client area of the specifed spread
//  sheet.  The function is similiar to ReleaseDC(), but can be
//  used when the spread sheet has no associated HWND.
//
//  Note: OCX controls may or may not have an associated HWND.
//

int SS_ReleaseDC(LPSS_BOOK lpBook, HDC hDC)
{
int iRet = 0;

if (lpBook->hWnd)
   iRet = ReleaseDC(lpBook->hWnd, hDC);
else
   iRet = ReleaseDC(GetDesktopWindow(), hDC);
return iRet;
}


//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
