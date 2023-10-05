/*    PortTool v2.2     FPTOOL06.C          */

#include <limits.h>
#include "fptools.h"
#include "zmouse.h"
#include <tchar.h>

BOOL  fMouseWheelActive = -1;
UINT  uiMsgMouseWheel = 0;
UINT  uiMouseWheelScrollLines;
BOOL  fMouseWheelNew = FALSE;
HHOOK g_hHook = 0;
short nMouseWheelHookCnt = 0;

#define PROP_FPMOUSEWHEEL _T("fpMouseWheel")

#ifndef SM_MOUSEWHEELPRESENT
#define SM_MOUSEWHEELPRESENT    75
#endif

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES 104
#endif

void fpMouseWheel_GetScrollLines(void);
LRESULT _export CALLBACK fpMouseWheel_GetMsgProc(int iCode, WPARAM wParam, LPARAM lParam);


//--------------------------------------------------------------------
//
//  Tests the equalness of two numbers with an accuracy of the magnitude
//  of the given values scaled by 2^-51 (1 bit stripped).  This is used
//  by the rounding and truncating methods to iron out small roundoff
//  errors.
//

BOOL approxEqual(double x, double y)
{
  if (x == y)
    return TRUE;
  return fabs(x - y) < fabs(x) / (16777216.0 * 16777216.0 * 8.0);
}

//--------------------------------------------------------------------
//
//  Rounds towards negative infinity taking ApproxEqual2 into account.
//

double approxFloor(double x)
{
  double r = floor(x);
  if (approxEqual(x, r + 1.0))
    return r + 1.0;
  return r;
}

//--------------------------------------------------------------------
//
//  Rounds towards positive infinity taking ApproxEqual2 into account.
//

double approxCeiling(double x)
{
  double r = ceil(x);
  if (approxEqual(x, r - 1.0))
    return r - 1.0;
  return r;
}

double pow10(int n)
{
  static double p10[] = {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16};
  if (0 <= n && n <= 16)
    return p10[n];
  return pow(10.0, (double)n);
}//--------------------------------------------------------------------
//
//  Rounds towards nearest neighbor taking ApproxEqual2 into account.
//  If both neighbors are equidistant then rounds away from zero.
//

double approxRound(double x, int digits)
{
  double power = pow10(abs(digits));
  if (digits < 0)
    x /= power;
  else
    x *= power;

  // RFW - 11/4/05 - 17533
  if (x > -100000000000000.0 && x < 100000000000000.0)
  {
    if (x < 0.0)
      x = approxCeiling(x - 0.5);
    else
      x = approxFloor(x + 0.5);
  }

  if (digits < 0)
    x *= power;
  else
    x /= power;
  return x;
}

BOOL FPLIB fpMouseWheel(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam,
                         HWND hWndVScroll, HWND hWndHScroll)
{
return (fpMouseWheelEx(hWnd, uiMsg, wParam, lParam, hWndVScroll, hWndHScroll, 0));
}


/*
iLinesToScroll - If 0, do normal processing, otherwise scroll the specified number of lines
*/
BOOL FPLIB fpMouseWheelEx(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam,
                          HWND hWndVScroll, HWND hWndHScroll, int iLinesToScroll)
{
if (fMouseWheelActive == -1)
   {
#ifdef WIN32
   OSVERSIONINFO osVersion;

   _fmemset(&osVersion, 0, sizeof(OSVERSIONINFO));
   osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&osVersion);

   if ((osVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
        (osVersion.dwMajorVersion >= 5 ||
         (osVersion.dwMajorVersion == 4 && osVersion.dwMinorVersion > 0))) ||
       (osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT &&
        osVersion.dwMajorVersion >= 4))
      fMouseWheelNew = TRUE;
   else
      // is this supposed to be an assignment? -scl
      //fMouseWheelActive == FALSE;
      fMouseWheelActive = FALSE;

#endif

   if (fMouseWheelActive == -1)
      {
#ifdef WIN32
      if (fMouseWheelNew)
         fMouseWheelActive = TRUE;
	     /* RFW - 6/26/03 - 12169
         fMouseWheelActive = (BOOL)(GetSystemMetrics(SM_MOUSEWHEELPRESENT) ? TRUE : FALSE);
		 */

      else
#endif
         {
         HWND hWndWheel;

         fMouseWheelActive = FALSE;

         hWndWheel = FindWindow(MSH_WHEELMODULE_CLASS,
                                MSH_WHEELMODULE_TITLE);

         if (hWndWheel)
            {
            UINT uiMsgWheelSupport = RegisterWindowMessage(MSH_WHEELSUPPORT);

            if (uiMsgWheelSupport)
               {
               fMouseWheelActive = TRUE;
//               fMouseWheelActive = (BOOL)SendMessage(hWndWheel,
//                                                     uiMsgWheelSupport, 0, 0);

               if (fMouseWheelActive)
                  uiMsgMouseWheel = RegisterWindowMessage(MSH_MOUSEWHEEL);
               }
            }
         }

      if (fMouseWheelActive)
         fpMouseWheel_GetScrollLines();
      }
   }

