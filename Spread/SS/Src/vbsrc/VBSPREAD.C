/*********************************************************
* VBSPREAD.C   
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code 
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* RAP01 - 8968                                     05.23.01         
*********************************************************/

//---------*---------*---------*---------*---------*---------*---------*-----
//
// UPDATE LOG:
//
//    	RWP01 	12.09.96  	<JOK983> Font setting are getting lost
//	    RWP02	10.20.97	<JIS3735> System hangs if SD is invoked from VBX
//							while a standalone version is running
//		RWP03	10.20.97	<JIS3493> No hourglass cursor when SD is loading
//		RWP04	11.04.97	<JIS3756> Deadlock when record source not selected
//							and SD is invoked through VBX and Data Binding
//							dialog is invoked
//
//---------*---------*---------*---------*---------*---------*---------*-----


#include <windows.h>
//#include <vbapi.h>
#include <math.h>
#include <string.h>
#include "fptools.h"
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_draw.h"
#include "..\sssrc\spread\ss_emode.h"
#include "..\sssrc\spread\ss_main.h"
#include "..\sssrc\spread\ss_virt.h"
#include "vbmisc.h"
#include "vbdattim.h"
#include "vbmsg.h"
#include "vbspread.h"
#include "vbsprea4.h"
#include "db_bind.h"
#include "..\ssprvw\vbssprvw.h"
#include "stdlib.h"

// CODESENT  
#ifndef CS_OFF
#if defined(FP_VB) || defined(FP_OCX)
#include "\fplibs\codesent\src\csentdef.h"
#endif
#endif
// CODESENT  

// Beginning of Spread Designer stuff
// Spread Designer Copy Data Message...
UINT WM_COPYDATA;              
// Modal Spread Designer hook stuff
LRESULT CALLBACK SD_MouseFilterHook(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SD_KeyFilterHook(int code, WPARAM wParam, LPARAM lParam);
HHOOK hhookMouseFilter = NULL;
HHOOK hhookKeyFilter = NULL;
// End of Spread Designer stuff

typedef struct tagTBMINMAXINFO
   {
   POINT ptReserved;
   POINT ptMaxSize;
   POINT ptMaxPosition;
   POINT ptMinTrackSize;
   POINT ptMaxTrackSize;
   } TBMINMAXINFO, FAR *LPTBMINMAXINFO;

typedef struct tagLOADMODULE
   {
   WORD   wEnvSeg;
   LPSTR  lpCmdLine;
   LPVOID lpCmdShow;
   DWORD  dwReserved;
   } LOADMODULE, FAR *LPLOADMODULE;

typedef struct tagSS_BUFFER
   {
   GLOBALHANDLE g;
   LONG         l;
   }SS_BUFFER, FAR *LPSS_BUFFER;

typedef struct tagCOPYDATASTRUCT
   {
   LONG lData;
   GLOBALHANDLE ghData;
   } COPYDATASTRUCT, FAR *LPCOPYDATASTRUCT; 

typedef unsigned int size_t;
void _far * _far _cdecl _fmemcpy(void _far *, const void _far *, size_t);

LONG (*CallBack[IPROP_SPREAD_CNT])();

extern HANDLE hDynamicInst;
extern PROPINFO PropertyNULL;
extern LoadEnumStrings(HCTL hCtl);

#ifdef SS_UTP
extern char szUtpDevice[120];
extern char szUtpDriver[50];
extern char szUtpOutput[50];
#endif

void            InitFunctionArray();
BOOL            vbSpreadFireQueryAdvance(HCTL hCtl, short dNext);
HLSTR           _SpreadGetTextString(HWND hWnd, LPSS_CELLTYPE lpCellType,
                                     SS_COORD Col, SS_COORD Row);
void            _SpreadSetItemData(HCTL hCtl, long Col, long Row, long lValue);
long            _SpreadGetItemData(HCTL hCtl, long Col, long Row);
long _export FAR PASCAL vbSSPopupProc(HWND hWnd, WORD Msg, WORD wParam,
                                      LONG lParam);
void            vbSSRegisterPopupProc(void);
void            vbSpreadRunDesigner(HWND hWnd);
BOOL FAR PASCAL _export SpreadCustomFunctionProc(HWND hWnd, LPSTR lpszFunction,
                                                 short dFunctionLen,
                                                 LPSS_VALUE lpResult,
                                                 LPSS_VALUE lpValues,
                                                 short dParamCnt);
LONG FAR PASCAL _export vbSpreadCallBackProc(HWND hWnd, WORD Msg,
                                             WPARAM wParam, LPARAM lParam);
#ifdef SS_UTP
void                    vbSpreadSetScrollArrows(HWND hWnd, LPSPREADSHEET lpSS);
#endif
void                    VBSS_About(HWND hWndSS);
BOOL FAR PASCAL _export VBSS_DlgAboutProc(HWND hWnd, WORD Msg, WPARAM wParam,
                                          LPARAM lParam);
void                    VBSS_WinHelp(HWND hWnd, long lIndex);

#ifdef  BUGS
// Bug-017
BOOL    GetSpreadAutoFill(HWND hWnd);
#endif

extern PROPINFO PI_NU235;
extern PROPINFO PI_NU236;

#ifdef SS_BOUNDCONTROL
#ifndef SS_QE
extern PROPINFO PI_NU237;
extern PROPINFO PI_NU238;
extern PROPINFO PI_NU239;
extern PROPINFO PI_NU240;
extern PROPINFO PI_NU241;
extern PROPINFO PI_NU242;
#endif
#endif

extern PROPINFO PI_NU248;

#ifdef IPROP_SPREAD_DATAFIELD
extern PROPINFO PropertyDataField;
#endif


LPMODELINFO FAR PASCAL _export VBGetModelInfo(USHORT usVersion)
{
#ifdef SS_V30
VBPV_InitModel(usVersion);
#endif
#ifdef SS_VB1ONLY
   return (&ModelInfoSpreadVB1);
#else
if (usVersion <= VB100_VERSION)
   return (&ModelInfoSpreadVB1);
else if (usVersion <= VB200_VERSION)
   return (&ModelInfoSpreadVB2);
else
   return (&ModelInfoSpread);
#endif
}


BOOL RegisterVBSpread(hInstance)

HANDLE hInstance;
{
NPPROPLIST npSpreadProps;
short      i;

#ifdef SS_VB1ONLY
npSpreadProps = SpreadProperties1;

for (i = 0; i < IPROP_SPREAD_CNT; i++)
   npSpreadProps[i] = SpreadProperties[i];
#else
if (VBGetVersion() < VB300_VERSION)
   {
   if (VBGetVersion() < VB200_VERSION)
      npSpreadProps = SpreadProperties1;
   else
      npSpreadProps = SpreadProperties2;

   for (i = 0; i < IPROP_SPREAD_CNT; i++)
      npSpreadProps[i] = SpreadProperties[i];
   }
else
   npSpreadProps = SpreadProperties;
#endif

#ifdef SS_BOUNDCONTROL
if (VBGetVersion() < VB300_VERSION)
   {
   npSpreadProps[IPROP_SPREAD_DATASOURCE] = &PI_NU235;
   npSpreadProps[IPROP_SPREAD_DATACHANGED] = &PI_NU236;

#ifndef SS_QE
#ifdef IPROP_SPREAD_DAUTOHEADINGS
   npSpreadProps[IPROP_SPREAD_DAUTOHEADINGS] = &PI_NU237;
#endif
#ifdef IPROP_SPREAD_DAUTOCELLTYPES
   npSpreadProps[IPROP_SPREAD_DAUTOCELLTYPES] = &PI_NU238;
#endif
#ifdef IPROP_SPREAD_DAUTOFILL
   npSpreadProps[IPROP_SPREAD_DAUTOFILL] = &PI_NU239;
#endif
#ifdef IPROP_SPREAD_DAUTOSIZECOLS
   npSpreadProps[IPROP_SPREAD_DAUTOSIZECOLS] = &PI_NU240;
#endif
#ifdef IPROP_SPREAD_DATAFIELD
   npSpreadProps[IPROP_SPREAD_DATAFIELD] = &PI_NU241;
#endif
#ifdef IPROP_SPREAD_DATAFILLEVENT
   npSpreadProps[IPROP_SPREAD_DATAFILLEVENT] = &PI_NU242;
#endif

#else
#ifdef IPROP_SPREAD_DATAFIELD
   npSpreadProps[IPROP_SPREAD_DATAFIELD] = &PropertyDataField;
#endif
#endif
   }
#endif

#ifdef IPROP_SPREAD_HELPCONTEXTID
if (VBGetVersion() < VB200_VERSION)
   npSpreadProps[IPROP_SPREAD_HELPCONTEXTID] = &PI_NU248;
#endif

#if 0
if (VBGetVersion() < VB200_VERSION)
   {
   // VB Version 1.0 has a bug in which no property past number 128
   // can have the PF_fSaveData flag set.  This also implies that it
   // cannot be shown at design time.

   for (i = 128; i < IPROP_SPREAD_CNT; i++)
      if (npSpreadProps[i] && (WORD)npSpreadProps[i] < ~IPROP_STD_LAST)
         {
         if (i != IPROP_SPREAD_DATACONNECT && i != IPROP_SPREAD_DATASELECT)
             {
            npSpreadProps[i]->fl &= ~PF_fSaveData;
            npSpreadProps[i]->fl |= PF_fNoShow;
            }
         }
      else
         break;
   }
#endif

vbSSRegisterPopupProc();
InitFunctionArray();

{
USHORT usVersion = VBGetVersion();

#ifdef SS_VB1ONLY
   return (VBRegisterModel(hInstance, &ModelSpreadVB1));
#else
if (usVersion <= VB100_VERSION)
   return (VBRegisterModel(hInstance, &ModelSpreadVB1));
else if (usVersion <= VB200_VERSION)
   return (VBRegisterModel(hInstance, &ModelSpreadVB2));
else
   return (VBRegisterModel(hInstance, &ModelSpread));
#endif
}
}


void vbSSInit(HCTL hCtl, HWND hWnd)
{
LPVBSPREAD lpSpread;

WM_COPYDATA = RegisterWindowMessage((LPCSTR)"COPY_DATA");

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

// SSSetBool(hWnd, SSB_REDRAW, 0);

lpSpread->CalcDependencies = TRUE;
lpSpread->FileNum = 0;
lpSpread->GridLines = TRUE;
lpSpread->Position = 0;
lpSpread->BlockMode = FALSE;

lpSpread->Col = ALLCOLS;
lpSpread->Row = ALLROWS;
lpSpread->CellType = 1;

lpSpread->nMaxEditLen = 60;
lpSpread->TypeEditCharSet = 0;
lpSpread->TypeEditCharCase = 1;

lpSpread->TypeTextJustifyVert = 2;
lpSpread->TypeTextWordWrap = FALSE;
lpSpread->TypeTextShadow = FALSE;

lpSpread->hTypePicMask = 0;
lpSpread->hTypePicDefText = 0;

DateDefault(&lpSpread->TypeDateFormat);
lpSpread->TypeDateMin.nMonth = 1;
lpSpread->TypeDateMin.nDay = 1;
lpSpread->TypeDateMin.nYear = 1900;
lpSpread->TypeDateMax.nMonth = 12;
lpSpread->TypeDateMax.nDay = 31;
lpSpread->TypeDateMax.nYear = 2100;

TimeDefault(&lpSpread->TypeTimeFormat);
lpSpread->TypeTimeMin.nHour = 0;
lpSpread->TypeTimeMin.nMinute = 0;
lpSpread->TypeTimeMin.nSecond = 0;
lpSpread->TypeTimeMax.nHour = 23;
lpSpread->TypeTimeMax.nMinute = 59;
lpSpread->TypeTimeMax.nSecond = 59;

#ifdef SS_V30
lpSpread->TypeIntegerMin = LONG_MIN;
lpSpread->TypeIntegerMax = LONG_MAX;
#else
lpSpread->TypeIntegerMin = -9999999;
lpSpread->TypeIntegerMax = 9999999;
#endif

lpSpread->TypeFloatSeparator = FALSE;
lpSpread->TypeFloatMoney = FALSE;
lpSpread->TypeFloatDecimalPlaces = 2;
lpSpread->TypeFloatMin = (float)-9999999.99;
lpSpread->TypeFloatMax = (float)9999999.99;

lpSpread->lIntegerSpinInc = 1;

lpSpread->dComboMaxDrop = 6;
lpSpread->dComboWidth = 0;
lpSpread->dComboMaxEditLen = 150;

lpSpread->ShadowColor = SS_EncodeSysColor(COLOR_BTNFACE);
lpSpread->ShadowText = SS_EncodeSysColor(COLOR_BTNTEXT);
lpSpread->ShadowDark = SS_EncodeSysColor(COLOR_BTNSHADOW);

lpSpread->TypeButtonColor.Color = SS_EncodeSysColor(COLOR_BTNFACE);
lpSpread->TypeButtonColor.ColorBorder = RGB(0, 0, 0);
lpSpread->TypeButtonColor.ColorShadow = SS_EncodeSysColor(COLOR_BTNSHADOW);
lpSpread->TypeButtonColor.ColorHighlight = SS_EncodeSysColor(COLOR_BTNHIGHLIGHT);
lpSpread->TypeButtonColor.ColorText = SS_EncodeSysColor(COLOR_BTNTEXT);
lpSpread->TypeButtonShadowSize = 2;

#ifndef SS_NOPRINT
lpSpread->Print.PrintBorder       = TRUE;
lpSpread->Print.PrintShadows      = TRUE;
lpSpread->Print.PrintGrid         = TRUE;
lpSpread->Print.PrintColHeaders   = TRUE;
lpSpread->Print.PrintRowHeaders   = TRUE;
lpSpread->Print.PrintUseDataMax   = TRUE;
lpSpread->Print.PrintColor        = FALSE;
lpSpread->Print.PrintType         = 0;
lpSpread->Print.PrintPageStart    = 1;
lpSpread->Print.PrintPageEnd      = 1;
lpSpread->Print.PrintMarginLeft   = 0L;
lpSpread->Print.PrintMarginTop    = 0L;
lpSpread->Print.PrintMarginRight  = 0L;
lpSpread->Print.PrintMarginBottom = 0L;
#endif

#ifdef SS_BOUNDCONTROL
lpSpread->DataOpts.fHeadings = TRUE;
lpSpread->DataOpts.fCellTypes = TRUE;
lpSpread->DataOpts.fAutoFill = TRUE;
lpSpread->DataOpts.fSizeCols = 2;
lpSpread->DataOpts.fAutoDataInform = TRUE;
lpSpread->DataOpts.fAutoSave = TRUE;
#endif

#ifdef SS_QE
if (lpSpread->DBInfo.fConnected)
   DBSS_EndBind(hWnd, &lpSpread->DBInfo);

if (lpSpread->DBInfo.hDataSelect)
   tbGlobalFree(lpSpread->DBInfo.hDataSelect);

if (lpSpread->DBInfo.hDataConnect)
   tbGlobalFree(lpSpread->DBInfo.hDataConnect);

lpSpread->DBInfo.hDataSelect = 0;
lpSpread->DBInfo.hDataConnect = 0;
#endif

lpSpread->dDataGetFillCol = -1;
lpSpread->dDataSetFillCol = -1;

SSSetUnitType(hWnd, SS_UNITTYPE_DEFAULT);

SSSetBool(hWnd, SSB_RETAINSELBLOCK, -1);

lpSpread->lMaxCols = SSGetMaxCols(hWnd);
lpSpread->lMaxRows = SSGetMaxRows(hWnd);

lpSpread->fUtopiaMode = 0;
#ifdef SS_UTP
vbSpreadSetUtopiaModeOn(hWnd, lpSpread, -1);
#endif
}


#ifdef SS_QE
void vbSSClearDB(HCTL hCtl, HWND hWnd)
{
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (lpSpread->DBInfo.fConnected)
   DBSS_EndBind(hWnd, &lpSpread->DBInfo);

if (lpSpread->DBInfo.hDataSelect)
   tbGlobalFree(lpSpread->DBInfo.hDataSelect);

if (lpSpread->DBInfo.hDataConnect)
   tbGlobalFree(lpSpread->DBInfo.hDataConnect);

lpSpread->DBInfo.hDataSelect = 0;
lpSpread->DBInfo.hDataConnect = 0;
}
#endif


/*******************************************************************************/
LONG FAR PASCAL _export vbSpreadProc(hCtl, hWnd, Msg, wParam, lParam)

HCTL                hCtl;
HWND                hWnd;
WORD                Msg;
WORD                wParam;
LONG                lParam;
{
LPVBSPREAD          lpSpread;
LPSPREADSHEET       lpSS;
SS_CELLTYPE         CellType;
LPTBMINMAXINFO      lpMinMaxInfo;
SS_CELLCOORD        CellUL;
SS_CELLCOORD        CellLR;
SPREADPARAMS1       Params1;
SPREADPARAMS2       Params2;
SPREADPARAMS3       Params3;
SPREADPARAMS4       Params4;
SPREADPARAMS5       Params5;
SPREADPARAMS6       Params6;
LPSS_LEAVECELL      LeaveCell;
LPMODEL             lpModel;
#ifndef SS_USE16BITCOORDS
LPSS_EDITMODEOFF    lpEditModeOff;
LPSS_CELLCOORD      lpCellCoord;
#endif
BOOL                Ret = TRUE;
BOOL                bParam;
BOOL                fChangeMade;
long                lRet;
short               Cancel;
short               Next;
short               Error;
SS_COORD            Col;
SS_COORD            Row;
SS_COORD            ColNew;
SS_COORD            RowNew;
short               KeyCode;
short               Shift;
WNDPROC             ProcInst;
short               dHelpID;

                                                  
switch (Msg)
   {
   case WM_CREATE:
#ifdef SS_NOSUPERCLASS
      lRet = SpreadSheetProc(hWnd, Msg, wParam, lParam);
#else
      lRet = VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam);
#endif

      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      ProcInst = (WNDPROC)MakeProcInstance((FARPROC)vbSpreadCallBackProc,
                                           hDynamicInst);
      SSSetCallBack(hWnd, ProcInst);
      SSSetOwner(hWnd, 0);

      lpModel = VBGetControlModel(hCtl);
#ifdef SS_BC
      lpSpread->fVB1 = TRUE;
#else
      // if (ModelSpread.npproplist == SpreadProperties1)
      if (VBGetVersion() < VB200_VERSION)
         lpSpread->fVB1 = TRUE;
#endif

      vbSSInit(hCtl, hWnd);

      SpreadInit(hWnd);

      //- Load enumerated property strings from resource file.
      //  (Masanori Iwasa)
      LoadEnumStrings(hCtl);

      return (lRet);

   case WM_CHAR:
   case WM_KEYDOWN:
   case WM_KEYUP:
      lpSS = SS_Lock(hWnd);

      if (lpSS->fMsgSentToVB)
         {
         long lRet = VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam);
         lpSS->wMsgLast = Msg;
         SS_Unlock(hWnd);
         return (lRet);
         }

#ifdef SS_VB
#ifdef SPREAD_JPN
      //-------------------------------------------------------------
      // This preprocessors should be left in. It's simulating
      // Visual Basic 3 for Visual Basic 2.0. A bug was added in
      // the VB JPN version where the 2nd byte of DBCS is removed
      // from the message queue.
      // JPNFIX0004 - (Masanori Iwasa)
      //-------------------------------------------------------------
      if (Msg == WM_CHAR)
         {
         SS_Unlock(hWnd);
         if(SS_USESELBAR(lpSS))     //- When bar selection fall through default.
            break;
         return(SpreadSheetProc(hWnd, Msg, wParam, lParam));
         }
#endif
#endif
      SS_Unlock(hWnd);
      break;

   case VBM_LOADED:
      lpSS = SS_Lock(hWnd);
#ifdef SS_BOUNDCONTROL
      if (VBGetVersion() > VB200_VERSION)
         vbSpreadBoundProc(lpSS, (LONG)hCtl, hWnd, VBM_DATA_INITIATE, wParam, lParam);
#endif

      PostMessage(hWnd, SSM_LOADED, 0, 0L);

      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      lpSpread->fVBMLoaded = TRUE;
      lpSS->fIgnoreSetFont = FALSE;
      SS_Unlock(hWnd);

      break;

   case VBM_INITPROPPOPUP:
      if (wParam == IPROP_SPREAD_ABOUT
#ifdef IPROP_SPREAD_SPREADDESIGNER
          || wParam == IPROP_SPREAD_SPREADDESIGNER
#endif
#ifdef SS_QE
          || wParam == IPROP_SPREAD_DATACONNECT
          || wParam == IPROP_SPREAD_DATASELECT)
#else
          )
