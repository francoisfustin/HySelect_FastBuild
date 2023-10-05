#include "fptools.h"
#include <tchar.h>

long FPLIB vbProcessTextExt
   (LPVBPROPPARAMS lpPP, BOOL fGetProp, WORD MsgGet, WORD MsgGetLen, WORD MsgSet, BOOL fLenMsg)
{
GLOBALHANDLE hText;
LPTSTR       lpszText;
long         lLen = 0L;
BOOL         fRet = FALSE;
FPCONTROL    fpTemp = {lpPP->hWnd, lpPP->hCtl, 0L, 0L};
if (fGetProp)
   {              
   if (fLenMsg)
      {
      if (!MsgGetLen)
         lLen = 1;
      }
   else
      lLen = MsgGetLen;
      
   if (lLen || (lLen = SendMessage(lpPP->hWnd, MsgGetLen, 0, 0L)))
      {
      if (hText = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (lLen + 1)*sizeof(TCHAR)))
         {
         lpszText = (LPTSTR)GlobalLock(hText);
         SendMessage(lpPP->hWnd, MsgGet, (WORD)lLen + 1, (long)lpszText);
         *(HSZ FAR *)lpPP->lParam = fpVBCreateHsz((LPFPCONTROL)&fpTemp, lpszText);
         GlobalUnlock(hText);
         GlobalFree(hText);
         fRet = TRUE;
         }
      }

   if (!fRet)
     *(HSZ FAR *)lpPP->lParam = fpVBCreateHsz((LPFPCONTROL)&fpTemp, _T(""));
   }

else if (MsgSet)
   {
   SendMessage(lpPP->hWnd, MsgSet, 0, lpPP->lParam);
   return (0);
   }

return (0);
}

COLORREF fpGetVBColor(LONG color)
{
    if (HIBYTE(HIWORD(color)))
        return GetSysColor(LOWORD(color));
    else
        return color;
};