if (fMouseWheelActive)
   {
   short nWheelDelta;

   if (uiMsg == WM_MOUSEWHEEL || uiMsg == uiMsgMouseWheel)
      {
      HWND hWndScrollBar = (hWndVScroll ? hWndVScroll : hWndHScroll);
// changed to assume VSCROLL unless HSCROLL hWnd is passed in -scl
//      WORD msgScroll = (hWndVScroll ? WM_VSCROLL : WM_HSCROLL);
      WORD msgScroll = (hWndHScroll ? WM_HSCROLL : WM_VSCROLL);
      WORD wKeys = 0;

#ifdef WIN32
      if (uiMsg == WM_MOUSEWHEEL)
         {
         wKeys = LOWORD(wParam);
         nWheelDelta = -(short)HIWORD(wParam);
         }
      else
#endif
         {
         if (GetKeyState(VK_SHIFT) & 0x8000)
            wKeys |= MK_SHIFT;
         if (GetKeyState(VK_CONTROL) & 0x8000)
            wKeys |= MK_CONTROL;
         if (GetKeyState(VK_LBUTTON) & 0x8000)
            wKeys |= MK_LBUTTON;
         if (GetKeyState(VK_MBUTTON) & 0x8000)
            wKeys |= MK_MBUTTON;
         if (GetKeyState(VK_RBUTTON) & 0x8000)
            wKeys |= MK_RBUTTON;

         nWheelDelta = -(short)wParam;
         }

      // Don't support zoom and datazoom
      if (wKeys & (MK_SHIFT | MK_CONTROL))
         return (FALSE);

      if (abs(nWheelDelta) >= WHEEL_DELTA &&
          uiMouseWheelScrollLines == WHEEL_PAGESCROLL)
         {
         WORD wScrollCode;

         if (nWheelDelta < 0)
            wScrollCode = SB_PAGEUP;
         else
            wScrollCode = SB_PAGEDOWN;

#ifdef WIN32
         SendMessage(hWnd, msgScroll, MAKELONG(wScrollCode, 0), (LPARAM)hWndScrollBar);
#else
         SendMessage(hWnd, msgScroll, wScrollCode, MAKELONG(0, hWndScrollBar));
#endif
         return TRUE; // 99915276 -scl
         }
//km 23834 
      else if (/*abs(nWheelDelta) >= WHEEL_DELTA &&*/ uiMouseWheelScrollLines > 0)
         {
         short nLines;

			if (iLinesToScroll)
			{
//km 23834 
				double delta = (double)nWheelDelta / (double)WHEEL_DELTA;
				double fLines = delta * (double)abs(iLinesToScroll);
				nLines = (short)approxRound(fLines, 0);
				//nLines = abs(iLinesToScroll) * (nWheelDelta > 0 ? 1 : -1);
			}
			else
			{
//km 23834 
				double delta = (double)nWheelDelta / (double)WHEEL_DELTA;
				double fLines = delta * (double)uiMouseWheelScrollLines;
				nLines = (short)approxRound(fLines, 0);
				//nLines = uiMouseWheelScrollLines * (nWheelDelta / WHEEL_DELTA);
			}

#if 0 // RFW - 5/7/00 - KEM57
         int   iMinPos;
         int   iMaxPos;
         int   iPos;
// changed to make this code work with the fpCombo control's child edit window -scl
#ifndef WIN32
         GetScrollRange(hWndScrollBar ? hWndScrollBar : hWnd,
                        hWndScrollBar ? SB_CTL : SB_VERT, &iMinPos, &iMaxPos);
         iPos = GetScrollPos(hWndScrollBar ? hWndScrollBar : hWnd,
                             hWndScrollBar ? SB_CTL : SB_VERT);
#else
		 SendMessage(hWndScrollBar ? hWndScrollBar : hWnd, SBM_GETRANGE, (WPARAM)&iMinPos, (LPARAM)&iMaxPos);
		 iPos = (int)SendMessage(hWndScrollBar ? hWndScrollBar : hWnd, SBM_GETPOS, 0, 0);
#endif
         if (nLines < 0)
            nLines = max(nLines, iMinPos - iPos);
         else
            nLines = min(nLines, iMaxPos - iPos);

         if (nLines)
            {
            WORD wScrollCode;
            WORD wPos = 0;

            if (nLines == 1)
               wScrollCode = SB_LINEDOWN;
            else if (nLines == -1)
               wScrollCode = SB_LINEUP;
            else
               {
               wScrollCode = SB_THUMBPOSITION;
               wPos = iPos + nLines;
               }

#ifdef WIN32
            SendMessage(hWnd, msgScroll, MAKELONG(wScrollCode, wPos), (LPARAM)hWndScrollBar);
#else
            SendMessage(hWnd, msgScroll, wScrollCode, MAKELONG(wPos, hWndScrollBar));
#endif
            }
#endif // 0

         if (nLines)
            {
            WORD  wScrollCode = (nLines > 0 ? SB_LINEDOWN : SB_LINEUP);
            short nLineCnt = abs(nLines);

            for ( ; nLineCnt > 0; nLineCnt--)
               {
#ifdef WIN32
               SendMessage(hWnd, msgScroll, MAKELONG(wScrollCode, 0), (LPARAM)hWndScrollBar);
#else
               SendMessage(hWnd, msgScroll, wScrollCode, MAKELONG(0, hWndScrollBar));
#endif
               }
            return TRUE; // 99915276 -scl
            }
         }
      }

   else if (uiMsg == WM_WININICHANGE && wParam == SPI_SETWHEELSCROLLLINES &&
            fMouseWheelActive)
      fpMouseWheel_GetScrollLines();
   else if (uiMsg == WM_CREATE && !fMouseWheelNew)
      {
      if (nMouseWheelHookCnt == 0)
#ifdef WIN32
         g_hHook = SetWindowsHookEx(WH_GETMESSAGE,
                                    (HOOKPROC)fpMouseWheel_GetMsgProc,
                                    fpInstance, (DWORD)GetCurrentThreadId());
#else
         g_hHook = SetWindowsHookEx(WH_GETMESSAGE,
                                    (HOOKPROC)fpMouseWheel_GetMsgProc,
                                    fpInstance, 0);
#endif

      nMouseWheelHookCnt++;
      SetProp(hWnd, PROP_FPMOUSEWHEEL, (HANDLE)1);
      }

   else if (uiMsg == WM_DESTROY && !fMouseWheelNew)
      {
      RemoveProp(hWnd, PROP_FPMOUSEWHEEL);
      nMouseWheelHookCnt--;
      if (g_hHook && nMouseWheelHookCnt == 0)
         UnhookWindowsHookEx(g_hHook);
      }
   }