#endif
         {
         HWND hWndPopup;

         hWndPopup = CreateWindow(CLASS_POPUP, NULL, WS_CHILD, 0, 0, 0, 0,
                                  hWnd, NULL, hDynamicInst, NULL);
         SetWindowWord(hWndPopup, 0, wParam);
         return (hWndPopup);
         }

      break;
  
   case VBM_SAVEPROPERTY:
#ifdef IPROP_SPREAD_SPREADDESIGNER
      if (wParam == IPROP_SPREAD_SPREADDESIGNER)
         {
         long         lLength;
         long         lLengthTemp;
         GLOBALHANDLE hBuffer;
         HPSTR        hpBuffer;

         lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

         if (lpSpread->fSaveDesignerInfo)
            {
            if (hBuffer = SSSaveToBuffer(hWnd, &lLength, FALSE))
               {
               VBWriteFormFile((HFORMFILE)lParam, (LPVOID)&lLength, 4);

               hpBuffer = (HPSTR)GlobalLock(hBuffer);

               while (lLength > 0)
                  {
                  lLengthTemp = min(lLength, 32767);
                  VBWriteFormFile((HFORMFILE)lParam, (LPVOID)hpBuffer,
                                  (WORD)lLengthTemp);

                  lLength -= lLengthTemp;
                  hpBuffer += lLengthTemp;
                  }

               GlobalUnlock(hBuffer);
               GlobalFree(hBuffer);
               }
            }

         else
            {
            lLength = 0L;
            VBWriteFormFile((HFORMFILE)lParam, (LPVOID)&lLength, 4);
            }

         return (0);
         }
#endif

      break;

   case VBM_LOADPROPERTY:
#ifdef IPROP_SPREAD_SPREADDESIGNER
      if (wParam == IPROP_SPREAD_SPREADDESIGNER)
         {
         long         lLength;
         long         lLengthTemp;
         long         lLengthOrig;
         GLOBALHANDLE hBuffer;
         HPSTR        hpBuffer;

         if (!VBReadFormFile((HFORMFILE)lParam, (LPVOID)&lLength, 4) &&
             lLength > 0L)
            {
            lLengthOrig = lLength;

            if (hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                      lLength + 1))
               {
               hpBuffer = (HPSTR)GlobalLock(hBuffer);

               while (lLength > 0)
                  {
                  lLengthTemp = min(lLength, 32767L);
                  VBReadFormFile((HFORMFILE)lParam, (LPVOID)hpBuffer,
                                 (WORD)lLengthTemp);

                  lLength -= lLengthTemp;
                  hpBuffer += lLengthTemp;
                  }

               GlobalUnlock(hBuffer);
               lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
               lpSpread->fSaveDesignerInfo = TRUE;

//RWP01m:001 ->
//             lpSS = SS_Lock(hWnd);
//             lpSS->fIgnoreSetFont = TRUE;
//             SS_Unlock(hWnd);
//RWP01m:001 <-

               SSLoadFromBuffer(hWnd, hBuffer, lLengthOrig);


//RWP01m:001 ->
               lpSS = SS_Lock(hWnd);
               lpSS->fIgnoreSetFont = TRUE;
               SS_Unlock(hWnd);
//RWP01m:001 <-

               GlobalFree(hBuffer);
               }
            }

         return (0);
         }
#endif

      break;

   case VBM_HELP:
      switch (LOBYTE(wParam))
         {
         case VBHELP_PROP:
            if (HIBYTE(wParam) > 20
#ifdef SS_BOUNDCONTROL
                && HIBYTE(wParam) != IPROP_SPREAD_DATASOURCE
                && HIBYTE(wParam) != IPROP_SPREAD_DATACHANGED
#endif
#ifdef IPROP_SPREAD_HELPCONTEXTID
                && HIBYTE(wParam) != IPROP_SPREAD_HELPCONTEXTID)
#else
                )
#endif
               {
#ifdef SS_V21
               VBSS_WinHelp(hWnd, 200L + (DWORD)HIBYTE(wParam));
#else               
// Property help base starts at 9000
               VBSS_WinHelp(hWnd, 9000L + (DWORD)HIBYTE(wParam));
#endif        
               return (0);
               }

            break;

         case VBHELP_EVT:
            {
            VBSPREAD Spread;
            lpSpread = &Spread;
            lpSpread->fVB1 = VBGetVersion() < VB200_VERSION ? TRUE : FALSE;

            dHelpID = HIBYTE(wParam);
            if (dHelpID != SS_EVENT_Click &&
                dHelpID != SS_EVENT_DblClick &&
                dHelpID != SS_EVENT_DragDrop &&
                dHelpID != SS_EVENT_DragOver &&
                dHelpID != SS_EVENT_GotFocus &&
                dHelpID != SS_EVENT_KeyDown &&
                dHelpID != SS_EVENT_KeyPress &&
                dHelpID != SS_EVENT_KeyUp &&
                dHelpID != SS_EVENT_LostFocus &&
                dHelpID != SS_EVENT_MouseDown &&
                dHelpID != SS_EVENT_MouseMove &&
                dHelpID != SS_EVENT_MouseUp)
               {
#ifdef SS_GRID
               if (lpSpread->fVB1)
#ifdef SS_V21
                  VBSS_WinHelp(hWnd, 550L + (DWORD)dHelpID);
#else
// Grid Event (VB1.0) help base starts at 5500 
                  VBSS_WinHelp(hWnd, 5500L + (DWORD)dHelpID);
#endif
               else
#ifdef SS_V21
                  VBSS_WinHelp(hWnd, 500L + (DWORD)dHelpID);
#else
// Grid Event (> VB1.0) help base starts at 5000
                  VBSS_WinHelp(hWnd, 5000L + (DWORD)dHelpID);
#endif
#else
#ifdef SS_V21
               VBSS_WinHelp(hWnd, 500L + (DWORD)dHelpID);
#else
// Spread Event help base starts at 5000
               VBSS_WinHelp(hWnd, 9500L + (DWORD)dHelpID);
#endif
#endif
               return (0);
               }
            }

            break;

         case VBHELP_CTL:
#ifdef SS_V21
            VBSS_WinHelp(hWnd, 1);
