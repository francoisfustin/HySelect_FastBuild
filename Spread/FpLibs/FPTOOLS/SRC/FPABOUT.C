#include "fptools.h"   
#include "fpabout.h"   
#include "string.h"   
#include <tchar.h>
#include <stdlib.h>
#include "shellapi.h"

// CODESENT
#ifndef CS_OFF //scl
#include "..\..\..\fplibs\codesent\src\codesent.h"
#include "..\..\..\fplibs\codesent\src\csentdef.h"
#endif

#ifndef WINENTRY
#define WINENTRY FPLIB_(LONG)
#endif

#ifndef _WIN32
#define MAX_PATH 255
#endif

/*
WINENTRY fpDlgAboutProc
   (HWND hWnd, WORD Msg, WPARAM wParam, LPARAM lParam);
*/

WINENTRY fpAboutDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

// IF OCX, Only supported at Runtime
PROPINFO fpPropAbout =
   {            
   _T("About"),
#ifdef FP_VB
   DT_HSZ | PF_fGetMsg | PF_fNoRuntimeW,                //VBX
#else
   DT_HSZ | PF_fGetMsg | PF_fNoRuntimeW | PF_fNoShow,   //OCX
#endif
   0, 0, 0, NULL, 0
   };

#ifndef FP_BTNDES // for 16-bit button designer -scl
long FP_About (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
#ifdef FP_VB
   FPCONTROL fpTemp = {0, lpPP->hCtl, 0L, 0L};
   if (fGetProp)
      {
      if (fpVBGetMode((LONG)NULL) == MODE_DESIGN)
         *(HSZ FAR *)lpPP->lParam = fpVBCreateHsz((LPFPCONTROL)&fpTemp, "Click on \"...\"");
      else
         {
         char buffer[16] = { '\0' };
         LoadString(fpInstance, IDS_ABOUT_VERSION, buffer, sizeof(buffer));
         *(HSZ FAR *)lpPP->lParam = fpVBCreateHsz((LPFPCONTROL)&fpTemp, buffer);
         }
      }
#endif
   return (0);
}

short fpVersion ()
{
   TCHAR buffer[16] = { 0 };
   int  i, n;
   LoadString(fpInstance, IDS_MODEL_VERSION, buffer, STRING_SIZE(buffer));
   for (i=0,n=0; buffer[i]; i++)
      n = n * i * 10 + (buffer[i] - '0');
   return n;
}
#endif // for 16-bit button designer -scl

#ifdef NO_MFC
long fpInitAboutPopUp(LPFPCONTROL lpObject, LPPURCHASEINFO lpPurchase)
{
//   static FPABOUT about;
   fpVBDialogBoxParam((LPFPCONTROL)lpObject, fpInstance,
                      (LPSTR)MAKEINTRESOURCE(IDD_ABOUTBOX), (FARPROC)fpAboutDlgProc, 
                      (LPARAM)lpPurchase);
           
  return 0L;
}

#else
//long fpInitAboutPopUp(HWND hWnd, LPSTR lpszName, LPSTR lpszDlgRC, int idBmp)
long fpInitAboutPopUp(HWND hWnd, LPPURCHASEINFO lpPurchase)
{
//   static FPABOUT about;
   HWND hWndPopup = 0;

/*
   about.hInstance   = fpInstance;
   about.lpszCtlName = lpszName;
   about.lpszRcName  = lpszDlgRC;
   about.dlgColor    = RGBCOLOR_PALEGRAY;
   about.backColor   = RGBCOLOR_PALEGRAY;
   about.textColor   = RGBCOLOR_BLACK;
   about.bmpId       = idBmp;
*/

#ifdef FP_OCX
   {
   FPCONTROL fpTemp =  {hWnd, 0, 0L, 0L};
/*
#ifndef WIN32
   fpVBDialogBoxParam((LPFPCONTROL)&fpTemp, (HANDLE)GetWindowWord(hWnd, GWW_HINSTANCE),
                      lpszDlgRC, (FARPROC)fpDlgAboutProc, (LONG)(LPVOID)&about);
#else
   fpVBDialogBoxParam((LPFPCONTROL)&fpTemp, (HANDLE)GetWindowLong(hWnd, GWL_HINSTANCE),
                       lpszDlgRC, (FARPROC)fpDlgAboutProc, (LONG)(LPVOID)&about);
#endif
*/
   fpVBDialogBoxParam((LPFPCONTROL)&fpTemp, fpInstance,
                      (LPSTR)MAKEINTRESOURCE(IDD_ABOUTBOX), (FARPROC)fpAboutDlgProc, 
                      (LPARAM)lpPurchase);
   }
#elif defined(FP_VB)
   fpRegisterAboutProc(fpInstance);
   
   hWndPopup = CreateWindow
     (CLASS_FPABOUT, NULL, WS_CHILD, 0, 0, 0, 0, 
        hWnd, NULL, fpInstance, (LPVOID)(LPARAM)lpPurchase);

  //SetWindowLong(hWndPopup, 0, (LONG)(LPFPABOUT)&about);
#else // FP_DLL -scl
{
FARPROC ProcInst;

ProcInst = MakeProcInstance((FARPROC)fpAboutDlgProc, fpInstance);
DialogBoxParam(fpInstance, (LPSTR)MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd,
              (DLGPROC)ProcInst, (LPARAM)lpPurchase);
FreeProcInstance(ProcInst);
}
#endif

  return (long)hWndPopup;
}
#endif