return (FALSE);
}


void fpMouseWheel_GetScrollLines(void)
{
#ifdef WIN32
if (fMouseWheelNew)
   SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uiMouseWheelScrollLines, 0);

else
#endif
   {
   UINT uiMsgScrollLines = RegisterWindowMessage(MSH_SCROLL_LINES);
   HWND hWndWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);

   if (hWndWheel && uiMsgScrollLines)
      uiMouseWheelScrollLines = (int)SendMessage(hWndWheel, uiMsgScrollLines,
                                                 0, 0);
   else
      uiMouseWheelScrollLines = 3;  // default
   }
}


LRESULT _export CALLBACK fpMouseWheel_GetMsgProc(int iCode, WPARAM wParam, LPARAM lParam)
{
LRESULT retValue = 0;
LPMSG   lpMsg = (LPMSG)lParam;

retValue = CallNextHookEx(g_hHook, iCode, wParam, lParam);

if (lpMsg->message == uiMsgMouseWheel && wParam != PM_NOREMOVE)
   {
   HWND hWndFocus = GetFocus();

   if (GetProp(hWndFocus, PROP_FPMOUSEWHEEL))
      {
      WPARAM wp = lpMsg->wParam;

#ifdef WIN32
      {
      WORD wKeys;

      if (GetKeyState(VK_SHIFT) & 0x8000)
         wKeys |= MK_SHIFT;
      if (GetKeyState(VK_CONTROL) & 0x8000)
         wKeys |= MK_CONTROL;
      if (GetKeyState(VK_LBUTTON) & 0x8000)
         wKeys |= MK_LBUTTON;
      if (GetKeyState(VK_MBUTTON) & 0x8000)
         wKeys |= MK_MBUTTON;
      if (GetKeyState(VK_RBUTTON) & 0x8000)
         wKeys |= MK_RBUTTON;

      wp = MAKELONG(wKeys, (WORD)lpMsg->wParam);
      }
#endif

      SendMessage(hWndFocus, WM_MOUSEWHEEL, wp, lpMsg->lParam);

      return (TRUE);
      }
   }

return (retValue);
}