#else            
// Spread Control help base starts at 1000
            VBSS_WinHelp(hWnd, 1000);
#endif
            return (0);
         }

      break;

   case WM_SETFONT:
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      if (!hWnd || !IsWindow(hWnd) || !SS_Lock(hWnd))
         return (0);

      if (VBGetMode() == MODE_DESIGN)
         Ret = SSSetFont(hWnd, ALLCOLS, ALLROWS, (HFONT)wParam, FALSE);

      else if (lpSpread->BlockMode)
         {
         VBSSVALCOLROW(lpSpread);
         VBSSVALCOL2ROW2(lpSpread);
         LIMIT_TO_COLROW;
         Ret = SSSetFontRange(hWnd, lpSpread->Col, lpSpread->Row,
                              lpSpread->Col2, lpSpread->Row2,
                              (HFONT)wParam, FALSE);
         }

      else
         {
         VBSSVALCOLROW(lpSpread);
         LIMIT_TO_COLROW;
         Ret = SSSetFont(hWnd, lpSpread->Col, lpSpread->Row, (HFONT)wParam,
                         FALSE);
         }

      if (lpSpread->hFontRgn)
         {
         lpSpread->fFontRgnSet = TRUE;
         GetUpdateRgn(hWnd, lpSpread->hFontRgn, FALSE);
         }

      if (!Ret)
         return (ERR_INVALID_PROPVAL);

      return (0);

   case WM_GETFONT:
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      if (VBGetMode() == MODE_DESIGN)
         return (SSGetFont(hWnd, ALLCOLS, ALLROWS));

      VBSSVALCOLROW(lpSpread);
      LIMIT_TO_COLROW;
      return (SSGetFont(hWnd, lpSpread->Col, lpSpread->Row));

   case WM_SETFOCUS:
      if (VBGetMode() == MODE_DESIGN)
         return (0);

      if (hWnd && (lpSS = SS_Lock(hWnd)))
         {
         if (lpSS->fIgnoreKillFocus)
            {
            SS_Unlock(hWnd);
            return (0);
            }

         SS_Unlock(hWnd);
         }

#ifdef SS_NOSUPERCLASS
      lRet = SpreadSheetProc(hWnd, Msg, wParam, lParam);
#else
      lRet = VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam);
#endif

#ifdef SS_NOPROCESSFOCUS
      {
      if ((lpSS = SS_Lock(hWnd)) != NULL)
         {
         if (!lpSS->lpBook->EditModeTurningOn && !lpSS->lpBook->EditModeTurningOff)
            {
            if (!lpSS->wMessageBeingSent)
               {
               lpSS->fSetFocusRecieved = FALSE;
               PostMessage(hWnd, SSM_SETFOCUSPOSTPROCESS, 0, 0L);
               }
            else if (lpSS->lpBook->hWndFocusPrev)
               {
               lpSS->fIgnoreKillFocus = TRUE;
               SetFocus(lpSS->lpBook->hWndFocusPrev);
               lpSS->fIgnoreKillFocus = FALSE;
               }
            }

         SS_Unlock(hWnd);
         }
      }
#endif

      return (lRet);

   case WM_KILLFOCUS:
   case WM_MOUSEACTIVATE:
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      if (hWnd && IsWindow(hWnd) && (lpSS = SS_Lock(hWnd)))
         {
         if (Msg == WM_KILLFOCUS && lpSS->fIgnoreKillFocus)
            {
            SS_Unlock(hWnd);
            return (0);
            }

         if (!lpSS->lpBook->EditModeTurningOn &&
             !lpSS->lpBook->EditModeTurningOff)
            {
#if 0
            if (Msg == WM_KILLFOCUS)
               {
               lpSS->fSaveBoundData = TRUE;
               PostMessage(hWnd, SSM_PROCESSSAVEBOUND, 0, 0L);
               }
#endif

            lpSpread->fWantSpecialKeyDown = FALSE;
            }

         if (Msg == WM_KILLFOCUS && lpSS->lpBook->EditModeTurningOn)
            {
            SS_Unlock(hWnd);
            return (0);
            }

         SS_Unlock(hWnd);
         }

      if (VBGetMode() == MODE_DESIGN)
         return (0);

      break;

#if 0
#ifdef SS_BOUNDCONTROL
   case SSM_PROCESSKILLFOCUS:
      if (hWnd && IsWindow(hWnd) && (lpSS = SS_Lock(hWnd)))
         {
         lpSS->fSaveBoundData = FALSE;
         SS_Unlock(hWnd);
         }

      break;

   case SSM_PROCESSSAVEBOUND:
      if (hWnd && IsWindow(hWnd) && (lpSS = SS_Lock(hWnd)))
         {
         if (lpSS->fSaveBoundData)
            {
            vbSpreadSaveBoundRow(lpSS, hCtl, hWnd, lpSS->Row.CurAt, lpSS->Row.CurAt,
                                 lpSS->Col.CurAt);
            lpSS->fSaveBoundData = FALSE;
            }

         SS_Unlock(hWnd);
         }

      return (0);
#endif
#endif

   case WM_CTLCOLOR:
      return (SpreadSheetProc(hWnd, Msg, wParam, lParam));

   case WM_LBUTTONDBLCLK:
#ifdef SS_NOSUPERCLASS
      lRet = SpreadSheetProc(hWnd, Msg, wParam, lParam);
#else
      VBSuperControlProc(hCtl, Msg, wParam, lParam);
#endif
      return (0);

/*
   case WM_RBUTTONDOWN:
      if (VBGetMode() == MODE_DESIGN)
         {
         return (0);
         }

      break;

   case WM_RBUTTONDBLCLK:
   case WM_RBUTTONDUP:
      if (VBGetMode() == MODE_DESIGN)
         return (0);

      break;
*/

   case WM_GETMINMAXINFO:
      lpMinMaxInfo = (LPTBMINMAXINFO)lParam;
      lpMinMaxInfo->ptMinTrackSize.x = 2;
      lpMinMaxInfo->ptMinTrackSize.y = 2;
      return (0);

   case VBM_SETPROPERTY:
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      if(CallBack[wParam])
          return (CallBack[wParam])(TRUE,hCtl,hWnd,Msg,wParam,lParam,lpSpread);

      break;

   case VBM_GETPROPERTY:
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      if(CallBack[wParam])
          return (CallBack[wParam])(FALSE,hCtl,hWnd,Msg,wParam,lParam,lpSpread);

      break;

   case VBM_FIREEVENT:
#ifdef SS_NOPROCESSFOCUS
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      if (wParam == (WPARAM)SS_EVENT_GotFocus)
         {
         lpSS = SS_Lock(hWnd);
         if (!lpSS->EditModeOn)
            {
#ifdef SS_NOSUPERCLASS
            SpreadSheetProc(hWnd, SSM_SETFOCUSINTERNAL, (WORD)lParam, 0L);
#else
            VBSuperControlProc(hCtl, SSM_SETFOCUSINTERNAL, (WORD)lParam, 0L);
#endif
            if (!IsWindow(hWnd))
               return (0);
            }

         SS_Unlock(hWnd);
         }

      else if (wParam == (WPARAM)SS_EVENT_LostFocus)
         {
         HWND hWndFocus = GetFocus();

         if (!hWndFocus || (hWndFocus != hWnd &&
             GetParent(hWndFocus) != hWnd &&
             (!GetParent(hWndFocus) ||
             GetParent(GetParent(hWndFocus)) != hWnd)))
            {
#ifdef SS_NOSUPERCLASS
            SpreadSheetProc(hWnd, SSM_KILLFOCUSINTERNAL, (WORD)hWndFocus, 0L);
#else
            VBSuperControlProc(hCtl, SSM_KILLFOCUSINTERNAL,
                               (WORD)hWndFocus, 0L);
#endif

            if (!IsWindow(hWnd))
               return (0);
            }
         }
#endif

      break;

   case VBM_WANTSPECIALKEY:
      /*************************
      * Only process a keydown
      *************************/

      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      if (!(GetKeyState(wParam) >> 15))
         {
         if (!lpSpread->fWantSpecialKeyDown)
            SendMessage(hWnd, SSM_FOCUSFROMTAB, HIBYTE(GetKeyState(VK_SHIFT)),
                        0L);

         lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
         lpSpread->fWantSpecialKeyDown = FALSE;
         break;
         }
      else
         lpSpread->fWantSpecialKeyDown = TRUE;

      if (wParam == VK_TAB && SSGetBool(hWnd, SSB_PROCESSTAB))
         {
         lpSS = SS_Lock(hWnd);
         if (lpSS->wOpMode == SS_OPMODE_READONLY)
            {
            SS_Unlock(hWnd);
            return (FALSE);
            }

         if (HIBYTE(GetKeyState(VK_SHIFT)))
            {
            if (SSx_ScrollPrevCell(lpSS, TRUE))
               if (!vbSpreadFireQueryAdvance(hCtl, 0))
                  {
                  SS_Unlock(hWnd);
                  return (FALSE);
                  }
            }
         else
            {
            if (SSx_ScrollNextCell(lpSS, TRUE, FALSE))
               if (!vbSpreadFireQueryAdvance(hCtl, 1))
                  {
                  SS_Unlock(hWnd);
                  return (FALSE);
                  }
            }

         SS_Unlock(hWnd);

         return (TRUE);
         }

      else if (wParam == VK_UP)
         {
         if (SSGetEditMode(hWnd))
            {
            SSGetActiveCell(hWnd, &Col, &Row);

            if (SSGetCellType(hWnd, Col, Row, &CellType) &&
                CellType.Type == SS_TYPE_COMBOBOX)
               return (TRUE);
            }

         lpSS = SS_Lock(hWnd);
         if (SS_ScrollLineUpTest(lpSS))
            if (!vbSpreadFireQueryAdvance(hCtl, 0))
               return (FALSE);

         return (TRUE);
         }

      else if (wParam == VK_DOWN)
         {
         if (SSGetEditMode(hWnd))
            {
            SSGetActiveCell(hWnd, &Col, &Row);

            if (SSGetCellType(hWnd, Col, Row, &CellType) &&
                CellType.Type == SS_TYPE_COMBOBOX)
               return (TRUE);
            }

         lpSS = SS_Lock(hWnd);
         if (SS_ScrollLineDownTest(lpSS))
            if (!vbSpreadFireQueryAdvance(hCtl, 1))
               return (FALSE);

         return (TRUE);
         }

      break;

   case VBM_ISMNEMONIC:
      lpSS = SS_Lock(hWnd);
      lRet = SS_ProcessMnemonic(lpSS, (char)wParam));
      SS_Unlock(hWnd);
      return lRet;

#ifdef SS_BOUNDCONTROL
   case VBM_DATA_AVAILABLE:
      lpSS = SS_Lock(hWnd);
      lpSS->fProcessingDataAvailable = TRUE;
      lRet = vbSpreadBoundProc(lpSS, (LONG)hCtl, hWnd, Msg, wParam, lParam);
      lpSS->fProcessingDataAvailable = FALSE;
      SS_Unlock(hWnd);
      return lRet;
   case VBM_DATA_REQUEST:   
      lpSS = SS_Lock(hWnd);
      lpSS->fProcessingDataRequest = TRUE;
      lRet = vbSpreadBoundProc(lpSS, (LONG)hCtl, hWnd, Msg, wParam, lParam);
      lpSS->fProcessingDataRequest = FALSE;
      SS_Unlock(hWnd);
      return lRet;
#endif

   case SSM_CELLLOCKED:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Error = EDITERROR_CELLLOCKED;
      Params3.lpParam1 = &Col;
      Params3.lpParam2 = &Row;
      Params3.lpParam3 = &Error;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
      return (1);

   case SSM_CLICK:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params2.lpParam1 = &Col;
      Params2.lpParam2 = &Row;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_Click, &Params2);
      return (0);

#if (!defined(SS_GRID))
   case SSM_COLRESTRICTED:
      Col = (SS_COORD)lParam;
      Row = -1;
      Error = EDITERROR_COLRESTRICTED;

      if (Col == SS_HEADER)
         Col = 0;

      Params3.lpParam1 = &Col;
      Params3.lpParam2 = &Row;
      Params3.lpParam3 = &Error;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
      return (0);
#endif

   case SSM_DBLCLK:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params2.lpParam1 = &Col;
      Params2.lpParam2 = &Row;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_DblClick, &Params2);
      return (0);

   case SSM_DATACHANGE:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params2.lpParam1 = &Col;
      Params2.lpParam2 = &Row;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_Change, &Params2);
      if (SS_IsDestroyed(hWnd))
         return 0;

#ifdef SS_BOUNDCONTROL
      lpSS = SS_Lock(hWnd);
      VBSSBoundChangeMade(lpSS, (LONG)hCtl, hWnd, Col, Row);
      SS_Unlock(hWnd);
#endif

      return (0);

   case SSM_EDITCHANGE:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params2.lpParam1 = &Col;
      Params2.lpParam2 = &Row;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EditChange, &Params2);
      return (0);

   case SSM_EDITMODEON:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      bParam = TRUE;
      wParam = 0;
      Params4.lpParam1 = &Col;
      Params4.lpParam2 = &Row;
      Params4.lpParam3 = &bParam;
      Params4.lpParam4 = &wParam;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EditMode, &Params4);
      return (0);

   case SSM_EDITMODEOFF:
      fChangeMade = FALSE;
      bParam = FALSE;

#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);

      if (wParam)
         fChangeMade = (WORD)-1;
