/*    PortTool v2.2     FPTOOL44.C          */

//
// FarPoint DrawText
//

#include "fptools.h"
#include "string.h"

//FPGLOBALDATAMANAGER dmLineTable = { 0 };


void FPLIB fpInitTextOut(HWND hWnd)
{          
   HINSTANCE hInstance;
   FPCONTROL    fpTemp = {hWnd, 0, 0L, 0L};

#ifdef FP_OCX
   hInstance = fpGetInstanceHandle((LPFPCONTROL)&fpTemp);   
#else       
   hInstance = GETGW_HINSTANCE(hWnd);
#endif   

//   fpCreateGlobalData(&dmLineTable, hInstance, sizeof(FPLINEDATA));
   fpGetFontTable();
}

void FPLIB fpInitTextOutEx(HINSTANCE hInst, HWND hWnd)
{          
   FPCONTROL    fpTemp = {hWnd, 0, 0L, 0L};

//   fpCreateGlobalData(&dmLineTable, hInst, sizeof(FPLINEDATA));
   fpGetFontTableEx(hInst);
}

void FPLIB fpKillTextOut(HWND hWnd)
{
   HINSTANCE hInstance;
   FPCONTROL    fpTemp = {hWnd, 0, 0L, 0L};

#ifdef FP_OCX
   hInstance = fpGetInstanceHandle((LPFPCONTROL)&fpTemp);   
#else       
   hInstance = GETGW_HINSTANCE(hWnd);
#endif     

//   fpDestroyGlobalData (&dmLineTable, hInstance, NULL);
   fpReleaseFontTable();
}

void FPLIB fpKillTextOutEx(HINSTANCE hInst, HWND hWnd)
{
   FPCONTROL    fpTemp = {hWnd, 0, 0L, 0L};

//   fpDestroyGlobalData (&dmLineTable, hInst, NULL);
   fpReleaseFontTableEx(hInst);
}

