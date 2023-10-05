#include "fptools.h"
#include "string.h"
#include <tchar.h>

PROPINFO fpPropDataSourcehWnd =
   {
   _T("DataSourcehWnd"),
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, 0, 0
   };


BOOL fpVB_SetDataSourcehWnd(HCTL hCtl, HCTL hCtlDataSource,
                            HWND hWndDataSource)
{
HCTL hCtlDataControl = 0;
BOOL fRet = FALSE;
FPCONTROL fpTemp = {0, hCtl, 0L, 0L};
FPCONTROL fpDataTemp = {hWndDataSource, 0, 0L, 0L};

if (IsWindow(hWndDataSource))
   {
   DATAACCESS da;
   BOOL       fIsRegistered;

   _fmemset(&da, '\0', sizeof(DATAACCESS));
   da.usVersion = VB_VERSION;
   da.hctlBound = hCtl;
   da.usDataType = DT_LONG;

   if (!(hCtlDataControl = hCtlDataSource))
      hCtlDataControl = (HCTL)fpVBGetDataSourceControl((LPFPCONTROL)&fpTemp, &fIsRegistered);

   if (hCtlDataControl)
      {
      da.hctlData = hCtlDataControl;
	  fpTemp.hCtl = hCtlDataControl;
      fpVBSendControlMsg((LPFPCONTROL)&fpTemp, VBM_DATA_TERMINATE, 0,
                       (LPARAM)(LPVOID)&da);
      }

   if (hWndDataSource)
      {
      hCtlDataControl = (HCTL)fpVBGetHwndControl((LPFPCONTROL)&fpDataTemp);
      da.hctlData = hCtlDataControl;
	  fpTemp.hCtl = hCtlDataControl;
      fpVBSendControlMsg((LPFPCONTROL)&fpTemp, VBM_DATA_INITIATE, 0,
                       (LPARAM)(LPVOID)&da);

      da.sAction = DATA_REFRESH;
	  fpTemp.hCtl = hCtl;
      fpVBSendControlMsg((LPFPCONTROL)&fpTemp, VBM_DATA_AVAILABLE, 0, (LPARAM)(LPVOID)&da);

      fRet = TRUE;
      }
   }

return (fRet);
}


long fpVB_DataSourcehWnd(LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
HWND hWndDataSource = GetProp(lpPP->hWnd, FP_DATASOURCEHWNDPROP);
FPCONTROL fpDataTemp = {hWndDataSource, 0, 0L, 0L};

if (fGetProp)
//   *(short FAR *)lpPP->lParam = hWndDataSource;		// CTF changed cast from short to HWND
   *(HWND FAR *)lpPP->lParam = hWndDataSource;

else
   {
   HCTL hCtlDataSource = 0;

   if (hWndDataSource)
      hCtlDataSource = (HCTL)fpVBGetHwndControl((LPFPCONTROL)&fpDataTemp);

   hWndDataSource = 0;

   if (fpVB_SetDataSourcehWnd(lpPP->hCtl, hCtlDataSource, (HWND)lpPP->lParam))
      hWndDataSource = (HWND)lpPP->lParam;

   SetProp(lpPP->hWnd, FP_DATASOURCEHWNDPROP, hWndDataSource);
   }

return (0);
}


void fpVB_DestroyDataSourcehWnd(HWND hWnd)
{
HWND hWndDataControl;
FPCONTROL fpTemp = {hWnd, 0, 0L, 0L};
FPCONTROL fpDataTemp = {0, 0, 0L, 0L};

if (hWndDataControl = GetProp(hWnd, FP_DATASOURCEHWNDPROP))
   {
   RemoveProp(hWnd, FP_DATASOURCEHWNDPROP);

   if (hWndDataControl && IsWindow(hWndDataControl))
      {
      DATAACCESS da;
      
      fpDataTemp.hWnd = hWndDataControl;
      _fmemset(&da, '\0', sizeof(DATAACCESS));
      da.usVersion = VB_VERSION;
      da.hctlBound = (HCTL)fpVBGetHwndControl((LPFPCONTROL)&fpTemp);
      da.usDataType = DT_LONG;
      da.hctlData = (HCTL)fpVBGetHwndControl((LPFPCONTROL)&fpDataTemp);
	
      fpTemp.hCtl = (HCTL)fpVBGetHwndControl((LPFPCONTROL)&fpDataTemp);
      fpVBSendControlMsg((LPFPCONTROL)&fpTemp, VBM_DATA_TERMINATE,
                       0, (LPARAM)(LPVOID)&da);
      }
   }
}