#else
      lpEditModeOff = (LPSS_EDITMODEOFF)lParam;

      Col = lpEditModeOff->Col;
      Row = lpEditModeOff->Row;
      if (lpEditModeOff->fChangeMade)
         fChangeMade = (WORD)-1;
#endif

      Params4.lpParam1 = &Col;
      Params4.lpParam2 = &Row;
      Params4.lpParam3 = &bParam;
      Params4.lpParam4 = &fChangeMade;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EditMode, &Params4);

      return (0);

   case SSM_ROWRESTRICTED:
      Row = (SS_COORD)lParam;
      Col = -1;
      Error = EDITERROR_ROWRESTRICTED;

      if (Row == SS_HEADER)
         Row = 0;

      Params3.lpParam1 = &Col;
      Params3.lpParam2 = &Row;
      Params3.lpParam3 = &Error;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
      return (0);

   case SSM_COLROWRESTRICTED:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif
      Error = EDITERROR_COLROWRESTRICTED;

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params3.lpParam1 = &Col;
      Params3.lpParam2 = &Row;
      Params3.lpParam3 = &Error;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
      return (0);

   case SSM_LEAVECELL:
      Cancel = 0;
      LeaveCell = (LPSS_LEAVECELL)lParam;
      Col = LeaveCell->ColCurrent;
      Row = LeaveCell->RowCurrent;
      ColNew = LeaveCell->ColNew;
      RowNew = LeaveCell->RowNew;
      Params5.lpParam1 = &Col;
      Params5.lpParam2 = &Row;
      Params5.lpParam3 = &ColNew;
      Params5.lpParam4 = &RowNew;
      Params5.lpParam5 = &Cancel;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_LeaveCell, &Params5);
      if (SS_IsDestroyed(hWnd))
         return 0;

#ifdef SS_BOUNDCONTROL
      lpSS = SS_Lock(hWnd);
      if (!Cancel && Row != RowNew)
         Cancel = vbSpreadSaveBoundRow(lpSS, hCtl, hWnd, Row, RowNew, ColNew);
      SS_Unlock(hWnd);
#endif

      return (Cancel);

   case SSM_KEYDOWN:
      if (SSGetEditMode(hWnd))
         {
         KeyCode = LOWORD(lParam);
         Shift = HIWORD(lParam);
         Params2.lpParam1 = &KeyCode;
         Params2.lpParam2 = &Shift;
         lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
         VBFireEvent(hCtl, SS_EVENT_KeyDown, &Params2);
         // BJO 29Aug97 JOK3402 - Begin fix
         return 0 == KeyCode;
         // BJO 29Aug97 JOK3402 - Edn fix
         }

      return (0);

   case SSM_KEYUP:
      if (SSGetEditMode(hWnd))
         {
         KeyCode = LOWORD(lParam);
         Shift = HIWORD(lParam);
         Params2.lpParam1 = &KeyCode;
         Params2.lpParam2 = &Shift;
         lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
         VBFireEvent(hCtl, SS_EVENT_KeyUp, &Params2);
         }

      return (0);

   case SSM_KEYPRESS:
      Params1.lpParam1 = (LPVOID)lParam;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_KeyPress, &Params1);
      return (0);

#ifndef SS_NOVIRTUAL
#ifndef SSE_NO_QUERYDATA
   case SSM_VQUERYDATA:
      {
      LPSS_VQUERYDATA lpVQueryData;

      lpVQueryData = (LPSS_VQUERYDATA)lParam;

      Params6.lpParam1 = &lpVQueryData->Row;
      Params6.lpParam2 = &lpVQueryData->RowsNeeded;
      Params6.lpParam3 = &lpVQueryData->RowsLoaded;
      Params6.lpParam4 = &lpVQueryData->wDirection;
      Params6.lpParam5 = &lpVQueryData->fAtTop;
      Params6.lpParam6 = &lpVQueryData->fAtBottom;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_QueryData, &Params6);
      if (SS_IsDestroyed(hWnd))
         return 0;

      lRet = 0;

#ifdef SS_BOUNDCONTROL
      if (VBGetVersion() > VB200_VERSION && VBGetMode() != MODE_DESIGN &&
          VBSSBoundIsBound((LONG)hCtl))
			{
			lpSS = SS_Lock(hWnd);
         lRet = vbSSBoundVQueryData(lpSS, (LONG)hCtl, hWnd, lpVQueryData);
			SS_Unlock(hWnd);
			}
#endif

#ifdef SS_QE
      if (lpSpread->DBInfo.fConnected)
         lpVQueryData->RowsLoaded = DBSS_GetRecs(hWnd, lpSpread,
                                                &lpSpread->DBInfo,
                                                lpVQueryData->Row,
                                                lpVQueryData->RowsNeeded,
                                                &lpVQueryData->fAtTop,
                                                &lpVQueryData->fAtBottom);
#endif
      }

      return (lRet);
#endif

#ifndef SSE_NO_VIRTUALCLEARDATA
   case SSM_VCLEARDATA:
      {
      LPSS_VCLEARDATA lpVClearData;

      lpVClearData = (LPSS_VCLEARDATA)lParam;

      Params2.lpParam1 = &lpVClearData->Row;
      Params2.lpParam2 = &lpVClearData->RowsBeingCleared;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_VirtualClearData, &Params2);
      if (SS_IsDestroyed(hWnd))
         return 0;

#ifdef SS_BOUNDCONTROL
      if (VBGetVersion() > VB200_VERSION && VBGetMode() != MODE_DESIGN &&
          VBSSBoundIsBound((LONG)hCtl))
			{
			lpSS = SS_Lock(hWnd);
         lRet = vbSSBoundVClearData(lpSS, (LONG)hCtl, hWnd, lpVClearData);
			SS_Unlock(hWnd);
			}
#endif

      lRet = 0;
      }
      return (lRet);
#endif
#endif

#ifndef SSE_NO_COLWIDTHCHANGE
   case SSM_COLWIDTHCHANGE:
      {
      LPSS_COORDRANGE lpCoordRange;
      SS_COORD        Coord1;
      SS_COORD        Coord2;

      lpCoordRange = (LPSS_COORDRANGE)lParam;

      Coord1 = lpCoordRange->Coord1;
      Coord2 = lpCoordRange->Coord2;
      Params2.lpParam1 = &Coord1;
      Params2.lpParam2 = &Coord2;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_ColWidthChange, &Params2);
      }
      break;
#endif

#ifndef SSE_NO_ROWHEIGHTCHANGE
   case SSM_ROWHEIGHTCHANGE:
      {
      LPSS_COORDRANGE lpCoordRange;
      SS_COORD        Coord1;
      SS_COORD        Coord2;

      lpCoordRange = (LPSS_COORDRANGE)lParam;

      Coord1 = lpCoordRange->Coord1;
      Coord2 = lpCoordRange->Coord2;
      Params2.lpParam1 = &Coord1;
      Params2.lpParam2 = &Coord2;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_RowHeightChange, &Params2);
      }
      break;
#endif

#ifndef SSE_NO_ENTERROW
   case SSM_ENTERROW:
      {
      LPSS_ENTERROW lpEnterRow = (LPSS_ENTERROW)lParam;

      Params2.lpParam1 = &lpEnterRow->Row;
      Params2.lpParam2 = &lpEnterRow->fNewRow;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_EnterRow, &Params2);
      }
      break;
#endif

#ifndef SSE_NO_LEAVEROW
   case SSM_LEAVEROW:
      {
      SPREADPARAMS7 Params7;
      LPSS_LEAVEROW lpLeaveRow;
      short         dNewRowIsLast;
      short         dRowWasLast;
      short         dAllCellsHaveData;
      short         dRowChanged;

      Cancel = 0;
      lpLeaveRow = (LPSS_LEAVEROW)lParam;
      Row = lpLeaveRow->RowCurrent;
      dRowWasLast = lpLeaveRow->fAddRow;
      dRowChanged = lpLeaveRow->fRowChangeMade;
      dAllCellsHaveData = lpLeaveRow->fAllCellsHaveData;
      RowNew = lpLeaveRow->RowNew;
      dNewRowIsLast = lpLeaveRow->fRowNewBeyondLastDataRow;
      Params7.lpParam1 = &Row;
      Params7.lpParam2 = &dRowWasLast;
      Params7.lpParam3 = &dRowChanged;
      Params7.lpParam4 = &dAllCellsHaveData;
      Params7.lpParam5 = &RowNew;
      Params7.lpParam6 = &dNewRowIsLast;
      Params7.lpParam7 = &Cancel;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_LeaveRow, &Params7);
      if (SS_IsDestroyed(hWnd))
         return 0;

#ifdef SS_BOUNDCONTROL
      lpSS = SS_Lock(hWnd);
      if (!Cancel && Row != RowNew && RowNew != -1)
         Cancel = vbSpreadSaveBoundRow(lpSS, hCtl, hWnd, Row, RowNew, -1);
      SS_Unlock(hWnd);

#endif

      }
      return (Cancel);
#endif

#ifndef SSE_NO_RIGHTCLICK
   case SSM_RBUTTON:
      {
      LPSS_RBUTTON lpRButton = (LPSS_RBUTTON)lParam;
      long         lMouseX;
      long         lMouseY;

      lMouseX = VBXPixelsToTwips(lpRButton->xPos);
      lMouseY = VBYPixelsToTwips(lpRButton->yPos);

      Params5.lpParam1 = &lpRButton->RButtonType;

      Col = lpRButton->Col;
      Row = lpRButton->Row;

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params5.lpParam2 = &Col;
      Params5.lpParam3 = &Row;
      Params5.lpParam4 = &lMouseX;
      Params5.lpParam5 = &lMouseY;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_RightClick, &Params5);
      }
      return (0);
#endif

#ifndef SS_NODRAGDROP
   case SSM_DRAGDROP:
      {
      LPSS_DRAGDROP  lpDragDrop = (LPSS_DRAGDROP)lParam;
      SPREADPARAMS12 Params12;

      Cancel = 0;
      Params12.lpParam1 = &lpDragDrop->BlockPrev.UL.Col;
      Params12.lpParam2 = &lpDragDrop->BlockPrev.UL.Row;
      Params12.lpParam3 = &lpDragDrop->BlockPrev.LR.Col;
      Params12.lpParam4 = &lpDragDrop->BlockPrev.LR.Row;
      Params12.lpParam5 = &lpDragDrop->BlockNew.UL.Col;
      Params12.lpParam6 = &lpDragDrop->BlockNew.UL.Row;
      Params12.lpParam7 = &lpDragDrop->BlockNew.LR.Col;
      Params12.lpParam8 = &lpDragDrop->BlockNew.LR.Row;
      Params12.lpParam9 = &lpDragDrop->fDataBeingOverwritten;
      Params12.lpParam10 = &lpDragDrop->fCopy;
      Params12.lpParam11 = &lpDragDrop->fDataOnly;
      Params12.lpParam12 = &lpDragDrop->fCancel;

      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_DragDropBlock, &Params12);

#ifdef SS_BOUNDCONTROL
      lpSS = SS_Lock(hWnd);

      if (!lpDragDrop->fCancel &&
          lpSS->Row.CurAt != lpDragDrop->BlockNew.UL.Row &&
          lpDragDrop->BlockNew.UL.Row != 0)
//         vbSpreadBoundClick(hCtl, hWnd, lpDragDrop->BlockNew.UL.Row);
         vbSpreadSaveBoundRow(lpSS, hCtl, hWnd, lpSS->Row.CurAt,
                              lpDragDrop->BlockNew.UL.Row,
                              lpDragDrop->BlockNew.UL.Col);

      SS_Unlock(hWnd);
#endif
      }

      return (0);
#endif

#ifndef SS_NOCT_OWNERDRAW
   case SSM_DRAWITEM:
      {
      LPSS_DRAWITEMSTRUCT lpDrawItemStruct;
      SPREADPARAMS8       Params8;
      long                lLeft;
      long                lTop;
      long                lBottom;
      long                lRight;

      lpDrawItemStruct = (LPSS_DRAWITEMSTRUCT)lParam;

      lLeft = VBXPixelsToTwips(lpDrawItemStruct->Rect.left);
      lTop = VBYPixelsToTwips(lpDrawItemStruct->Rect.top);
      lRight = VBXPixelsToTwips(lpDrawItemStruct->Rect.right);
      lBottom = VBYPixelsToTwips(lpDrawItemStruct->Rect.bottom);

      Params8.lpParam1 = &lpDrawItemStruct->Col;
      Params8.lpParam2 = &lpDrawItemStruct->Row;
      Params8.lpParam3 = &lpDrawItemStruct->hDC;
      Params8.lpParam4 = &lLeft;
      Params8.lpParam5 = &lTop;
      Params8.lpParam6 = &lRight;
      Params8.lpParam7 = &lBottom;
      Params8.lpParam8 = &lpDrawItemStruct->lStyle;

      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_DrawItem, &Params8);
      }
      return (0);
#endif

