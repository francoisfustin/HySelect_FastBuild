/*********************************************************
* VBMISC.C
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

#include <windows.h>
#include <vbapi.h>
#include <io.h>
#include "vbmisc.h"
#include "..\vbx\stringrc.h"
#ifdef MSC6
#define _access access
#endif

extern  HANDLE  hDynamicInst;

HANDLE GetSelText(hWnd)

HWND   hWnd;
{
HANDLE hData = 0;
LPSTR  Data;
long   lSel;
short  TextLen;
short  dStart;
short  dEnd;
short  i;

lSel = SendMessage(hWnd, EM_GETSEL, 0, 0L);

dStart = LOWORD(lSel);
dEnd = HIWORD(lSel);

if (dStart < dEnd)
   {
   TextLen = (short)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0L);

   if (TextLen)
      {
      if (hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, TextLen + 1))
         {
         Data = GlobalLock(hData);

         SendMessage(hWnd, WM_GETTEXT, TextLen + 1, (long)Data);

         for (i = 0; i < min(lstrlen(Data), dEnd) - dStart; i++)
            Data[i] = Data[dStart + i];

         Data[i] = '\0';
         GlobalUnlock(hData);
         }
      }
   }

return (hData);
}


#ifdef SS_V21
BOOL CheckRuntimeLicense(LPSTR lpszCaption, LPSTR lpszLicenseFileName)
{
OFSTRUCT     OFStruct;
/*
static char  Buffer[144];

GetSystemDirectory(Buffer, sizeof(Buffer));

if (Buffer[lstrlen(Buffer) - 1] != '\\')
   lstrcat(Buffer, "\\");

lstrcat(Buffer, lpszLicenseFileName);

if (_access(Buffer, 0))
*/

if (OpenFile(lpszLicenseFileName, &OFStruct, OF_EXIST) == HFILE_ERROR)
   {
	if (lpszCaption)
		DisplayRuntimeLicenseMsg(lpszCaption);
   return (FALSE);
   }

return (TRUE);
}


void DisplayRuntimeLicenseMsg(lpszCaption)

LPSTR lpszCaption;
{
char    szMsg[256];

LoadString(hDynamicInst, IDS_ERR_NOLICENSE + LANGUAGE_BASE, szMsg, sizeof(szMsg)-1);
MessageBox(NULL, szMsg, lpszCaption, MB_TASKMODAL | MB_OK | MB_ICONSTOP);
}
#endif
