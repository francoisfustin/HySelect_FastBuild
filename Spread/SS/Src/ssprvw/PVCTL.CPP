// pvctl.cpp : Implementation of the CPvCtrl OLE control class.

#include "..\ssocx\stdafx.h"
#include <afxctl.h>         // MFC support for OLE Custom Controls
#ifdef WIN32			// only needed in 32-bit OCX -SCL 11/6/96
#include "comcat.h"             // ActiveX Component Categories - SCL
// ActiveX Component Categories - SCL
/////////////////////////////////////////////////////////
//const IID IID_ICatRegister = {0x0002E012,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
const CATID CATID_SafeForScripting		= {0x7dd95801,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};
const CATID CATID_SafeForInitializing	= {0x7dd95802,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};
HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription);
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid);
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid);
/////////////////////////////////////////////////////////
// ActiveX Component Categories - SCL
#endif

// property categories -scl
#ifdef SS_V40
#include "propcat.h"
#endif

#include "fptools.h"
#include "fpxprop.h"
#include "ssprvw.h"
#include "pvctl.h"
//#include "pvppg.h"
#include "..\ssprvwpp\prvppgs.h"    // prvctl custom prop pages -scl
#include "msstkppg.h"   // stock ATL prop pages
#include "..\ssocx\resource.h"

// CODESENT  
#ifndef CS_OFF
#include "..\..\..\fplibs\codesent\src\codesent.h"
#include "..\..\..\fplibs\codesent\src\csentdef.h"
#endif

#if defined(FP_SPLASHEXPIRE)
extern "C" {
BOOL WINAPI _export fpSplashDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
}
#endif

void PV_DrawDesignTime(HWND hWnd, CDC* pdc, const CRect& rcBounds);

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

#define _APS_NEXT_RESOURCE_VALUE        201
#define _APS_NEXT_CONTROL_VALUE         201
#define _APS_NEXT_SYMED_VALUE           101
#define _APS_NEXT_COMMAND_VALUE         32768

#define FPMYDISP_CTRL_NAME  CPvCtrl                     //MYDISP


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

LPCTSTR idCursor[MAX_CURSOR_INDEX + 1] =
  {
  (LPCTSTR)IDC_ARROW,
  (LPCTSTR)IDC_ARROW,
  (LPCTSTR)IDC_CROSS,
  (LPCTSTR)IDC_IBEAM,
  (LPCTSTR)IDC_ICON,
#ifdef WIN32
  (LPCTSTR)IDC_SIZEALL,
#else
  (LPCTSTR)IDC_SIZE,
#endif
  (LPCTSTR)IDC_SIZENESW,
  (LPCTSTR)IDC_SIZENS,
  (LPCTSTR)IDC_SIZENWSE,
  (LPCTSTR)IDC_SIZEWE,
  (LPCTSTR)IDC_UPARROW,
  (LPCTSTR)IDC_WAIT,
  (LPCTSTR)IDC_NO,
  (LPCTSTR)IDC_APPSTARTING,
  (LPCTSTR)IDC_HELP,
  (LPCTSTR)IDC_SIZEALL
  };


IMPLEMENT_DYNCREATE(CPvCtrl, COleControl)

// Special WM_PAINT message handler that includes the HDC

#define ON_WM_PAINT_SPECIAL() \
	{ WM_PAINT, 0, 0, 0, AfxSig_vD, \
		(AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(CDC*))OnPaint },


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CPvCtrl, COleControl)
	//{{AFX_MSG_MAP(CPvCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
// fix for #8705 -scl
  ON_MESSAGE(OCM_COMMAND, OnOcmCommand)  
	//}}AFX_MSG_MAP
#ifndef _WIN32
    ON_STDOLEVERB(OLEIVERB_PRIMARY, OnVerbPrimary)
    ON_STDOLEVERB(OLEIVERB_UIACTIVATE, OnVerbUiActivate)
    ON_STDOLEVERB(OLEIVERB_INPLACEACTIVATE, OnVerbInPlaceActivate)
#endif
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

#if VC8
BEGIN_DISPATCH_MAP(CPvCtrl, COleControl)
#else
FP_BEGIN_DISPATCH_MAP(CPvCtrl, COleControl)
#endif
	//{{AFX_DISPATCH_MAP(CPvCtrl)
	// NOTE - ClassWizard will add and remove dispatch map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
    DISP_PROPERTY_NOTIFY_ID(CPvCtrl, "MousePointer", dispidMousePointer, m_enumMousePointer, OnMousePointerChanged, VT_I2)
    DISP_PROPERTY_EX_ID(CPvCtrl, "MouseIcon", dispidMouseIcon, GetMouseIcon, SetMouseIcon, VT_PICTURE)
#ifdef FP_OLEDRAG
    DISP_PROPERTY_EX_ID(CPvCtrl, "OLEDropMode", dispidOLEDropMode, GetOLEDropMode, SetOLEDropMode, VT_I2)
    DISP_FUNCTION_ID(CPvCtrl, "OLEDrag", dispidOLEDrag, OLEDrag, VT_EMPTY, VTS_NONE)
#endif
#ifdef SS_V35
    DISP_PROPERTY_EX_ID(CPvCtrl, "ScriptEnhanced", dispidScriptEnhanced, GetScriptEnhanced, SetScriptEnhanced, VT_BOOL)
#endif //SS_V35
    DISP_STOCKPROP_HWND()
	DISP_STOCKPROP_ENABLED()
	DISP_STOCKPROP_BORDERSTYLE()

	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CPvCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
#ifdef VC8
END_DISPATCH_MAP()
#else
FP_END_DISPATCH_MAP()
#endif

/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CPvCtrl, COleControl)
	//{{AFX_EVENT_MAP(CPvCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	EVENT_CUSTOM("PageChange", FirePageChange,  VTS_PI4)
    EVENT_CUSTOM("Zoom", FireZoom,  VTS_NONE)
#ifdef FP_OLEDRAG
#include "..\..\..\fplibs\fptools\src\dragemap.cpp"
#endif // FP_OLEDRAG
	EVENT_STOCK_CLICK()
	EVENT_STOCK_DBLCLICK()
	EVENT_STOCK_KEYDOWN()
	EVENT_STOCK_KEYPRESS()
	EVENT_STOCK_KEYUP()
	EVENT_STOCK_MOUSEDOWN()
	EVENT_STOCK_MOUSEMOVE()
	EVENT_STOCK_MOUSEUP()

	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
//BEGIN_PROPPAGEIDS(CPvCtrl, 1)
//	PROPPAGEID(CPvPropPage::guid)
//END_PROPPAGEIDS(CPvCtrl)
BEGIN_PROPPAGEIDS(CPvCtrl, 4)
	PROPPAGEID(CLSID_PPGeneral)
	PROPPAGEID(CLSID_PPPage)
	PROPPAGEID(CLSID_StockColorPage)
	PROPPAGEID(CLSID_StockPicturePage)