// removed 1/17/96 RAP
// Spread Designer no longer sends these messages.  
#ifdef SS_V21
#ifndef SSP_NO_SPREADDESIGNER
   case SSM_GETINFOFROMDESIGNER:
      if (VBGetMode() == MODE_DESIGN)
         {
         if (VBGetVersion() > VB200_VERSION)
            VBDirtyForm(hCtl);

         lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
         lpSpread->fSaveDesignerInfo = TRUE;
         }

      SSLoadFromBuffer(hWnd, (GLOBALHANDLE)wParam, lParam);
      GlobalFree((GLOBALHANDLE)wParam);

      return (0);
   case SSM_SAVEINFOTODESIGNER:
      {
      HWND hWndSrc = LOWORD(lParam);
      HWND hWndDest = HIWORD(lParam);

      if (wParam)                      // fInit = TRUE
         {
         lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
         lpSpread->hWndDesigner = hWndDest;
         }

      lpSS = SS_Lock(hWnd);

      {
      GLOBALHANDLE hBuffer;
      long         lBufferLen;

      if (hBuffer = SSSaveToBuffer(hWndSrc, &lBufferLen, FALSE))
         if (wParam)
            SendMessage(hWndDest, SSM_GETINFOFROMDESIGNER, hBuffer,
                        lBufferLen);
         else
            PostMessage(hWndDest, SSM_GETINFOFROMDESIGNER, hBuffer, lBufferLen);
      }

      SS_Unlock(hWnd);
      }
      return (0);
#endif
#endif

#ifndef SS_NOPRINT
   case SSM_FIREEVENTPRINTABORT:
      Params1.lpParam1 = (LPVOID)lParam;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_PrintAbort, &Params1);
      return (0);

   case SSM_FIREEVENTPRINTMSGOPEN:
      {
      HLSTR hlstrFieldName;
      WORD  PrintID = wParam;
      BOOL  fOpen = -1;
      BOOL  fProcessed = FALSE;

      hlstrFieldName = VBCreateHlstr((LPSTR)lParam, lstrlen((LPSTR)lParam));
      Params4.lpParam1 = hlstrFieldName;
      Params4.lpParam2 = &PrintID;
      Params4.lpParam3 = &fOpen;
      Params4.lpParam4 = &fProcessed;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_PrintMsgBox, &Params4);

      if (hlstrFieldName)
         VBDestroyHlstr(hlstrFieldName);

      return (fProcessed);
      }

   case SSM_FIREEVENTPRINTMSGCLOSE:
      {
      HLSTR hlstrFieldName;
      WORD  PrintID = wParam;
      BOOL  fOpen = 0;
      BOOL  fProcessed = FALSE;

      hlstrFieldName = VBCreateHlstr((LPSTR)lParam, lstrlen((LPSTR)lParam));
      Params4.lpParam1 = hlstrFieldName;
      Params4.lpParam2 = &PrintID;
      Params4.lpParam3 = &fOpen;
      Params4.lpParam4 = &fProcessed;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_PrintMsgBox, &Params4);

      if (hlstrFieldName)
         VBDestroyHlstr(hlstrFieldName);

      return (fProcessed);
      }
#endif

#ifndef SSE_NO_INSDEL
   case SSM_INSDEL:
      {
      LPSS_INSDEL lpInsDel = (LPSS_INSDEL)lParam;
      BOOL        fCancel = FALSE;

      Params6.lpParam1 = &lpInsDel->wType;
      Params6.lpParam2 = &lpInsDel->wOperation;
      Params6.lpParam3 = &lpInsDel->wDirection;
      Params6.lpParam4 = &lpInsDel->Coord1;
      Params6.lpParam5 = &lpInsDel->Coord2;
      Params6.lpParam6 = &fCancel;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_InsDel, &Params6);

      if (fCancel)
         return (1);
      else
         return (0);
      }
#endif

#ifndef SSE_NO_ARROWPRESSED
   case SSM_ARROWPRESSED:
      /*
      #define SS_MODE_ARROW_LEFT     1
      #define SS_MODE_ARROW_RIGHT    2
      #define SS_MODE_ARROW_UP       3
      #define SS_MODE_ARROW_DOWN     4
      */
      Params1.lpParam1 = &wParam;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_ArrowPressed, &Params1);
      return (0);
#endif

#ifdef SS_V30
#ifndef SSE_NO_COMBOCLOSEUP
   case SSM_COMBOCLOSEUP:
      {
      LPSS_COMBOCLOSEUP lpCloseUp = (LPSS_COMBOCLOSEUP)lParam;

      if (lpCloseUp->Col == SS_HEADER)
         lpCloseUp->Col = 0;

      if (lpCloseUp->Row == SS_HEADER)
         lpCloseUp->Row = 0;

      Params3.lpParam1 = &lpCloseUp->Col;
      Params3.lpParam2 = &lpCloseUp->Row;
      Params3.lpParam3 = &lpCloseUp->dSelChange;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_ComboCloseUp, &Params3);
      }
      return (0);
#endif

#ifndef SSE_NO_COMBODROPDOWN
   case SSM_COMBODROPDOWN:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params2.lpParam1 = &Col;
      Params2.lpParam2 = &Row;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_ComboDropDown, &Params2);
      return (0);
#endif

#ifndef SSE_NO_COMBOSELCHANGE
   case SSM_COMBOSELCHANGE:
#ifdef SS_USE16BITCOORDS
      Col = LOWORD(lParam);
      Row = HIWORD(lParam);
#else
      lpCellCoord = (LPSS_CELLCOORD)lParam;
      Col = lpCellCoord->Col;
      Row = lpCellCoord->Row;
#endif

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params2.lpParam1 = &Col;
      Params2.lpParam2 = &Row;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_ComboSelChange, &Params2);
      return (0);
#endif

#ifndef SSE_NO_TEXTTIPFETCH
   case SSM_TEXTTIPFETCH:
      {
      LPSS_TEXTTIPFETCH lpSSFetch = (LPSS_TEXTTIPFETCH)lParam;
      SPREADPARAMS6     Params6;
      HLSTR             hlstrText;
      LPSTR             lpszText;
      long              lWidth;

      Col = lpSSFetch->Col;
      Row = lpSSFetch->Row;

      if (Col == SS_HEADER)
         Col = 0;

      if (Row == SS_HEADER)
         Row = 0;

      Params6.lpParam1 = &Col;
      Params6.lpParam2 = &Row;
      Params6.lpParam3 = &lpSSFetch->wMultiLine;
      lWidth = VBXPixelsToTwips(lpSSFetch->nWidth);
      Params6.lpParam4 = &lWidth;

      hlstrText = VBCreateHlstr(lpSSFetch->szText, lstrlen(lpSSFetch->szText));
      Params6.lpParam5 = hlstrText;
      Params6.lpParam6 = &lpSSFetch->fShow;

      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      VBFireEvent(hCtl, SS_EVENT_TextTipFetch, &Params6);

      lpSSFetch->nWidth = VBXTwipsToPixels(lWidth);

      lpszText = VBDerefHlstr(hlstrText);
      _fmemset(lpSSFetch->szText, 0, SS_TT_TEXTMAX+1); //RAP01a
      _fmemcpy(lpSSFetch->szText, lpszText, min(VBGetHlstrLen(hlstrText),
               SS_TT_TEXTMAX));          

      if (hlstrText)
         VBDestroyHlstr(hlstrText);
      }
      return (0);
#endif
#endif // SS_V30

#ifdef SS_BOUNDCONTROL
   case SSM_POSTDATACHANGE:
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      if (lpSpread->DataOpts.fAutoSave)
			{
	      LPSPREADSHEET lpSS = SS_Lock(hWnd);
         VBSSBoundManualSave(lpSS, (LONG)hCtl, hWnd, lpSpread, 1,
                             max(lpSS->Row.DataCnt, (SS_COORD)lParam + 1), TRUE);
			SS_Unlock(hWnd);
			}
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      lpSpread->fChangeMsgPosted = FALSE;
      return (0);

   case SSM_POSTADDNEW:
	   lpSS = SS_Lock(hWnd);
      VBSSBoundProcessPostAdd(lpSS, (LONG)hCtl, hWnd, lParam);
		SS_Unlock(hWnd);
      return (0);

   case SSM_BOUNDCLICK:
	   lpSS = SS_Lock(hWnd);
      vbSpreadBoundClick(lpSS, (LONG)hCtl, ((LPSS_CELLCOORD)lParam)->Row);
		SS_Unlock(hWnd);

//      SSSetActiveCell(hWnd, ((LPSS_CELLCOORD)lParam)->Col,
//                      ((LPSS_CELLCOORD)lParam)->Row);

      return (0);
#endif

   case SSM_LOADED:
#ifdef SS_BOUNDCONTROL
#ifdef SS_QE
      if (VBGetMode() != MODE_DESIGN)
         SSDataRefresh(hCtl, hWnd);
#endif
#endif

      lpSS = SS_Lock(hWnd);
      if (lpSS->fAutoSize)
         SS_AutoSize(lpSS->lpBook, TRUE);
      SS_Unlock(hWnd);
      return (0);

#ifdef SS_V30
#ifndef SS_NOPRINT
   case PRVWM_GETINFO:
      VBSSPrintPreviewGetInfo(hCtl, hWnd, (LPPRVW_INFO)lParam);
      return (TRUE);

   case PRVWM_PRINTPAGE:
      VBSSPrintPreviewPrintPage(hCtl, hWnd, (LPPRVW_PAGE)lParam);
      return (TRUE);

   case PRVWM_SETMARGINS:
      VBSSPrintPreviewSetMargins(hCtl, hWnd, (LPPRVW_MARGINS)lParam);
      return (TRUE);
#endif
#endif

   case WM_COMMAND:
      switch (HIWORD(lParam))
         {
         case SSN_BLOCKSELECTED:
            SSGetSelectBlock(hWnd, &CellUL, &CellLR);
            Params4.lpParam1 = &CellUL.Col;
            Params4.lpParam2 = &CellUL.Row;
            Params4.lpParam3 = &CellLR.Col;
            Params4.lpParam4 = &CellLR.Row;
            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_BlockSelected, &Params4);
            return (0);

         case SSN_BLOCKCHANGED:
            SSGetSelectBlock(hWnd, &CellUL, &CellLR);
            Params6.lpParam1 = &CellUL.Col;
            Params6.lpParam2 = &CellUL.Row;
            Params6.lpParam3 = &CellLR.Col;
            Params6.lpParam4 = &CellLR.Row;

            SSGetCurrSelBlockPos(hWnd, &Col, &Row);

            Params6.lpParam5 = &Col;
            Params6.lpParam6 = &Row;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_SelChange, &Params6);
            return (0);

         case SSN_INVALIDDATA:
//            SSGetActiveCell(hWnd, &Col, &Row);

            lpSS = SS_Lock(hWnd);
            Col = lpSS->CurVisCell.Col;
            Row = lpSS->CurVisCell.Row;
            SS_Unlock(hWnd);

            Error = EDITERROR_INVALIDDATA;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &Error;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
            return (0);

         case SSN_INVALIDPASTEDATA:
            SSGetActiveCell(hWnd, &Col, &Row);

            Error = EDITERROR_INVALIDPASTEDATA;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &Error;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
            return (0);

         case SSN_TYPINGPROHIBITED:
            SSGetActiveCell(hWnd, &Col, &Row);

            Error = EDITERROR_TYPINGPROHIBITED;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &Error;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
            return (0);

         case SSN_EDITMODEINS:
            SSGetActiveCell(hWnd, &Col, &Row);

            Error = EDITERROR_EDITMODEINS;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &Error;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
            return (0);

         case SSN_CHECKBOXTYPING:
            SSGetActiveCell(hWnd, &Col, &Row);

            Error = EDITERROR_CHECKBOXTYPING;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &Error;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
            return (0);

#ifdef SS_UTP
         case SSN_INVALIDCOLTYPE:
            SSGetActiveCell(hWnd, &Col, &Row);

            Error = EDITERROR_INVALIDCOLTYPE;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &Error;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_EditError, &Params3);
            return (0);
#endif

         case SSN_EXITNEXT:
            Next = -1;
            Params1.lpParam1 = &Next;
            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_Advance, &Params1);
            return (0);

         case SSN_EXITPREV:
            Next = 0;
            Params1.lpParam1 = &Next;
            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_Advance, &Params1);
            return (0);

#ifndef SS_NOCT_BUTTON
         case SSN_BUTTONDOWN:
            {
            short ButtonDown;

            SSGetCellSendingMsg(hWnd, &Col, &Row);

            ButtonDown = -1;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &ButtonDown;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_ButtonClicked, &Params3);
            }
            return (0);

         case SSN_BUTTONUP:
            {
            short ButtonDown;

            SSGetCellSendingMsg(hWnd, &Col, &Row);

            ButtonDown = 0;
            Params3.lpParam1 = &Col;
            Params3.lpParam2 = &Row;
            Params3.lpParam3 = &ButtonDown;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_ButtonClicked, &Params3);
            }
            return (0);
#endif

#ifndef SSE_NO_USERFORMULAENTERED
         case SSN_USERFORMULA:
            SSGetActiveCell(hWnd, &Col, &Row);

            Params2.lpParam1 = &Col;
            Params2.lpParam2 = &Row;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_UserFormulaEntered, &Params2);
            return (0);
#endif

#ifndef SSE_NO_TOPLEFTCHANGE
         case SSN_TOPROWCHANGE:
            {
            SS_COORD Row2;

            SSGetTopLeftPrev(hWnd, &Col, &Row);

            lpSS = SS_Lock(hWnd);
            Col = lpSS->Col.UL;
            Row2 = lpSS->Row.UL;
            SS_Unlock(hWnd);

            Params4.lpParam1 = &Col;
            Params4.lpParam2 = &Row;
            Params4.lpParam3 = &Col;
            Params4.lpParam4 = &Row2;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_TopLeftChange, &Params4);
            }
            return (0);
#endif