WINENTRY fpAboutDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
   static HBRUSH hBrush;
   static short id = IDS_BANNER_BASE;
   TCHAR  buf[MAX_PATH];                    
   static short nCntr;
   static BOOL bFreeze = FALSE;
   static HWND hSecondDlg = 0;
   int    len = 0;  //GAB 12/21/01
#ifndef CS_OFF //scl
   static LPPURCHASEINFO lpPurchaseInfo;
#endif
                   
switch (Msg)
   {
   case WM_INITDIALOG:

      // hSecondDlg != 0, then instance of dialog is already up
      if (hSecondDlg)
         {           
         EndDialog(hDlg, TRUE);
         return (TRUE);
         }
//	use btnface instead of gray -scl
      //hBrush = CreateSolidBrush(RGBCOLOR_PALEGRAY);
	  hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
//	use btnface instead of gray -scl

      hSecondDlg = hDlg;

      fpCenterWindow(hDlg);
      
      if (LoadString(fpInstance, IDS_ABOUT_PRODUCT, buf, MAX_PATH))
         SetWindowText(hDlg, buf);
      
      if (LoadString(fpInstance, IDS_PRODUCT, buf, MAX_PATH))
         {
         SetWindowText(GetDlgItem(hDlg, IDC_PRODUCT), buf);
         SetWindowText(GetDlgItem(hDlg, IDC_MARQUEE), buf);
         }
      
      if (LoadString(fpInstance, IDS_COPYRIGHT, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_COPYRIGHT), buf);
      
      if (LoadString(fpInstance, IDS_WEBSITE, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_WEBSITE), buf);
      
      if (LoadString(fpInstance, IDS_EMAIL, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_EMAIL), buf);
      
/*  Removal of Compuserve information per G. Morris 3/3/98 -- CTF
      if (LoadString(fpInstance, IDS_COMPUSERVE, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_COMPUSERVE), buf);
*/
      SetWindowText(GetDlgItem(hDlg, IDC_COMPUSERVE), _T(""));
      
      if (LoadString(fpInstance, IDS_SALES, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_SALES), buf);
      
      if (LoadString(fpInstance, IDS_MAIN, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_MAIN), buf);
      
      if (LoadString(fpInstance, IDS_FAX, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_FAX), buf);
      
      if (LoadString(fpInstance, IDS_TECHSUPPORT, buf, MAX_PATH))
         SetWindowText(GetDlgItem(hDlg, IDC_TECHSUPPORT), buf);
// only if codesentinel is being used -scl
#ifndef CS_OFF
      if (lParam) 
         {  
         short nType, nCount;
         lpPurchaseInfo = (LPPURCHASEINFO)(LPVOID)lParam;
         if ((lpPurchaseInfo->productID) && (lpPurchaseInfo->productKey[0]))
            {
            SoftwareSecureQuery(lpPurchaseInfo->productKey, lpPurchaseInfo->productID, &nType, &nCount);
            if (nType == USAGE_TYPE_PURCHASE)
               ShowWindow(GetDlgItem(hDlg, IDC_PURCHASE_BTN), SW_HIDE);

//GAB 12/21/01 added for Unicode
//GAB 12/21/01   SetWindowText(GetDlgItem(hDlg, IDC_SERIALNUMBER), lpPurchaseInfo->serialNumber);
#ifdef _UNICODE
            len = strlen(lpPurchaseInfo->serialNumber);
            MultiByteToWideChar(CP_ACP, 0, lpPurchaseInfo->serialNumber, -1, buf, len);
            SetWindowText(GetDlgItem(hDlg, IDC_SERIALNUMBER), buf);
#else
            SetWindowText(GetDlgItem(hDlg, IDC_SERIALNUMBER), lpPurchaseInfo->serialNumber);
#endif           
            }
         else
            {
            ShowWindow(GetDlgItem(hDlg, IDC_PURCHASE_BTN), SW_HIDE);            
            }   
         }
// fix for GRB6039 -scl
#else
      ShowWindow(GetDlgItem(hDlg, IDC_PURCHASE_BTN), SW_HIDE);            
// fix for GRB6039 -scl
#endif         
// only if codesentinel is being used -scl
      SetTimer(hDlg, (UINT)hDlg, 150, 0);
      return (TRUE);

#ifndef WIN32
   case WM_CTLCOLOR:
      if (HIWORD(lParam) == CTLCOLOR_DLG)
         return (hBrush);

      else if (HIWORD(lParam) == CTLCOLOR_STATIC)
         {
//	use system colors -scl
         //SetTextColor((HDC)wParam, RGBCOLOR_BLACK);
         //SetBkColor((HDC)wParam, RGBCOLOR_PALEGRAY);
         SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
         SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
//	use system colors -scl
         return (hBrush);
         }

      return (0);
#else
   case WM_CTLCOLORDLG:
      return (long)(hBrush);

   case WM_CTLCOLORSTATIC:
//	use system colors -scl
      //SetTextColor((HDC)wParam, RGBCOLOR_BLACK);
      //SetBkColor((HDC)wParam, RGBCOLOR_PALEGRAY);
      SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
      SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
//	use system colors -scl
      return (long)(hBrush);
#endif

//	update hBrush when system colors are changed. -scl
   case WM_WININICHANGE:
   	  if (hBrush)
   	  	DeleteObject(hBrush);
   	  hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   	  InvalidateRect(hDlg, NULL, TRUE);
   	  UpdateWindow(hDlg);
   	  break;
//	update hBrush when system colors are changed. -scl
   	  
   case WM_PAINT:
      {
      HDC hdc;
      PAINTSTRUCT ps;
      HICON hIcon = NULL;
      HWND hWndCtl;
	  RECT rect;
         
      hWndCtl = GetDlgItem(hDlg, IDC_PRODUCT_ICON);

      if (LoadString(fpInstance, IDS_PRODUCT_ICON, buf, MAX_PATH))
         hIcon = LoadIcon(fpInstance, buf);

      if (hIcon)
         {
	      hdc = BeginPaint(hWndCtl, &ps); 
         DrawIcon(hdc, 0, 0, hIcon);
	      EndPaint(hWndCtl, &ps);
         DestroyIcon(hIcon);
         }

		if (LoadString(fpInstance, IDS_WEBADDR, buf, MAX_PATH))
		{
			int			nCnt = lstrlen(buf);
			LOGFONT		lf;
			HFONT		hFont, hFontUnderline, hFontOld;
			int			nBkModeOld;
			COLORREF	clrTextOld;

	        hdc = BeginPaint(hDlg, &ps); 

			hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(LOGFONT), &lf);
			lf.lfUnderline = TRUE;
//#ifdef _WIN32
			hFontUnderline = CreateFontIndirect(&lf);
			clrTextOld = SetTextColor(hdc, 0xff0000);
//#else
//			hFontUnderline = hFont;
//#endif
			hFontOld = SelectObject(hdc, hFontUnderline);
			nBkModeOld = SetBkMode(hdc, TRANSPARENT);
			
			GetWindowRect(GetDlgItem(hDlg, IDC_WEBSITE), &rect);
			MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rect, 2);
			rect.left = rect.right;
			rect.bottom = rect.top + DrawText(hdc, buf, nCnt, &rect, DT_LEFT|DT_TOP|DT_SINGLELINE|DT_CALCRECT);
			DrawText(hdc, buf, nCnt, &rect, DT_LEFT|DT_TOP|DT_SINGLELINE);

#ifdef _WIN32
			SetTextColor(hdc, clrTextOld);
#endif
			SetBkMode(hdc, nBkModeOld);
			SelectObject(hdc, hFontOld);
			DeleteObject(hFontUnderline);

	        EndPaint(hDlg, &ps);
		}
	  
      }
      break;