END_PROPPAGEIDS(CPvCtrl)

/////////////////////////////////////////////////////////////////////////////
//  Interface Map
#define FPOLEMAP_CTRL_CLSNAME  CPvCtrl                  //MYDISP
BEGIN_INTERFACE_MAP(CPvCtrl, COleControl)
#ifndef VC8
  INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IDispatch, MyDispatch)
#endif
// fix for SEL5002 -scl
#ifdef FP_OLEDRAG
    INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IDataObject, fpDataObject)
    INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IDropSource, fpDropSource)
    INTERFACE_PART(FPOLEMAP_CTRL_CLSNAME, IID_IDropTarget, fpDropTarget)
#endif
// fix for SEL5002 -scl
// property categories -scl
#ifdef SS_V40
    INTERFACE_PART(CPvCtrl, IID_ICategorizeProperties, CategorizeProperties)
#endif
END_INTERFACE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

/*#ifdef SPREAD_JPN
  #ifdef FP_ADB
    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.1",
    0xbf15a0e8, 0x4368, 0x11d3, 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3);
  //   0x35C82C71, 0x7984, 0x11D2, 0xB4, 0x64, 0x00, 0xAA, 0x00, 0xB4, 0x2B, 0x7C)
  #else
    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PV.PvCtrl.1",
    0xbf15a0e9, 0x4368, 0x11d3, 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3);
  //  0x664e2200, 0x24db, 0x11d2, 0x9a, 0x82, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0)
  #endif
#else // SPREAD_JPN*/
  #ifdef SS_V85
  #ifdef _UNICODE
        // {71146844-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVUADO.PvCtrl.6", // FP_GUIDS : string and fpSpreadPreview coclass
		0x6C30120B, 0x5FF5, 0x4352, 0x88, 0x73, 0xB8, 0xB2, 0x36,0x63, 0x48, 0xC2);
  #else
        // {71146846-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.6", // FP_GUIDS : string and fpSpreadPreview coclass
	    0x29CCEDC9, 0xB3FD, 0x4376, 0xA1, 0x4B, 0xF0, 0x58, 0xE5, 0x27, 0x1E, 0x6C); 
  #endif
  #elif SS_V80
  #ifdef _UNICODE
    #ifdef SPREAD_JPN
      // {857FBC00-D0C6-44e6-AB18-D03165173D42}
      IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVUADOJ.PvCtrl.5",
	  0x32841A1E, 0x687D, 0x45e0, 0xB6, 0x64, 0x2C, 0xF1, 0x49, 0x52, 0x3D, 0x07);
    #else
      #ifdef FP_ADB
        // {71146844-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVUADO.PvCtrl.5",
		0x6C30120B, 0x5FF5, 0x4352, 0x88, 0x73, 0xB8, 0xB2, 0x36,0x63, 0x48, 0xC2);
      #else
        // {71146845-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVU.PvCtrl.5",
		0x6C30120B, 0x5FF5, 0x4352, 0x88, 0x73, 0xB8, 0xB2, 0x36,0x63, 0x48, 0xC2);
      #endif
    #endif
  #else
    #ifdef SPREAD_JPN
      #ifdef FP_ADB
        // {857FBC01-D0C6-44e6-AB18-D03165173D42}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADOJ.PvCtrl.5",
        0x21E99B0B, 0x03DB, 0x4c19, 0xB8, 0x5B, 0xF5, 0x24, 0x7B, 0xC5, 0x53, 0xEE);
      #else
        // {857FBC02-D0C6-44e6-AB18-D03165173D42}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVJ.PvCtrl.5",
		0xB4CE3B96, 0xCE70, 0x4802, 0x8F, 0x2A, 0x4E, 0xCF, 0x19, 0xCD, 0xDA, 0xD7);
      #endif
    #else
      #ifdef FP_ADB
        // {71146846-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.5",
	    0x29CCEDC9, 0xB3FD, 0x4376, 0xA1, 0x4B, 0xF0, 0x58, 0xE5, 0x27, 0x1E, 0x6C); 
      #else
        // {71146847-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PV.PvCtrl.5",
	    0x29CCEDC9, 0xB3FD, 0x4376, 0xA1, 0x4B, 0xF0, 0x58, 0xE5, 0x27, 0x1E, 0x6C);
      #endif
    #endif
  #endif // _UNICODE

#elif SS_V70
  #ifdef _UNICODE
    #ifdef SPREAD_JPN
      // {857FBC00-D0C6-44e6-AB18-D03165173D42}
      IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVUADOJ.PvCtrl.4",
      0x857fbc00, 0xd0c6, 0x44e6, 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42);
    #else
      #ifdef FP_ADB
        // {71146844-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVUADO.PvCtrl.4",
	  	0x71146844, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
      #else
        // {71146845-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVU.PvCtrl.4",
	  	0x71146845, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
      #endif
    #endif
  #else
    #ifdef SPREAD_JPN
      #ifdef FP_ADB
        // {857FBC01-D0C6-44e6-AB18-D03165173D42}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADOJ.PvCtrl.4",
        0x857fbc01, 0xd0c6, 0x44e6, 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42);
      #else
        // {857FBC02-D0C6-44e6-AB18-D03165173D42}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVJ.PvCtrl.4",
        0x857fbc02, 0xd0c6, 0x44e6, 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42);
      #endif
    #else
      #ifdef FP_ADB
        // {71146846-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.4",
	  	0x71146846, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
      #else
        // {71146847-020D-4d16-80FD-6ACE384B66DF}
        IMPLEMENT_OLECREATE_EX(CPvCtrl, "PV.PvCtrl.4",
	  	0x71146847, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
      #endif
    #endif
  #endif // _UNICODE
#elif SS_V40
  #ifdef _UNICODE
    #ifdef FP_ADB
      // {B941166C-10E6-4a53-BE96-7FED334704FA}
      IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVUADO.PvCtrl.3",
      0xb941166c, 0x10e6, 0x4a53, 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa);
    #else
      // {B941166D-10E6-4a53-BE96-7FED334704FA}
      IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVU.PvCtrl.3",
      0xb941166d, 0x10e6, 0x4a53, 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa);
    #endif
  #else
    #ifdef FP_ADB
      // {41F841C6-AE16-11d5-8817-0050DA6EF5E5}
      IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.3",
      0x41f841c6, 0xae16, 0x11d5, 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5);
    #else
      // {41F841C7-AE16-11d5-8817-0050DA6EF5E5}
      IMPLEMENT_OLECREATE_EX(CPvCtrl, "PV.PvCtrl.3",
      0x41f841c7, 0xae16, 0x11d5, 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5);
    #endif
  #endif // _UNICODE
#elif SS_V35
  #ifdef FP_ADB
    // {EB9080DF-CA3B-4edf-B515-27E0F2ED3AB6}