#ifndef SSE_NO_TOPLEFTCHANGE
         case SSN_LEFTCOLCHANGE:
            {
            SS_COORD Col2;

            SSGetTopLeftPrev(hWnd, &Col, &Row);

            lpSS = SS_Lock(hWnd);
            Row = lpSS->Row.UL;
            Col2 = lpSS->Col.UL;
            SS_Unlock(hWnd);

            Params4.lpParam1 = &Col;
            Params4.lpParam2 = &Row;
            Params4.lpParam3 = &Col2;
            Params4.lpParam4 = &Row;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_TopLeftChange, &Params4);
            }
            return (0);
#endif

#ifndef SSE_NO_TOPLEFTCHANGE
         case SSN_TOPLEFTCHANGE:
            {
            SS_COORD Col2;
            SS_COORD Row2;

            SSGetTopLeftPrev(hWnd, &Col, &Row);

            lpSS = SS_Lock(hWnd);
            Col2 = lpSS->Col.UL;
            Row2 = lpSS->Row.UL;
            SS_Unlock(hWnd);

            Params4.lpParam1 = &Col;
            Params4.lpParam2 = &Row;
            Params4.lpParam3 = &Col2;
            Params4.lpParam4 = &Row2;

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
            VBFireEvent(hCtl, SS_EVENT_TopLeftChange, &Params4);
            }
            return (0);
#endif
         }

      return (SpreadSheetProc(hWnd, Msg, wParam, lParam));

   case WM_DESTROY:
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      lpSS = SS_Lock(hWnd);
      if (lpSS->EditModeOn)
         SS_CellEditModeOff(lpSS, (HWND)-1);
      SS_Unlock(hWnd);

#ifdef SS_QE
      if (lpSpread->DBInfo.fConnected)
         DBSS_EndBind(hWnd, &lpSpread->DBInfo);
#endif

#ifndef SS_NOPRINT
      if (lpSpread->Print.hPrintJobName)
         GlobalFree(lpSpread->Print.hPrintJobName);

      if (lpSpread->Print.hPrintAbortMsg)
         GlobalFree(lpSpread->Print.hPrintAbortMsg);

      if (lpSpread->Print.hPrintHeader)
         GlobalFree(lpSpread->Print.hPrintHeader);

      if (lpSpread->Print.hPrintFooter)
         GlobalFree(lpSpread->Print.hPrintFooter);
#endif

      if (lpSpread->hTypePicMask)
         GlobalFree(lpSpread->hTypePicMask);

      if (lpSpread->hTypePicDefText)
         GlobalFree(lpSpread->hTypePicDefText);

      if (lpSpread->hTypeComboList)
         GlobalFree(lpSpread->hTypeComboList);

      if (lpSpread->hTypeButtonText)
         GlobalFree(lpSpread->hTypeButtonText);

      if (lpSpread->hMultiSelBlocks)
         GlobalFree(lpSpread->hMultiSelBlocks);

      if (lpSpread->hCheckText)
         GlobalFree(lpSpread->hCheckText);

      if (lpSpread->hSortKeys)
         GlobalFree(lpSpread->hSortKeys);

#ifdef SS_QE
      if (lpSpread->DBInfo.hDataSelect)
         tbGlobalFree(lpSpread->DBInfo.hDataSelect);

      if (lpSpread->DBInfo.hDataConnect)
         tbGlobalFree(lpSpread->DBInfo.hDataConnect);
#endif

      if (lpSpread->hWndDesigner && IsWindow(lpSpread->hWndDesigner))
         PostAppMessage(GetWindowTask(lpSpread->hWndDesigner), WM_QUIT, 0, 0L);

      if (lpSpread->hWndPropPopup)
         DestroyWindow(lpSpread->hWndPropPopup);

      break;

// Spread Designer stuff   
   case SDM_HEREIAM:
   {
      COPYDATASTRUCT cds;
      HWND hWndSpreadDsg;   
      GLOBALHANDLE gh;
      HPBYTE hpS, hpD;
      long lCount; 
      hWndSpreadDsg = (HWND)lParam;   

      gh = SSSaveToBuffer(hWnd, (LPLONG)&(cds.lData), FALSE);     
      cds.ghData = GlobalAlloc(GHND | GMEM_SHARE, cds.lData);
      hpS = (HPBYTE)GlobalLock(gh);
      hpD = (HPBYTE)GlobalLock(cds.ghData);         
      for (lCount=0; lCount<cds.lData; lCount++)
      {
        *(hpD+lCount) = *(hpS+lCount);
      }      
      GlobalUnlock(gh);
      GlobalFree(gh);
      GlobalUnlock(cds.ghData);
      SendMessage(hWndSpreadDsg, WM_COPYDATA, 0, (LPARAM)(LPCOPYDATASTRUCT)&cds);
      GlobalFree(cds.ghData);   
      break;        
   }

   case SDM_GETCOLS:
   {
      #define ADD_OFFSET(lpv, dw)        ((LPVOID)((LPBYTE)lpv + dw))
      
      HWND hWndSpreadDsg = (HWND)lParam;    
      COPYDATASTRUCT cds;
      DWORD dwOffset=0;
//RWP04c ->      
//    GLOBALHANDLE gh;
      GLOBALHANDLE gh = 0;
//RWP04c <-      
      LPVOID lpData;
      HCTL hctlDataSource;
      BOOL bIsRegistered=TRUE;
      HWND hWndList;
      int iCount;
//RWP04a ->
	  BOOL	bRet = TRUE;

	  // Temporarily disable the hooks
     UnhookWindowsHookEx(hhookMouseFilter); 
     UnhookWindowsHookEx(hhookKeyFilter); 
//RWP04a <-

      hctlDataSource = VBGetDataSourceControl(VBGetHwndControl(hWnd), &bIsRegistered);
      if (hctlDataSource == (HCTL)NULL)
      {             
        // The control is not bound to a data source. Returning FALSE will result in a
        // warning message from Spread Designer.
//RWP04a ->
		// Re-enable the hooks
		hhookMouseFilter = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, 
                                    	(HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                                    	GetCurrentTask());
		hhookKeyFilter = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, 
                                  		(HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                                  		GetCurrentTask());
//RWP04a <-
		  return FALSE;
      } 

      // Create invisible listbox
      hWndList = CreateWindow("LISTBOX", NULL, 0L, 0, 0, 0, 0, hWnd, NULL,
                              GetWindowWord(hWnd, GWW_HINSTANCE), NULL);    

      {
      PPROPINFO pPropDataField = SpreadProperties[IPROP_SPREAD_DATAFIELD];
      SpreadProperties[IPROP_SPREAD_DATAFIELD] = PPROPINFO_STD_DATAFIELD;

      VBSendControlMsg(VBGetHwndControl(hWnd), VBM_INITPROPPOPUP,
                       IPROP_SPREAD_DATAFIELD, hWndList);
      // Backdoor call to fill the listbox
      VBSendControlMsg(VBGetHwndControl(hWnd), 0x1040, IPROP_SPREAD_DATAFIELD, hWndList);

      SpreadProperties[IPROP_SPREAD_DATAFIELD] = pPropDataField;
      }

      iCount = (int)SendMessage(hWndList, LB_GETCOUNT, 0, 0L);
      if (LB_ERR == iCount)
      {
//RWP04a ->
		// Re-enable the hooks
		hhookMouseFilter = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, 
                                    	(HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                                    	GetCurrentTask());
		hhookKeyFilter = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, 
                                  		(HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                                  		GetCurrentTask());
//RWP04a <-
        return FALSE;
	  }
      else
      {                                                           
        int i;
        int iLength;
    
        cds.lData = 0;
        for (i=0; i<iCount; i++)
        {  
          iLength = (int)SendMessage(hWndList, LB_GETTEXTLEN, i, 0L) + 1;
          if (cds.lData == 0)
          {
            cds.lData = iLength;
            gh = GlobalAlloc(GHND | GMEM_SHARE, cds.lData);
            lpData = (LPVOID)GlobalLock(gh);
          }
          else
          {
            cds.lData += iLength;
            gh = GlobalReAlloc(gh, cds.lData, GHND | GMEM_SHARE);
            lpData = (LPVOID)GlobalLock(gh); 
            lpData = ADD_OFFSET(lpData, dwOffset);
          }
          SendMessage(hWndList, LB_GETTEXT, i, (LPARAM)lpData);
          dwOffset = cds.lData;                          
          GlobalUnlock(gh); 
        }
//RWP04a ->
		// Re-enable the hooks
		hhookMouseFilter = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, 
                                   		(HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                                   		GetCurrentTask());
		hhookKeyFilter = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, 
                               			(HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                               			GetCurrentTask());
//RWP04a <-
        cds.ghData = gh;  
        {
          GLOBALHANDLE ghCDS = GlobalAlloc(GHND | GMEM_SHARE, sizeof(COPYDATASTRUCT));
          LPVOID lpCDS = (LPVOID)GlobalLock(ghCDS);
          _fmemcpy(lpCDS, (LPCOPYDATASTRUCT)&cds, sizeof(COPYDATASTRUCT));
          SendMessage(hWndSpreadDsg, WM_COPYDATA, 0, (LPARAM)(LPCOPYDATASTRUCT)lpCDS/*&cds*/);
//RWP04c ->
//---------*---------
//        GlobalFree(gh);
//---------*---------
		  if (gh)
          	GlobalFree(gh);
//RWP04c <-
          GlobalUnlock(ghCDS);
          GlobalFree(ghCDS);
          DestroyWindow(hWndList);
        }
      }
   }           
   return TRUE;
  
   case SDM_EXIT:
   {
     UnhookWindowsHookEx(hhookMouseFilter); 
     UnhookWindowsHookEx(hhookKeyFilter); 
   }
   break;
   
   default:
     if (WM_COPYDATA == Msg)
     {  
       //Load the buffer to the spreadsheet control
       COPYDATASTRUCT *pCDS = (COPYDATASTRUCT *)lParam;
       LPVBSPREAD lpSpread;
       
       if (wParam == 1) // copy format from the designer
       {
         LONG lenData;
         GLOBALHANDLE ghData = SSSaveToBuffer(hWnd, &lenData, TRUE);
         SSLoadFromBuffer(hWnd, pCDS->ghData, pCDS->lData); 
         SSLoadFromBuffer(hWnd, ghData, lenData);
         GlobalFree(ghData);
       }
       else if (wParam == 2) // copy data from the designer
       {
         SSClearData(hWnd, SS_ALLCOLS, SS_ALLROWS);
         SSLoadFromBuffer(hWnd, pCDS->ghData, pCDS->lData); 
       }  
       else 
       {
         SSLoadFromBuffer(hWnd, pCDS->ghData, pCDS->lData); 
       }  
       lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
       lpSpread->fSaveDesignerInfo = TRUE;
       SSSetBool(hWnd, SSB_REDRAW, TRUE);
//      VBDirtyForm(hCtl);
// Set a property so the control will be "Dirty", to force an update of the formfile. 
       VBSetControlProperty(hCtl, IPROP_SPREAD_SPREADDESIGNER, (long)(LPSTR)"XXX");
     } 
     break; 
   }

#ifdef SS_NOSUPERCLASS
return (SpreadSheetProc(hWnd, Msg, wParam, lParam));
#else
return (VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam));
#endif
}


/*******************************************************************************/
LONG FAR PASCAL _export vbSpreadCallBackProc(hWnd, Msg, wParam, lParam)

HWND   hWnd;
WORD   Msg;
WPARAM wParam;
LPARAM lParam;
{
HCTL   hCtl = VBGetHwndControl(hWnd);

if (hCtl)
   return (vbSpreadProc(hCtl, hWnd, Msg, wParam, lParam));

return (0);
}


short vbSSValCoords(lpSpread, lpCol, lpRow)

LPVBSPREAD lpSpread;
LPSS_COORD lpCol;
LPSS_COORD lpRow;
{
if (lpCol)
   if (*lpCol != SS_HEADER && (*lpCol < -1 || *lpCol > lpSpread->lMaxCols))
      return (vbSpreadSetError(ERR_RANGE_COL));

if (lpRow)
   if (*lpRow != SS_HEADER && (*lpRow < -1 || *lpRow > lpSpread->lMaxRows))
      return (vbSpreadSetError(ERR_RANGE_ROW));

return (0);
}


short vbSpreadSetError(nErrNum)

short nErrNum;
{
LPSTR lpszErrMsg;

switch (nErrNum)
   {
   case ERR_RANGE_COL:
      lpszErrMsg = ERRMSG_RANGE_COL;
      break;

   case ERR_RANGE_ROW:
      lpszErrMsg = ERRMSG_RANGE_ROW;
      break;
   }

return (VBSetErrorMessage(nErrNum, lpszErrMsg));
}


BOOL vbSpreadFireQueryAdvance(HCTL hCtl, short dNext)
{
#ifndef SSE_NO_QUERYADVANCE

LPVBSPREAD    lpSpread;
SPREADPARAMS2 Params2;
short         dCancel;

#ifdef SS_QUERYADVANCECANCEL
dCancel = 0;
#else
dCancel = -1;
#endif

if (dNext)
   dNext = -1;

Params2.lpParam1 = &dNext;
Params2.lpParam2 = &dCancel;
lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
VBFireEvent(hCtl, SS_EVENT_QueryAdvance, &Params2);

if (!dCancel)
   return (FALSE);

#endif

return (TRUE);
}


#ifdef SS_BOUNDCONTROL