#ifdef _WIN32
	  case WM_SETCURSOR:
		{
			POINT	pt;
			RECT	rect;
			HDC		hdc;
			
			if (LoadString(fpInstance, IDS_WEBADDR, buf, MAX_PATH))
			{
				int	nCnt = lstrlen(buf);

				GetCursorPos(&pt);
				GetWindowRect(GetDlgItem(hDlg, IDC_WEBSITE), &rect);
				MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rect, 2);
				rect.left = rect.right;
		        hdc = GetDC(hDlg);
				rect.bottom = rect.top + DrawText(hdc, buf, nCnt, &rect, DT_LEFT|DT_TOP|DT_SINGLELINE|DT_CALCRECT);
				ReleaseDC(hDlg, hdc);

				ScreenToClient(hDlg, &pt);
				if (PtInRect(&rect, pt))
				{
					HCURSOR hCur;// = LoadCursor(NULL, IDC_HAND);

					hCur = LoadCursor(fpInstance, MAKEINTRESOURCE(IDC_FPLINK));

					SetCursor(hCur);
					return TRUE;
				}
			}
		}
		  break;

	  case WM_LBUTTONDOWN:
		  {
			POINT	pt;
			RECT	rect;
			HDC		hdc;
PROCESS_INFORMATION pi;
STARTUPINFO si;
			
			if (LoadString(fpInstance, IDS_WEBADDR, buf, MAX_PATH))
			{
				int	nCnt = lstrlen(buf);

				GetCursorPos(&pt);
				GetWindowRect(GetDlgItem(hDlg, IDC_WEBSITE), &rect);
				MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rect, 2);
				rect.left = rect.right;
		        hdc = GetDC(hDlg);
				rect.bottom = rect.top + DrawText(hdc, buf, nCnt, &rect, DT_LEFT|DT_TOP|DT_SINGLELINE|DT_CALCRECT);
				ReleaseDC(hDlg, hdc);

				ScreenToClient(hDlg, &pt);
				if (PtInRect(&rect, pt))
				{
//					MessageBox(NULL, "open browser", "SD40", MB_OK);
					HKEY hKeyHTM, hKeyCMD;
					DWORD dwType;
					DWORD dwSize = MAX_PATH;

					if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, _T(".htm"), 0, KEY_ALL_ACCESS, &hKeyHTM))
					{
//GAB 12/21/01						if (!RegQueryValueEx(hKeyHTM, NULL, 0, &dwType, buf, &dwSize))
						if (!RegQueryValueEx(hKeyHTM, NULL, 0, &dwType, (LPBYTE)buf, &dwSize))
						{
							lstrcpy(buf + lstrlen(buf), _T("\\shell\\open\\command"));
							if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, buf, 0, KEY_ALL_ACCESS, &hKeyCMD))
							{
								dwSize = MAX_PATH;
//GAB 12/21/01								if (!RegQueryValueEx(hKeyCMD, NULL, 0, &dwType, buf, &dwSize))
                memset(buf, 0, MAX_PATH);
								if (!RegQueryValueEx(hKeyCMD, NULL, 0, &dwType, (LPBYTE)buf, &dwSize))
								{
									TCHAR temp[50];
									TCHAR* tok;
									DWORD  dwLenStart = lstrlen(buf);
					
									LoadString(fpInstance, IDS_WEBADDR, temp, 50);
//GAB 12/21/01		tok = strtok(buf, _T("%"));
                  tok = _tcstok(buf, _T("%"));
									dwSize = lstrlen(buf);
									if (dwLenStart == dwSize) // IE browser, add extra space
									{
										lstrcpy(buf + dwSize, _T(" "));
										lstrcpy(buf + dwSize + 1, temp);
									}
									else	// Netscape, remove extra quote
                    lstrcpy(buf + dwSize - 1, temp);

//GAB 01/24/02 added to fix 9619. 
#ifdef WIN32
                  ZeroMemory(&si,sizeof(STARTUPINFO));
                  si.cb = sizeof(STARTUPINFO);
                  si.dwFlags = STARTF_USESHOWWINDOW;
                  si.wShowWindow =  SW_SHOWNORMAL;
                  CreateProcess(NULL, buf, NULL, NULL, TRUE, CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
#else							
									WinExec(buf, SW_SHOWNORMAL);
#endif //WIN32
                }
								RegCloseKey(hKeyCMD);
							}
							RegCloseKey(hKeyHTM);
						}
					}


				}
			}
		  }
		  break;