// fix for 9189 -scl
//    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.1",
    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.2",
     0xeb9080df, 0xca3b, 0x4edf, 0xb5, 0x15, 0x27, 0xe0, 0xf2, 0xed, 0x3a, 0xb6);
  #else
    // {EC1465BB-AE0A-4b8d-9193-E581B7F3934C}
// fix for 9189 -scl
//    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PV.PvCtrl.1",
    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PV.PvCtrl.2",
    0xec1465bb, 0xae0a, 0x4b8d, 0x91, 0x93, 0xe5, 0x81, 0xb7, 0xf3, 0x93, 0x4c);
  #endif
#else
  #ifdef FP_ADB
    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PVADO.PvCtrl.1",
     0x35C82C71, 0x7984, 0x11D2, 0xB4, 0x64, 0x00, 0xAA, 0x00, 0xB4, 0x2B, 0x7C)
  #else
    IMPLEMENT_OLECREATE_EX(CPvCtrl, "PV.PvCtrl.1",
    0x664e2200, 0x24db, 0x11d2, 0x9a, 0x82, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0)
  #endif
#endif
//#endif // SPREAD_JPN

/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CPvCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs
#ifdef SS_V85
  #ifdef _UNICODE
  #else
  #endif
#elif SS_V80
  #ifdef _UNICODE
    #ifdef SPREAD_JPN
      // Unicode Spread 8.0 ADO JAPAN:
      // {857FBBFA-D0C6-44e6-AB18-D03165173D42}
      const IID BASED_CODE IID_DPv =
	{ 0x857fbbfa, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
      // {857FBBFB-D0C6-44e6-AB18-D03165173D42}
      const IID BASED_CODE IID_DPvEvents =
	{ 0x857fbbfb, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
    #else
      #ifdef FP_ADB // Unicode Spread 8.0 ADO:
        // {7114683C-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683C, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } }; // FP_GUIDS : _DSpreadPreview
        // {71146840-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents = // E34B66D2-761D-4b3d-B1A1-42541B427E35
	  { 0xe34b66d2, 0x761d, 0x4b3d, { 0xb1, 0xa1, 0x42, 0x54, 0x1b, 0x42, 0x7e, 0x35 } }; // FP_GUIDS : _DSpreadPreviewEvents
      #else // Unicode Spread 8.0 DAO:
        // {7114683D-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683D, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } }; // FP_GUIDS : _DSpreadPreview
        // {71146841-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents =
	  { 0x71146841, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } }; // FP_GUIDS : _DSpreadPreviewEvents
      #endif
    #endif
  #else
    #ifdef SPREAD_JPN
      #ifdef FP_ADB // ANSI Spread 8.0 ADO JAPAN:
        // {857FBBFC-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPv =
		{ 0x857fbbfc, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        // {857FBBFD-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPvEvents =
		{ 0x857fbbfd, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
      #else // ANSI Spread 8.0 DAO JAPAN:
        // {857FBBFE-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPv =
		{ 0x857fbbfe, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        // {857FBBFF-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPvEvents =
		{ 0x857fbbff, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
      #endif
    #else
      #ifdef FP_ADB // ANSI Spread 8.0 ADO:
        // {7114683E-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683E, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146842-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents = //F2FFBF52-A648-442f-83FB-7BA6D27FFF6F
	  //{ 0x71146842, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
     { 0xf2ffbf52, 0xa648, 0x442f, { 0x83, 0xfb, 0x7b, 0xa6, 0xd2, 0x7f, 0xff, 0x6f } };
      #else // ANSI Spread 8.0 DAO:
        // {7114683F-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683F, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146843-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents =
	  { 0x71146843, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
      #endif
    #endif
  #endif // _UNICODE
#elif SS_V70
  #ifdef _UNICODE
    #ifdef SPREAD_JPN
      // Unicode Spread 7.0 ADO JAPAN:
      // {857FBBFA-D0C6-44e6-AB18-D03165173D42}
      const IID BASED_CODE IID_DPv =
	{ 0x857fbbfa, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
      // {857FBBFB-D0C6-44e6-AB18-D03165173D42}
      const IID BASED_CODE IID_DPvEvents =
	{ 0x857fbbfb, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
    #else
      #ifdef FP_ADB // Unicode Spread 7.0 ADO:
        // {7114683C-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683C, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146840-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents =
	  { 0x71146840, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
      #else // Unicode Spread 7.0 DAO:
        // {7114683D-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683D, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146841-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents =
	  { 0x71146841, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
      #endif
    #endif
  #else
    #ifdef SPREAD_JPN
      #ifdef FP_ADB // ANSI Spread 7.0 ADO JAPAN:
        // {857FBBFC-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPv =
		{ 0x857fbbfc, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        // {857FBBFD-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPvEvents =
		{ 0x857fbbfd, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
      #else // ANSI Spread 7.0 DAO JAPAN:
        // {857FBBFE-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPv =
		{ 0x857fbbfe, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        // {857FBBFF-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DPvEvents =
		{ 0x857fbbff, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
      #endif
    #else
      #ifdef FP_ADB // ANSI Spread 7.0 ADO:
        // {7114683E-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683E, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146842-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents =
	  { 0x71146842, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
      #else // ANSI Spread 7.0 DAO:
        // {7114683F-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPv =
	  { 0x7114683F, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146843-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DPvEvents =
	  { 0x71146843, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
      #endif
    #endif
  #endif // _UNICODE
#elif SS_V40
  #ifdef _UNICODE
    #ifdef FP_ADB // Unicode Spread 6.0 ADO:
      // {B9411668-10E6-4a53-BE96-7FED334704FA}
      const IID BASED_CODE IID_DPv =
      { 0xb9411668, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
      // {B941166A-10E6-4a53-BE96-7FED334704FA}
      const IID BASED_CODE IID_DPvEvents =
      { 0xb941166a, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
    #else // Unicode Spread 6.0 DAO:
      // {B9411669-10E6-4a53-BE96-7FED334704FA}
      const IID BASED_CODE IID_DPv =
      { 0xb9411669, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
      // {B941166B-10E6-4a53-BE96-7FED334704FA}
      const IID BASED_CODE IID_DPvEvents =
      { 0xb941166b, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
    #endif
  #else
    #ifdef FP_ADB // ANSI Spread 6.0 ADO:
      // {41F841C8-AE16-11d5-8817-0050DA6EF5E5}
      const IID BASED_CODE IID_DPv =
      { 0x41f841c8, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
      // {41F841CA-AE16-11d5-8817-0050DA6EF5E5}
      const IID BASED_CODE IID_DPvEvents =
      { 0x41f841ca, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
    #else // ANSI Spread 6.0 DAO:
      // {41F841C9-AE16-11d5-8817-0050DA6EF5E5}
      const IID BASED_CODE IID_DPv =
      { 0x41f841c9, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
      // {41F841CB-AE16-11d5-8817-0050DA6EF5E5}
      const IID BASED_CODE IID_DPvEvents =
      { 0x41f841cb, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
    #endif
  #endif // _UNICODE
#elif SS_V35
  #ifdef FP_ADB // Spread 3.5 ADO:
    // {EF7E0E80-9076-4d64-83FE-B0653BADBD90}
    const IID BASED_CODE IID_DPv =
		    { 0xef7e0e80, 0x9076, 0x4d64, { 0x83, 0xfe, 0xb0, 0x65, 0x3b, 0xad, 0xbd, 0x90 } };
    // {E1EA9327-9A2A-471d-8D4B-80E5E7D0F7D9}
    const IID BASED_CODE IID_DPvEvents =
		    { 0xe1ea9327, 0x9a2a, 0x471d, { 0x8d, 0x4b, 0x80, 0xe5, 0xe7, 0xd0, 0xf7, 0xd9 } };
  #else // Spread 3.5 DAO:
    // {EE2AE47D-B039-412d-A262-5FD470702DB8}
    const IID BASED_CODE IID_DPv =
		    { 0xee2ae47d, 0xb039, 0x412d, { 0xa2, 0x62, 0x5f, 0xd4, 0x70, 0x70, 0x2d, 0xb8 } };
    // {E7388BD7-1A91-4950-8EAE-FDCA1D8F1942}
    const IID BASED_CODE IID_DPvEvents =
		    { 0xe7388bd7, 0x1a91, 0x4950, { 0x8e, 0xae, 0xfd, 0xca, 0x1d, 0x8f, 0x19, 0x42 } };
  #endif
#else // SS_V30:
  const IID BASED_CODE IID_DPv =
		  { 0x664e2201, 0x24db, 0x11d2, { 0x9a, 0x82, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };
  const IID BASED_CODE IID_DPvEvents =
		  { 0x664e2202, 0x24db, 0x11d2, { 0x9a, 0x82, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };
#endif

/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwPvOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CPvCtrl, IDS_PV, _dwPvOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::CPvCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CPvCtrl

BOOL CPvCtrl::CPvCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
    {
#ifdef WIN32						// only needed in 32-bit OCX -SCL 11/6/96
// ActiveX Component Categories - CTF
	    HRESULT hr;

	    hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls that are safely scriptable");
	    hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");
	    hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
	    hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
// ActiveX Component Categories - CTF
#endif
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_PVOCX,
			IDB_PVOCX,
#ifdef WIN32
            afxRegApartmentThreading,
#else
            FALSE,                      //  Not insertable
#endif
			_dwPvOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
    }
	else
    {
#ifdef WIN32						// only needed in 32-bit OCX -SCL 11/6/96
// ActiveX Component Categories - CTF
	    HRESULT hr;
	    
	    hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
	    hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
// ActiveX Component Categories - CTF
#endif
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
    }
}


/////////////////////////////////////////////////////////////////////////////
// Licensing strings

#if defined(SS_SPREADOCX)
static const TCHAR BASED_CODE _szLicFileName[] = _T("SSOCX.LIC");
#elif defined(SS_GRIDOCX)
static const TCHAR BASED_CODE _szLicFileName[] = _T("FPGRID.LIC");
#elif defined(SS_RSOCX)
static const TCHAR BASED_CODE _szLicFileName[] = _T("RSSPRD32.LIC");
#endif

static const OLECHAR BASED_CODE _szLicString[] =
    OLESTR("Copyright (c) 1995 FarPoint");

#if SS_V80
extern BOOL FileExist(LPTSTR lpszLicenseFileName);
#endif
                                          
/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::CPvCtrlFactory::VerifyUserLicense -
// Checks for existence of a user license

BOOL CPvCtrl::CPvCtrlFactory::VerifyUserLicense()
{
#ifdef SYNERGEX_TRIAL
  SYSTEMTIME st;

  GetSystemTime(&st);
  if( st.wYear > 2001 )
  {
    TCHAR expired[500];
    TCHAR control[50];

    LoadString(AfxGetInstanceHandle(), IDS_SYNERGEX_EXPIRED, expired, 500);
    LoadString(AfxGetInstanceHandle(), IDS_SSOCX, control, 50);
    ::MessageBox(NULL, expired, control, MB_OK);
    return FALSE;
  }
  else
    return TRUE;
#elif defined(FP_UNICODE_TEST)  //GAB Set to expire May 30, 2002
  SYSTEMTIME st;

  GetSystemTime(&st);
  if( (st.wYear >= 2002) && (st.wMonth > 5) )
  {
    TCHAR expired[500];
    TCHAR control[50];

    LoadString(AfxGetInstanceHandle(), IDS_UNICODE_TEST_EXPIRED, expired, 500);
    LoadString(AfxGetInstanceHandle(), IDS_SSOCX, control, 50);
    ::MessageBox(NULL, expired, control, MB_OK);
    return FALSE;
  }
  else
    return TRUE;
/*
#elif defined(FP_SPLASHEXPIRE) //JHS Set to expire Nov 13, 2008
  SYSTEMTIME st;
  int retVal = 0;
  short nType;

  GetSystemTime(&st);
  if( st.wYear >= 2008 && st.wMonth >= 12 && st.wDay >= 2 )
  {
    TCHAR expired[500];
    TCHAR control[50];

    LoadString(AfxGetInstanceHandle(), IDS_BETA_EXPIRED, expired, 500);
    LoadString(AfxGetInstanceHandle(), IDS_SSOCX, control, 50);
    ::MessageBox(NULL, expired, control, MB_OK);
    return FALSE;
  }
  else
  {
       retVal = SoftwareSecureOpen(SS_SECURE_PRODUCT_NAME, SS_SECURE_PREFIX, SS_SECURE_PRODUCT_CODE, 
                                   SS_SECURE_VERSION_MAJOR, SS_SECURE_VERSION_MINOR, SS_SECURE_VERSION_REV,
                                   SS_SECURE_PRODUCT_STRING, &nType, TRUE, AfxGetInstanceHandle());
		if (OPEN_AGREE == retVal)
			return TRUE;
		else
			return FALSE;
  }
*/
#else // !defined(SYNERGEX_TRIAL) && !defined(FP_UNICODE_TEST)

#if SS_V80
  // check for LIC file to get around splash / trial version for Multimedia Fusion -scl
  if( FileExist(_T("fpSpread80.lic")) )
     return TRUE;
#endif

  WORD wSplash = fpSplashIsExpired(AfxGetInstanceHandle());

  if (wSplash == FPSPLASH_NONE)
  {
     #ifdef WIN32
     #ifdef _UNICODE
        LPTSTR lpszName = _T("SS3UX25.OCX");
     #else
        LPTSTR lpszName = "SS32X25.OCX";
     #endif
     #else
        LPTSTR lpszName = "SS16X25.OCX";
     #endif

     return (SS_CheckLicense(lpszName, _T("SPREAD25.LIC"), FALSE));
  }
  else if (wSplash == FPSPLASH_EXPIRED)
    return (FALSE);

  return (TRUE);
#endif // !defined(SYNERGEX_TRIAL)
}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::CPvCtrlFactory::GetLicenseKey -
// Returns a runtime licensing key

BOOL CPvCtrl::CPvCtrlFactory::GetLicenseKey(DWORD dwReserved,
                                                  BSTR FAR* pbstrKey)
{
//  WORD wSplash = fpSplashIsExpired(AfxGetResourceHandle());

#if defined(FP_SPLASHEXPIRE) //JHS Set to expire Nov 13, 2008
  SYSTEMTIME st;
  int retVal = 0;
//  short nType;

  GetSystemTime(&st);
  if( st.wYear >= 2008 && st.wMonth >= 12 && st.wDay >= 2 )
  {
/*
    TCHAR expired[500];
    TCHAR control[50];

    LoadString(AfxGetInstanceHandle(), IDS_BETA_EXPIRED, expired, 500);
    LoadString(AfxGetInstanceHandle(), IDS_SSOCX, control, 50);
    ::MessageBox(NULL, expired, control, MB_OK);
*/
     DialogBoxParam(AfxGetInstanceHandle(), _T("SplashExpire"), 0, (DLGPROC)fpSplashDlgProc, 0L);
    return FALSE;
  }
  else
  {
//       retVal = SoftwareSecureOpen(SS_SECURE_PRODUCT_NAME, SS_SECURE_PREFIX, SS_SECURE_PRODUCT_CODE, 
//                                   SS_SECURE_VERSION_MAJOR, SS_SECURE_VERSION_MINOR, SS_SECURE_VERSION_REV,
//                                   SS_SECURE_PRODUCT_STRING, &nType, TRUE, AfxGetInstanceHandle());
//		if (OPEN_AGREE == retVal)
//			return TRUE;
//		else
//			return FALSE;
    *pbstrKey = SysAllocString((OLECHAR FAR *)"Trial version");
    return TRUE;
  }

#else // !defined(FP_BETA_V2)
/*#ifdef SS_V80
  short nTypeS, nCountS;
  SoftwareSecureQuery(SS_SECURE_PRODUCT_NAME_SOURCE, SS_SECURE_PRODUCT_CODE_SOURCE, &nTypeS, &nCountS);
  if (nTypeS == USAGE_TYPE_PURCHASE)
  {
    *pbstrKey = SysAllocString(_szLicString);
    return (*pbstrKey != NULL);
  }
#endif
  */
  WORD wSplash = fpSplashIsExpired(AfxGetResourceHandle());

#if !defined(CS_OFF) && !defined(SS_V80) // fpSplashIsExpired will show SoftwareSecure dialogs in v8 -scl
  short nType, nCount;

  SoftwareSecureQuery(SS_SECURE_PRODUCT_NAME, SS_SECURE_PRODUCT_CODE, &nType, &nCount);
 
  if ((wSplash == FPSPLASH_NONE) || (nType == USAGE_TYPE_PURCHASE))
#else

  if (wSplash == FPSPLASH_NONE || wSplash == FPSPLASH_OK)
#endif
  {   
    if (pbstrKey == NULL)
        return FALSE;

    *pbstrKey = SysAllocString(_szLicString);

    return (*pbstrKey != NULL);
  }
#ifndef CS_OFF
 else if (wSplash == FPSPLASH_OK)
    {
    if (pbstrKey == NULL)
        return FALSE;

    *pbstrKey = SysAllocString((OLECHAR FAR *)"Trial version");
    return (*pbstrKey != NULL);
    }
#endif
  else if (wSplash == FPSPLASH_EXPIRED)
    return (FALSE);

  return (TRUE);
#endif // FP_BETA_V2
}

// CODESENT  

BOOL CPvCtrl::CPvCtrlFactory::VerifyLicenseKey(BSTR bstrKey)
{
	BOOL bLicensed = FALSE;
	BSTR bstr = NULL;

	if (bstrKey != NULL)
	{
        UINT cch;

        bstr = SysAllocString((OLECHAR FAR *)"Trial version");
		
        ASSERT(bstr != NULL);

		// if length and content match, it's a trial version!

#ifdef WIN32
		cch = SysStringByteLen(bstr);
		if ((cch == SysStringByteLen(bstrKey)) &&
#else
		cch = SysStringLen(bstr);
		if ((cch == SysStringLen(bstrKey)) &&
#endif
			(memcmp(bstr, bstrKey, cch) == 0))
		{
			// throw up dialog with timer
            //fpSplashIsExpired(AfxGetResourceHandle());
            ::MessageBox(NULL, _T("This application was built with an unlicensed version of a FarPoint product."), 
                         _T("Possible License Violation"), MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL);
    		SysFreeString(bstr);
            return TRUE;
		}
		SysFreeString(bstr);    
	    
        bstr = SysAllocString((OLECHAR FAR *)_szLicString);
		
        ASSERT(bstr != NULL);

        // if length and content match, it's good!

#ifdef WIN32
   		cch = SysStringByteLen(bstr);
		if ((cch == SysStringByteLen(bstrKey)) &&
#else
		cch = SysStringLen(bstr);
		if ((cch == SysStringLen(bstrKey)) &&
#endif
		   (memcmp(bstr, bstrKey, cch) == 0))
		{
		   bLicensed = TRUE;
		}

		SysFreeString(bstr);    

	}
    return bLicensed;
}



/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::CPvCtrl - Constructor

CPvCtrl::CPvCtrl()
{
#ifndef VC8
if (!m_mydpIDispTbl.m_dispatchTbl)                       //MYDISP
   {
   // Create Dispatch table
   mydpInitTable(PVPropList, sizeof(PVPropList)/sizeof(FP_PROPLIST) );
   }  
#endif
    InitializeIIDs(&IID_DPv, &IID_DPvEvents);

	// TODO: Initialize your control's instance data here.

m_enumMousePointer = 0;
m_pictMouseIcon.CreateEmpty();
#ifdef FP_OLEDRAG
m_OLEDropMode = 0;
m_pfpDataObject = NULL;
#endif
EnableSimpleFrame();

fpOCXInitPreProps(&PrePropList);
}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::~CPvCtrl - Destructor

CPvCtrl::~CPvCtrl()
{
#ifdef FP_OLEDRAG
  if (m_pfpDataObject)
	  m_pfpDataObject->Release();
#endif
fpOCXFreePreProps(&PrePropList, PVPropList);
}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::OnDrawMetafile - Drawing function

void CPvCtrl::OnDrawMetafile(CDC* pdc, const CRect& rcBounds)
{
CDC     pdcMem;
HDC     hDCMem = 0;
HDC     hDCMeta = pdc->m_hDC;
HDC     hDCAttrib = pdc->m_hAttribDC;
HBITMAP hBmpDC;
HBITMAP hBmpOld;
CRect   rcClient;

pdc->SaveDC();

/* RFW - 5/16/07 - 20149
GetWindowRect(&rcClient);
rcClient.OffsetRect(-(rcClient.left), -(rcClient.top));
*/
GetClientRect(&rcClient);

if ((hDCAttrib) && (hDCAttrib != hDCMeta))
   {
   hBmpDC = CreateCompatibleBitmap(hDCAttrib, rcClient.right - rcClient.left,
                                   rcClient.bottom - rcClient.top);
   hDCMem = CreateCompatibleDC(hDCAttrib);
   pdcMem.SetOutputDC(hDCMem);
   pdcMem.SetAttribDC(hDCMem);
   hBmpOld = (HBITMAP)pdcMem.SelectObject((HGDIOBJ)hBmpDC);
   }

PV_DrawDesignTime(m_hWnd, &pdcMem, rcClient);

StretchBlt(hDCMeta,
           rcBounds.left, rcBounds.top,
           rcBounds.right - rcBounds.left,
           rcBounds.bottom - rcBounds.top,
           pdcMem.m_hDC, rcClient.left,
           rcClient.top,
           rcClient.right - rcClient.left,
           rcClient.bottom - rcClient.top,
           SRCCOPY);

if (hDCMem)
   {
   pdcMem.SelectObject((HGDIOBJ)hBmpOld);
   DeleteObject((HGDIOBJ)hBmpDC);
   pdcMem.ReleaseOutputDC();
   pdcMem.ReleaseAttribDC();
   DeleteDC(hDCMem);
   }

pdc->RestoreDC(-1);
}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::OnDraw - Drawing function

void CPvCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
   if( m_hWnd && !m_bInPlaceActive)
      {
      LPSSPREVIEW lpSSPreview = PV_Lock(m_hWnd);
      lpSSPreview->fNoMemDCPaint = TRUE;
      PV_Unlock(m_hWnd);
      }

   DoSuperclassPaint(pdc, rcBounds);

   if( m_hWnd && !m_bInPlaceActive)
      {
      LPSSPREVIEW lpSSPreview = PV_Lock(m_hWnd);
      lpSSPreview->fNoMemDCPaint = FALSE;
      PV_Unlock(m_hWnd);
      }
}


void PV_DrawDesignTime(HWND hWnd, CDC* pdc, const CRect& rcBounds)
{
CRect rcClient;

SaveDC(pdc->m_hDC);

::GetClientRect(hWnd, &rcClient);

// setup DC for Non-client painting
pdc->SetMapMode(MM_ANISOTROPIC);

pdc->SetWindowOrg(0, 0);
pdc->SetWindowExt(rcClient.Size());
pdc->SetViewportOrg(rcBounds.left, rcBounds.top);
pdc->SetViewportExt(rcBounds.Size() );

// direct call to paint client area

SendMessage(hWnd, WM_PRINT, (WPARAM)(pdc->m_hDC),
            (LPARAM)(PRF_CHILDREN | PRF_CLIENT));

RestoreDC(pdc->m_hDC, -1);
}


void CPvCtrl::OnMousePointerChanged()
{
    POINT ptMouse;
    UINT uHitTest;

    GetCursorPos( &ptMouse);
    uHitTest = (UINT)SendMessage( WM_NCHITTEST, 0, 
                        MAKELPARAM( ptMouse.x, ptMouse.y));
    SendMessage( WM_SETCURSOR, (WPARAM)m_hWnd, 
                        MAKELPARAM( uHitTest, WM_MOUSEMOVE));
	SetModifiedFlag();
}

BOOL CPvCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  POINT ptMouse;
  HCURSOR hCursor = NULL;
  HINSTANCE hInstance = NULL;

   LPSSPREVIEW lpSSPreview = PV_Lock(m_hWnd);

  if (AmbientUserMode() && pWnd == this && lpSSPreview->hCursorCurrent)
     {
#ifdef WIN32
     SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)lpSSPreview->hCursorCurrent);
#else
     SetClassWord(m_hWnd, GCW_HCURSOR, (WORD)lpSSPreview->hCursorCurrent);
#endif 
     SetCursor(lpSSPreview->hCursorCurrent);
     PV_Unlock(m_hWnd);
     return (TRUE);
     }

  PV_Unlock(m_hWnd);

  // If the MousePointer property doesn't indicate the default cursor
  if (AmbientUserMode() && (m_enumMousePointer))
     {
     // if the mouse cursor is over the client area
     if ((pWnd == this) && (nHitTest == HTCLIENT))
        {
        GetCursorPos(&ptMouse);
        pWnd->ScreenToClient(&ptMouse);

        // if the mousePointer prop indicates a custom cursor is not to be
        // used, set the mouse cursor to a stock bitmap.  Otherwise, use
        // the bitmap indicated by the MouseIcon prop.
        if (m_enumMousePointer != CUSTOM_CURSOR)
           {
           if (m_enumMousePointer > MAX_STANDARD_INDEX)
             hInstance = AfxGetResourceHandle();
           hCursor = LoadCursor( hInstance, idCursor[m_enumMousePointer]);
           }
        else if (m_pictMouseIcon.m_pPict)
           {
           m_pictMouseIcon.m_pPict->get_Handle((OLE_HANDLE *)&hCursor);
           // Under WIN32 you can use the icon handle as the cursor handle,
           // under WIN16 you need to convert it.
//ifndef _WIN32
           hCursor = OleIconToCursor(AfxGetInstanceHandle(), hCursor);
//endif
           }
        // If the bitmap exists, use it to set the mouse cursor
        if (hCursor)
           SetCursor(hCursor);

        return (hCursor != NULL);
        }
     }

	return COleControl::OnSetCursor(pWnd, nHitTest, message);
}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::DoPropExchange - Persistence support

void CPvCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

    if (NULL == m_hWnd && !pPX->IsLoading())
       CreateWindowForSubclassedControl();

    PX_Props(pPX);
    PX_Short(pPX, _T("MousePointer"), m_enumMousePointer, 0);
    PX_Picture(pPX, _T("MouseIcon"), m_pictMouseIcon);
#ifdef FP_OLEDRAG
	PX_Short(pPX, _T("OLEDropMode"), m_OLEDropMode, 0);
#endif
    if (pPX->IsLoading())
       SetModifiedFlag(FALSE);
}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::OnResetState - Reset control to default state

void CPvCtrl::OnResetState()
{
    m_pictMouseIcon.CreateEmpty();
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange
    m_sBorderStyle = 1;

}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::AboutBox - Display an "About" box to the user

void CPvCtrl::AboutBox()
{
   PURCHASEINFO purchaseInfo;
#if !defined(CS_OFF) && !defined(FP_BETA)
   _fstrcpy(purchaseInfo.productKey, SS_SECURE_PRODUCT_NAME);
   purchaseInfo.productID = SS_SECURE_PRODUCT_CODE;
       
   SoftwareSecureGetSerialNumber(purchaseInfo.productKey, purchaseInfo.serialNumber,
                                 purchaseInfo.productID, TRUE);
#else
   _fmemset(&purchaseInfo, '\0', sizeof(PURCHASEINFO));
#endif

   fpInitAboutPopUp(m_hWnd, (LPPURCHASEINFO)&purchaseInfo);
}


/////////////////////////////////////////////////////////////////////////////
// CPvCtrl::PreCreateWindow

BOOL CPvCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.lpszClass = FPCLASS_SSPREVIEW;
    cs.style |= WS_VSCROLL | WS_HSCROLL;
	return COleControl::PreCreateWindow(cs);
}

WNDPROC* CPvCtrl::GetSuperWndProcAddr(void)
{
	static WNDPROC NEAR pfnSuper;
	return &pfnSuper;
}

int CPvCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int rc;
    
    rc = COleControl::OnCreate(lpCreateStruct); 
	if (rc == -1)
		return rc;

    LPSSPREVIEW lpPV = PV_Lock(m_hWnd);

    lpPV->fDesignTime = !AmbientUserMode();
    fpOCXLoadPreProps(m_hWnd, &PrePropList, lpPV, (FP_PROPFUNC)PV_SetProp,
                      PVPropList);
    PV_Unlock(m_hWnd);

    // Added to fix bug SCS6287
    ::SetWindowLong(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) &
                    ~WS_EX_NOPARENTNOTIFY);

    return (0);
}


void CPvCtrl::OnDestroy()
{
COleControl::OnDestroy();
}

void CPvCtrl::OnFreezeEvents(BOOL bFreeze)
{
#ifdef FP_OLEDRAG
  if (bFreeze && m_OLEDropMode)
	  ::RevokeDragDrop(m_hWnd);
  else if (m_OLEDropMode)
	  ::RegisterDragDrop(m_hWnd, &m_xfpDropTarget);
#endif
  COleControl::OnFreezeEvents(bFreeze);
}

LPPICTUREDISP CPvCtrl::GetMouseIcon()
{
return m_pictMouseIcon.GetPictureDispatch();
}


void CPvCtrl::SetMouseIcon(LPPICTUREDISP newValue)
{
if (newValue)
   {
   CPictureHolder Picture;
   short          sType;

   Picture.SetPictureDispatch(newValue);

   Picture.m_pPict->get_Type((short FAR *)&sType);

   if( sType == PICTYPE_NONE )
     newValue = NULL;
   else if (sType != PICTYPE_ICON)
      return;
   }

m_pictMouseIcon.SetPictureDispatch(newValue);
BoundPropertyChanged(51);
SetModifiedFlag();
}

BOOL CPvCtrl::PX_Props(CPropExchange* pPX)
{
LPSSPREVIEW lpPV = 0;
BOOL        fRet;

if (m_hWnd)
   lpPV = PV_Lock(m_hWnd);

fRet = fpOCXPX(this, pPX, (LPVOID)lpPV, (FP_PROPFUNC)PV_GetProp,
               (FP_PROPFUNC)PV_SetProp, PVPropList, PVPROP_PROPCNT,
               &PrePropList, FPX_STR_GH);

if (lpPV)
   PV_Unlock(m_hWnd);

return (fRet);
}


void CPvCtrl::fpSetProp(WORD wPropID, long lValue, long lSetIndex)
{
LPSSPREVIEW lpPV = 0;

if (m_hWnd)
   lpPV = PV_Lock(m_hWnd);

if (AmbientUserMode() == FALSE)
   BoundPropertyChanged(DISPID_UNKNOWN);

fpOCXSetProp(this, m_hWnd, (LPVOID)lpPV, wPropID, lValue,
             (FP_PROPFUNC)PV_SetProp, PVPropList, &PrePropList, (int)lSetIndex);

if (lpPV)
   PV_Unlock(m_hWnd);

// Any special processing
}

long CPvCtrl::fpGetProp(WORD wPropID, long lGetIndex)
{
LPSSPREVIEW lpPV = 0;
long        lRet;

if (m_hWnd)
   lpPV = PV_Lock(m_hWnd);

lRet = fpOCXGetProp(this, m_hWnd, (LPVOID)lpPV, wPropID, 0,
                    (FP_PROPFUNC)PV_GetProp, PVPropList, (int)lGetIndex,
                    FPX_STR_GH, &PrePropList);

if (lpPV)
   PV_Unlock(m_hWnd);

return (lRet);
}


BOOL CPvCtrl::PreTranslateMessage(LPMSG lpMsg)
{
switch (lpMsg->message)
   {
   case WM_KEYDOWN:
      switch (lpMsg->wParam)
         {
         case VK_UP:
         case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
         case VK_PRIOR:
         case VK_NEXT:
         case VK_HOME:
         case VK_END:
            ::SendMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
            return (TRUE);
         }
      break;
   }

return (COleControl::PreTranslateMessage(lpMsg));
}


// fix for #8705 -scl
LRESULT CPvCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
  WORD wNotifyCode = HIWORD(wParam);
#else
  WORD wNotifyCode = HIWORD(lParam);
#endif

  OnCommand(wParam, lParam);
  // TODO: Switch on wNotifyCode here.

  return 0;
}

BOOL CPvCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
WORD wNotifyCode = HIWORD(wParam);
#else
WORD wNotifyCode = HIWORD(lParam);
#endif

switch (wNotifyCode)
   {
   case SPN_PAGECHANGE:
      long lPageCurrent;

      lPageCurrent = ::SendMessage(m_hWnd, SPM_GETPROPERTY,
                                   PVPROP_PAGECURRENT, 0L);

      FirePageChange(lPageCurrent);
      break;

   case SPN_ZOOM:
      FireZoom();
      break;
   }

return (TRUE);
}

short CPvCtrl::ShiftState()
{
	BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
	BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
	BOOL bAlt   = (GetKeyState(VK_MENU) < 0);

	return (short)(bShift + (bCtrl << 1) + (bAlt << 2));
}

void CPvCtrl::ButtonDown(USHORT iButton, UINT, CPoint point)
{
//	DWORD dwStockEventMask = GetStockEventMask();
//	if ((dwStockEventMask & STOCKEVENTS_MOUSE) || m_bPendingUIActivation)
//	{
		if (m_iButtonState == 0)
			SetCapture();

		m_iButtonState |= iButton;

//		if (dwStockEventMask & STOCKEVENT_MOUSEDOWN)
			FireMouseDown(iButton, ShiftState(), point.x, point.y);

		m_iDblClkState &= ~iButton;
//	}
}

void CPvCtrl::ButtonUp(USHORT iButton, UINT, CPoint point)
{
	if (m_iButtonState != 0)
	{
		m_iButtonState &= ~iButton;

		if (m_iButtonState == 0)
			ReleaseCapture();

//		DWORD dwStockEventMask = GetStockEventMask();

//		if ((dwStockEventMask & STOCKEVENT_CLICK) &&
//			!(m_iDblClkState & iButton))
		if (!(m_iDblClkState & iButton))
		{
			CRect rect;
			GetClientRect(&rect);
			if (rect.PtInRect(point))
				OnClick(iButton);
		}

//		if (dwStockEventMask & STOCKEVENT_MOUSEUP)
			FireMouseUp(iButton, ShiftState(), point.x, point.y);

		m_iDblClkState &= ~iButton;
	}
}

void CPvCtrl::ButtonDblClk(USHORT iButton, UINT, CPoint)
{
//	DWORD dwStockEventMask = GetStockEventMask();
//	if (dwStockEventMask & STOCKEVENTS_MOUSE)
//	{
//		SetCapture();
		m_iButtonState |= iButton;

//		if (dwStockEventMask & STOCKEVENT_DBLCLICK)
//		{
			FireDblClick();
			m_iDblClkState |= iButton;
//		}
//	}
}

void CPvCtrl::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
//	if (GetStockEventMask() & STOCKEVENT_MOUSEMOVE)
//	{
		HWND hWndSave = m_hWnd;
		FireMouseMove((short)m_iButtonState, ShiftState(), point.x, point.y);
		if (m_hWnd != hWndSave)
			return;
//	}
	Default();
}

void CPvCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	OnButtonDown(LEFT_BUTTON, nFlags, point);
}

void CPvCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp(LEFT_BUTTON, nFlags, point);
}

void CPvCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnButtonDblClk(LEFT_BUTTON, nFlags, point);
}

void CPvCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{
	OnButtonDown(MIDDLE_BUTTON, nFlags, point);
}

void CPvCtrl::OnMButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp(MIDDLE_BUTTON, nFlags, point);
}

void CPvCtrl::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	OnButtonDblClk(MIDDLE_BUTTON, nFlags, point);
}

void CPvCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	OnButtonDown(RIGHT_BUTTON, nFlags, point);
}

void CPvCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp(RIGHT_BUTTON, nFlags, point);
}

void CPvCtrl::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	OnButtonDblClk(RIGHT_BUTTON, nFlags, point);
}

void CPvCtrl::OnButtonDown(USHORT nButton, UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	if (nButton == LEFT_BUTTON)
		SetFocus();
	ButtonDown(nButton, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	Default();
}

void CPvCtrl::OnButtonUp(USHORT nButton, UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	Default();
	ButtonUp(nButton, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	if (m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation)
	{
		m_bPendingUIActivation = FALSE;
		HWND hWndFocus = ::GetFocus();
		if (hWndFocus == m_hWnd || ::IsChild(m_hWnd, hWndFocus))
			OnActivateInPlace(TRUE, NULL);
	}
}

void CPvCtrl::OnButtonDblClk(USHORT nButton, UINT nFlags, CPoint point)
{
	HWND hWndSave = m_hWnd;
	ButtonDblClk(nButton, nFlags, point);
	if (m_hWnd != hWndSave)
		return;

	Default();
}

void CPvCtrl::OnCancelMode()
{
	CWnd::OnCancelMode();

	if ((m_iButtonState != 0) || (m_iDblClkState != 0))
	{
		ReleaseCapture();
		m_iButtonState = 0;
		m_iDblClkState = 0;
	}
}

void CPvCtrl::OnClick(USHORT /*iButton*/)
{
	// May be overridden by subclass

//	if (GetStockEventMask() & STOCKEVENT_CLICK)
		FireClick();
}


#ifdef SS_V35

BOOL CPvCtrl::GetScriptEnhanced()
{
BOOL  bScriptEnhanced = FALSE;
LPSSPREVIEW lpPV = 0;

if (m_hWnd)
   {
   lpPV = PV_Lock(m_hWnd);
   bScriptEnhanced = lpPV->bScriptEnhanced;
   PV_Unlock(m_hWnd);
   }

return bScriptEnhanced;
}

//--------------------------------------------------------------------

void CPvCtrl::SetScriptEnhanced(BOOL NewValue)
{
LPSSPREVIEW lpPV = 0;

if (m_hWnd)
   {
   lpPV = PV_Lock(m_hWnd);
   lpPV->bScriptEnhanced = NewValue;
   PV_Unlock(m_hWnd);
   }
}

#endif

#ifndef VC8
// Uses the define FPMYDISP_CTRL_NAME"
#include "fpmydisp.cpp"                                  //MYDISP
#endif

#ifdef FP_OLEDRAG
#include "..\sssrc\spread\pvoledrg.cpp"
#endif

// property categories
#ifdef SS_V40

#define SSPV_PROPCAT_SCROLLBARS 0

STDMETHODIMP_(ULONG) CPvCtrl::XCategorizeProperties::AddRef()
{
  METHOD_MANAGE_STATE(CPvCtrl, CategorizeProperties)
  return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CPvCtrl::XCategorizeProperties::Release()
{
  METHOD_MANAGE_STATE(CPvCtrl, CategorizeProperties)
  return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CPvCtrl::XCategorizeProperties::QueryInterface(REFIID iid, LPVOID FAR *ppvObj)
{
  METHOD_MANAGE_STATE(CPvCtrl, CategorizeProperties)
  return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CPvCtrl::XCategorizeProperties::PropertyToCategory(DISPID dispid, PROPCAT *ppropcat)
{
METHOD_MANAGE_STATE(CPvCtrl, CategorizeProperties)

switch(dispid)
	{
   case 2:                    // GrayAreaColor
   case 3:                    // GrayAreaMarginH
   case 4:                    // GrayAreaMarginType
   case 5:                    // GrayAreaMarginV
   case 7:                    // PageBorderColor
   case 8:                    // PageBorderWidth
   case 9:                    // PageShadowColor
   case 10:                   // PageShadowWidth
   case 11:                   // PageViewPercentage
   case 12:                   // PageViewType
   case 17:                   // PageMultiCntH
   case 18:                   // PageMultiCntV
   case 19:                   // PageGutterH
   case 20:                   // PageGutterV
		*ppropcat = VB_PROPCAT_APPEARANCE;
		break;

   case 1:                    // AllowUserZoom
   case 50:                   // MousePointer
   case 51:                   // MouseIcon
		*ppropcat = VB_PROPCAT_BEHAVIOR;
		break;

   case 13:                   // ScrollBarH
   case 14:                   // ScrollBarV
   case 15:                   // ScrollIncH
   case 16:                   // ScrollIncV
		*ppropcat = SSPV_PROPCAT_SCROLLBARS;
		break;

	default:
		*ppropcat = VB_PROPCAT_MISC;
	}

return S_OK;
}

STDMETHODIMP CPvCtrl::XCategorizeProperties::GetCategoryName(PROPCAT propcat, LCID lcid, BSTR *pbstrName)
{
METHOD_MANAGE_STATE(CPvCtrl, CategorizeProperties)

switch(propcat)
	{
	case SSPV_PROPCAT_SCROLLBARS:
#ifdef SPREAD_JPN
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Ω∏€∞Ÿ ﬁ∞"));
#else
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Scroll Bars"));
#endif
		break;
	default:
		return E_FAIL;
	}

return S_OK;
}

#endif // SS_V40