BOOL vbSpreadSaveBoundRow(LPSPREADSHEET lpSS, HCTL hCtl, HWND hWnd, SS_COORD Row, SS_COORD RowNew,
                          SS_COORD ColNew)
{
LPVBSPREAD    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
LPSS_ROW      lpRow;
BOOL          fAdd;
BOOL          fCancel = FALSE;

#ifdef SS_QE
if (lpSpread->DBInfo.fConnected && lpSpread->DataOpts.fAutoSave)
   DBSS_ManualSave(hCtl, hWnd, lpSpread, lpSS, Row, Row);
#endif

if (VBSSBoundIsBound((LONG)hCtl))
   {
   fAdd = FALSE;
   if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      if ((lpRow->bDirty == SS_ROWDIRTY_SOME ||
           lpRow->bDirty == SS_ROWDIRTY_ALL) && !lpRow->hlstrBookMark &&
           lpSpread->DataOpts.fAutoSave)
         {
         VBSSBoundAdd(lpSS, (LONG)hCtl, hWnd, Row, Row, FALSE);
         fAdd = TRUE;
//         fCancel = TRUE;
         }

      SS_UnlockRowItem(lpSS, Row);
      }

   if (fAdd)
      {
      lpSpread->PostAddRowNew = (RowNew == -1 ? Row : RowNew);
      lpSpread->PostAddRowNewCol = (ColNew == -1 ? lpSS->Col.CurAt : ColNew);
      }
   else if (lpSpread->DataOpts.fAutoDataInform && RowNew != -1)
      vbSpreadBoundClick(lpSS, (LONG)hCtl, RowNew);
   }

return (fCancel);
}
#endif


void FAR PASCAL _export SpreadSaveDesignInfo(HWND hWndDest, HWND hWndSrc,
                                             BOOL fInit)
{
/*
GLOBALHANDLE hBuffer;
long         lBufferLen;
*/

if (IsWindow(hWndSrc) && IsWindow(hWndDest))
   {
   if (!fInit)
      SetFocus(hWndDest);

   SendMessage(hWndSrc, SSM_SAVEINFOTODESIGNER, (WPARAM)fInit,
               MAKELONG(hWndSrc, hWndDest));

#if 0
   if (hBuffer = SSSaveToBuffer(hWndSrc, &lBufferLen, FALSE))
      if (fInit)
         SendMessage(hWndDest, SSM_GETINFOFROMDESIGNER, hBuffer, lBufferLen);
      else
         PostMessage(hWndDest, SSM_GETINFOFROMDESIGNER, hBuffer, lBufferLen);
#endif
   }
}


#ifdef SS_V21
void vbSpreadRunDesigner(HWND hWnd)
{
#ifdef SS_IDNAME

HANDLE hModDesigner;
char   Buffer[20];

HourGlassOn();

wsprintf(Buffer, "%s %u", (LPSTR)SS_IDNAME, hWnd);

hModDesigner = WinExec(Buffer, SW_SHOWNORMAL);
HourGlassOff();

#endif
}

#else //!SS_V21

void vbSpreadRunDesigner(HWND hWnd)
{
#ifdef SS_SDNAME

#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND   2
#endif

#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND   3
#endif

char   NameBuffer[50];
char   CmdLineBuffer[50];
UINT   rc;
 
HourGlassOn();   
// Reset the SpreadDesigner property to xxx -- after an apply, SpreadDesigner is set to XXX
// to make the control "Dirty" so the modified properties are saved to the formfile.
VBSetControlProperty(VBGetHwndControl(hWnd), IPROP_SPREAD_SPREADDESIGNER, (long)(LPSTR)"xxx");
VBGetControlName(VBGetHwndControl(hWnd), NameBuffer);
wsprintf(CmdLineBuffer, "%s %u %s", (LPSTR)SS_SDNAME, hWnd, NameBuffer);
//RWP03m:001 ->
//HourGlassOff();
//RWP03m:001 <-

hhookMouseFilter = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, 
                                    (HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                                    GetCurrentTask());
hhookKeyFilter = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, 
                                  (HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE),
                                  GetCurrentTask());
rc = WinExec(CmdLineBuffer, SW_SHOWNORMAL);
//RWP02c ->
//---------*---------
//if ((rc == ERROR_FILE_NOT_FOUND) || (rc == ERROR_PATH_NOT_FOUND))
//{
//  UnhookWindowsHookEx(hhookMouseFilter);  
//  UnhookWindowsHookEx(hhookKeyFilter);    
//  MessageBeep((UINT)-1);
//}
//---------*---------
if (rc < 32)
{
  UnhookWindowsHookEx(hhookMouseFilter);  
  UnhookWindowsHookEx(hhookKeyFilter);    
  MessageBeep((UINT)-1);
}
//RWP02c <-

//RWP03m:001 ->
HourGlassOff();
//RWP03m:001 <-

#endif //SS_SDNAME
}
#endif //SS_V21


void vbSpreadSetUtopiaModeOn(HWND hWnd, LPVBSPREAD lpSpread, BOOL fValue)
{
#ifdef SS_UTP
LPSPREADSHEET lpSS;

lpSS = SS_Lock(hWnd);

if (fValue && !lpSpread->fUtopiaMode)
   {
   BOOL fRedraw = lpSS->Redraw;
   lpSS->Redraw                  = FALSE;

   lpSpread->fUtopiaMode         = TRUE;
   lpSS->fSmartPrint             = TRUE;
   lpSS->fAllowEditModePermSel   = TRUE;
   lpSS->fNoShowActiveWithSel    = TRUE;
   lpSS->fUseScrollArrows        = TRUE;
   lpSS->fUseEditModeShadow      = TRUE;
   lpSS->fValidateClipboardPaste = TRUE;
   lpSS->fProhibitTypingWhenSel  = TRUE;
   lpSS->fAllowUndo              = TRUE;
   lpSS->fPrintExpandMultiLine   = TRUE;

   lpSS->dVKeyIns = VK_INSERT;
   lpSS->dVKeyDel = VK_DELETE;

   lpSS->wEnterAction = SS_ENTERACTION_NEXTROW;

   SetToolBoxGlobal(TBG_CLEARFIELD, 0);
   SetToolBoxGlobal(TBG_CURRENTDATETIME, 0);
   SetToolBoxGlobal(TBG_POPUPCALENDAR, 0);

   SSSetBool(hWnd, SSB_HORZSCROLLBAR, FALSE);
   SSSetBool(hWnd, SSB_VERTSCROLLBAR, FALSE);
   SSSetBool(hWnd, SSB_PROCESSTAB, TRUE);
   SSSetBool(hWnd, SSB_EDITMODEPERMANENT, TRUE);
   SSSetBool(hWnd, SSB_EDITMODEREPLACE, TRUE);

   vbSpreadSetScrollArrows(hWnd, lpSS);
   SSSetActiveCell(hWnd, lpSS->Col.HeaderCnt + lpSS->Col.Frozen,
                   lpSS->Row.HeaderCnt + lpSS->Row.Frozen);

   SSSetCursor(hWnd, SS_CURSORTYPE_BUTTON, SS_CURSOR_ARROW);
   /*
   SSSetCursor(hWnd, SS_CURSORTYPE_BUTTON | SS_CURSORTYPE_GRAYAREA |
               SS_CURSORTYPE_LOCKEDCELL | SS_CURSORTYPE_COLHEADER |
               SS_CURSORTYPE_ROWHEADER, SS_CURSOR_ARROW);
   */

   SSSetGrayAreaColor(hWnd, RGBCOLOR_WHITE, RGBCOLOR_WHITE);

   SSSetUserResize(hWnd, SS_USERRESIZE_COL);
   SSSetBool(hWnd, SSB_REDRAW, fRedraw);
   }

SS_Unlock(hWnd);
#endif
}


#ifdef SS_UTP
void vbSpreadSetScrollArrows(HWND hWnd, LPSPREADSHEET lpSS)
{
SUPERBTNCOLOR SuperBtnColor;
SS_CELLTYPE   CellType;
COLORREF      Color;
SS_COORD      Col;
HICON         hIconUp = 0;
HICON         hIconDown = 0;
WORD          wIconUpType = 0;
WORD          wIconDownType = 0;

if (lpSS->fUseScrollArrows)
   {
   if (!lpSS->hBitmapScrollArrowTopDef)
      lpSS->hBitmapScrollArrowTopDef = LoadBitmap(hDynamicInst, "ArrowUp");

   if (!lpSS->hBitmapScrollArrowBottomDef)
      lpSS->hBitmapScrollArrowBottomDef = LoadBitmap(hDynamicInst, "ArrowDown");

   if (!lpSS->hBitmapScrollArrowLeftDef)
      lpSS->hBitmapScrollArrowLeftDef = LoadBitmap(hDynamicInst, "ArrowLeft");

   if (!lpSS->hBitmapScrollArrowRightDef)
      lpSS->hBitmapScrollArrowRightDef = LoadBitmap(hDynamicInst, "ArrowRight");

   lpSS->hBitmapScrollArrowTopUp = lpSS->hBitmapScrollArrowTopDef;
   lpSS->hBitmapScrollArrowBottomUp = lpSS->hBitmapScrollArrowBottomDef;
   lpSS->hBitmapScrollArrowLeftUp = lpSS->hBitmapScrollArrowLeftDef;
   lpSS->hBitmapScrollArrowRightUp = lpSS->hBitmapScrollArrowRightDef;
   lpSS->uScrollArrowTopUpType = BT_BITMAP;
   lpSS->uScrollArrowBottomUpType = BT_BITMAP;
   lpSS->uScrollArrowLeftUpType = BT_BITMAP;
   lpSS->uScrollArrowRightUpType = BT_BITMAP;

   /****************************
   * Set up Left Scroll Column
   ****************************/

   Col = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;

   SSSetTypeStaticText(hWnd, &CellType, 0);
   SSSetCellType(hWnd, Col, -1, &CellType);

   /*
   SSSetLock(hWnd, Col, -1, TRUE);
   SSSetLock(hWnd, Col, 0, FALSE);
   */

   SSSetBorder(hWnd, Col, -1, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM,
               SS_BORDERSTYLE_DOT, lpSS->GridColor);
   /*
   SSSetBorder(hWnd, Col, 0, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_BLANK,
               lpSS->GridColor);
   */
   SSSetBorder(hWnd, Col, 0, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID,
               lpSS->GridColor);
   SSSetBorder(hWnd, Col - 1, 0, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID,
               lpSS->GridColor);
   SSSetBorder(hWnd, Col + 1, 0, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID,
               lpSS->GridColor);

   SSGetColor(hWnd, -1, -1, &Color, NULL);

   SuperBtnColor.Color = Color;
   SuperBtnColor.ColorBorder = Color;
   SuperBtnColor.ColorShadow = Color;
   SuperBtnColor.ColorHighlight = Color;
   SuperBtnColor.ColorText = Color;

   hIconUp = lpSS->hBitmapScrollArrowLeftUp;
   if (hIconUp)
      wIconUpType = lpSS->uScrollArrowLeftUpType | BT_HANDLE;

   hIconDown = lpSS->hBitmapScrollArrowLeftDown;
   if (hIconDown)
      wIconDownType = lpSS->uScrollArrowLeftDownType | BT_HANDLE;

   SSSetTypeButton(hWnd, &CellType, 0L, NULL,
                   (LPSTR)(hIconUp ? &hIconUp : NULL), wIconUpType,
                   (LPSTR)(hIconDown ? &hIconDown : NULL), wIconDownType,
                   SUPERBTN_TYPE_NORMAL, 0, &SuperBtnColor);
   SSSetCellType(hWnd, Col, 0, &CellType);
   SSSetColWidthInPixels(hWnd, Col, lpSS->dScrollArrowWidth);

   SSSetFreeze(hWnd, 1, 0);

   SSShowCol(hWnd, lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1, FALSE);
   }
}


void FAR PASCAL _export SpreadSetColType(HCTL hCtl, SS_COORD Col,
                                         short ColType)
{
LPSPREADSHEET lpSS;
SS_CELLTYPE   CellType;
LPSS_COL      lpCol;
HWND          hWnd;

hWnd = VBGetControlHwnd(hCtl);
lpSS = SS_Lock(hWnd);

SSSetUserResizeCol(hWnd, Col, SS_RESIZE_DEFAULT);

switch (ColType)
   {
   case 0:                             // Few Words
      SSSetColWidthInPixels(hWnd, Col, lpSS->StandardFontWidth * 15);
      SSSetTypeEdit(hWnd, &CellType, ES_LEFT | ES_AUTOHSCROLL, 60, 0, 1);
      SSSetCellType(hWnd, Col, -1, &CellType);
      break;

   case 1:                             // Lots Of Words
      SSSetColWidthInPixels(hWnd, Col, lpSS->StandardFontWidth * 35);
      SSSetTypeEdit(hWnd, &CellType, ES_LEFT | ES_AUTOHSCROLL, 10000, 0, 1);
      SSSetCellType(hWnd, Col, -1, &CellType);
      break;

   case 2:                             // Numbers
      SSSetColWidthInPixels(hWnd, Col, lpSS->StandardFontWidth * 10);
      SSSetTypeFloat(hWnd, &CellType, ES_RIGHT | FS_SEPARATOR, 8, 6,
                     -99999999.99, 99999999.99);
      SSSetCellType(hWnd, Col, -1, &CellType);
      break;

   case 3:                             // Currency
      // RFW 7/26/94
      SSSetColWidthInPixels(hWnd, Col, lpSS->StandardFontWidth * 11);
      SSSetTypeFloat(hWnd, &CellType, ES_RIGHT | FS_SEPARATOR | FS_MONEY, 8,
                     2, -99999999.99, 99999999.99);
      SSSetCellType(hWnd, Col, -1, &CellType);
      break;

   case 4:                             // Date
      SSSetColWidthInPixels(hWnd, Col, lpSS->StandardFontWidth * 10);
      SSSetTypeDate(hWnd, &CellType, 0, NULL, NULL, NULL);
      SSSetCellType(hWnd, Col, -1, &CellType);
      break;

   case 5:                             // Non-Selectable
      SSSetTypeStaticText(hWnd, &CellType, SS_TEXT_LEFT | SS_TEXT_TOP);
      SSSetCellType(hWnd, Col, -1, &CellType);

      if (lpCol = SS_AllocLockCol(lpSS, Col))
         {
         lpCol->fNonSelectable = TRUE;
         SS_UnlockColItem(lpSS, Col);
         }

      break;

   case 6:                             // CheckBox
      SSSetTypeCheckBox(hWnd, &CellType, BS_CENTER, NULL, NULL, 0, NULL, 0,
                        NULL, 0, NULL, 0, NULL, 0, NULL, 0);
      SSSetCellType(hWnd, Col, -1, &CellType);

      SSSetUserResizeCol(hWnd, Col, SS_RESIZE_OFF);
      if (lpCol = SS_AllocLockCol(lpSS, Col))
         {
         lpCol->fNonSelectable = TRUE;
         SS_UnlockColItem(lpSS, Col);
         }

      break;
   }

SS_Unlock(hWnd);
}