#endif
   case WM_DRAWITEM:
      {  
      RECT rc;
      short nOffsetAmount;
      LPDRAWITEMSTRUCT lpDraw = (LPDRAWITEMSTRUCT)lParam;
/*
      FPDRAWTEXT dt;
      
      dt.fuLineSpacing = 0;
      dt.lpLineData = NULL;
      dt.fuStyle = 0;
      dt.z3DInfo.colorShadow = RGBCOLOR_BLACK;
      dt.z3DInfo.colorHilite = RGBCOLOR_WHITE;
      dt.z3DInfo.bVOffset = (BYTE)1;
      dt.z3DInfo.bHOffset = (BYTE)1;
      dt.z3DInfo.bStyle = (BYTE)1;
      dt.fuStyle = DTX_3D;
*/
      if (lpDraw->itemAction == ODA_SELECT)
         bFreeze = (!bFreeze);
         
      CopyRect(&rc, &lpDraw->rcItem);
      FillRect(lpDraw->hDC, &rc, hBrush);
      nOffsetAmount = (short)(rc.bottom - rc.top) / 18;
      rc.top = rc.bottom - (nOffsetAmount * nCntr);
      GetWindowText(lpDraw->hwndItem, buf, MAX_PATH);
      if (bFreeze)
//	use btnhighlight instead of white -scl
         //SetTextColor(lpDraw->hDC, RGBCOLOR_WHITE);
	  {
         SetTextColor(lpDraw->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
         SetBkColor(lpDraw->hDC, GetSysColor(COLOR_HIGHLIGHT));
	  }
//	use btnhighlight instead of white -scl
      else   
	  {
//	use btntext instead of blue -scl
         //SetTextColor(lpDraw->hDC, RGBCOLOR_BLUE);
         SetTextColor(lpDraw->hDC, GetSysColor(COLOR_BTNTEXT));
         SetBkColor(lpDraw->hDC, GetSysColor(COLOR_BTNFACE));
	  }
//	use btntext instead of blue -scl
//	use btnface instead of gray -scl
      //SetBkColor(lpDraw->hDC, RGBCOLOR_PALEGRAY);
//      SetBkColor(lpDraw->hDC, GetSysColor(COLOR_BTNFACE));
//	use btnface instead of gray -scl
      DrawText(lpDraw->hDC, buf, -1, &rc, DT_CENTER | DT_WORDBREAK);
//      fpDrawText(lpDraw->hwndItem, lpDraw->hDC, buf, -1, &rc, DT_CENTER | DT_WORDBREAK, &dt);
      }
      break;
      
   case WM_TIMER:
      {
      HWND hWndCtl = GetDlgItem(hDlg, IDC_MARQUEE);
      
      if (bFreeze)
         return (0);
         
      id++;
      switch (id)
         {
         case IDS_TABPRO: 
         case IDS_SPREAD:
         case IDS_SPREADWEB:
         case IDS_SPREADWIN:
         case IDS_BMAKER:
         case IDS_LISTPRO:
         case IDS_INPUTPRO:
         case IDS_INPUTPROWIN:
         case IDS_CALOBJX:
            LoadString(fpInstance, id, buf, MAX_PATH);
            nCntr = 0;
            break;
//         case IDS_DPLANIT:
//            LoadString(fpInstance, IDS_DPLANIT, buf, MAX_PATH);
//            nCntr = 0;
//            break;
         default:
            if (id >= IDS_BANNER_MAX)
               id = IDS_BANNER_BASE;
            else
               {
               InvalidateRect(hWndCtl, NULL, FALSE);   
               nCntr++;
               }
            return (0);
            break;
         }
      
      SetWindowText(hWndCtl, buf);
      }
      return (0);

   case WM_DESTROY:
      if (hSecondDlg == hDlg)
         {
         KillTimer(hDlg, (UINT)hDlg);
         DeleteObject(hBrush);
         hBrush = NULL;
         id = IDS_BANNER_BASE;
         nCntr = 0;
         bFreeze = FALSE;
         hSecondDlg = 0;
         }
      break;

   case WM_COMMAND:
      switch (WM_CMD_ID(wParam))
         {
         case IDOK:
         case IDCANCEL:
            EndDialog(hDlg, TRUE);
            return (TRUE);
// only if codesentinel is being used -scl
#ifndef CS_OFF         
         case IDC_PURCHASE_BTN:
            {
            char buffer[64];
            _fmemset((LPVOID)buffer, '\0', sizeof(buffer));
            if (SoftwareSecureUnlock(lpPurchaseInfo->productKey,             
                                   (LPSTR)buffer, lpPurchaseInfo->productID, 
                                   (HINSTANCE)fpInstance,
                                   0, 
                                   NULL,
#ifdef WIN32
                                   TRUE) == TRUE)
#else
                                   FALSE) == TRUE)
#endif

               {
               SoftwareSecureSetSerialNumber(lpPurchaseInfo->productKey, 
                                           lpPurchaseInfo->productID, 
                                           buffer, NULL);
               ShowWindow(GetDlgItem(hDlg, IDC_PURCHASE_BTN), SW_HIDE);            

               SoftwareSecureGetSerialNumber(lpPurchaseInfo->productKey, 
                                           lpPurchaseInfo->serialNumber, 
                                           lpPurchaseInfo->productID,
#ifdef WIN32
                                           TRUE);
#else
                                           FALSE);
#endif

#ifdef _UNICODE
  // If unicode, then convert string from single-byte chars to 
  // wide-chars.
                 {
                 UINT   len  = _fstrlen(lpPurchaseInfo->serialNumber);
                 LPWSTR tstr = (LPWSTR)malloc((len+1)*sizeof(WCHAR)); // alloc buffer
                 if (tstr)  // if buffer allocated, convert string to wide-chars
                   {
                   MultiByteToWideChar(CP_ACP, 0, lpPurchaseInfo->serialNumber,
                                       -1, tstr, len); 
                   SetWindowText(GetDlgItem(hDlg, IDC_SERIALNUMBER), tstr);
                   free(tstr); // free allocated buffer
                   }
                 }
#else  // not UNICODE
               SetWindowText(GetDlgItem(hDlg, IDC_SERIALNUMBER), (LPTSTR)lpPurchaseInfo->serialNumber);
#endif // UNICODE
               }
            SoftwareSecureClose(lpPurchaseInfo->productKey);                          	
            }
            break; 
