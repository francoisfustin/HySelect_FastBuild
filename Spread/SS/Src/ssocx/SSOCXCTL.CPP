//---------------------------------------------------------------------------
//
// MODULE:		ssocxctl.cpp 
//
// PURPOSE:		Implementation of the CSsocxCtrl OLE control class.
//
// UPDATE LOG:	
//
//		RWP01	08.28.96	<KEM3462> SD locks if RecordSource not set
//		RWP02	03.20.97	Rockwell Spread Designer changes
//		RWP03	10.20.97	<JIS3493> No hourglass cursor on SD load				
//
//---------------------------------------------------------------------------

#include "windowsx.h"         //DBOCX
#include <math.h>

#include "stdafx.h"

#include "..\..\..\fplibs\fptools\src\fptools.h"
#include "..\..\..\fplibs\fptools\src\fpconvrt.h"
#include "..\..\..\fplibs\fptools\src\fptstr.h"
#include "..\..\..\fplibs\fptools\src\vbinterf.h"

#ifdef WIN32
#include "comcat.h"           // ActiveX component categories - CTF
#include <objsafe.h>
#endif

// property categories -scl
#ifdef SS_V40
#include "propcat.h"
#include "AFXPRIV.H"
#endif

#include "ssocx.h"
#include "ssocxctl.h"
#include "ssocxdb.h"          //DBOCX
#ifndef SS_V30
#include "ssocxppg.h"
#include "opmodepr.h"
#include "virtualp.h"
#include "displayp.h"
#include "editmode.h"
#include "headersp.h"
#include "ppgcolor.h"
#endif
#include <io.h>
#include <stdlib.h>

#include "..\sssrc\classes\wintools.h"
#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_bool.h"
#include "..\sssrc\spread\ss_bord.h"
#include "..\sssrc\spread\ss_calc.h"
#include "..\sssrc\spread\ss_cell.h"
#include "..\sssrc\spread\ss_col.h"
#include "..\sssrc\spread\ss_curs.h"
#include "..\sssrc\spread\ss_data.h"
#include "..\sssrc\spread\ss_dde.h"
#include "..\sssrc\spread\ss_doc.h"
#include "..\sssrc\spread\ss_draw.h"
#include "..\sssrc\spread\ss_emode.h"
#include "..\sssrc\spread\ss_font.h"
#include "..\sssrc\spread\ss_formu.h"
#include "..\sssrc\spread\ss_main.h"
#include "..\sssrc\spread\ss_multi.h"
#include "..\sssrc\spread\ss_ole.h"
#include "..\sssrc\spread\ss_row.h"
#include "..\sssrc\spread\ss_save.h"
#include "..\sssrc\spread\ss_scrol.h"
#include "..\sssrc\spread\ss_type.h"
#include "..\sssrc\spread\ss_user.h"
#include "..\sssrc\spread\ss_virt.h"
#include "..\sssrc\spread\ss_save4.h"
#include "..\vbsrc\vbdattim.h"
#include "..\vbsrc\vbmsg.h"

#ifdef FP_ADB
#include "fpdbmfc.h"
#endif

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

#define ADD_OFFSET(lpv, dw)        ((LPVOID)((LPBYTE)lpv + dw))
#ifdef WIN32
  #ifndef SPREAD_JPN
    #ifdef SS_RSOCX
      #define SPREAD_DESIGNER "RS32D25.EXE"
    #else
	#ifdef SS_V80
		#if defined(UNICODE) || defined(_UNICODE)
			#define SPREAD_DESIGNER "SD32DU80.EXE"
		#else
//			#define SPREAD_DESIGNER "C://SD7//WINDEBUG70//SD32D70.EXE"
			#define SPREAD_DESIGNER "SD32D80.EXE"
		#endif
	#elif SS_V70
		#if defined(UNICODE) || defined(_UNICODE)
			#define SPREAD_DESIGNER "SD32DU70.EXE"
		#else
//			#define SPREAD_DESIGNER "C://SD7//WINDEBUG70//SD32D70.EXE"
			#define SPREAD_DESIGNER "SD32D70.EXE"
		#endif
	#elif SS_V40
		#if defined(UNICODE) || defined(_UNICODE)
			#define SPREAD_DESIGNER "SD32DU60.EXE"
		#else
			#define SPREAD_DESIGNER "SD32D60.EXE"
		#endif
	#elif SS_V35
		#ifdef _UNICODE
			#define SPREAD_DESIGNER "SD3UD35.EXE"
		#else
			#define SPREAD_DESIGNER "SD32D35.EXE"
		#endif
	#elif SS_V30
        #ifdef _UNICODE
          #define SPREAD_DESIGNER "SD3UD30.EXE"
        #else
          #define SPREAD_DESIGNER "SD32D30.EXE"
        #endif
    #else
        #define SPREAD_DESIGNER "SD32D25.EXE" 
    #endif // SS_V30
    #endif
  #else
	#ifdef SS_V80
		#if defined(UNICODE) || defined(_UNICODE)
			#define SPREAD_DESIGNER "SD32DU80J.EXE"
		#else
//			#define SPREAD_DESIGNER "C://SD7//WINDEBUG70//SD32D70.EXE"
			#define SPREAD_DESIGNER "SD32D80J.EXE"
		#endif
     #elif SS_V70
		#if defined(UNICODE) || defined(_UNICODE)
			#define SPREAD_DESIGNER "SD32DU70J.EXE"
		#else
//			#define SPREAD_DESIGNER "C://SD7//WINDEBUG70//SD32D70.EXE"
			#define SPREAD_DESIGNER "SD32D70J.EXE"
		#endif
     #elif SS_V40
       #define SPREAD_DESIGNER "SD32D60J.EXE"
	  #elif SS_V30
	    #define SPREAD_DESIGNER "SD32D30J.EXE" 
  	#else
	    #define SPREAD_DESIGNER "SD32D25J.EXE" 
	  #endif
  #endif
#else
  #ifndef SPREAD_JPN
    #ifdef SS_V30
      #define SPREAD_DESIGNER "SD16D30.EXE"
    #else
      #define SPREAD_DESIGNER "SD16D25.EXE" 
    #endif // SS_V30 
  #else
    #define SPREAD_DESIGNER "SD16D25J.EXE" 
  #endif
#endif  

#ifdef WIN32
// ActiveX component categories - CTF
/////////////////////////////////////////////////////////
//const IID IID_ICatRegister = {0x0002E012,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
//const CATID CATID_SafeForScripting		= {0x7dd95801,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};
//const CATID CATID_SafeForInitializing	= {0x7dd95802,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};
//HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription);
//HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid);
//HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid);
/////////////////////////////////////////////////////////
// ActiveX component categories - CTF
#endif

                                     
                                     
LRESULT CALLBACK SD_MouseFilterHook(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SD_KeyFilterHook(int code, WPARAM wParam, LPARAM lParam);
HHOOK hhookMouseFilter = NULL;
HHOOK hhookKeyFilter = NULL;


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif                            

extern "C" 
{
  extern HINSTANCE hDynamicInst;
}

DEFINE_GUID(IID_IVBGetControl, 0x40A050A0L, 0x3C31, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);

// OLESTR is defined in WIN32 but not WIN16, if not defined, define it as simply the parameter.
#ifndef OLESTR
#define OLESTR(x) x
#endif  

// WIN16
#ifndef WIN32
static UINT NEAR WM_COPYDATA = RegisterWindowMessage("COPY_DATA");
#endif


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for OCX About
// this dialog is not used -scl
/*
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	void SetObject(CSsocxCtrl *pObj) { pObject = pObj; }

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	virtual void OnProps();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	CSsocxCtrl *pObject;
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX_SSOCX)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_SETPROPS, OnProps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    if (fpIsTrialVersion() == FALSE)  // release build
    {
       (GetDlgItem(IDC_SETPROPS))->ShowWindow(SW_HIDE);
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnProps()
{   
   EndDialog(0);
   pObject->OnProperties(NULL, 
					     (pObject->GetParent())->GetSafeHwnd(), 
					     NULL);
}
*/

/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNCREATE(CSsocxCtrl, COleControl)

#ifdef	SPREAD_JPN
// define this global flag to inform Visual Basic that does't hook keyboard
// and mouse message in order to give user a change to close the 
// dialog box when errors occur. Fengwei
BOOL	bGetCursor = FALSE;
#endif

//DBOCX...
/////////////////////////////////////////////////////////////////////////////
//  Interface Map                                                 //DBOCX
//#ifdef VC40
//#define FPOLEMAP_CTRL_CLSNAME  CSsocxCtrl
//BEGIN_INTERFACE_MAP(CSsocxCtrl, CWnd)
//    INTERFACE_PART(CSsocxCtrl, IID_IBoundObject, BoundObject)
//#include "fpolemap.h"
//END_INTERFACE_MAP()
#ifdef WIN32
BEGIN_INTERFACE_MAP(CSsocxCtrl, COleControl)
    INTERFACE_PART(CSsocxCtrl, IID_IObjectSafety, ObjectSafety)
#ifndef FP_ADB
    INTERFACE_PART(CSsocxCtrl, IID_IBoundObject, BoundObject)
#endif
// fix for SEL5002 -scl
#ifdef FP_OLEDRAG
    INTERFACE_PART(CSsocxCtrl, IID_IDataObject, fpDataObject)
    INTERFACE_PART(CSsocxCtrl, IID_IDropSource, fpDropSource)
    INTERFACE_PART(CSsocxCtrl, IID_IDropTarget, fpDropTarget)
#endif
// fix for SEL5002 -scl
    INTERFACE_PART(CSsocxCtrl, IID_IPersistPropertyBag, PersistPropertyBag)
    INTERFACE_PART(CSsocxCtrl, IID_IQuickActivate, QuickActivate)
// property categories -scl
#ifdef SS_V40
    INTERFACE_PART(CSsocxCtrl, IID_ICategorizeProperties, CategorizeProperties)
#endif
END_INTERFACE_MAP()
#else
BEGIN_INTERFACE_MAP(CSsocxCtrl, COleControl)
    INTERFACE_PART(CSsocxCtrl, IID_IBoundObject, BoundObject)
END_INTERFACE_MAP()
#endif
//DBOCX.

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSsocxCtrl, COleControl)
    //{{AFX_MSG_MAP(CSsocxCtrl)
    ON_WM_CHAR()
  //}}AFX_MSG_MAP
  ON_OLEVERB(IDS_SPREAD_DESIGNER + LANGUAGE_BASE, OnProperties)
  ON_WM_CREATE()
  ON_MESSAGE(SSM_CELLLOCKED, OnSsmCellLocked)
  ON_MESSAGE(OCM_COMMAND, OnOcmCommand)  
  ON_MESSAGE(SSM_CLICK, OnSsmClick)
  ON_MESSAGE(SSM_COLRESTRICTED, OnSsmColRestricted)
  ON_MESSAGE(SSM_COLROWRESTRICTED, OnSsmColRowRestricted)
  ON_MESSAGE(SSM_COLWIDTHCHANGE, OnSsmColWidthChange)
  ON_MESSAGE(SSM_DATACHANGE, OnSsmDataChange)
  ON_MESSAGE(SSM_DBLCLK, OnSsmDblClk)
  ON_MESSAGE(SSM_DRAGDROP, OnSsmDragDrop)
  ON_MESSAGE(SSM_DRAWITEM, OnSsmDrawItem)
  ON_MESSAGE(SSM_EDITMODEOFF, OnSsmEditModeOff)
  ON_MESSAGE(SSM_EDITMODEON, OnSsmEditModeOn)
  ON_MESSAGE(SSM_ENTERROW, OnSsmEnterRow)
  ON_MESSAGE(SSM_FIREEVENTPRINTABORT, OnSsmFireEventPrintAbort)
  ON_MESSAGE(SSM_FIREEVENTPRINTMSGCLOSE, OnSsmFireEventPrintMsgClose)
  ON_MESSAGE(SSM_FIREEVENTPRINTMSGOPEN, OnSsmFireEventPrintMsgOpen)
  ON_MESSAGE(SSM_KEYDOWN, OnSsmKeyDown)
  ON_MESSAGE(SSM_KEYPRESS, OnSsmKeyPress)
  ON_MESSAGE(SSM_KEYUP, OnSsmKeyUp)
  ON_MESSAGE(SSM_LEAVECELL, OnSsmLeaveCell)
  ON_MESSAGE(SSM_LEAVEROW, OnSsmLeaveRow)
  #if defined(SS_DDE)
  ON_MESSAGE(SSM_LINKDISPATCHNOTCREATED, OnSsmLinkDispatchNotCreated)
  ON_MESSAGE(SSM_LINKERROR, OnSsmLinkError)
  ON_MESSAGE(SSM_LINKITEMNOTSUPPORTED, OnSsmLinkItemNotSupported)
  ON_MESSAGE(SSM_LINKITEMSUPPORTED, OnSsmLinkItemSupported)
  ON_MESSAGE(SSM_LINKNEWDATA, OnSsmLinkNewData)
  ON_MESSAGE(SSM_LINKNOTIFY, OnSsmLinkNotify)
  ON_MESSAGE(SSM_LINKOUTOFMEMORY, OnSsmLinkOutOfMemory)
  ON_MESSAGE(SSM_LINKPOKECOMPLETED, OnSsmLinkPokeCompleted)
  ON_MESSAGE(SSM_LINKREQUESTCOMPLETED, OnSsmLinkRequestCompleted)
  ON_MESSAGE(SSM_LINKSERVERDISCONNECTED, OnSsmLinkServerDisconnected)
  ON_MESSAGE(SSM_LINKUNABLETOCONNECTTOSERVER, OnSsmLinkUnableToConnectToServer)
  #endif // SS_DDE
  ON_MESSAGE(SSM_RBUTTON, OnSsmRButton)
  ON_MESSAGE(SSM_ROWHEIGHTCHANGE, OnSsmRowHeightChange)
  ON_MESSAGE(SSM_ROWRESTRICTED, OnSsmRowRestricted)
  ON_MESSAGE(SSM_VCLEARDATA, OnSsmVClearData)
  ON_MESSAGE(SSM_VQUERYDATA, OnSsmVQueryData)
#ifdef SS_V30
  ON_MESSAGE(SSM_COMBOCLOSEUP, OnSsmComboCloseUp)
  ON_MESSAGE(SSM_COMBODROPDOWN, OnSsmComboDropDown)
  ON_MESSAGE(SSM_COMBOSELCHANGE, OnSsmComboSelChange)
  ON_MESSAGE(SSM_TEXTTIPFETCH, OnSsmTextTipFetch)
  ON_MESSAGE(SSM_EDITCHANGE, OnSsmEditChange)
  ON_MESSAGE(PRVWM_GETINFO, OnPrvwmGetInfo)
  ON_MESSAGE(PRVWM_PRINTPAGE, OnPrvwmPrintPage)
#endif
#ifdef SS_V35
  ON_MESSAGE(SSM_BEFOREUSERSORT, OnSsmBeforeUserSort)
  ON_MESSAGE(SSM_AFTERUSERSORT, OnSsmAfterUserSort)
#endif
#ifdef SS_V70
  ON_MESSAGE(SSM_SHEETCHANGING, OnSsmSheetChanging)
  ON_MESSAGE(SSM_SHEETCHANGED, OnSsmSheetChanged)
  ON_MESSAGE(SSM_TABSCROLLING, OnSsmTabScrolling)
  ON_MESSAGE(SSM_TABSCROLLED, OnSsmTabScrolled)
  ON_MESSAGE(SSM_BEFOREEDITMODE, OnSsmBeforeEditMode)
  ON_MESSAGE(SSM_BEFORESCROLLTIP, OnSsmBeforeScrollTip)
  ON_MESSAGE(SSM_BEFORECOLMOVE, OnSsmBeforeColMove)
  ON_MESSAGE(SSM_BEFOREROWMOVE, OnSsmBeforeRowMove)
#endif // SS_V70
#ifdef SS_V80
  ON_MESSAGE(SSM_CIRCULARFORMULA, OnSsmCircularFormula)
#endif // SS_V80
  ON_MESSAGE(SSM_POSTDATACHANGE, OnSsmPostDataChange)   //DBOCX
  ON_MESSAGE(SSM_POSTADDNEW, OnSsmPostAddNew)           //DBOCX
  ON_MESSAGE(SSM_BOUNDCLICK, OnSsmBoundClick)           //DBOCX
  ON_WM_CANCELMODE()                                    //DBOCX
#ifdef SPREAD_JPN
  // SPRD022 Wei Feng 1997/9/26
  ON_WM_KEYDOWN()
#endif
  ON_WM_KILLFOCUS()                                     //DBOCX
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_MBUTTONDOWN()
  ON_WM_MBUTTONUP()
  ON_WM_MBUTTONDBLCLK()
  ON_WM_RBUTTONDOWN()
  ON_WM_RBUTTONUP()
  ON_WM_RBUTTONDBLCLK()
  ON_WM_MOUSEMOVE()
  ON_WM_NCPAINT()
#ifdef SS_V40
  ON_WM_CHAR()
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()
  ON_WM_SYSKEYDOWN()
  ON_WM_SYSKEYUP()
  ON_WM_INITMENU()
  ON_WM_INITMENUPOPUP()
#endif // SS_V40
  // Override subclass creation & showing
#ifndef WIN32  //used only for WIN16
  ON_STDOLEVERB(OLEIVERB_PRIMARY, OnVerbPrimary)
  ON_STDOLEVERB(OLEIVERB_UIACTIVATE, OnVerbUiActivate)
  ON_STDOLEVERB(OLEIVERB_INPLACEACTIVATE, OnVerbInPlaceActivate)
#endif
  // Spread Designer
  ON_MESSAGE(SSM_SPREADDSGEXIT, OnSpreadDsgExit)
  ON_MESSAGE(SSM_SPREADDSGHEREIAM, OnSpreadDsgHereIAm)
  ON_MESSAGE(SSM_GETCOLUMNNAMES, OnGetColumnNames)
#ifndef SS_V30
  ON_OLEVERB(IDS_SPREAD_DESIGNER + LANGUAGE_BASE, OnSpreadDesigner)
#endif
#ifdef WIN32
  ON_MESSAGE(WM_COPYDATA, OnCopyData)
#else
  ON_REGISTERED_MESSAGE(WM_COPYDATA, OnCopyData)
#endif
#ifdef FP_ADB
  ON_MESSAGE(SSM_FPDB_POSTMSG, OnDBPostMsg)
#endif
  ON_WM_NCDESTROY()    // SCP 7/22/99  Bug:SCS8735
  // Spread Designer
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CSsocxCtrl, COleControl)
    //{{AFX_DISPATCH_MAP(CSsocxCtrl)
    //}}AFX_DISPATCH_MAP
  #ifndef SSP_NO_ACTION
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Action", dispidAction, GetNotSupported, SetAction, VT_I2)
  #endif
  #ifndef SSP_NO_ACTIVECOL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ActiveCol", dispidActiveCol, GetActiveCol, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_ACTIVEROW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ActiveRow", dispidActiveRow, GetActiveRow, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_ALLOWCELLOVERFLOW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AllowCellOverflow", dispidAllowCellOverflow, GetAllowCellOverflow, SetAllowCellOverflow, VT_BOOL)
  #endif
  #ifndef SSP_NO_ALLOWDRAGDROP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AllowDragDrop", dispidAllowDragDrop, GetAllowDragDrop, SetAllowDragDrop, VT_BOOL)
  #endif
  #ifndef SSP_NO_ALLOWMULTIBLOCKS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AllowMultiBlocks", dispidAllowMultiBlocks, GetAllowMultiBlocks, SetAllowMultiBlocks, VT_BOOL)
  #endif
  #ifndef SSP_NO_ALLOWUSERFORMULAS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AllowUserFormulas", dispidAllowUserFormulas, GetAllowUserFormulas, SetAllowUserFormulas, VT_BOOL)
  #endif
  #ifndef SSP_NO_ARROWSEXITEDITMODE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ArrowsExitEditMode", dispidArrowsExitEditMode, GetArrowsExitEditMode, SetArrowsExitEditMode, VT_BOOL)
  #endif
  #ifndef SSP_NO_AUTOCALC
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AutoCalc", dispidAutoCalc, GetAutoCalc, SetAutoCalc, VT_BOOL)
  #endif
  #ifndef SSP_NO_AUTOCLIPBOARD
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AutoClipboard", dispidAutoClipboard, GetAutoClipboard, SetAutoClipboard, VT_BOOL)
  #endif
  #ifndef SSP_NO_AUTOSIZE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AutoSize", dispidAutoSize, GetAutoSize, SetAutoSize, VT_BOOL)
  #endif
  #ifndef SSP_NO_BLOCKMODE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "BlockMode", dispidBlockMode, GetBlockMode, SetBlockMode, VT_BOOL)
  #endif
  #ifndef SSP_NO_BUTTONDRAWMODE
#ifdef SS_V70
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ButtonDrawMode", dispidButtonDrawMode, GetButtonDrawMode, SetButtonDrawMode, VT_I4)
#else
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ButtonDrawMode", dispidButtonDrawMode, GetButtonDrawMode, SetButtonDrawMode, VT_I2)
#endif
  #endif
  #ifndef SSP_NO_CALCDEPENDENCIES
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CalcDependencies", dispidCalcDependencies, GetCalcDependencies, SetCalcDependencies, VT_BOOL)
  #endif
  #ifndef SSP_NO_CELLBORDERCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellBorderColor", dispidCellBorderColor, GetCellBorderColor, SetCellBorderColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_CELLBORDERSTYLE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellBorderStyle", dispidCellBorderStyle, GetCellBorderStyle, SetCellBorderStyle, VT_I2)
  #endif
  #ifndef SSP_NO_CELLBORDERTYPE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellBorderType", dispidCellBorderType, GetCellBorderType, SetCellBorderType, VT_I2)
  #endif
  #ifndef SSP_NO_CELLTYPE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellType", dispidCellType, GetCellType, SetCellType, VT_I2)
  #endif
  #ifndef SSP_NO_CHANGEMADE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ChangeMade", dispidChangeMade, GetChangeMade, SetChangeMade, VT_BOOL)
  #endif
  #ifndef SSP_NO_CLIP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Clip", dispidClip, GetClip, SetClip, VT_BSTR)
  #endif
  #ifndef SSP_NO_CLIPVALUE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ClipValue", dispidClipValue, GetClipValue, SetClipValue, VT_BSTR)
  #endif
  #ifndef SSP_NO_COL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Col", dispidCol, GetCol, SetCol, VT_I4)
  #endif
  #ifndef SSP_NO_COL2
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Col2", dispidCol2, GetCol2, SetCol2, VT_I4)
  #endif
  #ifndef SSP_NO_COLHEADERDISPLAY
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColHeaderDisplay", dispidColHeaderDisplay, GetColHeaderDisplay, SetColHeaderDisplay, VT_I2)
  #endif
  #ifndef SSP_NO_COLHIDDEN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColHidden", dispidColHidden, GetColHidden, SetColHidden, VT_BOOL)
  #endif
  #ifndef SSP_NO_COLPAGEBREAK
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColPageBreak", dispidColPageBreak, GetColPageBreak, SetColPageBreak, VT_BOOL)
  #endif
  #ifndef SSP_NO_COLSFROZEN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColsFrozen", dispidColsFrozen, GetColsFrozen, SetColsFrozen, VT_I4)
  #endif
  #ifndef SSP_NO_CURSORSTYLE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CursorStyle", dispidCursorStyle, GetCursorStyle, SetCursorStyle, VT_I2)
  #endif
  #ifndef SSP_NO_CURSORTYPE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CursorType", dispidCursorType, GetCursorType, SetCursorType, VT_I2)
  #endif
  #ifndef SSP_NO_DATACHANGED
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataChanged", dispidDataChanged, GetDataChanged, SetDataChanged, VT_BOOL)
  #endif  // DBOCX
  #ifndef SSP_NO_DATACOLCNT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataColCnt", dispidDataColCnt, GetDataColCnt, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_DATAFIELD
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataField", dispidDataField, GetDataField, SetDataField, VT_BSTR)
  #endif
  #ifndef SSP_NO_DATAFILLEVENT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataFillEvent", dispidDataFillEvent, GetDataFillEvent, SetDataFillEvent, VT_BOOL)
  #endif  // DBOCX
  #ifndef SSP_NO_DATAROWCNT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataRowCnt", dispidDataRowCnt, GetDataRowCnt, SetNotSupported, VT_I4)
  #endif  // DBOCX
  #ifndef SSP_NO_DATASOURCE
    #ifdef FP_ADB
      DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataSource", dispidDataSource, GetDataSource, SetDataSource, VT_UNKNOWN)  //DBOCX
    #else
      DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataSource", dispidDataSource, GetDataSource, SetDataSource, VT_UNKNOWN)  //DBOCX
    #endif
  #endif  // DBOCX
  #ifndef SSP_NO_DAUTOCELLTYPES
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DAutoCellTypes", dispidDAutoCellTypes, GetDAutoCellTypes, SetDAutoCellTypes, VT_BOOL)
  #endif
  #ifndef SSP_NO_DAUTOFILL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DAutoFill", dispidDAutoFill, GetDAutoFill, SetDAutoFill, VT_BOOL)
  #endif
  #ifndef SSP_NO_DAUTOHEADINGS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DAutoHeadings", dispidDAutoHeadings, GetDAutoHeadings, SetDAutoHeadings, VT_BOOL)
  #endif
  #ifndef SSP_NO_DAUTOSAVE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DAutoSave", dispidDAutoSave, GetDAutoSave, SetDAutoSave, VT_BOOL)
  #endif
  #ifndef SSP_NO_DAUTOSIZECOLS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DAutoSizeCols", dispidDAutoSizeCols, GetDAutoSizeCols, SetDAutoSizeCols, VT_I2) //DBOCX
  #endif
  #ifndef SSP_NO_DESTCOL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DestCol", dispidDestCol, GetDestCol, SetDestCol, VT_I4)
  #endif
  #ifndef SSP_NO_DESTROW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DestRow", dispidDestRow, GetDestRow, SetDestRow, VT_I4)
  #endif
  #ifndef SSP_NO_DINFORMACTIVEROWCHANGE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DInformActiveRowChange", dispidDInformActiveRowChange, GetDInformActiveRowChange, SetDInformActiveRowChange, VT_BOOL)
  #endif
  #ifndef SSP_NO_DISPLAYCOLHEADERS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DisplayColHeaders", dispidDisplayColHeaders, GetDisplayColHeaders, SetDisplayColHeaders, VT_BOOL)
  #endif
  #ifndef SSP_NO_DISPLAYROWHEADERS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DisplayRowHeaders", dispidDisplayRowHeaders, GetDisplayRowHeaders, SetDisplayRowHeaders, VT_BOOL)
  #endif
  #ifndef SSP_NO_EDITENTERACTION
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "EditEnterAction", dispidEditEnterAction, GetEditEnterAction, SetEditEnterAction, VT_I2)
  #endif
  #ifndef SSP_NO_EDITMODE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "EditMode", dispidEditMode, GetEditMode, SetEditMode, VT_BOOL)
  #endif
  #ifndef SSP_NO_EDITMODEPERMANENT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "EditModePermanent", dispidEditModePermanent, GetEditModePermanent, SetEditModePermanent, VT_BOOL)
  #endif
  #ifndef SSP_NO_EDITMODEREPLACE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "EditModeReplace", dispidEditModeReplace, GetEditModeReplace, SetEditModeReplace, VT_BOOL)
  #endif
  #ifndef SSP_NO_FILENUM
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "FileNum", dispidFileNum, GetFileNum, SetFileNum, VT_I2)
  #endif
  #ifndef SSP_NO_FLOATDEFCURRENCYCHAR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "FloatDefCurrencyChar", dispidFloatDefCurrencyChar, GetFloatDefCurrencyChar, SetFloatDefCurrencyChar, VT_I2)
  #endif
  #ifndef SSP_NO_FLOATDEFDECIMALCHAR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "FloatDefDecimalChar", dispidFloatDefDecimalChar, GetFloatDefDecimalChar, SetFloatDefDecimalChar, VT_I2)
  #endif
  #ifndef SSP_NO_FLOATDEFSEPCHAR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "FloatDefSepChar", dispidFloatDefSepChar, GetFloatDefSepChar, SetFloatDefSepChar, VT_I2)
  #endif
  #ifndef SSP_NO_FORMULA
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Formula", dispidFormula, GetFormula, SetFormula, VT_BSTR)
  #endif
  #ifndef SSP_NO_GRAYAREABACKCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "GrayAreaBackColor", dispidGrayAreaBackColor, GetGrayAreaBackColor, SetGrayAreaBackColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_GRIDCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "GridColor", dispidGridColor, GetGridColor, SetGridColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_GRIDSHOWHORIZ
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "GridShowHoriz", dispidGridShowHoriz, GetGridShowHoriz, SetGridShowHoriz, VT_BOOL)
  #endif
  #ifndef SSP_NO_GRIDSHOWVERT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "GridShowVert", dispidGridShowVert, GetGridShowVert, SetGridShowVert, VT_BOOL)
  #endif
  #ifndef SSP_NO_GRIDSOLID
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "GridSolid", dispidGridSolid, GetGridSolid, SetGridSolid, VT_BOOL)
  #endif
  #ifndef SSP_NO_HDCPRINTER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "hDCPrinter", dispidHDCPrinter, GetHDCPrinter, SetHDCPrinter, VT_HANDLE)
  #endif
  #ifndef SSP_NO_SPREADDESIGNER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SpreadDesigner", dispidInterfaceDesigner, GetNotSupported, SetNotSupported, VT_I2)
  #endif
  #ifndef SSP_NO_ISBLOCKSELECTED
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "IsBlockSelected", dispidIsBlockSelected, GetIsBlockSelected, SetNotSupported, VT_BOOL)
  #endif
  #ifndef SSP_NO_LEFTCOL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "LeftCol", dispidLeftCol, GetLeftCol, SetLeftCol, VT_I4)
  #endif
  #ifndef SSP_NO_LOCK
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Lock", dispidLock, GetLock, SetLock, VT_BOOL)
  #endif
  #ifndef SSP_NO_LOCKBACKCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "LockBackColor", dispidLockBackColor, GetLockBackColor, SetLockBackColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_LOCKFORECOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "LockForeColor", dispidLockForeColor, GetLockForeColor, SetLockForeColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_MAXCOLS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "MaxCols", dispidMaxCols, GetMaxCols, SetMaxCols, VT_I4)
  #endif
  #ifndef SSP_NO_MAXROWS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "MaxRows", dispidMaxRows, GetMaxRows, SetMaxRows, VT_I4)
  #endif
  #ifndef SSP_NO_MAXTEXTCELLHEIGHT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "MaxTextCellHeight", dispidMaxTextCellHeight, GetMaxTextCellHeight, SetNotSupported, VT_R8)
  #endif
  #ifndef SSP_NO_MAXTEXTCELLWIDTH
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "MaxTextCellWidth", dispidMaxTextCellWidth, GetMaxTextCellWidth, SetNotSupported, VT_R8)
  #endif
  #ifndef SSP_NO_MOVEACTIVEONFOCUS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "MoveActiveOnFocus", dispidMoveActiveOnFocus, GetMoveActiveOnFocus, SetMoveActiveOnFocus, VT_BOOL)
  #endif
  #ifndef SSP_NO_MULTISELCOUNT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "MultiSelCount", dispidMultiSelCount, GetMultiSelCount, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_MULTISELINDEX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "MultiSelIndex", dispidMultiSelIndex, GetMultiSelIndex, SetMultiSelIndex, VT_I4)
  #endif
  #ifndef SSP_NO_NOBEEP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "NoBeep", dispidNoBeep, GetNoBeep, SetNoBeep, VT_BOOL)
  #endif
  #ifndef SSP_NO_NOBORDER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "NoBorder", dispidNoBorder, GetNoBorder, SetNoBorder, VT_BOOL)
  #endif
  #ifndef SSP_NO_OPERATIONMODE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "OperationMode", dispidOperationMode, GetOperationMode, SetOperationMode, VT_I2)
  #endif
  #ifndef SSP_NO_POSITION
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Position", dispidPosition, GetPosition, SetPosition, VT_I2)
  #endif
  #ifndef SSP_NO_PRINTABORTMSG
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintAbortMsg", dispidPrintAbortMsg, GetPrintAbortMsg, SetPrintAbortMsg, VT_BSTR)
  #endif
  #ifndef SSP_NO_PRINTBORDER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintBorder", dispidPrintBorder, GetPrintBorder, SetPrintBorder, VT_BOOL)
  #endif
  #ifndef SSP_NO_PRINTCOLHEADERS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintColHeaders", dispidPrintColHeaders, GetPrintColHeaders, SetPrintColHeaders, VT_BOOL)
  #endif
  #ifndef SSP_NO_PRINTCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintColor", dispidPrintColor, GetPrintColor, SetPrintColor, VT_BOOL)
  #endif
  #ifndef SSP_NO_PRINTFOOTER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintFooter", dispidPrintFooter, GetPrintFooter, SetPrintFooter, VT_BSTR)
  #endif
  #ifndef SSP_NO_PRINTGRID
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintGrid", dispidPrintGrid, GetPrintGrid, SetPrintGrid, VT_BOOL)
  #endif
  #ifndef SSP_NO_PRINTHEADER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintHeader", dispidPrintHeader, GetPrintHeader, SetPrintHeader, VT_BSTR)
  #endif
  #ifndef SSP_NO_PRINTJOBNAME
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintJobName", dispidPrintJobName, GetPrintJobName, SetPrintJobName, VT_BSTR)
  #endif
  #ifndef SSP_NO_PRINTMARGINBOTTOM
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintMarginBottom", dispidPrintMarginBottom, GetPrintMarginBottom, SetPrintMarginBottom, VT_I4)
  #endif
  #ifndef SSP_NO_PRINTMARGINLEFT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintMarginLeft", dispidPrintMarginLeft, GetPrintMarginLeft, SetPrintMarginLeft, VT_I4)
  #endif
  #ifndef SSP_NO_PRINTMARGINRIGHT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintMarginRight", dispidPrintMarginRight, GetPrintMarginRight, SetPrintMarginRight, VT_I4)
  #endif
  #ifndef SSP_NO_PRINTMARGINTOP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintMarginTop", dispidPrintMarginTop, GetPrintMarginTop, SetPrintMarginTop, VT_I4)
  #endif
  #ifndef SSP_NO_PRINTPAGEEND
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintPageEnd", dispidPrintPageEnd, GetPrintPageEnd, SetPrintPageEnd, VT_I2)
  #endif
  #ifndef SSP_NO_PRINTPAGESTART
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintPageStart", dispidPrintPageStart, GetPrintPageStart, SetPrintPageStart, VT_I2)
  #endif
  #ifndef SSP_NO_PRINTROWHEADERS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintRowHeaders", dispidPrintRowHeaders, GetPrintRowHeaders, SetPrintRowHeaders, VT_BOOL)
  #endif
  #ifndef SSP_NO_PRINTSHADOWS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintShadows", dispidPrintShadows, GetPrintShadows, SetPrintShadows, VT_BOOL)
  #endif
  #ifndef SSP_NO_PRINTTYPE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintType", dispidPrintType, GetPrintType, SetPrintType, VT_I2)
  #endif
  #ifndef SSP_NO_PRINTUSEDATAMAX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintUseDataMax", dispidPrintUseDataMax, GetPrintUseDataMax, SetPrintUseDataMax, VT_BOOL)
  #endif
  #ifndef SSP_NO_PROCESSTAB
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ProcessTab", dispidProcessTab, GetProcessTab, SetProcessTab, VT_BOOL)
  #endif
  #ifndef SSP_NO_PROTECT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Protect", dispidProtect, GetProtect, SetProtect, VT_BOOL)
  #endif
  #ifndef SSP_NO_REDRAW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ReDraw", dispidReDraw, GetReDraw, SetReDraw, VT_BOOL)
  #endif
  #ifndef SSP_NO_RESTRICTCOLS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RestrictCols", dispidRestrictCols, GetRestrictCols, SetRestrictCols, VT_BOOL)
  #endif
  #ifndef SSP_NO_RESTRICTROWS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RestrictRows", dispidRestrictRows, GetRestrictRows, SetRestrictRows, VT_BOOL)
  #endif
  #ifndef SSP_NO_RETAINSELBLOCK
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RetainSelBlock", dispidRetainSelBlock, GetRetainSelBlock, SetRetainSelBlock, VT_BOOL)
  #endif
  #ifndef SSP_NO_ROW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Row", dispidRow, GetRow, SetRow, VT_I4)
  #endif
  #ifndef SSP_NO_ROW2
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Row2", dispidRow2, GetRow2, SetRow2, VT_I4)
  #endif
  #ifndef SSP_NO_ROWHEADERDISPLAY
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowHeaderDisplay", dispidRowHeaderDisplay, GetRowHeaderDisplay, SetRowHeaderDisplay, VT_I2)
  #endif
  #ifndef SSP_NO_ROWHIDDEN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowHidden", dispidRowHidden, GetRowHidden, SetRowHidden, VT_BOOL)
  #endif
  #ifndef SSP_NO_ROWPAGEBREAK
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowPageBreak", dispidRowPageBreak, GetRowPageBreak, SetRowPageBreak, VT_BOOL)
  #endif
  #ifndef SSP_NO_ROWSFROZEN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowsFrozen", dispidRowsFrozen, GetRowsFrozen, SetRowsFrozen, VT_I4)
  #endif
  #ifndef SSP_NO_SCROLLBAREXTMODE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarExtMode", dispidScrollBarExtMode, GetScrollBarExtMode, SetScrollBarExtMode, VT_BOOL)
  #endif
  #ifndef SSP_NO_SCROLLBARMAXALIGN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarMaxAlign", dispidScrollBarMaxAlign, GetScrollBarMaxAlign, SetScrollBarMaxAlign, VT_BOOL)
  #endif
  #ifndef SSP_NO_SCROLLBARS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBars", dispidScrollBars, GetScrollBars, SetScrollBars, VT_I2)
  #endif
  #ifndef SSP_NO_SCROLLBARSHOWMAX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarShowMax", dispidScrollBarShowMax, GetScrollBarShowMax, SetScrollBarShowMax, VT_BOOL)
  #endif
  #ifndef SSP_NO_SELBLOCKCOL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelBlockCol", dispidSelBlockCol, GetSelBlockCol, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_SELBLOCKCOL2
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelBlockCol2", dispidSelBlockCol2, GetSelBlockCol2, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_SELBLOCKROW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelBlockRow", dispidSelBlockRow, GetSelBlockRow, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_SELBLOCKROW2
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelBlockRow2", dispidSelBlockRow2, GetSelBlockRow2, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_SELECTBLOCKOPTIONS
#ifdef SS_V70
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelectBlockOptions", dispidSelectBlockOptions, GetSelectBlockOptions, SetSelectBlockOptions, VT_I4)
#else
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelectBlockOptions", dispidSelectBlockOptions, GetSelectBlockOptions, SetSelectBlockOptions, VT_I2)
#endif
  #endif
  #ifndef SSP_NO_SELLENGTH
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelLength", dispidSelLength, GetSelLength, SetSelLength, VT_I4)
  #endif
  #ifndef SSP_NO_SELMODEINDEX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelModeIndex", dispidSelModeIndex, GetSelModeIndex, SetSelModeIndex, VT_I4)
  #endif
  #ifndef SSP_NO_SELMODESELCOUNT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelModeSelCount", dispidSelModeSelCount, GetSelModeSelCount, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_SELMODESELECTED
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelModeSelected", dispidSelModeSelected, GetSelModeSelected, SetSelModeSelected, VT_BOOL)
  #endif
  #ifndef SSP_NO_SELSTART
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelStart", dispidSelStart, GetSelStart, SetSelStart, VT_I4)
  #endif
  #ifndef SSP_NO_SELTEXT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelText", dispidSelText, GetSelText, SetSelText, VT_BSTR)
  #endif
  #ifndef SSP_NO_SHADOWCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ShadowColor", dispidShadowColor, GetShadowColor, SetShadowColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_SHADOWDARK
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ShadowDark", dispidShadowDark, GetShadowDark, SetShadowDark, VT_COLOR)
  #endif
  #ifndef SSP_NO_SHADOWTEXT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ShadowText", dispidShadowText, GetShadowText, SetShadowText, VT_COLOR)
  #endif
  #ifndef SSP_NO_SORTBY
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SortBy", dispidSortBy, GetSortBy, SetSortBy, VT_I2)
  #endif
  #ifndef SSP_NO_STARTINGCOLNUMBER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "StartingColNumber", dispidStartingColNumber, GetStartingColNumber, SetStartingColNumber, VT_I4)
  #endif
  #ifndef SSP_NO_STARTINGROWNUMBER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "StartingRowNumber", dispidStartingRowNumber, GetStartingRowNumber, SetStartingRowNumber, VT_I4)
  #endif
  #ifndef SSP_NO_TOPROW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TopRow", dispidTopRow, GetTopRow, SetTopRow, VT_I4)
  #endif
  #ifndef SSP_NO_TYPEBUTTONALIGN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonAlign", dispidTypeButtonAlign, GetTypeButtonAlign, SetTypeButtonAlign, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEBUTTONBORDERCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonBorderColor", dispidTypeButtonBorderColor, GetTypeButtonBorderColor, SetTypeButtonBorderColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_TYPEBUTTONCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonColor", dispidTypeButtonColor, GetTypeButtonColor, SetTypeButtonColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_TYPEBUTTONDARKCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonDarkColor", dispidTypeButtonDarkColor, GetTypeButtonDarkColor, SetTypeButtonDarkColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_TYPEBUTTONLIGHTCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonLightColor", dispidTypeButtonLightColor, GetTypeButtonLightColor, SetTypeButtonLightColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_TYPEBUTTONPICTURE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonPicture", dispidTypeButtonPicture, GetTypeButtonPicture, SetTypeButtonPicture, VT_PICTURE)
  #endif
  #ifndef SSP_NO_TYPEBUTTONPICTUREDOWN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonPictureDown", dispidTypeButtonPictureDown, GetTypeButtonPictureDown, SetTypeButtonPictureDown, VT_PICTURE)
  #endif
  #ifndef SSP_NO_TYPEBUTTONSHADOWSIZE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonShadowSize", dispidTypeButtonShadowSize, GetTypeButtonShadowSize, SetTypeButtonShadowSize, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEBUTTONTEXT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonText", dispidTypeButtonText, GetTypeButtonText, SetTypeButtonText, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPEBUTTONTEXTCOLOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonTextColor", dispidTypeButtonTextColor, GetTypeButtonTextColor, SetTypeButtonTextColor, VT_COLOR)
  #endif
  #ifndef SSP_NO_TYPEBUTTONTYPE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeButtonType", dispidTypeButtonType, GetTypeButtonType, SetTypeButtonType, VT_I2)
  #endif
  #ifndef SSP_NO_TYPECHECKCENTER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCheckCenter", dispidTypeCheckCenter, GetTypeCheckCenter, SetTypeCheckCenter, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPECHECKTEXT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCheckText", dispidTypeCheckText, GetTypeCheckText, SetTypeCheckText, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPECHECKTEXTALIGN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCheckTextAlign", dispidTypeCheckTextAlign, GetTypeCheckTextAlign, SetTypeCheckTextAlign, VT_I2)
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXCOUNT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxCount", dispidTypeComboBoxCount, GetTypeComboBoxCount, SetNotSupported, VT_I2)
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXCURSEL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxCurSel", dispidTypeComboBoxCurSel, GetTypeComboBoxCurSel, SetTypeComboBoxCurSel, VT_I2)
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXEDITABLE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxEditable", dispidTypeComboBoxEditable, GetTypeComboBoxEditable, SetTypeComboBoxEditable, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXINDEX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxIndex", dispidTypeComboBoxIndex, GetTypeComboBoxIndex, SetTypeComboBoxIndex, VT_I2)
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXLIST
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxList", dispidTypeComboBoxList, GetTypeComboBoxList, SetTypeComboBoxList, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXSTRING
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxString", dispidTypeComboBoxString, GetTypeComboBoxString, SetTypeComboBoxString, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPEDATECENTURY
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeDateCentury", dispidTypeDateCentury, GetTypeDateCentury, SetTypeDateCentury, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEDATEFORMAT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeDateFormat", dispidTypeDateFormat, GetTypeDateFormat, SetTypeDateFormat, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEDATEMAX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeDateMax", dispidTypeDateMax, GetTypeDateMax, SetTypeDateMax, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPEDATEMIN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeDateMin", dispidTypeDateMin, GetTypeDateMin, SetTypeDateMin, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPEDATESEPARATOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeDateSeparator", dispidTypeDateSeparator, GetTypeDateSeparator, SetTypeDateSeparator, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEEDITCHARCASE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeEditCharCase", dispidTypeEditCharCase, GetTypeEditCharCase, SetTypeEditCharCase, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEEDITCHARSET
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeEditCharSet", dispidTypeEditCharSet, GetTypeEditCharSet, SetTypeEditCharSet, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEEDITLEN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeEditLen", dispidTypeEditLen, GetTypeEditLen, SetTypeEditLen, VT_I4)
  #endif
  #ifndef SSP_NO_TYPEEDITMULTILINE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeEditMultiLine", dispidTypeEditMultiLine, GetTypeEditMultiLine, SetTypeEditMultiLine, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEEDITPASSWORD
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeEditPassword", dispidTypeEditPassword, GetTypeEditPassword, SetTypeEditPassword, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEFLOATCURRENCYCHAR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatCurrencyChar", dispidTypeFloatCurrencyChar, GetTypeFloatCurrencyChar, SetTypeFloatCurrencyChar, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEFLOATDECIMALCHAR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatDecimalChar", dispidTypeFloatDecimalChar, GetTypeFloatDecimalChar, SetTypeFloatDecimalChar, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEFLOATDECIMALPLACES
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatDecimalPlaces", dispidTypeFloatDecimalPlaces, GetTypeFloatDecimalPlaces, SetTypeFloatDecimalPlaces, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEFLOATMAX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatMax", dispidTypeFloatMax, GetTypeFloatMax, SetTypeFloatMax, VT_R8)
  #endif
  #ifndef SSP_NO_TYPEFLOATMIN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatMin", dispidTypeFloatMin, GetTypeFloatMin, SetTypeFloatMin, VT_R8)
  #endif
  #ifndef SSP_NO_TYPEFLOATMONEY
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatMoney", dispidTypeFloatMoney, GetTypeFloatMoney, SetTypeFloatMoney, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEFLOATSEPARATOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatSeparator", dispidTypeFloatSeparator, GetTypeFloatSeparator, SetTypeFloatSeparator, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEFLOATSEPCHAR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeFloatSepChar", dispidTypeFloatSepChar, GetTypeFloatSepChar, SetTypeFloatSepChar, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEHALIGN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeHAlign", dispidTypeHAlign, GetTypeHAlign, SetTypeHAlign, VT_I2)
  #endif
  #ifndef SSP_NO_TYPEINTEGERMAX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeIntegerMax", dispidTypeIntegerMax, GetTypeIntegerMax, SetTypeIntegerMax, VT_I4)
  #endif
  #ifndef SSP_NO_TYPEINTEGERMIN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeIntegerMin", dispidTypeIntegerMin, GetTypeIntegerMin, SetTypeIntegerMin, VT_I4)
  #endif
  #ifndef SSP_NO_TYPEINTEGERSPININC
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeIntegerSpinInc", dispidTypeIntegerSpinInc, GetTypeIntegerSpinInc, SetTypeIntegerSpinInc, VT_I4)
  #endif
  #ifndef SSP_NO_TYPEINTEGERSPINWRAP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeIntegerSpinWrap", dispidTypeIntegerSpinWrap, GetTypeIntegerSpinWrap, SetTypeIntegerSpinWrap, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEOWNERDRAWSTYLE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeOwnerDrawStyle", dispidTypeOwnerDrawStyle, GetTypeOwnerDrawStyle, SetTypeOwnerDrawStyle, VT_I4)
  #endif
  #ifndef SSP_NO_TYPEPICDEFAULTTEXT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePicDefaultText", dispidTypePicDefaultText, GetTypePicDefaultText, SetTypePicDefaultText, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPEPICMASK
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePicMask", dispidTypePicMask, GetTypePicMask, SetTypePicMask, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPEPICTCENTER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePictCenter", dispidTypePictCenter, GetTypePictCenter, SetTypePictCenter, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEPICTMAINTAINSCALE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePictMaintainScale", dispidTypePictMaintainScale, GetTypePictMaintainScale, SetTypePictMaintainScale, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPEPICTPICTURE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePictPicture", dispidTypePictPicture, GetTypePictPicture, SetTypePictPicture, VT_PICTURE)
  #endif
  #ifndef SSP_NO_TYPEPICTSTRETCH
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePictStretch", dispidTypePictStretch, GetTypePictStretch, SetTypePictStretch, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPESPIN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeSpin", dispidTypeSpin, GetTypeSpin, SetTypeSpin, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPETEXTALIGNVERT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTextAlignVert", dispidTypeTextAlignVert, GetTypeTextAlignVert, SetTypeTextAlignVert, VT_I2)
  #endif
  #ifndef SSP_NO_TYPETEXTPREFIX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTextPrefix", dispidTypeTextPrefix, GetTypeTextPrefix, SetTypeTextPrefix, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPETEXTSHADOW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTextShadow", dispidTypeTextShadow, GetTypeTextShadow, SetTypeTextShadow, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPETEXTSHADOWIN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTextShadowIn", dispidTypeTextShadowIn, GetTypeTextShadowIn, SetTypeTextShadowIn, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPETEXTWORDWRAP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTextWordWrap", dispidTypeTextWordWrap, GetTypeTextWordWrap, SetTypeTextWordWrap, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPETIME24HOUR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTime24Hour", dispidTypeTime24Hour, GetTypeTime24Hour, SetTypeTime24Hour, VT_I2)
  #endif
  #ifndef SSP_NO_TYPETIMEMAX
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTimeMax", dispidTypeTimeMax, GetTypeTimeMax, SetTypeTimeMax, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPETIMEMIN
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTimeMin", dispidTypeTimeMin, GetTypeTimeMin, SetTypeTimeMin, VT_BSTR)
  #endif
  #ifndef SSP_NO_TYPETIMESECONDS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTimeSeconds", dispidTypeTimeSeconds, GetTypeTimeSeconds, SetTypeTimeSeconds, VT_BOOL)
  #endif
  #ifndef SSP_NO_TYPETIMESEPARATOR
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTimeSeparator", dispidTypeTimeSeparator, GetTypeTimeSeparator, SetTypeTimeSeparator, VT_I2)
  #endif
  #ifndef SSP_NO_UNITTYPE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "UnitType", dispidUnitType, GetUnitType, SetUnitType, VT_I2)
  #endif
  #ifndef SSP_NO_USERRESIZE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "UserResize", dispidUserResize, GetUserResize, SetUserResize, VT_I2)
  #endif
  #ifndef SSP_NO_USERRESIZECOL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "UserResizeCol", dispidUserResizeCol, GetUserResizeCol, SetUserResizeCol, VT_I2)
  #endif
  #ifndef SSP_NO_USERRESIZEROW
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "UserResizeRow", dispidUserResizeRow, GetUserResizeRow, SetUserResizeRow, VT_I2)
  #endif
  #ifndef SSP_NO_VALUE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Value", dispidValue, GetValue, SetValue, VT_BSTR)
  #endif
  #ifndef SSP_NO_VIRTUALCURROWCOUNT
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VirtualCurRowCount", dispidVirtualCurRowCount, GetVirtualCurRowCount, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_VIRTUALCURTOP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VirtualCurTop", dispidVirtualCurTop, GetVirtualCurTop, SetNotSupported, VT_I4)
  #endif
  #ifndef SSP_NO_VIRTUALMAXROWS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VirtualMaxRows", dispidVirtualMaxRows, GetVirtualMaxRows, SetVirtualMaxRows, VT_I4)
  #endif
  #ifndef SSP_NO_VIRTUALMODE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VirtualMode", dispidVirtualMode, GetVirtualMode, SetVirtualMode, VT_BOOL)
  #endif
  #ifndef SSP_NO_VIRTUALOVERLAP
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VirtualOverlap", dispidVirtualOverlap, GetVirtualOverlap, SetVirtualOverlap, VT_I4)
  #endif
  #ifndef SSP_NO_VIRTUALROWS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VirtualRows", dispidVirtualRows, GetVirtualRows, SetVirtualRows, VT_I4)
  #endif
  #ifndef SSP_NO_VIRTUALSCROLLBUFFER
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VirtualScrollBuffer", dispidVirtualScrollBuffer, GetVirtualScrollBuffer, SetVirtualScrollBuffer, VT_BOOL)
  #endif
  #ifndef SSP_NO_VISIBLECOLS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VisibleCols", dispidVisibleCols, GetVisibleCols, SetVisibleCols, VT_I4)
  #endif
  #ifndef SSP_NO_VISIBLEROWS
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VisibleRows", dispidVisibleRows, GetVisibleRows, SetVisibleRows, VT_I4)
  #endif
  #ifndef SSP_NO_VSCROLLSPECIAL
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VScrollSpecial", dispidVScrollSpecial, GetVScrollSpecial, SetVScrollSpecial, VT_BOOL)
  #endif
  #ifndef SSP_NO_VSCROLLSPECIALTYPE
#ifdef SS_V70
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "VScrollSpecialType", dispidVScrollSpecialType, GetVScrollSpecialType, SetVScrollSpecialType, VT_I4)
#else
	DISP_PROPERTY_EX_ID(CSsocxCtrl, "VScrollSpecialType", dispidVScrollSpecialType, GetVScrollSpecialType, SetVScrollSpecialType, VT_I2)
#endif
  #endif
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "FontBold", dispidFontBold, GetFontBold, SetFontBold, VT_BOOL)
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "FontItalic", dispidFontItalic, GetFontItalic, SetFontItalic, VT_BOOL)
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "FontName", dispidFontName, GetFontName, SetFontName, VT_BSTR)
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "FontSize", dispidFontSize, GetFontSize, SetFontSize, VT_R4)
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "FontStrikethru", dispidFontStrikethru, GetFontStrikethru, SetFontStrikethru, VT_BOOL)
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "FontUnderline", dispidFontUnderline, GetFontUnderline, SetFontUnderline, VT_BOOL)
  #ifndef SSP_NO_CURSORICON
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CursorIcon", dispidCursorIcon, GetCursorIcon, SetCursorIcon, VT_PICTURE)
  #endif
  #ifndef SSP_NO_PRINTORIENTATION
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintOrientation", dispidPrintOrientation, GetPrintOrientation, SetPrintOrientation, VT_I2)
  #endif
  #ifndef SSP_NO_BACKCOLORSTYLE
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "BackColorStyle", dispidBackColorStyle, GetBackColorStyle, SetBackColorStyle, VT_I2)
  #endif
  #ifndef SSP_NO_FORMULASYNC
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "FormulaSync", dispidFormulaSync, GetFormulaSync, SetFormulaSync, VT_BOOL)
  #endif
  #ifndef SSP_NO_COLWIDTH
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "ColWidth", dispidColWidth, GetColWidth, SetColWidth, VT_R8, VTS_I4)
  #endif
  #ifndef SSP_NO_MAXTEXTCOLWIDTH
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "MaxTextColWidth", dispidMaxTextColWidth, GetMaxTextColWidth, SetNotSupported, VT_R8, VTS_I4)
  #endif
  #ifndef SSP_NO_MAXTEXTROWHEIGHT
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "MaxTextRowHeight", dispidMaxTextRowHeight, GetMaxTextRowHeight, SetNotSupported, VT_R8, VTS_I4)
  #endif
  #ifndef SSP_NO_ROWHEIGHT
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "RowHeight", dispidRowHeight, GetRowHeight, SetRowHeight, VT_R8, VTS_I4)
  #endif
  #ifndef SSP_NO_SORTKEY
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "SortKey", dispidSortKey, GetSortKey, SetSortKey, VT_I4, VTS_I2)
  #endif
  #ifndef SSP_NO_SORTKEYORDER
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "SortKeyOrder", dispidSortKeyOrder, GetSortKeyOrder, SetSortKeyOrder, VT_I2, VTS_I2)
  #endif
  #ifndef SSP_NO_TYPECHECKPICTURE
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "TypeCheckPicture", dispidTypeCheckPicture, GetTypeCheckPicture, SetTypeCheckPicture, VT_PICTURE, VTS_I2)
  #endif
  #ifndef SSF_NO_AddCustomFunction
    DISP_FUNCTION_ID(CSsocxCtrl, "AddCustomFunction", dispidAddCustomFunction, AddCustomFunction, VT_BOOL, VTS_BSTR VTS_I2)
  #endif
  #ifndef SSF_NO_AddCustomFunctionExt
    DISP_FUNCTION_ID(CSsocxCtrl, "AddCustomFunctionExt", dispidAddCustomFunctionExt, AddCustomFunctionExt, VT_BOOL, VTS_BSTR VTS_I2 VTS_I2 VTS_I4)
  #endif
  #ifndef SSF_NO_CFGetCellParam
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetCellParam", dispidCFGetCellParam, CFGetCellParam, VT_EMPTY, VTS_I2 VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_CFGetDoubleParam
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetDoubleParam", dispidCFGetDoubleParam, CFGetDoubleParam, VT_R8, VTS_I2)
  #endif
  #ifndef SSF_NO_CFGetDoubleParamExt
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetDoubleParamExt", dispidCFGetDoubleParamExt, CFGetDoubleParamExt, VT_R8, VTS_I2 VTS_PR8)
  #endif
  #ifndef SSF_NO_CFGetLongParam
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetLongParam", dispidCFGetLongParam, CFGetLongParam, VT_I4, VTS_I2)
  #endif
  #ifndef SSF_NO_CFGetParamInfo
#ifdef SS_V70
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetParamInfo", dispidCFGetParamInfo, CFGetParamInfo, VT_BOOL, VTS_I2 VTS_PI4 VTS_PI4)
#else
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetParamInfo", dispidCFGetParamInfo, CFGetParamInfo, VT_BOOL, VTS_I2 VTS_PI2 VTS_PI2)
#endif
  #endif
  #ifndef SSF_NO_CFGetRangeParam
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetRangeParam", dispidCFGetRangeParam, CFGetRangeParam, VT_EMPTY, VTS_I2 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_CFGetStringParam
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetStringParam", dispidCFGetStringParam, CFGetStringParam, VT_BSTR, VTS_I2)
  #endif
  #ifndef SSF_NO_CFSetResult
    DISP_FUNCTION_ID(CSsocxCtrl, "CFSetResult", dispidCFSetResult, CFSetResult, VT_EMPTY, VTS_VARIANT)
  #endif
  #ifndef SSF_NO_ColNumberToLetter
    DISP_FUNCTION_ID(CSsocxCtrl, "ColNumberToLetter", dispidColNumberToLetter, ColNumberToLetter, VT_BSTR, VTS_I4)
  #endif
  #ifndef SSF_NO_ColWidthToTwips
    DISP_FUNCTION_ID(CSsocxCtrl, "ColWidthToTwips", dispidColWidthToTwips, ColWidthToTwips, VT_EMPTY, VTS_R4 VTS_PI4)
  #endif
  #ifndef SSF_NO_GetBottomRightCell
    DISP_FUNCTION_ID(CSsocxCtrl, "GetBottomRightCell", dispidGetBottomRightCell, GetBottomRightCell, VT_EMPTY, VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_GetCellDirtyFlag
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCellDirtyFlag", dispidGetCellDirtyFlag, GetCellDirtyFlag, VT_BOOL, VTS_I4 VTS_I4)
  #endif
  #ifndef SSF_NO_GetCellFromScreenCoord
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCellFromScreenCoord", dispidGetCellFromScreenCoord, GetCellFromScreenCoord, VT_EMPTY, VTS_PI4 VTS_PI4 VTS_I4 VTS_I4)
  #endif
  #ifndef SSF_NO_GetCellPos
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCellPos", dispidGetCellPos, GetCellPos, VT_BOOL, VTS_I4 VTS_I4 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_GetClientArea
    DISP_FUNCTION_ID(CSsocxCtrl, "GetClientArea", dispidGetClientArea, GetClientArea, VT_EMPTY, VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_GetColItemData
    DISP_FUNCTION_ID(CSsocxCtrl, "GetColItemData", dispidGetColItemData, GetColItemData, VT_I4, VTS_I4)
  #endif
  #ifndef SSF_NO_QueryCustomName
    DISP_FUNCTION_ID(CSsocxCtrl, "QueryCustomName", dispidQueryCustomName, QueryCustomName, VT_BSTR, VTS_BSTR)
  #endif
  #ifndef SSF_NO_GetCustomName
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCustomName", dispidGetCustomName, GetCustomName, VT_BSTR, VTS_BSTR)
  #endif
  #ifndef SSF_NO_GetDataFillData
    DISP_FUNCTION_ID(CSsocxCtrl, "GetDataFillData", dispidGetDataFillData, GetDataFillData, VT_BOOL, VTS_PVARIANT VTS_I2)
  #endif
  #ifndef SSF_NO_GetFirstValidCell
    DISP_FUNCTION_ID(CSsocxCtrl, "GetFirstValidCell", dispidGetFirstValidCell, GetFirstValidCell, VT_EMPTY, VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_GetItemData
    DISP_FUNCTION_ID(CSsocxCtrl, "GetItemData", dispidGetItemData, GetItemData, VT_I4, VTS_NONE)
  #endif
  #ifndef SSF_NO_GetIteration
    DISP_FUNCTION_ID(CSsocxCtrl, "GetIteration", dispidGetIteration, GetIteration, VT_BOOL, VTS_PI2 VTS_PR8)
  #endif
  #ifndef SSF_NO_GetLastValidCell
    DISP_FUNCTION_ID(CSsocxCtrl, "GetLastValidCell", dispidGetLastValidCell, GetLastValidCell, VT_EMPTY, VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_GetMultiSelItem
    DISP_FUNCTION_ID(CSsocxCtrl, "GetMultiSelItem", dispidGetMultiSelItem, GetMultiSelItem, VT_I4, VTS_I4)
  #endif
  #ifndef SSF_NO_GetRefStyle
#ifdef SS_V70
    DISP_FUNCTION_ID(CSsocxCtrl, "GetRefStyle", dispidGetRefStyle, GetRefStyle, VT_I4, VTS_NONE)
#else
    DISP_FUNCTION_ID(CSsocxCtrl, "GetRefStyle", dispidGetRefStyle, GetRefStyle, VT_I2, VTS_NONE)
#endif
  #endif
  #ifndef SSF_NO_GetRowItemData
    DISP_FUNCTION_ID(CSsocxCtrl, "GetRowItemData", dispidGetRowItemData, GetRowItemData, VT_I4, VTS_I4)
  #endif
  #ifndef SSF_NO_GetText
    DISP_FUNCTION_ID(CSsocxCtrl, "GetText", dispidGetText, fpGetText, VT_BOOL, VTS_I4 VTS_I4 VTS_PVARIANT)
  #endif
  #ifndef SSF_NO_IsCellSelected
    DISP_FUNCTION_ID(CSsocxCtrl, "IsCellSelected", dispidIsCellSelected, IsCellSelected, VT_BOOL, VTS_I4 VTS_I4)
  #endif
  #ifndef SSF_NO_IsFormulaValid
    DISP_FUNCTION_ID(CSsocxCtrl, "IsFormulaValid", dispidIsFormulaValid, IsFormulaValid, VT_BOOL, VTS_BSTR)
  #endif
  #ifndef SSF_NO_IsVisible
    DISP_FUNCTION_ID(CSsocxCtrl, "IsVisible", dispidIsVisible, IsVisible, VT_BOOL, VTS_I4 VTS_I4 VTS_BOOL)
  #endif
  #ifndef SSF_NO_LoadFromFile
    DISP_FUNCTION_ID(CSsocxCtrl, "LoadFromFile", dispidLoadFromFile, LoadFromFile, VT_BOOL, VTS_BSTR)
  #endif
  #ifndef SSF_NO_LoadTabFile
    DISP_FUNCTION_ID(CSsocxCtrl, "LoadTabFile", dispidLoadTabFile, LoadTabFile, VT_BOOL, VTS_BSTR)
  #endif
  #ifndef SSF_NO_RowHeightToTwips
    DISP_FUNCTION_ID(CSsocxCtrl, "RowHeightToTwips", dispidRowHeightToTwips, RowHeightToTwips, VT_EMPTY, VTS_I4 VTS_R4 VTS_PI4)
  #endif
  #ifndef SSF_NO_SaveTabFile
    DISP_FUNCTION_ID(CSsocxCtrl, "SaveTabFile", dispidSaveTabFile, SaveTabFile, VT_BOOL, VTS_BSTR)
  #endif
  #ifndef SSF_NO_SaveToFile
    DISP_FUNCTION_ID(CSsocxCtrl, "SaveToFile", dispidSaveToFile, SaveToFile, VT_BOOL, VTS_BSTR VTS_BOOL)
  #endif
  #ifndef SSF_NO_SetCellDirtyFlag
    DISP_FUNCTION_ID(CSsocxCtrl, "SetCellDirtyFlag", dispidSetCellDirtyFlag, SetCellDirtyFlag, VT_BOOL, VTS_I4 VTS_I4 VTS_BOOL)
  #endif
  #ifndef SSF_NO_SetColItemData
    DISP_FUNCTION_ID(CSsocxCtrl, "SetColItemData", dispidSetColItemData, SetColItemData, VT_EMPTY, VTS_I4 VTS_I4)
  #endif
  #ifndef SSF_NO_SetCustomName
    DISP_FUNCTION_ID(CSsocxCtrl, "SetCustomName", dispidSetCustomName, SetCustomName, VT_BOOL, VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_SetDataFillData
    DISP_FUNCTION_ID(CSsocxCtrl, "SetDataFillData", dispidSetDataFillData, SetDataFillData, VT_BOOL, VTS_VARIANT)
  #endif
  #ifndef SSF_NO_SetItemData
    DISP_FUNCTION_ID(CSsocxCtrl, "SetItemData", dispidSetItemData, SetItemData, VT_EMPTY, VTS_I4)
  #endif
  #ifndef SSF_NO_SetIteration
    DISP_FUNCTION_ID(CSsocxCtrl, "SetIteration", dispidSetIteration, SetIteration, VT_EMPTY, VTS_BOOL VTS_I2 VTS_R8)
  #endif
  #ifndef SSF_NO_SetRefStyle
#ifdef SS_V70
    DISP_FUNCTION_ID(CSsocxCtrl, "SetRefStyle", dispidSetRefStyle, SetRefStyle, VT_EMPTY, VTS_I4)
#else
    DISP_FUNCTION_ID(CSsocxCtrl, "SetRefStyle", dispidSetRefStyle, SetRefStyle, VT_EMPTY, VTS_I2)
#endif
  #endif
  #ifndef SSF_NO_SetRowItemData
    DISP_FUNCTION_ID(CSsocxCtrl, "SetRowItemData", dispidSetRowItemData, SetRowItemData, VT_EMPTY, VTS_I4 VTS_I4)
  #endif
  #ifndef SSF_NO_SetText
    DISP_FUNCTION_ID(CSsocxCtrl, "SetText", dispidSetText, fpSetText, VT_EMPTY, VTS_I4 VTS_I4 VTS_VARIANT)
  #endif
  #ifndef SSF_NO_TwipsToColWidth
    DISP_FUNCTION_ID(CSsocxCtrl, "TwipsToColWidth", dispidTwipsToColWidth, TwipsToColWidth, VT_EMPTY, VTS_I4 VTS_PR4)
  #endif
  #ifndef SSF_NO_TwipsToRowHeight
    DISP_FUNCTION_ID(CSsocxCtrl, "TwipsToRowHeight", dispidTwipsToRowHeight, TwipsToRowHeight, VT_EMPTY, VTS_I4 VTS_I4 VTS_PR4)
  #endif
  #ifndef SSP_NO_TYPECHECKTYPE
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCheckType", dispidTypeCheckType, GetTypeCheckType, SetTypeCheckType, VT_I2)
  #endif
  #ifndef SSF_NO_DdeQueryLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeQueryLink", dispidDdeQueryLink, DdeQueryLink, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_PBSTR VTS_PBSTR VTS_PBSTR)
  #endif
  #ifndef SSF_NO_DdeAddLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeAddLink", dispidDdeAddLink, DdeAddLink, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeRemoveLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeRemoveLink", dispidDdeRemoveLink, DdeRemoveLink, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeRequestLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeRequestLink", dispidDdeRequestLink, DdeRequestLink, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdePokeLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdePokeLink", dispidDdePokeLink, DdePokeLink, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdePokeDataToLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdePokeDataToLink", dispidDdePokeDataToLink, DdePokeDataToLink, VT_BOOL, VTS_I4 VTS_I4)
  #endif
  #ifndef SSF_NO_DdeGetMode
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeGetMode", dispidDdeGetMode, DdeGetMode, VT_I2, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeSetMode
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeSetMode", dispidDdeSetMode, DdeSetMode, VT_I2, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I2)
  #endif
  #ifndef SSF_NO_DdeGetReadExpr
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeGetReadExpr", dispidDdeGetReadExpr, DdeGetReadExpr, VT_BSTR, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeSetReadExpr
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeSetReadExpr", dispidDdeSetReadExpr, DdeSetReadExpr, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeGetWriteExpr
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeGetWriteExpr", dispidDdeGetWriteExpr, DdeGetWriteExpr, VT_BSTR, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeSetWriteExpr
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeSetWriteExpr", dispidDdeSetWriteExpr, DdeSetWriteExpr, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeGetLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeGetLink", dispidDdeGetLink, DdeGetLink, VT_BOOL, VTS_I4 VTS_I4 VTS_PBSTR VTS_PBSTR VTS_PBSTR VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_DdeSetLink
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeSetLink", dispidDdeSetLink, DdeSetLink, VT_BOOL, VTS_I4 VTS_I4 VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_I4)
  #endif
  #ifndef SSF_NO_DdeQueryDependCell  
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeQueryDependCell", dispidDdeQueryDependCell, DdeQueryDependCell, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_DdeQueryDependCol  
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeQueryDependCol", dispidDdeQueryDependCol, DdeQueryDependCol, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_PI4)
  #endif
  #ifndef SSF_NO_DdeQueryDependRow  
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeQueryDependRow", dispidDdeQueryDependRow, DdeQueryDependRow, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_PI4)
  #endif
  #ifndef SSF_NO_DdeQueryDependName  
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeQueryDependName", dispidDdeQueryDependName, DdeQueryDependName, VT_BSTR, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_DdeGetDim  
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeGetDim", dispidDdeGetDim, DdeGetDim, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_PI4 VTS_PI4)
  #endif
  #ifndef SSF_NO_DdeIsLink  
    DISP_FUNCTION_ID(CSsocxCtrl, "DdeIsLink", dispidDdeIsLink, DdeIsLink, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif


#ifdef SS_V30
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Appearance", dispidAppearance, GetAppearance, SetAppearance, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxMaxDrop", dispidTypeComboBoxMaxDrop, GetTypeComboBoxMaxDrop, SetTypeComboBoxMaxDrop, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeMaxEditLen", dispidTypeMaxEditLen, GetTypeMaxEditLen, SetTypeMaxEditLen, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxWidth", dispidTypeComboBoxWidth, GetTypeComboBoxWidth, SetTypeComboBoxWidth, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TextTip", dispidTextTip, GetTextTip, SetTextTip, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TextTipDelay", dispidTextTipDelay, GetTextTipDelay, SetTextTipDelay, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarTrack", dispidScrollBarTrack, GetScrollBarTrack, SetScrollBarTrack, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeVAlign", dispidTypeVAlign, GetTypeVAlign, SetTypeVAlign, VT_I2)
#ifdef SS_V70
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ClipboardOptions", dispidClipboardOptions, GetClipboardOptions, SetClipboardOptions, VT_I4)
#else
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ClipboardOptions", dispidClipboardOptions, GetClipboardOptions, SetClipboardOptions, VT_I2)
#endif
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintSmartPrint", dispidPrintSmartPrint, GetPrintSmartPrint, SetPrintSmartPrint, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintPageOrder", dispidPrintPageOrder, GetPrintPageOrder, SetPrintPageOrder, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintPageCount", dispidPrintPageCount, GetPrintPageCount, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintFirstPageNumber", dispidPrintFirstPageNumber, GetPrintFirstPageNumber, SetPrintFirstPageNumber, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintNextPageBreakCol", dispidPrintNextPageBreakCol, GetPrintNextPageBreakCol, SetPrintNextPageBreakCol, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintNextPageBreakRow", dispidPrintNextPageBreakRow, GetPrintNextPageBreakRow, SetPrintNextPageBreakRow, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxhWnd", dispidTypeComboBoxhWnd, GetTypeComboBoxhWnd, SetTypeComboBoxhWnd, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelBackColor", dispidSelBackColor, GetSelBackColor, SetSelBackColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelForeColor", dispidSelForeColor, GetSelForeColor, SetSelForeColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TwoDigitYearMax", dispidTwoDigitYearMax, GetTwoDigitYearMax, SetTwoDigitYearMax, VT_I2)

#ifdef SS_V70
    DISP_FUNCTION_ID(CSsocxCtrl, "GetActionKey", dispidGetActionKey, GetActionKey, VT_BOOL, VTS_I4 VTS_PBOOL VTS_PBOOL VTS_PI2)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetActionKey", dispidSetActionKey, SetActionKey, VT_BOOL, VTS_I4 VTS_BOOL VTS_BOOL VTS_I2)
#else
    DISP_FUNCTION_ID(CSsocxCtrl, "GetActionKey", dispidGetActionKey, GetActionKey, VT_BOOL, VTS_I2 VTS_PBOOL VTS_PBOOL VTS_PI2)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetActionKey", dispidSetActionKey, SetActionKey, VT_BOOL, VTS_I2 VTS_BOOL VTS_BOOL VTS_I2)
#endif
    DISP_FUNCTION_ID(CSsocxCtrl, "GetOddEvenRowColor", dispidGetOddEvenRowColor, GetOddEvenRowColor, VT_BOOL, VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetOddEvenRowColor", dispidSetOddEvenRowColor, SetOddEvenRowColor, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "GetFloat", dispidGetFloat, GetFloat, VT_BOOL, VTS_I4 VTS_I4 VTS_PR8)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetFloat", dispidSetFloat, SetFloat, VT_BOOL, VTS_I4 VTS_I4 VTS_R8)
    DISP_FUNCTION_ID(CSsocxCtrl, "GetInteger", dispidGetInteger, GetInteger, VT_BOOL, VTS_I4 VTS_I4 VTS_PI4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetInteger", dispidSetInteger, SetInteger, VT_BOOL, VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetCalText", dispidSetCalText, SetCalText, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "GetTextTipAppearance", dispidGetTextTipAppearance, GetTextTipAppearance, VT_BOOL, VTS_PBSTR VTS_PI2 VTS_PBOOL VTS_PBOOL VTS_PI4 VTS_PI4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetTextTipAppearance", dispidSetTextTipAppearance, SetTextTipAppearance, VT_BOOL, VTS_BSTR VTS_I2 VTS_BOOL VTS_BOOL VTS_I4 VTS_I4)
#ifdef SS_V80
	#ifndef SSF_NO_ExportToHTML
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportToHTMLEx", dispidExportToHTMLEx, ExportToHTMLEx, VT_BOOL, VTS_BSTR VTS_BOOL VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
	#endif
	#ifndef SSF_NO_ExportRangeToHTML
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportRangeToHTMLEx", dispidExportRangeToHTMLEx, ExportRangeToHTMLEx, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BOOL VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
	#endif
#endif
	#ifndef SSF_NO_ExportToHTML
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportToHTML", dispidExportToHTML, ExportToHTML, VT_BOOL, VTS_BSTR VTS_BOOL VTS_BSTR)
	#endif
	#ifndef SSF_NO_ExportRangeToHTML
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportRangeToHTML", dispidExportRangeToHTML, ExportRangeToHTML, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BOOL VTS_BSTR)
	#endif
#ifdef SS_V35
		DISP_FUNCTION_ID(CSsocxCtrl, "IsFetchCellNote", dispidIsFetchCellNote, IsFetchCellNote, VT_BOOL, VTS_NONE)
	#ifndef SSF_NO_ExportToXML
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportToXML", dispidExportToXML, ExportToXML, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR)
	#endif
	#ifndef SSF_NO_ExportRangeToXML
        DISP_FUNCTION_ID(CSsocxCtrl, "ExportRangeToXML", dispidExportRangeToXML, ExportRangeToXML, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR)
	#endif
	#ifndef SSF_NO_ExportToXMLBuffer
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportToXMLBuffer", dispidExportToXMLBuffer, ExportToXMLBuffer, VT_BOOL, VTS_BSTR VTS_BSTR VTS_PBSTR VTS_I4 VTS_BSTR)
	#endif
	#ifndef SSF_NO_ExportRangeToXMLBuffer
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportRangeToXMLBuffer", dispidExportRangeToXMLBuffer, ExportRangeToXMLBuffer, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BSTR VTS_PBSTR VTS_I4 VTS_BSTR)
	#endif
    DISP_FUNCTION_ID(CSsocxCtrl, "OwnerPrintDraw", dispidOwnerPrintDraw, OwnerPrintDraw, VT_BOOL, VTS_HANDLE VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "OwnerPrintPageCount", dispidOwnerPrintPageCount, OwnerPrintPageCount, VT_BOOL, VTS_HANDLE VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PI2)
	#ifndef SSF_NO_ExportRangeToTextFile
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportRangeToTextFile", dispidExportRangeToTextFile, ExportRangeToTextFile, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR)
	#endif
	#ifndef SSF_NO_ExportToTextFile
		DISP_FUNCTION_ID(CSsocxCtrl, "ExportToTextFile", dispidExportToTextFile, ExportToTextFile, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR)
	#endif
	DISP_FUNCTION_ID(CSsocxCtrl, "LoadTextFile", dispidLoadTextFile, LoadTextFile, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR)
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "ColUserSortIndicator", dispidColUserSortIndicator, GetColUserSortIndicator, SetColUserSortIndicator, VT_I2, VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetCalTextOverride", dispidSetCalTextOverride, SetCalTextOverride, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCalTextOverride", dispidGetCalTextOverride, GetCalTextOverride, VT_EMPTY, VTS_PBSTR VTS_PI2 VTS_PBSTR VTS_PI2 VTS_PBSTR VTS_PI2 VTS_PBSTR VTS_PI2 VTS_PBSTR VTS_PI2 VTS_PBSTR VTS_PI2)
#endif

    DISP_FUNCTION_ID(CSsocxCtrl, "GetArray", dispidGetArray, GetArray, VT_BOOL, VTS_I4 VTS_I4 VTS_VARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetArray", dispidSetArray, SetArray, VT_BOOL, VTS_I4 VTS_I4 VTS_VARIANT)
  //rap
  #ifndef SSF_NO_IsExcelFile
	DISP_FUNCTION_ID(CSsocxCtrl, "IsExcelFile", dispidIsExcelFile, IsExcelFile, VT_I2, VTS_BSTR)
  #endif
  #ifndef SSF_NO_GetExcelSheetList
  DISP_FUNCTION_ID(CSsocxCtrl, "GetExcelSheetList", dispidGetExcelSheetList, GetExcelSheetList, VT_BOOL, VTS_BSTR VTS_VARIANT VTS_PI2 VTS_BSTR VTS_PI2 VTS_BOOL)
  #endif
  #ifndef SSF_NO_ImportExcelSheet
	DISP_FUNCTION_ID(CSsocxCtrl, "ImportExcelSheet", dispidImportExcelSheet, ImportExcelSheet, VT_BOOL, VTS_I2 VTS_VARIANT)
  #endif
  #ifndef SSF_NO_ExportToExcel
	DISP_FUNCTION_ID(CSsocxCtrl, "ExportToExcel", dispidExportToExcel, ExportToExcel, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
  #endif
  #ifndef SSF_NO_EnumCustomFunction
  DISP_FUNCTION_ID(CSsocxCtrl, "EnumCustomFunction", dispidEnumCustomFunction, EnumCustomFunction, VT_BOOL, VTS_BSTR VTS_PBSTR)
  #endif
  #ifndef SSF_NO_GetCustomFunction
  DISP_FUNCTION_ID(CSsocxCtrl, "GetCustomFunction", dispidGetCustomFunction, GetCustomFunction, VT_BOOL, VTS_BSTR VTS_PI2 VTS_PI2 VTS_PI4)
  #endif
  #ifndef SSF_NO_RemoveCustomFunction
  DISP_FUNCTION_ID(CSsocxCtrl, "RemoveCustomFunction", dispidRemoveCustomFunction, RemoveCustomFunction, VT_BOOL, VTS_BSTR)
  #endif
  #ifndef SSF_NO_ReCalcCell
  DISP_FUNCTION_ID(CSsocxCtrl, "ReCalcCell", dispidReCalcCell, ReCalcCell, VT_BOOL, VTS_I4 VTS_I4)
  #endif
#ifdef FP_OLEDRAG
  DISP_PROPERTY_EX_ID(CSsocxCtrl, "OLEDropMode", dispidOLEDropMode, GetOLEDropMode, SetOLEDropMode, VT_I2)
  DISP_FUNCTION_ID(CSsocxCtrl, "OLEDrag", dispidOLEDrag, OLEDrag, VT_EMPTY, VTS_NONE)
#endif
  #if (!defined(SSP_NO_DATASOURCE) && defined(FP_ADB))
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "DataMember", dispidDataMember, GetDataMember, SetDataMember, VT_BSTR)       //DBOCX
  #endif  // DBOCX
#endif // SS_V30

#ifdef SS_V35
    DISP_FUNCTION_ID(CSsocxCtrl, "OwnerPrintDraw", dispidOwnerPrintDraw, OwnerPrintDraw, VT_BOOL, VTS_HANDLE VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "OwnerPrintPageCount", dispidOwnerPrintPageCount, OwnerPrintPageCount, VT_BOOL, VTS_HANDLE VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PI2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellNote", dispidCellNote, GetCellNote, SetCellNote, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "UserColAction", dispidUserColAction, GetUserColAction, SetUserColAction, VT_I2)
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "ColUserSortIndicator", dispidColUserSortIndicator, GetColUserSortIndicator, SetColUserSortIndicator, VT_I2, VTS_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ShowScrollTips", dispidShowScrollTips, GetShowScrollTips, SetShowScrollTips, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellNoteIndicator", dispidCellNoteIndicator, GetCellNoteIndicator, SetCellNoteIndicator, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeEllipses", dispidTypeEllipses, GetTypeEllipses, SetTypeEllipses, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScriptEnhanced", dispidScriptEnhanced, GetScriptEnhanced, SetScriptEnhanced, VT_BOOL)
#endif //SS_V35

#ifdef SS_V40
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColMerge", dispidColMerge, GetColMerge, SetColMerge, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowMerge", dispidRowMerge, GetRowMerge, SetRowMerge, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColID", dispidColID, GetColID, SetColID, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowHeaderCols", dispidRowHeaderCols, GetRowHeaderCols, SetRowHeaderCols, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowHeadersAutoTextIndex", dispidRowHeadersAutoTextIndex, GetRowHeadersAutoTextIndex, SetRowHeadersAutoTextIndex, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColHeaderRows", dispidColHeaderRows, GetColHeaderRows, SetColHeaderRows, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColHeadersAutoTextIndex", dispidColHeadersAutoTextIndex, GetColHeadersAutoTextIndex, SetColHeadersAutoTextIndex, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColHeadersUserSortIndex", dispidColHeadersUserSortIndex, GetColHeadersUserSortIndex, SetColHeadersUserSortIndex, VT_I4)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeComboBoxhAutoSearch", dispidTypeComboBoxAutoSearch, GetTypeComboBoxAutoSearch, SetTypeComboBoxAutoSearch, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeSpinInc", dispidTypeSpinInc, GetTypeSpinInc, SetTypeSpinInc, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeSpinWrap", dispidTypeSpinWrap, GetTypeSpinWrap, SetTypeSpinWrap, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNegRed", dispidTypeNegRed, GetTypeNegRed, SetTypeNegRed, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyMin", dispidTypeCurrencyMin, GetTypeCurrencyMin, SetTypeCurrencyMin, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyMax", dispidTypeCurrencyMax, GetTypeCurrencyMax, SetTypeCurrencyMax, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyDecimal", dispidTypeCurrencyDecimal, GetTypeCurrencyDecimal, SetTypeCurrencyDecimal, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencySeparator", dispidTypeCurrencySeparator, GetTypeCurrencySeparator, SetTypeCurrencySeparator, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencySymbol", dispidTypeCurrencySymbol, GetTypeCurrencySymbol, SetTypeCurrencySymbol, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyLeadingZero", dispidTypeCurrencyLeadingZero, GetTypeCurrencyLeadingZero, SetTypeCurrencyLeadingZero, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyDecPlaces", dispidTypeCurrencyDecPlaces, GetTypeCurrencyDecPlaces, SetTypeCurrencyDecPlaces, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyNegStyle", dispidTypeCurrencyNegStyle, GetTypeCurrencyNegStyle, SetTypeCurrencyNegStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyPosStyle", dispidTypeCurrencyPosStyle, GetTypeCurrencyPosStyle, SetTypeCurrencyPosStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyShowSep", dispidTypeCurrencyShowSep, GetTypeCurrencyShowSep, SetTypeCurrencyShowSep, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCurrencyShowSymbol", dispidTypeCurrencyShowSymbol, GetTypeCurrencyShowSymbol, SetTypeCurrencyShowSymbol, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberMin", dispidTypeNumberMin, GetTypeNumberMin, SetTypeNumberMin, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberMax", dispidTypeNumberMax, GetTypeNumberMax, SetTypeNumberMax, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberDecimal", dispidTypeNumberDecimal, GetTypeNumberDecimal, SetTypeNumberDecimal, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberSeparator", dispidTypeNumberSeparator, GetTypeNumberSeparator, SetTypeNumberSeparator, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberLeadingZero", dispidTypeNumberLeadingZero, GetTypeNumberLeadingZero, SetTypeNumberLeadingZero, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberDecPlaces", dispidTypeNumberDecPlaces, GetTypeNumberDecPlaces, SetTypeNumberDecPlaces, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberNegStyle", dispidTypeNumberNegStyle, GetTypeNumberNegStyle, SetTypeNumberNegStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeNumberShowSep", dispidTypeNumberShowSep, GetTypeNumberShowSep, SetTypeNumberShowSep, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePercentMin", dispidTypePercentMin, GetTypePercentMin, SetTypePercentMin, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePercentMax", dispidTypePercentMax, GetTypePercentMax, SetTypePercentMax, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePercentDecimal", dispidTypePercentDecimal, GetTypePercentDecimal, SetTypePercentDecimal, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePercentDecPlaces", dispidTypePercentDecPlaces, GetTypePercentDecPlaces, SetTypePercentDecPlaces, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePercentNegStyle", dispidTypePercentNegStyle, GetTypePercentNegStyle, SetTypePercentNegStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeTextOrient", dispidTypeTextOrient, GetTypeTextOrient, SetTypeTextOrient, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellTag", dispidCellTag, GetCellTag, SetCellTag, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AllowEditOverflow", dispidAllowEditOverflow, GetAllowEditOverflow, SetAllowEditOverflow, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColHeadersAutoText", dispidColHeadersAutoText, GetColHeaderDisplay, SetColHeaderDisplay, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ColHeadersShow", dispidColHeadersShow, GetDisplayColHeaders, SetDisplayColHeaders, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowHeadersAutoText", dispidRowHeadersAutoText, GetRowHeaderDisplay, SetRowHeaderDisplay, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "RowHeadersShow", dispidRowHeadersShow, GetDisplayRowHeaders, SetDisplayRowHeaders, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypePercentLeadingZero", dispidTypePercentLeadingZero, GetTypePercentLeadingZero, SetTypePercentLeadingZero, VT_I2)
#endif // SS_V40
#ifdef SS_V70
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SheetCount", dispidSheetCount, GetSheetCount, SetSheetCount, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ActiveSheet", dispidActiveSheet, GetActiveSheet, SetActiveSheet, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TabStripLeftSheet", dispidTabStripLeftSheet, GetTabStripLeftSheet, SetTabStripLeftSheet, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TabStripFont", dispidTabStripFont, GetTabStripFont, SetTabStripFont, VT_FONT)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SheetName", dispidSheetName, GetSheetName, SetSheetName, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TabStripPolicy", dispidTabStripPolicy, GetTabStripPolicy, SetTabStripPolicy, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TabStripRatio", dispidTabStripRatio, GetTabStripRatio, SetTabStripRatio, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintZoomFactor", dispidPrintZoomFactor, GetPrintZoomFactor, SetPrintZoomFactor, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarWidth", dispidScrollBarWidth, GetScrollBarWidth, SetScrollBarWidth, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarHeight", dispidScrollBarHeight, GetScrollBarHeight, SetScrollBarHeight, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ActiveCellHighlightStyle", dispidActiveCellHighlightStyle, GetActiveCellHighlightStyle, SetActiveCellHighlightStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintScalingMethod", dispidPrintScalingMethod, GetPrintScalingMethod, SetPrintScalingMethod, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintBestFitPagesWide", dispidPrintBestFitPagesWide, GetPrintBestFitPagesWide, SetPrintBestFitPagesWide, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintBestFitPagesTall", dispidPrintBestFitPagesTall, GetPrintBestFitPagesTall, SetPrintBestFitPagesTall, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintCenterOnPageH", dispidPrintCenterOnPageH, GetPrintCenterOnPageH, SetPrintCenterOnPageH, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "PrintCenterOnPageV", dispidPrintCenterOnPageV, GetPrintCenterOnPageV, SetPrintCenterOnPageV, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AllowColMove", dispidAllowColMove, GetAllowColMove, SetAllowColMove, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AllowRowMove", dispidAllowRowMove, GetAllowRowMove, SetAllowRowMove, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeScientificMin", dispidTypeScientificMin, GetTypeScientificMin, SetTypeScientificMin, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeScientificMax", dispidTypeScientificMax, GetTypeScientificMax, SetTypeScientificMax, VT_R8)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeScientificDecimal", dispidTypeScientificDecimal, GetTypeScientificDecimal, SetTypeScientificDecimal, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeScientificDecPlaces", dispidTypeScientificDecPlaces, GetTypeScientificDecPlaces, SetTypeScientificDecPlaces, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarHColor", dispidScrollBarHColor, GetScrollBarHColor, SetScrollBarHColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarVColor", dispidScrollBarVColor, GetScrollBarVColor, SetScrollBarVColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Sheet", dispidSheet, GetSheet, SetSheet, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SheetSendingEvent", dispidSheetSendingEvent, GetSheetSendingEvent, SetNotSupported, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SheetVisible", dispidSheetVisible, GetSheetVisible, SetSheetVisible, VT_BOOL)
#endif // SS_V70
#ifdef SS_V80
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "AppearanceStyle", dispidAppearanceStyle, GetAppearanceStyle, SetAppearanceStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TabEnhancedShape", dispidTabEnhancedShape, GetTabEnhancedShape, SetTabEnhancedShape, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellNoteIndicatorColor", dispidCellNoteIndicatorColor, GetCellNoteIndicatorColor, SetCellNoteIndicatorColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "CellNoteIndicatorShape", dispidCellNoteIndicatorShape, GetCellNoteIndicatorShape, SetCellNoteIndicatorShape, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "TabStripButtonPolicy", dispidTabStripButtonPolicy, GetTabStripButtonPolicy, SetTabStripButtonPolicy, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "HighlightHeaders", dispidHighlightHeaders, GetHighlightHeaders, SetHighlightHeaders, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "HighlightStyle", dispidHighlightStyle, GetHighlightStyle, SetHighlightStyle, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "HighlightAlphaBlendColor", dispidHighlightAlphaBlendColor, GetHighlightAlphaBlendColor, SetHighlightAlphaBlendColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "HighlightAlphaBlend", dispidHighlightAlphaBlend, GetHighlightAlphaBlend, SetHighlightAlphaBlend, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "UseVisualStyles", dispidUseVisualStyles, GetUseVisualStyles, SetUseVisualStyles, VT_I2)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "EnhanceStaticCells", dispidEnhanceStaticCells, GetEnhanceStaticCells, SetEnhanceStaticCells, VT_BOOL)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ScrollBarStyle", dispidScrollBarStyle, GetScrollBarStyle, SetScrollBarStyle, VT_I2)
#endif
#ifdef SS_V35
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "SelectionCount", dispidSelectionCount, GetSelectionCount, SetSelectionCount, VT_I4)
#endif //SS_V35

/**/
#ifdef SS_V35
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptCFGetCellParam", dispidScriptCFGetCellParam, ScriptCFGetCellParam, VT_EMPTY, VTS_I2 VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptCFGetDoubleParamExt", dispidScriptCFGetDoubleParamExt, ScriptCFGetDoubleParamExt, VT_R8, VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptCFGetParamInfo", dispidScriptCFGetParamInfo, ScriptCFGetParamInfo, VT_BOOL, VTS_I2 VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptCFGetRangeParam", dispidScriptCFGetRangeParam, ScriptCFGetRangeParam, VT_EMPTY, VTS_I2 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptColWidthToTwips", dispidScriptColWidthToTwips, ScriptColWidthToTwips, VT_EMPTY, VTS_R4 VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetBottomRightCell", dispidScriptGetBottomRightCell, ScriptGetBottomRightCell, VT_EMPTY, VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetCellFromScreenCoord", dispidScriptGetCellFromScreenCoord, ScriptGetCellFromScreenCoord, VT_EMPTY, VTS_PVARIANT VTS_PVARIANT VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetCellPos", dispidScriptGetCellPos, ScriptGetCellPos, VT_BOOL, VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetClientArea", dispidScriptGetClientArea, ScriptGetClientArea, VT_EMPTY, VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetFirstValidCell", dispidScriptGetFirstValidCell, ScriptGetFirstValidCell, VT_EMPTY, VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetIteration", dispidScriptGetIteration, ScriptGetIteration, VT_BOOL, VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetLastValidCell", dispidScriptGetLastValidCell, ScriptGetLastValidCell, VT_EMPTY, VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptRowHeightToTwips", dispidScriptRowHeightToTwips, ScriptRowHeightToTwips, VT_EMPTY, VTS_I4 VTS_R4 VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptTwipsToColWidth", dispidScriptTwipsToColWidth, ScriptTwipsToColWidth, VT_EMPTY, VTS_I4 VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptTwipsToRowHeight", dispidScriptTwipsToRowHeight, ScriptTwipsToRowHeight, VT_EMPTY, VTS_I4 VTS_I4 VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetActionKey", dispidScriptGetActionKey, ScriptGetActionKey, VT_BOOL, VTS_I2 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetOddEvenRowColor", dispidScriptGetOddEvenRowColor, ScriptGetOddEvenRowColor, VT_BOOL, VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetFloat", dispidScriptGetFloat, ScriptGetFloat, VT_BOOL, VTS_I4 VTS_I4 VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetInteger", dispidScriptGetInteger, ScriptGetInteger, VT_BOOL, VTS_I4 VTS_I4 VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetTextTipAppearance", dispidScriptGetTextTipAppearance, ScriptGetTextTipAppearance, VT_BOOL, VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetExcelSheetList", dispidScriptGetExcelSheetList, ScriptGetExcelSheetList, VT_BOOL, VTS_BSTR VTS_VARIANT VTS_PVARIANT VTS_BSTR VTS_PVARIANT VTS_BOOL)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptEnumCustomFunction", dispidScriptEnumCustomFunction, ScriptEnumCustomFunction, VT_BOOL, VTS_BSTR VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetCustomFunction", dispidScriptGetCustomFunction, ScriptGetCustomFunction, VT_BOOL, VTS_BSTR VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptExportToXMLBuffer", dispidScriptExportToXMLBuffer, ScriptExportToXMLBuffer, VT_BOOL, VTS_BSTR VTS_BSTR VTS_PVARIANT VTS_I4 VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptExportRangeToXMLBuffer", dispidScriptExportRangeToXMLBuffer, ScriptExportRangeToXMLBuffer, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BSTR VTS_PVARIANT VTS_I4 VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptOwnerPrintPageCount", dispidScriptOwnerPrintPageCount, ScriptOwnerPrintPageCount, VT_BOOL, VTS_HANDLE VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptGetCalTextOverride", dispidScriptGetCalTextOverride, ScriptGetCalTextOverride, VT_EMPTY, VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "PrintSheet", dispidPrintSheet, PrintSheet, VT_EMPTY, VTS_VARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "AddSelection", dispidAddSelection, AddSelection, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "ClearRange", dispidClearRange, ClearRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BOOL)
    DISP_FUNCTION_ID(CSsocxCtrl, "ClearSelection", dispidClearSelection, ClearSelection, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CSsocxCtrl, "ClipboardCopy", dispidClipboardCopy, ClipboardCopy, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CSsocxCtrl, "ClipboardCut", dispidClipboardCut, ClipboardCut, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CSsocxCtrl, "ClipboardPaste", dispidClipboardPaste, ClipboardPaste, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CSsocxCtrl, "CopyRange", dispidCopyRange, CopyRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "DeleteCols", dispidDeleteCols, DeleteCols, VT_EMPTY, VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "DeleteRows", dispidDeleteRows, DeleteRows, VT_EMPTY, VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "DataSave", dispidDataSave, DataSave, VT_EMPTY, VTS_NONE)
#ifdef SS_V70
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCellBorder", dispidGetCellBorder, GetCellBorder, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT)
#else
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCellBorder", dispidGetCellBorder, GetCellBorder, VT_EMPTY, VTS_I4 VTS_I4 VTS_I2 VTS_PVARIANT VTS_PVARIANT)
#endif
    DISP_FUNCTION_ID(CSsocxCtrl, "GetSelection", dispidGetSelection, GetSelection, VT_EMPTY, VTS_I4 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "InsertCols", dispidInsertCols, InsertCols, VT_EMPTY, VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "InsertRows", dispidInsertRows, InsertRows, VT_EMPTY, VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "MoveRange", dispidMoveRange, MoveRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "ReCalc", dispidReCalc, ReCalc, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CSsocxCtrl, "Reset", dispidReset, Reset, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetActiveCell", dispidSetActiveCell, SetActiveCell, VT_EMPTY, VTS_I4 VTS_I4)
#ifdef SS_V70
    DISP_FUNCTION_ID(CSsocxCtrl, "SetCellBorder", dispidSetCellBorder, SetCellBorder, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
#else
    DISP_FUNCTION_ID(CSsocxCtrl, "SetCellBorder", dispidSetCellBorder, SetCellBorder, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I2 VTS_I4 VTS_I4)
#endif
    DISP_FUNCTION_ID(CSsocxCtrl, "SetSelection", dispidSetSelection, SetSelection, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "ShowCell", dispidShowCell, ShowCell, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4)
#ifdef SS_V40
    DISP_FUNCTION_ID(CSsocxCtrl, "Sort", dispidSort, Sort, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT)
#else
    DISP_FUNCTION_ID(CSsocxCtrl, "Sort", dispidSort, Sort, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT)
#endif // SS_V40
    DISP_FUNCTION_ID(CSsocxCtrl, "SwapRange", dispidSwapRange, SwapRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "TypeComboBoxClear", dispidTypeComboBoxClear, TypeComboBoxClear, VT_EMPTY, VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "TypeComboBoxRemoveItem", dispidTypeComboBoxRemoveItem, TypeComboBoxRemoveItem, VT_EMPTY, VTS_I4 VTS_I4 VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "VirtualRefresh", dispidVirtualRefresh, VirtualRefresh, VT_EMPTY, VTS_NONE)
#endif // SS_V35
/**/
#ifdef SS_V40
    DISP_FUNCTION_ID(CSsocxCtrl, "AddCellSpan", dispidAddCellSpan, AddCellSpan, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "GetCellSpan", dispidGetCellSpan, GetCellSpan, VT_I2, VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "RemoveCellSpan", dispidRemoveCellSpan, RemoveCellSpan, VT_EMPTY, VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "GetColFromID", dispidGetColFromID, GetColFromID, VT_I4, VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "CopyColRange", dispidCopyColRange, CopyColRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "CopyRowRange", dispidCopyRowRange, CopyRowRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "MoveColRange", dispidMoveColRange, MoveColRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "MoveRowRange", dispidMoveRowRange, MoveRowRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SwapColRange", dispidSwapColRange, SwapColRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SwapRowRange", dispidSwapRowRange, SwapRowRange, VT_EMPTY, VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SearchCol", dispidSearchCol, SearchCol, VT_I4, VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "SearchRow", dispidSearchRow, SearchRow, VT_I4, VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_I2)
    DISP_PROPERTY_PARAM_ID(CSsocxCtrl, "EventEnabled", dispidEventEnabled, GetEventEnabled, SetEventEnabled, VT_BOOL, VTS_I2)
//GAB
    #ifndef SSF_NO_ExportRangeToTextFileU
	    DISP_FUNCTION_ID(CSsocxCtrl, "ExportRangeToTextFileU", dispidExportRangeToTextFileU, ExportRangeToTextFileU, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR)
    #endif
    #ifndef SSF_NO_ExportToTextFileU
	    DISP_FUNCTION_ID(CSsocxCtrl, "ExportToTextFileU", dispidExportToTextFileU, ExportToTextFileU, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR)
    #endif
    #ifndef SSF_NO_SaveTabFileU
      DISP_FUNCTION_ID(CSsocxCtrl, "SaveTabFileU", dispidSaveTabFileU, SaveTabFileU, VT_BOOL, VTS_BSTR)
    #endif
#endif // SS_V40
#ifdef SS_V70
    DISP_FUNCTION_ID(CSsocxCtrl, "ColLetterToNumber", dispidColLetterToNumber, ColLetterToNumber, VT_I4, VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "DataRefresh", dispidDataRefresh, DataRefresh, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION_ID(CSsocxCtrl, "ImportExcelBook", dispidImportExcelBook, ImportExcelBook, VT_BOOL, VTS_BSTR VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "ExportExcelBook", dispidExportExcelBook, ExportExcelBook, VT_BOOL, VTS_BSTR VTS_BSTR)
    DISP_FUNCTION_ID(CSsocxCtrl, "InsertSheets", dispidInsertSheets, InsertSheets, VT_BOOL, VTS_I2 VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "DeleteSheets", dispidDeleteSheets, DeleteSheets, VT_BOOL, VTS_I2 VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "MoveSheets", dispidMoveSheets, MoveSheets, VT_BOOL, VTS_I2 VTS_I2 VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "ResetSheet", dispidResetSheet, ResetSheet, VT_BOOL, VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "LoadPicture", dispidLoadPicture, LoadPicture, VT_DISPATCH, VTS_BSTR VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "LoadResPicture", dispidLoadResPicture, LoadResPicture, VT_DISPATCH, VTS_I4 VTS_BSTR VTS_BSTR VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "LoadPictureBuffer", dispidLoadPictureBuffer, LoadPictureBuffer, VT_DISPATCH, VTS_PVARIANT VTS_PVARIANT VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "SavePicture", dispidSavePicture, SavePicture, VT_BOOL, VTS_DISPATCH VTS_BSTR VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "SavePictureBuffer", dispidSavePictureBuffer, SavePictureBuffer, VT_BOOL, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetCellParamExt", dispidCFGetCellParamExt, CFGetCellParamExt, VT_EMPTY, VTS_I2 VTS_PI2 VTS_PI4 VTS_PI4)
    DISP_FUNCTION_ID(CSsocxCtrl, "CFGetRangeParamExt", dispidCFGetRangeParamExt, CFGetRangeParamExt, VT_EMPTY, VTS_I2 VTS_PI2 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptCFGetCellParamExt", dispidScriptCFGetCellParamExt, ScriptCFGetCellParamExt, VT_EMPTY, VTS_I2 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
    DISP_FUNCTION_ID(CSsocxCtrl, "ScriptCFGetRangeParamExt", dispidScriptCFGetRangeParamExt, ScriptCFGetRangeParamExt, VT_EMPTY, VTS_I2 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
  	DISP_FUNCTION_ID(CSsocxCtrl, "ExportToExcelEx", dispidExportToExcelEx, ExportToExcelEx, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I2)
    DISP_FUNCTION_ID(CSsocxCtrl, "ExportExcelBookEx", dispidExportExcelBookEx, ExportExcelBookEx, VT_BOOL, VTS_BSTR VTS_BSTR VTS_I2)
#endif // SS_V70
#ifdef SS_V80
    DISP_FUNCTION_ID(CSsocxCtrl, "SetEnhancedSheetTabColors", dispidSetEnhancedSheetTabColors, SetEnhancedSheetTabColors, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetEnhancedScrollBarColors", dispidSetEnhancedScrollBarColors, SetEnhancedScrollBarColors, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 )
    DISP_FUNCTION_ID(CSsocxCtrl, "SetEnhancedColumnHeaderColors", dispidSetEnhancedColumnHeaderColors, SetEnhancedColumnHeaderColors, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CSsocxCtrl, "SetEnhancedRowHeaderColors", dispidSetEnhancedRowHeaderColors, SetEnhancedRowHeaderColors, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 )
    DISP_FUNCTION_ID(CSsocxCtrl, "SetEnhancedCornerColors", dispidSetEnhancedCornerColors, SetEnhancedCornerColors, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	DISP_FUNCTION_ID(CSsocxCtrl, "IsVerticalScrollBarVisible", dispidIsVerticalScrollBarVisible, IsVerticalScrollBarVisible, VT_BOOL, VTS_NONE)
	DISP_FUNCTION_ID(CSsocxCtrl, "IsHorizontalScrollBarVisible", dispidIsHorizontalScrollBarVisible, IsHorizontalScrollBarVisible, VT_BOOL, VTS_NONE)
	DISP_FUNCTION_ID(CSsocxCtrl, "SaveToBuffer", dispidSaveToBuffer, SaveToBuffer, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION_ID(CSsocxCtrl, "LoadFromBuffer", dispidLoadFromBuffer, LoadFromBuffer, VT_BOOL, VTS_VARIANT)
	DISP_FUNCTION_ID(CSsocxCtrl, "SaveBlockToBuffer", dispidSaveBlockToBuffer, SaveBlockToBuffer, VT_VARIANT, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BOOL VTS_BOOL)
	DISP_FUNCTION_ID(CSsocxCtrl, "LoadBlockFromBuffer", dispidLoadBlockFromBuffer, LoadBlockFromBuffer, VT_BOOL, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT)
   DISP_FUNCTION_ID(CSsocxCtrl, "AddCustomCellType", dispidAddCustomCellType, AddCustomCellType, VT_BOOL, VTS_BSTR VTS_BOOL VTS_BOOL VTS_BOOL VTS_BOOL)
   DISP_FUNCTION_ID(CSsocxCtrl, "RemoveCustomCellType", dispidRemoveCustomCellType, RemoveCustomCellType, VT_BOOL, VTS_BSTR)
   DISP_FUNCTION_ID(CSsocxCtrl, "EnumCustomCellType", dispidEnumCustomCellType, EnumCustomCellType, VT_BOOL, VTS_BSTR VTS_PBSTR)
   DISP_FUNCTION_ID(CSsocxCtrl, "GetCustomCellType", dispidGetCustomCellType, GetCustomCellType, VT_BOOL, VTS_BSTR VTS_PBOOL VTS_PBOOL VTS_PBOOL VTS_PBOOL)
   DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCustomName", dispidTypeCustomName, GetTypeCustomName, SetTypeCustomName, VT_BSTR)
   DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCustomStyle", dispidTypeCustomStyle, GetTypeCustomStyle, SetTypeCustomStyle, VT_I4)
   DISP_FUNCTION_ID(CSsocxCtrl, "GetCustomEditor", dispidGetCustomEditor, GetCustomEditor, VT_VARIANT, VTS_BSTR VTS_I4)
   DISP_FUNCTION_ID(CSsocxCtrl, "GetCustomRenderer", dispidGetCustomRenderer, GetCustomRenderer, VT_VARIANT, VTS_BSTR VTS_I4)
   DISP_FUNCTION_ID(CSsocxCtrl, "PaintControl", dispidPaintControl, PaintControl, VT_BOOL, VTS_VARIANT VTS_HANDLE VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_BOOL)
   DISP_FUNCTION_ID(CSsocxCtrl, "GetFontHandle", dispidGetFontHandle, GetFontHandle, VT_HANDLE, VTS_FONT)
   DISP_PROPERTY_EX_ID(CSsocxCtrl, "TypeCustomItemData", dispidTypeCustomItemData, GetTypeCustomItemData, SetTypeCustomItemData, VT_VARIANT)
#ifdef XL12
   DISP_FUNCTION_ID(CSsocxCtrl, "OpenExcel2007File", dispidOpenExcel2007File, OpenExcel2007File, VT_BOOL, VTS_BSTR VTS_BSTR VTS_I2 VTS_I2 VTS_BSTR)
   DISP_FUNCTION_ID(CSsocxCtrl, "SaveExcel2007File", dispidSaveExcel2007File, SaveExcel2007File, VT_BOOL, VTS_BSTR VTS_BSTR VTS_I2 VTS_BSTR)
   DISP_FUNCTION_ID(CSsocxCtrl, "IsExcel2007File", dispidIsExcel2007File, IsExcel2007File, VT_I2, VTS_BSTR)
#endif // XL12
   DISP_FUNCTION_ID(CSsocxCtrl, "RenderToMetafile", 564, RenderToMetafile, VT_DISPATCH, VTS_NONE)
#endif // SS_V80

    DISP_PROPERTY_EX_ID(CSsocxCtrl, "BackColor", DISPID_BACKCOLOR, GetBackColor, SetBackColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "ForeColor", DISPID_FORECOLOR, GetForeColor, SetForeColor, VT_COLOR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "hWnd", DISPID_HWND, GetHWnd, SetNotSupported, VT_HANDLE)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Text", DISPID_TEXT, GetText, SetText, VT_BSTR)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "Font", DISPID_FONT, GetFont, SetFont, VT_FONT)
    DISP_PROPERTY_EX_ID(CSsocxCtrl, "BorderStyle", DISPID_BORDERSTYLE, GetBorderStyle, SetBorderStyle, VT_I2)
    DISP_STOCKPROP_ENABLED()
    DISP_STOCKFUNC_REFRESH()
    DISP_FUNCTION_ID(CSsocxCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CSsocxCtrl, COleControl)
    //{{AFX_EVENT_MAP(CSsocxCtrl)
    //}}AFX_EVENT_MAP
    #ifndef SSE_NO_ADVANCE
    EVENT_CUSTOM("Advance", FireAdvance, VTS_BOOL)
    #endif
    #ifndef SSE_NO_BLOCKSELECTED
    EVENT_CUSTOM("BlockSelected", FireBlockSelected, VTS_I4  VTS_I4  VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_BUTTONCLICKED
    EVENT_CUSTOM("ButtonClicked", FireButtonClicked, VTS_I4  VTS_I4  VTS_I2)
    #endif
    #ifndef SSE_NO_CHANGE
    EVENT_CUSTOM("Change", FireChange, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_CLICK
    EVENT_CUSTOM("Click", FireClick, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_COLWIDTHCHANGE
    EVENT_CUSTOM("ColWidthChange", FireColWidthChange, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_CUSTOMFUNCTION
#ifdef SS_V70
    EVENT_CUSTOM("CustomFunction", FireCustomFunction, VTS_BSTR  VTS_I2  VTS_I4  VTS_I4  VTS_PI4)
#else
    EVENT_CUSTOM("CustomFunction", FireCustomFunction, VTS_BSTR  VTS_I2  VTS_I4  VTS_I4  VTS_PI2)
#endif
    #endif
#if 0
    #ifndef SSE_NO_DATAADDNEW
    EVENT_CUSTOM("DataAddNew", FireDataAddNew, VTS_NONE)
    #endif
#endif
    #ifndef SSE_NO_DATACOLCONFIG
    EVENT_CUSTOM("DataColConfig", FireDataColConfig, VTS_I4  VTS_BSTR  VTS_I2)
    #endif
    #ifndef SSE_NO_DATAFILL
    EVENT_CUSTOM("DataFill", FireDataFill, VTS_I4  VTS_I4  VTS_I2  VTS_I2  VTS_PI2)
    #endif
    #ifndef SSE_NO_DBLCLICK
    EVENT_CUSTOM("DblClick", FireDblClick, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_DRAGDROPBLOCK
    EVENT_CUSTOM("DragDropBlock", FireDragDropBlock, VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_BOOL  VTS_PI2  VTS_PBOOL  VTS_PBOOL)
    #endif
    #ifndef SSE_NO_DRAWITEM
    EVENT_CUSTOM("DrawItem", FireDrawItem, VTS_I4  VTS_I4  VTS_HANDLE  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_EDITERROR
    EVENT_CUSTOM("EditError", FireEditError, VTS_I4  VTS_I4  VTS_I2)
    #endif
    #ifndef SSE_NO_EDITMODE
    EVENT_CUSTOM("EditMode", FireEditMode, VTS_I4  VTS_I4  VTS_I2  VTS_BOOL)
    #endif
    #ifndef SSE_NO_ENTERROW
    EVENT_CUSTOM("EnterRow", FireEnterRow, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_LEAVECELL
    EVENT_CUSTOM("LeaveCell", FireLeaveCell, VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_PBOOL)
    #endif
    #ifndef SSE_NO_LEAVEROW
    EVENT_CUSTOM("LeaveRow", FireLeaveRow, VTS_I4  VTS_BOOL  VTS_BOOL  VTS_BOOL  VTS_I4  VTS_I4  VTS_PBOOL)
    #endif
    #ifndef SSE_NO_PRINTABORT
    EVENT_CUSTOM("PrintAbort", FirePrintAbort, VTS_PBOOL)
    #endif
    #ifndef SSE_NO_QUERYADVANCE
    EVENT_CUSTOM("QueryAdvance", FireQueryAdvance, VTS_BOOL  VTS_PBOOL)
    #endif
    #ifndef SSE_NO_QUERYDATA
    EVENT_CUSTOM("QueryData", FireQueryData, VTS_I4  VTS_I4  VTS_PI4  VTS_I2  VTS_PBOOL  VTS_PBOOL)
    #endif
    #ifndef SSE_NO_RIGHTCLICK
    EVENT_CUSTOM("RightClick", FireRightClick, VTS_I2  VTS_I4  VTS_I4  VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_ROWHEIGHTCHANGE
    EVENT_CUSTOM("RowHeightChange", FireRowHeightChange, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_SELCHANGE
    EVENT_CUSTOM("SelChange", FireSelChange, VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_TOPLEFTCHANGE
    EVENT_CUSTOM("TopLeftChange", FireTopLeftChange, VTS_I4  VTS_I4  VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_USERFORMULAENTERED
    EVENT_CUSTOM("UserFormulaEntered", FireUserFormulaEntered, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_VIRTUALCLEARDATA
    EVENT_CUSTOM("VirtualClearData", FireVirtualClearData, VTS_I4  VTS_I4)
    #endif
    #ifndef SSE_NO_PRINTMSGBOX
    EVENT_CUSTOM("PrintMsgBox", FirePrintMsgBox, VTS_BSTR VTS_I4 VTS_BOOL VTS_PBOOL)
    #endif
    #ifdef SS_V30
    EVENT_CUSTOM("ComboCloseUp", FireComboCloseUp, VTS_I4 VTS_I4 VTS_I2)
    EVENT_CUSTOM("ComboDropDown", FireComboDropDown, VTS_I4 VTS_I4)
    EVENT_CUSTOM("ComboSelChange", FireComboSelChange, VTS_I4 VTS_I4)
#ifdef SS_V70
    EVENT_CUSTOM("TextTipFetch", FireTextTipFetch, VTS_I4 VTS_I4 VTS_PI4 VTS_PI4 VTS_PBSTR VTS_PBOOL)
#else
    EVENT_CUSTOM("TextTipFetch", FireTextTipFetch, VTS_I4 VTS_I4 VTS_PI2 VTS_PI4 VTS_PBSTR VTS_PBOOL)
#endif
    EVENT_CUSTOM("TextEditChange", FireEditChange, VTS_I4 VTS_I4)
#ifdef FP_OLEDRAG
#include "..\..\..\fplibs\fptools\src\dragemap.cpp"
#endif // FP_OLEDRAG
    #endif
#if 0
    #ifndef SSE_NO_LINKDISPATCHNOTCREATED
    EVENT_CUSTOM("LinkDispatchNotCreated", FireLinkDispatchNotCreated, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKERROR
    EVENT_CUSTOM("LinkError", FireLinkError, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I2 VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKITEMNOTSUPPORTED
    EVENT_CUSTOM("LinkItemNotSupported", FireLinkItemNotSupported, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKITEMSUPPORTED
    EVENT_CUSTOM("LinkItemSupported", FireLinkItemSupported, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKNEWDATA
    EVENT_CUSTOM("LinkNewData", FireLinkNewData, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKNOTIFY
    EVENT_CUSTOM("LinkNotify", FireLinkNotify, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKOUTOFMEMORY
    EVENT_CUSTOM("LinkOutOfMemory", FireLinkOutOfMemory, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKPOKECOMPLETED
    EVENT_CUSTOM("LinkPokeCompleted", FireLinkPokeCompleted, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4)
    #endif
    #ifndef SSE_NO_LINKREQUESTCOMPLETED
    EVENT_CUSTOM("LinkRequestCompleted", FireLinkRequestCompleted, VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4)
    #endif
    #ifndef SSE_NO_LINKSERVERDISCONNECTED
    EVENT_CUSTOM("LinkServerDisconnected", FireLinkServerDisconnected, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
    #ifndef SSE_NO_LINKUNABLETOCONNECTTOSERVER
    EVENT_CUSTOM("LinkUnableToConnectToServer", FireLinkUnableToConnectToServer, VTS_BSTR VTS_BSTR VTS_BSTR)
    #endif
#endif
#ifdef SS_V35
    EVENT_CUSTOM("BeforeUserSort", FireBeforeUserSort, VTS_I4 VTS_I4 VTS_PI4)
    EVENT_CUSTOM("AfterUserSort", FireAfterUserSort, VTS_I4)
#endif  // SS_V35

    EVENT_STOCK_KEYDOWN()
    EVENT_STOCK_KEYPRESS()
    EVENT_STOCK_KEYUP()
    EVENT_STOCK_MOUSEDOWN()
    EVENT_STOCK_MOUSEMOVE()
    EVENT_STOCK_MOUSEUP()

#ifdef SS_V35
    EVENT_CUSTOM("ScriptCustomFunction", FireScriptCustomFunction, VTS_BSTR  VTS_I2  VTS_I4  VTS_I4  VTS_PVARIANT)
    EVENT_CUSTOM("ScriptDataFill", FireScriptDataFill, VTS_I4  VTS_I4  VTS_I2  VTS_I2  VTS_PVARIANT)
    EVENT_CUSTOM("ScriptDragDropBlock", FireScriptDragDropBlock, VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_BOOL  VTS_PVARIANT  VTS_PVARIANT  VTS_PVARIANT)
    EVENT_CUSTOM("ScriptLeaveCell", FireScriptLeaveCell, VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_PVARIANT)
    EVENT_CUSTOM("ScriptLeaveRow", FireScriptLeaveRow, VTS_I4  VTS_BOOL  VTS_BOOL  VTS_BOOL  VTS_I4  VTS_I4  VTS_PVARIANT)
    EVENT_CUSTOM("ScriptPrintAbort", FireScriptPrintAbort, VTS_PVARIANT)
    EVENT_CUSTOM("ScriptQueryAdvance", FireScriptQueryAdvance, VTS_BOOL  VTS_PVARIANT)
    EVENT_CUSTOM("ScriptQueryData", FireScriptQueryData, VTS_I4  VTS_I4  VTS_PVARIANT  VTS_I2  VTS_PVARIANT  VTS_PVARIANT)
    EVENT_CUSTOM("ScriptPrintMsgBox", FireScriptPrintMsgBox, VTS_BSTR VTS_I4 VTS_BOOL VTS_PVARIANT)
    EVENT_CUSTOM("ScriptTextTipFetch", FireScriptTextTipFetch, VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT)
//#ifdef FP_OLEDRAG
//#include "..\..\..\fplibs\fptools\src\dragemap.cpp"
//#endif // FP_OLEDRAG
    EVENT_CUSTOM("ScriptBeforeUserSort", FireScriptBeforeUserSort, VTS_I4 VTS_I4 VTS_PVARIANT)
    EVENT_CUSTOM("ScriptKeyDown", FireScriptKeyDown, VTS_PVARIANT VTS_I2)
    EVENT_CUSTOM("ScriptKeyPress", FireScriptKeyPress, VTS_PVARIANT)
    EVENT_CUSTOM("ScriptKeyUp", FireScriptKeyUp,  VTS_PVARIANT VTS_I2)
#endif // SS_V35

#ifdef SS_V70
    EVENT_CUSTOM("SheetChanging", FireSheetChanging, VTS_I2 VTS_I2 VTS_PVARIANT)
    EVENT_CUSTOM("SheetChanged", FireSheetChanged, VTS_I2 VTS_I2)
    EVENT_CUSTOM("TabScrolling", FireTabScrolling, VTS_I2 VTS_I2)
    EVENT_CUSTOM("TabScrolled", FireTabScrolled, VTS_I2 VTS_I2)
    EVENT_CUSTOM("BeforeEditMode", FireBeforeEditMode, VTS_I4 VTS_I4 VTS_I4 VTS_PVARIANT VTS_PVARIANT)
    EVENT_CUSTOM("DataLoaded", FireDataLoaded, VTS_NONE)
    EVENT_CUSTOM("BeforeScrollTip", FireBeforeScrollTip, VTS_BOOL VTS_I4 VTS_PBSTR VTS_PVARIANT)
    EVENT_CUSTOM("BeforeColMove", FireBeforeColMove, VTS_I4 VTS_I4 VTS_I4 VTS_PVARIANT)
    EVENT_CUSTOM("BeforeRowMove", FireBeforeRowMove, VTS_I4 VTS_I4 VTS_I4 VTS_PVARIANT)
#endif // SS_V70

#ifdef SS_V80
    EVENT_CUSTOM("CircularFormula", FireCircularFormula, VTS_I2 VTS_I4 VTS_I4)
    EVENT_CUSTOM_ID("CTCancelEditing", eventidCTCancelEditing, FireCTCancelEditing, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4)
    EVENT_CUSTOM_ID("CTCreateEditorControl", eventidCTCreateEditorControl, FireCTCreateEditorControl, VTS_BSTR VTS_PVARIANT)
    EVENT_CUSTOM_ID("CTCreateRendererControl", eventidCTCreateRendererControl, FireCTCreateRendererControl, VTS_BSTR VTS_PVARIANT)
    EVENT_CUSTOM_ID("CTGetEditorValue", eventidCTGetEditorValue, FireCTGetEditorValue, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_PVARIANT)
    EVENT_CUSTOM_ID("CTInitializeControl", eventidCTInitializeControl, FireCTInitializeControl, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_DISPATCH)
    EVENT_CUSTOM_ID("CTIsReservedKey", eventidCTIsReservedKey, FireCTIsReservedKey, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_I2 VTS_I2)
    EVENT_CUSTOM_ID("CTIsReservedLocation", eventidCTIsReservedLocation, FireCTIsReservedLocation, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XSIZE_PIXELS VTS_YSIZE_PIXELS VTS_DISPATCH VTS_VARIANT VTS_PI4)
    EVENT_CUSTOM_ID("CTIsValid", eventidCTIsValid, FireCTIsValid, VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_PBOOL)
    EVENT_CUSTOM_ID("CTGetPreferredSize", eventidCTGetPreferredSize, FireCTGetPreferredSize, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_DISPATCH VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XSIZE_PIXELS VTS_YSIZE_PIXELS VTS_VARIANT VTS_PXSIZE_PIXELS VTS_PYSIZE_PIXELS)
    EVENT_CUSTOM_ID("CTGetReservedCursor", eventidCTGetReservedCursor, FireCTGetReservedCursor, VTS_BSTR VTS_VARIANT VTS_I4 VTS_PDISPATCH)
    EVENT_CUSTOM_ID("CTPaintCell", eventidCTPaintCell, FireCTPaintCell, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_VARIANT VTS_HANDLE VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XSIZE_PIXELS VTS_YSIZE_PIXELS VTS_DISPATCH VTS_BOOL VTS_BOOL)
    EVENT_CUSTOM_ID("CTSetValue", eventidCTSetValue, FireCTSetValue, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_VARIANT)
    EVENT_CUSTOM_ID("CTStartEditing", eventidCTStartEditing, FireCTStartEditing, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BOOL VTS_BOOL)
    EVENT_CUSTOM_ID("CTStopEditing", eventidCTStopEditing, FireCTStopEditing, VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_PBOOL)
    EVENT_CUSTOM_ID("CTStringFormat", eventidCTStringFormat, FireCTStringFormat, VTS_BSTR VTS_VARIANT VTS_PBSTR)
    EVENT_CUSTOM_ID("CTStringUnformat", eventidCTStringUnformat, FireCTStringUnformat, VTS_BSTR VTS_BSTR VTS_PVARIANT)

#endif // SS_V80

END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
#if defined(SS_SPREADOCX) || defined(SS_RSOCX)
#ifndef SS_V30
BEGIN_PROPPAGEIDS(CSsocxCtrl, 8)
    PROPPAGEID(CSsocxPropPage::GetGuid()) 
    PROPPAGEID(CDisplayPropPage::GetGuid())
    PROPPAGEID(COpModePropPage::GetGuid())
    PROPPAGEID(CHeadersPropPage::GetGuid())
    PROPPAGEID(CEditModePropPage::GetGuid())
    PROPPAGEID(CVirtualPropPage::GetGuid())
    PROPPAGEID(CLSID_CFontPropPage)
    PROPPAGEID(CMyColorPropPage::GetGuid())
END_PROPPAGEIDS(CSsocxCtrl)
#endif // SS_V30
#elif defined(SS_GRIDOCX)
BEGIN_PROPPAGEIDS(CSsocxCtrl, 2)
    PROPPAGEID(CLSID_CFontPropPage)
    PROPPAGEID(CMyColorPropPage::GetGuid())
END_PROPPAGEIDS(CSsocxCtrl)
#endif
// RFW - 5/3/04 - 14180
BEGIN_PROPPAGEIDS(CSsocxCtrl, 1)
    PROPPAGEID(CLSID_CFontPropPage)
END_PROPPAGEIDS(CSsocxCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

/*
#if defined(SPREAD_JPN)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI Spread 3.0 coclass -scl
		// {BF15A0E6-4368-11d3-BB8D-00600845E4F3}
		IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadjADO.fpSpread.2",
		0xbf15a0e6, 0x4368, 0x11d3, 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3);
//      0xd166ee70, 0x4d87, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
    #else // non-ADO ANSI Spread 3.0 coclass -scl
		// {BF15A0E7-4368-11d3-BB8D-00600845E4F3}
		IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadj.Spread.2",
		0xbf15a0e7, 0x4368, 0x11d3, 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3);
//      0xe2454650, 0x4d87, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
    #endif // FP_ADB
  #endif // SS_V30
#elif defined(SS_SPREADOCX)
*/
#if defined(SS_SPREADOCX)
  #if defined(_UNICODE)
   #ifdef SS_V85
          // {71146838-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpreadADO.fpSpread.7", // FP_GUIDS : string and fpSpread coclass
		  0xDE52502E, 0xF837, 0x492b, 0xAE, 0x14, 0xA1, 0x82, 0x53, 0x1A, 0xFA, 0xF4);
	#elif SS_V80
      #ifdef SPREAD_JPN
        // ADO Unicode Spread 8.0 JAPAN coclass -scl
        // {857FBBF7-D0C6-44e6-AB18-D03165173D42}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpreadADOJ.fpSpread.6",
		0xC591EEDB, 0x580B, 0x426d, 0xAB, 0x60, 0x67, 0x48, 0x38, 0x11, 0xB6, 0x9F);
      #else
        #ifdef FP_ADB // ADO Unicode Spread 8.0 coclass -scl
          // {71146838-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpreadADO.fpSpread.6",
		  0xDE52502E, 0xF837, 0x492b, 0xAE, 0x14, 0xA1, 0x82, 0x53, 0x1A, 0xFA, 0xF4);
        #else // non-ADO Unicode Spread 8.0 coclass -scl
          // {71146839-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpread.Spread.6",
		  0xDE52502E, 0xF837, 0x492b, 0xAE, 0x14, 0xA1, 0x82, 0x53, 0x1A, 0xFA, 0xF4);
        #endif // FP_ADB
      #endif // SPREAD_JPN
	#elif SS_V70
      #ifdef SPREAD_JPN
        // ADO Unicode Spread 7.0 JAPAN coclass -scl
        // {857FBBF7-D0C6-44e6-AB18-D03165173D42}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpreadADOJ.fpSpread.5",
        0x857fbbf7, 0xd0c6, 0x44e6, 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42);
      #else
        #ifdef FP_ADB // ADO Unicode Spread 7.0 coclass -scl
          // {71146838-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpreadADO.fpSpread.5",
		  0x71146838, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
        #else // non-ADO Unicode Spread 7.0 coclass -scl
          // {71146839-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpread.Spread.5",
		  0x71146839, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
        #endif // FP_ADB
      #endif // SPREAD_JPN
    #elif SS_V40
      #ifdef FP_ADB // ADO Unicode Spread 6.0 coclass -scl
        // {B9411666-10E6-4a53-BE96-7FED334704FA}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpreadADO.fpSpread.4",
        0xb9411666, 0x10e6, 0x4a53, 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa);
      #else // non-ADO Unicode Spread 6.0 coclass -scl
        // {B9411667-10E6-4a53-BE96-7FED334704FA}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpread.Spread.4",
        0xb9411667, 0x10e6, 0x4a53, 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa);
      #endif // FP_ADB
    #elif SS_V30
      #ifdef FP_ADB // ADO Unicode fpSpread 3.0 coclass -scl
        // {87B6D4C0-4D87-11d2-B8B2-0000C00A958C}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpreadADO.fpUSpread.2",
        0x87b6d4c0, 0x4d87, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
      #else // non-ADO Unicode fpSpread 3.0 coclass -scl
        // {931A6680-4D87-11d2-B8B2-0000C00A958C}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpread.fpUSpread.2",
        0x931a6680, 0x4d87, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
      #endif // FP_ADB
    #else // non-ADO Unicode vaSpread 2.5 coclass -scl
      IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPUSpread.Spread.1",
      0xf5ca1ac3, 0xa9d8, 0x11cf, 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19)
    #endif //SS_V30
  #else // not Unicode -scl
   #ifdef SS_V85
          // {7114683A-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADO.fpSpread.7", // FP_GUIDS : string and fpSpread coclass
		  0x8E5109F0, 0xF4FF, 0x42e7, 0xAF, 0x24, 0x90, 0x7A, 0x9D, 0x53, 0x24, 0x76);
	#elif SS_V80
      #ifdef SPREAD_JPN
        #ifdef FP_ADB // ADO ANSI Spread 8.0 JAPAN coclass -scl
          // {857FBBF8-D0C6-44e6-AB18-D03165173D42}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADOJ.fpSpread.6",
		  0x3FE61CD7, 0x0925, 0x4ec6, 0xBB, 0x3A, 0x82, 0x5C, 0xE1, 0xA6, 0x34, 0x35);
        #else // non-ADO ANSI Spread 8.0 JAPAN coclass -scl
          // {857FBBF9-D0C6-44e6-AB18-D03165173D42}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadJ.Spread.6",
		  0x3FE61CD7, 0x0925, 0x4ec6, 0xBB, 0x3A, 0x82,0x5C, 0xE1, 0xA6, 0x34, 0x35);
        #endif // FP_ADB
      #else
        #ifdef FP_ADB // ADO ANSI Spread 8.0 coclass -scl
          // {7114683A-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADO.fpSpread.6",
		  0x8E5109F0, 0xF4FF, 0x42e7, 0xAF, 0x24, 0x90, 0x7A, 0x9D, 0x53, 0x24, 0x76);
        #else // non-ADO ANSI Spread 8.0 coclass -scl
          // {7114683B-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpread.Spread.6",
		  0x8E5109F0, 0xF4FF, 0x42e7, 0xAF, 0x24, 0x90, 0x7A, 0x9D, 0x53, 0x24, 0x76);
        #endif // FP_ADB
      #endif
	#elif SS_V70
      #ifdef SPREAD_JPN
        #ifdef FP_ADB // ADO ANSI Spread 7.0 JAPAN coclass -scl
          // {857FBBF8-D0C6-44e6-AB18-D03165173D42}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADOJ.fpSpread.5",
          0x857fbbf8, 0xd0c6, 0x44e6, 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42);
        #else // non-ADO ANSI Spread 7.0 JAPAN coclass -scl
          // {857FBBF9-D0C6-44e6-AB18-D03165173D42}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadJ.Spread.5",
          0x857fbbf9, 0xd0c6, 0x44e6, 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42);
        #endif // FP_ADB
      #else
        #ifdef FP_ADB // ADO ANSI Spread 7.0 coclass -scl
          // {7114683A-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADO.fpSpread.5",
		  0x7114683A, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
        #else // non-ADO ANSI Spread 7.0 coclass -scl
          // {7114683B-020D-4d16-80FD-6ACE384B66DF}
          IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpread.Spread.5",
		  0x7114683B, 0x20d, 0x4d16, 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf);
        #endif // FP_ADB
      #endif
    #elif SS_V40
      #ifdef FP_ADB // ADO ANSI Spread 6.0 coclass -scl
        // {41F841C0-AE16-11d5-8817-0050DA6EF5E5}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADO.fpSpread.4",
        0x41f841c0, 0xae16, 0x11d5, 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5);
      #else // non-ADO ANSI Spread 6.0 coclass -scl
        // {41F841C1-AE16-11d5-8817-0050DA6EF5E5}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpread.Spread.4",
        0x41f841c1, 0xae16, 0x11d5, 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5);
      #endif // FP_ADB
    #elif SS_V35
      #ifdef FP_ADB // ADO ANSI Spread 3.5 coclass -scl
        // {F6F0030F-7A0D-4106-849A-264C2F6D4052}
// fix for 9189 -scl
//        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADO.fpSpread.2",
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADO.fpSpread.3",
        0xf6f0030f, 0x7a0d, 0x4106, 0x84, 0x9a, 0x26, 0x4c, 0x2f, 0x6d, 0x40, 0x52);
      #else // non-ADO ANSI Spread 3.5 coclass -scl
        // {F5FF8524-A033-4836-8370-5365E38FC516}
// fix for 9189 -scl
//        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpread.Spread.2",
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpread.Spread.3",
        0xf5ff8524, 0xa033, 0x4836, 0x83, 0x70, 0x53, 0x65, 0xe3, 0x8f, 0xc5, 0x16);
      #endif // FP_ADB
    #elif SS_V30
      #ifdef FP_ADB // ADO ANSI Spread 3.0 coclass -scl
        // {D166EE70-4D87-11d2-B8B2-0000C00A958C}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpreadADO.fpSpread.2",
        0xd166ee70, 0x4d87, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
      #else // non-ADO ANSI Spread 3.0 coclass -scl
        // {E2454650-4D87-11d2-B8B2-0000C00A958C}
        IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpread.Spread.2",
        0xe2454650, 0x4d87, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
      #endif // FP_ADB
    #else // non-ADO ANSI Spread 2.5 -scl
      IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPSpread.Spread.1",
      0xb02f3641, 0x766b, 0x11ce, 0xaf, 0x28, 0xc3, 0xa2, 0xfb, 0xe7, 0x6a, 0x13)
    #endif // SS_V30
  #endif // _UNICODE
#elif defined(SS_GRIDOCX)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI fpGrid 3.0 coclass -scl
      // {B3687AC0-4D88-11d2-B8B2-0000C00A958C}
      IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPGridADO.fpGrid.2",
      0xb3687ac0, 0x4d88, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
    #else // non-ADO ANSI fpGrid 3.0 coclass -scl
      // {C023DDB0-4D88-11d2-B8B2-0000C00A958C}
      IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPGrid.fpGrid.2",
      0xc023ddb0, 0x4d88, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
    #endif // FP_ADB
  #else // non-ADO ANSI vaGrid 2.5 coclass -scl
    IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "FPGrid.Grid.1",
    0x4b687080, 0xf21e, 0x11ce, 0x86, 0x47, 0xd9, 0x52, 0x18, 0x5a, 0xe9, 0x18)
  #endif // SS_V30
#elif defined(SS_RSOCX)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI RSSpread 3.0 coclass -scl
      // {45E4FF30-4D89-11d2-B8B2-0000C00A958C}
      IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "RSSpreadADO.Spread.2",
      0x45e4ff30, 0x4d89, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
    #else // non-ADO ANSI RSSpread 3.0 coclass -scl
      // {4F1AD180-4D89-11d2-B8B2-0000C00A958C}
      IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "RSSpread.Spread.2",
      0x4f1ad180, 0x4d89, 0x11d2, 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c);
    #endif // FP_ADB
  #else // non-ADO ANSI RSSpread 3.0 coclass -scl
    IMPLEMENT_OLECREATE_EX(CSsocxCtrl, "RSSpread.Spread.1",
    0x5a276523, 0xc536, 0x11cf, 0x87, 0x9a, 0, 0xaa, 0, 0xbc, 0x8f, 0x75)
  #endif // SS_V30
#endif

/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CSsocxCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

/*
#if defined(SPREAD_JPN)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI fpSpread 3.0 dispinterfaces -scl
      const IID BASED_CODE IID_DSsocx =
			{ 0xbf15a0e2, 0x4368, 0x11d3, { 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3 } };
//        { 0xbdf071e0, 0x4d74, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      const IID BASED_CODE IID_DSsocxEvents =
			{ 0xbf15a0e4, 0x4368, 0x11d3, { 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3 } };
//        { 0x76baab50, 0x4d81, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #else // non-ADO ANSI fpSpread 3.0 dispinterfaces -scl
      const IID BASED_CODE IID_DSsocx =
			{ 0xbf15a0e3, 0x4368, 0x11d3, { 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3 } };
//        { 0x233be5c0, 0x4d75, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      const IID BASED_CODE IID_DSsocxEvents =
			{ 0xbf15a0e5, 0x4368, 0x11d3, { 0xbb, 0x8d, 0x0, 0x60, 0x8, 0x45, 0xe4, 0xf3 } };
//        { 0x842be590, 0x4d81, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #endif // FP_ADB
  #endif // SS_V30
#elif defined(SS_SPREADOCX)
*/
#if defined(SS_SPREADOCX)
  #if defined(_UNICODE)
   #ifdef SS_V85
        // {71146830-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0xDE5381C7, 0x0763, 0x4b3a, { 0xBB, 0x96, 0xEA, 0x5B, 0x78, 0x0E, 0xCF, 0x87 } }; // FP_GUIDS : _DSpreadSheet
        // {71146834-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x3390568D, 0x5A27, 0x4fa2,{ 0x9A, 0xAB, 0x2C, 0x68, 0x25, 0xF0, 0x00, 0x47 } }; // FP_GUIDS : _DSpreadEvents
	#elif SS_V80
      #ifdef SPREAD_JPN
        // ADO Unicode fpSpread 7.0 JAPAN dispinterfaces -scl
        // {857FBBF1-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocx =
		{ 0x74BD8670, 0xE210, 0x4b74, {0xA6, 0x1C, 0xC0, 0x5F, 0xF4, 0x72, 0xEB, 0x96  } };
        // {857FBBF2-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x107E9244, 0x9E99, 0x4183, { 0xBB, 0xDD, 0x07, 0xD4, 0x80, 0xD1, 0x32, 0x4A } } ;
      #else
        #ifdef FP_ADB // ADO Unicode fpSpread 7.0 dispinterfaces -scl
        // {71146830-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0xDE5381C7, 0x0763, 0x4b3a, { 0xBB, 0x96, 0xEA, 0x5B, 0x78, 0x0E, 0xCF, 0x87 } };
        // {71146834-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x3390568D, 0x5A27, 0x4fa2,{ 0x9A, 0xAB, 0x2C, 0x68, 0x25, 0xF0, 0x00, 0x47 } };
        #else // non-ADO Unicode fpSpread 7.0 dispinterfaces -scl
        // {71146831-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0x94A92B9F, 0xA8D6, 0x49ab, { 0x88, 0xE7, 0x94, 0x7C, 0xD4, 0x03, 0x30, 0xA6 } };
        // {71146835-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x57AA20D7, 0xD9EE, 0x4cea, {0x88, 0x76, 0x0E, 0x68, 0x68, 0xBE, 0xAD, 0x6C } };
        #endif // FP_ADB
      #endif
	#elif SS_V70
      #ifdef SPREAD_JPN
        // ADO Unicode fpSpread 7.0 JAPAN dispinterfaces -scl
        // {857FBBF1-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocx =
		{ 0x857fbbf1, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        // {857FBBF2-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x857fbbf2, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
      #else
        #ifdef FP_ADB // ADO Unicode fpSpread 7.0 dispinterfaces -scl
        // {71146830-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0x71146830, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146834-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x71146834, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        #else // non-ADO Unicode fpSpread 7.0 dispinterfaces -scl
        // {71146831-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0x71146831, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146835-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x71146835, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        #endif // FP_ADB
      #endif
    #elif SS_V40
      #ifdef FP_ADB // ADO Unicode fpSpread 6.0 dispinterfaces -scl
        // {B9411662-10E6-4a53-BE96-7FED334704FA}
        const IID BASED_CODE IID_DSsocx =
        { 0xb9411662, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
        // {B9411664-10E6-4a53-BE96-7FED334704FA}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0xb9411664, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
      #else // non-ADO Unicode fpSpread 6.0 dispinterfaces -scl
        // {B9411663-10E6-4a53-BE96-7FED334704FA}
        const IID BASED_CODE IID_DSsocx =
        { 0xb9411663, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
        // {B9411665-10E6-4a53-BE96-7FED334704FA}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0xb9411665, 0x10e6, 0x4a53, { 0xbe, 0x96, 0x7f, 0xed, 0x33, 0x47, 0x4, 0xfa } };
      #endif // FP_ADB
    #elif SS_V30
      #ifdef FP_ADB // ADO Unicode Spread 3.0 dispinterfaces -scl
        // {CA5A3B30-4D7B-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocx =
          { 0xca5a3b30, 0x4d7b, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
        // {9698EFE0-4D80-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocxEvents =
          { 0x9698efe0, 0x4d80, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      #else // non-ADO Unicode fpSpread 3.0 dispinterfaces -scl
        // {1E51ADC0-4D7C-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocx =
          { 0x1e51adc0, 0x4d7c, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
        // {BB34A0D0-4D80-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocxEvents =
          { 0xbb34a0d0, 0x4d80, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      #endif // FP_ADB
    #else // non-ADO Unicode vaSpread 2.5 dispinterfaces -scl
      const IID BASED_CODE IID_DSsocx =
        { 0xf5ca1ac1, 0xa9d8, 0x11cf, { 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19 } };
      const IID BASED_CODE IID_DSsocxEvents =
        { 0xf5ca1ac2, 0xa9d8, 0x11cf, { 0x87, 0x5e, 0x87, 0x2a, 0x37, 0x29, 0xe, 0x19 } };
    #endif // SS_V30
  #else // not Unicode -scl
   #ifdef SS_V85
        // {71146832-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0xB373C459, 0xBC86, 0x4499, {0x8D, 0xC9, 0x25, 0xD8, 0x89, 0x3F, 0xE3, 0xEE } }; // FP_GUIDS : _DSpreadSheet
        // {71146836-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0xEB915D5D, 0xBD94, 0x4648, { 0x90, 0xFB, 0x37, 0xE9, 0xB6, 0x8D, 0x63, 0xA7 } }; // FP_GUIDS : _DSpreadEvents
	#elif SS_V80
      #ifdef SPREAD_JPN
        #ifdef FP_ADB // ADO ANSI fpSpread 8.0 JAPAN dispinterfaces -scl
        // {857FBBF3-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocx =
		{ 0xF4A90BD3, 0x8336, 0x4c04, { 0x85, 0xDC, 0xED, 0xA4, 0x04, 0x1A, 0x63, 0xD3 } };
        // {857FBBF4-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0xCC803617, 0x8FF6, 0x4cd5, { 0x94, 0x60, 0x17, 0x41, 0xD2, 0xAC, 0x80, 0xC7 } };
        #else // non-ADO ANSI fpSpread 8.0 JAPAN dispinterfaces -scl
        // {857FBBF5-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocx =
		{ 0x66BAB2C0, 0xC90E, 0x4792, { 0x94, 0xB4, 0x4D, 0x16, 0x29, 0xD3, 0x6F, 0x5E } };
        // {857FBBF6-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0xC416DAD0, 0x990B, 0x40e8, { 0xAE, 0x8D, 0x71, 0xB2, 0x46, 0x7F, 0xFA, 0x03 } };
        #endif // FP_ADB
      #else
        #ifdef FP_ADB // ADO ANSI fpSpread 8.0 dispinterfaces -scl
        // {71146832-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0xB373C459, 0xBC86, 0x4499, {0x8D, 0xC9, 0x25, 0xD8, 0x89, 0x3F, 0xE3, 0xEE } };
        // {71146836-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0xEB915D5D, 0xBD94, 0x4648, { 0x90, 0xFB, 0x37, 0xE9, 0xB6, 0x8D, 0x63, 0xA7 } };
        #else // non-ADO ANSI fpSpread 8.0 dispinterfaces -scl
        // {71146833-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0x1190CC45, 0x9DE2, 0x4ca5, {0x85, 0x85, 0x13, 0x4D, 0x3E, 0x9D, 0xF9, 0xE7 } };
        // {71146837-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x5DDCC739, 0x8F74, 0x421e, { 0x80, 0x38, 0x13, 0xDD, 0x26, 0x35, 0x08, 0x67 } };
        #endif // FP_ADB
      #endif
	#elif SS_V70
      #ifdef SPREAD_JPN
        #ifdef FP_ADB // ADO ANSI fpSpread 7.0 JAPAN dispinterfaces -scl
        // {857FBBF3-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocx =
		{ 0x857fbbf3, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        // {857FBBF4-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x857fbbf4, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        #else // non-ADO ANSI fpSpread 7.0 JAPAN dispinterfaces -scl
        // {857FBBF5-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocx =
		{ 0x857fbbf5, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        // {857FBBF6-D0C6-44e6-AB18-D03165173D42}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x857fbbf6, 0xd0c6, 0x44e6, { 0xab, 0x18, 0xd0, 0x31, 0x65, 0x17, 0x3d, 0x42 } };
        #endif // FP_ADB
      #else
        #ifdef FP_ADB // ADO ANSI fpSpread 7.0 dispinterfaces -scl
        // {71146832-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0x71146832, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146836-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x71146836, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        #else // non-ADO ANSI fpSpread 7.0 dispinterfaces -scl
        // {71146833-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocx =
		{ 0x71146833, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        // {71146837-020D-4d16-80FD-6ACE384B66DF}
        const IID BASED_CODE IID_DSsocxEvents =
		{ 0x71146837, 0x20d, 0x4d16, { 0x80, 0xfd, 0x6a, 0xce, 0x38, 0x4b, 0x66, 0xdf } };
        #endif // FP_ADB
      #endif
    #elif SS_V40
      #ifdef FP_ADB // ADO ANSI fpSpread 6.0 dispinterfaces -scl
        // {41F841C2-AE16-11d5-8817-0050DA6EF5E5}
        const IID BASED_CODE IID_DSsocx =
        { 0x41f841c2, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
        // {41F841C4-AE16-11d5-8817-0050DA6EF5E5}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0x41f841c4, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
      #else // non-ADO ANSI fpSpread 6.0 dispinterfaces -scl
        // {41F841C3-AE16-11d5-8817-0050DA6EF5E5}
        const IID BASED_CODE IID_DSsocx =
        { 0x41f841c3, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
        // {41F841C5-AE16-11d5-8817-0050DA6EF5E5}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0x41f841c5, 0xae16, 0x11d5, { 0x88, 0x17, 0x0, 0x50, 0xda, 0x6e, 0xf5, 0xe5 } };
      #endif // FP_ADB
    #elif SS_V35
      #ifdef FP_ADB // ADO ANSI fpSpread 3.5 dispinterfaces -scl
        // {FCC384F5-899A-4611-A885-ACF1A13397F9}
        const IID BASED_CODE IID_DSsocx =
        { 0xfcc384f5, 0x899a, 0x4611, { 0xa8, 0x85, 0xac, 0xf1, 0xa1, 0x33, 0x97, 0xf9 } };
        // {F0A4FB27-3EC0-43e9-BCCF-699BA092C8C4}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0xf0a4fb27, 0x3ec0, 0x43e9, { 0xbc, 0xcf, 0x69, 0x9b, 0xa0, 0x92, 0xc8, 0xc4 } };
      #else // non-ADO ANSI fpSpread 3.5 dispinterfaces -scl
        // {F0DE1E34-8C70-423b-96CD-450074BAB784}
        const IID BASED_CODE IID_DSsocx =
        { 0xf0de1e34, 0x8c70, 0x423b, { 0x96, 0xcd, 0x45, 0x0, 0x74, 0xba, 0xb7, 0x84 } };
        // {FE6E6C70-BCEB-4524-A88C-7E0AE79C0F04}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0xfe6e6c70, 0xbceb, 0x4524, { 0xa8, 0x8c, 0x7e, 0xa, 0xe7, 0x9c, 0xf, 0x4 } };
      #endif // FP_ADB
    #elif SS_V30
      #ifdef FP_ADB // ADO ANSI fpSpread 3.0 dispinterfaces -scl
        // {BDF071E0-4D74-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocx =
        { 0xbdf071e0, 0x4d74, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
        // {76BAAB50-4D81-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0x76baab50, 0x4d81, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      #else // non-ADO ANSI fpSpread 3.0 dispinterfaces -scl
        // {233BE5C0-4D75-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocx =
        { 0x233be5c0, 0x4d75, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
        // {842BE590-4D81-11d2-B8B2-0000C00A958C}
        const IID BASED_CODE IID_DSsocxEvents =
        { 0x842be590, 0x4d81, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      #endif // FP_ADB
    #else // non-ADO ANSI vaSpread 2.5 dispinterfaces -scl
      const IID BASED_CODE IID_DSsocx =
        { 0xc75c4fe1, 0x848d, 0x11ce, { 0xaf, 0x28, 0x86, 0x1b, 0xf4, 0x69, 0x9, 0xcc } };
      const IID BASED_CODE IID_DSsocxEvents =
        { 0xc75c4fe3, 0x848d, 0x11ce, { 0xaf, 0x28, 0x86, 0x1b, 0xf4, 0x69, 0x9, 0xcc } };
    #endif // SS_V30
  #endif // _UNICODE
#elif defined(SS_GRIDOCX)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI fpGrid 3.0 dispinterfaces -scl
      // {C0F09600-4D7C-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocx =
        { 0xc0f09600, 0x4d7c, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      // {F50D5860-4D81-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocxEvents =
        { 0xf50d5860, 0x4d81, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #else // non-ADO ANSI fpGrid 3.0 dispinterfaces -scl
      // {E1E209D0-4D7C-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocx =
        { 0xe1e209d0, 0x4d7c, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      // {FF8101B0-4D81-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocxEvents =
        { 0xff8101b0, 0x4d81, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #endif // FP_ADB
  #else // non-ADO ANSI vaGrid 2.5 dispinterfaces -scl
    const IID BASED_CODE IID_DSsocx =
      { 0x4b687081, 0xf21e, 0x11ce, { 0x86, 0x47, 0xd9, 0x52, 0x18, 0x5a, 0xe9, 0x18 } };
    const IID BASED_CODE IID_DSsocxEvents =
      { 0x4b687082, 0xf21e, 0x11ce, { 0x86, 0x47, 0xd9, 0x52, 0x18, 0x5a, 0xe9, 0x18 } };
  #endif // SS_V30
#elif defined(SS_RSOCX)
  #ifdef SS_V30
    #ifdef FP_ADB // ADO ANSI RSSpread 3.0 dispinterfaces -scl
      // {6D1C6A40-4D7D-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocx =
        { 0x6d1c6a40, 0x4d7d, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      // {FA1ED010-4D82-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocxEvents =
        { 0xfa1ed010, 0x4d82, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #else // non-ADO ANSI RSSpread 3.0 dispinterfaces -scl
      // {956E91D0-4D7D-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocx =
        { 0x956e91d0, 0x4d7d, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
      // {06C34730-4D83-11d2-B8B2-0000C00A958C}
      const IID BASED_CODE IID_DSsocxEvents =
        { 0x6c34730, 0x4d83, 0x11d2, { 0xb8, 0xb2, 0x0, 0x0, 0xc0, 0xa, 0x95, 0x8c } };
    #endif // FP_ADB
  #else // non-ADO ANSI RSSpread 2.5 dispinterfaces -scl
    const IID BASED_CODE IID_DSsocx =
      { 0x5a276521, 0xc536, 0x11cf, { 0x87, 0x9a, 0, 0xaa, 0, 0xbc, 0x8f, 0x75 } };
    const IID BASED_CODE IID_DSsocxEvents =
      { 0x5a276522, 0xc536, 0x11cf, { 0x87, 0x9a, 0, 0xaa, 0, 0xbc, 0x8f, 0x75 } };
  #endif // SS_V30
#endif

/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwSsocxOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_RECOMPOSEONRESIZE;

#if defined(SS_SPREADOCX) || defined(SS_RSOCX)
IMPLEMENT_OLECTLTYPE(CSsocxCtrl, IDS_SSOCX + LANGUAGE_BASE, _dwSsocxOleMisc)
#elif defined(SS_GRIDOCX)
IMPLEMENT_OLECTLTYPE(CSsocxCtrl, IDS_SSOCX_GRID, _dwSsocxOleMisc)
#endif



/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::CSsocxCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CSsocxCtrl

BOOL CSsocxCtrl::CSsocxCtrlFactory::UpdateRegistry(BOOL bRegister)
{

    if (bRegister)
    {   
        
#ifdef WIN32
// ActiveX component categories - CTF
//	    HRESULT hr;

//	    hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls that are safely scriptable");
//	    hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");
//	    hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
//	    hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);

		
		
		// ActiveX component categories - CTF
#endif

        return AfxOleRegisterControlClass(
//             AfxGetInstanceHandle(),
               hDynamicInst,
               m_clsid,
               m_lpszProgID,
               IDS_SSOCX,
               IDB_SSOCX,
#ifdef WIN32
               afxRegApartmentThreading,
#else
               FALSE,                      //  Not insertable
#endif
               _dwSsocxOleMisc,
               _tlid,
               _wVerMajor,
               _wVerMinor);
    }
    else
    {
#ifdef WIN32
// ActiveX component categories - CTF
//	    HRESULT hr;
	    
//	    hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
//	    hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
// ActiveX component categories - CTF
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


/*
void GetTheNameOfContainerDocument(LPVOID lpObject)
{
IMoniker* ptrfullMoniker = NULL;
char objectname[300];
LPOLESTR ppszDisplaynamefull;
IBindCtx* pbcfull = NULL;
COleControl *pObject = (COleControl *)lpObject;

LPOLECLIENTSITE pOleClientSite = pObject->GetClientSite();

if(pOleClientSite)
{
 if(SUCCEEDED(pOleClientSite->GetMoniker(OLEGETMONIKER_FORCEASSIGN,         
                              OLEWHICHMK_OBJFULL, &ptrfullMoniker)))
    // The typedefs for OLEGETMONIKER and OLWHICHMK are in oleidl.h
 {
  if (SUCCEEDED(CreateBindCtx( 0, &pbcfull )))
  {
   if(SUCCEEDED(ptrfullMoniker->GetDisplayName(pbcfull,NULL,
                                           &ppszDisplaynamefull)))
   {
	wcstombs(objectname,ppszDisplaynamefull,300);
	AfxMessageBox(objectname);
//	AfxMessageBox(ExtractDocumentName(objectname));
	ptrfullMoniker->Release();
   }
  }
  pbcfull->Release();
 }
 pOleClientSite->Release();
}
}
*/


BOOL FileExist(LPTSTR lpszLicenseFileName)
{
HANDLE hFile;
BOOL   fRet = FALSE;
TCHAR  lpszLicFilePath[200];

UINT len = GetSystemDirectory(lpszLicFilePath, 200);
_stprintf(lpszLicFilePath, _T("%s\\%s"), lpszLicFilePath, lpszLicenseFileName);

hFile = CreateFile(lpszLicFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                   NULL);

if (hFile != INVALID_HANDLE_VALUE)
{
	fRet = TRUE;
  CloseHandle(hFile);
}

return (fRet);
}


#if 0
void SS_LogMsg(LPTSTR lpStr)
{
OFSTRUCT    OfStruct;
HFILE       hFile;
TCHAR       Buffer[200];
LPTSTR      Dir = _T("\\fp.log");

if (OpenFile((LPSTR)Dir, &OfStruct, OF_EXIST) == HFILE_ERROR)
   {
   hFile = OpenFile((LPSTR)Dir, &OfStruct, OF_CREATE | OF_WRITE);
   _lclose(hFile);
   }

hFile = OpenFile((LPSTR)Dir, &OfStruct, OF_WRITE);

lstrcpy(Buffer, lpStr);

// Add a time stamp to the log
{
TIMEFORMAT TimeFormat = {TRUE, TRUE, ':', FALSE};

lstrcat(Buffer, _T(", "));
TimeGetCurrentTime(&Buffer[lstrlen(Buffer)], &TimeFormat);
}

lstrcpy(&Buffer[lstrlen(Buffer)], _T("\r\n"));

_llseek(hFile, 0, 2);
_lwrite(hFile, Buffer, lstrlen(Buffer)*sizeof(TCHAR));
_lclose(hFile);
}
#endif


BOOL xIsContainerOffice()
{
  // If MS Office then ignore licensing
  TCHAR szFileName[200];
  TCHAR *szSearchNames[] = {_T("excel.exe"), _T("visio.exe"), _T("word.exe"), _T("msaccess.exe"), _T("outlook.exe"), _T("view.exe")};
  short iLenFileName;
  short i;

  GetModuleFileName(NULL, szFileName, sizeof(szFileName));
  iLenFileName = lstrlen(szFileName);

  //SS_LogMsg(szFileName); // Shannon

  for (i = 0; i < sizeof(szSearchNames) / sizeof(char *); i++)
    if (iLenFileName >= lstrlen(szSearchNames[i]) &&
        lstrcmpi(&szFileName[iLenFileName - lstrlen(szSearchNames[i])], szSearchNames[i]) == 0)
		{
		TCHAR *lpszLicFile;
#ifdef SS_V70
		lpszLicFile = _T("fpSpread70.lic");
#else
		lpszLicFile = _T("fpSpread60.lic");
#endif

		if (FileExist(lpszLicFile))
			return (TRUE);

		return (FALSE);
		}

    return (FALSE);
}


BOOL IsContainerOffice()
{
  static BOOL fIsOffice = FALSE;
  static BOOL fInitialized = FALSE;

  if (!fInitialized)
  {
    fInitialized = TRUE;
    fIsOffice = xIsContainerOffice();
  }

  return (fIsOffice);
}


BOOL xIsContainerAccess()
{
  // If MS Office then ignore licensing
  TCHAR szFileName[200];
  TCHAR *szSearchNames[] = { _T("msaccess.exe")};
  short iLenFileName;
  short i;

  GetModuleFileName(NULL, szFileName, sizeof(szFileName));
  iLenFileName = lstrlen(szFileName);

  for (i = 0; i < sizeof(szSearchNames) / sizeof(char *); i++)
    if (iLenFileName >= lstrlen(szSearchNames[i]) &&
        lstrcmpi(&szFileName[iLenFileName - lstrlen(szSearchNames[i])], szSearchNames[i]) == 0)
		return (TRUE);

    return (FALSE);
}


BOOL IsContainerAccess()
{
  static BOOL fIsAccess = FALSE;
  static BOOL fInitialized = FALSE;

  if (!fInitialized)
  {
    fInitialized = TRUE;
    fIsAccess = xIsContainerAccess();
  }

  return (fIsAccess);
}

                                          
/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::CSsocxCtrlFactory::VerifyUserLicense -
// Checks for existence of a user license

BOOL CSsocxCtrl::CSsocxCtrlFactory::VerifyUserLicense()
{
#ifdef SYNERGEX_TRIAL
  SYSTEMTIME st;

  GetSystemTime(&st);
// change expiration date to May 12, 2002
//  if( st.wYear > 2001 )
  if( st.wYear >= 2002 && st.wMonth >= 5 && st.wDay >= 12 )
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
#elif defined(FP_UNICODE_TEST) //GAB Set to expire March 31, 2004
  SYSTEMTIME st;

  GetSystemTime(&st);
  if( (st.wYear >= 2004) && (st.wMonth > 3) )
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

  // If MS Office then ignore licensing
  if (IsContainerOffice())
    return (TRUE);

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
// CSsocxCtrl::CSsocxCtrlFactory::GetLicenseKey -
// Returns a runtime licensing key

BOOL CSsocxCtrl::CSsocxCtrlFactory::GetLicenseKey(DWORD dwReserved,
                                                  BSTR FAR* pbstrKey)
{
//  WORD wSplash = fpSplashIsExpired(AfxGetResourceHandle());

#if defined(FP_SPLASHEXPIRE) //JHS Set to expire Nov 13, 2008
  SYSTEMTIME st;
  int retVal = 0;
//  short nType;

  GetSystemTime(&st);
  if( st.wYear >= 2009 && st.wMonth >= 12 && st.wDay >= 2 )
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

  // If MS Office then ignore licensing
  if (IsContainerOffice())
  {
    if (pbstrKey == NULL)
        return FALSE;

    *pbstrKey = SysAllocString(_szLicString);

    return (*pbstrKey != NULL);
  }

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

#ifndef CS_OFF
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

BOOL CSsocxCtrl::CSsocxCtrlFactory::VerifyLicenseKey(BSTR bstrKey)
{
	BOOL bLicensed = FALSE;
	BSTR bstr = NULL;

#ifdef FP_UNICODE_TEST //GAB Set to expire March 31, 2004
  SYSTEMTIME st;

  GetSystemTime(&st);
  if( (st.wYear >= 2004) && (st.wMonth > 3) )
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
#else
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
#endif
}


/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::CSsocxCtrl - Constructor


static const FONTDESC _fontdescSS =
{
#ifndef SPREAD_JPN
//  sizeof(FONTDESC), OLESTR("MS Sans Serif"), FONTSIZE(8), FW_BOLD,
  sizeof(FONTDESC), OLESTR("MS Sans Serif"), FONTSIZE(8), FW_NORMAL,
  DEFAULT_CHARSET, FALSE, FALSE, FALSE
#else
// 96' 5/26 BOC Gao. for setting default font
//  sizeof(FONTDESC), OLESTR("System"), FONTSIZE(8), FW_BOLD,
//  ANSI_CHARSET, FALSE, FALSE, FALSE
#ifdef WIN32
//#ifdef SS_V40
//  sizeof(FONTDESC), OLESTR("MS PGothic"), FONTSIZE(9), FW_NORMAL,
//  128, FALSE, FALSE, FALSE
//#else
  sizeof(FONTDESC), OLESTR("‚l‚r ‚oƒSƒVƒbƒN"), FONTSIZE(9), FW_NORMAL,
  SHIFTJIS_CHARSET, FALSE, FALSE, FALSE
//#endif
#else
  sizeof(FONTDESC), OLESTR("System"), FONTSIZE(8), FW_NORMAL,
  SHIFTJIS_CHARSET, FALSE, FALSE, FALSE
#endif
// -----------------------------------------<<
#endif
};

void DateDefault(LPDATEFORMAT lpDateFormat)
{
  TCHAR         szTemp[20];
  LPTSTR        Ptr;
  UINT          uiDate;

  uiDate = GetProfileInt(_T("intl"), _T("iDate"), -1);
  switch (uiDate)
  {
    case -1:
    case 0:
      lpDateFormat->nFormat = IDF_MMDDYY;
      break;
    case 1:
      lpDateFormat->nFormat = IDF_DDMMYY;
      break;
    case 2:
      lpDateFormat->nFormat = IDF_YYMMDD;
      break;
    default:
      lpDateFormat->nFormat = IDF_MMDDYY;
      break;

  }
  GetProfileString (_T("intl"), _T("sDate"), _T("/"), szTemp, 2);
  if (szTemp[0])
    lpDateFormat->cSeparator = szTemp[0];
  else                               // Windows seems to return NULL
    lpDateFormat->cSeparator = ' ';  //    if you specified a space.
  /* RFW - 2/25/05 - 15795
  GetProfileString(_T("intl"), _T("sShortDate"), _T("M/d/yy"), szTemp, 11);
  */
  GetProfileString(_T("intl"), _T("sShortDate"), _T("M/d/yy"), szTemp, 20);
  lpDateFormat->bCentury = 0;
  if (Ptr = StriChr(szTemp, 'y'))
    if (StriStr(Ptr, _T("yyyy")))
      lpDateFormat->bCentury = 1;

  if (lpDateFormat->nFormat == IDF_DDMMYY)
    if (Ptr = StriChr(szTemp, 'M'))
      if (StriStr(Ptr, _T("MMM")))
			lpDateFormat->nFormat = IDF_DDMONYY;

  lpDateFormat->bSpin = 0;
}


void TimeDefault(LPTIMEFORMAT lpTimeFormat)
{
  TCHAR szTemp[2];

  lpTimeFormat->b24Hour = (BOOL)(GetProfileInt(_T("intl"), _T("iTime"), 0) ? 1 : 0);
  lpTimeFormat->bSeconds = FALSE;
  GetProfileString(_T("intl"), _T("sTime"), _T(":"), szTemp, 2);
  lpTimeFormat->cSeparator = szTemp[0];
  lpTimeFormat->bSpin = 0;
}

void CSsocxCtrl::InitOcx(void)
{
  DATEFORMAT defDateFormat;
  TIMEFORMAT defTimeFormat;
  int i;

  m_lCol = SS_ALLCOLS;
  m_lRow = SS_ALLROWS;
  m_lCol2 = 0;
  m_lRow2 = 0;
  m_lDestCol = 0;
  m_lDestRow = 0;
  m_bBlockMode = FALSE;
  m_bCalcDependencies = TRUE;
  m_lppictCursorIcon = NULL;
  for( i = 0; i < sizeof(m_lppictCursors)/sizeof(m_lppictCursors[0]); i++ )
    m_lppictCursors[i] = NULL;
  for( i = 0; i < sizeof(m_hcurCursors)/sizeof(m_hcurCursors[0]); i++ )
    m_hcurCursors[i] = NULL;
  m_nCursorType = 0;
  m_nFileNum = 0;
  m_hDCPrinter = NULL;
  m_hMultiSelBlocks = NULL;
  m_nMultiSelCount = 0;
  m_nMultiSelIndex = 0;
  m_nPosition = 0;
  m_nSortBy = 0;
  m_hSortKeys = 0;
  m_nSortKeyCnt = 0;
  m_nCellBorderStyle = 0;
  m_nCellBorderType = 0;
  m_crCellBorderColor = 0;
  m_nCellType = 0;
#ifdef SS_V30
  m_lPrintNextPageBreakCol = 1;
  m_lPrintNextPageBreakRow = 1;
#endif
  m_lpCFResult = NULL;
  m_lpCFArgs = NULL;
  m_nCFArgCnt = 0;
  m_nTypeButtonAlign = 0;  // bottom
  m_crTypeButtonBorderColor = RGB(0,0,0);
  m_crTypeButtonColor = SS_EncodeSysColor(COLOR_BTNFACE);
  m_crTypeButtonDarkColor = SS_EncodeSysColor(COLOR_BTNSHADOW);
  m_crTypeButtonLightColor = SS_EncodeSysColor(COLOR_BTNHIGHLIGHT);
  m_lppictTypeButtonPicture = NULL;
  m_lppictTypeButtonPictureDown = NULL;
  m_nTypeButtonShadowSize = 2;
  m_sTypeButtonText = _T("");
  m_crTypeButtonTextColor = SS_EncodeSysColor(COLOR_BTNTEXT);
  m_nTypeButtonType = 0;  // normal
  m_bTypeCheckCenter = FALSE;
  for( i = 0; i < 6; i++ )
    m_lppictTypeCheckPicture[i] = NULL;
  m_nTypeCheckPictureIndex = 0;
  m_sTypeCheckText = _T("");
  m_nTypeCheckTextAlign = 1;  // right
  m_nTypeCheckType = 0;  // normal
  m_bTypeComboBoxEditable = FALSE;
  m_nTypeComboBoxIndex = 0;
  m_sTypeComboBoxList = _T("");
  DateDefault(&defDateFormat);
  m_bTypeDateCentury = defDateFormat.bCentury;
  m_nTypeDateFormat = defDateFormat.nFormat - IDF_DDMONYY;
  m_TypeDateMax.nDay = 31;
  m_TypeDateMax.nMonth = 12;
#ifdef SPREAD_JPN
  m_TypeDateMax.nYear = 2099;
#else
  m_TypeDateMax.nYear = 2100;
#endif
  m_TypeDateMin.nDay = 1;
  m_TypeDateMin.nMonth = 1;
#ifdef SPREAD_JPN
  m_TypeDateMin.nYear = 1900;
#else
  m_TypeDateMin.nYear = 1990;
#endif
  m_nTypeDateSeparator = defDateFormat.cSeparator;
  m_nTypeEditCharCase = 1;  // no case
  m_nTypeEditCharSet = 0;   // ASCII
#ifdef SS_V70
  m_lTypeEditLen = 32000;
#else
  m_lTypeEditLen = 60;
#endif
  m_bTypeEditMultiLine = FALSE;
  m_bTypeEditPassword = FALSE;
  m_nTypeFloatCurrencyChar = 36;
  m_nTypeFloatDecimalChar = 46;  // period
  m_nTypeFloatDecimalPlaces = 2;
  m_dfTypeFloatMax = 9999999.99;
  m_dfTypeFloatMin = -9999999.99;
  m_bTypeFloatMoney = FALSE;
  m_bTypeFloatSeparator = FALSE;
  m_nTypeFloatSepChar = 44;  // comma
  m_lTypeIntegerMax = 9999999;
  m_lTypeIntegerMin = -9999999;
  m_lTypeIntegerSpinInc = 1;
  m_bTypeIntegerSpinWrap = FALSE;
  m_lTypeOwnerDrawStyle = 0;
  m_sTypePicDefaultText = _T("");
  m_sTypePicMask = _T("");
  m_bTypePictCenter = FALSE;
  m_bTypePictMaintainScale = FALSE;
  m_lppictTypePictPicture = NULL;
  m_bTypePictStretch = FALSE;
  m_bTypeSpin = FALSE;
  m_nTypeTextAlignVert = 2;  // top
  m_bTypeTextPrefix = FALSE ;
  m_bTypeTextShadow = FALSE;
  m_bTypeTextShadowIn = FALSE;
  m_bTypeTextWordWrap = FALSE;
  TimeDefault(&defTimeFormat);
  m_nTypeTime24Hour = defTimeFormat.b24Hour;
  m_TypeTimeMax.nHour = 23;
  m_TypeTimeMax.nMinute = 59;
  m_TypeTimeMax.nSecond = 59;
  m_TypeTimeMin.nHour = 0;
  m_TypeTimeMin.nMinute = 0;
  m_TypeTimeMin.nSecond = 0;
  m_bTypeTimeSeconds = defTimeFormat.bSeconds;
  m_nTypeTimeSeparator = defTimeFormat.cSeparator;
  m_nTypeComboMaxDrop = 6;
  m_nTypeComboWidth = 0;
  m_nTypeComboMaxEditLen = 150;

  m_nTextTip = 0;
  fTextTipSet = FALSE;
  m_lTextTipDelay = 500;
  fTextTipDelaySet = FALSE;

  m_hWndPrintDlgProc = 0; 
  m_fSafeForScripting = FALSE;
  m_hWndSpreadDsg = (HWND)0; // Spread Designer
  m_fIgnoreKeyDown = FALSE;

#ifdef FP_OLEDRAG
  m_OLEDropMode = 0;
  m_pfpDataObject = NULL;
#endif

#ifdef SPREAD_JPN
  m_bFiring = FALSE;
#endif

  m_fInsideCreateControlWindow = FALSE;

  LPSS_BOOK lpBook = LockBook();

  if( m_hAccel )
    SS_DestroyAcceleratorTable(m_hAccel);
  m_hAccel = 0;
  m_cAccel = 0;

  lpBook->fRetainSelBlock = TRUE; // ???? agrees with VBX, disagrees with VBX 2.1 manual ????

  SSOcxInitDB(m_hVBSpread);

 /* _fmemset(lpBook->m_szPrinterDevice, '\0', sizeof(lpBook->m_szPrinterDevice));
  _fmemset(lpBook->m_szPrinterDriver, '\0', sizeof(lpBook->m_szPrinterDriver));
  _fmemset(lpBook->m_szPrinterOutput, '\0', sizeof(lpBook->m_szPrinterOutput));
  _fmemset(&(lpBook->m_PrinterDevMode), '\0', sizeof(lpBook->m_PrinterDevMode));
*/
#ifdef SS_V35
  m_nShowScrollTips = SS_SHOWSCROLLTIPS_OFF;
  m_nCellNoteIndicator = SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT;
#endif

  SetEventEnabled(0, TRUE);

#ifdef SS_V40
  m_bTypeComboAutoSearch = 0;

  _fmemset(&m_TypeCurrency, '\0', sizeof(SS_CELLTYPE));
  m_TypeCurrency.Type = SS_TYPE_CURRENCY;
  m_TypeCurrency.Style = ES_RIGHT;
  m_TypeCurrency.Spec.Currency.Right = 2;
  m_TypeCurrency.Spec.Currency.Max = 9999999.99;
  m_TypeCurrency.Spec.Currency.Min = -9999999.99;
  m_TypeCurrency.Spec.Currency.SpinInc = 1.1;
  m_TypeCurrency.Spec.Currency.fShowCurrencySymbol = TRUE;

  _fmemset(&m_TypeNumber, '\0', sizeof(SS_CELLTYPE));
  m_TypeNumber.Type = SS_TYPE_NUMBER;
  m_TypeNumber.Style = ES_RIGHT;
  m_TypeNumber.Spec.Number.Right = 2;
  m_TypeNumber.Spec.Number.Max = 9999999.99;
  m_TypeNumber.Spec.Number.Min = -9999999.99;
  m_TypeNumber.Spec.Number.SpinInc = 1.1;

  _fmemset(&m_TypePercent, '\0', sizeof(SS_CELLTYPE));
  m_TypePercent.Type = SS_TYPE_PERCENT;
  m_TypePercent.Style = ES_RIGHT;
  m_TypePercent.Spec.Percent.Right = 2;
  m_TypePercent.Spec.Percent.Max = 9999.99;
  m_TypePercent.Spec.Percent.Min = -9999.99;
  m_TypePercent.Spec.Percent.SpinInc = 1.1;

  m_fTypeNegRed = FALSE;
#endif // SS_V40

#ifdef SS_V70
  _fmemset(&m_TypeScientific, '\0', sizeof(SS_CELLTYPE));
  m_TypeScientific.Type = SS_TYPE_SCIENTIFIC;
  m_TypeScientific.Style = ES_RIGHT;
  m_TypeScientific.Spec.Scientific.Right = 2;
  m_TypeScientific.Spec.Scientific.Max = 1.7E308;
  m_TypeScientific.Spec.Scientific.Min = -1.7E308;

  /*
  m_dfPrintZoomFactor = 1.0;
  m_nPrintScalingMethod = 0;
  m_nPrintBestFitPagesWide = 1;
  m_nPrintBestFitPagesTall = 1;
  m_fPrintCenterOnPageH = FALSE;
  m_fPrintCenterOnPageV = FALSE;
  */
#endif // SS_V70

#if SS_V80
  SS_CT_PROCS Procs = 
  {
     CT_CreateEditorControl,
     CT_CreateRendererControl,
     CT_InitializeControl,
     CT_GetEditorValue,
     CT_SetValue,
     CT_StartEditing,
     CT_CancelEditing,
     CT_StopEditing,
     CT_IsReservedKey,
     CT_IsReservedLocation,
     CT_IsValid,
     CT_GetReservedCursor,
     CT_GetPreferredSize,
     CT_PaintCell,
     CT_StringFormat,
     CT_StringUnformat
  };
  MemHugeCpy(&CT_Procs, &Procs, sizeof(SS_CT_PROCS)); 
  m_sTypeCustomName = "";
  m_lTypeCustomStyle = 0;
  VariantInit(&m_TypeCustomItemData);
#endif // SS_V80
  UnlockBook();
}

#ifdef SS_V70
////////////////////////////////////////////////////////////////////////////
// COleControl::XTabFontNotification

STDMETHODIMP_(ULONG) CSsocxCtrl::XTabFontNotification::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) CSsocxCtrl::XTabFontNotification::Release()
{
	return 0;
}

STDMETHODIMP CSsocxCtrl::XTabFontNotification::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	ASSERT(ppvObj != NULL);

	if (IsEqualIID(iid, IID_IUnknown) ||
		IsEqualIID(iid, IID_IPropertyNotifySink))
	{
		*ppvObj = this;
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP CSsocxCtrl::XTabFontNotification::OnChanged(DISPID)
{
	METHOD_PROLOGUE_EX(CSsocxCtrl, TabFontNotification)
	pThis->OnTabFontChanged();
	pThis->BoundPropertyChanged(dispidTabStripFont);
	return S_OK;
}

STDMETHODIMP CSsocxCtrl::XTabFontNotification::OnRequestEdit(DISPID)
{
	return S_OK;
}
#endif // SS_V70

CSsocxCtrl::CSsocxCtrl() :
#ifdef SS_V70
  m_fontSS(&m_xFontNotification), m_fontTab(&m_xTabFontNotification)
#else
  m_fontSS(&m_xFontNotification)
#endif // SS_V70
{
  InitializeIIDs(&IID_DSsocx, &IID_DSsocxEvents);

  // BJO 15Sep97 - Begin fix
  // The VC 5.0 MFC classes incorrectly include custom properties
  // (BackColor, BorderStyle, Font, ForeColor, Text) in the stock
  // property mask.  The spreadsheet only has one stock property
  // (Enable).  In \VC5\MFC\SRC\CTLPROP.CPP in COleControl::
  // InitStockPropMask(), the pEntry->flags field is not checked
  // for afxDispStock.  InitStockPropMask() was called by
  // InitializeIDDs().  The STOCKPROP_ENABLE constant was taken
  // from \VC5\MFC\SRC\CTLPROP.CPP
  #if defined(VC50)
  #define STOCKPROP_ENABLED 0x00000040
  const AFX_DISPMAP* pDispMap = GetDispatchMap();
  *pDispMap->lpStockPropMask = STOCKPROP_ENABLED;
  #endif
  // BJO 15Sep97  - End fix

  m_bFirstPaint = TRUE;
  m_fontSS.InitializeFont(&_fontdescSS, AmbientFont());
#ifdef SS_V70
  m_fontTab.InitializeFont(&_fontdescSS, AmbientFont());
#endif // SS_V70
  m_sBorderStyle = 1;
  m_hBook = SS_CreatePreHwnd(AmbientForeColor(), AmbientBackColor(),
                             m_fontSS.GetFontHandle());
  LPSS_BOOK lpBook = LockBook();
  lpBook->lpOleControl = this;
#if SS_V80
  lpBook->pCTProcs = &CT_Procs;
#endif
  UnlockBook();

  m_hAccel = 0;
  m_cAccel = 0;

//DBOCX...
  m_lpVBSpread = NULL;
  SSOcxCreateVBSpread( &m_hVBSpread);
// fix for 15451 -scl
#if !defined(SS_V70) || !defined(FP_ADB)
  SSOcxInitOleBinding( this);
#endif
//DBOCX.

  InitOcx();
}


/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::~CSsocxCtrl - Destructor

CSsocxCtrl::~CSsocxCtrl()
{
  int i;

#ifdef FP_OLEDRAG
  if (m_pfpDataObject)
	  m_pfpDataObject->Release();
#endif

  SSOcxDestroyVBSpread( m_hVBSpread);               //DBOCX

  #ifndef SSP_NO_MULTISELINDEX
  if( m_hMultiSelBlocks )
    GlobalFree(m_hMultiSelBlocks);
  #endif

  if( m_lppictCursorIcon )
  {
    m_lppictCursorIcon->Release();
    m_lppictCursorIcon = NULL;
  }

  for( i = 0; i < sizeof(m_lppictCursors)/sizeof(m_lppictCursors[0]); i++ )
  {
    if( m_lppictCursors[i] )
      m_lppictCursors[i]->Release();
    m_lppictCursors[i] = NULL;
  }
  
  for( i = 0; i < sizeof(m_hcurCursors)/sizeof(m_hcurCursors[0]); i++)
  {
    if( m_hcurCursors[i] )
      DestroyCursor(m_hcurCursors[i]);
    m_hcurCursors[i] = NULL;
  }

  DestroyPicts();

  m_fontSS.ReleaseFont();
#ifdef SS_V70
  m_fontTab.ReleaseFont();
#endif // SS_V70

  if (m_hSortKeys)
     GlobalFree(m_hSortKeys);
  m_hSortKeys = 0;
  m_nSortKeyCnt = 0;

  if( m_hAccel )
     SS_DestroyAcceleratorTable(m_hAccel);

  SS_DestroyPostHwnd(m_hBook);
  m_hBook = 0;
}

void CSsocxCtrl::DestroyPicts(void)
{
  int i;

  #ifndef SSP_NO_TYPEBUTTONPICTURE
  if( m_lppictTypeButtonPicture )
    m_lppictTypeButtonPicture->Release();
  m_lppictTypeButtonPicture = NULL;
  #endif

  #ifndef SSP_NOTYPEBUTTONPICTUREDOWN
  if( m_lppictTypeButtonPictureDown )
    m_lppictTypeButtonPictureDown->Release();
  m_lppictTypeButtonPictureDown = NULL;
  #endif

  #ifndef SSP_NO_TYPECHECKPICTURE
  for( i = 0; i < 6; i++ )
  {
    if( m_lppictTypeCheckPicture[i] )
      m_lppictTypeCheckPicture[i]->Release();
    m_lppictTypeCheckPicture[i] = NULL;
  }
  #endif

  #ifndef SSP_NO_TYPEPICTPICTURE
  if( m_lppictTypePictPicture )
    m_lppictTypePictPicture->Release();
  m_lppictTypePictPicture = NULL;
  #endif
}

void CSsocxCtrl::OnFinalRelease()
{
  LPSS_BOOK lpBook = LockBook();
  if (lpBook) // RFW - 5/4/06 - 18643
  {
    lpBook->fObjectBeingDestroyed = TRUE;
    UnlockBook();
  }

  COleControl::OnFinalRelease();
}

void CSsocxCtrl::OnSetClientSite()
{
  COleControl::OnSetClientSite();
}

BOOL CSsocxCtrl::OnProperties(LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect)
{
BOOL  fRet;
#ifndef SS_V30
fRet = COleControl::OnProperties(lpMsg, hWndParent, lpRect);
#else
// This was added for a bug reported by GrapeCity in that the designer comes up at runtime.
if (!AmbientUserMode())
	fRet = OnSpreadDesigner(lpMsg, hWndParent, lpRect);
#endif
return (fRet);
}

/////////////////////////////////////////////////////////////////////////////
//
//  _AfxDrawBorder() is copied from CTLCORE.CPP file.
//

void _AfxDrawBorder(CDC* pDC, CRect& rc, int cxBorder, int cyBorder, COLORREF cr)
{
  CBrush brush(cr);
  CBrush* pBrushSave = pDC->SelectObject(&brush);
  int cxRect = rc.Width();
  int cyRect = rc.Height();
  pDC->PatBlt(rc.left, rc.top, cxRect, cyBorder, PATCOPY);
  pDC->PatBlt(rc.left, rc.top, cxBorder, cyRect, PATCOPY);
  pDC->PatBlt(rc.left, rc.bottom - cyBorder, cxRect, cyBorder, PATCOPY);
  pDC->PatBlt(rc.right - cxBorder, rc.top, cxBorder, cyRect, PATCOPY);
  pDC->SelectObject(pBrushSave);
}

//- Wei Feng add for working arround a bug in MFC
#ifdef SPREAD_JPN
#ifndef	WIN32
CParkingWnd* _gpWndParking;
#endif
#endif

void CSsocxCtrl::FireQueryAdvanceCommon(BOOL AdvanceNext, BOOL FAR* Cancel)
{
#ifdef SS_V35
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_BOOL; 
     varParam.boolVal  = (*(BOOL FAR *)Cancel ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptQueryAdvance(AdvanceNext, &varParam);

     VariantChangeType(&arg, &varParam, 0, VT_BOOL); 
     *(BOOL FAR *)Cancel = (arg.boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FireQueryAdvance(AdvanceNext, Cancel);

  UnlockBook();
#else
  FireQueryAdvance(AdvanceNext, Cancel);
#endif
}

/////////////////////////////////////////////////////////////////////////////
//
// CSsocxCtrl::GetControlFlags - Drawing function
//

DWORD CSsocxCtrl::GetControlFlags()
{
#ifdef WIN32
  DWORD dw = COleControl::GetControlFlags();

  dw &= ~(DWORD)fastBeginPaint;  // remove fastBeginPaint flag
  return dw;
#else
  return (0);
#endif
}    

/////////////////////////////////////////////////////////////////////////////
//
// CSsocxCtrl::OnDrawMetafile - Drawing function
//


void CSsocxCtrl::OnNcPaint()
{
//if (m_bInPlaceActive)
   COleControl::OnNcPaint();
}


void CSsocxCtrl::OnDrawMetafile(CDC* pdc, const CRect& rcBounds)
{
  LPSPREADSHEET lpSS = LockActiveSheet();

  HDC hDCMeta = pdc->m_hDC;
  HDC hDCAttrib = pdc->m_hAttribDC;
  HDC hDCAttrMem = NULL;
  HBITMAP hBmpDC;
  HBITMAP hBmpOld;
  int cxBorder = GetSystemMetrics(SM_CXBORDER);
  int cyBorder = GetSystemMetrics(SM_CYBORDER);
  CRect rcScroll;
  BOOL fHScrollVisible = TRUE;  // default does not affect "if" condition
  BOOL fVScrollVisible = TRUE;

  if (lpSS)
  {
    pdc->SaveDC();

    CRect rcClient;

    GetClientRect(&rcClient);

//    if ((hDCAttrib) && (hDCAttrib != hDCMeta))
//    {
      hBmpDC = CreateCompatibleBitmap(hDCAttrib, rcClient.right - rcClient.left,
                                      rcClient.bottom - rcClient.top);
      hDCAttrMem = CreateCompatibleDC(hDCAttrib);
      hBmpOld = (HBITMAP)SelectObject(hDCAttrMem, (HGDIOBJ)hBmpDC);
//    }
//    else
//      hDCAttrMem = hDCMeta;

    SS_Paint(lpSS, hDCAttrMem);

    #ifdef WIN32   // if WIN32, include these in the following if's
    fHScrollVisible = ::IsWindowVisible(lpSS->lpBook->hWndHScroll);
    fVScrollVisible = ::IsWindowVisible(lpSS->lpBook->hWndVScroll);
    #endif

    if (lpSS->lpBook->HorzScrollBar && lpSS->lpBook->hWndHScroll && fHScrollVisible)
    {
      ::GetClientRect(lpSS->lpBook->hWndHScroll, &rcScroll);
      if (rcScroll.Size().cx && rcScroll.Size().cy)
      {
        if (::SetMapMode(hDCAttrMem, MM_ANISOTROPIC))
        {
          ::SetWindowOrgEx(hDCAttrMem, 0, 0, NULL);
          ::SetWindowExtEx(hDCAttrMem, rcScroll.right - rcScroll.left,
                           rcScroll.bottom - rcScroll.top, NULL);
          ::SetViewportOrgEx(hDCAttrMem, rcClient.left - cxBorder,
                             rcClient.bottom - rcScroll.Height() + cyBorder, NULL);
          ::SetViewportExtEx(hDCAttrMem, rcScroll.right - rcScroll.left,
                             rcScroll.bottom - rcScroll.top, NULL);
        }
        ::SendMessage(lpSS->lpBook->hWndHScroll, WM_PAINT, (WPARAM)hDCAttrMem, 0);
      }
    }
    
    if (lpSS->lpBook->VertScrollBar && lpSS->lpBook->hWndVScroll && fVScrollVisible)
    {
      int cxBorderVScroll = lpSS->lpBook->fUseSpecialVScroll ? 0 : cxBorder;
      int cyBorderVScroll = lpSS->lpBook->fUseSpecialVScroll ? 0 : cyBorder;
      ::GetClientRect(lpSS->lpBook->hWndVScroll, &rcScroll);
      if (rcScroll.Size().cx && rcScroll.Size().cy)
      {
        if (::SetMapMode(hDCAttrMem, MM_ANISOTROPIC))
        {
        ::SetWindowOrgEx(hDCAttrMem, 0, 0, NULL);
        ::SetWindowExtEx(hDCAttrMem, rcScroll.right - rcScroll.left,
                         rcScroll.bottom - rcScroll.top, NULL);
        ::SetViewportOrgEx(hDCAttrMem, rcClient.right - rcScroll.Width() + cxBorderVScroll,
                           rcClient.top  - cyBorderVScroll, NULL);
        ::SetViewportExtEx(hDCAttrMem, rcScroll.right - rcScroll.left,
                           rcScroll.bottom - rcScroll.top, NULL);
        }
        ::SendMessage(lpSS->lpBook->hWndVScroll, WM_PAINT, (WPARAM)hDCAttrMem, 0);
      }
    }
       
    if ( m_sBorderStyle )
    {
      if (::SetMapMode(hDCAttrMem, MM_ANISOTROPIC))
      {
        ::SetWindowOrgEx(hDCAttrMem, 0, 0, NULL);
        ::SetViewportOrgEx(hDCAttrMem, 0, 0, NULL);
      }
    
      CRect rcBorder = rcClient;
      rcBorder.InflateRect(cxBorder, cyBorder);
      ::_AfxDrawBorder(CDC::FromHandle(hDCAttrMem), rcBorder, 
                       cxBorder, cyBorder, GetSysColor(COLOR_WINDOWFRAME));
    }

//    if ((hDCAttrib) && (hDCAttrib != hDCMeta))
//    {
      StretchBlt(hDCMeta, 
                 rcBounds.left, rcBounds.top, 
                 rcBounds.right - rcBounds.left, 
                 rcBounds.bottom - rcBounds.top,
                 hDCAttrMem, rcClient.left, 
                 rcClient.top, 
                 rcClient.right - rcClient.left, 
                 rcClient.bottom - rcClient.top,
                 SRCCOPY);

      SelectObject(hDCAttrMem, (HGDIOBJ)hBmpOld);
      DeleteObject((HGDIOBJ)hBmpDC);
      DeleteDC(hDCAttrMem);
//    }

    pdc->RestoreDC(-1);

    UnlockActiveSheet();
  }

}


/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::OnDraw - Drawing function

void CSsocxCtrl::OnDraw(
           CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
  LPSPREADSHEET lpSS = LockActiveSheet();
  BOOL          fDrawingToPrinter = FALSE;
  CRect         rcClient;

  if (!lpSS)
    return;

// fix for 9055 - added check for m_hWnd to print from IE / paint in VC 6
//                (force scrollbars to paint)   CTF

  // RFW - 12/8/03 - 12890
  //if (m_hWnd && (!pdc->IsKindOf( RUNTIME_CLASS( CPaintDC ) )))
  if (m_hWnd && (pdc->IsKindOf( RUNTIME_CLASS( CMetaFileDC ) )))
     {
     OnDrawMetafile(pdc, rcBounds);
     return;
     }

  // This was added to handle printing the Spread within IE
  if (GetDeviceCaps(pdc->m_hDC, TECHNOLOGY) == DT_RASPRINTER)
  {
     return;
     /*
     if (m_hWnd == NULL)
       CreateWindowForSubclassedControl();

     fDrawingToPrinter = TRUE;
     */
  }

// 96' 4/17 BOC Gao. Using when DragMode is Auto.
  BOOL bDragMode = FALSE;
  LPDISPATCH lpdispExtendedCtrl;

  if (lpdispExtendedCtrl = GetExtendedControl())
  {
    DISPID dispid; 
    OLECHAR FAR* szName = OLESTR("DragMode");

    //- Get the dispatch ID for 'DragMode' property.
    HRESULT hResult = lpdispExtendedCtrl->GetIDsOfNames(IID_NULL, (LPOLESTR*)&szName, 1, 0, &dispid); 
    //- Check if supported by container.
    if(SCODE(hResult) == S_OK)
    {
      COleDispatchDriver driver;
      //- Make attachment to the OLE dispatch driver.
      driver.AttachDispatch(lpdispExtendedCtrl, TRUE);
      if (driver.m_lpDispatch != NULL)
      {
        TRY
        {
          driver.GetProperty(dispid, VT_I2, &bDragMode);
        }
        END_TRY
        //- Detach from OLE dispatch driver.
        driver.DetachDispatch();
      }
    }
    lpdispExtendedCtrl->Release();
  }

// --------------------------------------------<<

  int cxBorder = GetSystemMetrics(SM_CXBORDER);
  int cyBorder = GetSystemMetrics(SM_CYBORDER);
  
  if (m_bFirstPaint && !fDrawingToPrinter)
// ----------------------------<<
  {
    if (lpSS->lpBook->fAutoSize)
      SS_AutoSize(lpSS->lpBook, TRUE);
    m_bFirstPaint = FALSE;

// SCS7356 - RFW - 12/3/98
    if (!AmbientUserMode())
       ::RedrawWindow(m_hWnd, 0, 0, RDW_FRAME | RDW_INVALIDATE);
    else
       {
       ::RedrawWindow(m_hWnd, 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
       return;
       }
  }
  
  if ((rcBounds.IsRectNull()) && (rcInvalid.IsRectNull()))
  {
    // initial paint came in too early??
    InvalidateControl();
    return;
  }

  // paint in this order to avoid
  //       overpainting controls:
  // paint scrollbars
  // paint border
  // do normal superclasspaint (designtime or runtime)

  // 96' 4/17 BOC Gao. Using when DragMode is Auto.
  //if( lpSS->lpBook->hWnd && !AmbientUserMode() )
  if( lpSS->lpBook->hWnd && (!AmbientUserMode() || bDragMode) )
  // --------------------------------------------<<
  {

    GetClientRect(&rcClient);
    pdc->SetMapMode(MM_ANISOTROPIC);
    pdc->SetWindowOrg(0, 0);
    pdc->SetWindowExt(rcClient.right, rcClient.bottom);
    pdc->SetViewportOrg(rcBounds.left, rcBounds.top);
    pdc->SetViewportExt(rcBounds.Size());
    SS_Paint(lpSS, pdc->GetSafeHdc());
  }
  else
  {
    // do SuperClassPaint for all modes
    CopyRect(&lpSS->lpBook->RectPaint, rcInvalid);
    lpSS->lpBook->fOnDraw = TRUE;
    OffsetRect(&lpSS->lpBook->RectPaint, -rcBounds.left, -rcBounds.top);
    DoSuperclassPaint(pdc, rcBounds);
  
    SetRectEmpty(&lpSS->lpBook->RectPaint);
    lpSS->lpBook->fOnDraw = FALSE;
    GetClientRect(&rcClient);
  }

//- Wei Feng add for working arround a bug in MFC
#ifdef	SPREAD_JPN
#ifndef	WIN32
  if( !AmbientUserMode() && !m_hWnd && _gpWndParking )
  {
     CRect rcPos(0, 0, rcBounds.Width(), rcBounds.Height());
     rcPos.InflateRect(cxBorder, cyBorder);
     CreateControlWindow(_gpWndParking->m_hWnd, rcPos);
     
     CRect rcClient;
     GetClientRect(&rcClient);
     pdc->SetMapMode(MM_ANISOTROPIC);
     pdc->SetWindowOrg(0, 0);
	 pdc->SetWindowExt(rcClient.right, rcClient.bottom);
	 pdc->SetViewportOrg(rcBounds.left, rcBounds.top);
	 pdc->SetViewportExt(rcBounds.Size());
	 SS_Paint(lpSS, pdc->GetSafeHdc());
  }
#endif
#endif

  // 96' 4/17 BOC Gao. Using when DragMode is Auto.
  //if( lpSS->lpBook->hWnd && !AmbientUserMode() )
  //if( lpSS->lpBook->hWnd && !m_bInPlaceActive )  // BJO 07May96 KEM3548
  if( lpSS->lpBook->hWnd && (!m_bInPlaceActive || bDragMode) )
  // --------------------------------------------<<
  {
    CRect rcScroll;
    int iSaveDC;
    BOOL fHScrollVisible = lpSS->lpBook->fHScrollVisible; // RFW - 10/31/97 - LYW349
    BOOL fVScrollVisible = lpSS->lpBook->fVScrollVisible; // RFW - 10/31/97 - LYW349
  
    iSaveDC = SaveDC(pdc->m_hDC);

    // BJO 14May97 KEM3548 - Begin removal of code
    //#ifdef WIN32   // if WIN32, include these in the following if's
    //fHScrollVisible = ::IsWindowVisible(lpSS->lpBook->hWndHScroll);
    //fVScrollVisible = ::IsWindowVisible(lpSS->lpBook->hWndVScroll);
    //#endif
    // BJO 14May97 KEM3548 - End removal of code

    if (lpSS->lpBook->HorzScrollBar && lpSS->lpBook->hWndHScroll && fHScrollVisible)
    {
      ::GetClientRect(lpSS->lpBook->hWndHScroll, &rcScroll);
      if (rcScroll.Width() && rcScroll.Height())
      {
        //rcScroll.InflateRect(0, 0, -(cxBorder + 1), 0);
        if (pdc->SetMapMode(MM_ANISOTROPIC))
        {
			 CRect rcScrollScaled = rcScroll;

          if (fDrawingToPrinter)
          {
				rcScrollScaled.right = (int)((double)rcScroll.right * ((double)rcBounds.Width() / (double)rcClient.Width()));
				rcScrollScaled.bottom = (int)((double)rcScroll.bottom * ((double)rcBounds.Height() / (double)rcClient.Height()));
			 }

          pdc->SetWindowOrg(0, 0);
          pdc->SetWindowExt(rcScroll.Size());
          pdc->SetViewportOrg(rcBounds.left - cxBorder,
                              rcBounds.bottom - rcScrollScaled.Height() + cyBorder);
          pdc->SetViewportExt(rcScrollScaled.Size());
        }

        ::SendMessage(lpSS->lpBook->hWndHScroll, WM_PAINT, (WPARAM)pdc->GetSafeHdc(), 0);
      }
    }
    
    if (lpSS->lpBook->VertScrollBar && lpSS->lpBook->hWndVScroll && fVScrollVisible)
    {
      int cxBorderVScroll = lpSS->lpBook->fUseSpecialVScroll ? 0 : cxBorder;
      int cyBorderVScroll = lpSS->lpBook->fUseSpecialVScroll ? 0 : cyBorder;
      ::GetClientRect(lpSS->lpBook->hWndVScroll, &rcScroll);

		if (rcScroll.Width() && rcScroll.Height())
      {
        // rcScroll.InflateRect(0, 0, 0, -(cyBorderVScroll + 1));
        if (pdc->SetMapMode(MM_ANISOTROPIC))
        {
			 CRect rcScrollScaled = rcScroll;

          if (fDrawingToPrinter)
          {
				rcScrollScaled.right = (int)((double)rcScroll.right * ((double)rcBounds.Width() / (double)rcClient.Width()));
				rcScrollScaled.bottom = (int)((double)rcScroll.bottom * ((double)rcBounds.Height() / (double)rcClient.Height()));
			 }

          pdc->SetWindowOrg(0, 0);
          pdc->SetWindowExt(rcScroll.Size());
          pdc->SetViewportOrg(rcBounds.right - rcScrollScaled.Width() + cxBorderVScroll,
                              rcBounds.top  - cyBorderVScroll);
          pdc->SetViewportExt(rcScrollScaled.Size());
        }

        ::SendMessage(lpSS->lpBook->hWndVScroll, WM_PAINT, (WPARAM)pdc->GetSafeHdc(), 0);
        
        // 96' 4/17 BOC Gao. for border is out of drawing at design mode.
        /*
        if( lpSS->fUseSpecialVScroll )
        {
          rcScroll.InflateRect(cxBorder, cyBorder);
          ::_AfxDrawBorder(pdc, rcScroll, cxBorder, cyBorder,
                           GetSysColor(COLOR_WINDOWFRAME));
        }
        */
        // --------------------------------------------------<<
      }
    }

    RestoreDC(pdc->m_hDC, iSaveDC);    
    iSaveDC = SaveDC(pdc->m_hDC);

    // now paint border at designtime
    if( m_sBorderStyle )
    {
      //CRect rcBorder(0, 0, rcBounds.Width(), rcBounds.Height());
      if (pdc->SetMapMode(MM_ANISOTROPIC))
      {
        pdc->SetWindowOrg(0, 0);
        pdc->SetViewportOrg(0, 0);
        pdc->SetWindowExt(rcBounds.Size());
        pdc->SetViewportExt(rcBounds.Size());
      }
      CRect rcBorder = rcBounds;
      rcBorder.InflateRect(cxBorder, cyBorder);
      ::_AfxDrawBorder(pdc, rcBorder, cxBorder, cyBorder,
                       GetSysColor(COLOR_WINDOWFRAME));
    }

    RestoreDC(pdc->m_hDC, iSaveDC);
  }

  // RFW - 10/20/98 - GRB6059 (VB4 Only bug)
  else
  {
    if (lpSS->lpBook->HorzScrollBar && lpSS->lpBook->hWndHScroll && lpSS->lpBook->fHScrollVisible)
    {
      if (rcInvalid.bottom > rcBounds.bottom - lpSS->lpBook->dHScrollHeight + 1)
         ::InvalidateRect(lpSS->lpBook->hWndHScroll, NULL, TRUE);
    }

    if (lpSS->lpBook->VertScrollBar && lpSS->lpBook->hWndVScroll && lpSS->lpBook->fVScrollVisible)
    {
      if (rcInvalid.right > rcBounds.right - lpSS->lpBook->dVScrollWidth + 1)
         ::InvalidateRect(lpSS->lpBook->hWndVScroll, NULL, TRUE);
    }

  }

  UnlockActiveSheet();

}


/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::DoPropExchange - Persistence support

void CSsocxCtrl::DoPropExchange(CPropExchange* pPX)
{

#if 0
  BOOL bAllowCellOverflow, def_bAllowCellOverflow = FALSE;
  BOOL bAllowDragDrop, def_bAllowDragDrop = FALSE;
  BOOL bAllowMultiBlocks, def_bAllowMultiBlocks = FALSE;
  BOOL bAllowUserFormulas, def_bAllowUserFormulas = FALSE;
  BOOL bArrowsExitEditMode, def_bArrowsExitEditMode = FALSE;
  BOOL bAutoCalc, def_bAutoCalc = TRUE;
  BOOL bAutoClipboard, def_bAutoClipboard = TRUE;
  BOOL bAutoSize, def_bAutoSize = FALSE;
  short nBackColorStyle, def_nBackColorStyle = 0;
  short nBorderStyle, def_nBorderStyle = 1;
  short nButtonDrawMode, def_nButtonDrawMode = 0;
  short nColHeaderDisplay, def_nColHeaderDisplay = 2;
  long lColsFrozen, def_lColsFrozen = 0;
  BOOL bDAutoCellTypes, def_bDAutoCellTypes = TRUE;
  BOOL bDAutoFill, def_bDAutoFill = TRUE;
  BOOL bDAutoHeadings, def_bDAutoHeadings = TRUE;
  BOOL bDAutoSave, def_bDAutoSave = TRUE;
  short nDAutoSizeCols, def_nDAutoSizeCols = 2;
  BOOL bDInformActiveRowChange, def_bDInformActiveRowChange = TRUE;
  BOOL bDisplayColHeaders, def_bDisplayColHeaders = TRUE;
  BOOL bDisplayRowHeaders, def_bDisplayRowHeaders = TRUE;
  short nEditEnterAction, def_nEditEnterAction = 0;
  BOOL bEditModePermanent, def_bEditModePermanent = FALSE;
  BOOL bEditModeReplace, def_bEditModeReplace = FALSE;
  BOOL bFormulaSync, def_bFormulaSync = TRUE;
  COLORREF crGrayAreaBackColor, def_crGrayAreaBackColor = RGBCOLOR_PALEGRAY;
  COLORREF crGridColor, def_crGridColor = RGBCOLOR_PALEGRAY;
  BOOL bGridShowHoriz, def_bGridShowHoriz = TRUE;
  BOOL bGridShowVert, def_bGridShowVert = TRUE;
  BOOL bGridSolid, def_bGridSolid = TRUE;
  long lMaxCols, def_lMaxCols = 500;
  long lMaxRows, def_lMaxRows = 500;
  BOOL bMoveActiveOnFocus, def_bMoveActiveOnFocus = TRUE;
  BOOL bNoBeep, def_bNoBeep = FALSE;
  BOOL bNoBorder, def_bNoBorder = FALSE;
  short nOperationMode, def_nOperationMode = 0;
  BOOL bProcessTab, def_bProcessTab = FALSE;
  BOOL bProtect, def_bProtect = TRUE;
  BOOL bReDraw, def_bReDraw = TRUE;
  BOOL bRestrictCols, def_bRestrictCols = FALSE;
  BOOL bRestrictRows, def_bRestrictRows = FALSE;
  BOOL bRetainSelBlock, def_bRetainSelBlock = TRUE;
  short nRowHeaderDisplay, def_nRowHeaderDisplay = 1;
  long lRowsFrozen, def_lRowsFrozen = 0;
  BOOL bScrollBarExtMode, def_bScrollBarExtMode = FALSE;
  BOOL bScrollBarMaxAlign, def_bScrollBarMaxAlign = TRUE;
  short nScrollBars, def_nScrollBars = 3;
  BOOL bScrollBarShowMax, def_bScrollBarShowMax = TRUE;
  short nSelectBlockOptions, def_nSelectBlockOptions = 15;
  COLORREF crShadowColor, def_crShadowColor = SS_EncodeSysColor(COLOR_BTNFACE);
  COLORREF crShadowDark, def_crShadowDark = SS_EncodeSysColor(COLOR_BTNSHADOW);
  COLORREF crShadowText, def_crShadowText = SS_EncodeSysColor(COLOR_BTNTEXT);
  long lStartingColNumber, def_lStartingColNumber = 1;
  long lStartingRowNumber, def_lStartingRowNumber = 1;
  short nUnitType, def_nUnitType = 1;
  short nUserResize, def_nUserResize = 3;
  long lVirtualMaxRows, def_lVirtualMaxRows = -1;
  BOOL bVirtualMode, def_bVirtualMode = FALSE;
  long lVirtualOverlap, def_lVirtualOverlap = 0;
  long lVirtualRows, def_lVirtualRows = 0;
  BOOL bVirtualScrollBuffer, def_bVirtualScrollBuffer = FALSE;
  long lVisibleCols, def_lVisibleCols = 0;
  long lVisibleRows, def_lVisibleRows = 0;
  BOOL bVScrollSpecial, def_bVScrollSpecial = FALSE;
  short nVScrollSpecialType, def_nVScrollSpecialType = 0;
#ifdef SS_V30
  short nAppearance, def_nAppearance = 0;
  short nTextTip, def_nTextTip = 0;
  long lTextTipDelay, def_lTextTipDelay = 500;
  short nScrollBarTrack, def_nScrollBarTrack = 0;
  short nClipboardOptions, def_nClipboardOptions = 15;  // copy/paste all headers
#endif
#endif // 0

  BOOL bAllowCellOverflow;
  #define def_bAllowCellOverflow FALSE
  BOOL bAllowDragDrop;
  #define def_bAllowDragDrop FALSE
  BOOL bAllowMultiBlocks;
  #define def_bAllowMultiBlocks FALSE
  BOOL bAllowUserFormulas;
  #define def_bAllowUserFormulas FALSE
  BOOL bArrowsExitEditMode;
  #define def_bArrowsExitEditMode FALSE
  BOOL bAutoCalc;
  #define def_bAutoCalc TRUE
  BOOL bAutoClipboard;
  #define def_bAutoClipboard TRUE
  BOOL bAutoSize;
  #define def_bAutoSize FALSE
  short nBackColorStyle;
  #define def_nBackColorStyle 0
  short nBorderStyle;
  #define def_nBorderStyle 1
  short nButtonDrawMode;
  #define def_nButtonDrawMode 0
  short nColHeaderDisplay;
  #define def_nColHeaderDisplay 2
  long lColsFrozen;
  #define def_lColsFrozen 0
  BOOL bDAutoCellTypes;
  #define def_bDAutoCellTypes TRUE
  BOOL bDAutoFill;
  #define def_bDAutoFill TRUE
  BOOL bDAutoHeadings;
  #define def_bDAutoHeadings TRUE
  BOOL bDAutoSave;
  #define def_bDAutoSave TRUE
  short nDAutoSizeCols;
  #define def_nDAutoSizeCols 2
  BOOL bDInformActiveRowChange;
  #define def_bDInformActiveRowChange TRUE
  BOOL bDisplayColHeaders;
  #define def_bDisplayColHeaders TRUE
  BOOL bDisplayRowHeaders;
  #define def_bDisplayRowHeaders TRUE
  short nEditEnterAction;
  #define def_nEditEnterAction 0
  BOOL bEditModePermanent;
  #define def_bEditModePermanent FALSE
  BOOL bEditModeReplace;
  #define def_bEditModeReplace FALSE
  BOOL bFormulaSync;
  #define def_bFormulaSync TRUE
  COLORREF crGrayAreaBackColor;
  #define def_crGrayAreaBackColor RGBCOLOR_PALEGRAY
  COLORREF crGridColor;
  #define def_crGridColor RGBCOLOR_PALEGRAY
  BOOL bGridShowHoriz;
  #define def_bGridShowHoriz TRUE
  BOOL bGridShowVert;
  #define def_bGridShowVert TRUE
  BOOL bGridSolid;
  #define def_bGridSolid TRUE
  long lMaxCols;
  #define def_lMaxCols 500
  long lMaxRows;
  #define def_lMaxRows 500
  BOOL bMoveActiveOnFocus;
  #define def_bMoveActiveOnFocus TRUE
  BOOL bNoBeep;
  #define def_bNoBeep FALSE
  BOOL bNoBorder;
  #define def_bNoBorder FALSE
  short nOperationMode;
  #define def_nOperationMode 0
  BOOL bProcessTab;
  #define def_bProcessTab FALSE
  BOOL bProtect;
  #define def_bProtect TRUE
  BOOL bReDraw;
  #define def_bReDraw TRUE
  BOOL bRestrictCols;
  #define def_bRestrictCols FALSE
  BOOL bRestrictRows;
  #define def_bRestrictRows FALSE
  BOOL bRetainSelBlock;
  #define def_bRetainSelBlock TRUE
  short nRowHeaderDisplay;
  #define def_nRowHeaderDisplay 1
  long lRowsFrozen;
  #define def_lRowsFrozen 0
  BOOL bScrollBarExtMode;
  #define def_bScrollBarExtMode FALSE
  BOOL bScrollBarMaxAlign;
  #define def_bScrollBarMaxAlign TRUE
  short nScrollBars;
  #define def_nScrollBars 3
  BOOL bScrollBarShowMax;
  #define def_bScrollBarShowMax TRUE
  short nSelectBlockOptions;
  #define def_nSelectBlockOptions 15
  COLORREF crShadowColor;
  #define def_crShadowColor SS_EncodeSysColor(COLOR_BTNFACE)
  COLORREF crShadowDark;
  #define def_crShadowDark SS_EncodeSysColor(COLOR_BTNSHADOW)
  COLORREF crShadowText;
  #define def_crShadowText SS_EncodeSysColor(COLOR_BTNTEXT)
  long lStartingColNumber;
  #define def_lStartingColNumber 1
  long lStartingRowNumber;
  #define def_lStartingRowNumber 1
  short nUnitType;
  #define def_nUnitType 1
  short nUserResize;
  #define def_nUserResize 3
  long lVirtualMaxRows;
  #define def_lVirtualMaxRows -1
  BOOL bVirtualMode;
  #define def_bVirtualMode FALSE
  long lVirtualOverlap;
  #define def_lVirtualOverlap 0
  long lVirtualRows;
  #define def_lVirtualRows 0
  BOOL bVirtualScrollBuffer;
  #define def_bVirtualScrollBuffer FALSE
  long lVisibleCols;
  #define def_lVisibleCols 0
  long lVisibleRows;
  #define def_lVisibleRows 0
  BOOL bVScrollSpecial;
  #define def_bVScrollSpecial FALSE
  short nVScrollSpecialType;
  #define def_nVScrollSpecialType 0
#ifdef SS_V30
  short nAppearance = 0;
  #define def_nAppearance 0
  short nTextTip = 0;
  #define def_nTextTip 0
  long lTextTipDelay = 500;
  #define def_lTextTipDelay 500
  short nScrollBarTrack = 0;
  #define def_nScrollBarTrack 0
  short nClipboardOptions = 15;
  #define def_nClipboardOptions 15  // copy/paste all headers
#endif
#ifdef SS_V35
  short nCellNoteIndicator = 0;
  #define def_nCellNoteIndicator 0
  short nShowScrollTips = 0;
  #define def_nShowScrollTips 0
#endif
#ifdef SS_V80
#define def_bEnhanceStaticCells FALSE
  BOOL bEnhanceStaticCells = FALSE;
  #define def_nCellNoteShape 0
  short nCellNoteShape = 0;
  #define def_nAppearanceStyle 2
  short nAppearanceStyle = 2;
  #define def_nScrollBarStyle 0
  short nScrollBarStyle = 0;
#define def_nUseVisualStyles 0
  short nUseVisualStyles = 0;
  #define def_nTabEnhancedShape 2
  short nTabEnhancedShape = 2;
  COLORREF crCellNoteIndicatorColor;
  #define def_crCellNoteIndicatorColor RGBCOLOR_RED
  #define def_nHighlightHeaders 0
  short nHighlightHeaders = 0;
  #define def_nHighlightStyle 0
  short nHighlightStyle = 0;
  #define def_clrHighlightAlphaBlendColor SS_EncodeSysColor(COLOR_HIGHLIGHT)
  COLORREF clrHighlightAlphaBlendColor;
  #define def_nHighlightAlphaBlend SS_HIGHLIGHT_ALPHABLEND
  short nHighlightAlphaBlend;
#endif // SS_V80
  long lDataLen = 0;
  GLOBALHANDLE hData = 0;
  GLOBALHANDLE hBlob = 0;
  HPBYTE hpBlob;
  HPBYTE hpData;
  LPSPREADSHEET lpSS = LockSheet(0);

  if( !pPX->IsLoading() )
  {
// 25104 (moved down) -scl
//#ifdef SS_V80
//   if (HIWORD(m_dwVersionLoaded) < 8)
// 	  SS_SetAppearanceStyle(lpSS->lpBook, 0);
//#endif
    hData = SS_SaveToBuffer(lpSS->lpBook, &lDataLen, FALSE, NULL);
    hBlob = GlobalAlloc(GMEM_MOVEABLE, lDataLen + sizeof(long));
    hpBlob = (HPBYTE)GlobalLock(hBlob);
    hpData = (HPBYTE)GlobalLock(hData);
    *(LPLONG)hpBlob = lDataLen;
    MemHugeCpy(hpBlob + sizeof(long), hpData, lDataLen);

    GlobalUnlock(hData);
    GlobalUnlock(hBlob);
    GlobalFree(hData);

    bAllowCellOverflow = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWCELLOVERFLOW);
    bAllowDragDrop = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWDRAGDROP);
    bAllowMultiBlocks = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWMULTIBLOCKS);
    bAllowUserFormulas = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWUSERFORMULAS);
    bArrowsExitEditMode = SS_GetBool(lpSS->lpBook, lpSS, SSB_ARROWSEXITEDITMODE);
    bAutoCalc = SS_GetBool(lpSS->lpBook, lpSS, SSB_AUTOCALC);
    bAutoClipboard = SS_GetBool(lpSS->lpBook, lpSS, SSB_AUTOCLIPBOARD);
    bAutoSize = SS_GetBool(lpSS->lpBook, lpSS, SSB_AUTOSIZE);
    nBackColorStyle = SS_GetBackColorStyle(lpSS->lpBook);
    nBorderStyle = m_sBorderStyle;
    nButtonDrawMode = SS_GetButtonDrawMode(lpSS->lpBook);
    nColHeaderDisplay = SS_GetColHeaderDisplay(lpSS);
    SS_GetFreeze(lpSS, &lColsFrozen, &lRowsFrozen);
    #ifndef SSP_NO_DAUTOCELLTYPES
    bDAutoCellTypes = GetDAutoCellTypes();
    #endif
    #ifndef SSP_NO_DAUTOFILL
    bDAutoFill = GetDAutoFill();
    #endif
    #ifndef SSP_NO_DAUTOHEADINGS
    bDAutoHeadings = GetDAutoHeadings();
    #endif
    bDAutoSave = GetDAutoSave();
    nDAutoSizeCols = GetDAutoSizeCols();
    bDInformActiveRowChange = GetDInformActiveRowChange();
    bDisplayColHeaders = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWCOLHEADERS);
    bDisplayRowHeaders = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWROWHEADERS);
    nEditEnterAction = SS_GetEditEnterAction(lpSS->lpBook);
    bEditModePermanent = SS_GetBool(lpSS->lpBook, lpSS, SSB_EDITMODEPERMANENT);
    bEditModeReplace = SS_GetBool(lpSS->lpBook, lpSS, SSB_EDITMODEREPLACE);
    bFormulaSync = SS_GetBool(lpSS->lpBook, lpSS, SSB_FORMULASYNC);
    SS_GetGrayAreaColor(lpSS->lpBook, &crGrayAreaBackColor, NULL);
    crGridColor = SS_GetGridColor(lpSS);
    bGridShowHoriz = SS_GetGridType(lpSS) & SS_GRID_HORIZONTAL ? TRUE : FALSE;
    bGridShowVert = SS_GetGridType(lpSS) & SS_GRID_VERTICAL ? TRUE : FALSE;
    bGridSolid = SS_GetGridType(lpSS) & SS_GRID_SOLID ? TRUE : FALSE;
    lMaxCols = GetMaxCols();
    lMaxRows = GetMaxRows();
    bMoveActiveOnFocus = SS_GetBool(lpSS->lpBook, lpSS, SSB_MOVEACTIVEONFOCUS);
    bNoBeep = SS_GetBool(lpSS->lpBook, lpSS, SSB_NOBEEP);
    bNoBorder = SS_GetBool(lpSS->lpBook, lpSS, SSB_NOBORDER);
    nOperationMode = SS_GetOperationMode(lpSS);
#ifdef SS_V80
	nAppearanceStyle = SS_GetAppearanceStyle(lpSS->lpBook);
	nUseVisualStyles = SS_GetUseVisualStyles(lpSS->lpBook);
	bEnhanceStaticCells = SS_GetEnhanceStaticCells(lpSS->lpBook);
	nTabEnhancedShape = SS_GetTabEnhancedShape(lpSS->lpBook);
	crCellNoteIndicatorColor = SS_GetCellNoteIndicatorColor(lpSS->lpBook);
	nCellNoteShape = SS_GetCellNoteIndicatorShape(lpSS->lpBook);
	nHighlightHeaders = SS_GetHighlightHeaders(lpSS->lpBook);
	nHighlightStyle = SS_GetHighlightStyle(lpSS->lpBook);
	nScrollBarStyle = SS_GetScrollBarStyle(lpSS->lpBook);
	SS_GetHighlightAlphaBlend(lpSS->lpBook, &clrHighlightAlphaBlendColor, &nHighlightAlphaBlend);
#endif
    bProcessTab = SS_GetBool(lpSS->lpBook, lpSS, SSB_PROCESSTAB);
    bProtect = SS_GetBool(lpSS->lpBook, lpSS, SSB_PROTECT);
    bReDraw = SS_GetBool(lpSS->lpBook, lpSS, SSB_REDRAW);
    bRestrictCols = SS_GetBool(lpSS->lpBook, lpSS, SSB_RESTRICTCOLS);
    bRestrictRows = SS_GetBool(lpSS->lpBook, lpSS, SSB_RESTRICTROWS);
    bRetainSelBlock = SS_GetBool(lpSS->lpBook, lpSS, SSB_RETAINSELBLOCK);
    nRowHeaderDisplay = SS_GetRowHeaderDisplay(lpSS);
    bScrollBarExtMode = SS_GetBool(lpSS->lpBook, lpSS, SSB_SCROLLBAREXTMODE);
    bScrollBarMaxAlign = SS_GetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARMAXALIGN);
    nScrollBars = (SS_GetBool(lpSS->lpBook, lpSS, SSB_HORZSCROLLBAR) ? 1 : 0) |
                  (SS_GetBool(lpSS->lpBook, lpSS, SSB_VERTSCROLLBAR) ? 2 : 0);
    bScrollBarShowMax = SS_GetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARSHOWMAX);
    nSelectBlockOptions = SS_GetSelBlockOptions(lpSS->lpBook);
    SS_GetShadowColor(lpSS, &crShadowColor, &crShadowText, &crShadowDark, NULL);
    SS_GetStartingNumbers(lpSS, &lStartingColNumber, &lStartingRowNumber);
    nUnitType = SS_GetUnitType(lpSS->lpBook);;
    nUserResize = SS_GetUserResize(lpSS);
    lVirtualMaxRows = SS_VGetMax(lpSS);
    bVirtualMode = SS_GetBool(lpSS->lpBook, lpSS, SSB_VIRTUALMODE);;
    SS_VGetBufferSize(lpSS, &lVirtualRows, &lVirtualOverlap);
    bVirtualScrollBuffer = (SS_VGetStyle(lpSS) & SSV_SCROLLBARSHOWBUFFER) ?
                           TRUE : FALSE;
    SS_GetAutoSizeVisible(lpSS, &lVisibleCols, &lVisibleRows);
    #ifndef SS_NOSCBAR
    bVScrollSpecial = SS_VScrollGetSpecial(lpSS->lpBook, (LPWORD)&nVScrollSpecialType);
    #endif
#ifdef SS_V30
    nAppearance = (short)SS_GetAppearance(lpSS->lpBook);
    nTextTip = GetTextTip();
    lTextTipDelay = GetTextTipDelay();
    nScrollBarTrack = GetScrollBarTrack();
    nClipboardOptions = (short)lpSS->lpBook->bClipboardOptions;
#endif
#ifdef SS_V35
    nCellNoteIndicator = (short)SS_GetCellNoteIndicator(lpSS->lpBook);
    nShowScrollTips = (short)SS_GetShowScrollTips(lpSS->lpBook);
#endif
  }

  ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
  COleControl::DoPropExchange(pPX);

  PX_Bool(pPX, _T("AllowCellOverflow"), bAllowCellOverflow, def_bAllowCellOverflow);
  PX_Bool(pPX, _T("AllowDragDrop"), bAllowDragDrop, def_bAllowDragDrop);
  PX_Bool(pPX, _T("AllowMultiBlocks"), bAllowMultiBlocks, def_bAllowMultiBlocks);
  PX_Bool(pPX, _T("AllowUserFormulas"), bAllowUserFormulas, def_bAllowUserFormulas);
  PX_Bool(pPX, _T("ArrowsExitEditMode"), bArrowsExitEditMode, def_bArrowsExitEditMode);
  PX_Bool(pPX, _T("AutoCalc"), bAutoCalc, def_bAutoCalc);
  PX_Bool(pPX, _T("AutoClipboard"), bAutoClipboard, def_bAutoClipboard);
  PX_Bool(pPX, _T("AutoSize"), bAutoSize, def_bAutoSize);
  PX_Short(pPX, _T("BackColorStyle"), nBackColorStyle, def_nBackColorStyle);
  PX_Short(pPX, _T("BorderStyle"), nBorderStyle, def_nBorderStyle);
  PX_Short(pPX, _T("ButtonDrawMode"), nButtonDrawMode, def_nButtonDrawMode);
  PX_Short(pPX, _T("ColHeaderDisplay"), nColHeaderDisplay, def_nColHeaderDisplay);
  PX_Long(pPX, _T("ColsFrozen"), lColsFrozen, def_lColsFrozen);
  PX_Bool(pPX, _T("DAutoCellTypes"), bDAutoCellTypes, def_bDAutoCellTypes);
  PX_Bool(pPX, _T("DAutoFill"), bDAutoFill, def_bDAutoFill);
  PX_Bool(pPX, _T("DAutoHeadings"), bDAutoHeadings, def_bDAutoHeadings);
  PX_Bool(pPX, _T("DAutoSave"), bDAutoSave, def_bDAutoSave);
  PX_Short(pPX, _T("DAutoSizeCols"), nDAutoSizeCols, def_nDAutoSizeCols);
  PX_Bool(pPX, _T("DInformActiveRowChange"), bDInformActiveRowChange, def_bDInformActiveRowChange);
  PX_Bool(pPX, _T("DisplayColHeaders"), bDisplayColHeaders, def_bDisplayColHeaders);
  PX_Bool(pPX, _T("DisplayRowHeaders"), bDisplayRowHeaders, def_bDisplayRowHeaders);
  PX_Short(pPX, _T("EditEnterAction"), nEditEnterAction, def_nEditEnterAction);
  PX_Bool(pPX, _T("EditModePermanent"), bEditModePermanent, def_bEditModePermanent);
  PX_Bool(pPX, _T("EditModeReplace"), bEditModeReplace, def_bEditModeReplace);
  if( IsConvertingVBX() )
    PX_VBXFontConvert(pPX, m_fontSS);
  else
    PX_Font(pPX, _T("Font"), m_fontSS, &_fontdescSS);
  PX_Bool(pPX, _T("FormulaSync"), bFormulaSync, def_bFormulaSync);
  PX_Color(pPX, _T("GrayAreaBackColor"), crGrayAreaBackColor, def_crGrayAreaBackColor);
  PX_Color(pPX, _T("GridColor"), crGridColor, def_crGridColor);
  PX_Bool(pPX, _T("GridShowHoriz"), bGridShowHoriz, def_bGridShowHoriz);
  PX_Bool(pPX, _T("GridShowVert"), bGridShowVert, def_bGridShowVert);
  PX_Bool(pPX, _T("GridSolid"), bGridSolid, def_bGridSolid);
  PX_Long(pPX, _T("MaxCols"), lMaxCols, def_lMaxCols);
  PX_Long(pPX, _T("MaxRows"), lMaxRows, def_lMaxRows);
  PX_Bool(pPX, _T("MoveActiveOnFocus"), bMoveActiveOnFocus, def_bMoveActiveOnFocus);
  PX_Bool(pPX, _T("NoBeep"), bNoBeep, def_bNoBeep);
  PX_Bool(pPX, _T("NoBorder"), bNoBorder, def_bNoBorder);
  PX_Short(pPX, _T("OperationMode"), nOperationMode, def_nOperationMode);
  PX_Short(pPX, _T("Position"), m_nPosition, 0);
  PX_Bool(pPX, _T("ProcessTab"), bProcessTab, def_bProcessTab);
  PX_Bool(pPX, _T("Protect"), bProtect, def_bProtect);
  PX_Bool(pPX, _T("ReDraw"), bReDraw, def_bReDraw);
  PX_Bool(pPX, _T("RestrictCols"), bRestrictCols, def_bRestrictCols);
  PX_Bool(pPX, _T("RestrictRows"), bRestrictRows, def_bRestrictRows);
  PX_Bool(pPX, _T("RetainSelBlock"), bRetainSelBlock, def_bRetainSelBlock);
  PX_Short(pPX, _T("RowHeaderDisplay"), nRowHeaderDisplay, def_nRowHeaderDisplay);
  PX_Long(pPX, _T("RowsFrozen"), lRowsFrozen, def_lRowsFrozen);
  PX_Bool(pPX, _T("ScrollBarExtMode"), bScrollBarExtMode, def_bScrollBarExtMode);
  PX_Bool(pPX, _T("ScrollBarMaxAlign"), bScrollBarMaxAlign, def_bScrollBarMaxAlign);
  PX_Short(pPX, _T("ScrollBars"), nScrollBars, def_nScrollBars);
  PX_Bool(pPX, _T("ScrollBarShowMax"), bScrollBarShowMax, def_bScrollBarShowMax);
  PX_Short(pPX, _T("SelectBlockOptions"), nSelectBlockOptions, def_nSelectBlockOptions);
  PX_Color(pPX, _T("ShadowColor"), crShadowColor, def_crShadowColor);
  PX_Color(pPX, _T("ShadowDark"), crShadowDark, def_crShadowDark);
  PX_Color(pPX, _T("ShadowText"), crShadowText, def_crShadowText);

  if (!PX_Blob(pPX, _T("SpreadDesigner"), hBlob) && !pPX->IsLoading())
	  AfxThrowOleException(E_FAIL);

  PX_Long(pPX, _T("StartingColNumber"), lStartingColNumber, def_lStartingColNumber);
  PX_Long(pPX, _T("StartingRowNumber"), lStartingRowNumber, def_lStartingRowNumber);
  PX_Short(pPX, _T("UnitType"), nUnitType, def_nUnitType);
  PX_Short(pPX, _T("UserResize"), nUserResize, def_nUserResize);
  PX_Long(pPX, _T("VirtualMaxRows"), lVirtualMaxRows, def_lVirtualMaxRows);
  PX_Bool(pPX, _T("VirtualMode"), bVirtualMode, def_bVirtualMode);
  PX_Long(pPX, _T("VirtualOverlap"), lVirtualOverlap, def_lVirtualOverlap);
  PX_Long(pPX, _T("VirtualRows"), lVirtualRows, def_lVirtualRows);
  PX_Bool(pPX, _T("VirtualScrollBuffer"), bVirtualScrollBuffer, def_bVirtualScrollBuffer);
  PX_Long(pPX, _T("VisibleCols"), lVisibleCols, def_lVisibleCols);
  PX_Long(pPX, _T("VisibleRows"), lVisibleRows, def_lVisibleRows);
  PX_Bool(pPX, _T("VScrollSpecial"), bVScrollSpecial, def_bVScrollSpecial);
  PX_Short(pPX, _T("VScrollSpecialType"), nVScrollSpecialType, def_nVScrollSpecialType);
#ifdef SS_V30
//  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(_wVerMinor, _wVerMajor) )
  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(0, 3) )
    {
    PX_Short(pPX, _T("Appearance"), nAppearance, def_nAppearance);
    PX_Short(pPX, _T("TextTip"), nTextTip, def_nTextTip);
    PX_Long(pPX, _T("TextTipDelay"), lTextTipDelay, def_lTextTipDelay);
    PX_Short(pPX, _T("ScrollBarTrack"), nScrollBarTrack, def_nScrollBarTrack);
    PX_Short(pPX, _T("ClipboardOptions"), nClipboardOptions, def_nClipboardOptions);
    }
#endif
#ifdef SS_V35
//  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(_wVerMinor, _wVerMajor) )
  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(5, 3) )
    {
    PX_Short(pPX, _T("CellNoteIndicator"), nCellNoteIndicator, def_nCellNoteIndicator);
    PX_Short(pPX, _T("ShowScrollTips"), nShowScrollTips, def_nShowScrollTips);
    }
#endif
//  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(_wVerMinor, _wVerMajor) )
//  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(_wVerMinor, _wVerMajor) )
//  {
#ifdef FP_ADB
    // If saving, copy current value to temp CString
    CString cstrDataMember;
#ifdef SS_V70
    if ( !pPX->IsLoading() && lpSS->pfpOleBinding )
    {
      BSTR bstrDataMember = NULL;
      ((CfpOleBinding*)lpSS->pfpOleBinding)->GetDataMember(&bstrDataMember);
      cstrDataMember = (LPCWSTR)bstrDataMember;
      if (bstrDataMember)
        SysFreeString(bstrDataMember);
    }

    if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(0, 3) ) // 26787 -scl
      // Load/Save string value for DataMember property
      PX_String(pPX, _T("DataMember"), cstrDataMember, _T(""));

    // If loading, set value in m_fpOleBinding object.
    if (pPX->IsLoading())
    {
      BSTR bstrDataMember = cstrDataMember.AllocSysString();
      if( !lpSS->pfpOleBinding )
        SSOcxInitSheetOleBinding(this, lpSS);
      ((CfpOleBinding*)lpSS->pfpOleBinding)->SetDataMember(bstrDataMember);
      if (bstrDataMember)
        SysFreeString(bstrDataMember);
    }
#else
    if ( !pPX->IsLoading() )
    {
      BSTR bstrDataMember = NULL;
      m_fpOleBinding.GetDataMember(&bstrDataMember);
      cstrDataMember = (LPCWSTR)bstrDataMember;
      if (bstrDataMember)
        SysFreeString(bstrDataMember);
    }

    // Load/Save string value for DataMember property
    PX_String(pPX, _T("DataMember"), cstrDataMember, _T(""));

    // If loading, set value in m_fpOleBinding object.
    if (pPX->IsLoading())
    {
      BSTR bstrDataMember = cstrDataMember.AllocSysString();
      m_fpOleBinding.SetDataMember(bstrDataMember);
      if (bstrDataMember)
        SysFreeString(bstrDataMember);
    }
#endif // !SS_V70
#else
    CString cstrDataMember("");
    PX_String(pPX, _T("DataMember"), cstrDataMember, _T(""));
#endif  // if FP_ADB
//  }
#ifdef FP_OLEDRAG
  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(0, 3) )
     PX_Short(pPX, _T("OLEDropMode"), m_OLEDropMode, 0);
#endif

#ifdef SS_V80
  if( !pPX->IsLoading() || m_dwVersionLoaded >= (DWORD)MAKELONG(0, 8) )
  {
  PX_Short(pPX, _T("AppearanceStyle"), nAppearanceStyle, def_nAppearanceStyle);
  PX_Short(pPX, _T("TabEnhancedShape"), nTabEnhancedShape, def_nTabEnhancedShape);
  PX_Color(pPX, _T("CellNoteIndicatorColor"), crCellNoteIndicatorColor, def_crCellNoteIndicatorColor);
  PX_Short(pPX, _T("CellNoteIndicatorShape"), nCellNoteShape, def_nCellNoteShape);
  PX_Short(pPX, _T("HighlightHeaders"), nHighlightHeaders, def_nHighlightHeaders);
  PX_Short(pPX, _T("HighlightStyle"), nHighlightHeaders, def_nHighlightStyle);
  PX_Color(pPX, _T("HighlightAlphaBlendColor"), clrHighlightAlphaBlendColor, def_clrHighlightAlphaBlendColor);
  PX_Short(pPX, _T("HighlightAlphaBlend"), nHighlightAlphaBlend, def_nHighlightAlphaBlend);
  PX_Short(pPX, _T("UseVisualStyles"), nUseVisualStyles, def_nUseVisualStyles);
  PX_Bool(pPX, _T("EnhanceStaticCells"), bEnhanceStaticCells, def_bEnhanceStaticCells);
   TRY
   {
      PX_Short(pPX, _T("ScrollBarStyle"), nScrollBarStyle, def_nScrollBarStyle);
   }
   CATCH_ALL(e)
   {
   }
   END_CATCH_ALL
  }
#endif

  if( pPX->IsLoading() )
  {
    SS_SetBool(lpSS->lpBook, lpSS, SSB_REDRAW, bReDraw);
    //SS_SetBool(lpSS->lpBook, lpSS, SSB_REDRAW, TRUE);
    if( nBorderStyle != m_sBorderStyle )
      SetBorderStyle(nBorderStyle);
    if( !hBlob && IsConvertingVBX() ) // check for old VBX
      PX_Blob(pPX, _T("InterfaceDesigner"), hBlob);
    if( !hBlob )  // check for old beta
      PX_Blob(pPX, _T("BlobData"), hBlob);

    if( hBlob )
    {
      hpBlob = (HPBYTE)GlobalLock(hBlob);
      if( IsConvertingVBX() )
        hpBlob = hpBlob + sizeof(long);
      lDataLen = *(LPLONG)hpBlob;
      hpData = hpBlob + sizeof(long);
      if( lDataLen )
        SS_LoadFromBuffer(lpSS->lpBook, hpData, lDataLen);
      GlobalUnlock(hBlob);
    }

// 25104 -scl
#ifdef SS_V80
   if (HIWORD(m_dwVersionLoaded) < 8)
 	  SS_SetAppearanceStyle(lpSS->lpBook, 0);
#endif
    if( 0 == lDataLen )
    {
// RFW - 7/31/03 - GrapeCity font is being initialized to garbage.
//      OnFontChanged();
      if( bAllowCellOverflow != def_bAllowCellOverflow )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWCELLOVERFLOW, bAllowCellOverflow);
      if( bAllowDragDrop != def_bAllowDragDrop )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWDRAGDROP, bAllowDragDrop);
      if( bAllowMultiBlocks != def_bAllowMultiBlocks )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWMULTIBLOCKS, bAllowMultiBlocks);
      if( bAllowUserFormulas != def_bAllowUserFormulas )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWUSERFORMULAS, bAllowUserFormulas);
      if( bArrowsExitEditMode != def_bArrowsExitEditMode )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_ARROWSEXITEDITMODE, bArrowsExitEditMode);
      if( bAutoCalc != def_bAutoCalc )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOCALC, bAutoCalc);
      if( bAutoClipboard != def_bAutoClipboard )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOCLIPBOARD, bAutoClipboard);
      if( bAutoSize != def_bAutoSize )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOSIZE, bAutoSize);
      if( nBackColorStyle != def_nBackColorStyle )
        SS_SetBackColorStyle(lpSS->lpBook, nBackColorStyle);
      if( nButtonDrawMode != def_nButtonDrawMode )
        SS_SetButtonDrawMode(lpSS->lpBook, nButtonDrawMode);
      if( nColHeaderDisplay != def_nColHeaderDisplay )
        SS_SetColHeaderDisplay(lpSS, nColHeaderDisplay);
      if( lColsFrozen != def_lColsFrozen || lRowsFrozen != def_lRowsFrozen )
        SS_SetFreeze(lpSS, lColsFrozen, lRowsFrozen);
      #ifndef SSP_NO_DAUTOCELLTYPES
      if( bDAutoCellTypes != def_bDAutoCellTypes )
        SetDAutoCellTypes(bDAutoCellTypes);
      #endif
      #ifndef SSP_NO_DAUTOFILL
      if( bDAutoFill != def_bDAutoFill )
        SetDAutoFill(bDAutoFill);
      #endif
      #ifndef SSP_NO_DAUTOHEADINGS
      if( bDAutoHeadings != def_bDAutoHeadings )
        SetDAutoHeadings(bDAutoHeadings);
      #endif
      if( bDAutoSave != def_bDAutoSave )
        SetDAutoSave(bDAutoSave);
      if( nDAutoSizeCols != def_nDAutoSizeCols )
        SetDAutoSizeCols(nDAutoSizeCols);
      if( bDInformActiveRowChange != def_bDInformActiveRowChange )
        SetDInformActiveRowChange(bDInformActiveRowChange);
      if( bDisplayColHeaders != def_bDisplayColHeaders )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_SHOWCOLHEADERS, bDisplayColHeaders);
      if( bDisplayRowHeaders != def_bDisplayRowHeaders )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_SHOWROWHEADERS, bDisplayRowHeaders);
      if( nEditEnterAction != def_nEditEnterAction )
        SS_SetEditEnterAction(lpSS->lpBook, nEditEnterAction);
      if( bEditModePermanent != def_bEditModePermanent )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_EDITMODEPERMANENT, bEditModePermanent);
      if( bEditModeReplace != def_bEditModeReplace )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_EDITMODEREPLACE, bEditModeReplace);
      if( bFormulaSync != def_bFormulaSync )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_FORMULASYNC, bFormulaSync);
      if( crGrayAreaBackColor != def_crGrayAreaBackColor )
        SS_SetGrayAreaColor(lpSS->lpBook, crGrayAreaBackColor, RGB(0,0,0));
      if( crGridColor != def_crGridColor )
        SS_SetGridColor(lpSS, crGridColor);
      if( bGridShowHoriz != def_bGridShowHoriz ||
          bGridShowVert != def_bGridShowVert ||
          bGridSolid != def_bGridSolid )
        SS_SetGridType(lpSS, (bGridShowHoriz ? SS_GRID_HORIZONTAL : 0) |
                             (bGridShowVert ? SS_GRID_VERTICAL : 0) |
                             (bGridSolid ? SS_GRID_SOLID : 0));
      if( lMaxCols != def_lMaxCols )
        SetMaxCols(lMaxCols);
      if( lMaxRows != def_lMaxRows )
        SetMaxRows(lMaxRows);
      if( bMoveActiveOnFocus != def_bMoveActiveOnFocus )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_MOVEACTIVEONFOCUS, bMoveActiveOnFocus);
      if( bNoBeep != def_bNoBeep )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_NOBEEP, bNoBeep);
      if( bNoBorder != def_bNoBorder )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_NOBORDER, bNoBorder);
      if( nOperationMode != def_nOperationMode )
        SS_SetOperationMode(lpSS, nOperationMode);
#ifdef SS_V80
	   if (bEnhanceStaticCells != def_bEnhanceStaticCells)
		  SS_SetEnhanceStaticCells(lpSS->lpBook, bEnhanceStaticCells);
	   if (nAppearanceStyle != def_nAppearanceStyle)
		  SS_SetAppearanceStyle(lpSS->lpBook, nAppearanceStyle);
	   if (nTabEnhancedShape != def_nTabEnhancedShape)
		  SS_SetTabEnhancedShape(lpSS->lpBook, nTabEnhancedShape);
	   if (nScrollBarStyle != def_nScrollBarStyle)
		  SS_SetScrollBarStyle(lpSS->lpBook, nScrollBarStyle);
      if( crCellNoteIndicatorColor != def_crCellNoteIndicatorColor )
        SS_SetCellNoteIndicatorColor(lpSS->lpBook, crCellNoteIndicatorColor);
      if( nCellNoteShape != def_nCellNoteShape )
        SS_SetCellNoteIndicatorShape(lpSS->lpBook, nCellNoteShape);
	   if (nHighlightHeaders != def_nHighlightHeaders)
		  SS_SetHighlightHeaders(lpSS->lpBook, nHighlightHeaders);
	   if (nHighlightStyle != def_nHighlightHeaders)
		  SS_SetHighlightStyle(lpSS->lpBook, nHighlightStyle);
	   if (clrHighlightAlphaBlendColor != def_clrHighlightAlphaBlendColor && nHighlightAlphaBlend != def_nHighlightAlphaBlend)
		  SS_SetHighlightAlphaBlend(lpSS->lpBook, clrHighlightAlphaBlendColor, nHighlightAlphaBlend);
	   if (nUseVisualStyles != def_nUseVisualStyles)
		  SS_SetUseVisualStyles(lpSS->lpBook, nUseVisualStyles);
#endif
      if( bProcessTab != def_bProcessTab )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_PROCESSTAB, bProcessTab);
      if( bProtect != def_bProtect )
          SS_SetBool(lpSS->lpBook, lpSS, SSB_PROTECT, bProtect);
      if( bReDraw != def_bReDraw )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_REDRAW, bReDraw);
        if( bRestrictCols != def_bRestrictCols )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_RESTRICTCOLS, bRestrictCols);
      if( bRestrictRows != def_bRestrictRows )
          SS_SetBool(lpSS->lpBook, lpSS, SSB_RESTRICTROWS, bRestrictRows);
      if( bRetainSelBlock != def_bRetainSelBlock )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_RETAINSELBLOCK, bRetainSelBlock);
        if( nRowHeaderDisplay != def_nRowHeaderDisplay )
        SS_SetRowHeaderDisplay(lpSS, nRowHeaderDisplay);
      if( bScrollBarExtMode != def_bScrollBarExtMode )
          SS_SetBool(lpSS->lpBook, lpSS, SSB_SCROLLBAREXTMODE, bScrollBarExtMode);
      if( bScrollBarMaxAlign != def_bScrollBarMaxAlign )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARMAXALIGN, bScrollBarMaxAlign);
        if( nScrollBars != def_nScrollBars )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_HORZSCROLLBAR, nScrollBars & 1 ? TRUE : FALSE);
      if( nScrollBars != def_nScrollBars )
          SS_SetBool(lpSS->lpBook, lpSS, SSB_VERTSCROLLBAR, nScrollBars & 2 ? TRUE : FALSE);
      if( bScrollBarShowMax != def_bScrollBarShowMax )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARSHOWMAX, bScrollBarShowMax);
        if( nSelectBlockOptions != def_nSelectBlockOptions )
        SS_SetSelBlockOptions(lpSS->lpBook, nSelectBlockOptions);
      if( crShadowColor != def_crShadowColor ||
            crShadowText != def_crShadowText ||
          crShadowDark != def_crShadowDark )
        SS_SetShadowColor(lpSS, crShadowColor, crShadowText, crShadowDark, RGB(255,255,255));
        if( lStartingColNumber != def_lStartingColNumber ||
          lStartingRowNumber != def_lStartingRowNumber )
        SS_SetStartingNumbers(lpSS, lStartingColNumber, lStartingRowNumber);
        if( nUnitType != def_nUnitType )
        SS_SetUnitType(lpSS->lpBook, nUnitType);
      if( nUserResize != def_nUserResize )
          SS_SetUserResize(lpSS, nUserResize);
      if( lVirtualMaxRows != def_lVirtualMaxRows )
        SS_VSetMax(lpSS, lVirtualMaxRows);
        if( bVirtualMode != def_bVirtualMode )
        SS_SetBool(lpSS->lpBook, lpSS, SSB_VIRTUALMODE, bVirtualMode);
      if( lVirtualOverlap != def_lVirtualOverlap )
          SS_VSetBufferSize(lpSS, lVirtualRows, lVirtualOverlap);
      if( bVirtualScrollBuffer != def_bVirtualScrollBuffer )
        SS_VSetStyle(lpSS, bVirtualScrollBuffer ? SSV_SCROLLBARSHOWBUFFER : 0);
        if( lVisibleCols != def_lVisibleCols || lVisibleRows != def_lVisibleRows )
        SS_SetAutoSizeVisible(lpSS, lVisibleCols, lVisibleRows);
      #ifndef SS_NOSCBAR
      if( bVScrollSpecial != def_bVScrollSpecial ||
            nVScrollSpecialType != def_nVScrollSpecialType )
        SS_VScrollSetSpecial(lpSS->lpBook, bVScrollSpecial, nVScrollSpecialType);
      #endif
#ifdef SS_V30
      if( nAppearance != def_nAppearance )
          SS_SetAppearance(lpSS->lpBook, nAppearance);
      if( nTextTip != def_nTextTip )
          SetTextTip(nTextTip);
      if( lTextTipDelay != def_lTextTipDelay )
          SetTextTipDelay(lTextTipDelay);
      if( nScrollBarTrack != def_nScrollBarTrack )
          SetScrollBarTrack(nScrollBarTrack);
      if( nClipboardOptions != def_nClipboardOptions )
          SetClipboardOptions(nClipboardOptions);
#endif
#ifdef SS_V35
      if( nCellNoteIndicator != def_nCellNoteIndicator )
          SetCellNoteIndicator(nCellNoteIndicator);
      if( nShowScrollTips != def_nShowScrollTips )
          SetShowScrollTips(nShowScrollTips);
#endif
      SetModifiedFlag(FALSE);
    }

  }

  if( hBlob )
    GlobalFree(hBlob);
  UnlockSheet(0);
}


/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::OnResetState - Reset control to default state

void CSsocxCtrl::OnResetState()
{
  COleControl::OnResetState();  // Resets defaults found in DoPropExchange

  // TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::AboutBox - Display an "About" box to the user

void CSsocxCtrl::AboutBox()
{
/*
    CAboutDlg dlgAbout;
   
    dlgAbout.SetObject(this);
   
    PreModalDialog();
    
    dlgAbout.DoModal();

    PostModalDialog();
*/
//  CDialog dlgAbout(IDD_ABOUTBOX_SSOCX);
//  dlgAbout.DoModal();
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

// Implementation of IObjectSafety
STDMETHODIMP CSsocxCtrl::XObjectSafety::GetInterfaceSafetyOptions(
			REFIID riid, 
			DWORD __RPC_FAR *pdwSupportedOptions, 
			DWORD __RPC_FAR *pdwEnabledOptions)
{
    METHOD_PROLOGUE_EX(CSsocxCtrl, ObjectSafety)

	if (!pdwSupportedOptions || !pdwEnabledOptions)
	{
		return E_POINTER;
	}

	*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
	*pdwEnabledOptions = 0;

	if (NULL == pThis->GetInterface(&riid))
	{
		TRACE(_T("Requested interface is not supported.\n"));
		return E_NOINTERFACE;
	}

	// What interface is being checked out anyhow?
	OLECHAR szGUID[39];
	int i = StringFromGUID2(riid, szGUID, 39);

	if (riid == IID_IDispatch)
	{
		// Client wants to know if object is safe for scripting
		*pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
		return S_OK;
	}
	else if (riid == IID_IPersistPropertyBag 
		  || riid == IID_IPersistStreamInit
		  || riid == IID_IPersistStorage
		  || riid == IID_IPersistMemory)
	{
		// Those are the persistence interfaces COleControl derived controls support
		// as indicated in AFXCTL.H
		// Client wants to know if object is safe for initializing from persistent data
		*pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
		return S_OK;
	}
	else
	{
		// Find out what interface this is, and decide what options to enable
		TRACE(_T("We didn't account for the safety of this interface, and it's one we support...\n"));
		return E_NOINTERFACE;
	}	
}

STDMETHODIMP CSsocxCtrl::XObjectSafety::SetInterfaceSafetyOptions(
		REFIID riid, 
		DWORD dwOptionSetMask, 
		DWORD dwEnabledOptions)
{
    METHOD_PROLOGUE_EX(CSsocxCtrl, ObjectSafety)

	OLECHAR szGUID[39];
	// What is this interface anyway?
	// We can do a quick lookup in the registry under HKEY_CLASSES_ROOT\Interface
	int i = StringFromGUID2(riid, szGUID, 39);

	if (0 == dwOptionSetMask && 0 == dwEnabledOptions)
	{
		// the control certainly supports NO requests through the specified interface
		// so it's safe to return S_OK even if the interface isn't supported.
		return S_OK;
	}

	// Do we support the specified interface?
	if (NULL == pThis->GetInterface(&riid))
	{
		TRACE1("%s is not support.\n", szGUID);
		return E_FAIL;
	}


	if (riid == IID_IDispatch)
	{
		TRACE(_T("Client asking if it's safe to call through IDispatch.\n"));
		TRACE(_T("In other words, is the control safe for scripting?\n"));
		if (INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwOptionSetMask && INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwEnabledOptions)
		{
            pThis->m_fSafeForScripting = TRUE;
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}
	}
	else if (riid == IID_IPersistPropertyBag 
		  || riid == IID_IPersistStreamInit
		  || riid == IID_IPersistStorage
		  || riid == IID_IPersistMemory)
	{
		TRACE(_T("Client asking if it's safe to call through IPersist*.\n"));
		TRACE(_T("In other words, is the control safe for initializing from persistent data?\n"));

		if (INTERFACESAFE_FOR_UNTRUSTED_DATA == dwOptionSetMask && INTERFACESAFE_FOR_UNTRUSTED_DATA == dwEnabledOptions)
		{
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}
	}
	else
	{
		TRACE1("We didn't account for the safety of %s, and it's one we support...\n", szGUID);
		return E_FAIL;
	}
}

STDMETHODIMP_(ULONG) CSsocxCtrl::XObjectSafety::AddRef()
{
    METHOD_PROLOGUE_EX_(CSsocxCtrl, ObjectSafety)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CSsocxCtrl::XObjectSafety::Release()
{
    METHOD_PROLOGUE_EX_(CSsocxCtrl, ObjectSafety)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CSsocxCtrl::XObjectSafety::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
    METHOD_PROLOGUE_EX_(CSsocxCtrl, ObjectSafety)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

/////////////////////////////////////////////////////////////////////////////

void CSsocxCtrl::OnGetControlInfo(LPCONTROLINFO pControlInfo)
{
  LPSPREADSHEET lpSS = LockActiveSheet();
  if( m_hAccel )
    SS_DestroyAcceleratorTable(m_hAccel);
  m_hAccel = SS_CreateAcceleratorTable(lpSS, &m_cAccel);
  pControlInfo->cb = sizeof(CONTROLINFO);
  pControlInfo->hAccel = m_hAccel;
  pControlInfo->cAccel = m_cAccel;
  pControlInfo->dwFlags = 0;
  UnlockActiveSheet();
}

void CSsocxCtrl::OnMnemonic(LPMSG lpMsg)
{
  if( m_hWnd && WM_SYSKEYDOWN == lpMsg->message )
  {
    LPSPREADSHEET lpSS = LockActiveSheet();
    SS_ProcessMnemonic(lpSS, (TCHAR)lpMsg->wParam);
    UnlockActiveSheet();
  }
}

/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::PreCreateWindow - Modify parameters for CreateWindowEx

BOOL CSsocxCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.lpCreateParams = (LPVOID)m_hBook;
  cs.lpszClass = VA_SPREAD_CLASS_NAME;
  cs.dwExStyle &= ~WS_EX_NOPARENTNOTIFY;  // BJO 25Feb98 JOK4480 - Added
  return COleControl::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK SSOcxCallBack(HWND hwnd, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
  LPSS_BOOK lpBook = SS_BookLock(hwnd);
  LRESULT lRet = 0;

  if (lpBook)
  {
    CSsocxCtrl FAR* lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
    lRet = lpCtrl->WindowProc(uMsg, wParam, lParam);
    SS_BookUnlock(hwnd);
  }

  return lRet;
}

/////////////////////////////////////////////////////////////////////////////

int CSsocxCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  LPSS_BOOK lpBook;
  BOOL bRet;

  bRet = COleControl::OnCreate(lpCreateStruct);
  lpBook = LockBook();
  lpBook->hWndOwner = 0;
  lpBook->lpfnCallBack = SSOcxCallBack;

  lpBook->bDesignTime = !AmbientUserMode();

  // BJO 11Nov97 - Begin fix
  // Version 132 of SSOCXCTL.CPP was checked into Source Safe by
  // Rick on 29Oct97.  Prior to that version, the spread HWND was
  // created before the SpreadDesigner information was loaded in
  // the DoPropExchange().  After that version, the spread HWND
  // was created after the DoPropExchange().  Some of the low
  // low level cell overflow routines still require a HWND.  The
  // following code ensures that the overflow flags are properly
  // set after the HWND is created.  If the low level cell overflow
  // are ever modified (so that they do not require a HWND) then
  // the following code can be removed.
  if (lpBook->fAllowCellOverflow)
  {
    LPSPREADSHEET lpSS;
    LPSS_CELL lpCell;
    SS_COORD  i;
    SS_COORD  j;
    short s;

    for (s = 0; s < lpBook->nSheetCnt; s++)
    {
      lpSS = LockSheet(s);
      for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
        for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.DataCnt; j++)
          if (lpCell = SS_LockCellItem(lpSS, j, i))
          {
            if (!lpCell->Data.bOverflow)
              SS_CalcCellOverflow(lpSS, lpCell, NULL, j, i);
            SS_UnlockCellItem(lpSS, j, i);
          }
      UnlockSheet(s);
    }

    SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
  }
  // BJO 11Nov97 - End fix

  if (fTextTipSet)
     SetTextTip(m_nTextTip);

  if (fTextTipDelaySet)
     SetTextTipDelay(m_lTextTipDelay);

  m_fIsVBMyEnvironment = FALSE;
  IVBGetControl *pGetControl; 
  LPDISPATCH lpdispExtendedCtrl;
  if (lpdispExtendedCtrl = GetExtendedControl())
    {
    if (NOERROR == lpdispExtendedCtrl->QueryInterface(IID_IVBGetControl, (LPVOID FAR *)&pGetControl))
      {
      m_fIsVBMyEnvironment = TRUE;
      pGetControl->Release();
      }
    lpdispExtendedCtrl->Release();
    }

  if (m_pReflect
      /* RFW - 2/16/09 - 24730 - I removed the fix for 21264 because it does not allow the
         hour glass to be shown when set on the form.  I put it back to the way it was originally.
      // RFW - 10/3/07 - 21264 - Added the flag m_fIsVBMyEnvironment
      || m_fIsVBMyEnvironment
      */
      )
	  lpBook->fDontSendParentWMSetCursor = TRUE;

#ifdef FP_OLEDRAG
  // If Drag-drop, we have hWnd, and not design time
  if (m_OLEDropMode && m_hWnd && !lpBook->bDesignTime)
    ::RegisterDragDrop(m_hWnd, &m_xfpDropTarget);
#endif

  UnlockBook();
  return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::GetSuperWndProcAddr - Provide storage for window proc

WNDPROC* CSsocxCtrl::GetSuperWndProcAddr(void)
{
  static WNDPROC NEAR pfnSuper;
  return &pfnSuper;
}

/////////////////////////////////////////////////////////////////////////////
// BJO 10Nov97 GRB5822
//
// Copied from \VC5\MFC\AFXIMPL.H header file.

#ifdef WIN32
#define AFX_WNDOLECONTROL_REG   (0x0020)
#ifndef _UNICODE
#define _UNICODE_SUFFIX
#else
#define _UNICODE_SUFFIX _T("u")
#endif
#ifndef _DEBUG
#define _DEBUG_SUFFIX
#else
#define _DEBUG_SUFFIX _T("d")
#endif
#ifdef _AFXDLL
#define _STATIC_SUFFIX
#else
#define _STATIC_SUFFIX _T("s")
#endif
#define AFX_WNDCLASS(s) \
	_T("Afx") _T(s) _T("42") _STATIC_SUFFIX _UNICODE_SUFFIX _DEBUG_SUFFIX
#define AFX_WNDOLECONTROL   AFX_WNDCLASS("OleControl")
#define AfxDeferRegisterClass(fClass) \
	((afxRegisteredClasses & fClass) ? TRUE : AfxEndDeferRegisterClass(fClass))
#ifdef VC60 // VC6 defines this function to take a long -scl
extern BOOL AFXAPI AfxEndDeferRegisterClass(long fClass);
#else
extern BOOL AFXAPI AfxEndDeferRegisterClass(short fClass);
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// BJO 10Nov97 GRB5822
//
// Copied from \VC5\MFC\SRC\CTLIMPL.H header file.

#ifdef WIN32
class CReflectorWnd : public CWnd
{
public:
	CReflectorWnd() : m_pCtrl(NULL) { }
	BOOL Create(const CRect& rect, HWND hWndParent);
	void SetControl(COleControl* pCtrl);
protected:
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	COleControl* m_pCtrl;
};
#endif

#if SS_V80
/*
BOOL CSsocxCtrl::MyCreateControlWindow(HWND hWndParent, const CRect& rcPos, LPCRECT prcClip)
{
	if (m_hWnd == NULL)
	{
		// If window doesn't exist, create it.

		// Test if:
		// we're not subclassing a Windows control, or
		// container reflects messages for us...

      // REMOVED WS_VISIBLE -scl
		//DWORD dwStyle = WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
		DWORD dwStyle = WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
		if (m_sBorderStyle)
			dwStyle |= WS_BORDER;
		if (!m_bEnabled)
			dwStyle |= WS_DISABLED;
		DWORD dwExStyle = WS_EX_NOPARENTNOTIFY;
		if (m_sAppearance)
			dwExStyle |= WS_EX_CLIENTEDGE;

		// we create normally if:
		//       (we're not subclassing -or- the container reflects)
		// -and- the container autoclips for us
		if ((!IsSubclassedControl() || m_bMsgReflect) && m_bAutoClip)
		{
			// Just create the control's window.
			VERIFY(AfxDeferRegisterClass(AFX_WNDOLECONTROL_REG));
			CreateEx(dwExStyle, AFX_WNDOLECONTROL, m_strText, dwStyle,
				rcPos.left, rcPos.top, rcPos.Width(), rcPos.Height(),
				hWndParent, 0);
		}
		else    // ...we're subclassing a Windows control.
		{
			if (m_pReflect == NULL)
			{
				// Create a window to reflect notification messages.
				m_pReflect = new CReflectorWnd;
				if (prcClip == NULL)
					prcClip = rcPos;
				if (!m_pReflect->Create(prcClip, hWndParent))
				{
					// If m_pReflect->Create failed, then m_pReflect deleted itself.
					m_pReflect = NULL;
				}
			}
			else
			{
				// Reflector window already exists... just reparent it.
				if (m_pReflect->m_hWnd != NULL)
				{
					::SetParent(m_pReflect->m_hWnd, hWndParent);
					::SetWindowPos(m_pReflect->m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|
						SWP_SHOWWINDOW);
				}
			}

			if (m_pReflect != NULL && m_pReflect->m_hWnd != NULL)
			{
				// Create the control's window.
				CreateEx(dwExStyle, NULL, m_strText, dwStyle,
					m_ptOffset.x, m_ptOffset.y, rcPos.Width(), rcPos.Height(),
					m_pReflect->m_hWnd, 0);
				if (m_hWnd == NULL)
				{
					// Window creation failed: cleanup.
					m_pReflect->DestroyWindow();
					m_pReflect = NULL;
				}
			}
		}

		// Set the new window's font.
		OnFontChanged();
	}
	else
	{
		// If window does exist, reparent it...
		CWnd* pWndOuter = GetOuterWindow();
		ASSERT(pWndOuter != NULL);

		if (::GetParent(pWndOuter->m_hWnd) != hWndParent)
			ReparentControlWindow(pWndOuter->m_hWnd, hWndParent);

		::SetWindowPos(pWndOuter->m_hWnd, NULL, 0, 0, 0, 0,
			SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|
			SWP_SHOWWINDOW);

		// And then reposition it...
		OnSetObjectRects(rcPos, prcClip);
	}

	ASSERT(m_hWnd != NULL);
	return (m_hWnd != NULL);
}
*/
#endif
/////////////////////////////////////////////////////////////////////////////
// BJO 10Nov97 GRB5822
//
// Copied from \VC5\MFC\SRC\CTLCORE.CPP source file.  The call
// to OnFontChanged() was removed.

#ifdef WIN32
BOOL CSsocxCtrl::CreateControlWindow(HWND hWndParent, const CRect& rcPos,
	LPCRECT prcClipped)
{
BOOL fRet;

m_fInsideCreateControlWindow = TRUE;
//#if SS_V80
//fRet = MyCreateControlWindow(hWndParent, rcPos, prcClipped);
//#else
fRet = COleControl::CreateControlWindow(hWndParent, rcPos, prcClipped);
//#endif
m_fInsideCreateControlWindow = FALSE;

return (fRet);
}


#if 0
BOOL CSsocxCtrl::CreateControlWindow(HWND hWndParent, const CRect& rcPos,
	LPCRECT prcClipped)
{
	if (prcClipped == NULL)
		prcClipped = &rcPos;

	if (m_hWnd == NULL)
	{
		// If window doesn't exist, create it.

		// Test if:
		// we're not subclassing a Windows control, or
		// container reflects messages for us...

		DWORD dwStyle = WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
		if (m_sBorderStyle)
			dwStyle |= WS_BORDER;
		if (!m_bEnabled)
			dwStyle |= WS_DISABLED;
//        DWORD dwExStyle = WS_EX_NOPARENTNOTIFY; // RFW 11/22/98 SCS6287
        DWORD dwExStyle = 0;
		if (m_sAppearance)
			dwExStyle |= WS_EX_CLIENTEDGE;

		// we create normally if:
		//       (we're not subclassing -or- the container reflects)
		// -and- the container autoclips for us
		if ((!IsSubclassedControl() || m_bMsgReflect) && m_bAutoClip)
		{
			// Just create the control's window.
			if (AfxDeferRegisterClass(AFX_WNDOLECONTROL_REG))
			{
				CreateEx(dwExStyle, AFX_WNDOLECONTROL, m_strText, dwStyle,
					rcPos.left, rcPos.top, rcPos.Width(), rcPos.Height(),
					hWndParent, 0);
			}
		}
		else    // ...we're subclassing a Windows control.
		{
			if (m_pReflect == NULL)
			{
				// Create a window to reflect notification messages.
				m_pReflect = new CReflectorWnd;
				if (!m_pReflect->Create(prcClipped, hWndParent))
				{
					// If m_pReflect->Create failed, then m_pReflect deleted itself.
					m_pReflect = NULL;
				}
			}
			else
			{
				// Reflector window already exists... just reparent it.
				if (m_pReflect->m_hWnd != NULL)
				{
					::SetParent(m_pReflect->m_hWnd, hWndParent);
					::SetWindowPos(m_pReflect->m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|
						SWP_SHOWWINDOW);
				}
			}

			if (m_pReflect != NULL && m_pReflect->m_hWnd != NULL)
			{
				// Create the control's window.
				CreateEx(dwExStyle, NULL, m_strText, dwStyle,
					m_ptOffset.x, m_ptOffset.y,
					rcPos.Width(), rcPos.Height(),
					m_pReflect->m_hWnd, 0);
				if (m_hWnd == NULL)
				{
					// Window creation failed: cleanup.
					m_pReflect->DestroyWindow();
					m_pReflect = NULL;
				}
			}
		}

		// Set the new window's font.
		//OnFontChanged();  // BJO 10Nov97 GRB5822 - Removed
	}
	else
	{
		// If window does exist, reparent and reposition it.
		CWnd* pWndOuter = GetOuterWindow();
		ASSERT(pWndOuter != NULL);

		if (::GetParent(pWndOuter->m_hWnd) != hWndParent)
			ReparentControlWindow(pWndOuter->m_hWnd, hWndParent);

        ::SetWindowPos(pWndOuter->m_hWnd, NULL, 0, 0, 0, 0,
            SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|
            SWP_SHOWWINDOW);

		if (m_pReflect == NULL)
			::MoveWindow(m_hWnd, rcPos.left, rcPos.top,
				rcPos.Width(), rcPos.Height(), TRUE);
		else
		{
			pWndOuter->MoveWindow(prcClipped, TRUE);
			::MoveWindow(m_hWnd, m_ptOffset.x, m_ptOffset.y,
				rcPos.Width(), rcPos.Height(), TRUE);
		}
	}

	ASSERT(m_hWnd != NULL);
	return (m_hWnd != NULL);
}
#endif // 0
#endif // WIN32

/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl::OnOcmCommand - Handle command messages

LRESULT CSsocxCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
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


/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl message handlers
BOOL CSsocxCtrl::PreTranslateMessage(LPMSG lpMsg)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  lpSS = LockActiveSheet();

  switch( lpMsg->message )
  {
    case WM_KEYDOWN:
      // [VS] 2007/05/05 - Workaround broken IME-emulation layer bugs on Vista.
      {
		  OSVERSIONINFO osvi;
		  static int bIsVistaOrLater = -1; // cached
		  SS_CELLTYPE CellType;
		  SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.CurAt, lpSS->Row.CurAt);

          // Check for IME CellType, correct EditMode, and all lpMsg->wParam between '0' & 'Z'; NUMPAD; & some Spl. Codes.
          if(((CellType.Spec.Edit.ChrSet == SS_CHRSET_ALLCHR_IME) || (CellType.Spec.Edit.ChrSet == SS_CHRSET_KNJONLY_IME)) &&
			  ((SS_GetEditMode(lpSS->lpBook) == FALSE) &&
			    (((lpMsg->wParam >= 0x30) && (lpMsg->wParam <= 0x5a)) ||
				 ((lpMsg->wParam >= VK_NUMPAD0) && (lpMsg->wParam <= VK_DIVIDE)) ||
				 ((lpMsg->wParam >= 0xba) && (lpMsg->wParam <= 0xde))) )) {

          osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		  osvi.dwMajorVersion = 0;
          if(bIsVistaOrLater < 0) {
			  GetVersionEx(&osvi);
			  if((osvi.dwMajorVersion > 6) || ((osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 0))) {
			    bIsVistaOrLater = 1;

				if(!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			      keybd_event(lpMsg->wParam, 0, 0, 0);
			      keybd_event(lpMsg->wParam, 0, KEYEVENTF_KEYUP, 0);
				}
			  } else {
			    bIsVistaOrLater = 0;
			  }
		  } else if(bIsVistaOrLater == 1){
				if(!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			      keybd_event(lpMsg->wParam, 0, 0, 0);
			      keybd_event(lpMsg->wParam, 0, KEYEVENTF_KEYUP, 0);
				}
		  }
/*			INPUT input[2];
			memset(input, 0, sizeof(input));
			input[0].type = INPUT_KEYBOARD;

			input[0].ki.wVk = (USHORT)lpMsg->wParam;
			input[0].ki.dwFlags = 0;
			input[0].ki.time = 0;
			input[0].ki.dwExtraInfo = 0;

			input[1].ki.wVk = (USHORT)lpMsg->wParam;
			input[1].ki.dwFlags = KEYEVENTF_KEYUP;
			input[1].ki.time = 0;
			input[1].ki.dwExtraInfo = 0;

			SendInput(2, input, sizeof(INPUT));*/
		  }
          // fallthrough
      }
      // [VS] 2007/05/05 - Fix wrong IME-mode in last-character bugs on WinXP.
      switch( lpMsg->wParam )
      {
        case VK_NUMPAD0:
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
        case VK_F2:
        //case VK_SPACE:
        case VK_BACK:
          if(SS_GetEditMode(lpSS->lpBook) == FALSE)
            SS_SetEditMode(lpSS->lpBook, TRUE);
          // fallthrough
			break;
		// SAS - 21962
		case VK_DELETE:
			if (IsContainerAccess())
			{
				bRet = true;
				break;
			}
		// SAS - 21962

      }

    case WM_KEYUP:
      switch( lpMsg->wParam )
      {
        case VK_UP:
          bRet = TRUE;
          if( WM_KEYDOWN == lpMsg->message )
          {
            SS_COORD lCol, lRow;
            SS_CELLTYPE cellType;
            SS_GetCellBeingEdited(lpSS, &lCol, &lRow);
            if( SS_GetEditMode(lpSS->lpBook) &&
                SS_GetCellType(lpSS, lCol, lRow, &cellType) &&
                SS_TYPE_COMBOBOX == cellType.Type )
            {
              // RFW - 4/7/05 - 16000
              lpMsg->hwnd = ::GetFocus(); //SS_RetrieveControlhWnd(lpSS, lCol, lRow);
              bRet = TRUE;
            }
            else if( SS_ScrollLineUpTest(lpSS) )
            {
              FireQueryAdvanceCommon(FALSE, &bRet);
            }
          }
          break;
        case VK_DOWN:
          bRet = TRUE;
          if( WM_KEYDOWN == lpMsg->message )
          {
            SS_COORD lCol, lRow;
            SS_CELLTYPE cellType;
            SS_GetCellBeingEdited(lpSS, &lCol, &lRow);
            if( SS_GetEditMode(lpSS->lpBook) &&
                SS_GetCellType(lpSS, lCol, lRow, &cellType) &&
                SS_TYPE_COMBOBOX == cellType.Type )
            {
              // RFW - 4/7/05 - 16000
              lpMsg->hwnd = ::GetFocus(); //SS_RetrieveControlhWnd(lpSS, lCol, lRow);
              bRet = TRUE;
            }
            else if( SS_ScrollLineDownTest(lpSS) )
            {
              FireQueryAdvanceCommon(TRUE, &bRet);
            }
          }
          break;
        case VK_LEFT:
        case VK_RIGHT:
		  case VK_PRIOR:
		  case VK_NEXT:
		  case VK_HOME:
	     case VK_END:
          // RFW - 2/22/05 - 15782
          // This is to work around a Delphi problem
          if (SS_GetEditMode(lpSS->lpBook) && lpMsg->hwnd == lpSS->lpBook->hWnd)
            bRet = FALSE;
          else
            bRet = TRUE;

          break;
        case VK_TAB:
          if( SS_OPMODE_READONLY != lpSS->wOpMode &&
              SS_GetBool(lpSS->lpBook, lpSS, SSB_PROCESSTAB) &&
              !(GetKeyState(VK_CONTROL) & 0x8000) )
          {
            bRet = TRUE;
            if( WM_KEYDOWN == lpMsg->message )
            {
              if( HIBYTE(GetKeyState(VK_SHIFT)) )
              {
                if( SSx_ScrollPrevCell(lpSS, TRUE) )
                  FireQueryAdvanceCommon(FALSE, &bRet);
              }
              else
              {
                if( SSx_ScrollNextCell(lpSS, TRUE, FALSE) )
                  FireQueryAdvanceCommon(TRUE, &bRet);
              }
            }
          }
          /* RFW - 5/26/09 - 25498
		    else // 20059 -scl
          */
          else if (lpSS->lpBook->EditModeOn)
			   SendMessage(SSM_SETEDITMODE, FALSE, 0L);

          break;
        case VK_ESCAPE: // RFW - 6/20/06 - 19089  Allow combo box to close on ESC key
          if( SS_GetEditMode(lpSS->lpBook) )
          {
            SS_COORD lCol, lRow;
            SS_CELLTYPE cellType;
            HWND hWndCombo;

            SS_GetCellBeingEdited(lpSS, &lCol, &lRow);
            hWndCombo = SS_RetrieveControlhWnd(lpSS, lCol, lRow);
            if( SS_GetCellType(lpSS, lCol, lRow, &cellType) &&
                SS_TYPE_COMBOBOX == cellType.Type &&
                ::SendMessage(hWndCombo, CB_GETDROPPEDSTATE, 0, 0) )
            {
              ::SendMessage(hWndCombo, CB_SHOWDROPDOWN, FALSE, 0);
              return (TRUE);
            }
          }

          break;

      }

      // RFW - 4/14/00 - SCS9037
//      if (!m_fIsVBMyEnvironment && !bRet)
		// RFW - 8/12/03 - 12324
      if (!bRet)
      {
         if ((GetKeyState(VK_CONTROL) & 0x8000) && lpSS->lpBook->fAutoClipboard)
           /* RFW - 7/22/04 - 14909
           if (lpMsg->wParam == 'x' || lpMsg->wParam == 'X' ||
               lpMsg->wParam == 'c' || lpMsg->wParam == 'C' ||
               lpMsg->wParam == 'v' || lpMsg->wParam == 'V')
           */
           if (lpMsg->wParam == 'X' || lpMsg->wParam == 'C' || lpMsg->wParam == 'V')
           {
					   /* RFW - 8/21/02 - 10853 */
             if (!SS_GetEditMode(lpSS->lpBook))
				 {
               bRet = COleControl::PreTranslateMessage(lpMsg);
               if( !bRet ) 
               {
                 //GAB added check to fix Grapecity Ctrl+C menu bug
                 // RFW - 3/8/04 - 13845
                 //::SendMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
                 /* RFW - 7/22/04 - 14909
                 if( !(lpMsg->wParam == 'c' || lpMsg->wParam == 'C'))
                 */
                 /* RFW - 2/16/05 - 10853
                 if( !(lpMsg->wParam == 'C'))
                 */
                 /* RFW - 7/17/08 - 22691
                 if( !(lpMsg->wParam == 'C') || !m_fIsVBMyEnvironment)
                 */
                 if( !(lpMsg->wParam == 'C' || lpMsg->wParam == 'V') || !m_fIsVBMyEnvironment)
                 {
                   ::SendMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
                   UnlockActiveSheet();
                   return (TRUE);
                 }
               }
             }
             /* RFW - 7/22/04 - 14909
             else if (lpMsg->wParam == 'c' || lpMsg->wParam == 'C')
             */
             else if (lpMsg->wParam == 'C')
             {
               SS_SendMessageToVB(lpSS->lpBook->hWnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
               ::SendMessage(lpMsg->hwnd, WM_CHAR, 3, lpMsg->lParam);
               UnlockActiveSheet();
               return (TRUE);
             }
           }

			/* RFW - 12/30/03 - 13474 
         // RFW - 4/19/00 - SCS9529
         if (lpMsg->wParam == VK_DELETE)
         {
           if (!SS_GetEditMode(lpSS->lpBook))
			  {
             // RFW - 12/9/03 - 13172
             bRet = COleControl::PreTranslateMessage(lpMsg);
             if( !bRet ) 
             {
               ::SendMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
               //bRet = TRUE;
             }
             return (bRet);
           }
         }
			*/
       }

      break;
  }

  UnlockActiveSheet();

  if( bRet )
    ::SendMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
  else
    bRet = COleControl::PreTranslateMessage(lpMsg);

  return bRet;
}

#if 0
//--------------------------------------------------------------------

//LPCTSTR szBorderStyle[] =
//{
//  _T("0 - None"),
//  _T("1 - Fixed Single"),
//};

LPCTSTR szColHeaderDisplay[] =
{
   _T("0 - Blank"),
   _T("1 - Display Numbers"),
   _T("2 - Display Letters"),
};

LPCTSTR szEditEnterAction[] =
{
   _T("0 - None"),
   _T("1 - Up"),
   _T("2 - Down"),
   _T("3 - Left"),
   _T("4 - Right"),
   _T("5 - Next"),
   _T("6 - Previous"),
   _T("7 - Same"),
   _T("8 - Next Row"),
};

LPCTSTR szOperationMode[] =
{
  _T("0 - Normal"),
  _T("1 - Read Only"),
  _T("2 - Row Mode"),
  _T("3 - Single Select"),
  _T("4 - Multi Select"),
  _T("5 - Extended Select"),
};
#ifdef SS_V80
LPCTSTR szAppearanceStyle[] =
{
  _T("0 - Classic"),
  _T("1 - Visual Styles"),
  _T("2 - Enhanced"),
};
LPCTSTR szScrollBarStyle[] =
{
  _T("0 - Inherit"),
  _T("1 - Classic"),
  _T("2 - Visual Styles"),
  _T("3 - Enhanced"),
};
LPCTSTR szTabEnhancedShape[] =
{
  _T("0 - Rectangle"),
  _T("1 - Rounded Rectangle"),
  _T("2 - Trapezoid"),
  _T("3 - Isosceles Trapezoid"),
};
LPCTSTR szHighlightHeaders[] =
{
  _T("0 - Default"),
  _T("1 - Off"),
  _T("2 - On"),
};
LPCTSTR szHighlightStyle[] =
{
  _T("0 - Default"),
  _T("1 - Invert"),
  _T("2 - Color"),
  _T("3 - AlphaBlend"),
};
LPCTSTR szUseVisualStyles[] =
{
  _T("0 - Inherit"),
  _T("1 - Yes"),
  _T("2 - No"),
};
#endif // SS_V80
LPCTSTR szPosition[] =
{
   _T("0 - Upper Left"),
   _T("1 - Upper Center"),
   _T("2 - Upper Right"),
   _T("3 - Center Left"),
   _T("4 - Center"),
   _T("5 - Center Right"),
   _T("6 - Bottom Left"),
   _T("7 - Bottom Center"),
   _T("8 - Bottom Right"),
};

LPCTSTR szScrollBars[] =
{
   _T("0 - None"),
   _T("1 - Horizontal"),
   _T("2 - Vertical"),
   _T("3 - Both"),
};

LPCTSTR szUnitType[] =
{
   _T("0 - Normal"),
   _T("1 - VGA Base"),
   _T("2 - Twips"),
};

LPCTSTR szCellNoteIndicator[] =
{
   _T("ShowAndFireEvent"),
   _T("ShowAndDoNotFireEvent"),
   _T("DoNotShowAndFireEvent"),
   _T("DoNotShowAndDoNotFireEvent"),
};

LPCTSTR szShowScrollTips[] =
{
   _T("Off"),
   _T("Vertical"),
   _T("Horizontal"),
   _T("Both"),
};

//--------------------------------------------------------------------

LPCTSTR FAR* CSsocxCtrl::DispToEnum(DISPID dispid, int FAR* lpnMax)
{
  LPCTSTR FAR* lplpszEnum = NULL;

  switch( dispid )
  {
    //case dispidBorderStyle:
    //  lplpszEnum = szBorderStyle;
    //  *lpnMax = sizeof(szBorderStyle) / sizeof(szBorderStyle);
    //  break;
    #ifndef SSP_NO_COLHEADERDISPLAY
    case dispidColHeaderDisplay:
    case dispidRowHeaderDisplay:
      lplpszEnum = szColHeaderDisplay;
      *lpnMax = sizeof(szColHeaderDisplay) / sizeof(szColHeaderDisplay[0]);
      break;
    #endif
    #ifndef SSP_NO_EDITENTERACTION
    case dispidEditEnterAction:
      lplpszEnum = szEditEnterAction;
      *lpnMax = sizeof(szEditEnterAction) / sizeof(szEditEnterAction[0]);
      break;
    #endif
    #ifndef SSP_NO_OPERATIONMODE
    case dispidOperationMode:
      lplpszEnum = szOperationMode;
      *lpnMax = sizeof(szOperationMode) / sizeof(szOperationMode[0]);
      break;  
    #endif
#ifdef SS_V80
    #ifndef SSP_NO_USEVISUALSTYLES
    case dispidUseVisualStyles:
      lplpszEnum = szUseVisualStyles;
      *lpnMax = sizeof(szUseVisualStyles) / sizeof(szUseVisualStyles[0]);
      break;  
    #endif
    #ifndef SSP_NO_APPEARANCESTYLE
    case dispidAppearanceStyle:
      lplpszEnum = szAppearanceStyle;
      *lpnMax = sizeof(szAppearanceStyle) / sizeof(szAppearanceStyle[0]);
      break;  
    #endif
    #ifndef SSP_NO_SCROLLBARSTYLE
    case dispidScrollBarStyle:
      lplpszEnum = szScrollBarStyle;
      *lpnMax = sizeof(szScrollBarStyle) / sizeof(szScrollBarStyle[0]);
      break;  
    #endif
    #ifndef SSP_NO_TABENHANCEDSHAPE
    case dispidTabEnhancedShape:
      lplpszEnum = szTabEnhancedShape;
      *lpnMax = sizeof(szTabEnhancedShape) / sizeof(szTabEnhancedShape[0]);
      break;  
    #endif
    #ifndef SSP_NO_HIGHLIGHTHEADERS
    case dispidHighlightHeaders:
      lplpszEnum = szHighlightHeaders;
      *lpnMax = sizeof(szHighlightHeaders) / sizeof(szHighlightHeaders[0]);
      break;  
    #endif
    #ifndef SSP_NO_HIGHLIGHTSTYLE
    case dispidHighlightStyle:
      lplpszEnum = szHighlightStyle;
      *lpnMax = sizeof(szHighlightStyle) / sizeof(szHighlightStyle[0]);
      break;  
    #endif
#endif // SS_V80
    #ifndef SSP_NO_POSITION
    case dispidPosition:
      lplpszEnum = szPosition;
      *lpnMax = sizeof(szPosition) / sizeof(szPosition[0]);
      break;
    #endif
    #ifndef SSP_NO_SCROLLBARS
    case dispidScrollBars:
      lplpszEnum = szScrollBars;
      *lpnMax = sizeof(szScrollBars) / sizeof(szScrollBars[0]);
      break;
    #endif
    #ifndef SSP_NO_UNITTYPE
    case dispidUnitType:
      lplpszEnum = szUnitType;
      *lpnMax = sizeof(szUnitType) / sizeof(szUnitType[0]);
      break;
    #endif
#ifdef SS_V35
    case dispidCellNoteIndicator:
      lplpszEnum = szCellNoteIndicator;
      *lpnMax = sizeof(szCellNoteIndicator) / sizeof(szCellNoteIndicator[0]);
      break;
    case dispidShowScrollTips:
      lplpszEnum = szShowScrollTips;
      *lpnMax = sizeof(szShowScrollTips) / sizeof(szShowScrollTips[0]);
      break;
#endif
  }
  return lplpszEnum;
}

//--------------------------------------------------------------------
//
//  Copied from COleControl::OnGetDisplayString() in CTLPROP.CPP
//  file.
// 

BOOL CSsocxCtrl::OnGetDisplayString(DISPID dispid, CString& strValue)
{
  LPCTSTR FAR* lplpszEnum;
  int nEnumMax;
  short nValue = -1;

  TRY
  {
    switch (dispid)
    {
      //case dispidBorderStyle:
      //  nValue = GetBorderStyle();
      //  break;
      #ifndef SSP_NO_COLHEADERDISPLAY
      case dispidColHeaderDisplay:
        nValue = GetColHeaderDisplay();
        break;
      #endif
      #ifndef SSP_NO_EDITENTERACTION
      case dispidEditEnterAction:
        nValue = GetEditEnterAction();
        break;
      #endif
#ifdef SS_V80
      #ifndef SSP_NO_USEVISUALSTYLES
      case dispidUseVisualStyles:
        nValue = GetUseVisualStyles();
        break;
      #endif
      #ifndef SSP_NO_APPEARANCESTYLE
      case dispidAppearanceStyle:
        nValue = GetAppearanceStyle();
        break;
      #endif
      #ifndef SSP_NO_SCROLLBARSTYLE
      case dispidScrollBarStyle:
        nValue = GetScrollBarStyle();
        break;
      #endif
      #ifndef SSP_NO_TABENHANCEDSHAPE
      case dispidTabEnhancedShape:
        nValue = GetTabEnhancedShape();
        break;
      #endif
      #ifndef SSP_NO_HIGHLIGHTHEADERS
      case dispidHighlightHeaders:
        nValue = GetHighlightHeaders();
        break;
      #endif
      #ifndef SSP_NO_HIGHLIGHTSTYLE
      case dispidHighlightStyle:
        nValue = GetHighlightStyle();
        break;
      #endif
#endif
      #ifndef SSP_NO_OPERATIONMODE
      case dispidOperationMode:
        nValue = GetOperationMode();
        break;
      #endif
      #ifndef SSP_NO_POSITION
      case dispidPosition:
        nValue = GetPosition();
        break;
      #endif
      #ifndef SSP_NO_ROWHEADERDISPLAY
      case dispidRowHeaderDisplay:
        nValue = GetRowHeaderDisplay();
        break;
      #endif
      #ifndef SSP_NO_SCROLLBARS
      case dispidScrollBars:
        nValue = GetScrollBars();
        break;
      #endif
      #ifndef SSP_NO_UNITTYPE
      case dispidUnitType:
        nValue = GetUnitType();
        break;
      #endif
#ifdef SS_V35
      case dispidCellNoteIndicator:
        nValue = GetCellNoteIndicator();
        break;
      case dispidShowScrollTips:
        nValue = GetShowScrollTips();
        break;
#endif
    }
    if( lplpszEnum = DispToEnum(dispid, &nEnumMax) )
      if( 0 <= nValue && nValue < nEnumMax )
      {
        strValue = lplpszEnum[nValue];
        return TRUE;
      }
  }
  END_TRY

  return COleControl::OnGetDisplayString(dispid, strValue);
}

//--------------------------------------------------------------------
//
// Copied from COleControl::OnGetPredefinedStrings() in CTLPROP.CPP
// file.
// 

#endif

#ifdef FP_ADB
BOOL CSsocxCtrl::OnGetPredefinedStrings(DISPID dispid,
                                        CStringArray* pStringArray,
                                        CDWordArray* pCookieArray)
{

#ifdef FP_ADB
  // If DataMember property, call fpdb func to get list of DataMembers.
  if (dispid == dispidDataMember)
  {
    // Get the DataMember names, if ok, return true, else false.
#ifdef SS_V70
    LPSS_BOOK lpBook = LockBook();
    if( 0 == lpBook->nActiveSheet )
    {
      LPSPREADSHEET lpSS = LockSheet(0);
      BOOL ret = lpSS->pfpOleBinding ? fpMFCGetListOfDataMemberNames((CfpOleBinding*)lpSS->pfpOleBinding, pStringArray, 
        pCookieArray) : FALSE;
      UnlockSheet(0);
      UnlockBook();
      return ret;
    }
    UnlockBook();
#else
    return fpMFCGetListOfDataMemberNames(&m_fpOleBinding, pStringArray, 
      pCookieArray);
#endif
  }
#endif

  return COleControl::OnGetPredefinedStrings(dispid, pStringArray,
                                             pCookieArray);
}
#endif

//--------------------------------------------------------------------
//
// Copied from COleControl::OnGetPredefinedValue() in CTLPROP.CPP
// file.
// 

#ifdef FP_ADB
BOOL CSsocxCtrl::OnGetPredefinedValue(DISPID dispid, DWORD dwCookie,
                                      VARIANT FAR* lpvarOut)
{
  // If DataMember property, call fpdb func to get list of DataMembers.
  if (dispid == dispidDataMember)
  {
    // Get the DataMember names.
    BSTR bstr = NULL;
#ifdef SS_V70
    LPSPREADSHEET lpSS = LockSheet(0);
    BOOL fResult = lpSS->pfpOleBinding ? ((CfpOleBinding*)lpSS->pfpOleBinding)->GetDataMemberNameForCookie(
      dwCookie, &bstr) : FALSE;
    UnlockSheet(0);
#else
    BOOL fResult = m_fpOleBinding.GetDataMemberNameForCookie(
      dwCookie, &bstr);
#endif
    VariantClear(lpvarOut);
    // If get was successful, copy name to variant.
    if (fResult && bstr)
    {
      V_VT(lpvarOut) = VT_BSTR;
      V_BSTR(lpvarOut) = bstr;
    }
    return fResult;
  }

  return COleControl::OnGetPredefinedValue(dispid, dwCookie, lpvarOut);
}
#endif


void CSsocxCtrl::OnEventAdvise(BOOL bAdvise)
{
  if (!bAdvise)
  {
    if (m_hWndSpreadDsg != (HWND)0)
    {
      // Exit Spread Designer
     ::SendMessage(m_hWndSpreadDsg, SSM_SPREADDSGEXIT, 0, 0L);
    }
  }

  COleControl::OnEventAdvise(bAdvise);
}

BOOL CSsocxCtrl::OnSpreadDesigner(LPMSG lpmsg, HWND hWnd, LPCRECT lpcrect)
{
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND   2
#endif

#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND   3
#endif

  if (m_hWndSpreadDsg != (HWND)0)
  {
    ::MessageBeep((UINT)-1);
  }  
  else
  {
    char buffer[200];

#ifdef WIN32
    hhookMouseFilter = ::SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, 0,
                                        GetCurrentThreadId());
    hhookKeyFilter = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, 0,
                                        GetCurrentThreadId());
#else  
//RWP03a ->
	HourGlassOn();   
//RWP03a <-
	 LPSS_BOOK lpBook = LockBook();
    FPCONTROL fpParent = {0, 0, 0, 0};  // "True" parent
    LPDISPATCH lpdispExtendedCtrl;
    if (lpdispExtendedCtrl = GetExtendedControl())
    {
      // "Frame" window for Subclassed controls
      //    fpParent.hWnd = fpGetParent(&fpControl);
      // "True parent" for Subclassed controls
      FPCONTROL fpControl = {lpBook->hWnd, 0, (long)lpSS->lpOleControl, 
                             (long)lpdispExtendedCtrl};
      fpParent.lpDispatch = fpGetControlDispatch(&fpControl, GC_CONTAINER);                         
      fpParent.hWnd = fpVBGetControlHwnd(&fpParent);
      lpdispExtendedCtrl->Release();
    }

    UnlockBook();
    hhookMouseFilter = ::SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, (HINSTANCE)GetWindowWord(fpParent.hWnd, GWW_HINSTANCE),
                                          GetCurrentTask());
    hhookKeyFilter = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, (HINSTANCE)GetWindowWord(fpParent.hWnd, GWW_HINSTANCE),
                                          GetCurrentTask());
    // release dispatch now that we have the Parent
    if (fpParent.lpDispatch)
       fpDispatchRelease((LONG)fpParent.lpDispatch);
#endif
//    #pragma message(__FILE__ ": Note to Robby: WinExec() only works with an ANSI char string.")      
#if defined(_UNICODE)
    CHAR szBuff[200];
    WideCharToMultiByte(CP_ACP, 0, AmbientDisplayName().GetBuffer(sizeof(TCHAR)), -1, szBuff, sizeof(szBuff), NULL, NULL);
 #ifdef _DEBUG
    sprintf(buffer, "Window handle %ld\n", GetSafeHwnd());
    OutputDebugStringA(buffer);

    sprintf(buffer, "%s %ld %s",SPREAD_DESIGNER, GetSafeHwnd(), szBuff);
//    sprintf(buffer, "%s%s %ld %s", "c:\\sd7\\win32ud70\\", SPREAD_DESIGNER, GetSafeHwnd(), szBuff);
  #else
    sprintf(buffer, "%s %ld %s",SPREAD_DESIGNER, GetSafeHwnd(), szBuff);
  #endif
#else
  sprintf(buffer, "%s %ld %s",SPREAD_DESIGNER, GetSafeHwnd(), AmbientDisplayName().GetBuffer(sizeof(TCHAR)));
#endif
  UINT rc = ::WinExec(buffer, SW_SHOWNORMAL);
  if (rc < 32) // less than 32 indicates an error...
    {
      UnhookWindowsHookEx(hhookMouseFilter);  
      UnhookWindowsHookEx(hhookKeyFilter);    
      MessageBeep((UINT)-1);
    }
  }

//RWP03a ->
#ifndef WIN32
  HourGlassOff();   
#endif//WIN32
//RWP03a <-

  return TRUE;
}

LONG CSsocxCtrl::OnSpreadDsgHereIAm(WPARAM wParam, LPARAM lParam)
{
//AfxMessageBox("OnSpreadDsgHereIAm");         
  m_hWndSpreadDsg = (HWND)lParam;   
#ifdef WIN32
  COPYDATASTRUCT cds;
  SS_BUFFER ss_buffer;
  ss_buffer.g = SSSaveToBuffer(GetSafeHwnd(), &(ss_buffer.l), FALSE);
  cds.lpData = (PVOID)GlobalLock(ss_buffer.g);
  cds.cbData = (DWORD)ss_buffer.l;
  ::SendMessage(m_hWndSpreadDsg, WM_COPYDATA, 0, (LPARAM)(COPYDATASTRUCT FAR *)&cds);
  GlobalUnlock(ss_buffer.g);
  GlobalFree(ss_buffer.g);
#else
  GLOBALHANDLE gh;
  HPBYTE hpS, hpD;
  long lCount; 
  GLOBALHANDLE ghCDS = GlobalAlloc(GHND | GMEM_SHARE, sizeof(COPYDATASTRUCT));  
  COPYDATASTRUCT FAR *pCDS = (COPYDATASTRUCT FAR *)GlobalLock(ghCDS);
  gh = SSSaveToBuffer(GetSafeHwnd(), (LPLONG)&(pCDS->lData), FALSE);    
  pCDS->ghData = GlobalAlloc(GHND | GMEM_SHARE, pCDS->lData);
  hpS = (HPBYTE)GlobalLock(gh);
  hpD = (HPBYTE)GlobalLock(pCDS->ghData);         
  for (lCount=0; lCount<pCDS->lData; lCount++)
  {
    *(hpD+lCount) = *(hpS+lCount);
  }      
  GlobalUnlock(gh);
  GlobalFree(gh);
  GlobalUnlock(pCDS->ghData);  

  ::SendMessage(m_hWndSpreadDsg, WM_COPYDATA, 0, (long)pCDS);
  pCDS = (COPYDATASTRUCT FAR *)GlobalLock(ghCDS);   

  GlobalFree(pCDS->ghData);
  GlobalUnlock(ghCDS);
  GlobalFree(ghCDS);  
#endif  
  return 0L;
}

LONG CSsocxCtrl::OnCopyData(WPARAM wParam, LPARAM lParam)
{
  //Load the buffer to the spreadsheet control
  COPYDATASTRUCT FAR *pCDS = (COPYDATASTRUCT FAR *)lParam;

#ifdef WIN32
  GLOBALHANDLE gh = GlobalAlloc(GHND, pCDS->cbData);
  LONG lDataLen = pCDS->cbData;
  LPVOID lpDataD = (LPVOID)GlobalLock(gh);
  _fmemcpy(lpDataD, pCDS->lpData, lDataLen);
  GlobalUnlock(gh); 
#endif

  if (wParam == 1) // copy format from the designer
  {
    LONG lenData;
    GLOBALHANDLE ghData = SSSaveToBuffer(GetSafeHwnd(), &lenData, TRUE);
#ifdef WIN32
    SSLoadFromBuffer(GetSafeHwnd(), gh, lDataLen);  
#else
    SSLoadFromBuffer(GetSafeHwnd(), pCDS->ghData, pCDS->lData);  
#endif
    SSLoadFromBuffer(GetSafeHwnd(), ghData, lenData);
    SSSetBool(GetSafeHwnd(), SSB_REDRAW, TRUE);
    GlobalFree(ghData);
  }
  else if (wParam == 2) // copy data from the designer
  {
    SSClearData(GetSafeHwnd(), SS_ALLCOLS, SS_ALLROWS);
#ifdef WIN32
    SSLoadFromBuffer(GetSafeHwnd(), gh, lDataLen);  
#else
    SSLoadFromBuffer(GetSafeHwnd(), pCDS->ghData, pCDS->lData);  
#endif
    SSSetBool(GetSafeHwnd(), SSB_REDRAW, TRUE);
  } 
  else 
  {
#ifdef WIN32                       
    SSLoadFromBuffer(GetSafeHwnd(), gh, lDataLen);  
#else
    SSLoadFromBuffer(GetSafeHwnd(), pCDS->ghData, pCDS->lData);  
#endif
    SSSetBool(GetSafeHwnd(), SSB_REDRAW, TRUE);
  } 

#ifdef WIN32
  GlobalFree(gh);
#endif

  BoundPropertyChanged(DISPID_UNKNOWN);
  {
  // In MSVC: invoking Spread Designer and changing
  // OCX properties does not result in a propety
  // exchange occurring. Therefore, no SD changes
  // are saved. The code below forces a property 
  // exchange.
  // I am not sure what SetControlSize() sets that
  // produces the property exchange...

  int cx, cy;
    GetControlSize(&cx, &cy);
  SetControlSize(cx+1, cy);
  SetControlSize(cx, cy);

  SetModifiedFlag();
  InvalidateControl();
  }
  
  return 0L;
}
     
LRESULT CSsocxCtrl::OnSpreadDsgExit(WPARAM wParam, LPARAM lParam)
{
    m_hWndSpreadDsg = (HWND)0;
    UnhookWindowsHookEx(hhookMouseFilter);  
    UnhookWindowsHookEx(hhookKeyFilter);    

  return 0L;
}

LRESULT CALLBACK SD_MouseFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
// Fengwei
#ifdef	SPREAD_JPN	
    if (code < 0 || bGetCursor)
#else
    if (code < 0)
#endif
        return CallNextHookEx(hhookMouseFilter, code, wParam, lParam);
    return (LRESULT)TRUE;
}

LRESULT CALLBACK SD_KeyFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
//Fengwei
#ifdef	SPREAD_JPN
    if (code < 0 || bGetCursor)
#else
    if (code < 0)
#endif
//-
        return CallNextHookEx(hhookKeyFilter, code, wParam, lParam);
    return (LRESULT)TRUE;
}

LRESULT CSsocxCtrl::OnGetColumnNames(WPARAM wParam, LPARAM lParam)
{  
  m_hWndSpreadDsg = (HWND)lParam;   

//RWP01a
  // Return value, defaults to TRUE
  short sRet = TRUE;

  // Temporarily disable the hooks in case the RecordSource property is not set
  // in the DataBind control (re-hook before this method exits)
  UnhookWindowsHookEx(hhookMouseFilter);  
  UnhookWindowsHookEx(hhookKeyFilter);    
//RWP01a

  COPYDATASTRUCT cds;
  DWORD dwOffset=0;
  GLOBALHANDLE gh;
  LPVOID lpData;
#ifdef WIN32
  cds.cbData = 0; 
#else
  cds.lData = 0;
#endif
      
#ifdef WIN32
  cds.cbData = 0; 
  CStringArray StringArray;
  CDWordArray CookieArray;  
//AfxMessageBox("OnGetColumnNames");

#ifdef FP_ADB
#ifdef SS_V70
  LPSPREADSHEET lpSS = LockSheet(0);
  BOOL fResult = lpSS->pfpOleBinding ? fpMFCGetListOfColumnNames((CfpOleBinding*)lpSS->pfpOleBinding, &StringArray, 
      &CookieArray) : FALSE;
  UnlockSheet(0);
#else
  BOOL fResult = fpMFCGetListOfColumnNames(&m_fpOleBinding, &StringArray, 
        &CookieArray);
#endif
#else
#ifdef	SPREAD_JPN
  // Add the flag to enable user close the dialog box VB popped up. Fengwei
  bGetCursor = TRUE;
#endif
  BOOL fResult = m_fpOleBinding.GetListOfColumnNames(&StringArray, &CookieArray);
#ifdef	SPREAD_JPN
  // Add the flag to enable user close the dialog box VB popped up. Fengwei
  bGetCursor = FALSE;
#endif
#endif

  if (fResult)
  {
    // The control is connected to a Data Source.
    // Return the "DataField" strings to the Spread Designer
    CString cstr;
    for (int i=0; i<=StringArray.GetUpperBound(); i++)
    {
      cstr = StringArray.GetAt(i);
//AfxMessageBox(cstr);  
      if (cds.cbData == 0)
      {
        cds.cbData += (cstr.GetLength()+1) * sizeof(TCHAR);
        gh = GlobalAlloc(GHND, cds.cbData);
        lpData = (LPVOID)GlobalLock(gh);
      }
      else
      {
        cds.cbData += (cstr.GetLength()+1) * sizeof(TCHAR);
        gh = GlobalReAlloc(gh, cds.cbData, GHND);
        lpData = (LPVOID)GlobalLock(gh);
        lpData = ADD_OFFSET(lpData, dwOffset);
      }
      _fmemcpy(lpData, cstr.GetBuffer(cstr.GetLength()), cstr.GetLength()*sizeof(TCHAR)); //Add 1 to get the NULL
//AfxMessageBox((LPSTR)lpData);   
      dwOffset = cds.cbData;                          
      GlobalUnlock(gh); 
    } // end for
  }
  else
  {
//RWP01c
	// return FALSE;
    sRet = FALSE; 
//RWP01c
  } 

//RWP01a
  // Re-enable hooks (32 bit)
  hhookMouseFilter = ::SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, 0,
                                      GetCurrentThreadId());
  hhookKeyFilter = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, 0,
                                      GetCurrentThreadId());

  if (!sRet)
	  return sRet;
  else
//RWP01a

  cds.lpData = GlobalLock(gh);
  ::SendMessage(m_hWndSpreadDsg, WM_COPYDATA, 0, (LPARAM)&cds);
  GlobalFree(gh);
#else
  CStringArray StringArray;
  CDWordArray CookieArray;  
//AfxMessageBox("OnGetColumnNames");
  if (m_fpOleBinding.GetListOfColumnNames(&StringArray, &CookieArray))
  {
    // The control is connected to a Data Source.
    // Return the "DataField" strings to the Spread Designer
    CString cstr;
    for (int i=0; i<=StringArray.GetUpperBound(); i++)
    {
      cstr = StringArray.GetAt(i);
//AfxMessageBox(cstr);  
      if (cds.lData == 0)
      {
        cds.lData += (cstr.GetLength()+1) * sizeof(TCHAR);
        gh = GlobalAlloc(GHND, cds.lData);
        lpData = (LPVOID)GlobalLock(gh);
      }
      else
      {
        cds.lData += (cstr.GetLength()+1) * sizeof(TCHAR);
        gh = GlobalReAlloc(gh, cds.lData, GHND);
        lpData = (LPVOID)GlobalLock(gh);
        lpData = ADD_OFFSET(lpData, dwOffset);
      }

      _fmemcpy(lpData, cstr.GetBuffer(cstr.GetLength()), cstr.GetLength()*sizeof(TCHAR)); 
//AfxMessageBox((LPSTR)lpData);   
      dwOffset = cds.lData;                         
      GlobalUnlock(gh); 
    } // end for
  }
  else
  {

//RWP01c
	// return FALSE;
    sRet = FALSE; 
//RWP01c
  } 

//RWP01a
	// Re-enable hooks (16 bit)
    LPSPREADSHEET lpSS = LockSheet();
    FPCONTROL fpParent = {0, 0, 0, 0};  // "True" parent
    LPDISPATCH lpdispExtendedCtrl;
    if (lpdispExtendedCtrl = GetExtendedControl())
    {
      // "Frame" window for Subclassed controls
      //    fpParent.hWnd = fpGetParent(&fpControl);
      // "True parent" for Subclassed controls
      FPCONTROL fpControl = {lpSS->lpBook->hWnd, 0, (long)lpSS->lpOleControl, 
                             (long)lpdispExtendedCtrl};
      fpParent.lpDispatch = fpGetControlDispatch(&fpControl, GC_CONTAINER);                         
      fpParent.hWnd = fpVBGetControlHwnd(&fpParent);
      lpdispExtendedCtrl->Release();
    }
    hhookMouseFilter = ::SetWindowsHookEx(WH_MOUSE, (HOOKPROC)SD_MouseFilterHook, (HINSTANCE)GetWindowWord(fpParent.hWnd, GWW_HINSTANCE),
                                          GetCurrentTask());
    hhookKeyFilter = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SD_KeyFilterHook, (HINSTANCE)GetWindowWord(fpParent.hWnd, GWW_HINSTANCE),
                                          GetCurrentTask());
    // release dispatch now that we have the Parent
    if (fpParent.lpDispatch)
       fpDispatchRelease((LONG)fpParent.lpDispatch);

	if (!sRet)
		return sRet;
	else
//RWP01a

  cds.ghData = gh;  
  ::SendMessage(m_hWndSpreadDsg, WM_COPYDATA, 0, (LPARAM)&cds);
  GlobalFree(gh);
#endif  

//RWP01c
//  return TRUE;
	return sRet;
//RWP01c
}

#ifndef SS_V40
void CSsocxCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    // TODO: Add your message handler code here and/or call default
#if defined(SPREAD_JPN)   
      //-------------------------------------------------------------
      // This preprocessors should be left in. It's simulating
      // Visual Basic 3 for Visual Basic 2.0. A bug was added in
      // the VB JPN version where the 2nd byte of DBCS is removed
      // from the message queue.
      // JPNFIX0004 - (Masanori Iwasa)
      //-------------------------------------------------------------

// BOC FMH 1996.06.27. for BUG028 ------------------->>
    // if left button is pressing, key input will cause sheet black.
// 97' 2/3 Added by BOC Gao. for BUG000634
// When using IME's single char select, it doesn't work.
// I have to cut the following away.
//    if (HIBYTE(GetKeyState(VK_LBUTTON)))
//        return;
//-----------------------<<
     LPSPREADSHEET lpSS;

     lpSS = LockSheet();

     if (SS_USESELBAR(lpSS))     //- When bar selection fall through default.
        COleControl::OnChar(nChar, nRepCnt, nFlags);
     else
        SpreadSheetProc(lpSS->lpBook->hWnd, WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
     UnlockSheet();
     return ;

#else
      COleControl::OnChar(nChar, nRepCnt, nFlags);
      return;
#endif
}
#endif // SS_V40


//--------------------------------------------------------------------
//
//  _AfxShiftState() is a helper fuction which was copied for
//  CTLEVENT.CPP file.
//

short _AfxShiftState()
{
  BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
  BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
  BOOL bAlt   = (GetKeyState(VK_MENU) < 0);

  return (short)(bShift + (bCtrl << 1) + (bAlt << 2));
}

// RFW - 4/28/05 - 16154
void CSsocxCtrl::OnInitMenu(CMenu* pMenu)
{
}

// RFW - 4/28/05 - 16154
void CSsocxCtrl::OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu)
{
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  BOOL fLButtonDown;
  int  iSwapButton;

  // RFW - 1/3/02 - JUW569
  m_fKillFocusReceived = FALSE;

  iSwapButton = GetSystemMetrics(SM_SWAPBUTTON);
  fLButtonDown = (GetAsyncKeyState(iSwapButton ? VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0;

//  if (m_bEventEnabled[EventEnabled_MouseDown])
    FireMouseDown(LEFT_BUTTON, _AfxShiftState(), point.x, point.y);

  if( !::GetCapture() && !m_fKillFocusReceived)
    if (!fLButtonDown || (GetAsyncKeyState(iSwapButton ? VK_RBUTTON : VK_LBUTTON) & 0x8000) != 0)
      CWnd::OnLButtonDown(nFlags, point);

  m_fKillFocusReceived = FALSE;
  if( m_bInPlaceActive && !m_bUIActive && m_bPendingUIActivation &&
      (GetFocus() == this || IsChild(GetFocus())) )
  {
    m_bPendingUIActivation = FALSE;
    OnActivateInPlace(TRUE, NULL);
  }
}


//--------------------------------------------------------------------

void CSsocxCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
#ifdef SPREAD_JPN
// 96' 12/17 Added by BOC Gao. BUG00680
// Show another form as Modal within MouseUp event proc
// cause a re-entrent problem.
ReleaseCapture();
// ---------------------------------<<
#endif
  CWnd::OnLButtonUp(nFlags, point);
//  if (m_bEventEnabled[EventEnabled_MouseUp])
    FireMouseUp(LEFT_BUTTON, _AfxShiftState(), point.x, point.y);
}


//--------------------------------------------------------------------

void CSsocxCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  CWnd::OnLButtonDblClk(nFlags, point);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{
//  if (m_bEventEnabled[EventEnabled_MouseDown])
    FireMouseDown(MIDDLE_BUTTON, _AfxShiftState(), point.x, point.y);
  CWnd::OnMButtonDown(nFlags, point);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnMButtonUp(UINT nFlags, CPoint point)
{
  CWnd::OnMButtonUp(nFlags, point);
//  if (m_bEventEnabled[EventEnabled_MouseUp])
    FireMouseUp(MIDDLE_BUTTON, _AfxShiftState(), point.x, point.y);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnMButtonDblClk(UINT nFlags, CPoint point)
{
  CWnd::OnMButtonDblClk(nFlags, point);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
//  if (m_bEventEnabled[EventEnabled_MouseDown])
    FireMouseDown(RIGHT_BUTTON, _AfxShiftState(), point.x, point.y);
  CWnd::OnRButtonDown(nFlags, point);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
  CWnd::OnRButtonUp(nFlags, point);
//  if (m_bEventEnabled[EventEnabled_MouseUp])
    FireMouseUp(RIGHT_BUTTON, _AfxShiftState(), point.x, point.y);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnRButtonDblClk(UINT nFlags, CPoint point)
{
  CWnd::OnRButtonDblClk(nFlags, point);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
  short iLeft = nFlags & MK_LBUTTON ? LEFT_BUTTON : 0;
  short iRight = nFlags & MK_RBUTTON ? RIGHT_BUTTON : 0;
  short iMiddle = nFlags & MK_MBUTTON ? MIDDLE_BUTTON: 0;
  short iButton = iLeft | iRight | iMiddle;
//  if (m_bEventEnabled[EventEnabled_MouseMove])
    FireMouseMove(iButton, _AfxShiftState(), point.x, point.y);
  CWnd::OnMouseMove(nFlags, point);
}

//--------------------------------------------------------------------
//DBOCX...
void CSsocxCtrl::OnKillFocus(CWnd* pNewWnd)
{
  HWND hWndFocus = NULL;
  CWnd *CWndFocus = GetFocus();

  if (CWndFocus)
     hWndFocus = CWndFocus->m_hWnd;

  if (!hWndFocus || !::IsChild(m_hWnd, hWndFocus))
    m_fKillFocusReceived = TRUE;

  COleControl::OnKillFocus(pNewWnd);
}
//DBOCX.

//DBOCX...
void CSsocxCtrl::OnCancelMode()
{
/*  HWND hWndFocus = NULL;
  CWnd *CWndFocus = GetFocus();

  if (CWndFocus)
     hWndFocus = CWndFocus->m_hWnd;
  LPSPREADSHEET lpSS = LockSheet();
  if (lpSS->lpBook->hWnd)
  {
    SpreadSheetProc(lpSS->lpBook->hWnd, SSM_KILLFOCUSINTERNAL, (WPARAM)hWndFocus, 0L);
    UnlockSheet();
  }
*/
  COleControl::OnCancelMode();
}
//DBOCX.


BOOL CSsocxCtrl::IsSubclassedControl()
{
  return TRUE;
}


//DBOCX...
// For some reason, this isn't defined for WIN16
#ifndef WIN32
#define METHOD_PROLOGUE_EX(theClass, localClass) \
  METHOD_PROLOGUE(theClass, localClass)
#endif


#ifndef FP_ADB
STDMETHODIMP_(ULONG) CSsocxCtrl::XBoundObject::AddRef()
{
  METHOD_MANAGE_STATE(CSsocxCtrl, BoundObject)
//  return (ULONG)pThis->m_lpOwner->ExternalAddRef();
  return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CSsocxCtrl::XBoundObject::Release()
{
  METHOD_MANAGE_STATE(CSsocxCtrl, BoundObject)
//  return (ULONG)pThis->m_lpOwner->ExternalRelease();
  return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CSsocxCtrl::XBoundObject::QueryInterface(REFIID iid, LPVOID FAR *ppvObj)
{
  METHOD_MANAGE_STATE(CSsocxCtrl, BoundObject)
  return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CSsocxCtrl::XBoundObject::OnSourceChanged(DISPID dispid, BOOL fBound, LPBOOL lpfOwnXferOut)
{
  METHOD_MANAGE_STATE(CSsocxCtrl, BoundObject)

  // RFW - 10/18/98 - GIC7004
  if (pThis->m_hWnd == NULL)
     pThis->CreateWindowForSubclassedControl();

  return pThis->m_fpOleBinding.XBound_OnSourceChanged(dispid, 
      fBound, lpfOwnXferOut);
}

STDMETHODIMP CSsocxCtrl::XBoundObject::IsDirty(DISPID dispid)
{
  METHOD_MANAGE_STATE(CSsocxCtrl, BoundObject)
  return pThis->m_fpOleBinding.XBound_IsDirty(dispid); 
}
#endif // not FP_ADB
//DBOCX.


#ifdef __cplusplus
extern "C" {
#endif

BOOL DLLENTRY SSOCXGetMessageReflect(LPSS_BOOK lpBook)
{

  CSsocxCtrl *pObject = (CSsocxCtrl *)lpBook->lpOleControl;
  BOOL bRet = FALSE;
  BOOL bContinue = FALSE;

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CSsocxCtrl))))
      bContinue = TRUE;
  else if (IsWindow((HWND)lpBook->hWnd))
  {
    pObject = (CSsocxCtrl *)CWnd::FromHandle((HWND)lpBook->hWnd);
      if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CSsocxCtrl))))
         bContinue = TRUE;
  }

  if (bContinue)
  {
  if (!pObject->GetAmbientProperty(DISPID_AMBIENT_MESSAGEREFLECT, VT_BOOL, &bRet))
    bRet = FALSE;
  return bRet;
  }

  return bRet;

}

// BJO 02Apr98 JOK4981 - Begin fix
BOOL CSsocxCtrl::SetChangingExtent(BOOL bNewVal)
{
  BOOL bOldVal = m_bChangingExtent;
  m_bChangingExtent = bNewVal;
  return bOldVal;
}
// BJO 02Apr98 JOK4981 - End fix

BOOL FP_API SSOCXSetControlSize(LPFPCONTROL lpObject, long cx, long cy)
{

  CSsocxCtrl *pObject = (CSsocxCtrl *)lpObject->lObject;
  BOOL bRet = FALSE;
  BOOL bContinue = FALSE;

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CSsocxCtrl))))
      bContinue = TRUE;
  else if (IsWindow((HWND)lpObject->hWnd))
  {
    pObject = (CSsocxCtrl *)CWnd::FromHandle((HWND)lpObject->hWnd);
      if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CSsocxCtrl))))
         bContinue = TRUE;
  }

  if (bContinue)
  {
    LPSS_BOOK lpBook = pObject->LockBook();
    lpBook->fInAutoSize = TRUE;
	 /* RFW - 8/5/99
    if (!pObject->GetBorderStyle())
    {
      cx--;
		cy--;
    }
	 */

    // BJO 18Feb97 SEL8129 - Begin fix
    // Spreadsheet can be trapped in an endless loop when calling
    // SetControlSize() with a negative width or height.
    cx = max(cx, 0);
    cy = max(cy, 0);
    // BJO 18Feb97 SEL8129 - End fix

    BOOL bChangingExtentSaved = pObject->SetChangingExtent(FALSE);  // BJO 02Apr98 JOK4981

    bRet = pObject->SetControlSize((int)cx, (int)cy);

    pObject->SetChangingExtent(bChangingExtentSaved);  // BJO 02Apr98 JOK4981

      lpBook->dClientWidth = (short)cx;
      lpBook->dClientHeight = (short)cy;
    if (bRet)
     pObject->InvalidateControl();
    lpBook->fInAutoSize = FALSE;
    pObject->UnlockBook();
  }
  return bRet;

}


LPDISPATCH SS_GetExtendedControl(LPSS_BOOK lpBook)
{
  LPDISPATCH lpdisp = NULL;
  if( lpBook->lpOleControl )
    lpdisp = ((CSsocxCtrl*)lpBook->lpOleControl)->GetExtendedControl();
  return lpdisp;
}


#ifdef __cplusplus
}
#endif

//--------------------------------------------------------------------


#if defined(SS_RSOCX)
BOOL SS_CalcIsDesignTime(LPSS_BOOK lpBook)
{
  return (SS_IsDesignTime(lpBook));
}
#endif


BOOL SS_IsDesignTime(LPSS_BOOK lpBook)
{
  CSsocxCtrl FAR* lpCtrl;
  BOOL bRet = FALSE;

  if( lpBook )
  {
    lpCtrl = (CSsocxCtrl FAR*)lpBook->lpOleControl;
    bRet = !lpCtrl->AmbientUserMode();
  }
  return bRet;
}


void CSsocxCtrl::OnFreezeEvents(BOOL bFreeze)
{
#if FP_ADB
  // Let the Data binding object know that the "Events" status has changed.
#ifdef SS_V70
  LPSPREADSHEET lpSS;
  int i, cnt = GetSheetCount();
  for( i = 0; i < cnt; i++ )
  {
    lpSS = LockSheet(i);
    if( lpSS->pfpOleBinding )
      ((CfpOleBinding*)lpSS->pfpOleBinding)->OnFreezeEvents(bFreeze);
    UnlockSheet(i);
  }
#else
  m_fpOleBinding.OnFreezeEvents(bFreeze);
#endif
#endif

#ifdef FP_OLEDRAG
  // If Drag-drop and we have hWnd
  if (m_OLEDropMode && m_hWnd)
  {
    if (bFreeze) // if freeze, revoke, else register
      ::RevokeDragDrop(m_hWnd);
    else
      ::RegisterDragDrop(m_hWnd, &m_xfpDropTarget);
  }
#endif
  COleControl::OnFreezeEvents(bFreeze);
}

// SCP 7/22/99  Bug:SCS8735
// Use this to release the connection to the DataSource.
//
// NOTE: This is not needed for projects built with VB, only for VC6 projects.
//       For some reason, in VC6 projects, OnSourceChanged(...) is not 
//       being called when the dialog is ending, which in turn, prevents
//       the control from being destroyed.  So, to work around the problem,
//       we insure that the DataSource connection is "freed" by calling
//       OnSourceChanged() ourself when the hWnd is destroyed.  
//
// NOTE #2: OnNcDestroy() works "better" than OnDestroy(), in the 
//       later case, OnDestroy() gets called TWICE if we implement 
//       that function, OnNcDestroy() only gets called once when we
//       implement it.
void CSsocxCtrl::OnNcDestroy(void)
{
  // Insure that we release the connection to the DataSource.
#ifdef FP_ADB  // SCP 7/22/99  Bug:SCS8735 ...>>>
  DataSource *pDataSrc = NULL;
#ifdef SS_V70
  int i, cnt = GetSheetCount();
  LPSPREADSHEET lpSS;
  for( i = 0; i < cnt; i++ )
  {
    lpSS = LockSheet(i);
    if( !lpSS->pfpOleBinding )
    {
      UnlockSheet(i);
      continue;
    }

    HRESULT hr = ((CfpOleBinding*)lpSS->pfpOleBinding)->GetDataSource(&pDataSrc);
    // if we still have a Data Source, then set it to NULL.
    if (hr == S_OK && pDataSrc)
    {
      hr = ((CfpOleBinding*)lpSS->pfpOleBinding)->SetDataSource(NULL);
      pDataSrc->Release();      // GetDataSource() added a reference.
    }
    SSOcxFreeSheetOleBinding(lpSS);
    UnlockSheet(i);
  }
#else
  HRESULT hr = m_fpOleBinding.GetDataSource(&pDataSrc);
  // if we still have a Data Source, then set it to NULL.
  if (hr == S_OK && pDataSrc)
  {
    hr = m_fpOleBinding.SetDataSource(NULL);
    pDataSrc->Release();      // GetDataSource() added a reference.
  }
#endif
#else // not ADB
  BOOL fDummy;
  m_fpOleBinding.XBound_OnSourceChanged(dispidDataSource, FALSE, &fDummy);
#endif         // <<<... SCP 7/22/99  Bug:SCS8735

  // RFW - 5/23/03 - 11929
#ifdef FP_OLEDRAG
  // If Drag-drop and we have hWnd
  if (m_OLEDropMode && m_hWnd)
    ::RevokeDragDrop(m_hWnd);
#endif

  COleControl::OnNcDestroy();
}


#ifdef SS_V30
LONG CSsocxCtrl::OnPrvwmGetInfo(UINT wParam, LONG lParam)
{
LPPRVW_INFO      lpPrvwInfo = (LPPRVW_INFO)lParam;
SS_PRINTPAGECALC PrintPageCalc;
LPSPREADSHEET lpSS = LockSheet();

lpPrvwInfo->Margins.fMarginLeft = lpSS->PrintOptions.pf.fMarginLeft;
lpPrvwInfo->Margins.fMarginTop = lpSS->PrintOptions.pf.fMarginTop;
lpPrvwInfo->Margins.fMarginRight = lpSS->PrintOptions.pf.fMarginRight;
lpPrvwInfo->Margins.fMarginBottom = lpSS->PrintOptions.pf.fMarginBottom;

PrintSpread(SS_PRINT_ACTION_PAGECALC, 0, NULL, 0, &PrintPageCalc);
//Modify by BOC 99.5.17(hyt)-------------------------
//for not restrict end page end when printtype = 3
if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_PAGERANGE)
{
  if (lpSS->PrintOptions.pf.nPageStart == -1)
    lpPrvwInfo->lPageBeg = 1;
  else
    /* RFW - 6/2/04 - 14268
    lpPrvwInfo->lPageBeg = max(1, min(PrintPageCalc.lPageCnt, lpSS->PrintOptions.pf.nPageStart));
    */
    lpPrvwInfo->lPageBeg = max(1, lpSS->PrintOptions.pf.nPageStart);

  if (lpSS->PrintOptions.pf.nPageEnd == -1)
    lpPrvwInfo->lPageEnd = PrintPageCalc.lPageCnt;
  else
    lpPrvwInfo->lPageEnd = min(lpSS->PrintOptions.pf.nPageEnd,  PrintPageCalc.lPageCnt);
}
else
{
   lpPrvwInfo->lPageBeg = 1;
   lpPrvwInfo->lPageEnd = PrintPageCalc.lPageCnt;
}
//---------------------------------------------------
lpPrvwInfo->fPageWidth = PrintPageCalc.fPageWidth;
lpPrvwInfo->fPageHeight = PrintPageCalc.fPageHeight;

UnlockSheet();
return (TRUE);
}


LONG CSsocxCtrl::OnPrvwmPrintPage(UINT wParam, LONG lParam)
{
LPPRVW_PAGE lpPrvwPage = (LPPRVW_PAGE)lParam;

PrintSpread(SS_PRINT_ACTION_PREVIEW, lpPrvwPage->hDC,
            &lpPrvwPage->Rect, lpPrvwPage->lPage, NULL);

return (TRUE);
}

#ifdef FP_ADB
// We received a message posted by FPDB via spread
LONG CSsocxCtrl::OnDBPostMsg(WPARAM wParam, LPARAM lParam)
{
#ifdef SS_V70
  CfpOleBinding * pOleBinding = (CfpOleBinding*)lParam;
  pOleBinding->ForwardMsg(VBM_DATA_POSTMSGTOSELF, wParam, lParam);
#else
  m_fpOleBinding.ForwardMsg(VBM_DATA_POSTMSGTOSELF, wParam, lParam);
#endif
  return TRUE;
}
#endif


#ifdef FP_OLEDRAG
#include "..\sssrc\spread\ssoledrg.cpp"

#include "..\..\..\fplibs\fptools\src\fpdatobj.cpp"
#include "..\..\..\fplibs\fptools\src\gdicopy.cpp"
#endif

#endif // SS_V30

// property categories
#ifdef SS_V40

#define SS_PROPCAT_COLS       0
#define SS_PROPCAT_ROWS       1
#define SS_PROPCAT_SCROLLBARS 2
#define SS_PROPCAT_TEXTTIPS   3
#define SS_PROPCAT_VIRTUAL    4
#define SS_PROPCAT_GRID       5
#define SS_PROPCAT_SELECTION  6

STDMETHODIMP_(ULONG) CSsocxCtrl::XCategorizeProperties::AddRef()
{
  METHOD_MANAGE_STATE(CSsocxCtrl, CategorizeProperties)
  return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CSsocxCtrl::XCategorizeProperties::Release()
{
  METHOD_MANAGE_STATE(CSsocxCtrl, CategorizeProperties)
  return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CSsocxCtrl::XCategorizeProperties::QueryInterface(REFIID iid, LPVOID FAR *ppvObj)
{
  METHOD_MANAGE_STATE(CSsocxCtrl, CategorizeProperties)
  return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CSsocxCtrl::XCategorizeProperties::PropertyToCategory(DISPID dispid, PROPCAT *ppropcat)
{
METHOD_MANAGE_STATE(CSsocxCtrl, CategorizeProperties)

switch(dispid)
	{
	case 35:                   // DataSource
	case 36:                   // DAutoCellTypes
	case 37:                   // DAutoFill
	case 38:                   // DAutoHeadings
	case 39:                   // DAutoSave
	case 40:                   // DAutoSizeCols
	case 43:                   // DInformActiveRowChange
	#ifdef FP_ADB
	case 317:                  // DataMember
	#endif
		*ppropcat = VB_PROPCAT_DATA;
		break;

	case 198:                  // VirtualMaxRows
	case 199:                  // VirtualMode
	case 200:                  // VirtualOverlap
	case 201:                  // VirtualRows
	case 202:                  // VirtualScrollBuffer
		*ppropcat = SS_PROPCAT_VIRTUAL;
		break;

	case 108:                  // ScrollBarExtMode
	case 109:                  // ScrollBarMaxAlign
	case 110:                  // ScrollBars
	case 111:                  // ScrollBarShowMax
	case 279:                  // ScrollBarTrack
	case 205:                  // VScrollSpecial
	case 206:                  // VScrollSpecialType
	case 389:                  // ScrollBarWidth
	case 390:                  // ScrollBarHeight
	case 403:                  // ScrollBarHColor
	case 404:                  // ScrollBarVColor
		*ppropcat = SS_PROPCAT_SCROLLBARS;
		break;

	case 27:                   // ColsFrozen
	case 67:                   // MaxCols
	case 127:                  // StartingColNumber
	case 342:                  // ColHeaderRows
	case 343:                  // ColHeadersAutoTextIndex
	case 344:                  // ColHeadersUserSortIndex
	case 376:                  // ColHeadersAutoText
	case 377:                  // ColHeadersShow
		*ppropcat = SS_PROPCAT_COLS;
		break;

	case 68:                   // MaxRows
	case 107:                  // RowsFrozen
	case 128:                  // StartingRowNumber
	case 340:                  // RowHeaderCols
	case 341:                  // RowHeadersAutoTextIndex
	case 378:                  // RowHeadersAutoText
	case 379:                  // RowHeadersShow
		*ppropcat = SS_PROPCAT_ROWS;
		break;

	case 56:                   // GridColor
	case 57:                   // GridShowHoriz
	case 58:                   // GridShowVert
	case 59:                   // GridSolid
		*ppropcat = SS_PROPCAT_GRID;
		break;

	case 4:                    // AllowCellOverflow
	case 5:                    // AllowDragDrop
	case 7:                    // AllowUserFormulas
	case 8:                    // ArrowsExitEditMode
	case 9:                    // AutoCalc
	case 10:                   // AutoClipboard
	case 13:                   // ButtonDrawMode
	case 46:                   // EditEnterAction
	case 48:                   // EditModePermanent
	case 49:                   // EditModeReplace
	case 71:                   // MoveActiveOnFocus
	case 74:                   // NoBeep
	case 76:                   // OperationMode
	case 96:                   // ProcessTab
	case 97:                   // Protect
	case 100:                  // RestrictRows
	case 99:                   // RestrictCols
	case 281:                  // ClipboardOptions
	case 319:                  // UserColAction
	case 191:                  // UnitType
	case 192:                  // UserResize
	case 216:                  // FormulaSync
	case 291:                  // TwoDigitYearMax
	case 315:                  // OLEDropMode
	case 336:                  // ScriptEnhanced
	case 375:                  // AllowEditOverflow
	case DISPID_ENABLED:       // Enabled
#ifdef SS_V70
   case 397:                  // AllowColMove
   case 398:                  // AllowRowMove
#endif // SS_V70
		*ppropcat = VB_PROPCAT_BEHAVIOR;
		break;

	case 11:                   // AutoSize
	case 55:                   // GrayAreaBackColor
	case 75:                   // NoBorder
	case 98:                   // ReDraw
	case 123:                  // ShadowColor
	case 124:                  // ShadowDark
	case 125:                  // ShadowText
	case 203:                  // VisibleCols
	case 204:                  // VisibleRows
	case 215:                  // BackColorStyle
	case 273:                  // Appearance
	case 289:                  // SelBackColor
	case 290:                  // SelForeColor
	case 321:                  // CellNoteIndicator
	case DISPID_BORDERSTYLE:   // BorderStyle
	case DISPID_FONT:          // Font
#ifdef SS_V70
	case 384:                  // TabStripFont
	case 386:                  // TabStripPolicy
	case 387:                  // TabStripRatio
   case 391:                  // ActiveCellHighlightStyle
#ifdef SS_V80
   case 525:					//AppearanceStyle
   case 533:					//TabEnhancedShape
   case 534:					//CellNoteIndicatorColor
   case 535:					//CellNoteIndicatorShape
   case 538:					//TabStripButtonPolicy
   case 539:					//HighlightHeaders
   case 540:					//HighlightStyle
   case 545:					//HighlightAlphaBlendColor
   case 546:					//HighlightAlphaBlend
   case 558:					//UseVisualStyles
   case 559:					//EnhanceStaticCells
   case 560:					//ScrollBarStyle
#endif
#endif // SS_V70
		*ppropcat = VB_PROPCAT_APPEARANCE;
		break;

	case 277:                  // TextTip
	case 278:                  // TextTipDelay
	case 320:                  // ShowScrollTips
		*ppropcat = SS_PROPCAT_TEXTTIPS;
		break;

	case 6:                    // AllowMultiBlocks
	case 101:                  // RetainSelBlock
	case 116:                  // SelectBlockOptions
		*ppropcat = SS_PROPCAT_SELECTION;
		break;

	default:
		*ppropcat = VB_PROPCAT_MISC;
	}

return S_OK;
}

STDMETHODIMP CSsocxCtrl::XCategorizeProperties::GetCategoryName(PROPCAT propcat, LCID lcid, BSTR *pbstrName)
{
METHOD_MANAGE_STATE(CSsocxCtrl, CategorizeProperties)

#ifdef SPREAD_JPN

switch(propcat)
	{
	case SS_PROPCAT_COLS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("—ñ"));
		break;
	case SS_PROPCAT_ROWS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("s"));
		break;
	case SS_PROPCAT_SCROLLBARS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("½¸Û°ÙÊÞ°"));
		break;
	case SS_PROPCAT_TEXTTIPS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Ã·½ÄÁ¯Ìß"));
		break;
	case SS_PROPCAT_VIRTUAL:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("‰¼‘zÓ°ÄÞ"));
		break;
	case SS_PROPCAT_GRID:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("¸ÞØ¯ÄÞü"));
		break;
	case SS_PROPCAT_SELECTION:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("‘I‘ðÓ°ÄÞ"));
		break;
	default:
		return E_FAIL;
	}

#else

switch(propcat)
	{
	case SS_PROPCAT_COLS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Columns"));
		break;
	case SS_PROPCAT_ROWS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Rows"));
		break;
	case SS_PROPCAT_SCROLLBARS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Scroll Bars"));
		break;
	case SS_PROPCAT_TEXTTIPS:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Text Tips"));
		break;
	case SS_PROPCAT_VIRTUAL:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Virtual"));
		break;
	case SS_PROPCAT_GRID:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Grid"));
		break;
	case SS_PROPCAT_SELECTION:
		*pbstrName = (BSTR)fpSysAllocBSTRFromTString(_T("Selection"));
		break;
	default:
		return E_FAIL;
	}

#endif // SPREAD_JPN

return S_OK;
}

// RFW - 10/28/05 - 17351
HRESULT CSsocxCtrl::OnActivateInPlace(BOOL bUIActivate, LPMSG pMsg)
{
HRESULT hRes;

if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
	m_fIgnoreKeyDown = TRUE;

hRes = COleControl::OnActivateInPlace(bUIActivate, pMsg);
m_fIgnoreKeyDown = FALSE;

return hRes;
}


void CSsocxCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// RFW - 10/28/05 - 17351
   if (!m_fIgnoreKeyDown)
   {
		HWND hWndSave = m_hWnd;
		USHORT nCharShort = (USHORT)nChar;
		if (m_bEventEnabled[EventEnabled_KeyDown])
			COleControl::KeyDown(&nCharShort);
		if ((m_hWnd == hWndSave) && (nCharShort != 0))
			DefWindowProc(WM_KEYDOWN, nCharShort, MAKELONG(nRepCnt, nFlags));
   }
}

void CSsocxCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HWND hWndSave = m_hWnd;
	USHORT nCharShort = (USHORT)nChar;
	if (m_bEventEnabled[EventEnabled_KeyUp])
		COleControl::KeyUp(&nCharShort);
	if ((m_hWnd == hWndSave) && (nCharShort != 0))
		DefWindowProc(WM_KEYUP, nCharShort, MAKELONG(nRepCnt, nFlags));
}


void CSsocxCtrl::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HWND hWndSave = m_hWnd;
	USHORT nCharShort = (USHORT)nChar;
	if (m_bEventEnabled[EventEnabled_KeyDown])
		COleControl::KeyDown(&nCharShort);
	if ((m_hWnd == hWndSave) && (nCharShort != 0))
		DefWindowProc(WM_SYSKEYDOWN, nCharShort, MAKELONG(nRepCnt, nFlags));
}

void CSsocxCtrl::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HWND hWndSave = m_hWnd;
	USHORT nCharShort = (USHORT)nChar;
	if (m_bEventEnabled[EventEnabled_KeyUp])
		COleControl::KeyUp(&nCharShort);
	if ((m_hWnd == hWndSave) && (nCharShort != 0))
		DefWindowProc(WM_SYSKEYUP, nCharShort, MAKELONG(nRepCnt, nFlags));
}

AFX_STATIC void AFXAPI _AfxPostTrailByte(CWnd* pWnd, BYTE bTrailByte)
{
	// Force new trail byte to the front of the queue.
	pWnd->PostMessage(WM_QUEUE_SENTINEL);
	pWnd->PostMessage(WM_CHAR, bTrailByte);
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOYIELD | PM_REMOVE) &&
		(msg.message != WM_QUEUE_SENTINEL))
	{
		::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	}

	ASSERT(msg.message == WM_QUEUE_SENTINEL);
	ASSERT(msg.hwnd == pWnd->m_hWnd);
}

#define STOCKEVENT_KEYPRESS         0x00000008

void CSsocxCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	USHORT nCharShort = (USHORT)nChar;
	USHORT nCharSave = nCharShort;
	BOOL bLeadByte = IsDBCSLeadByte((BYTE)nCharShort);
	MSG msg;

#if defined(SPREAD_JPN)   
     LPSPREADSHEET lpSS;

     lpSS = LockSheet();

     if (!SS_USESELBAR(lpSS))     //- When bar selection fall through default.
     {
        SpreadSheetProc(lpSS->lpBook->hWnd, WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
        UnlockSheet();
        return ;
     }
     UnlockSheet();
#endif // SPREAD_JPN

	if (GetStockEventMask() & STOCKEVENT_KEYPRESS)
	{
		if (bLeadByte)
		{
			// We have the lead-byte of a DBCS character.  Peek for the
			// next WM_CHAR message, which will contain the other byte.

			BOOL bMessage;
			VERIFY(bMessage = ::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
				PM_NOYIELD | PM_NOREMOVE));

			// Combine the bytes to form the DBCS character.

			if (bMessage)
				nCharShort = (USHORT)((nCharShort << 8) | msg.wParam);
		}

		HWND hWndSave = m_hWnd;
		nCharSave = nCharShort;
      if (m_bEventEnabled[EventEnabled_KeyPress])
      {
		  COleControl::FireKeyPress(&nCharShort);

		  // If handler set nCharShort to zero, cancel further processing.
		  if (nCharShort != 0)
		    	OnKeyPressEvent(nCharShort);
      }
		if (m_hWnd != hWndSave)
			return;
	}

	if (nCharShort != 0)
	{
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

					VERIFY(::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
						PM_NOYIELD | PM_REMOVE));
					_AfxPostTrailByte(this, LOBYTE(nCharShort));
					nChar = HIBYTE(nCharShort);
				}
				else
				{
					// Event handler changed character from DBCS to SBCS:
					// Remove the second byte from the queue, and forward
					// along the new single-byte character.

					VERIFY(::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
						PM_NOYIELD | PM_REMOVE));
				}
			}
			else
			{
				if (bNewLeadByte)
				{
					// Event handler changed character from SBCS to DBCS:
					// Post the new trail byte.

					_AfxPostTrailByte(this, LOBYTE(nCharShort));
					nChar = HIBYTE(nCharShort);
				}
			}
		}

		DefWindowProc(WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
	}

/*
	if (bLeadByte)
	{
		// Cleanup after processing a DBCS character:
		// Remove the next WM_CHAR message (containing the second byte) from
		// the message queue, UNLESS we're subclassing an Edit, ListBox, or
		// ComboBox control.

		TCHAR szClassName[10];
		if ((!::GetClassName(m_hWnd, szClassName, 10)) ||  // didn't get class
			(lstrcmpi(szClassName, _T("Edit")) &&           // not Edit
			 lstrcmpi(szClassName, _T("ListBox")) &&        // not ListBox
			 lstrcmpi(szClassName, _T("ComboBox"))))        // not ComboBox
		{
			VERIFY(::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR,
				PM_NOYIELD | PM_REMOVE));
		}
	}
*/
}

#endif // SS_V40

LPSPREADSHEET CSsocxCtrl::LockSheet()
{
//  return (LPSPREADSHEET)tbGlobalLock (m_hBook);
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = SS_BookLockSheet(lpBook);
  UnlockBook();
  return lpSS;
}

LPSPREADSHEET CSsocxCtrl::LockActiveSheet()
{
//  return (LPSPREADSHEET)tbGlobalLock (m_hBook);
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
  UnlockBook();
  return lpSS;
}

LPSPREADSHEET CSsocxCtrl::LockSheet(int SheetNum)
{
//  return (LPSPREADSHEET)tbGlobalLock (m_hBook);
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, SheetNum);
  UnlockBook();
  return lpSS;
}

LPSPREADSHEET CSsocxCtrl::LockSheetSendingMsg()
{
//  return (LPSPREADSHEET)tbGlobalLock (m_hBook);
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheetSendingMsg);
  UnlockBook();
  return lpSS;
}

void CSsocxCtrl::UnlockSheet()
{
#ifdef SS_NO_USE_SH
  LPSS_BOOK lpBook = LockBook();
  SS_BookUnlockSheet(lpBook);
  UnlockBook();
#endif
}

void CSsocxCtrl::UnlockActiveSheet()
{
#ifdef SS_NO_USE_SH
  LPSS_BOOK lpBook = LockBook();
  SS_BookUnlockActiveSheet(lpBook);
  UnlockBook();
#endif
}

void CSsocxCtrl::UnlockSheet(int SheetNum)
{
#ifdef SS_NO_USE_SH
  LPSS_BOOK lpBook = LockBook();
  SS_BookUnlockSheetIndex(lpBook, SheetNum);
  UnlockBook();
#endif
}

void CSsocxCtrl::UnlockSheetSendingMsg()
{
#ifdef SS_NO_USE_SH
  LPSS_BOOK lpBook = LockBook();
  SS_BookUnlockSheetIndex(lpBook, lpBook->nSheetSendingMsg);
  UnlockBook();
#endif
}

LPSS_BOOK CSsocxCtrl::LockBook()
{
  return (LPSS_BOOK)tbGlobalLock (m_hBook);
}

void CSsocxCtrl::UnlockBook()
{
  tbGlobalUnlock (m_hBook);
}

#if SS_V80
void CSsocxCtrl::DoSuperclassPaint(CDC* pDC, const CRect& rcBounds)
{
   LPSS_BOOK lpBook = LockBook();
   
   lpBook->fWmPrintSkipScrollBars = TRUE;
   COleControl::DoSuperclassPaint(pDC, rcBounds);
   lpBook->fWmPrintSkipScrollBars = FALSE;
   UnlockBook();
}
#endif