short FAR PASCAL _export SpreadGetColType(HCTL hCtl, SS_COORD Col)
{
SS_CELLTYPE   CellType;
short         ColType;
HWND          hWnd;

hWnd = VBGetControlHwnd(hCtl);

SSGetCellType(hWnd, Col, -1, &CellType);

switch (CellType.Type)
   {
   case SS_TYPE_DATE:
      ColType = 4;
      break;

   case SS_TYPE_EDIT:
      if (CellType.Spec.Edit.Len == 60)
         ColType = 0;
      else
         ColType = 1;

      break;

   case SS_TYPE_FLOAT:
      if (CellType.Style & FS_MONEY)
         ColType = 3;
      else
         ColType = 2;

      break;

   case SS_TYPE_STATICTEXT:
      ColType = 5;
      break;

   case SS_TYPE_CHECKBOX:
      ColType = 6;
      break;

   default:
      ColType = -1;
      break;
   }

return (ColType);
}


void FAR PASCAL _export SpreadSetScrollArrows(HCTL hCtl,
                        HANDLE hTopUp, WORD wTopUpType,
                        HANDLE hTopDown, WORD wTopDownType,
                        HANDLE hBottomUp, WORD wBottomUpType,
                        HANDLE hBottomDown, WORD wBottomDownType,
                        HANDLE hLeftUp, WORD wLeftUpType,
                        HANDLE hLeftDown, WORD wLeftDownType,
                        HANDLE hRightUp, WORD wRightUpType,
                        HANDLE hRightDown, WORD wRightDownType)
{
LPSPREADSHEET lpSS;
HWND          hWnd;

hWnd = VBGetControlHwnd(hCtl);
lpSS = SS_Lock(hWnd);

lpSS->hBitmapScrollArrowTopUp = hTopUp;
lpSS->uScrollArrowTopUpType = wTopUpType;
lpSS->hBitmapScrollArrowTopDown = hTopDown;
lpSS->uScrollArrowTopUpType = wTopDownType;

lpSS->hBitmapScrollArrowBottomUp = hBottomUp;
lpSS->uScrollArrowBottomUpType = wBottomUpType;
lpSS->hBitmapScrollArrowBottomDown = hBottomDown;
lpSS->uScrollArrowBottomUpType = wBottomDownType;

lpSS->hBitmapScrollArrowLeftUp = hLeftUp;
lpSS->uScrollArrowLeftUpType = wLeftUpType;
lpSS->hBitmapScrollArrowLeftDown = hLeftDown;
lpSS->uScrollArrowLeftUpType = wLeftDownType;

lpSS->hBitmapScrollArrowRightUp = hRightUp;
lpSS->uScrollArrowRightUpType = wRightUpType;
lpSS->hBitmapScrollArrowRightDown = hRightDown;
lpSS->uScrollArrowRightUpType = wRightDownType;

SS_Unlock(hWnd);
}


void FAR PASCAL _export SpreadSetPrinter(HCTL hCtl, LPSTR lpszDevice,
                                         LPSTR lpszDriver, LPSTR lpszOutput)
{
lstrcpy(szUtpDevice, lpszDevice);
lstrcpy(szUtpDriver, lpszDriver);
lstrcpy(szUtpOutput, lpszOutput);
}
#endif


#ifdef SS_QE
void SSDataRefresh(HCTL hCtl, HWND hWnd)
{
LPSPREADSHEET lpSS;
LPVBSPREAD    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
LPSS_COL      lpCol;
SS_COORD      i;
int           iRet;

if (lpSpread->DBInfo.hDataConnect)
   {
   LPSTR lpszDataConnect;
   LPSTR lpszDataSelect;

   lpSS = SS_Lock(hWnd);

   for (i = 1; i < lpSS->Col.AllocCnt; i++)
      if (lpCol = SS_LockColItem(lpSS, i))
         {
         if (lpSS->Row.DataCnt > 1 && lpCol->dFieldNum)
            SSClearDataRange(hWnd, i, -1, i, -1);

         lpCol->dFieldNum = 0;

         SS_UnlockColItem(lpSS, i);
         }

   lpszDataConnect = (LPSTR)tbGlobalLock(lpSpread->DBInfo.hDataConnect);

   if (lpSpread->DBInfo.hDataSelect)
      lpszDataSelect = (LPSTR)tbGlobalLock(lpSpread->DBInfo.hDataSelect);
   else
      lpszDataSelect = NULL;

   iRet = DBSS_BindDatabase(hWnd, lpSpread, &lpSpread->DBInfo, lpszDataConnect,
                            lpszDataSelect);

   tbGlobalUnlock(lpSpread->DBInfo.hDataConnect);

   if (iRet && lpSS->fVirtualMode)
      SS_VRefreshBuffer(hWnd, lpSS, TRUE);

   SS_Unlock(hWnd);
   }
}

#endif


void vbSSRegisterPopupProc(void)
{
WNDCLASS wc;

wc.style = 0;
wc.lpfnWndProc = (WNDPROC)vbSSPopupProc;
wc.cbClsExtra = 0;
wc.cbWndExtra = 2;
wc.hInstance = hDynamicInst;
wc.hIcon = 0;
wc.hCursor = 0;
wc.hbrBackground = 0;
wc.lpszMenuName = 0;
wc.lpszClassName = CLASS_POPUP;

RegisterClass(&wc);
}


long _export FAR PASCAL vbSSPopupProc(HWND hWnd, WORD Msg, WORD wParam,
                                      LONG lParam)
{
switch (Msg)
   {
   case WM_DESTROY:
      break;

   case WM_SHOWWINDOW:
      if (wParam)
         {
         PostMessage(hWnd, SM_SHOWPOPUP, 0, 0L);
         return 0L;
         }

      break;

   case SM_SHOWPOPUP:
      switch (GetWindowWord(hWnd, 0))
         {
#ifdef IPROP_SPREAD_SPREADDESIGNER
         case IPROP_SPREAD_SPREADDESIGNER:
            vbSpreadRunDesigner(GetParent(hWnd));
            break;
#endif

#ifdef SS_QE
         case IPROP_SPREAD_DATACONNECT:
            {
            LPVBSPREAD     lpSpread;
            TBGLOBALHANDLE hDataConnect;
            LPSTR          lpszDataConnect;
            HCTL           hCtl;

            hCtl = VBGetHwndControl(GetParent(hWnd));
            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

            if (DBSS_QueryConnect(hWnd))
               {
               lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
               if (lpSpread->DBInfo.hDataConnect)
                  {
                  hDataConnect = lpSpread->DBInfo.hDataConnect;
                  lpSpread->DBInfo.hDataConnect = 0;
                  lpszDataConnect = (LPSTR)tbGlobalLock(hDataConnect);
                  VBSetControlProperty(hCtl, IPROP_SPREAD_DATACONNECT,
                                       (long)lpszDataConnect);
                  tbGlobalUnlock(hDataConnect);
                  tbGlobalFree(hDataConnect);
                  }
               else
                  VBSetControlProperty(hCtl, IPROP_SPREAD_DATASELECT, 0L);
               }
            }

            break;

         case IPROP_SPREAD_DATASELECT:
            {
            LPVBSPREAD     lpSpread;
            TBGLOBALHANDLE hDataSelect;
            LPSTR          lpszDataSelect;
            HCTL           hCtl;

            hCtl = VBGetHwndControl(GetParent(hWnd));
            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

            DBSS_PromptQueryString(GetParent(hWnd), &lpSpread->DBInfo);

            if (lpSpread->DBInfo.hDataSelect)
               {
               hDataSelect = lpSpread->DBInfo.hDataSelect;
               lpSpread->DBInfo.hDataSelect = 0;
               lpszDataSelect = (LPSTR)tbGlobalLock(hDataSelect);
               VBSetControlProperty(hCtl, IPROP_SPREAD_DATASELECT,
                                    (long)lpszDataSelect);
               tbGlobalUnlock(hDataSelect);
               tbGlobalFree(hDataSelect);
               }
            else
               VBSetControlProperty(hCtl, IPROP_SPREAD_DATASELECT, 0L);
            }

            break;
#endif

         case IPROP_SPREAD_ABOUT:
            VBSS_About(hWnd);
            break;
         }

      return (0);
   }

return DefWindowProc(hWnd, Msg, wParam, lParam);
}


void VBSS_About(HWND hWndSS)
{
//VBDialogBoxParam(GetWindowWord(hWndSS, GWW_HINSTANCE), "AboutBox",
//                 (FARPROC)VBSS_DlgAboutProc, 0);
  PURCHASEINFO purchaseInfo;
  HWND         hWndAbout;
  
#ifndef CS_OFF
  _fstrcpy(purchaseInfo.productKey, SS_SECURE_PRODUCT_NAME);
  purchaseInfo.productID = SS_SECURE_PRODUCT_CODE;

  SoftwareSecureGetSerialNumber(purchaseInfo.productKey, purchaseInfo.serialNumber, 
                              purchaseInfo.productID, FALSE);
#else
  _fmemset(&purchaseInfo, '\0', sizeof(PURCHASEINFO));
#endif

  hWndAbout = (HWND)fpInitAboutPopUp(hWndSS, (LPPURCHASEINFO)&purchaseInfo);
  
  // for some reason, spread does not automatically show the about box, so the next line is necessary
  
  ShowWindow(hWndAbout, SW_SHOWNORMAL);
}

/*
BOOL FAR PASCAL _export VBSS_DlgAboutProc(HWND hWnd, WORD Msg, WPARAM wParam,
                                          LPARAM lParam)
{
static HBRUSH hBrush;

switch (Msg)
   {
   case WM_INITDIALOG:
      DlgBoxCenter(hWnd);
#ifdef SS_GRID
//      hBrush = CreateSolidBrush(RGBCOLOR_YELLOW);
      hBrush = CreateSolidBrush(RGBCOLOR_PALEGRAY);
#else
      hBrush = CreateSolidBrush(RGBCOLOR_PALEGRAY);
#endif
      return (TRUE);

   case WM_CTLCOLOR:
      if (HIWORD(lParam) == CTLCOLOR_DLG)
         return (hBrush);

      else if (HIWORD(lParam) == CTLCOLOR_STATIC)
         {
//
//#ifdef SS_GRID
//         SetTextColor((HDC)wParam, RGBCOLOR_BLACK);
//         SetBkColor((HDC)wParam, RGBCOLOR_YELLOW);
//#else
//
//         SetTextColor((HDC)wParam, RGBCOLOR_DARKRED);
         SetTextColor((HDC)wParam, RGBCOLOR_BLACK);
         SetBkColor((HDC)wParam, RGBCOLOR_PALEGRAY);
//#endif
         return (hBrush);
         }

      return (0);

   case WM_DESTROY:
      DeleteObject(hBrush);
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
void VBSS_WinHelp(HWND hWnd, long lIndex)
{
#ifdef SS_HELPFILENAME
LPSTR lpszHelpFileName = SS_HELPFILENAME;
#else
LPSTR lpszHelpFileName = NULL;
#endif

if (lpszHelpFileName)
   WinHelp(hWnd, lpszHelpFileName, HELP_CONTEXT, lIndex);
}


// RFW - 8/3/94

void VBSS_SetMax(HWND hWnd)
{
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));

lpSpread->lMaxCols = SSGetMaxCols(hWnd);
lpSpread->lMaxRows = SSGetMaxRows(hWnd);
}


#ifdef  BUGS
// Bug-017
BOOL    GetSpreadAutoFill(HWND hWnd)
{
LPVBSPREAD     lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
return lpSpread->DataOpts.fAutoFill;

}
#endif

LRESULT CALLBACK SD_MouseFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
    if (code < 0)
        return CallNextHookEx(hhookMouseFilter, code, wParam, lParam);
  return (LRESULT)TRUE;
}

LRESULT CALLBACK SD_KeyFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
    if (code < 0)
        return CallNextHookEx(hhookKeyFilter, code, wParam, lParam);
  return (LRESULT)TRUE;
}