#endif // not CS_OFF

// only if codesentinel is being used -scl
         }
      break;

   }

return (FALSE);
}

/*
WINENTRY fpDlgAboutProc
   (HWND hWnd, WORD Msg, WPARAM wParam, LPARAM lParam)
{
   static LPFPABOUT lpAbout = NULL;
   static HBRUSH  hBrush;
   static HBITMAP hBitmap;
   
   switch (Msg)
      {
      case WM_INITDIALOG:
         lpAbout = (LPFPABOUT)lParam;
         fpCenterWindow(hWnd);
         hBrush  = CreateSolidBrush(lpAbout->dlgColor);
         hBitmap = LoadBitmap(lpAbout->hInstance, MAKEINTRESOURCE(lpAbout->bmpId));
         SetWindowText(GetDlgItem(hWnd, IDC_CTLNAME), lpAbout->lpszCtlName);
         return (TRUE);

      case WM_PAINT:
         {
         HDC hdcMem, hdc;
         PAINTSTRUCT ps;
         BITMAP bm;
         RECT rcCtl;
         POINT pt;
         
         hdc = BeginPaint(hWnd, &ps); 
         
         GetWindowRect (GetDlgItem(hWnd, IDC_BITMAP), &rcCtl);
         pt.x = rcCtl.left;  
         pt.y = rcCtl.top;
         ScreenToClient (hWnd, &pt);
         
         rcCtl.bottom -= rcCtl.top;
         rcCtl.right  -= rcCtl.left;
         rcCtl.top    = pt.y;
         rcCtl.left   = pt.x;
          
         hdcMem = CreateCompatibleDC (hdc);
         SelectObject (hdcMem, hBitmap);
         GetObject (hBitmap, sizeof(BITMAP), (LPBITMAP)&bm);
         
         StretchBlt (hdc, rcCtl.left, rcCtl.top, rcCtl.right, rcCtl.bottom,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); 

         DeleteObject(hdcMem);
         
         EndPaint(hWnd, &ps);
         }
         break;
      
#ifndef WIN32
      case WM_CTLCOLOR:
         if (HIWORD(lParam) == CTLCOLOR_DLG)
            return (long)(hBrush);
         else if (HIWORD(lParam) == CTLCOLOR_STATIC)
            {
            SetTextColor((HDC)wParam, lpAbout->textColor);
            SetBkColor((HDC)wParam, lpAbout->backColor);
            return (long)(hBrush);
            }
         return (0);
#else
      case WM_CTLCOLORDLG:
         return (long)(hBrush);

      case WM_CTLCOLORSTATIC:
         SetTextColor((HDC)wParam, lpAbout->textColor);
         SetBkColor((HDC)wParam, lpAbout->backColor);
         return (long)(hBrush);
#endif

      case WM_DESTROY:
         DeleteObject(hBrush);
         DeleteObject(hBitmap);
         break;

      case WM_COMMAND:
         switch (wParam)
            {
            case IDOK:
            case IDCANCEL:
               EndDialog(hWnd, 0);
             break;

            default:
               return (FALSE);
            }
         break;
   }
   return (FALSE);
}

*/

long _export FAR PASCAL fpAboutProc
   (HWND hWnd, WORD Msg, WORD wParam, LONG lParam)
{           
// only if codesentinel is being used -scl
#ifndef CS_OFF
   static PURCHASEINFO purchaseInfo;
#endif
// only if codesentinel is being used -scl
   
   switch (Msg)
      {           
// only if codesentinel is being used -scl
#ifndef CS_OFF
   case WM_CREATE:
      if (((CREATESTRUCT FAR *)lParam)->lpCreateParams)
         _fmemcpy(&purchaseInfo, ((CREATESTRUCT FAR *)lParam)->lpCreateParams, sizeof(PURCHASEINFO));
      break;
#endif   
// only if codesentinel is being used -scl
      case WM_DESTROY:
         break;
                                                                                                          
      case WM_SHOWWINDOW:
         if (wParam)
            {
            PostMessage(hWnd, FPM_SHOWWINDOW, 0, 0L);
            return 0;
            }
         break;
#if defined(FP_VB) || defined(FP_OCX)
      case FPM_SHOWWINDOW:
         {                   
         FPCONTROL fpTemp =  {hWnd, 0, 0L, 0L};
         fpVBDialogBoxParam((LPFPCONTROL)&fpTemp, fpInstance,
                      (LPSTR)MAKEINTRESOURCE(IDD_ABOUTBOX), 
                      (FARPROC)fpAboutDlgProc, 
// only if codesentinel is being used -scl
#ifndef CS_OFF
                      (LPARAM)(LPVOID)&purchaseInfo);
#else
					  (LPARAM)(LPVOID)NULL);
#endif					                       
// only if codesentinel is being used -scl
/*
         LPFPABOUT lpAbout = (LPFPABOUT)GetWindowLong (hWnd, 0);
#ifndef WIN32
         fpVBDialogBoxParam((LPFPCONTROL)&fpTemp, (HANDLE)GetWindowWord(hWnd, GWW_HINSTANCE), 
            lpAbout->lpszRcName, (FARPROC)fpDlgAboutProc, (LONG)lpAbout);
#else
         fpVBDialogBoxParam((LPFPCONTROL)&fpTemp, (HANDLE)GetWindowLong(hWnd, GWL_HINSTANCE), 
            lpAbout->lpszRcName, (FARPROC)fpDlgAboutProc, (LONG)lpAbout);
#endif
*/
         }
         break;
#endif

      }

    return DefWindowProc(hWnd, Msg, wParam, lParam);
}


void fpRegisterAboutProc (HINSTANCE hInstance)
{
   WNDCLASS wc;

   wc.style = 0;
   wc.lpfnWndProc = (WNDPROC)fpAboutProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 4;
   wc.hInstance = hInstance;
   wc.hIcon = 0;
   wc.hCursor = 0;
   wc.hbrBackground = 0;
   wc.lpszMenuName = 0;
   wc.lpszClassName = CLASS_FPABOUT;

   RegisterClass(&wc);
}

