// ssocxctl.h : Declaration of the CSsocxCtrl OLE control class.

#include "..\..\..\fplibs\fpdb\src\fpoledb.h"                                           //DBOCX
#ifdef WIN32
#include <objsafe.h>
#endif

#ifdef FP_OLEDRAG
#include "..\..\..\fplibs\fptools\src\fpdatobj.h"
#endif

// property categories -scl
#ifdef SS_V40
#include "propcat.h"
#endif

#if SS_V80
#include "ctappear.h"
#endif

// 96' 4/17 BOC Gao. Using when DragMode is Auto.
#ifdef WIN32
#define         OLESTR(str)         L##str
#else
#define         OLESTR(str)         str
#define         LPOLESTR            LPSTR
#endif
// --------------------------------------------<<

#define EDITERROR_CELLLOCKED       1
#define EDITERROR_COLRESTRICTED    2
#define EDITERROR_ROWRESTRICTED    3
#define EDITERROR_COLROWRESTRICTED 4
#define EDITERROR_INVALIDDATA      5
#define EDITERROR_INVALIDPASTEDATA 6
#define EDITERROR_TYPINGPROHIBITED 7
#define EDITERROR_EDITMODEINS      8
#define EDITERROR_CHECKBOXTYPING   9
#define EDITERROR_INVALIDCOLTYPE   10

#define EDITERROR_DB_DAO_BEGINUPDATE_ADD_FAILED      100
#define EDITERROR_DB_DAO_BEGINUPDATE_UPDATE_FAILED   101   
#define EDITERROR_DB_DAO_DELETE_FAILED               102   
#define EDITERROR_DB_DAO_SETCOLUMN_FAILED            103  
#define EDITERROR_DB_DAO_UPDATE_FAILED               104  
#define EDITERROR_DB_SETVALUE_FAILED                 105   
#define EDITERROR_DB_CURRENTPOSITIONINVALID          106   
#define EDITERROR_DB_MEMORYALLOCATION                107   
#define EDITERROR_DB_NOTSUPPORTED                    108   
#define EDITERROR_DB_NONUPDATABLE_RECORDSET          109   
#define EDITERROR_DB_UPDATEINPROGRESS                110  
#define EDITERROR_DB_MOVETONEWROW_FAILED             111
#define EDITERROR_DB_OPERATION_FAILED                112

enum
{
  EventEnabled_AllEvents,          // 0
  EventEnabled_Advance,            // 1
  EventEnabled_AfterUserSort,      // 2
  EventEnabled_BeforeUserSort,     // 3
  EventEnabled_BlockSelected,      // 4
  EventEnabled_ButtonClicked,      // 5
  EventEnabled_Change,             // 6
  EventEnabled_Click,              // 7
  EventEnabled_ColWidthChange,     // 8
  EventEnabled_ComboCloseUp,       // 9
  EventEnabled_ComboDropDown,      // 10
  EventEnabled_ComboSelChange,     // 11
  EventEnabled_CustomFunction,     // 12
  EventEnabled_DataColConfig,      // 13
  EventEnabled_DataFill,           // 14
  EventEnabled_DblClick,           // 15
  EventEnabled_DragDropBlock,      // 16
  EventEnabled_DrawItem,           // 17
  EventEnabled_EditChange,         // 18
  EventEnabled_EditError,          // 19
  EventEnabled_EditMode,           // 20
  EventEnabled_EnterRow,           // 21
  EventEnabled_KeyDown,            // 22
  EventEnabled_KeyPress,           // 23
  EventEnabled_KeyUp,              // 24
  EventEnabled_LeaveCell,          // 25
  EventEnabled_LeaveRow,           // 26
  EventEnabled_OLECompleteDrag,    // 27
  EventEnabled_OLEDragDrop,        // 28
  EventEnabled_OLEDragOver,        // 29
  EventEnabled_OLEGiveFeedback,    // 30
  EventEnabled_OLESetData,         // 31
  EventEnabled_OLEStartDrag,       // 32
  EventEnabled_PrintAbort,         // 33
  EventEnabled_PrintMsgBox,        // 34
  EventEnabled_QueryAdvance,       // 35
  EventEnabled_QueryData,          // 36
  EventEnabled_RightClick,         // 37
  EventEnabled_RowHeightChange,    // 38
  EventEnabled_SelChange,          // 39
  EventEnabled_TextTipFetch,       // 40
  EventEnabled_TopLeftChange,      // 41
  EventEnabled_UserFormulaEntered, // 42
  EventEnabled_VirtualClearData,   // 43
#ifdef SS_V70
  EventEnabled_SheetChanging,      // 44
  EventEnabled_SheetChanged,       // 45
  EventEnabled_TabScrolling,       // 46
  EventEnabled_TabScrolled,        // 47
  EventEnabled_BeforeEditMode,     // 48
  EventEnabled_DataLoaded,         // 49
  EventEnabled_BeforeScrollTip,    // 50
  EventEnabled_BeforeColMove,      // 51
  EventEnabled_BeforeRowMove,      // 52
#endif // SS_V70
#ifdef SS_V80
  EventEnabled_CircularFormula,    // 53
  EventEnabled_CTCancelEditing,           // 54
  EventEnabled_CTCreateEditorControl,     // 55
  EventEnabled_CTCreateRendererControl,   // 56
  EventEnabled_CTGetEditorValue,          // 57
  EventEnabled_CTInitializeControl,       // 58
  EventEnabled_CTIsReservedKey,           // 59
  EventEnabled_CTIsReservedLocation,      // 60
  EventEnabled_CTIsValid,                 // 61
  EventEnabled_CTGetPreferredSize,        // 62
  EventEnabled_CTGetReservedCursor,       // 63
  EventEnabled_CTPaintCell,               // 64
  EventEnabled_CTSetValue,                // 65
  EventEnabled_CTStartEditing,            // 66
  EventEnabled_CTStopEditing,             // 67
  EventEnabled_CTStringFormat,            // 68
  EventEnabled_CTStringUnformat,          // 69
#endif
  EVENTENABLED_CNT                        // 70
};


// Spread Designer
typedef struct ss_buffer {
  GLOBALHANDLE g;
  LONG     l;
} SS_BUFFER;

#ifndef WIN32
typedef struct copydata {
  LONG lData;
  GLOBALHANDLE ghData;
} COPYDATASTRUCT; 
#endif
// Spread Designer

#if defined(SS_SPREADOCX)

  // List of all events that should not be included
  #define SSE_NO_ARROWPRESSED
  #define SSE_NO_INSDEL
  #define SSE_NO_LINKDISPATCHNOTCREATED
  #define SSE_NO_LINKERROR
  #define SSE_NO_LINKITEMNOTSUPPORTED
  #define SSE_NO_LINKITEMSUPPORTED
  #define SSE_NO_LINKNEWDATA
  #define SSE_NO_LINKNOTIFY
  #define SSE_NO_LINKOUTOFMEMORY
  #define SSE_NO_LINKPOKECOMPLETED
  #define SSE_NO_LINKREQUESTCOMPLETED
  #define SSE_NO_LINKSERVERDISCONNECTED
  #define SSE_NO_LINKUNABLETOCONNECTTOSERVER
  // List of all functions that should not be included
  #define SSF_NO_DdeQueryLink
  #define SSF_NO_DdeAddLink
  #define SSF_NO_DdeRemoveLink
  #define SSF_NO_DdeRequestLink
  #define SSF_NO_DdePokeLink
  #define SSF_NO_DdePokeDataToLink
  #define SSF_NO_DdeGetMode
  #define SSF_NO_DdeSetMode
  #define SSF_NO_DdeGetReadExpr
  #define SSF_NO_DdeSetReadExpr
  #define SSF_NO_DdeGetWriteExpr
  #define SSF_NO_DdeSetWriteExpr
  #define SSF_NO_DdeGetLink
  #define SSF_NO_DdeSetLink
  #define SSF_NO_DdeQueryDependCell
  #define SSF_NO_DdeQueryDependCol
  #define SSF_NO_DdeQueryDependRow
  #define SSF_NO_DdeQueryDependName
  #define SSF_NO_DdeGetDim
  #define SSF_NO_DdeIsLink

#elif  defined(SS_RSOCX)

  // List of all events that should not be included
  #define SSE_NO_ARROWPRESSED
  #define SSE_NO_INSDEL

#elif defined(SS_GRIDOCX)

  // List of all properties that should not be included
  #define SSP_NO_ALLOWCELLOVERFLOW
  #define SSP_NO_ALLOWDRAGDROP
  #define SSP_NO_ALLOWMULTIBLOCKS
  #define SSP_NO_ALLOWUSERFORMULAS
//  #define SSP_NO_ARROWSEXITEDITMODE
  #define SSP_NO_AUTOCALC
//  #define SSP_NO_AUTOCLIPBOARD
  #define SSP_NO_AUTOSIZE
//  #define SSP_NO_BACKCOLORSTYLE
  #define SSP_NO_BUTTONDRAWMODE
  #define SSP_NO_CALCDEPENDENCIES
  #define SSP_NO_CELLBORDERCOLOR
  #define SSP_NO_CELLBORDERSTYLE
  #define SSP_NO_CELLBORDERTYPE
//  #define SSP_NO_CLIPVALUE
//  #define SSP_NO_COLHEADERDISPLAY
  #define SSP_NO_COLPAGEBREAK
//  #define SSP_NO_COLSFROZEN
  #define SSP_NO_CURSORICON
  #define SSP_NO_CURSORSTYLE
  #define SSP_NO_CURSORTYPE
  #define SSP_NO_DATAFILLEVENT
  #define SSP_NO_DAUTOCELLTYPES
  #define SSP_NO_DAUTOFILL
  #define SSP_NO_DAUTOHEADINGS
  #define SSP_NO_DESTCOL
  #define SSP_NO_DESTROW
//  #define SSP_NO_EDITENTERACTION
  #define SSP_NO_FILENUM
//  #define SSP_NO_FLOATDEFCURRENCYCHAR
//  #define SSP_NO_FLOATDEFDECIMALCHAR
//  #define SSP_NO_FLOATDEFSEPCHAR
  #define SSP_NO_FORMULA
  #define SSP_NO_FORMULASYNC
  #define SSP_NO_GRAYAREABACKCOLOR
  #define SSP_NO_HDCPRINTER
//  #define SSP_NO_LOCKBACKCOLOR
//  #define SSP_NO_LOCKFORECOLOR
  #define SSP_NO_MAXTEXTCELLHEIGHT
  #define SSP_NO_MAXTEXTCELLWIDTH
  #define SSP_NO_MAXTEXTCOLWIDTH
  #define SSP_NO_MAXTEXTROWHEIGHT
  #define SSP_NO_MULTISELCOUNT
  #define SSP_NO_MULTISELINDEX
//  #define SSP_NO_NOBEEP
//  #define SSP_NO_NOBORDER
  #define SSP_NO_OPERATIONMODE
  #define SSP_NO_APPEARANCESTYLE
#define SSP_NO_SCROLLBARSTYLE
#define SSP_NO_USEVISUALSTYLES
  #define SSP_NO_TABENHANCEDSHAPE
  #define SSP_NO_HIGHLIGHTHEADERS
  #define SSP_NO_HIGHLIGHTSTYLE
  #define SSP_NO_HIGHLIGHTALPHABLENDCOLOR
  #define SSP_NO_HIGHLIGHTALPHABLEND
  #define SSP_NO_POSITION
  #define SSP_NO_PRINTABORTMSG
  #define SSP_NO_PRINTBORDER
  #define SSP_NO_PRINTCOLHEADERS
  #define SSP_NO_PRINTCOLOR
  #define SSP_NO_PRINTFOOTER
  #define SSP_NO_PRINTGRID
  #define SSP_NO_PRINTHEADER
  #define SSP_NO_PRINTJOBNAME
  #define SSP_NO_PRINTMARGINBOTTOM
  #define SSP_NO_PRINTMARGINLEFT
  #define SSP_NO_PRINTMARGINRIGHT
  #define SSP_NO_PRINTMARGINTOP
  #define SSP_NO_PRINTORIENTATION
  #define SSP_NO_PRINTPAGEEND
  #define SSP_NO_PRINTPAGESTART
  #define SSP_NO_PRINTROWHEADERS
  #define SSP_NO_PRINTSHADOWS
  #define SSP_NO_PRINTTYPE
  #define SSP_NO_PRINTUSEDATAMAX
  #define SSP_NO_PROTECT
  #define SSP_NO_RESTRICTCOLS
  #define SSP_NO_RESTRICTROWS
//  #define SSP_NO_RETAINSELBLOCK
//  #define SSP_NO_ROWHEADERDISPLAY
  #define SSP_NO_ROWPAGEBREAK
//  #define SSP_NO_ROWSFROZEN
//  #define SSP_NO_SCROLLBAREXTMODE
  #define SSP_NO_SCROLLBARMAXALIGN
  #define SSP_NO_SCROLLBARSHOWMAX
  #define SSP_NO_SELLENGTH
  #define SSP_NO_SELMODEINDEX
  #define SSP_NO_SELMODESELCOUNT
  #define SSP_NO_SELMODESELECTED
  #define SSP_NO_SELSTART
  #define SSP_NO_SELTEXT
  #define SSP_NO_SORTBY
  #define SSP_NO_SORTKEY
  #define SSP_NO_SORTKEYORDER
//  #define SSP_NO_STARTINGCOLNUMBER
//  #define SSP_NO_STARTINGROWNUMBER
  #define SSP_NO_TYPEBUTTONBORDERCOLOR
  #define SSP_NO_TYPEBUTTONDARKCOLOR
  #define SSP_NO_TYPEBUTTONLIGHTCOLOR
  #define SSP_NO_TYPEBUTTONSHADOWSIZE
//  #define SSP_NO_TYPECHECKTEXTALIGN
//  #define SSP_NO_TYPECOMBOBOXCOUNT
//  #define SSP_NO_TYPECOMBOBOXEDITABLE
//  #define SSP_NO_TYPECOMBOBOXINDEX
//  #define SSP_NO_TYPECOMBOBOXSTRING
//  #define SSP_NO_TYPEDATESEPARATOR
//  #define SSP_NO_TYPEEDITCHARCASE
//  #define SSP_NO_TYPEEDITCHARSET
  #define SSP_NO_TYPEEDITMULTILINE
//  #define SSP_NO_TYPEEDITPASSWORD
//  #define SSP_NO_TYPEFLOATCURRENCYCHAR
//  #define SSP_NO_TYPEFLOATDECIMALCHAR
//  #define SSP_NO_TYPEFLOATSEPCHAR
  #define SSP_NO_TYPEINTEGERMAX
  #define SSP_NO_TYPEINTEGERMIN
  #define SSP_NO_TYPEINTEGERSPININC
  #define SSP_NO_TYPEINTEGERSPINWRAP
  #define SSP_NO_TYPEOWNERDRAWSTYLE
  #define SSP_NO_TYPESPIN
//  #define SSP_NO_TYPETEXTPREFIX
//  #define SSP_NO_TYPETEXTSHADOWIN
//  #define SSP_NO_TYPETIMESEPARATOR
//  #define SSP_NO_USERRESIZECOL
//  #define SSP_NO_USERRESIZEROW
  #define SSP_NO_UTOPIA
  #define SSP_NO_VIRTUALCURROWCOUNT
  #define SSP_NO_VIRTUALCURTOP
  #define SSP_NO_VIRTUALOVERLAP
  #define SSP_NO_VIRTUALROWS
  #define SSP_NO_VIRTUALSCROLLBUFFER
  #define SSP_NO_VISIBLEROWS
  #define SSP_NO_VISIBLECOLS
//  #define SSP_NO_VSCROLLSPECIAL
//  #define SSP_NO_VSCROLLSPECIALTYPE

  // List of all events that should not be included
  #define SSE_NO_ARROWPRESSED
//  #define SSE_NO_COLWIDTHCHANGE
  #define SSE_NO_CUSTOMFUNCTION
  #define SSE_NO_DRAGDROPBLOCK
  #define SSE_NO_DRAWITEM
  #define SSE_NO_ENTERROW
  #define SSE_NO_INSDEL
  #define SSE_NO_LEAVEROW
  #define SSE_NO_LINKDISPATCHNOTCREATED
  #define SSE_NO_LINKERROR
  #define SSE_NO_LINKITEMNOTSUPPORTED
  #define SSE_NO_LINKITEMSUPPORTED
  #define SSE_NO_LINKNEWDATA
  #define SSE_NO_LINKNOTIFY
  #define SSE_NO_LINKOUTOFMEMORY
  #define SSE_NO_LINKPOKECOMPLETED
  #define SSE_NO_LINKREQUESTCOMPLETED
  #define SSE_NO_LINKSERVERDISCONNECTED
  #define SSE_NO_LINKUNABLETOCONNECTTOSERVER
  #define SSE_NO_PRINTABORT
  #define SSE_NO_PRINTMSGBOX
  #define SSE_NO_QUERYADVANCE
//  #define SSE_NO_RIGHTCLICK
//  #define SSE_NO_ROWHEIGHTCHANGE
//  #define SSE_NO_TOPLEFTCHANGE
  #define SSE_NO_USERFORMULAENTERED

  // List of all functions that should not be included
  #define SSF_NO_AddCustomFunction
  #define SSF_NO_AddCustomFunctionExt
  #define SSF_NO_CFGetCellParam
  #define SSF_NO_CFGetDoubleParam
  #define SSF_NO_CFGetDoubleParamExt
  #define SSF_NO_CFGetLongParam
  #define SSF_NO_CFGetParamInfo
  #define SSF_NO_CFGetRangeParam
  #define SSF_NO_CFGetStringParam
  #define SSF_NO_CFSetResult
//  #define SSF_NO_ColNumberToLetter
//  #define SSF_NO_ColWidthToTwips
  #define SSF_NO_GetBottomRightCell
  #define SSF_NO_GetCellFromScreenCoord
  #define SSF_NO_GetCellPos
  #define SSF_NO_GetClientArea
  #define SSF_NO_GetColItemData
  #define SSF_NO_GetCurrSelBlockPos
  #define SSF_NO_GetCustomName
  #define SSF_NO_GetFirstValidCell
  #define SSF_NO_GetItemData
  #define SSF_NO_GetIteration
  #define SSF_NO_GetLastValidCell
  #define SSF_NO_GetMultiSelItem
  #define SSF_NO_GetRefStyle
  #define SSF_NO_GetRowItemData
  #define SSF_NO_IsCellSelected
  #define SSF_NO_IsFormulaValid
  #define SSF_NO_IsVisible
  #define SSF_NO_QueryCustomName
//  #define SSF_NO_RowHeightToTwips
  #define SSF_NO_SetColItemData
  #define SSF_NO_SetCustomName
  #define SSF_NO_SetItemData
  #define SSF_NO_SetIteration
  #define SSF_NO_SetRefStyle
  #define SSF_NO_SetRowItemData
//  #define SSF_NO_TwipsToColWidth
//  #define SSF_NO_TwipsToRowHeight

  // List of all actions that should not be included
  #define SSA_NO_Goto
  #define SSA_NO_Load
  #define SSA_NO_SaveAll
  #define SSA_NO_SaveValues
  #define SSA_NO_Recalc
  #define SSA_NO_Print
  #define SSA_NO_SetBorder
  #define SSA_NO_AddMultiSelBlock
  #define SSA_NO_GetMultiSelBlocks
  #define SSA_NO_CopyRange
  #define SSA_NO_MoveRange
  #define SSA_NO_SwapRange
//  #define SSA_NO_ClipboardCopy
//  #define SSA_NO_ClipboardCut
//  #define SSA_NO_ClipboardPaste
  #define SSA_NO_Sort
//  #define SSA_NO_ComboClear
//  #define SSA_NO_ComboRemoveString
  #define SSA_NO_SelModeClear
  #define SSA_NO_SmartPrint

#endif

/////////////////////////////////////////////////////////////////////////////
// CSsocxCtrl : See ssocxctl.cpp for implementation.

class CSsocxCtrl : public COleControl
{
  DECLARE_DYNCREATE(CSsocxCtrl)

// Constructor
public:
  CSsocxCtrl();

// BJO 10Nov97 GRB5822 - Begin fix
// Copied from \VC5\MFC\SRC\AFXCTL.H
#if SS_V80
//   BOOL MyCreateControlWindow(HWND hWndParent, const CRect& rcPos, LPCRECT prcClipped);
#endif
#ifdef WIN32
	BOOL CreateControlWindow(HWND hWndParent, const CRect& rcPos, LPCRECT prcClipped = NULL);
#endif
// BJO 10Nov97 GRB5822 - End fix

// Overrides
    // Functions related to "OnVerb" (Visible/Invisible support)
#ifndef WIN32  //used only for WIN16
    HRESULT MyOnActivateInPlace(BOOL bUIActivate, LPMSG pMsg);
    BOOL MyCreateControlWindow(HWND hWndParent, const CRect& rcPos);
  BOOL OnVerbUiActivate(LPMSG lpMsg, HWND, LPCRECT lprcPosRect);
  BOOL OnVerbInPlaceActivate(LPMSG lpMsg, HWND, LPCRECT lprcPosRect);
  BOOL OnVerbPrimary(LPMSG lpMsg, HWND, LPCRECT lprcPosRect);
#endif

#ifdef SS_V40
  HRESULT OnActivateInPlace(BOOL bUIActivate, LPMSG pMsg);
#endif

//void FireMouseMove(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
//	void AFX_CDECL FireEvent(DISPID dispid, BYTE* pbParams, ...);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

  // Drawing function
  virtual DWORD GetControlFlags();
  virtual void OnDrawMetafile(CDC* pdc, const CRect& rcBounds);
  virtual void OnDraw(
        CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
  virtual void OnNcPaint(void);

    // Persistence
  virtual void DoPropExchange(CPropExchange* pPX);

    // Reset control state
  virtual void OnResetState();

    // Client Site is set
  virtual void OnSetClientSite();

  // Properties verb has been invoked
  virtual BOOL OnProperties(LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);

    // Receive notification when the control is being deleted
  virtual void OnEventAdvise(BOOL bAdvise);

  virtual void OnFreezeEvents(BOOL bFreeze);

#ifdef SS_V35
  virtual void OnKeyUpEvent( USHORT nChar, USHORT nShiftState );
  virtual void OnKeyDownEvent( USHORT nChar, USHORT nShiftState );
  virtual void OnKeyPressEvent( USHORT nChar );
#endif

#ifdef SS_V40
  virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  virtual void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  virtual void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  virtual void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
#endif // SS_V40

// Implementation
protected:
  virtual BOOL IsSubclassedControl();
  ~CSsocxCtrl();
  BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  //virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

  BEGIN_OLEFACTORY(CSsocxCtrl)        // Class factory and guid
    virtual BOOL VerifyUserLicense();
    virtual BOOL GetLicenseKey(DWORD, BSTR FAR*);
// CODESENT  
    virtual BOOL VerifyLicenseKey(BSTR bstrKey);
  END_OLEFACTORY(CSsocxCtrl)

  DECLARE_OLETYPELIB(CSsocxCtrl)      // GetTypeInfo
#ifndef SS_V30
  DECLARE_PROPPAGEIDS(CSsocxCtrl)     // Property page IDs
#endif
  DECLARE_OLECTLTYPE(CSsocxCtrl)        // Type name and misc status


  //DBOCX...
public:
  // INTERFACE for DataBinding
  // Following creates member variable:  XBoundObject m_xBoundObject;
#ifndef FP_ADB  // not ADB
  BEGIN_INTERFACE_PART(BoundObject, IBoundObject)
    INIT_INTERFACE_PART(CSsocxCtrl, BoundObject)
    STDMETHOD(OnSourceChanged)(DISPID, BOOL, LPBOOL);
    STDMETHOD(IsDirty)(DISPID);
  END_INTERFACE_PART(BoundObject)
#endif // not ADB
  //DBOCX.

// property categories -scl
#ifdef SS_V40
  BEGIN_INTERFACE_PART(CategorizeProperties, ICategorizeProperties)
    INIT_INTERFACE_PART(CSsocxCtrl, CategorizeProperties)
	STDMETHOD(PropertyToCategory)(DISPID dispid, PROPCAT *ppropcat);
	STDMETHOD(GetCategoryName)(PROPCAT propcat, LCID lcid, BSTR *pbstrName);
  END_INTERFACE_PART(CategorizeProperties)
#endif

#ifdef SS_V70
// IPropertyNotifySink for font updates (not exposed via QueryInterface)
  BEGIN_INTERFACE_PART(TabFontNotification, IPropertyNotifySink)
	INIT_INTERFACE_PART(CSsocxCtrl, TabFontNotification)
	STDMETHOD(OnChanged)(DISPID dispid);
	STDMETHOD(OnRequestEdit)(DISPID dispid);
  END_INTERFACE_PART_STATIC(TabFontNotification)
#endif // SS_V70

#ifdef FP_OLEDRAG
#define FPMYDISP_CTRL_NAME CSsocxCtrl
#include "..\sssrc\spread\ssoledrg.h"
#endif

#ifdef WIN32
	// IPersistPropertyBag
  BEGIN_INTERFACE_PART(PersistPropertyBag, IPersistPropertyBag)
    INIT_INTERFACE_PART(COleControl, PersistPropertyBag)
    STDMETHOD(GetClassID)(LPCLSID);
    STDMETHOD(InitNew)();
    STDMETHOD(Load)(LPPROPERTYBAG, LPERRORLOG);
    STDMETHOD(Save)(LPPROPERTYBAG, BOOL, BOOL);
  END_INTERFACE_PART(PersistPropertyBag)

	  // IQuickActivate
  BEGIN_INTERFACE_PART(QuickActivate, IQuickActivate)
 	STDMETHOD(QuickActivate)(QACONTAINER *pQAContainer, QACONTROL *pqacontrol);
	STDMETHOD(SetContentExtent)(LPSIZEL lpsizel);
	STDMETHOD(GetContentExtent)(LPSIZEL lpsizel);
  END_INTERFACE_PART(QuickActivate)

  BEGIN_INTERFACE_PART(ObjectSafety, IObjectSafety)
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD __RPC_FAR *pdwSupportedOptions, DWORD __RPC_FAR *pdwEnabledOptions);
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);
  END_INTERFACE_PART(ObjectSafety)
#endif

   DECLARE_INTERFACE_MAP();

public:
#if !defined(SS_V70) || !defined(FP_ADB)
    CfpOleBinding  m_fpOleBinding;      // Ole DB Binding as
                                        // implemented by FarPoint (fpoledb.h)
#endif
    TBGLOBALHANDLE m_hVBSpread;         // Handle of memory
    LPVOID         m_lpVBSpread;        // ptr to "locked" memory

    BOOL PrivateSetTypePictPicture(SS_COORD lCol, SS_COORD lRow, 
      HBITMAP hBitmap, HPALETTE hPal, HICON hIcon);
    LONG OnSsmPostDataChange(UINT wParam, LONG lParam);  // in SSOCXDB.C
    LONG OnSsmPostAddNew(UINT wParam, LONG lParam);      // in SSOCXDB.C
    LONG OnSsmBoundClick(UINT wParam, LONG lParam);      // in SSOCXDB.C
    void SSCreateWindowForSubclassedCtrl(void);          // in SSOCXDB.C

protected:
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg void OnCancelMode(void);
  afx_msg void OnNcDestroy(void);  // SCP 7/22/99  Bug:SCS8735
  afx_msg void OnInitMenu(CMenu* pMenu);
  afx_msg void OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu);

public:
  BOOL SetChangingExtent(BOOL bNewVal);  // BJO 04Apr98 JOK4981

public:  // ???? remove this
  BOOL        m_bFirstPaint;
  CFontHolder m_fontSS;
#ifdef SS_V70
  CFontHolder m_fontTab;
#endif // SS_V70
  TBGLOBALHANDLE m_hBook;
  SS_COORD m_lCol;
  SS_COORD m_lRow;
  SS_COORD m_lCol2;
  SS_COORD m_lRow2;
  SS_COORD m_lDestCol;
  SS_COORD m_lDestRow;
  BOOL m_bBlockMode;
  BOOL m_bCalcDependencies;
  LPPICTURE m_lppictCursorIcon;
  LPPICTURE m_lppictCursors[10];
  HCURSOR m_hcurCursors[10];
  short m_nCursorType;
  short m_nFileNum;
  GLOBALHANDLE m_hMultiSelBlocks;
  short m_nMultiSelCount;
  short m_nMultiSelIndex;
  short m_nPosition;
  short m_nSortBy;
  HGLOBAL m_hSortKeys;
  short m_nSortKeyCnt;
  short m_nCellBorderStyle;
  short m_nCellBorderType;
  COLORREF m_crCellBorderColor;
  short m_nCellType;
  HDC m_hDCPrinter;
#ifdef SS_V30
  long m_lPrintNextPageBreakCol;
  long m_lPrintNextPageBreakRow;
#endif
  LPSS_VALUE m_lpCFResult;
  LPSS_VALUE m_lpCFArgs;
  short m_nCFArgCnt;
  short m_nTypeButtonAlign;
  OLE_COLOR m_crTypeButtonBorderColor;
  OLE_COLOR m_crTypeButtonColor;
  OLE_COLOR m_crTypeButtonDarkColor;
  OLE_COLOR m_crTypeButtonLightColor;
  LPPICTURE m_lppictTypeButtonPicture;
  LPPICTURE m_lppictTypeButtonPictureDown;
  short m_nTypeButtonShadowSize;
  CString m_sTypeButtonText;
  OLE_COLOR m_crTypeButtonTextColor;
  short m_nTypeButtonType;
  BOOL m_bTypeCheckCenter;
  LPPICTURE m_lppictTypeCheckPicture[6];
  short m_nTypeCheckPictureIndex;
  CString m_sTypeCheckText;
  short m_nTypeCheckTextAlign;
  short m_nTypeCheckType;
  //short m_nTypeComboBoxCount;
  //short m_nTypeComboBoxCurSel;
  BOOL m_bTypeComboBoxEditable;
  short m_nTypeComboBoxIndex;
  CString m_sTypeComboBoxList;
  //CString m_sTypeComboBoxString;
  BOOL m_bTypeDateCentury;
  short m_nTypeDateFormat;
  TB_DATE m_TypeDateMax;
  TB_DATE m_TypeDateMin;
  short m_nTypeDateSeparator;
  short m_nTypeEditCharCase;
  short m_nTypeEditCharSet;
  long m_lTypeEditLen;
  BOOL m_bTypeEditMultiLine;
  BOOL m_bTypeEditPassword;
  short m_nTypeFloatCurrencyChar;
  short m_nTypeFloatDecimalChar;
  short m_nTypeFloatDecimalPlaces;
  double m_dfTypeFloatMax;
  double m_dfTypeFloatMin;
  BOOL m_bTypeFloatMoney;
  BOOL m_bTypeFloatSeparator;
  short m_nTypeFloatSepChar;
  short m_nTypeHAlign;
  short m_nTypeVAlign;
  long m_lTypeIntegerMax;
  long m_lTypeIntegerMin;
  long m_lTypeIntegerSpinInc;
  BOOL m_bTypeIntegerSpinWrap;
  long m_lTypeOwnerDrawStyle;
  CString m_sTypePicDefaultText;
  CString m_sTypePicMask;
  BOOL m_bTypePictCenter;
  BOOL m_bTypePictMaintainScale;
  LPPICTURE m_lppictTypePictPicture;
  BOOL m_bTypePictStretch;
  BOOL m_bTypeSpin;
  short m_nTypeTextAlignVert;
  BOOL m_bTypeTextPrefix;
  BOOL m_bTypeTextShadow;
  BOOL m_bTypeTextShadowIn;
  BOOL m_bTypeTextWordWrap;
  short m_nTypeTime24Hour;
  TIMEFORMAT m_TypeTimeFormat;
  TIME m_TypeTimeMax;
  TIME m_TypeTimeMin;
  BOOL m_bTypeTimeSeconds;
  short m_nTypeTimeSeparator;
#ifdef SS_V30
  short m_nTypeComboMaxDrop;
  short m_nTypeComboWidth;
  short m_nTypeComboMaxEditLen;
  HWND  m_nTypeCombohWnd;
  short m_nTextTip;
  BYTE  fTextTipSet;
  long  m_lTextTipDelay;
  BYTE  fTextTipDelaySet;
#endif
  HWND m_hWndPrintDlgProc;
  CString m_strDataField;                       //DBOCX
// Spread Designer
  HWND m_hWndSpreadDsg; 
  LPVOID m_lpSDDialog;
  BOOL m_fSafeForScripting;
// Spread Designer

#ifdef SS_V35
  BOOL m_bTypeEllipses;
  short m_nShowScrollTips;
  short m_nCellNoteIndicator;
#endif

#ifdef SS_V40
  BYTE m_bTypeComboAutoSearch;
  SS_CELLTYPE m_TypeCurrency;
  SS_CELLTYPE m_TypeNumber;
  SS_CELLTYPE m_TypePercent;
  short m_nTypeTextOrient;
  BOOL m_fTypeNegRed;
#endif
#ifdef SS_V70
  SS_CELLTYPE m_TypeScientific;
#endif // SS_V70
#ifdef SS_V80
  CString m_sTypeCustomName;
  long m_lTypeCustomStyle;
  VARIANT m_TypeCustomItemData;
#endif // SS_V80
  BYTE m_bEventEnabled[EVENTENABLED_CNT];

  BOOL m_fKillFocusReceived;
  BOOL m_fInsideCreateControlWindow;
  HACCEL m_hAccel;
  USHORT m_cAccel;
  BOOL   m_fIsVBMyEnvironment;
  BOOL   m_fIgnoreKeyDown;
#ifdef SPREAD_JPN
  BOOL m_bFiring;
#endif

  //TCHAR m_szPrinterDevice[200];
  //TCHAR m_szPrinterDriver[100];
  //TCHAR m_szPrinterOutput[100];
  //DEVMODE m_PrinterDevMode;

  // Property sheet
  virtual LPCLSID GetPropPageIDs(ULONG& cPropPages);

  virtual void OnGetControlInfo(LPCONTROLINFO pControlInfo);
  virtual void OnMnemonic(LPMSG lpMsg);
  virtual BOOL PreTranslateMessage(LPMSG lpMsg);
  virtual void OnFinalRelease();

//  LPCSTR FAR* DispToEnum(DISPID dispid, int FAR* lpnMax);
//  virtual BOOL OnGetDisplayString(DISPID dispid, CString& strValue);
#ifdef FP_ADB
  virtual BOOL OnGetPredefinedStrings(DISPID dispid,
                                      CStringArray* pStringArray,
                                      CDWordArray* pCookieArray);
  virtual BOOL OnGetPredefinedValue(DISPID dispid, DWORD dwCookie,
                                    VARIANT FAR* lpvarOut);
#endif

#ifdef SS_V70
  virtual void OnTabFontChanged();
#endif // SS_V70

  //void fpContainerCoordsToPixels(long FAR *lpxPixels, long FAR *lpyPixels,
  //                               long x, long y, BOOL fSize);
  //void fpPixelsToContainerCoords(long FAR *lpxCoord, long FAR *lpyCoord,
  //                               long x, long y, BOOL fSize);
  void InitOcx(void);
  void DestroyPicts(void);
  void SetActionPrint(void);
  void PrintSpread(WORD wAction, HDC hDCPreview, LPRECT lpRectPreview,
                   long lXtra, LPSS_PRINTPAGECALC lpPrintPageCalc);
  LPSPREADSHEET LockSheet(void);
  LPSPREADSHEET LockActiveSheet(void);
  LPSPREADSHEET LockSheet(int SheetNum);
  LPSPREADSHEET LockSheetSendingMsg(void);
  void UnlockSheet(void);
  void UnlockActiveSheet(void);
  void UnlockSheet(int SheetNum);
  void UnlockSheetSendingMsg(void);
  LPSS_BOOK LockBook(void);
  void UnlockBook(void);

  virtual void OnFontChanged();
  BOOL GetType(LPSS_CELLTYPE lpCellType);
  BOOL GetTypeOf(LPSS_CELLTYPE lpCellType, BYTE nType);
  BOOL SetType(DISPID dispid);
  BOOL SetTypeButton(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeCheck(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeCombo(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeDate(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeEdit(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeFloat(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
#ifdef SS_V40
  BOOL SetTypeCurrency(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeNumber(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypePercent(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
#endif // SS_V40
#ifdef SS_V70
  BOOL SetTypeScientific(LPSPREADSHEET lpSS, DISPID dispid,
                         LPSS_CELLTYPE lpCellTypeOld,
                         LPSS_CELLTYPE lpCellTypeNew);
#endif // SS_V70
#if SS_V80
  BOOL SetTypeCustom(LPSPREADSHEET lpSS, DISPID dispid,
                              LPSS_CELLTYPE lpCellTypeOld,
                              LPSS_CELLTYPE lpCellTypeNew);
#endif
  #ifndef SS_NOCT_INT
  BOOL SetTypeInteger(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  #endif
  #ifndef SS_NOCT_OWNERDRAW
  BOOL SetTypeOwner(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  #endif
  BOOL SetTypePic(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypePict(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeStatic(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);
  BOOL SetTypeTime(LPSPREADSHEET lpSS, DISPID dispid,
                      LPSS_CELLTYPE lpCellTypeOld,
                      LPSS_CELLTYPE lpCellTypeNew);

  afx_msg LONG OnSsmCellLocked(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmClick(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmColRestricted(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmColRowRestricted(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmColWidthChange(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmDataChange(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmDblClk(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmDragDrop(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmDrawItem(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmEditModeOff(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmEditModeOn(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmEnterRow(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmFireEventPrintAbort(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmFireEventPrintMsgClose(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmFireEventPrintMsgOpen(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmKeyDown(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmKeyPress(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmKeyUp(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLeaveCell(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLeaveRow(UINT wParam, LONG lParam);
  #if defined(SS_RSOCX)
  afx_msg LONG OnSsmLinkDispatchNotCreated(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkError(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkItemNotSupported(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkItemSupported(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkNewData(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkNotify(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkOutOfMemory(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkPokeCompleted(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkRequestCompleted(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkServerDisconnected(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmLinkUnableToConnectToServer(UINT wParam, LONG lParam);
  #endif // SS_RSOCX
  afx_msg LONG OnSsmRButton(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmRowHeightChange(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmRowRestricted(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmVClearData(UINT wParam, LONG lparam);
  afx_msg LONG OnSsmVQueryData(UINT wParam, LONG lParam);
#ifdef SS_V30
  afx_msg LONG OnSsmComboCloseUp(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmComboDropDown(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmComboSelChange(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmTextTipFetch(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmEditChange(UINT wParam, LONG lParam);
  afx_msg LONG OnPrvwmGetInfo(UINT wParam, LONG lParam);
  afx_msg LONG OnPrvwmPrintPage(UINT wParam, LONG lParam);
#endif

#ifdef SS_V35
  afx_msg LONG OnSsmBeforeUserSort(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmAfterUserSort(UINT wParam, LONG lParam);
#endif

#ifdef SS_V70
  afx_msg LONG OnSsmSheetChanging(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmSheetChanged(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmTabScrolling(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmTabScrolled(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmBeforeEditMode(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmBeforeScrollTip(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmBeforeColMove(UINT wParam, LONG lParam);
  afx_msg LONG OnSsmBeforeRowMove(UINT wParam, LONG lParam);
#endif // SS_V70

#ifdef SS_V80
  afx_msg LONG OnSsmCircularFormula(UINT wParam, LONG lParam);
#endif

  afx_msg void OnSsnBlockChanged(UINT id);
  afx_msg void OnSsnBlockSelected(UINT id);
  afx_msg void OnSsnButtonDown(UINT id);
  afx_msg void OnSsnButtonUp(UINT id);
  afx_msg void OnSsnCheckBoxTyping(UINT id);
  afx_msg void OnSsnEditModeIns(UINT id);
  afx_msg void OnSsnExitNext(UINT id);
  afx_msg void OnSsnExitPrev(UINT id);
  afx_msg void OnSsnInvalidColType(UINT id);
  afx_msg void OnSsnInvalidData(UINT id);
  afx_msg void OnSsnInvalidPasteData(UINT id);
  afx_msg void OnSsnLeftColChange(UINT id);
  afx_msg void OnSsnTopLeftChange(UINT id);
  afx_msg void OnSsnTopRowChange(UINT id);
  afx_msg void OnSsnTypingProhibited(UINT id);
  afx_msg void OnSsnUserFormula(UINT id);
// Spread Designer
  afx_msg BOOL OnSpreadDesigner(LPMSG lpmsg, HWND hWnd, LPCRECT lpcrect);
  afx_msg LONG OnSpreadDsgExit(UINT wParam, LONG lParam);
  afx_msg LONG OnSpreadDsgApply(UINT wParam, LONG lParam);
  afx_msg LONG OnSpreadDsgHereIAm(WPARAM wParam, LPARAM lParam);
  afx_msg LONG OnCopyData(WPARAM wParam, LPARAM lParam);
  afx_msg LONG OnGetColumnNames(WPARAM wParam, LPARAM lParam);
// Spread Designer

#ifdef FP_ADB
  afx_msg LONG OnDBPostMsg(WPARAM wParam, LPARAM lParam);
#endif

  // Subclassed control support
  BOOL PreCreateWindow(CREATESTRUCT& cs);
  WNDPROC* GetSuperWndProcAddr(void);
  LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

  BOOL CreateFont(CFontHolder *font, LPLOGFONT lpLogFont);

// Message maps
  //{{AFX_MSG(CSsocxCtrl)
#ifndef SS_V40
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
#endif
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:

// Dispatch maps
  //{{AFX_DISPATCH(CSsocxCtrl)
    //}}AFX_DISPATCH
  #ifndef SSP_NO_ACTION
    afx_msg void SetAction(short nNewValue);
  #endif
  #ifndef SSP_NO_ACTIVECOL
    afx_msg long GetActiveCol();
  #endif
  #ifndef SSP_NO_ACTIVEROW
    afx_msg long GetActiveRow();
  #endif
  #ifndef SSP_NO_ALLOWCELLOVERFLOW
    afx_msg BOOL GetAllowCellOverflow();
    afx_msg void SetAllowCellOverflow(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_ALLOWDRAGDROP
    afx_msg BOOL GetAllowDragDrop();
    afx_msg void SetAllowDragDrop(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_ALLOWMULTIBLOCKS
    afx_msg BOOL GetAllowMultiBlocks();
    afx_msg void SetAllowMultiBlocks(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_ALLOWUSERFORMULAS
    afx_msg BOOL GetAllowUserFormulas();
    afx_msg void SetAllowUserFormulas(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_ARROWSEXITEDITMODE
    afx_msg BOOL GetArrowsExitEditMode();
    afx_msg void SetArrowsExitEditMode(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_AUTOCALC
    afx_msg BOOL GetAutoCalc();
    afx_msg void SetAutoCalc(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_AUTOCLIPBOARD
    afx_msg BOOL GetAutoClipboard();
    afx_msg void SetAutoClipboard(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_AUTOSIZE
    afx_msg BOOL GetAutoSize();
    afx_msg void SetAutoSize(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_BUTTONDRAWMODE
#ifdef SS_V70
    afx_msg long GetButtonDrawMode();
    afx_msg void SetButtonDrawMode(long lNewValue);
#else
    afx_msg short GetButtonDrawMode();
    afx_msg void SetButtonDrawMode(short nNewValue);
#endif
  #endif
  #ifndef SSP_NO_BLOCKMODE
    afx_msg BOOL GetBlockMode();
    afx_msg void SetBlockMode(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_CALCDEPENDENCIES
    afx_msg BOOL GetCalcDependencies();
    afx_msg void SetCalcDependencies(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_CELLBORDERCOLOR
    afx_msg OLE_COLOR GetCellBorderColor();
    afx_msg void SetCellBorderColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_CELLBORDERSTYLE
    afx_msg short GetCellBorderStyle();
    afx_msg void SetCellBorderStyle(short nNewValue);
  #endif
  #ifndef SSP_NO_CELLBORDERTYPE
    afx_msg short GetCellBorderType();
    afx_msg void SetCellBorderType(short nNewValue);
  #endif
  #ifndef SSP_NO_CELLTYPE
    afx_msg short GetCellType();
    afx_msg void SetCellType(short nNewValue);
  #endif
  #ifndef SSP_NO_CHANGEMADE
    afx_msg BOOL GetChangeMade();
    afx_msg void SetChangeMade(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_CLIP
    afx_msg BSTR GetClip();
    afx_msg void SetClip(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_CLIPVALUE
    afx_msg BSTR GetClipValue();
    afx_msg void SetClipValue(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_COL
    afx_msg long GetCol();
    afx_msg void SetCol(long nNewValue);
  #endif
  #ifndef SSP_NO_COL2
    afx_msg long GetCol2();
    afx_msg void SetCol2(long nNewValue);
  #endif
  #ifndef SSP_NO_COLHEADERDISPLAY
    afx_msg short GetColHeaderDisplay();
    afx_msg void SetColHeaderDisplay(short nNewValue);
  #endif
  #ifndef SSP_NO_COLHIDDEN
    afx_msg BOOL GetColHidden();
    afx_msg void SetColHidden(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_COLPAGEBREAK
    afx_msg BOOL GetColPageBreak();
    afx_msg void SetColPageBreak(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_COLSFROZEN
    afx_msg long GetColsFrozen();
    afx_msg void SetColsFrozen(long nNewValue);
  #endif
  #ifndef SSP_NO_CURSORICON
    afx_msg LPPICTUREDISP GetCursorIcon();
    afx_msg void SetCursorIcon(LPPICTUREDISP newValue);
  #endif
  #ifndef SSP_NO_CURSORSTYLE
    afx_msg short GetCursorStyle();
    afx_msg void SetCursorStyle(short nNewValue);
  #endif
  #ifndef SSP_NO_CURSORTYPE
    afx_msg short GetCursorType();
    afx_msg void SetCursorType(short nNewValue);
  #endif
  #ifndef SSP_NO_DATACOLCNT
    afx_msg long GetDataColCnt();
  #endif
//DBOCX...
  #ifndef SSP_NO_DATASOURCE
    #ifdef FP_ADB
      afx_msg DataSource *GetDataSource();
      afx_msg void SetDataSource(DataSource *newValue);
      afx_msg BSTR GetDataMember();
      afx_msg void SetDataMember(LPCTSTR lpszNewValue);
    #else // not FP_ADB
      afx_msg LPUNKNOWN GetDataSource();
      afx_msg void SetDataSource(LPUNKNOWN newValue);
    #endif
  #endif

//DBOCX.
  #ifndef SSP_NO_DATAFIELD
    afx_msg BSTR GetDataField();
    afx_msg void SetDataField(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_DATAFILLEVENT
    afx_msg BOOL GetDataFillEvent();
    afx_msg void SetDataFillEvent(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_DATAROWCNT
    afx_msg long GetDataRowCnt();
  #endif
  #ifndef SSP_NO_DAUTOCELLTYPES
    afx_msg BOOL GetDAutoCellTypes();
    afx_msg void SetDAutoCellTypes(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_DAUTOFILL
    afx_msg BOOL GetDAutoFill();
    afx_msg void SetDAutoFill(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_DAUTOHEADINGS
    afx_msg BOOL GetDAutoHeadings();
    afx_msg void SetDAutoHeadings(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_DAUTOSAVE
    afx_msg BOOL GetDAutoSave();
    afx_msg void SetDAutoSave(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_DAUTOSIZECOLS
    afx_msg short GetDAutoSizeCols();                          //DBOCX
    afx_msg void SetDAutoSizeCols(short sNewValue);            //DBOCX
  #endif
  #ifndef SSP_NO_DESTCOL
    afx_msg long GetDestCol();
    afx_msg void SetDestCol(long nNewValue);
  #endif
  #ifndef SSP_NO_DESTROW
    afx_msg long GetDestRow();
    afx_msg void SetDestRow(long nNewValue);
  #endif
  #ifndef SSP_NO_DINFORMACTIVEROWCHANGE
    afx_msg BOOL GetDInformActiveRowChange();
    afx_msg void SetDInformActiveRowChange(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_DISPLAYCOLHEADERS
    afx_msg BOOL GetDisplayColHeaders();
    afx_msg void SetDisplayColHeaders(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_DISPLAYROWHEADERS
    afx_msg BOOL GetDisplayRowHeaders();
    afx_msg void SetDisplayRowHeaders(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_EDITENTERACTION
    afx_msg short GetEditEnterAction();
    afx_msg void SetEditEnterAction(short nNewValue);
  #endif
  #ifndef SSP_NO_EDITMODE
    afx_msg BOOL GetEditMode();
    afx_msg void SetEditMode(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_EDITMODEPERMANENT
    afx_msg BOOL GetEditModePermanent();
    afx_msg void SetEditModePermanent(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_EDITMODEREPLACE
    afx_msg BOOL GetEditModeReplace();
    afx_msg void SetEditModeReplace(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_FILENUM
    afx_msg short GetFileNum();
    afx_msg void SetFileNum(short nNewValue);
  #endif
  #ifndef SSP_NO_FLOATDEFCURRENCYCHAR
    afx_msg short GetFloatDefCurrencyChar();
    afx_msg void SetFloatDefCurrencyChar(short nNewValue);
  #endif
  #ifndef SSP_NO_FLOATDEFDECIMALCHAR
    afx_msg short GetFloatDefDecimalChar();
    afx_msg void SetFloatDefDecimalChar(short nNewValue);
  #endif
  #ifndef SSP_NO_FLOATDEFSEPCHAR
    afx_msg short GetFloatDefSepChar();
    afx_msg void SetFloatDefSepChar(short nNewValue);
  #endif
  #ifndef SSP_NO_FORMULA
    afx_msg BSTR GetFormula();
    afx_msg void SetFormula(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_GRAYAREABACKCOLOR
    afx_msg OLE_COLOR GetGrayAreaBackColor();
    afx_msg void SetGrayAreaBackColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_GRIDCOLOR
    afx_msg OLE_COLOR GetGridColor();
    afx_msg void SetGridColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_GRIDSHOWHORIZ
    afx_msg BOOL GetGridShowHoriz();
    afx_msg void SetGridShowHoriz(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_GRIDSHOWVERT
    afx_msg BOOL GetGridShowVert();
    afx_msg void SetGridShowVert(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_GRIDSOLID
    afx_msg BOOL GetGridSolid();
    afx_msg void SetGridSolid(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_HDCPRINTER
    afx_msg OLE_HANDLE GetHDCPrinter();
    afx_msg void SetHDCPrinter(OLE_HANDLE newValue);
  #endif
  #ifndef SSP_NO_ISBLOCKSELECTED
    afx_msg BOOL GetIsBlockSelected();
  #endif
  #ifndef SSP_NO_LEFTCOL
    afx_msg long GetLeftCol();
    afx_msg void SetLeftCol(long nNewValue);
  #endif
  #ifndef SSP_NO_LOCK
    afx_msg BOOL GetLock();
    afx_msg void SetLock(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_LOCKBACKCOLOR
    afx_msg OLE_COLOR GetLockBackColor();
    afx_msg void SetLockBackColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_LOCKFORECOLOR
    afx_msg OLE_COLOR GetLockForeColor();
    afx_msg void SetLockForeColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_MAXCOLS
    afx_msg long GetMaxCols();
    afx_msg void SetMaxCols(long nNewValue);
  #endif
  #ifndef SSP_NO_MAXROWS
    afx_msg long GetMaxRows();
    afx_msg void SetMaxRows(long nNewValue);
  #endif
  #ifndef SSP_NO_MAXTEXTCELLHEIGHT
    afx_msg double GetMaxTextCellHeight();
  #endif
  #ifndef SSP_NO_MAXTEXTCELLWIDTH
    afx_msg double GetMaxTextCellWidth();
  #endif
  #ifndef SSP_NO_MOVEACTIVEONFOCUS
    afx_msg BOOL GetMoveActiveOnFocus();
    afx_msg void SetMoveActiveOnFocus(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_MULTISELCOUNT
    afx_msg long GetMultiSelCount();
  #endif
  #ifndef SSP_NO_MULTISELINDEX
    afx_msg long GetMultiSelIndex();
    afx_msg void SetMultiSelIndex(long nNewValue);
  #endif
  #ifndef SSP_NO_NOBEEP
    afx_msg BOOL GetNoBeep();
    afx_msg void SetNoBeep(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_NOBORDER
    afx_msg BOOL GetNoBorder();
    afx_msg void SetNoBorder(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_OPERATIONMODE
    afx_msg short GetOperationMode();
    afx_msg void SetOperationMode(short nNewValue);
  #endif
  #ifndef SSP_NO_POSITION
    afx_msg short GetPosition();
    afx_msg void SetPosition(short nNewValue);
  #endif
  #ifndef SSP_NO_PRINTABORTMSG
    afx_msg BSTR GetPrintAbortMsg();
    afx_msg void SetPrintAbortMsg(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_PRINTBORDER
    afx_msg BOOL GetPrintBorder();
    afx_msg void SetPrintBorder(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PRINTCOLHEADERS
    afx_msg BOOL GetPrintColHeaders();
    afx_msg void SetPrintColHeaders(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PRINTCOLOR
    afx_msg BOOL GetPrintColor();
    afx_msg void SetPrintColor(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PRINTFOOTER
    afx_msg BSTR GetPrintFooter();
    afx_msg void SetPrintFooter(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_PRINTGRID
    afx_msg BOOL GetPrintGrid();
    afx_msg void SetPrintGrid(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PRINTHEADER
    afx_msg BSTR GetPrintHeader();
    afx_msg void SetPrintHeader(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_PRINTJOBNAME
    afx_msg BSTR GetPrintJobName();
    afx_msg void SetPrintJobName(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_PRINTMARGINBOTTOM
    afx_msg long GetPrintMarginBottom();
    afx_msg void SetPrintMarginBottom(long nNewValue);
  #endif
  #ifndef SSP_NO_PRINTMARGINLEFT
    afx_msg long GetPrintMarginLeft();
    afx_msg void SetPrintMarginLeft(long nNewValue);
  #endif
  #ifndef SSP_NO_PRINTMARGINRIGHT
    afx_msg long GetPrintMarginRight();
    afx_msg void SetPrintMarginRight(long nNewValue);
  #endif
  #ifndef SSP_NO_PRINTMARGINTOP
    afx_msg long GetPrintMarginTop();
    afx_msg void SetPrintMarginTop(long nNewValue);
  #endif
  #ifndef SSP_NO_PRINTORIENTATION
    afx_msg short GetPrintOrientation();
    afx_msg void SetPrintOrientation(short nNewValue);
  #endif
  #ifndef SSP_NO_PRINTPAGEEND
    afx_msg short GetPrintPageEnd();
    afx_msg void SetPrintPageEnd(short nNewValue);
  #endif
  #ifndef SSP_NO_PRINTPAGESTART
    afx_msg short GetPrintPageStart();
    afx_msg void SetPrintPageStart(short nNewValue);
  #endif
  #ifndef SSP_NO_PRINTROWHEADERS
    afx_msg BOOL GetPrintRowHeaders();
    afx_msg void SetPrintRowHeaders(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PRINTSHADOWS
    afx_msg BOOL GetPrintShadows();
    afx_msg void SetPrintShadows(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PRINTTYPE
    afx_msg short GetPrintType();
    afx_msg void SetPrintType(short nNewValue);
  #endif
  #ifndef SSP_NO_PRINTUSEDATAMAX
    afx_msg BOOL GetPrintUseDataMax();
    afx_msg void SetPrintUseDataMax(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PROCESSTAB
    afx_msg BOOL GetProcessTab();
    afx_msg void SetProcessTab(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_PROTECT
    afx_msg BOOL GetProtect();
    afx_msg void SetProtect(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_REDRAW
    afx_msg BOOL GetReDraw();
    afx_msg void SetReDraw(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_RESTRICTCOLS
    afx_msg BOOL GetRestrictCols();
    afx_msg void SetRestrictCols(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_RESTRICTROWS
    afx_msg BOOL GetRestrictRows();
    afx_msg void SetRestrictRows(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_RETAINSELBLOCK
    afx_msg BOOL GetRetainSelBlock();
    afx_msg void SetRetainSelBlock(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_ROW
    afx_msg long GetRow();
    afx_msg void SetRow(long nNewValue);
  #endif
  #ifndef SSP_NO_ROW2
    afx_msg long GetRow2();
    afx_msg void SetRow2(long nNewValue);
  #endif
  #ifndef SSP_NO_ROWHEADERDISPLAY
    afx_msg short GetRowHeaderDisplay();
    afx_msg void SetRowHeaderDisplay(short nNewValue);
  #endif
  #ifndef SSP_NO_ROWHIDDEN
    afx_msg BOOL GetRowHidden();
    afx_msg void SetRowHidden(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_ROWPAGEBREAK
    afx_msg BOOL GetRowPageBreak();
    afx_msg void SetRowPageBreak(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_ROWSFROZEN
    afx_msg long GetRowsFrozen();
    afx_msg void SetRowsFrozen(long nNewValue);
  #endif
  #ifndef SSP_NO_SCROLLBAREXTMODE
    afx_msg BOOL GetScrollBarExtMode();
    afx_msg void SetScrollBarExtMode(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_SCROLLBARMAXALIGN
    afx_msg BOOL GetScrollBarMaxAlign();
    afx_msg void SetScrollBarMaxAlign(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_SCROLLBARS
    afx_msg short GetScrollBars();
    afx_msg void SetScrollBars(short nNewValue);
  #endif
  #ifndef SSP_NO_SCROLLBARSHOWMAX
    afx_msg BOOL GetScrollBarShowMax();
    afx_msg void SetScrollBarShowMax(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_SELBLOCKCOL
    afx_msg long GetSelBlockCol();
  #endif
  #ifndef SSP_NO_SELBLOCKCOL2
    afx_msg long GetSelBlockCol2();
  #endif
  #ifndef SSP_NO_SELBLOCKROW
    afx_msg long GetSelBlockRow();
  #endif
  #ifndef SSP_NO_SELBLOCKROW2
    afx_msg long GetSelBlockRow2();
  #endif
  #ifndef SSP_NO_SELECTBLOCKOPTIONS
#ifdef SS_V70
    afx_msg long GetSelectBlockOptions();
    afx_msg void SetSelectBlockOptions(long lNewValue);
#else
    afx_msg short GetSelectBlockOptions();
    afx_msg void SetSelectBlockOptions(short nNewValue);
#endif
  #endif
  #ifndef SSP_NO_SELLENGTH
    afx_msg long GetSelLength();
    afx_msg void SetSelLength(long nNewValue);
  #endif
  #ifndef SSP_NO_SELMODEINDEX
    afx_msg long GetSelModeIndex();
    afx_msg void SetSelModeIndex(long nNewValue);
  #endif
  #ifndef SSP_NO_SELMODESELCOUNT
    afx_msg long GetSelModeSelCount();
  #endif
  #ifndef SSP_NO_SELMODESELECTED
    afx_msg BOOL GetSelModeSelected();
    afx_msg void SetSelModeSelected(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_SELSTART
    afx_msg long GetSelStart();
    afx_msg void SetSelStart(long nNewValue);
  #endif
  #ifndef SSP_NO_SELTEXT
    afx_msg BSTR GetSelText();
    afx_msg void SetSelText(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_SHADOWCOLOR
    afx_msg OLE_COLOR GetShadowColor();
    afx_msg void SetShadowColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_SHADOWDARK
    afx_msg OLE_COLOR GetShadowDark();
    afx_msg void SetShadowDark(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_SHADOWTEXT
    afx_msg OLE_COLOR GetShadowText();
    afx_msg void SetShadowText(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_SORTBY
    afx_msg short GetSortBy();
    afx_msg void SetSortBy(short nNewValue);
  #endif
  #ifndef SSP_NO_STARTINGCOLNUMBER
    afx_msg long GetStartingColNumber();
    afx_msg void SetStartingColNumber(long nNewValue);
  #endif
  #ifndef SSP_NO_STARTINGROWNUMBER
    afx_msg long GetStartingRowNumber();
    afx_msg void SetStartingRowNumber(long nNewValue);
  #endif
  #ifndef SSP_NO_TOPROW
    afx_msg long GetTopRow();
    afx_msg void SetTopRow(long nNewValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONALIGN
    afx_msg short GetTypeButtonAlign();
    afx_msg void SetTypeButtonAlign(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONBORDERCOLOR
    afx_msg OLE_COLOR GetTypeButtonBorderColor();
    afx_msg void SetTypeButtonBorderColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONCOLOR
    afx_msg OLE_COLOR GetTypeButtonColor();
    afx_msg void SetTypeButtonColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONDARKCOLOR
    afx_msg OLE_COLOR GetTypeButtonDarkColor();
    afx_msg void SetTypeButtonDarkColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONLIGHTCOLOR
    afx_msg OLE_COLOR GetTypeButtonLightColor();
    afx_msg void SetTypeButtonLightColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONPICTURE
    afx_msg LPPICTUREDISP GetTypeButtonPicture();
    afx_msg void SetTypeButtonPicture(LPPICTUREDISP newValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONPICTUREDOWN
    afx_msg LPPICTUREDISP GetTypeButtonPictureDown();
    afx_msg void SetTypeButtonPictureDown(LPPICTUREDISP newValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONSHADOWSIZE
    afx_msg short GetTypeButtonShadowSize();
    afx_msg void SetTypeButtonShadowSize(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONTEXT
    afx_msg BSTR GetTypeButtonText();
    afx_msg void SetTypeButtonText(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONTEXTCOLOR
    afx_msg OLE_COLOR GetTypeButtonTextColor();
    afx_msg void SetTypeButtonTextColor(OLE_COLOR newValue);
  #endif
  #ifndef SSP_NO_TYPEBUTTONTYPE
    afx_msg short GetTypeButtonType();
    afx_msg void SetTypeButtonType(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPECHECKCENTER
    afx_msg BOOL GetTypeCheckCenter();
    afx_msg void SetTypeCheckCenter(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPECHECKTEXT
    afx_msg BSTR GetTypeCheckText();
    afx_msg void SetTypeCheckText(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPECHECKTEXTALIGN
    afx_msg short GetTypeCheckTextAlign();
    afx_msg void SetTypeCheckTextAlign(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPECHECKTYPE
    afx_msg short GetTypeCheckType();
    afx_msg void SetTypeCheckType(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXCOUNT
    afx_msg short GetTypeComboBoxCount();
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXCURSEL
    afx_msg short GetTypeComboBoxCurSel();
    afx_msg void SetTypeComboBoxCurSel(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXEDITABLE
    afx_msg BOOL GetTypeComboBoxEditable();
    afx_msg void SetTypeComboBoxEditable(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXINDEX
    afx_msg short GetTypeComboBoxIndex();
    afx_msg void SetTypeComboBoxIndex(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXLIST
    afx_msg BSTR GetTypeComboBoxList();
    afx_msg void SetTypeComboBoxList(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPECOMBOBOXSTRING
    afx_msg BSTR GetTypeComboBoxString();
    afx_msg void SetTypeComboBoxString(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPEDATECENTURY
    afx_msg BOOL GetTypeDateCentury();
    afx_msg void SetTypeDateCentury(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEDATEFORMAT
    afx_msg short GetTypeDateFormat();
    afx_msg void SetTypeDateFormat(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEDATEMAX
    afx_msg BSTR GetTypeDateMax();
    afx_msg void SetTypeDateMax(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPEDATEMIN
    afx_msg BSTR GetTypeDateMin();
    afx_msg void SetTypeDateMin(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPEDATESEPARATOR
    afx_msg short GetTypeDateSeparator();
    afx_msg void SetTypeDateSeparator(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEEDITCHARCASE
    afx_msg short GetTypeEditCharCase();
    afx_msg void SetTypeEditCharCase(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEEDITCHARSET
    afx_msg short GetTypeEditCharSet();
    afx_msg void SetTypeEditCharSet(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEEDITLEN
    afx_msg long GetTypeEditLen();
    afx_msg void SetTypeEditLen(long nNewValue);
  #endif
  #ifndef SSP_NO_TYPEEDITMULTILINE
    afx_msg BOOL GetTypeEditMultiLine();
    afx_msg void SetTypeEditMultiLine(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEEDITPASSWORD
    afx_msg BOOL GetTypeEditPassword();
    afx_msg void SetTypeEditPassword(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATCURRENCYCHAR
    afx_msg short GetTypeFloatCurrencyChar();
    afx_msg void SetTypeFloatCurrencyChar(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATDECIMALCHAR
    afx_msg short GetTypeFloatDecimalChar();
    afx_msg void SetTypeFloatDecimalChar(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATDECIMALPLACES
    afx_msg short GetTypeFloatDecimalPlaces();
    afx_msg void SetTypeFloatDecimalPlaces(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATMAX
    afx_msg double GetTypeFloatMax();
    afx_msg void SetTypeFloatMax(double newValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATMIN
    afx_msg double GetTypeFloatMin();
    afx_msg void SetTypeFloatMin(double newValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATMONEY
    afx_msg BOOL GetTypeFloatMoney();
    afx_msg void SetTypeFloatMoney(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATSEPARATOR
    afx_msg BOOL GetTypeFloatSeparator();
    afx_msg void SetTypeFloatSeparator(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEFLOATSEPCHAR
    afx_msg short GetTypeFloatSepChar();
    afx_msg void SetTypeFloatSepChar(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEHALIGN
    afx_msg short GetTypeHAlign();
    afx_msg void SetTypeHAlign(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPEINTEGERMAX
    afx_msg long GetTypeIntegerMax();
    afx_msg void SetTypeIntegerMax(long nNewValue);
  #endif
  #ifndef SSP_NO_TYPEINTEGERMIN
    afx_msg long GetTypeIntegerMin();
    afx_msg void SetTypeIntegerMin(long nNewValue);
  #endif
  #ifndef SSP_NO_TYPEINTEGERSPININC
    afx_msg long GetTypeIntegerSpinInc();
    afx_msg void SetTypeIntegerSpinInc(long nNewValue);
  #endif
  #ifndef SSP_NO_TYPEINTEGERSPINWRAP
    afx_msg BOOL GetTypeIntegerSpinWrap();
    afx_msg void SetTypeIntegerSpinWrap(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEOWNERDRAWSTYLE
    afx_msg long GetTypeOwnerDrawStyle();
    afx_msg void SetTypeOwnerDrawStyle(long nNewValue);
  #endif
  #ifndef SSP_NO_TYPEPICDEFAULTTEXT
    afx_msg BSTR GetTypePicDefaultText();
    afx_msg void SetTypePicDefaultText(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPEPICMASK
    afx_msg BSTR GetTypePicMask();
    afx_msg void SetTypePicMask(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPEPICTCENTER
    afx_msg BOOL GetTypePictCenter();
    afx_msg void SetTypePictCenter(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEPICTMAINTAINSCALE
    afx_msg BOOL GetTypePictMaintainScale();
    afx_msg void SetTypePictMaintainScale(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPEPICTPICTURE
    afx_msg LPPICTUREDISP GetTypePictPicture();
    afx_msg void SetTypePictPicture(LPPICTUREDISP newValue);
  #endif
  #ifndef SSP_NO_TYPEPICTSTRETCH
    afx_msg BOOL GetTypePictStretch();
    afx_msg void SetTypePictStretch(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPESPIN
    afx_msg BOOL GetTypeSpin();
    afx_msg void SetTypeSpin(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPETEXTALIGNVERT
    afx_msg short GetTypeTextAlignVert();
    afx_msg void SetTypeTextAlignVert(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPETEXTPREFIX
    afx_msg BOOL GetTypeTextPrefix();
    afx_msg void SetTypeTextPrefix(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPETEXTSHADOW
    afx_msg BOOL GetTypeTextShadow();
    afx_msg void SetTypeTextShadow(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPETEXTSHADOWIN
    afx_msg BOOL GetTypeTextShadowIn();
    afx_msg void SetTypeTextShadowIn(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPETEXTWORDWRAP
    afx_msg BOOL GetTypeTextWordWrap();
    afx_msg void SetTypeTextWordWrap(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPETIME24HOUR
    afx_msg short GetTypeTime24Hour();
    afx_msg void SetTypeTime24Hour(short nNewValue);
  #endif
  #ifndef SSP_NO_TYPETIMEMAX
    afx_msg BSTR GetTypeTimeMax();
    afx_msg void SetTypeTimeMax(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPETIMEMIN
    afx_msg BSTR GetTypeTimeMin();
    afx_msg void SetTypeTimeMin(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_TYPETIMESECONDS
    afx_msg BOOL GetTypeTimeSeconds();
    afx_msg void SetTypeTimeSeconds(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_TYPETIMESEPARATOR
    afx_msg short GetTypeTimeSeparator();
    afx_msg void SetTypeTimeSeparator(short nNewValue);
  #endif
  #ifndef SSP_NO_UNITTYPE
    afx_msg short GetUnitType();
    afx_msg void SetUnitType(short nNewValue);
  #endif
  #ifndef SSP_NO_USERRESIZE
    afx_msg short GetUserResize();
    afx_msg void SetUserResize(short nNewValue);
  #endif
  #ifndef SSP_NO_USERRESIZECOL
    afx_msg short GetUserResizeCol();
    afx_msg void SetUserResizeCol(short nNewValue);
  #endif
  #ifndef SSP_NO_USERRESIZEROW
    afx_msg short GetUserResizeRow();
    afx_msg void SetUserResizeRow(short nNewValue);
  #endif
  #ifndef SSP_NO_VALUE
    afx_msg BSTR GetValue();
    afx_msg void SetValue(LPCTSTR lpszNewValue);
  #endif
  #ifndef SSP_NO_VIRTUALCURROWCOUNT
    afx_msg long GetVirtualCurRowCount();
    afx_msg long GetVirtualCurTop();
  #endif
  #ifndef SSP_NO_VIRTUALMAXROWS
    afx_msg long GetVirtualMaxRows();
    afx_msg void SetVirtualMaxRows(long nNewValue);
  #endif
  #ifndef SSP_NO_VIRTUALMODE
    afx_msg BOOL GetVirtualMode();
    afx_msg void SetVirtualMode(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_VIRTUALOVERLAP
    afx_msg long GetVirtualOverlap();
    afx_msg void SetVirtualOverlap(long nNewValue);
  #endif
  #ifndef SSP_NO_VIRTUALROWS
    afx_msg long GetVirtualRows();
    afx_msg void SetVirtualRows(long nNewValue);
  #endif
  #ifndef SSP_NO_VIRTUALSCROLLBUFFER
    afx_msg BOOL GetVirtualScrollBuffer();
    afx_msg void SetVirtualScrollBuffer(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_VISIBLECOLS
    afx_msg long GetVisibleCols();
    afx_msg void SetVisibleCols(long nNewValue);
  #endif
  #ifndef SSP_NO_VISIBLEROWS
    afx_msg long GetVisibleRows();
    afx_msg void SetVisibleRows(long nNewValue);
  #endif
  #ifndef SSP_NO_VSCROLLSPECIAL
    afx_msg BOOL GetVScrollSpecial();
    afx_msg void SetVScrollSpecial(BOOL bNewValue);
  #endif
  #ifndef SSP_NO_VSCROLLSPECIALTYPE
#ifdef SS_V70
    afx_msg long GetVScrollSpecialType();
    afx_msg void SetVScrollSpecialType(long lNewValue);
#else
    afx_msg short GetVScrollSpecialType();
    afx_msg void SetVScrollSpecialType(short nNewValue);
#endif
  #endif
  #ifndef SSP_NO_BACKCOLORSTYLE
    afx_msg short GetBackColorStyle();
    afx_msg void SetBackColorStyle(short nNewValue);
  #endif
  #ifndef SSP_NO_FORMULASYNC
    afx_msg BOOL GetFormulaSync();
    afx_msg void SetFormulaSync(BOOL bNewValue);
  #endif
//DBOCX...
  #ifndef SSP_NO_DATACHANGED
    afx_msg BOOL GetDataChanged();
    afx_msg void SetDataChanged(BOOL bNewValue);
  #endif
//DBOCX.
    afx_msg short GetBorderStyle();
    afx_msg void SetBorderStyle(short nNewValue);
    afx_msg OLE_COLOR GetBackColor();
    afx_msg void SetBackColor(OLE_COLOR newValue);
    afx_msg LPFONTDISP GetFont();
    afx_msg void SetFont(LPFONTDISP newValue);
    afx_msg BOOL GetFontBold();
    afx_msg void SetFontBold(BOOL);
    afx_msg BOOL GetFontItalic();
    afx_msg void SetFontItalic(BOOL);
    afx_msg BSTR GetFontName();
    afx_msg void SetFontName(LPCTSTR);
    afx_msg float GetFontSize();
    afx_msg void SetFontSize(float);
    afx_msg BOOL GetFontStrikethru();
    afx_msg void SetFontStrikethru(BOOL);
    afx_msg BOOL GetFontUnderline();
    afx_msg void SetFontUnderline(BOOL);
    afx_msg OLE_COLOR GetForeColor();
    afx_msg void SetForeColor(OLE_COLOR newValue);
    afx_msg OLE_HANDLE GetHWnd();
    afx_msg BSTR GetText();
    afx_msg void SetText(LPCTSTR lpszNewValue);
  #ifndef SSP_NO_COLWIDTH
    afx_msg double GetColWidth(long lCol);
  #endif
  #ifndef SSP_NO_COLWIDTH
    afx_msg void SetColWidth(long lCol, double newValue);
  #endif
  #ifndef SSP_NO_MAXTEXTCOLWIDTH
    afx_msg double GetMaxTextColWidth(long lCol);
  #endif
  #ifndef SSP_NO_MAXTEXTROWHEIGHT
    afx_msg double GetMaxTextRowHeight(long lRow);
  #endif
  #ifndef SSP_NO_ROWHEIGHT
    afx_msg double GetRowHeight(long lRow);
  #endif
  #ifndef SSP_NO_ROWHEIGHT
    afx_msg void SetRowHeight(long lRow, double newValue);
  #endif
  #ifndef SSP_NO_SORTKEY
    afx_msg long GetSortKey(short nIndex);
    afx_msg void SetSortKey(short nIndex, long nNewValue);
  #endif
  #ifndef SSP_NO_SORTKEYORDER
    afx_msg short GetSortKeyOrder(short nIndex);
    afx_msg void SetSortKeyOrder(short nIndex, short nNewValue);
  #endif
    afx_msg LONG FP_Sort(BOOL fSetProp, short nIndex, long lValue, BOOL fOrder);
  #ifndef SSP_NO_TYPECHECKPICTURE
    afx_msg LPPICTUREDISP GetTypeCheckPicture(short Index);
  #endif
  #ifndef SSP_NO_TYPECHECKPICTURE
    afx_msg void SetTypeCheckPicture(short Index, LPPICTUREDISP newValue);
  #endif
  #ifndef SSF_NO_AddCustomFunction
  afx_msg BOOL AddCustomFunction(LPCTSTR FunctionName, short ParameterCnt);
  #endif
  #ifndef SSF_NO_AddCustomFunctionExt
  afx_msg BOOL AddCustomFunctionExt(LPCTSTR FunctionName, short MinParamCnt, short MaxParamCnt, long Flags);
  #endif
  #ifndef SSF_NO_CFGetCellParam
  afx_msg void CFGetCellParam(short Param, long FAR* Col, long FAR* Row);
  #endif
  #ifndef SSF_NO_CFGetDoubleParam
  afx_msg double CFGetDoubleParam(short Param);
  #endif
  #ifndef SSF_NO_CFGetDoubleParamExt
  afx_msg void CFGetDoubleParamExt(short Param, double FAR* ParamValue);
  #endif
  #ifndef SSF_NO_CFGetLongParam
  afx_msg long CFGetLongParam(short Param);
  #endif
  #ifndef SSF_NO_CFGetParamInfo
#ifdef SS_V70
  afx_msg BOOL CFGetParamInfo(short Param, long FAR* Type, long FAR* Status);
#else
  afx_msg BOOL CFGetParamInfo(short Param, short FAR* Type, short FAR* Status);
#endif
  #endif
  #ifndef SSF_NO_CFGetRangeParam
  afx_msg void CFGetRangeParam(short Param, long FAR* Col, long FAR* Row, long FAR* Col2, long FAR* Row2);
  #endif
  #ifndef SSF_NO_CFGetStringParam
  afx_msg BSTR CFGetStringParam(short Param);
  #endif
  #ifndef SSF_NO_CFSetResult
  afx_msg void CFSetResult(const VARIANT FAR& Var);
  #endif
  #ifndef SSF_NO_ColNumberToLetter
  afx_msg BSTR ColNumberToLetter(long HeaderNumber);
  #endif
  #ifndef SSF_NO_ColWidthToTwips
  afx_msg void ColWidthToTwips(float Width, long FAR* Twips);
  #endif
  #ifndef SSF_NO_GetBottomRightCell
  afx_msg void GetBottomRightCell(long FAR* Col, long FAR* Row);
  #endif
  #ifndef SSF_NO_GetCellDirtyFlag
  afx_msg BOOL GetCellDirtyFlag(long Col, long Row);
  #endif
  #ifndef SSF_NO_GetCellFromScreenCoord
  afx_msg void GetCellFromScreenCoord(long FAR* Col, long FAR* Row, long x, long y);
  #endif
  #ifndef SSF_NO_GetCellPos
  afx_msg BOOL GetCellPos(long Col, long Row, long FAR* x, long FAR* y, long FAR* Width, long FAR* Height);
  #endif
  #ifndef SSF_NO_GetClientArea
  afx_msg void GetClientArea(long FAR* Width, long FAR* Height);
  #endif
  #ifndef SSF_NO_GetColItemData
  afx_msg long GetColItemData(long Col);
  #endif
  #ifndef SSF_NO_QueryCustomName
  afx_msg BSTR QueryCustomName(LPCTSTR Name);
  #endif
  #ifndef SSF_NO_GetCustomName
  afx_msg BSTR GetCustomName(LPCTSTR Name);
  #endif
  #ifndef SSF_NO_GetDataFillData
  afx_msg BOOL GetDataFillData(VARIANT FAR* Var, short VarType);
  #endif
  #ifndef SSF_NO_GetFirstValidCell
  afx_msg void GetFirstValidCell(long FAR* Col, long FAR* Row);
  #endif
  #ifndef SSF_NO_GetItemData
  afx_msg long GetItemData();
  #endif
  #ifndef SSF_NO_GetIteration
  afx_msg BOOL GetIteration(short FAR* MaxIterations, double FAR* MaxChange);
  #endif
  #ifndef SSF_NO_GetLastValidCell
  afx_msg void GetLastValidCell(long FAR* Col, long FAR* Row);
  #endif
  #ifndef SSF_NO_GetMultiSelItem
  afx_msg long GetMultiSelItem(long SelPrev);
  #endif
  #ifndef SSF_NO_GetRefStyle
#ifdef SS_V70
  afx_msg long GetRefStyle();
#else
  afx_msg short GetRefStyle();
#endif
  #endif
  #ifndef SSF_NO_GetRowItemData
  afx_msg long GetRowItemData(long Row);
  #endif
  #ifndef SSF_NO_fpGetText
  afx_msg BOOL fpGetText(long Col, long Row, VARIANT FAR* Var);
  #endif
  #ifndef SSF_NO_IsCellSelected
  afx_msg BOOL IsCellSelected(long Col, long Row);
  #endif
  #ifndef SSF_NO_IsFormulaValid
  afx_msg BOOL IsFormulaValid(LPCTSTR Formula);
  #endif
  #ifndef SSF_NO_IsVisible
  afx_msg BOOL IsVisible(long Col, long Row, BOOL Partial);
  #endif
  #ifndef SSF_NO_RowHeightToTwips
  afx_msg void RowHeightToTwips(long Row, float Height, long FAR* Twips);
  #endif
  #ifndef SSF_NO_LoadFromFile
  afx_msg BOOL LoadFromFile(LPCTSTR FileName);
  #endif
  #ifndef SSF_NO_LoadTabFile
  afx_msg BOOL LoadTabFile(LPCTSTR FileName);
  #endif
  #ifndef SSF_NO_SaveTabFile
  afx_msg BOOL SaveTabFile(LPCTSTR FileName);
  #endif
  #ifndef SSF_NO_SaveToFile
  afx_msg BOOL SaveToFile(LPCTSTR FileName, BOOL DataOnly);
  #endif
  #ifndef SSF_NO_SetCellDirtyFlag
  afx_msg BOOL SetCellDirtyFlag(long Col, long Row, BOOL Dirty);
  #endif
  #ifndef SSF_NO_SetColItemData
  afx_msg void SetColItemData(long Col, long Value);
  #endif
  #ifndef SSF_NO_SetCustomName
  afx_msg BOOL SetCustomName(LPCTSTR Name, LPCTSTR Value);
  #endif
  #ifndef SSF_NO_SetDataFillData
  afx_msg BOOL SetDataFillData(const VARIANT FAR& Var);
  #endif
  #ifndef SSF_NO_SetItemData
  afx_msg void SetItemData(long Value);
  #endif
  #ifndef SSF_NO_SetIteration
  afx_msg void SetIteration(BOOL Iteration, short MaxIterations, double MaxChange);
  #endif
  #ifndef SSF_NO_SetRefStyle
#ifdef SS_V70
  afx_msg void SetRefStyle(long RefStyle);
#else
  afx_msg void SetRefStyle(short RefStyle);
#endif
  #endif
  #ifndef SSF_NO_SetRowItemData
  afx_msg void SetRowItemData(long Row, long Value);
  #endif
  #ifndef SSF_NO_fpSetText
  afx_msg void fpSetText(long Col, long Row, const VARIANT FAR& Var);
  #endif
  #ifndef SSF_NO_TwipsToColWidth
  afx_msg void TwipsToColWidth(long Twips, float FAR* ColWidth);
  #endif
  #ifndef SSF_NO_TwipsToRowHeight
  afx_msg void TwipsToRowHeight(long Row, long Twips, float FAR* RowHeight);
  #endif
  #ifndef SSF_NO_DdeQueryLink
  afx_msg BOOL DdeQueryLink(LPCTSTR ServerPrev, LPCTSTR TopicPrev, LPCTSTR ItemPrev, BSTR FAR* Server, BSTR FAR* Topic, BSTR FAR* Item);
  #endif
  #ifndef SSF_NO_DdeAddLink
  afx_msg BOOL DdeAddLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif
  #ifndef SSF_NO_DdeRemoveLink
  afx_msg BOOL DdeRemoveLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif
  #ifndef SSF_NO_DdeRequestLink
  afx_msg BOOL DdeRequestLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif
  #ifndef SSF_NO_DdePokeLink
  afx_msg BOOL DdePokeLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif
  #ifndef SSF_NO_DdePokeDataToLink
  afx_msg BOOL DdePokeDataToLink(long Col, long Row);
  #endif
  #ifndef SSF_NO_DdeGetMode
  afx_msg short DdeGetMode(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif
  #ifndef SSF_NO_DdeSetMode
  afx_msg short DdeSetMode(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, short Mode);
  #endif
  #ifndef SSF_NO_DdeGetReadExpr
  afx_msg BSTR DdeGetReadExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif
  #ifndef SSF_NO_DdeSetReadExpr
  afx_msg BOOL DdeSetReadExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, LPCTSTR Expr);
  #endif
  #ifndef SSF_NO_DdeGetWriteExpr
  afx_msg BSTR DdeGetWriteExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif
  #ifndef SSF_NO_DdeSetWriteExpr
  afx_msg BOOL DdeSetWriteExpr(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, LPCTSTR Expr);
  #endif
  #ifndef SSF_NO_DdeGetLink
  afx_msg BOOL DdeGetLink(long Col, long Row, BSTR FAR* Server, BSTR FAR* Topic, BSTR FAR* Item, long FAR* ItemCol, long FAR* ItemRow);
  #endif
  #ifndef SSF_NO_DdeSetLink
  afx_msg BOOL DdeSetLink(long Col, long Row, LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long ItemCol, long ItemRow);
  #endif
  #ifndef SSF_NO_DdeQueryDependCell
  afx_msg BOOL DdeQueryDependCell(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long ColPrev, long RowPrev, long FAR* Col, long FAR* Row);
  #endif
  #ifndef SSF_NO_DdeQueryDependCol
  afx_msg BOOL DdeQueryDependCol(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long ColPrev, long FAR* Col);
  #endif
  #ifndef SSF_NO_DdeQueryDependRow
  afx_msg BOOL DdeQueryDependRow(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long RowPrev, long FAR* Row);
  #endif
  #ifndef SSF_NO_DdeQueryDependName
  afx_msg BSTR DdeQueryDependName(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, LPCTSTR NamePrev);
  #endif
  #ifndef SSF_NO_DdeGetDim
  afx_msg BOOL DdeGetDim(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long FAR* Cols, long FAR* Rows);
  #endif
  #ifndef SSF_NO_DdeIsLink
  afx_msg BOOL DdeIsLink(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item);
  #endif

#ifdef SS_V30
  afx_msg short GetAppearance(void);
  afx_msg void SetAppearance(short nNewValue);
  afx_msg short GetTypeComboBoxMaxDrop(void);
  afx_msg void SetTypeComboBoxMaxDrop(short newValue);
  afx_msg short GetTypeMaxEditLen(void);
  afx_msg void SetTypeMaxEditLen(short nNewValue);
  afx_msg short GetTypeComboBoxWidth(void);
  afx_msg void SetTypeComboBoxWidth(short newValue);
  afx_msg short GetTextTip(void);
  afx_msg void SetTextTip(short nNewValue);
  afx_msg long GetTextTipDelay(void);
  afx_msg void SetTextTipDelay(long lNewValue);
  afx_msg short GetScrollBarTrack(void);
  afx_msg void SetScrollBarTrack(short nNewValue);
  afx_msg short GetTypeVAlign(void);
  afx_msg void SetTypeVAlign(short nNewValue);
#ifdef SS_V70
  afx_msg long GetClipboardOptions(void);
  afx_msg void SetClipboardOptions(long lNewValue);
#else
  afx_msg short GetClipboardOptions(void);
  afx_msg void SetClipboardOptions(short nNewValue);
#endif
  afx_msg BOOL GetPrintSmartPrint(void);
  afx_msg void SetPrintSmartPrint(BOOL fNewValue);
  afx_msg short GetPrintPageOrder(void);
  afx_msg void SetPrintPageOrder(short nNewValue);
  afx_msg long GetPrintPageCount(void);
  afx_msg long GetPrintFirstPageNumber(void);
  afx_msg void SetPrintFirstPageNumber(long lNewValue);
  afx_msg long GetPrintNextPageBreakCol(void);
  afx_msg void SetPrintNextPageBreakCol(long lNewValue);
  afx_msg long GetPrintNextPageBreakRow(void);
  afx_msg void SetPrintNextPageBreakRow(long lNewValue);
  afx_msg long GetTypeComboBoxhWnd(void);
  afx_msg void SetTypeComboBoxhWnd(long lNewValue);
  afx_msg OLE_COLOR GetSelBackColor(void);
  afx_msg void SetSelBackColor(OLE_COLOR clrNewValue);
  afx_msg OLE_COLOR GetSelForeColor(void);
  afx_msg void SetSelForeColor(OLE_COLOR clrNewValue);
  afx_msg short GetTwoDigitYearMax(void);
  afx_msg void SetTwoDigitYearMax(short nNewValue);

#ifdef SS_V70
  afx_msg BOOL GetActionKey(long Action, short FAR *fShift, short FAR *fCtrl, short FAR *Key);
  afx_msg BOOL SetActionKey(long Action, short fShift, short fCtrl, short Key);
#else
  afx_msg BOOL GetActionKey(short Action, short FAR *fShift, short FAR *fCtrl, short FAR *Key);
  afx_msg BOOL SetActionKey(short Action, short fShift, short fCtrl, short Key);
#endif
  afx_msg BOOL GetOddEvenRowColor(COLORREF FAR *clrBackOdd, COLORREF FAR *clrForeOdd, COLORREF FAR *clrBackEven, COLORREF FAR *clrForeEven);
  afx_msg BOOL SetOddEvenRowColor(COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven);
  afx_msg BOOL GetFloat(long Col, long Row, double FAR *Value);
  afx_msg BOOL SetFloat(long Col, long Row, double Value);
  afx_msg BOOL GetInteger(long Col, long Row, long FAR *Value);
  afx_msg BOOL SetInteger(long Col, long Row, long Value);
  afx_msg void SetCalText(LPCTSTR ShortDays, LPCTSTR LongDays, LPCTSTR ShortMonths, LPCTSTR LongMonths, LPCTSTR OkText, LPCTSTR CancelText);
  afx_msg BOOL GetTextTipAppearance(BSTR FAR *FontName, short FAR *FontSize, short FAR *FontBold, short FAR *FontItalic, long FAR *BackColor, long FAR *ForeColor);
  afx_msg BOOL SetTextTipAppearance(LPCTSTR FontName, short FontSize, short FontBold, short FontItalic, long BackColor, long ForeColor);
  afx_msg BOOL SetArray(SS_COORD ColLeft, SS_COORD RowTop, VARIANT FAR& VarArray);
  afx_msg BOOL GetArray(SS_COORD ColLeft, SS_COORD RowTop, VARIANT FAR& VarArray);
  afx_msg BOOL _SpreadArray(VARIANT FAR *lpSA, SS_COORD ColLeft, SS_COORD RowTop, BOOL fGet);
  afx_msg BOOL EnumCustomFunction(LPCTSTR PrevFuncName, BSTR FAR* FuncName);
  afx_msg BOOL GetCustomFunction(LPCTSTR FuncName, short FAR* MinArgs, short FAR* MaxArgs, long FAR* Flags);
  afx_msg BOOL RemoveCustomFunction(LPCTSTR FuncName);
  afx_msg BOOL ReCalcCell(long Col, long Row);
#ifdef SS_V80
  afx_msg short GetTabStripButtonPolicy(void);
  afx_msg void SetTabStripButtonPolicy(short newValue);
  afx_msg BOOL GetEnhanceStaticCells(void);
  afx_msg void SetEnhanceStaticCells(BOOL newValue);
  afx_msg short GetAppearanceStyle(void);
  afx_msg void  SetAppearanceStyle(short newValue);
  afx_msg short GetScrollBarStyle(void);
  afx_msg void  SetScrollBarStyle(short newValue);
  afx_msg short GetUseVisualStyles(void);
  afx_msg void  SetUseVisualStyles(short newValue);
  afx_msg short GetTabEnhancedShape(void);
  afx_msg void  SetTabEnhancedShape(short newValue);
  afx_msg short GetCellNoteIndicatorShape(void);
  afx_msg void  SetCellNoteIndicatorShape(short newValue);
  afx_msg OLE_COLOR GetCellNoteIndicatorColor();
  afx_msg void SetCellNoteIndicatorColor(OLE_COLOR newValue);
  afx_msg short GetHighlightHeaders(void);
  afx_msg void  SetHighlightHeaders(short newValue);
  afx_msg short GetHighlightStyle(void);
  afx_msg void  SetHighlightStyle(short newValue);
  afx_msg OLE_COLOR GetHighlightAlphaBlendColor();
  afx_msg void SetHighlightAlphaBlendColor(OLE_COLOR newValue);
  afx_msg short GetHighlightAlphaBlend();
  afx_msg void SetHighlightAlphaBlend(short newValue);
  afx_msg BSTR GetTypeCustomName();
  afx_msg void SetTypeCustomName(LPCTSTR newValue);
  afx_msg long GetTypeCustomStyle();
  afx_msg void SetTypeCustomStyle(long newValue);
  afx_msg VARIANT GetTypeCustomItemData();
  afx_msg void SetTypeCustomItemData(VARIANT *newValue);

  afx_msg VARIANT SaveToBuffer ();
  afx_msg BOOL LoadFromBuffer (VARIANT *pBuff);
  afx_msg VARIANT SaveBlockToBuffer (long col, long row, long col2, long row2, BOOL fSaveColInfo, BOOL fSaveRowInfo);
  afx_msg BOOL LoadBlockFromBuffer (long col, long row, long col2, long row2, VARIANT *pBuff);
  afx_msg BOOL IsVerticalScrollBarVisible();
  afx_msg BOOL IsHorizontalScrollBarVisible();
  afx_msg BOOL SetEnhancedCornerColors(COLORREF clrBackColor, COLORREF clrHoverBackColor, COLORREF clrTriangleColor , COLORREF clrTriangleHover, COLORREF clrTriangleBorderColor, COLORREF clrTriangleHoverBorderColor);

  afx_msg BOOL SetEnhancedColumnHeaderColors(COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor);
  afx_msg BOOL SetEnhancedRowHeaderColors(COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor);

  afx_msg BOOL SetEnhancedSheetTabColors(COLORREF clrNormalUpperStartColor, COLORREF clrNormalUpperEndColor,
								   COLORREF clrNormalLowerStartColor, COLORREF clrNormalLowerEndColor, COLORREF clrHoverUpperStartColor,
								   COLORREF clrHoverUpperEndColor, COLORREF clrHoverLowerStartColor, COLORREF clrHoverLowerEndColor,
								   COLORREF clrTabOuterBorderColor, COLORREF clrTabInnerBorderColor, COLORREF clrButtonUpperNormalStartColor,
								   COLORREF clrButtonUpperNormalEndColor, COLORREF clrButtonLowerNormalStartColor, COLORREF clrButtonLowerNormalEndColor,
								   COLORREF clrButtonUpperHoverStartColor, COLORREF clrButtonUpperHoverEndColor, COLORREF clrButtonLowerHoverStartColor,
								   COLORREF clrButtonLowerHoverEndColor,COLORREF clrButtonUpperPushedStartColor, COLORREF clrButtonUpperPushedEndColor,
								   COLORREF clrButtonLowerPushedStartColor, COLORREF clrButtonLowerPushedEndColor, COLORREF clrArrowColor,
								   COLORREF clrBackColorStartColor, COLORREF clrBackColorEndColor, COLORREF clrSplitBoxStartColor,
								   COLORREF clrSplitBoxEndColor, COLORREF clrForeColor, COLORREF clrActiveForeColor);

  afx_msg BOOL SetEnhancedScrollBarColors(COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor);

  afx_msg BOOL AddCustomCellType(LPCTSTR Name, BOOL Editable, BOOL CanOverflow, BOOL CanBeOverflown, BOOL UseRendererControl);
  afx_msg BOOL RemoveCustomCellType(LPCTSTR Name);
  afx_msg BOOL EnumCustomCellType(LPCTSTR PrevName, BSTR* Name);
  afx_msg BOOL GetCustomCellType(LPCTSTR Name, VARIANT_BOOL* Editable, VARIANT_BOOL* CanOverflow, VARIANT_BOOL* CanBeOverflown, VARIANT_BOOL* UseRendererControl);
  afx_msg VARIANT GetCustomEditor(LPCTSTR Name, long Style);
  afx_msg VARIANT GetCustomRenderer(LPCTSTR Name, long Style);
  afx_msg BOOL PaintControl(VARIANT *Ctrl, OLE_HANDLE hDC, OLE_XPOS_PIXELS Left, OLE_YPOS_PIXELS Top, OLE_XPOS_PIXELS Right, OLE_YPOS_PIXELS Bottom, VARIANT_BOOL ClientOnly);
  afx_msg long GetFontHandle(IFontDisp *pFont);

  SS_CT_PROCS CT_Procs;

	#ifndef SSF_NO_ExportToHTML
		afx_msg BOOL ExportToHTMLEx (LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass);
	#endif
	#ifndef SSF_NO_ExportRangeToHTML
		afx_msg BOOL ExportRangeToHTMLEx (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile, LPCTSTR lpszTitle, LPCTSTR lpszCssFile, LPCTSTR lpszTableClass, LPCTSTR lpszTableRowClass, LPCTSTR lpszTableDefinitionClass, LPCTSTR lpszTableHeaderClass);
	#endif
#ifdef XL12
  afx_msg BOOL SaveExcel2007File(LPCTSTR FileName, LPCTSTR Password, short Flags, LPCTSTR LogFileName);
  afx_msg BOOL OpenExcel2007File(LPCTSTR FileName, LPCTSTR Password, short Sheet, short ExcelSheet, LPCTSTR LogFileName);
  afx_msg short IsExcel2007File(LPCTSTR FileName);
#endif // XL12
  afx_msg IPictureDisp* RenderToMetafile();
#endif // SS_V80
	// jb
	#ifndef SSF_NO_ExportToHTML
		afx_msg BOOL ExportToHTML (LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile);
	#endif
	#ifndef SSF_NO_ExportRangeToHTML
		afx_msg BOOL ExportRangeToHTML (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR lpszFileName, BOOL bAppendFlag, LPCTSTR lpszLogFile);
	#endif
  //rap
  #ifndef SSF_NO_IsExcelFile
          afx_msg BOOL IsExcelFile (LPCTSTR lpszFileName);
  #endif
  #ifndef SSF_NO_GetExcelSheetList
          afx_msg BOOL GetExcelSheetList (LPCTSTR FileName, VARIANT FAR& VarArray, LPSHORT ListCount, LPCTSTR LogFileName, short FAR *WorkbookHandle, BOOL Replace);
          afx_msg BOOL _GetSheetList(VARIANT FAR *lpArray, LPCTSTR FileName, LPSHORT ListCount, LPCTSTR LogFileName, short FAR *WBHandle, BOOL Replace);
  #endif
  #ifndef SSF_NO_ImportExcelSheet
          afx_msg BOOL ImportExcelSheet (short WorkbookHandle, VARIANT FAR& Sheet);
  #endif
  #ifndef SSF_NO_ExportToExcel
          afx_msg BOOL ExportToExcel (LPCTSTR FileName, LPCTSTR SheetName, LPCTSTR LogFileName);
  #endif

#endif // SS_V30

#ifdef SS_V35
  afx_msg BSTR GetCellNote();
  afx_msg void SetCellNote(LPCTSTR Item);
  afx_msg short GetShowScrollTips();
  afx_msg void SetShowScrollTips(short nNewValue);
  afx_msg short GetCellNoteIndicator();
  afx_msg void SetCellNoteIndicator(short nNewValue);

  afx_msg BOOL GetTypeEllipses();
  afx_msg void SetTypeEllipses(BOOL bNewValue);

  afx_msg BOOL GetScriptEnhanced();
  afx_msg void SetScriptEnhanced(BOOL bNewValue);
  afx_msg long GetSelectionCount();
  afx_msg void SetSelectionCount();

  afx_msg short GetUserColAction();
  afx_msg void  SetUserColAction(short nNewValue);
  afx_msg short GetColUserSortIndicator(long lCol);
  afx_msg void  SetColUserSortIndicator(long lCol, short NewValue);
  afx_msg BOOL IsFetchCellNote ();

	#ifndef SSF_NO_ExportToXML
		afx_msg BOOL ExportToXML (LPCTSTR lpszFileName, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR lpszLogFile);
	#endif
	#ifndef SSF_NO_ExportRangeToXML
		afx_msg BOOL ExportRangeToXML (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR lpszFileName, LPCTSTR pszRoot, LPCTSTR pszCollection, long lFlags, LPCTSTR lpszLogFile);
	#endif
	#ifndef SSF_NO_ExportToXMLBuffer
		afx_msg BOOL ExportToXMLBuffer (LPCTSTR pszRoot, LPCTSTR pszCollection, BSTR *pbStrBuff, long lFlags, LPCTSTR lpszLogFile);
	#endif
	#ifndef SSF_NO_ExportRangeToXMLBuffer
		afx_msg BOOL ExportRangeToXMLBuffer (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR pszRoot, LPCTSTR pszCollection, BSTR *pbStrBuff, long lFlags, LPCTSTR lpszLogFile);
	#endif
	#ifndef SSF_NO_ExportRangeToTextFile
		afx_msg	BOOL ExportRangeToTextFile (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR bstrFile, LPCTSTR bstrCellDelim, LPCTSTR bstrColDelim, LPCTSTR bstrRowDelim, long lFlags, LPCTSTR bstrLogFile);
	#endif
	#ifndef SSF_NO_ExportToTextFile
		afx_msg	BOOL ExportToTextFile (LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile);
	#endif
	afx_msg BOOL LoadTextFile (LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile);
    afx_msg BOOL OwnerPrintDraw(OLE_HANDLE hDC, long Left, long Top, long Right, long Bottom, short Page);
    afx_msg BOOL OwnerPrintPageCount(OLE_HANDLE hDC, long Left, long Top, long Right, long Bottom, short *pPageCount);

  afx_msg void SetCalTextOverride(LPCTSTR ShortDays, LPCTSTR LongDays, LPCTSTR ShortMonths, LPCTSTR LongMonths, LPCTSTR OkText, LPCTSTR CancelText);
  afx_msg void GetCalTextOverride(BSTR *pShortDays, LPSHORT pnLenShortDays, BSTR *pLongDays, LPSHORT pnLenLongDays, BSTR *pShortMonths, LPSHORT pnLenShortMonths, 
                                  BSTR *pLongMonths, LPSHORT pnLenLongMonths, BSTR *pOkText, LPSHORT pnLenOkText, BSTR *pCancelText, LPSHORT pnLenCancelText);
#endif

#ifdef SS_V40
	#ifndef SSF_NO_ExportRangeToTextFileU
		afx_msg	BOOL ExportRangeToTextFileU (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR bstrFile, LPCTSTR bstrCellDelim, LPCTSTR bstrColDelim, LPCTSTR bstrRowDelim, long lFlags, LPCTSTR bstrLogFile);
	#endif
	#ifndef SSF_NO_ExportToTextFileU
		afx_msg	BOOL ExportToTextFileU (LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile);
	#endif

  #ifndef SSF_NO_SaveTabFileU
  afx_msg BOOL SaveTabFileU(LPCTSTR FileName);
  #endif
#endif //SS_V40

#ifdef SS_V70
  afx_msg long ColLetterToNumber(LPCTSTR ColLetter);
  afx_msg void DataRefresh(void);
  afx_msg BOOL ImportExcelBook (LPCTSTR FileName, LPCTSTR LogFileName);
  afx_msg BOOL ExportExcelBook (LPCTSTR FileName, LPCTSTR LogFileName);
  afx_msg BOOL InsertSheets(short SheetIndex, short SheetCnt);
  afx_msg BOOL DeleteSheets(short SheetIndex, short SheetCnt);
  afx_msg BOOL MoveSheets(short SheetSrc, short SheetCnt, short SheetDest);
  afx_msg BOOL ResetSheet(short Sheet);
  afx_msg IPictureDisp* LoadPicture(LPCTSTR lpszFileName, short nPictType);
  afx_msg IPictureDisp* LoadResPicture(long hInstance, LPCTSTR lpszResName, LPCTSTR lpszResType, short nPictType);
  afx_msg BOOL SavePicture(IPictureDisp* pPicture, LPCTSTR lpszFileName, short nPictType);
  afx_msg IPictureDisp* LoadPictureBuffer(VARIANT* Buffer, VARIANT* lSize, short nPictType);
  afx_msg BOOL SavePictureBuffer(IPictureDisp* pPicture, short nPictType, VARIANT *pBuffer, VARIANT *plSize);
  afx_msg void CFGetCellParamExt(short Param, short FAR* Sheet, long FAR* Col, long FAR* Row);
  afx_msg void CFGetRangeParamExt(short Param, short FAR* Sheet, long FAR* Col, long FAR* Row, long FAR* Col2, long FAR* Row2);
  afx_msg void ScriptCFGetCellParamExt(short Param, VARIANT FAR* Sheet, VARIANT FAR* Col, VARIANT FAR* Row);
  afx_msg void ScriptCFGetRangeParamExt(short Param, VARIANT FAR* Sheet, VARIANT FAR* Col, VARIANT FAR* Row, VARIANT FAR* Col2, VARIANT FAR* Row2);
  afx_msg BOOL ExportToExcelEx(LPCTSTR FileName, LPCTSTR SheetName, LPCTSTR LogFileName, SHORT Flags);
  afx_msg BOOL ExportExcelBookEx(LPCTSTR FileName, LPCTSTR LogFileName, SHORT Flags);

#endif // SS_V70

#ifdef SS_V40
  afx_msg short GetColMerge();
  afx_msg void  SetColMerge(short nNewValue);
  afx_msg short GetRowMerge();
  afx_msg void  SetRowMerge(short nNewValue);
  afx_msg BSTR GetColID();
  afx_msg void SetColID(LPCTSTR lpszNewValue);
  afx_msg long GetRowHeaderCols();
  afx_msg void SetRowHeaderCols(long newValue);
  afx_msg long GetRowHeadersAutoTextIndex();
  afx_msg void SetRowHeadersAutoTextIndex(long newValue);
  afx_msg long GetColHeaderRows();
  afx_msg void SetColHeaderRows(long newValue);
  afx_msg long GetColHeadersAutoTextIndex();
  afx_msg void SetColHeadersAutoTextIndex(long newValue);
  afx_msg long GetColHeadersUserSortIndex();
  afx_msg void SetColHeadersUserSortIndex(long newValue);
  afx_msg short GetTypeComboBoxAutoSearch(void);
  afx_msg void SetTypeComboBoxAutoSearch(short lNewValue);
  afx_msg double GetTypeSpinInc(void);
  afx_msg void SetTypeSpinInc(double newValue);
  afx_msg BOOL GetTypeSpinWrap(void);
  afx_msg void SetTypeSpinWrap(BOOL newValue);
  afx_msg BOOL GetTypeNegRed(void);
  afx_msg void SetTypeNegRed(BOOL newValue);
  afx_msg double GetTypeCurrencyMin(void);
  afx_msg void SetTypeCurrencyMin(double newValue);
  afx_msg double GetTypeCurrencyMax(void);
  afx_msg void SetTypeCurrencyMax(double newValue);
  afx_msg BSTR GetTypeCurrencyDecimal(void);
  afx_msg void SetTypeCurrencyDecimal(LPCTSTR newValue);
  afx_msg BSTR GetTypeCurrencySeparator(void);
  afx_msg void SetTypeCurrencySeparator(LPCTSTR newValue);
  afx_msg BSTR GetTypeCurrencySymbol(void);
  afx_msg void SetTypeCurrencySymbol(LPCTSTR newValue);
  afx_msg short GetTypeCurrencyLeadingZero(void);
  afx_msg void SetTypeCurrencyLeadingZero(short newValue);
  afx_msg short GetTypeCurrencyDecPlaces(void);
  afx_msg void SetTypeCurrencyDecPlaces(short newValue);
  afx_msg short GetTypeCurrencyNegStyle(void);
  afx_msg void SetTypeCurrencyNegStyle(short newValue);
  afx_msg short GetTypeCurrencyPosStyle(void);
  afx_msg void SetTypeCurrencyPosStyle(short newValue);
  afx_msg BOOL GetTypeCurrencyShowSep(void);
  afx_msg void SetTypeCurrencyShowSep(BOOL newValue);
  afx_msg BOOL GetTypeCurrencyShowSymbol(void);
  afx_msg void SetTypeCurrencyShowSymbol(BOOL newValue);
  afx_msg double GetTypeNumberMin(void);
  afx_msg void SetTypeNumberMin(double newValue);
  afx_msg double GetTypeNumberMax(void);
  afx_msg void SetTypeNumberMax(double newValue);
  afx_msg BSTR GetTypeNumberDecimal(void);
  afx_msg void SetTypeNumberDecimal(LPCTSTR newValue);
  afx_msg BSTR GetTypeNumberSeparator(void);
  afx_msg void SetTypeNumberSeparator(LPCTSTR newValue);
  afx_msg short GetTypeNumberLeadingZero(void);
  afx_msg void SetTypeNumberLeadingZero(short newValue);
  afx_msg short GetTypeNumberDecPlaces(void);
  afx_msg void SetTypeNumberDecPlaces(short newValue);
  afx_msg short GetTypeNumberNegStyle(void);
  afx_msg void SetTypeNumberNegStyle(short newValue);
  afx_msg BOOL GetTypeNumberShowSep(void);
  afx_msg void SetTypeNumberShowSep(BOOL newValue);
  afx_msg double GetTypePercentMin(void);
  afx_msg void SetTypePercentMin(double newValue);
  afx_msg double GetTypePercentMax(void);
  afx_msg void SetTypePercentMax(double newValue);
  afx_msg BSTR GetTypePercentDecimal(void);
  afx_msg void SetTypePercentDecimal(LPCTSTR newValue);
  afx_msg short GetTypePercentDecPlaces(void);
  afx_msg void SetTypePercentDecPlaces(short newValue);
  afx_msg short GetTypePercentNegStyle(void);
  afx_msg void SetTypePercentNegStyle(short newValue);
  afx_msg short GetTypePercentLeadingZero(void);
  afx_msg void SetTypePercentLeadingZero(short newValue);
  afx_msg short GetTypeTextOrient(void);
  afx_msg void SetTypeTextOrient(short newValue);
  afx_msg BSTR GetCellTag(void);
  afx_msg void SetCellTag(LPCTSTR lpszNewValue);
  afx_msg BOOL GetAllowEditOverflow(void);
  afx_msg void SetAllowEditOverflow(BOOL newValue);
#endif // SS_V40
#ifdef SS_V70
  afx_msg short GetSheetCount(void);
  afx_msg void SetSheetCount(short newValue);
  afx_msg short GetActiveSheet(void);
  afx_msg void SetActiveSheet(short newValue);
  afx_msg short GetTabStripLeftSheet(void);
  afx_msg void SetTabStripLeftSheet(short newValue);
  afx_msg LPFONTDISP GetTabStripFont(void);
  afx_msg void SetTabStripFont(LPFONTDISP newValue);
  afx_msg BSTR GetSheetName(void);
  afx_msg void SetSheetName(LPCTSTR newValue);
  afx_msg short GetTabStripPolicy(void);
  afx_msg void SetTabStripPolicy(short newValue);
  afx_msg double GetTabStripRatio(void);
  afx_msg void SetTabStripRatio(double newValue);
  afx_msg double GetPrintZoomFactor(void);
  afx_msg void SetPrintZoomFactor(double newValue);
  afx_msg short GetScrollBarWidth(void);
  afx_msg void SetScrollBarWidth(short newValue);
  afx_msg short GetScrollBarHeight(void);
  afx_msg void SetScrollBarHeight(short newValue);
  afx_msg short GetActiveCellHighlightStyle(void);
  afx_msg void SetActiveCellHighlightStyle(short newValue);
  afx_msg short GetPrintScalingMethod(void);
  afx_msg void SetPrintScalingMethod(short newValue);
  afx_msg short GetPrintBestFitPagesWide(void);
  afx_msg void SetPrintBestFitPagesWide(short newValue);
  afx_msg short GetPrintBestFitPagesTall(void);
  afx_msg void SetPrintBestFitPagesTall(short newValue);
  afx_msg BOOL GetPrintCenterOnPageH(void);
  afx_msg void SetPrintCenterOnPageH(BOOL newValue);
  afx_msg BOOL GetPrintCenterOnPageV(void);
  afx_msg void SetPrintCenterOnPageV(BOOL newValue);
  afx_msg BOOL GetAllowColMove(void);
  afx_msg void SetAllowColMove(BOOL newValue);
  afx_msg BOOL GetAllowRowMove(void);
  afx_msg void SetAllowRowMove(BOOL newValue);
  afx_msg double GetTypeScientificMin(void);
  afx_msg void SetTypeScientificMin(double newValue);
  afx_msg double GetTypeScientificMax(void);
  afx_msg void SetTypeScientificMax(double newValue);
  afx_msg BSTR GetTypeScientificDecimal(void);
  afx_msg void SetTypeScientificDecimal(LPCTSTR newValue);
  afx_msg short GetTypeScientificDecPlaces(void);
  afx_msg void SetTypeScientificDecPlaces(short newValue);
  afx_msg OLE_COLOR GetScrollBarHColor(void);
  afx_msg void SetScrollBarHColor(OLE_COLOR newValue);
  afx_msg OLE_COLOR GetScrollBarVColor(void);
  afx_msg void SetScrollBarVColor(OLE_COLOR newValue);
  afx_msg short GetSheet(void);
  afx_msg void SetSheet(short newValue);
  afx_msg short GetSheetSendingEvent(void);
  afx_msg BOOL GetSheetVisible(void);
  afx_msg void SetSheetVisible(BOOL newValue);
#endif // SS_V70
  void FireQueryAdvanceCommon(BOOL AdvanceNext, BOOL FAR* Cancel);

/**/
#ifdef SS_V35
  afx_msg void ScriptCFGetCellParam(short Param, VARIANT FAR* Col, VARIANT FAR* Row);
  afx_msg BOOL ScriptCFGetParamInfo(short Param, VARIANT FAR* Type, VARIANT FAR* Status);
  afx_msg void ScriptCFGetDoubleParamExt(short Param, VARIANT FAR* ParamValue);
  afx_msg void ScriptCFGetRangeParam(short Param, VARIANT FAR* Col, VARIANT FAR* Row, VARIANT FAR* Col2, VARIANT FAR* Row2);
  afx_msg void ScriptColWidthToTwips(float Width, VARIANT FAR* Twips);
  afx_msg void ScriptGetBottomRightCell(VARIANT FAR* Col, VARIANT FAR* Row);
  afx_msg void ScriptGetCellFromScreenCoord(VARIANT FAR* Col, VARIANT FAR* Row, long x, long y);
  afx_msg BOOL ScriptGetCellPos(long Col, long Row, VARIANT FAR* x, VARIANT FAR* y, VARIANT FAR* Width, VARIANT FAR* Height);
  afx_msg void ScriptGetClientArea(VARIANT FAR* Width, VARIANT FAR* Height);
  afx_msg void ScriptGetFirstValidCell(VARIANT FAR* Col, VARIANT FAR* Row);
  afx_msg BOOL ScriptGetIteration(VARIANT FAR* MaxIterations, VARIANT FAR* MaxChange);
  afx_msg void ScriptGetLastValidCell(VARIANT FAR* Col, VARIANT FAR* Row);
  afx_msg void ScriptRowHeightToTwips(long Row, float Height, VARIANT FAR* Twips);
  afx_msg void ScriptTwipsToColWidth(long Twips, VARIANT FAR* ColWidth);
  afx_msg void ScriptTwipsToRowHeight(long Row, long Twips, VARIANT FAR* RowHeight);
  afx_msg BOOL ScriptGetActionKey(short Action, VARIANT FAR *Shift, VARIANT FAR *Ctrl, VARIANT FAR *Key);
  afx_msg BOOL ScriptGetOddEvenRowColor(VARIANT FAR *clrBackOdd, VARIANT FAR *clrForeOdd, VARIANT FAR *clrBackEven, VARIANT FAR *clrForeEven);
  afx_msg BOOL ScriptGetFloat(long Col, long Row, VARIANT FAR *Value);
  afx_msg BOOL ScriptGetInteger(long Col, long Row, VARIANT FAR *Value);
  afx_msg BOOL ScriptGetTextTipAppearance(VARIANT FAR *FontName, VARIANT FAR *FontSize, VARIANT FAR *FontBold, VARIANT FAR *FontItalic, VARIANT FAR *BackColor, VARIANT FAR *ForeColor);
  afx_msg BOOL ScriptEnumCustomFunction(LPCTSTR PrevFuncName, VARIANT FAR* FuncName);
  afx_msg BOOL ScriptGetCustomFunction(LPCTSTR FuncName, VARIANT FAR* MinArgs, VARIANT FAR* MaxArgs, VARIANT FAR* Flags);
  afx_msg BOOL ScriptGetExcelSheetList (LPCTSTR FileName, VARIANT FAR& VarArray, VARIANT FAR* ListCount, LPCTSTR LogFileName, VARIANT FAR* WorkbookHandle, BOOL Replace);
  afx_msg BOOL ScriptExportToXMLBuffer (LPCTSTR pszRoot, LPCTSTR pszCollection, VARIANT FAR *pbStrBuff, long lFlags, LPCTSTR lpszLogFile);
  afx_msg BOOL ScriptExportRangeToXMLBuffer (SS_COORD col, SS_COORD row, SS_COORD col2, SS_COORD row2, LPCTSTR pszRoot, LPCTSTR pszCollection, VARIANT FAR *pbStrBuff, long lFlags, LPCTSTR lpszLogFile);
  afx_msg BOOL ScriptOwnerPrintPageCount(OLE_HANDLE hDC, long Left, long Top, long Right, long Bottom, VARIANT FAR *pPageCount);
  afx_msg void ScriptGetCalTextOverride(VARIANT FAR *pShortDays, VARIANT FAR *pnLenShortDays, VARIANT FAR *pLongDays, VARIANT FAR *pnLenLongDays, VARIANT FAR *pShortMonths, VARIANT FAR *pnLenShortMonths, 
                                  VARIANT FAR *pLongMonths, VARIANT FAR *pnLenLongMonths, VARIANT FAR *pOkText, VARIANT FAR *pnLenOkText, VARIANT FAR *pCancelText, VARIANT FAR *pnLenCancelText);

  afx_msg void PrintSheet(VARIANT *pvFlags);
  afx_msg void AddSelection(long lCol, long lRow, long lCol2, long lRow2);
  afx_msg void ClearRange(long lCol, long lRow, long lCol2, long lRow2, BOOL bDataOnly);
  afx_msg void ClearSelection();
  afx_msg void ClipboardCopy();
  afx_msg void ClipboardCut();
  afx_msg void ClipboardPaste();
  afx_msg void CopyRange(long lCol, long lRow, long lCol2, long lRow2, long lColDest, long lRowDest);
  afx_msg void DeleteCols(long lCol, long lNumCols);
  afx_msg void DeleteRows(long lRow, long lNumRows);
  afx_msg void DataSave();
#ifdef SS_V70
  afx_msg void GetCellBorder(long lCol, long lRow, long nIndex, VARIANT *pcrColor, VARIANT *plStyle);
#else
  afx_msg void GetCellBorder(long lCol, long lRow, short nIndex, VARIANT *pcrColor, VARIANT *plStyle);
#endif
  afx_msg void GetSelection(long lIndex, VARIANT *plCol, VARIANT *plRow, VARIANT *plCol2, VARIANT *plRow2);
  afx_msg void InsertCols(long lCol, long lNumCols);
  afx_msg void InsertRows(long lRow, long lNumRows);
  afx_msg void MoveRange(long lCol, long lRow, long lCol2, long lRow2, long lColDest, long lRowDest);
  afx_msg void ReCalc();
  afx_msg void Reset();
  afx_msg void SetActiveCell(long lCol, long lRow);
#ifdef SS_V70
  afx_msg void SetCellBorder(long lCol, long lRow, long lCol2, long lRow2, long nIndex, OLE_COLOR crColor,  long lStyle);
#else
  afx_msg void SetCellBorder(long lCol, long lRow, long lCol2, long lRow2, short nIndex, OLE_COLOR crColor,  long lStyle);
#endif
  afx_msg void SetSelection(long lCol, long lRow, long lCol2, long lRow2);
  afx_msg void ShowCell(long lCol, long lRow, long lPosition);
#ifdef SS_V40
  afx_msg BOOL Sort(long lCol, long lRow, long lCol2, long lRow2, long lSortBy, VARIANT *SortKeys, VARIANT *SortKeyOrders);
#else
  afx_msg void Sort(long lCol, long lRow, long lCol2, long lRow2, long lSortBy, VARIANT *SortKeys, VARIANT *SortKeyOrders);
#endif // SS_V40
  afx_msg void SwapRange(long lCol, long lRow, long lCol2, long lRow2, long lColDest, long lRowDest);
  afx_msg void TypeComboBoxClear(long lCol, long lRow);
  afx_msg void TypeComboBoxRemoveItem(long lCol, long lRow, short nIndex);
  afx_msg void VirtualRefresh();
#endif // SS_V35
/**/
#ifdef SS_V40
afx_msg BOOL AddCellSpan(long lCol, long lRow, long lNumCols, long lNumRows);
afx_msg short GetCellSpan(long lCol, long lRow, VARIANT *pvColAnchor, VARIANT *pvRowAnchor, VARIANT *pvNumCols, VARIANT *pvNumRows);
afx_msg void RemoveCellSpan(long lCol, long lRow);
afx_msg long GetColFromID(LPCTSTR ColID);
afx_msg void CopyColRange(long lCol, long lCol2, long lColDest);
afx_msg void CopyRowRange(long lRow, long lRow2, long lRowDest);
afx_msg void MoveColRange(long lCol, long lCol2, long lColDest);
afx_msg void MoveRowRange(long lRow, long lRow2, long lRowDest);
afx_msg void SwapColRange(long lCol, long lCol2, long lColDest);
afx_msg void SwapRowRange(long lRow, long lRow2, long lRowDest);
afx_msg long SearchCol(long lCol, long lRowStart, long lRowEnd, LPCTSTR Text, short SearchFlags);
afx_msg long SearchRow(long lRow, long lColStart, long lColEnd, LPCTSTR Text, short SearchFlags);
afx_msg BOOL GetEventEnabled(short Event);
#endif // SS_V40
afx_msg void SetEventEnabled(short Event, BOOL IsEnabled);

  DECLARE_DISPATCH_MAP()

  afx_msg void AboutBox();

// Dispatch and event IDs
public:
  enum {
  //{{AFX_DISP_ID(CSsocxCtrl)
    //}}AFX_DISP_ID

/* saved copy of dispids 5/7/01 - rap
#if defined(SS_SPREADOCX) || defined(SS_RSOCX)
    dispidAction = 1L,
    dispidActiveCol = 2L,
    dispidActiveRow = 3L,
    dispidAllowCellOverflow = 4L,
    dispidAllowDragDrop = 5L,
    dispidAllowMultiBlocks = 6L,
    dispidAllowUserFormulas = 7L,
    dispidArrowsExitEditMode = 8L,
    dispidAutoCalc = 9L,
    dispidAutoClipboard = 10L,
    dispidAutoSize = 11L,
    dispidBlockMode = 12L,
    dispidButtonDrawMode = 13L,
    dispidCalcDependencies = 14L,
    dispidCellBorderColor = 15L,
    dispidCellBorderStyle = 16L,
    dispidCellBorderType = 17L,
    dispidCellType = 18L,
    dispidChangeMade = 19L,
    dispidClip = 20L,
    dispidClipValue = 21L,
    dispidCol = 22L,
    dispidCol2 = 23L,
    dispidColHeaderDisplay = 24L,
    dispidColHidden = 25L,
    dispidColPageBreak = 26L,
    dispidColsFrozen = 27L,
    dispidCursorStyle = 28L,
    dispidCursorType = 29L,
    dispidDataChanged = 30L,    //DBOCX
    dispidDataColCnt = 31L,
    dispidDataField  = 32L,     //DBOCX
    dispidDataFillEvent = 33L,  //DBOCX
    dispidDataRowCnt = 34L,
    dispidDataSource = 35L,     //DBOCX
    dispidDAutoCellTypes = 36L,
    dispidDAutoFill = 37L,
    dispidDAutoHeadings = 38L,
    dispidDAutoSave = 39L,
    dispidDAutoSizeCols = 40L,
    dispidDestCol = 41L,
    dispidDestRow = 42L,
    dispidDInformActiveRowChange = 43L,
    dispidDisplayColHeaders = 44L,
    dispidDisplayRowHeaders = 45L,
    dispidEditEnterAction = 46L,
    dispidEditMode = 47L,
    dispidEditModePermanent = 48L,
    dispidEditModeReplace = 49L,
    dispidFileNum = 50L,
    dispidFloatDefCurrencyChar = 51L,
    dispidFloatDefDecimalChar = 52L,
    dispidFloatDefSepChar = 53L,
    dispidFormula = 54L,
    dispidGrayAreaBackColor = 55L,
    dispidGridColor = 56L,
    dispidGridShowHoriz = 57L,
    dispidGridShowVert = 58L,
    dispidGridSolid = 59L,
    dispidHDCPrinter = 60L,
    dispidInterfaceDesigner = 61L,
    dispidIsBlockSelected = 62L,
    dispidLeftCol = 63L,
    dispidLock = 64L,
    dispidLockBackColor = 65L,
    dispidLockForeColor = 66L,
    dispidMaxCols = 67L,
    dispidMaxRows = 68L,
    dispidMaxTextCellHeight = 69L,
    dispidMaxTextCellWidth = 70L,
    dispidMoveActiveOnFocus = 71L,
    dispidMultiSelCount = 72L,
    dispidMultiSelIndex = 73L,
    dispidNoBeep = 74L,
    dispidNoBorder = 75L,
    dispidOperationMode = 76L,
    dispidPosition = 77L,
    dispidPrintAbortMsg = 78L,
    dispidPrintBorder = 79L,
    dispidPrintColHeaders = 80L,
    dispidPrintColor = 81L,
    dispidPrintFooter = 82L,
    dispidPrintGrid = 83L,
    dispidPrintHeader = 84L,
    dispidPrintJobName = 85L,
    dispidPrintMarginBottom = 86L,
    dispidPrintMarginLeft = 87L,
    dispidPrintMarginRight = 88L,
    dispidPrintMarginTop = 89L,
    dispidPrintPageEnd = 90L,
    dispidPrintPageStart = 91L,
    dispidPrintRowHeaders = 92L,
    dispidPrintShadows = 93L,
    dispidPrintType = 94L,
    dispidPrintUseDataMax = 95L,
    dispidProcessTab = 96L,
    dispidProtect = 97L,
    dispidReDraw = 98L,
    dispidRestrictCols = 99L,
    dispidRestrictRows = 100L,
    dispidRetainSelBlock = 101L,
    dispidRow = 102L,
    dispidRow2 = 103L,
    dispidRowHeaderDisplay = 104L,
    dispidRowHidden = 105L,
    dispidRowPageBreak = 106L,
    dispidRowsFrozen = 107L,
    dispidScrollBarExtMode = 108L,
    dispidScrollBarMaxAlign = 109L,
    dispidScrollBars = 110L,
    dispidScrollBarShowMax = 111L,
    dispidSelBlockCol = 112L,
    dispidSelBlockCol2 = 113L,
    dispidSelBlockRow = 114L,
    dispidSelBlockRow2 = 115L,
    dispidSelectBlockOptions = 116L,
    dispidSelLength = 117L,
    dispidSelModeIndex = 118L,
    dispidSelModeSelCount = 119L,
    dispidSelModeSelected = 120L,
    dispidSelStart = 121L,
    dispidSelText = 122L,
    dispidShadowColor = 123L,
    dispidShadowDark = 124L,
    dispidShadowText = 125L,
    dispidSortBy = 126L,
    dispidStartingColNumber = 127L,
    dispidStartingRowNumber = 128L,
    dispidTopRow = 129L,
    dispidTypeButtonAlign = 130L,
    dispidTypeButtonBorderColor = 131L,
    dispidTypeButtonColor = 132L,
    dispidTypeButtonDarkColor = 133L,
    dispidTypeButtonLightColor = 134L,
    dispidTypeButtonPicture = 135L,
    dispidTypeButtonPictureDown = 136L,
    dispidTypeButtonShadowSize = 137L,
    dispidTypeButtonText = 138L,
    dispidTypeButtonTextColor = 139L,
    dispidTypeButtonType = 140L,
    dispidTypeCheckCenter = 141L,
    dispidTypeCheckText = 142L,
    dispidTypeCheckTextAlign = 143L,
    dispidTypeComboBoxCount = 144L,
    dispidTypeComboBoxCurSel = 145L,
    dispidTypeComboBoxEditable = 146L,
    dispidTypeComboBoxIndex = 147L,
    dispidTypeComboBoxList = 148L,
    dispidTypeComboBoxString = 149L,
    dispidTypeDateCentury = 150L,
    dispidTypeDateFormat = 151L,
    dispidTypeDateMax = 152L,
    dispidTypeDateMin = 153L,
    dispidTypeDateSeparator = 154L,
    dispidTypeEditCharCase = 155L,
    dispidTypeEditCharSet = 156L,
    dispidTypeEditLen = 157L,
    dispidTypeEditMultiLine = 158L,
    dispidTypeEditPassword = 159L,
    dispidTypeFloatCurrencyChar = 160L,
    dispidTypeFloatDecimalChar = 161L,
    dispidTypeFloatDecimalPlaces = 162L,
    dispidTypeFloatMax = 163L,
    dispidTypeFloatMin = 164L,
    dispidTypeFloatMoney = 165L,
    dispidTypeFloatSeparator = 166L,
    dispidTypeFloatSepChar = 167L,
    dispidTypeHAlign = 168L,
    dispidTypeIntegerMax = 169L,
    dispidTypeIntegerMin = 170L,
    dispidTypeIntegerSpinInc = 171L,
    dispidTypeIntegerSpinWrap = 172L,
    dispidTypeOwnerDrawStyle = 173L,
    dispidTypePicDefaultText = 174L,
    dispidTypePicMask = 175L,
    dispidTypePictCenter = 176L,
    dispidTypePictMaintainScale = 177L,
    dispidTypePictPicture = 178L,
    dispidTypePictStretch = 179L,
    dispidTypeSpin = 180L,
    dispidTypeTextAlignVert = 181L,
    dispidTypeTextPrefix = 182L,
    dispidTypeTextShadow = 183L,
    dispidTypeTextShadowIn = 184L,
    dispidTypeTextWordWrap = 185L,
    dispidTypeTime24Hour = 186L,
    dispidTypeTimeMax = 187L,
    dispidTypeTimeMin = 188L,
    dispidTypeTimeSeconds = 189L,
    dispidTypeTimeSeparator = 190L,
    dispidUnitType = 192L,
    dispidUserResize = 192L,
    dispidUserResizeCol = 193L,
    dispidUserResizeRow = 194L,
    dispidValue = 195L,
    dispidVirtualCurRowCount = 196L,
    dispidVirtualCurTop = 197L,
    dispidVirtualMaxRows = 198L,
    dispidVirtualMode = 199L,
    dispidVirtualOverlap = 200L,
    dispidVirtualRows = 201L,
    dispidVirtualScrollBuffer = 202L,
    dispidVisibleCols = 203L,
    dispidVisibleRows = 204L,
    dispidVScrollSpecial = 205L,
    dispidVScrollSpecialType = 206L,
    dispidFontBold = 207L,
    dispidFontItalic = 208L,
    dispidFontName = 209L,
    dispidFontSize = 210L,
    dispidFontStrikethru = 211L,
    dispidFontUnderline = 212L,
    dispidCursorIcon = 213L,
    dispidPrintOrientation = 214L,
    dispidBackColorStyle = 215L,
    dispidFormulaSync = 216L,
    dispidColWidth = 217L,
    dispidMaxTextColWidth = 218L,
    dispidMaxTextRowHeight = 219L,
    dispidRowHeight = 220L,
    dispidSortKey = 221L,
    dispidSortKeyOrder = 222L,
    dispidTypeCheckPicture = 223L,
    dispidAddCustomFunction = 224L,
    dispidAddCustomFunctionExt = 225L,
    dispidCFGetCellParam = 226L,
    dispidCFGetDoubleParam = 227L,
    dispidCFGetDoubleParamExt = 228L,
    dispidCFGetLongParam = 229L,
    dispidCFGetParamInfo = 230L,
    dispidCFGetRangeParam = 231L,
    dispidCFGetStringParam = 232L,
    dispidCFSetResult = 233L,
    dispidColNumberToLetter = 234L,
    dispidColWidthToTwips = 235L,
    dispidGetBottomRightCell = 236L,
    dispidGetCellDirtyFlag = 237L,
    dispidGetCellFromScreenCoord = 238L,
    dispidGetCellPos = 239L,
    dispidGetClientArea = 240L,
    dispidGetColItemData = 241L,
    dispidQueryCustomName = 242L,
    dispidGetCustomName = 243L,
    dispidGetDataFillData = 244L,
    dispidGetFirstValidCell = 245L,
    dispidGetItemData = 246L,
    dispidGetIteration = 247L,
    dispidGetLastValidCell = 248L,
    dispidGetMultiSelItem = 249L,
    dispidGetRefStyle = 250L,
    dispidGetRowItemData = 251L,
    dispidGetText = 252L,
    dispidIsCellSelected = 253L,
    dispidIsFormulaValid = 254L,
    dispidIsVisible = 255L,
    dispidLoadFromFile = 256L,
    dispidLoadTabFile = 257L,
    dispidRowHeightToTwips = 258L,
    dispidSaveTabFile = 259L,
    dispidSaveToFile = 260L,
    dispidSetCellDirtyFlag = 261L,
    dispidSetColItemData = 262L,
    dispidSetCustomName = 263L,
    dispidSetDataFillData = 264L,
    dispidSetItemData = 265L,
    dispidSetIteration = 266L,
    dispidSetRefStyle = 267L,
    dispidSetRowItemData = 268L,
    dispidSetText = 269L,
    dispidTwipsToColWidth = 270L,
    dispidTwipsToRowHeight = 271L,
    dispidTypeCheckType = 272L,
	dispidGetActionKey = 273L,
	dispidSetActionKey = 274L,
	dispidGetOddEvenRowColor = 275L,
	dispidSetOddEvenRowColor = 276L,
	dispidGetFloat = 277L,
	dispidSetFloat = 278L,
	dispidGetInteger = 279L,
	dispidSetInteger = 280L,
	dispidSetCalText = 281L,
	dispidGetTextTipAppearance = 282L,
	dispidSetTextTipAppearance = 283L,
	dispidExportToHTML = 284L,
	dispidExportRangeToHTML = 285L,
	dispidGetArray = 286L,
	dispidSetArray = 287L,
	dispidIsExcelFile = 288L,
	dispidGetExcelSheetList = 289L,
	dispidImportExcelSheet = 290L,
	dispidExportToExcel = 291L,
	dispidEnumCustomFunction = 292L,
	dispidGetCustomFunction = 293L,
	dispidRemoveCustomFunction = 294L,
	dispidReCalcCell = 295L,

    #if defined(SS_RSOCX)
    dispidDdeQueryLink = 273L,
    dispidDdeAddLink = 274L,
    dispidDdeRemoveLink = 275L,
    dispidDdeRequestLink = 276L,
    dispidDdePokeLink = 277L,
    dispidDdePokeDataToLink = 278L,
    dispidDdeGetLinkMode = 279L,
    dispidDdeSetLinkMode = 280L,
    dispidDdeGetLinkReadExpr = 281L,
    dispidDdeSetLinkReadExpr = 282L,
    dispidDdeGetLinkWriteExpr = 283L,
    dispidDdeSetLinkWriteExpr = 284L,
    dispidDdeGetLink = 285L,
    dispidDdeSetLink = 286L,
    dispidDdeQueryDependCell = 287L,
    dispidDdeQueryDependCol = 288L,
    dispidDdeQueryDependRow = 289L,
    dispidDdeQueryDependName = 290L,
    dispidDdeGetDim = 291L,
    dispidDdeIsLink = 292L,

    #endif

    #ifdef SS_V30
    #if defined(SS_RSOCX)
    dispidAppearance = 293L,
    dispidTypeComboBoxMaxDrop = 294L,
    dispidTypeMaxEditLen = 295L,
    dispidTypeComboBoxWidth = 296L,
    dispidTextTip = 297L,
    dispidTextTipDelay = 298L,
    dispidScrollBarTrack = 299L,
    dispidTypeVAlign = 300L,
    dispidClipboardOptions = 301L,
    dispidPrintSmartPrint = 302L,
    dispidPrintPageOrder = 303L,
    dispidPrintPageCount = 304L,
    dispidPrintFirstPageNumber = 305L,
    dispidPrintNextPageBreakCol = 306L,
    dispidPrintNextPageBreakRow = 307L,
    dispidTypeComboBoxhWnd = 308L,
    dispidSelBackColor = 309L,
    dispidSelForeColor = 310L,
    dispidTwoDigitYearMax = 311L,
    dispidDataMember = 337L,
    #else
    dispidAppearance = 273L,
    dispidTypeComboBoxMaxDrop = 274L,
    dispidTypeMaxEditLen = 275L,
    dispidTypeComboBoxWidth = 276L,
    dispidTextTip = 277L,
    dispidTextTipDelay = 278L,
    dispidScrollBarTrack = 279L,
    dispidTypeVAlign = 280L,
    dispidClipboardOptions = 281L,
    dispidPrintSmartPrint = 282L,
    dispidPrintPageOrder = 283L,
    dispidPrintPageCount = 284L,
    dispidPrintFirstPageNumber = 285L,
    dispidPrintNextPageBreakCol = 286L,
    dispidPrintNextPageBreakRow = 287L,
    dispidTypeComboBoxhWnd = 288L,
    dispidSelBackColor = 289L,
    dispidSelForeColor = 290L,
    dispidTwoDigitYearMax = 291L,
    dispidDataMember = 317L,
    #endif
    #endif // SS_V30

    #endif
    eventidAdvance = 1L,
    eventidBlockSelected = 2L,
    eventidButtonClicked = 3L,
    eventidChange = 4L,
    eventidClick = 5L,
    eventidColWidthChange = 6L,
    eventidCustomFunction = 7L,
    eventidDataAddNew = 8L,
    eventidDataColConfig = 9L,
    eventidDataFill = 10L,
    eventidDblClick = 11L,
    eventidDragDropBlock = 12L,
    eventidDrawItem = 13L,
    eventidEditError = 14L,
    eventidEditMode = 15L,
    eventidEnterRow = 16L,
    eventidLeaveCell = 17L,
    eventidLeaveRow = 18L,
    eventidPrintAbort = 19L,
    eventidQueryAdvance = 20L,
    eventidQueryData = 21L,
    eventidRightClick = 22L,
    eventidRowHeightChange = 23L,
    eventidSelChange = 24L,
    eventidTopLeftChange = 25L,
    eventidUserFormulaEntered = 26L,
    eventidVirtualClearData = 27L,
    eventidPrintMsgBox = 28L,
    #if !defined(SS_RSOCX)
    eventidComboCloseUp = 29L,
    eventidComboDropDown = 30L,
    eventidComboSelChange = 31L,
    eventidTextTipFetch = 32L,
    eventidEditChange = 33L,
#ifdef FP_OLEDRAG
	dispidOLEDropMode = 315L,
	dispidOLEDrag = 316L,
	eventidOLECompleteDrag = 34L,
	eventidOLEDragDrop = 35L,
	eventidOLEDragOver = 36L,
	eventidOLEGiveFeedback = 37L,
	eventidOLESetData = 38L,
	eventidOLEStartDrag = 39L,
#endif
    #endif
#if defined(SS_RSOCX)
    eventidLinkDispatchNotCreated = 29L,
    eventidLinkError = 30L,
    eventidLinkItemNotSupported = 31L,
    eventidLinkItemSupported = 32L,
    eventidLinkNewData = 33L,
    eventidLinkNotify = 34L,
    eventidLinkOutOfMemory = 35L,
    eventidLinkPokeCompleted = 36L,
    eventidLinkRequestCompleted = 37L,
    eventidLinkServerDisconnected = 38L,
    eventidLinkUnableToConnectToServer = 39L,
    eventidComboCloseUp = 40L,
    eventidComboDropDown = 41L,
    eventidComboSelChange = 42L,
    eventidComboTextTipFetch = 43L,
    eventidComboEditChange = 44L,
#ifdef FP_OLEDRAG
	dispidOLEDropMode = 335L,
	dispidOLEDrag = 336L,
	eventidOLECompleteDrag = 45L,
	eventidOLEDragDrop = 46L,
	eventidOLEDragOver = 47L,
	eventidOLEGiveFeedback = 48L,
	eventidOLESetData = 49L,
	eventidOLEStartDrag = 50L,
#endif
#elif defined(SS_GRIDOCX)
    dispidAction = 1L,
    dispidActiveCol = 2L,
    dispidActiveRow = 3L,
    dispidArrowsExitEditMode = 4L,
    dispidAutoClipboard = 5L,
    dispidBlockMode = 6L,
    dispidCellType = 7L,
    dispidChangeMade = 8L,
    dispidClip = 9L,
    dispidCol = 10L,
    dispidCol2 = 11L,
    dispidColHeaderDisplay = 12L,
    dispidColHidden = 13L,
    dispidColsFrozen = 14L,
    dispidDataChanged = 15L,                        //DBOCX
    dispidDataColCnt = 16L,
    dispidDataField = 17L,
    dispidDataRowCnt = 18L,
    dispidDataSource = 19L,                         //DBOCX
    dispidDAutoSave = 20L,
    dispidDAutoSizeCols = 21L,
    dispidDInformActiveRowChange = 22L,
    dispidDisplayColHeaders = 23L,
    dispidDisplayRowHeaders = 24L,
    dispidEditEnterAction = 25L,
    dispidEditMode = 26L,
    dispidEditModePermanent = 27L,
    dispidEditModeReplace = 28L,
    dispidGridColor = 29L,
    dispidGridShowHoriz = 10L,
    dispidGridShowVert = 31L,
    dispidGridSolid = 32L,
    dispidInterfaceDesigner = 33L,
    dispidIsBlockSelected = 34L,
    dispidLeftCol = 35L,
    dispidLock = 36L,
    dispidMaxCols = 37L,
    dispidMaxRows = 38L,
    dispidMoveActiveOnFocus = 39L,
    dispidNoBeep = 40L,
    dispidNoBorder = 41L,
    dispidProcessTab = 42L,
    dispidReDraw = 43L,
    dispidRetainSelBlock = 44L,
    dispidRow = 45L,
    dispidRow2 = 46L,
    dispidRowHeaderDisplay = 47L,
    dispidRowHidden = 48L,
    dispidRowsFrozen = 49L,
    dispidScrollBarExtMode = 50L,
    dispidScrollBars = 51L,
    dispidSelBlockCol = 52L,
    dispidSelBlockCol2 = 53L,
    dispidSelBlockRow = 54L,
    dispidSelBlockRow2 = 55L,
    dispidSelectBlockOptions = 56L,
    dispidShadowColor = 57L,
    dispidShadowDark = 58L,
    dispidShadowText = 59L,
    dispidStartingColNumber = 60L,
    dispidStartingRowNumber = 61L,
    dispidTopRow = 62L,
    dispidTypeButtonAlign = 63L,
    dispidTypeButtonColor = 64L,
    dispidTypeButtonPicture = 65L,
    dispidTypeButtonPictureDown = 66L,
    dispidTypeButtonText = 67L,
    dispidTypeButtonTextColor = 68L,
    dispidTypeButtonType = 69L,
    dispidTypeCheckCenter = 70L,
    dispidTypeCheckText = 71L,
    dispidTypeCheckTextAlign = 72L,
    dispidTypeComboBoxCurSel = 73L,
    dispidTypeComboBoxEditable = 74L,
    dispidTypeComboBoxList = 75L,
    dispidTypeDateCentury = 76L,
    dispidTypeDateFormat = 77L,
    dispidTypeDateMax = 78L,
    dispidTypeDateMin = 79L,
    dispidTypeDateSeparator = 80L,
    dispidTypeEditCharCase = 81L,
    dispidTypeEditCharSet = 82L,
    dispidTypeEditLen = 83L,
    dispidTypeEditPassword = 84L,
    dispidTypeFloatCurrencyChar = 85L,
    dispidTypeFloatDecimalChar = 86L,
    dispidTypeFloatDecimalPlaces = 87L,
    dispidTypeFloatMax = 88L,
    dispidTypeFloatMin = 89L,
    dispidTypeFloatMoney = 90L,
    dispidTypeFloatSeparator = 91L,
    dispidTypeFloatSepChar = 92L,
    dispidTypeHAlign = 93L,
    dispidTypePicDefaultText = 94L,
    dispidTypePicMask = 95L,
    dispidTypePictCenter = 96L,
    dispidTypePictMaintainScale = 97L,
    dispidTypePictPicture = 98L,
    dispidTypePictStretch = 99L,
    dispidTypeTextAlignVert = 100L,
    dispidTypeTextPrefix = 101L,
    dispidTypeTextShadow = 102L,
    dispidTypeTextShadowIn = 103L,
    dispidTypeTextWordWrap = 104L,
    dispidTypeTime24Hour = 105L,
    dispidTypeTimeMax = 106L,
    dispidTypeTimeMin = 107L,
    dispidTypeTimeSeconds = 108L,
    dispidTypeTimeSeparator = 109L,
    dispidUnitType = 110L,
    dispidUserResize = 111L,
    dispidValue = 112L,
    dispidVirtualMaxRows = 113L,
    dispidVirtualMode = 114L,
    dispidVScrollSpecial = 115L,
    dispidVScrollSpecialType = 116L,
    dispidFontBold = 117L,
    dispidFontItalic = 118L,
    dispidFontName = 119L,
    dispidFontSize = 120L,
    dispidFontStrikethru = 121L,
    dispidFontUnderline = 122L,
    dispidBackColorStyle = 123L,
    dispidColWidth = 124L,
    dispidRowHeight = 125L,
    dispidTypeCheckPicture = 126L,
    dispidGetCellDirtyFlag = 127L,
    dispidGetDataFillData = 128L,
    dispidGetText = 129L,
    dispidLoadFromFile = 130L,
    dispidLoadTabFile = 131L,
    dispidSaveTabFile = 132L,
    dispidSaveToFile = 133L,
    dispidSetDataFillData = 134L,
    dispidSetCellDirtyFlag = 1322L,
    dispidSetText = 136L,
    dispidTypeCheckType = 137L,
    eventidAdvance = 1L,
    eventidBlockSelected = 2L,
    eventidButtonClicked = 3L,
    eventidChange = 4L,                                                                               
    eventidClick = 5L,
    eventidColWidthChange = 6L,
    eventidDataAddNew = 7L,
    eventidDataColConfig = 8L,
    eventidDataFill = 9L,
    eventidDblClick = 10L,
    eventidEditError = 11L,
    eventidEditMode = 12L,
    eventidLeaveCell = 13L,
    eventidQueryData = 14L,
    eventidRightClick = 15L,
    eventidRowHeightChange = 16L,
    eventidSelChange = 17L,
    eventidTopLeftChange = 18L,
    eventidVirtualClearData = 19L,
#endif
#ifdef SS_V35
    dispidCellNote = 318,
    dispidUserColAction = 319L,
    dispidShowScrollTips = 320L,
    dispidCellNoteIndicator = 321L,
    dispidColUserSortIndicator = 322L,
    dispidIsFetchCellNote = 323,
  	dispidExportToXML = 324,
    dispidExportRangeToXML = 325,
    dispidExportToXMLBuffer = 326L,
    dispidExportRangeToXMLBuffer = 327L,
    dispidOwnerPrintDraw = 328L,
    dispidOwnerPrintPageCount = 329L,
    eventidBeforeUserSort = 40L,
    eventidAfterUserSort = 41L,
#endif //SS_V35
*/

#if defined(SS_SPREADOCX)
    dispidAction = 1L,
    dispidActiveCol = 2L,
    dispidActiveRow = 3L,
    dispidAllowCellOverflow = 4L,
    dispidAllowDragDrop = 5L,
    dispidAllowMultiBlocks = 6L,
    dispidAllowUserFormulas = 7L,
    dispidArrowsExitEditMode = 8L,
    dispidAutoCalc = 9L,
    dispidAutoClipboard = 10L,
    dispidAutoSize = 11L,
    dispidBlockMode = 12L,
    dispidButtonDrawMode = 13L,
    dispidCalcDependencies = 14L,
    dispidCellBorderColor = 15L,
    dispidCellBorderStyle = 16L,
    dispidCellBorderType = 17L,
    dispidCellType = 18L,
    dispidChangeMade = 19L,
    dispidClip = 20L,
    dispidClipValue = 21L,
    dispidCol = 22L,
    dispidCol2 = 23L,
    dispidColHeaderDisplay = 24L,
    dispidColHidden = 25L,
    dispidColPageBreak = 26L,
    dispidColsFrozen = 27L,
    dispidCursorStyle = 28L,
    dispidCursorType = 29L,
    dispidDataChanged = 30L,    //DBOCX
    dispidDataColCnt = 31L,
    dispidDataField  = 32L,     //DBOCX
    dispidDataFillEvent = 33L,  //DBOCX
    dispidDataRowCnt = 34L,
    dispidDataSource = 35L,     //DBOCX
    dispidDAutoCellTypes = 36L,
    dispidDAutoFill = 37L,
    dispidDAutoHeadings = 38L,
    dispidDAutoSave = 39L,
    dispidDAutoSizeCols = 40L,
    dispidDestCol = 41L,
    dispidDestRow = 42L,
    dispidDInformActiveRowChange = 43L,
    dispidDisplayColHeaders = 44L,
    dispidDisplayRowHeaders = 45L,
    dispidEditEnterAction = 46L,
    dispidEditMode = 47L,
    dispidEditModePermanent = 48L,
    dispidEditModeReplace = 49L,
    dispidFileNum = 50L,
    dispidFloatDefCurrencyChar = 51L,
    dispidFloatDefDecimalChar = 52L,
    dispidFloatDefSepChar = 53L,
    dispidFormula = 54L,
    dispidGrayAreaBackColor = 55L,
    dispidGridColor = 56L,
    dispidGridShowHoriz = 57L,
    dispidGridShowVert = 58L,
    dispidGridSolid = 59L,
    dispidHDCPrinter = 60L,
    dispidInterfaceDesigner = 61L,
    dispidIsBlockSelected = 62L,
    dispidLeftCol = 63L,
    dispidLock = 64L,
    dispidLockBackColor = 65L,
    dispidLockForeColor = 66L,
    dispidMaxCols = 67L,
    dispidMaxRows = 68L,
    dispidMaxTextCellHeight = 69L,
    dispidMaxTextCellWidth = 70L,
    dispidMoveActiveOnFocus = 71L,
    dispidMultiSelCount = 72L,
    dispidMultiSelIndex = 73L,
    dispidNoBeep = 74L,
    dispidNoBorder = 75L,
    dispidOperationMode = 76L,
    dispidPosition = 77L,
    dispidPrintAbortMsg = 78L,
    dispidPrintBorder = 79L,
    dispidPrintColHeaders = 80L,
    dispidPrintColor = 81L,
    dispidPrintFooter = 82L,
    dispidPrintGrid = 83L,
    dispidPrintHeader = 84L,
    dispidPrintJobName = 85L,
    dispidPrintMarginBottom = 86L,
    dispidPrintMarginLeft = 87L,
    dispidPrintMarginRight = 88L,
    dispidPrintMarginTop = 89L,
    dispidPrintPageEnd = 90L,
    dispidPrintPageStart = 91L,
    dispidPrintRowHeaders = 92L,
    dispidPrintShadows = 93L,
    dispidPrintType = 94L,
    dispidPrintUseDataMax = 95L,
    dispidProcessTab = 96L,
    dispidProtect = 97L,
    dispidReDraw = 98L,
    dispidRestrictCols = 99L,
    dispidRestrictRows = 100L,
    dispidRetainSelBlock = 101L,
    dispidRow = 102L,
    dispidRow2 = 103L,
    dispidRowHeaderDisplay = 104L,
    dispidRowHidden = 105L,
    dispidRowPageBreak = 106L,
    dispidRowsFrozen = 107L,
    dispidScrollBarExtMode = 108L,
    dispidScrollBarMaxAlign = 109L,
    dispidScrollBars = 110L,
    dispidScrollBarShowMax = 111L,
    dispidSelBlockCol = 112L,
    dispidSelBlockCol2 = 113L,
    dispidSelBlockRow = 114L,
    dispidSelBlockRow2 = 115L,
    dispidSelectBlockOptions = 116L,
    dispidSelLength = 117L,
    dispidSelModeIndex = 118L,
    dispidSelModeSelCount = 119L,
    dispidSelModeSelected = 120L,
    dispidSelStart = 121L,
    dispidSelText = 122L,
    dispidShadowColor = 123L,
    dispidShadowDark = 124L,
    dispidShadowText = 125L,
    dispidSortBy = 126L,
    dispidStartingColNumber = 127L,
    dispidStartingRowNumber = 128L,
    dispidTopRow = 129L,
    dispidTypeButtonAlign = 130L,
    dispidTypeButtonBorderColor = 131L,
    dispidTypeButtonColor = 132L,
    dispidTypeButtonDarkColor = 133L,
    dispidTypeButtonLightColor = 134L,
    dispidTypeButtonPicture = 135L,
    dispidTypeButtonPictureDown = 136L,
    dispidTypeButtonShadowSize = 137L,
    dispidTypeButtonText = 138L,
    dispidTypeButtonTextColor = 139L,
    dispidTypeButtonType = 140L,
    dispidTypeCheckCenter = 141L,
    dispidTypeCheckText = 142L,
    dispidTypeCheckTextAlign = 143L,
    dispidTypeComboBoxCount = 144L,
    dispidTypeComboBoxCurSel = 145L,
    dispidTypeComboBoxEditable = 146L,
    dispidTypeComboBoxIndex = 147L,
    dispidTypeComboBoxList = 148L,
    dispidTypeComboBoxString = 149L,
    dispidTypeDateCentury = 150L,
    dispidTypeDateFormat = 151L,
    dispidTypeDateMax = 152L,
    dispidTypeDateMin = 153L,
    dispidTypeDateSeparator = 154L,
    dispidTypeEditCharCase = 155L,
    dispidTypeEditCharSet = 156L,
    dispidTypeEditLen = 157L,
    dispidTypeEditMultiLine = 158L,
    dispidTypeEditPassword = 159L,
    dispidTypeFloatCurrencyChar = 160L,
    dispidTypeFloatDecimalChar = 161L,
    dispidTypeFloatDecimalPlaces = 162L,
    dispidTypeFloatMax = 163L,
    dispidTypeFloatMin = 164L,
    dispidTypeFloatMoney = 165L,
    dispidTypeFloatSeparator = 166L,
    dispidTypeFloatSepChar = 167L,
    dispidTypeHAlign = 168L,
    dispidTypeIntegerMax = 169L,
    dispidTypeIntegerMin = 170L,
    dispidTypeIntegerSpinInc = 171L,
    dispidTypeIntegerSpinWrap = 172L,
    dispidTypeOwnerDrawStyle = 173L,
    dispidTypePicDefaultText = 174L,
    dispidTypePicMask = 175L,
    dispidTypePictCenter = 176L,
    dispidTypePictMaintainScale = 177L,
    dispidTypePictPicture = 178L,
    dispidTypePictStretch = 179L,
    dispidTypeSpin = 180L,
    dispidTypeTextAlignVert = 181L,
    dispidTypeTextPrefix = 182L,
    dispidTypeTextShadow = 183L,
    dispidTypeTextShadowIn = 184L,
    dispidTypeTextWordWrap = 185L,
    dispidTypeTime24Hour = 186L,
    dispidTypeTimeMax = 187L,
    dispidTypeTimeMin = 188L,
    dispidTypeTimeSeconds = 189L,
    dispidTypeTimeSeparator = 190L,
    dispidUnitType = 191L,
    dispidUserResize = 192L,
    dispidUserResizeCol = 193L,
    dispidUserResizeRow = 194L,
    dispidValue = 195L,
    dispidVirtualCurRowCount = 196L,
    dispidVirtualCurTop = 197L,
    dispidVirtualMaxRows = 198L,
    dispidVirtualMode = 199L,
    dispidVirtualOverlap = 200L,
    dispidVirtualRows = 201L,
    dispidVirtualScrollBuffer = 202L,
    dispidVisibleCols = 203L,
    dispidVisibleRows = 204L,
    dispidVScrollSpecial = 205L,
    dispidVScrollSpecialType = 206L,
    dispidFontBold = 207L,
    dispidFontItalic = 208L,
    dispidFontName = 209L,
    dispidFontSize = 210L,
    dispidFontStrikethru = 211L,
    dispidFontUnderline = 212L,
    dispidCursorIcon = 213L,
    dispidPrintOrientation = 214L,
    dispidBackColorStyle = 215L,
    dispidFormulaSync = 216L,
    dispidColWidth = 217L,
    dispidMaxTextColWidth = 218L,
    dispidMaxTextRowHeight = 219L,
    dispidRowHeight = 220L,
    dispidSortKey = 221L,
    dispidSortKeyOrder = 222L,
    dispidTypeCheckPicture = 223L,
    dispidAddCustomFunction = 224L,
    dispidAddCustomFunctionExt = 225L,
    dispidCFGetCellParam = 226L,
    dispidCFGetDoubleParam = 227L,
    dispidCFGetDoubleParamExt = 228L,
    dispidCFGetLongParam = 229L,
    dispidCFGetParamInfo = 230L,
    dispidCFGetRangeParam = 231L,
    dispidCFGetStringParam = 232L,
    dispidCFSetResult = 233L,
    dispidColNumberToLetter = 234L,
    dispidColWidthToTwips = 235L,
    dispidGetBottomRightCell = 236L,
    dispidGetCellDirtyFlag = 237L,
    dispidGetCellFromScreenCoord = 238L,
    dispidGetCellPos = 239L,
    dispidGetClientArea = 240L,
    dispidGetColItemData = 241L,
    dispidQueryCustomName = 242L,
    dispidGetCustomName = 243L,
    dispidGetDataFillData = 244L,
    dispidGetFirstValidCell = 245L,
    dispidGetItemData = 246L,
    dispidGetIteration = 247L,
    dispidGetLastValidCell = 248L,
    dispidGetMultiSelItem = 249L,
    dispidGetRefStyle = 250L,
    dispidGetRowItemData = 251L,
    dispidGetText = 252L,
    dispidIsCellSelected = 253L,
    dispidIsFormulaValid = 254L,
    dispidIsVisible = 255L,
    dispidLoadFromFile = 256L,
    dispidLoadTabFile = 257L,
    dispidRowHeightToTwips = 258L,
    dispidSaveTabFile = 259L,
    dispidSaveToFile = 260L,
    dispidSetCellDirtyFlag = 261L,
    dispidSetColItemData = 262L,
    dispidSetCustomName = 263L,
    dispidSetDataFillData = 264L,
    dispidSetItemData = 265L,
    dispidSetIteration = 266L,
    dispidSetRefStyle = 267L,
    dispidSetRowItemData = 268L,
    dispidSetText = 269L,
    dispidTwipsToColWidth = 270L,
    dispidTwipsToRowHeight = 271L,
    dispidTypeCheckType = 272L,
    
    dispidGetActionKey = 292L,
    dispidSetActionKey = 293L,
    dispidGetOddEvenRowColor = 294L,
    dispidSetOddEvenRowColor = 295L,
    dispidGetFloat = 296L,
    dispidSetFloat = 297L,
    dispidGetInteger = 298L,
    dispidSetInteger = 299L,
    dispidSetCalText = 300L,
    dispidGetTextTipAppearance = 301L,
    dispidSetTextTipAppearance = 302L,
    dispidExportToHTML = 303L,
    dispidExportRangeToHTML = 304L,
    dispidGetArray = 305L,
    dispidSetArray = 306L,
    dispidIsExcelFile = 307L,
    dispidGetExcelSheetList = 308L,
    dispidImportExcelSheet = 309L,
    dispidExportToExcel = 310L,
    dispidEnumCustomFunction = 311L,
    dispidGetCustomFunction = 312L,
    dispidRemoveCustomFunction = 313L,
    dispidReCalcCell = 314L,

    #ifdef SS_V30
    dispidAppearance = 273L,
    dispidTypeComboBoxMaxDrop = 274L,
    dispidTypeMaxEditLen = 275L,
    dispidTypeComboBoxWidth = 276L,
    dispidTextTip = 277L,
    dispidTextTipDelay = 278L,
    dispidScrollBarTrack = 279L,
    dispidTypeVAlign = 280L,
    dispidClipboardOptions = 281L,
    dispidPrintSmartPrint = 282L,
    dispidPrintPageOrder = 283L,
    dispidPrintPageCount = 284L,
    dispidPrintFirstPageNumber = 285L,
    dispidPrintNextPageBreakCol = 286L,
    dispidPrintNextPageBreakRow = 287L,
    dispidTypeComboBoxhWnd = 288L,
    dispidSelBackColor = 289L,
    dispidSelForeColor = 290L,
    dispidTwoDigitYearMax = 291L,
    dispidDataMember = 317L,
    #endif // SS_V30

    eventidAdvance = 1L,
    eventidBlockSelected = 2L,
    eventidButtonClicked = 3L,
    eventidChange = 4L,
    eventidClick = 5L,
    eventidColWidthChange = 6L,
    eventidCustomFunction = 7L,
    eventidDataAddNew = 8L,
    eventidDataColConfig = 9L,
    eventidDataFill = 10L,
    eventidDblClick = 11L,
    eventidDragDropBlock = 12L,
    eventidDrawItem = 13L,
    eventidEditError = 14L,
    eventidEditMode = 15L,
    eventidEnterRow = 16L,
    eventidLeaveCell = 17L,
    eventidLeaveRow = 18L,
    eventidPrintAbort = 19L,
    eventidQueryAdvance = 20L,
    eventidQueryData = 21L,
    eventidRightClick = 22L,
    eventidRowHeightChange = 23L,
    eventidSelChange = 24L,
    eventidTopLeftChange = 25L,
    eventidUserFormulaEntered = 26L,
    eventidVirtualClearData = 27L,
    eventidPrintMsgBox = 28L,
    eventidComboCloseUp = 29L,
    eventidComboDropDown = 30L,
    eventidComboSelChange = 31L,
    eventidTextTipFetch = 32L,
    eventidEditChange = 33L,
#ifdef SS_V35
    dispidCellNote = 318,
    dispidUserColAction = 319L,
    dispidShowScrollTips = 320L,
    dispidCellNoteIndicator = 321L,
    dispidColUserSortIndicator = 322L,
    dispidIsFetchCellNote = 323,
  	 dispidExportToXML = 324,
    dispidExportRangeToXML = 325,
    dispidExportToXMLBuffer = 326L,
    dispidExportRangeToXMLBuffer = 327L,
    dispidOwnerPrintDraw = 328L,
    dispidOwnerPrintPageCount = 329L,
    dispidTypeEllipses = 330L,
    dispidScriptEnhanced = 336L,
    dispidSelectionCount = 489L,
    eventidBeforeUserSort = 40L,
    eventidAfterUserSort = 41L,
	 dispidExportRangeToTextFile = 331L,
 	 dispidExportToTextFile = 332L,
 	 dispidLoadTextFile = 333L,
    dispidSetCalTextOverride = 334L,
    dispidGetCalTextOverride = 335L,
#endif //SS_V35
#ifdef SS_V40
    dispidColMerge = 337L,
    dispidRowMerge = 338L,
    dispidColID = 339L,
    dispidRowHeaderCols = 340L,
    dispidRowHeadersAutoTextIndex = 341L,
    dispidColHeaderRows = 342L,
    dispidColHeadersAutoTextIndex = 343L,
    dispidColHeadersUserSortIndex = 344L,
    dispidTypeComboBoxAutoSearch = 345L,
    dispidTypeSpinInc = 346L,
    dispidTypeSpinWrap = 347L,
    dispidTypeNegRed = 348L,
    dispidTypeCurrencyMin = 349L,
    dispidTypeCurrencyMax = 350L,
    dispidTypeCurrencyDecimal = 351L,
    dispidTypeCurrencySeparator = 352L,
    dispidTypeCurrencySymbol = 353L,
    dispidTypeCurrencyLeadingZero = 354L,
    dispidTypeCurrencyDecPlaces = 355L,
    dispidTypeCurrencyNegStyle = 356L,
    dispidTypeCurrencyPosStyle = 357L,
    dispidTypeCurrencyShowSep = 358L,
    dispidTypeCurrencyShowSymbol = 359L,
    dispidTypeNumberMin = 360L,
    dispidTypeNumberMax = 361L,
    dispidTypeNumberDecimal = 362L,
    dispidTypeNumberSeparator = 363L,
    dispidTypeNumberLeadingZero = 364L,
    dispidTypeNumberDecPlaces = 365L,
    dispidTypeNumberNegStyle = 366L,
    dispidTypeNumberShowSep = 367L,
    dispidTypePercentMin = 368L,
    dispidTypePercentMax = 369L,
    dispidTypePercentDecimal = 370L,
    dispidTypePercentDecPlaces = 371L,
    dispidTypePercentNegStyle = 372L,
    dispidTypeTextOrient = 373L,
    dispidCellTag = 374L,
    dispidAllowEditOverflow = 375L,
    dispidColHeadersAutoText = 376L,
    dispidColHeadersShow = 377L,
    dispidRowHeadersAutoText = 378L,
    dispidRowHeadersShow = 379L,
    dispidTypePercentLeadingZero = 380L,
#endif // SS_V40
#ifdef SS_V70
    dispidSheetCount = 381L,
    dispidActiveSheet = 382L,
    dispidTabStripLeftSheet = 383L,
    dispidTabStripFont = 384L,
    dispidSheetName = 385L,
    dispidTabStripPolicy = 386L,
    dispidTabStripRatio = 387L,
    dispidPrintZoomFactor = 388L,
    dispidScrollBarWidth = 389L,
    dispidScrollBarHeight = 390L,
    dispidActiveCellHighlightStyle = 391L,
    dispidPrintScalingMethod = 392L,
    dispidPrintBestFitPagesWide = 393L,
    dispidPrintBestFitPagesTall = 394L,
    dispidPrintCenterOnPageH = 395L,
    dispidPrintCenterOnPageV = 396L,
    dispidAllowColMove = 397L,
    dispidAllowRowMove = 398L,
    dispidTypeScientificMin = 399L,
    dispidTypeScientificMax = 400L,
    dispidTypeScientificDecimal = 401L,
    dispidTypeScientificDecPlaces = 402L,
    dispidScrollBarHColor = 403L,
    dispidScrollBarVColor = 404L,
    dispidSheet = 405L,
    dispidSheetSendingEvent = 406L,
    dispidSheetVisible = 407L,
#endif // SS_V70
#ifdef SS_V80
    dispidAppearanceStyle = 525L,
	dispidSetEnhancedSheetTabColors = 526L,
	dispidSetEnhancedScrollBarColors = 527L,
	dispidSetEnhancedColumnHeaderColors = 528L,
	dispidSetEnhancedRowHeaderColors = 529L,
	dispidSetEnhancedCornerColors = 530L,
	dispidIsVerticalScrollBarVisible = 531L,
	dispidIsHorizontalScrollBarVisible = 532L,
	dispidTabEnhancedShape = 533L,
	dispidCellNoteIndicatorColor = 534L,
	dispidCellNoteIndicatorShape = 535L,
   dispidExportToHTMLEx = 536L,
   dispidExportRangeToHTMLEx = 537L,
	dispidTabStripButtonPolicy = 538L,
	dispidHighlightHeaders = 539L,
	dispidHighlightStyle = 540L,
	dispidSaveToBuffer = 541L,
	dispidLoadFromBuffer = 542L,
	dispidSaveBlockToBuffer = 543L,
	dispidLoadBlockFromBuffer = 544L,
	dispidHighlightAlphaBlendColor = 545L,
	dispidHighlightAlphaBlend = 546L,
   dispidAddCustomCellType = 547L,
   dispidRemoveCustomCellType = 548L,
   dispidEnumCustomCellType = 549L,
   dispidGetCustomCellType = 550L,
   dispidTypeCustomName = 551L,
   dispidTypeCustomStyle = 552L,
   dispidGetCustomEditor = 553L,
   dispidGetCustomRenderer = 554L,
   dispidPaintControl = 555L,
   dispidGetFontHandle = 556L,
   dispidTypeCustomItemData = 557L,
   dispidUseVisualStyles = 558L,
   dispidEnhanceStaticCells = 559L,
   dispidScrollBarStyle = 560L,
#ifdef XL12
    dispidOpenExcel2007File = 561L,
    dispidSaveExcel2007File = 562L,
    dispidIsExcel2007File = 563L,
#endif // XL12
#endif // SS_V80
/**/
#ifdef SS_V35
    dispidScriptCFGetCellParam = 435L,
    dispidScriptCFGetDoubleParamExt = 436L,
    dispidScriptCFGetParamInfo = 437L,
    dispidScriptCFGetRangeParam = 438L,
    dispidScriptColWidthToTwips = 439L,
    dispidScriptGetBottomRightCell = 440L,
    dispidScriptGetCellFromScreenCoord = 441L,
    dispidScriptGetCellPos = 442L,
    dispidScriptGetClientArea = 443L,
    dispidScriptGetFirstValidCell = 444L,
    dispidScriptGetIteration = 445L,
    dispidScriptGetLastValidCell = 446L,
    dispidScriptRowHeightToTwips = 447L,
    dispidScriptTwipsToColWidth = 448L,
    dispidScriptTwipsToRowHeight = 449L,
    dispidScriptGetActionKey = 450L,
    dispidScriptGetOddEvenRowColor = 451L,
    dispidScriptGetFloat = 452L,
    dispidScriptGetInteger = 453L,
    dispidScriptGetTextTipAppearance = 454L,
    dispidScriptGetExcelSheetList = 455L,
    dispidScriptEnumCustomFunction = 456L,
    dispidScriptGetCustomFunction = 457L,
    dispidScriptExportToXMLBuffer = 458L,
    dispidScriptExportRangeToXMLBuffer = 459L,
    dispidScriptOwnerPrintPageCount = 460L,
    dispidScriptGetCalTextOverride = 461L,
    dispidPrintSheet = 462L,
    dispidAddSelection = 463L,
    dispidClearRange = 464L,
    dispidClearSelection = 465L,
    dispidClipboardCopy = 466L,
    dispidClipboardCut = 467L,
    dispidClipboardPaste = 468L,
    dispidCopyRange = 469L,
    dispidDeleteCols = 470L,
    dispidDeleteRows = 471L,
    dispidDataSave = 472L,
    dispidGetCellBorder = 473L,
    dispidGetSelection = 474L,
    dispidInsertCols = 475L,
    dispidInsertRows = 476L,
    dispidMoveRange = 477L,
    dispidReCalc = 478L,
    dispidReset = 479L,
    dispidSetActiveCell = 480L,
    dispidSetCellBorder = 481L,
    dispidSetSelection = 482L,
    dispidShowCell = 483L,
    dispidSort = 484L,
    dispidSwapRange = 485L,
    dispidTypeComboBoxClear = 486L,
    dispidTypeComboBoxRemoveItem = 487L,
    dispidVirtualRefresh = 488L,

/**/
    eventidScriptCustomFunction = 101L,
    eventidScriptDataFill = 102L,
    eventidScriptDragDropBlock = 103L,
    eventidScriptLeaveCell = 104L,
    eventidScriptLeaveRow = 105L,
    eventidScriptPrintAbort = 106L,
    eventidScriptQueryAdvance = 107L,
    eventidScriptQueryData = 108L,
    eventidScriptPrintMsgBox = 109L,
    eventidScriptTextTipFetch = 110L,
    eventidScriptOLECompleteDrag = 111L,
    eventidScriptOLEDragDrop = 112L,
    eventidScriptOLEDragOver = 113L,
    eventidScriptOLEGiveFeedback = 114L,
    eventidScriptOLESetData = 115L,
// fix for bug 9184 -scl
//    eventidScriptOLEStartDrag = 116L,
    eventidScriptOLEStartDrag = 115L,
    eventidScriptBeforeUserSort = 117L,
    eventidScriptKeyDown = 118L,
    eventidScriptKeyPress = 119L,
    eventidScriptKeyUp = 120L,
/**/
#endif
#ifdef SS_V40
    dispidAddCellSpan = 490L,
    dispidGetCellSpan = 491L,
    dispidRemoveCellSpan = 492L,
    dispidGetColFromID = 493L,
    dispidCopyColRange = 494L,
    dispidCopyRowRange = 495L,
    dispidMoveColRange = 496L,
    dispidMoveRowRange = 497L,
    dispidSwapColRange = 498L,
    dispidSwapRowRange = 499L,
    dispidSearchCol = 500L,
    dispidSearchRow = 501L,
    dispidEventEnabled = 502L,
	 dispidExportRangeToTextFileU = 503L,
 	 dispidExportToTextFileU = 504L,
    dispidSaveTabFileU = 505L,
#endif // SS_V40

#ifdef SS_V70
    dispidColLetterToNumber = 506L,
    dispidDataRefresh = 507L,
    dispidImportExcelBook = 508L,
    dispidExportExcelBook = 509L,
    dispidInsertSheets = 510L,
    dispidDeleteSheets = 511L,
    dispidMoveSheets = 512L,
    dispidResetSheet = 513L,
    dispidLoadPicture = 514L,
    dispidLoadResPicture = 515L,
    dispidLoadPictureBuffer = 516L,
    dispidSavePicture = 517L,
    dispidSavePictureBuffer = 518L,
    dispidCFGetCellParamExt = 519L,
    dispidCFGetRangeParamExt = 520L,
    dispidScriptCFGetCellParamExt = 521L,
    dispidScriptCFGetRangeParamExt = 522L,
    dispidExportToExcelEx = 523L,
    dispidExportExcelBookEx = 524L,
#endif // SS_V70

#ifdef SS_V70
    eventidSheetChanging = 121L,
    eventidSheetChanged = 122L,
    eventidTabScrolling = 123L,
    eventidTabScrolled = 124L,
    eventidBeforeEditMode = 125L,
    eventidDataLoaded = 126L,
    eventidBeforeScrollTip = 127L,
    eventidBeforeColMove = 128L,
    eventidBeforeRowMove = 129L,
#endif // SS_V70

#ifdef SS_V80
    eventidCircularFormula = 130L,
    eventidCTCancelEditing = 131L,
    eventidCTCreateEditorControl = 132L,
    eventidCTCreateRendererControl = 133L,
    eventidCTGetEditorValue = 134L,
    eventidCTInitializeControl = 135L,
    eventidCTIsReservedKey = 136L,
    eventidCTIsReservedLocation = 137L,
    eventidCTIsValid = 138L,
    eventidCTGetPreferredSize = 139L,
    eventidCTGetReservedCursor = 140L,
    eventidCTPaintCell = 141L,
    eventidCTSetValue = 142L,
    eventidCTStartEditing = 143L,
    eventidCTStopEditing = 144L,
    eventidCTStringFormat = 145L,
    eventidCTStringUnformat = 146L,
#endif

/**/
#ifdef FP_OLEDRAG
	  dispidOLEDropMode = 315L,
	  dispidOLEDrag = 316L,
	  eventidOLECompleteDrag = 34L,
	  eventidOLEDragDrop = 35L,
	  eventidOLEDragOver = 36L,
	  eventidOLEGiveFeedback = 37L,
	  eventidOLESetData = 38L,
	  eventidOLEStartDrag = 39L,
#endif // FP_OLEDRAG
#endif //SS_SPREADOCX
  };

public: // ???? remove this
// Event maps
  //{{AFX_EVENT(CSsocxCtrl)
    //}}AFX_EVENT
  #ifndef SSE_NO_ADVANCE
    void FireAdvance(BOOL AdvanceNext)
        {if (m_bEventEnabled[EventEnabled_Advance]) FireEvent(eventidAdvance,EVENT_PARAM(VTS_BOOL), AdvanceNext);}
  #endif
  #ifndef SSE_NO_BLOCKSELECTED
    void FireBlockSelected(long BlockCol, long BlockRow, long BlockCol2, long BlockRow2)
        {if (m_bEventEnabled[EventEnabled_BlockSelected]) FireEvent(eventidBlockSelected,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4), BlockCol, BlockRow, BlockCol2, BlockRow2);}
  #endif
  #ifndef SSE_NO_BUTTONCLICKED
    void FireButtonClicked(long Col, long Row, short ButtonDown)
        {if (m_bEventEnabled[EventEnabled_ButtonClicked]) FireEvent(eventidButtonClicked,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I2), Col, Row, ButtonDown);}
  #endif
  #ifndef SSE_NO_CHANGE
    void FireChange(long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_Change]) FireEvent(eventidChange,EVENT_PARAM(VTS_I4  VTS_I4), Col, Row);}
  #endif
  #ifndef SSE_NO_CLICK
    void FireClick(long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_Click]) FireEvent(eventidClick,EVENT_PARAM(VTS_I4  VTS_I4), Col, Row);}
  #endif
  #ifndef SSE_NO_COLWIDTHCHANGE
    void FireColWidthChange(long Col1, long Col2)
        {if (m_bEventEnabled[EventEnabled_ColWidthChange]) FireEvent(eventidColWidthChange,EVENT_PARAM(VTS_I4  VTS_I4), Col1, Col2);}
  #endif
  #ifndef SSE_NO_CUSTOMFUNCTION
    void FireCustomFunction(LPCTSTR FunctionName, short ParameterCnt, long Col, long Row, short FAR* Status)
#ifdef SS_V70
	{
		if (m_bEventEnabled[EventEnabled_CustomFunction])
		{
			long status = (long)*Status;
			FireEvent(eventidCustomFunction,EVENT_PARAM(VTS_BSTR  VTS_I2  VTS_I4  VTS_I4  VTS_PI4), FunctionName, ParameterCnt, Col, Row, &status);
			*Status = (short)status;
		}
	}
#else
        {if (m_bEventEnabled[EventEnabled_CustomFunction]) FireEvent(eventidCustomFunction,EVENT_PARAM(VTS_BSTR  VTS_I2  VTS_I4  VTS_I4  VTS_PI2), FunctionName, ParameterCnt, Col, Row, Status);}
#endif
  #endif
  #ifndef SSE_NO_DATAADDNEW
    void FireDataAddNew()
        {FireEvent(eventidDataAddNew,EVENT_PARAM(VTS_NONE));}
  #endif
  #ifndef SSE_NO_DATACOLCONFIG
    void FireDataColConfig(long Col, LPCTSTR DataField, short DataType)
        {if (m_bEventEnabled[EventEnabled_DataColConfig]) FireEvent(eventidDataColConfig,EVENT_PARAM(VTS_I4  VTS_BSTR  VTS_I2), Col, DataField, DataType);}
  #endif
  #ifndef SSE_NO_DATAFILL
    void FireDataFill(long Col, long Row, short DataType, short fGetData, short FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_DataFill]) FireEvent(eventidDataFill,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I2  VTS_I2  VTS_PI2), Col, Row, DataType, fGetData, Cancel);}
  #endif
  #ifndef SSE_NO_DBLCLICK
    void FireDblClick(long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_DblClick]) FireEvent(eventidDblClick,EVENT_PARAM(VTS_I4  VTS_I4), Col, Row);}
  #endif
  #ifndef SSE_NO_DRAGDROPBLOCK
    void FireDragDropBlock(long Col, long Row, long Col2, long Row2, long NewCol, long NewRow, long NewCol2, long NewRow2, BOOL Overwrite, short FAR* Action, BOOL FAR* DataOnly, BOOL FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_DragDropBlock]) FireEvent(eventidDragDropBlock,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_BOOL  VTS_PI2  VTS_PBOOL  VTS_PBOOL), Col, Row, Col2, Row2, NewCol, NewRow, NewCol2, NewRow2, Overwrite, Action, DataOnly, 
        Cancel);}
  #endif
  #ifndef SSE_NO_DRAWITEM
    void FireDrawItem(long Col, long Row, OLE_HANDLE hDC, long Left, long Top, long Right, long Bottom, long Style)
        {if (m_bEventEnabled[EventEnabled_DrawItem]) FireEvent(eventidDrawItem,EVENT_PARAM(VTS_I4  VTS_I4  VTS_HANDLE  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4), Col, Row, hDC, Left, Top, Right, Bottom, Style);}
  #endif
  #ifndef SSE_NO_EDITERROR
    void FireEditError(long Col, long Row, short EditError)
        {if (m_bEventEnabled[EventEnabled_EditError]) FireEvent(eventidEditError,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I2), Col, Row, EditError);}
  #endif
  #ifndef SSE_NO_EDITMODE
    void FireEditMode(long Col, long Row, short Mode, BOOL ChangeMade)
        {if (m_bEventEnabled[EventEnabled_EditMode]) FireEvent(eventidEditMode,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I2  VTS_BOOL), Col, Row, Mode, ChangeMade);}
  #endif
  #ifndef SSE_NO_ENTERROW
    void FireEnterRow(long Row, long RowIsLast)
        {if (m_bEventEnabled[EventEnabled_EnterRow]) FireEvent(eventidEnterRow,EVENT_PARAM(VTS_I4  VTS_I4), Row, RowIsLast);}
  #endif
  #ifndef SSE_NO_LEAVECELL
    void FireLeaveCell(long Col, long Row, long NewCol, long NewRow, BOOL FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_LeaveCell]) FireEvent(eventidLeaveCell,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_PBOOL), Col, Row, NewCol, NewRow, Cancel);}
  #endif
  #ifndef SSE_NO_LEAVEROW
    void FireLeaveRow(long Row, BOOL RowWasLast, BOOL RowChanged, BOOL AllCellsHaveData, long NewRow, long NewRowIsLast, BOOL FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_LeaveRow]) FireEvent(eventidLeaveRow,EVENT_PARAM(VTS_I4  VTS_BOOL  VTS_BOOL  VTS_BOOL  VTS_I4  VTS_I4  VTS_PBOOL), Row, RowWasLast, RowChanged, AllCellsHaveData, NewRow, NewRowIsLast, Cancel);}
  #endif
  #ifndef SSE_NO_LINKDISPATCHNOTCREATED
    void FireLinkDispatchNotCreated(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkDispatchNotCreated])  FireEvent(eventidLinkDispatchNotCreated, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_LINKERROR
    void FireLinkError(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, short ErrorNum, LPCTSTR ErrorString)
        {if (m_bEventEnabled[EventEnabled_LinkError])  FireEvent(eventidLinkError, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR VTS_I2 VTS_BSTR), Server, Topic, Item, ErrorNum, ErrorString);}
  #endif
  #ifndef SSE_NO_LINKITEMNOTSUPPORTED
    void FireLinkItemNotSupported(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkItemNotSupported])  FireEvent(eventidLinkItemNotSupported, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_LINKITEMSUPPORTED
    void FireLinkItemSupported(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkItemSupported])  FireEvent(eventidLinkItemSupported, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_LINKNEWDATA
    void FireLinkNewData(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkNewData])  FireEvent(eventidLinkNewData, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_LINKNOTIFY
    void FireLinkNotify(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkNotify])  FireEvent(eventidLinkNotify, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_LINKOUTOFMEMORY
    void FireLinkOutOfMemory(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkOutOfMemory])  FireEvent(eventidLinkOutOfMemory, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_LINKPOKECOMPLETED
    void FireLinkPokeCompleted(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long Code)
        {if (m_bEventEnabled[EventEnabled_LinkPokeCompleted])  FireEvent(eventidLinkPokeCompleted, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4), Server, Topic, Item, Code);}
  #endif
  #ifndef SSE_NO_LINKREQUESTCOMPLETED
    void FireLinkRequestCompleted(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item, long Code)
        {if (m_bEventEnabled[EventEnabled_LinkRequestCompleted])  FireEvent(eventidLinkRequestCompleted, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4), Server, Topic, Item, Code);}
  #endif
  #ifndef SSE_NO_LINKSERVERDISCONNECTED
    void FireLinkServerDisconnected(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkServerDisconnected])  FireEvent(eventidLinkServerDisconnected, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_LINKUNABLETOCONNECTTOSERVER
    void FireLinkUnableToConnectToServer(LPCTSTR Server, LPCTSTR Topic, LPCTSTR Item)
        {if (m_bEventEnabled[EventEnabled_LinkUnableToConnectToServer])  FireEvent(eventidLinkUnableToConnectToServer, EVENT_PARAM(VTS_BSTR VTS_BSTR VTS_BSTR), Server, Topic, Item);}
  #endif
  #ifndef SSE_NO_PRINTABORT
    void FirePrintAbort(BOOL FAR* Abort)
        {if (m_bEventEnabled[EventEnabled_PrintAbort]) FireEvent(eventidPrintAbort,EVENT_PARAM(VTS_PBOOL), Abort);}
  #endif
  #ifndef SSE_NO_PRINTMSGBOX
    void FirePrintMsgBox(LPCTSTR Text, long PrintID, BOOL OpenMsgBox, BOOL FAR* Processed)
        {if (m_bEventEnabled[EventEnabled_PrintMsgBox]) FireEvent(eventidPrintMsgBox,EVENT_PARAM(VTS_BSTR VTS_I4 VTS_BOOL VTS_PBOOL), Text, PrintID, OpenMsgBox, Processed);}
  #endif
  #ifndef SSE_NO_QUERYADVANCE
    void FireQueryAdvance(BOOL AdvanceNext, BOOL FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_QueryAdvance]) FireEvent(eventidQueryAdvance,EVENT_PARAM(VTS_BOOL  VTS_PBOOL), AdvanceNext, Cancel);}
  #endif
  #ifndef SSE_NO_QUERYDATA
    void FireQueryData(long Row, long RowsNeeded, long FAR* RowsLoaded, short Direction, BOOL FAR* AtTop, BOOL FAR* AtBottom)
        {if (m_bEventEnabled[EventEnabled_QueryData]) FireEvent(eventidQueryData,EVENT_PARAM(VTS_I4  VTS_I4  VTS_PI4  VTS_I2  VTS_PBOOL  VTS_PBOOL), Row, RowsNeeded, RowsLoaded, Direction, AtTop, AtBottom);}
  #endif
  #ifndef SSE_NO_RIGHTCLICK
    void FireRightClick(short ClickType, long Col, long Row, long MouseX, long MouseY)
        {if (m_bEventEnabled[EventEnabled_RightClick]) FireEvent(eventidRightClick,EVENT_PARAM(VTS_I2  VTS_I4  VTS_I4  VTS_I4  VTS_I4), ClickType, Col, Row, MouseX, MouseY);}
  #endif
  #ifndef SSE_NO_ROWHEIGHTCHANGE
    void FireRowHeightChange(long Row1, long Row2)
        {if (m_bEventEnabled[EventEnabled_RowHeightChange]) FireEvent(eventidRowHeightChange,EVENT_PARAM(VTS_I4  VTS_I4), Row1, Row2);}
  #endif
  #ifndef SSE_NO_SELCHANGE
    void FireSelChange(long BlockCol, long BlockRow, long BlockCol2, long BlockRow2, long CurCol, long CurRow)
        {if (m_bEventEnabled[EventEnabled_SelChange]) FireEvent(eventidSelChange,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4), BlockCol, BlockRow, BlockCol2, BlockRow2, CurCol, CurRow);}
  #endif
  #ifndef SSE_NO_TOPLEFTCHANGE
    void FireTopLeftChange(long OldLeft, long OldTop, long NewLeft, long NewTop)
        {if (m_bEventEnabled[EventEnabled_TopLeftChange]) FireEvent(eventidTopLeftChange,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4), OldLeft, OldTop, NewLeft, NewTop);}
  #endif
  #ifndef SSE_NO_USERFORMULAENTERED
    void FireUserFormulaEntered(long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_UserFormulaEntered]) FireEvent(eventidUserFormulaEntered,EVENT_PARAM(VTS_I4  VTS_I4), Col, Row);}
  #endif
  #ifndef SSE_NO_VIRTUALCLEARDATA
    void FireVirtualClearData(long Row, long RowsBeingCleared)
        {if (m_bEventEnabled[EventEnabled_VirtualClearData]) FireEvent(eventidVirtualClearData,EVENT_PARAM(VTS_I4  VTS_I4), Row, RowsBeingCleared);}
  #endif
  #ifdef SS_V30
    void FireComboCloseUp(long Col, long Row, short SelChange)
        {if (m_bEventEnabled[EventEnabled_ComboCloseUp]) FireEvent(eventidComboCloseUp,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I2), Col, Row, SelChange);}
    void FireComboDropDown(long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_ComboDropDown]) FireEvent(eventidComboDropDown,EVENT_PARAM(VTS_I4  VTS_I4), Col, Row);}
    void FireComboSelChange(long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_ComboSelChange]) FireEvent(eventidComboSelChange,EVENT_PARAM(VTS_I4  VTS_I4), Col, Row);}
    void FireTextTipFetch(long Col, long Row, short FAR *MultiLine, long FAR *TipWidth, BSTR FAR *TextTip, BOOL FAR *Show)
#ifdef SS_V70
	{
		if (m_bEventEnabled[EventEnabled_TextTipFetch])
		{
			long multiline = (long)*MultiLine;
			// RFW - 12/28/04 - 13494
			FireEvent(eventidTextTipFetch,EVENT_PARAM(VTS_I4  VTS_I4  VTS_PI4  VTS_PI4  VTS_PBSTR  VTS_PBOOL), Col, Row, &multiline, TipWidth, TextTip, Show);
			*MultiLine = (short)multiline;
		}
	}
#else
        {if (m_bEventEnabled[EventEnabled_TextTipFetch]) FireEvent(eventidTextTipFetch,EVENT_PARAM(VTS_I4  VTS_I4  VTS_PI2  VTS_PI4  VTS_PBSTR  VTS_PBOOL), Col, Row, MultiLine, TipWidth, TextTip, Show);}
#endif
    void FireEditChange(long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_EditChange]) FireEvent(eventidEditChange,EVENT_PARAM(VTS_I4  VTS_I4), Col, Row);}
  #endif
#ifdef SS_V35
    void FireBeforeUserSort(long Col, long State, long FAR *DefaultAction)
        {if (m_bEventEnabled[EventEnabled_BeforeUserSort]) FireEvent(eventidBeforeUserSort,EVENT_PARAM(VTS_I4  VTS_I4  VTS_PI4), Col, State, DefaultAction);}
    void FireAfterUserSort(long Col)
        {if (m_bEventEnabled[EventEnabled_AfterUserSort]) FireEvent(eventidAfterUserSort,EVENT_PARAM(VTS_I4), Col);}
#ifdef SS_V70
    void FireSheetChanging(short OldSheet, short NewSheet, VARIANT FAR *Cancel)
        {if (m_bEventEnabled[EventEnabled_SheetChanging]) FireEvent(eventidSheetChanging,EVENT_PARAM(VTS_I2  VTS_I2  VTS_PVARIANT), OldSheet, NewSheet, Cancel);}
    void FireSheetChanged(short OldSheet, short NewSheet)
        {if (m_bEventEnabled[EventEnabled_SheetChanged]) FireEvent(eventidSheetChanged,EVENT_PARAM(VTS_I2  VTS_I2), OldSheet, NewSheet);}
    void FireTabScrolling(short OldLeftSheet, short NewLeftSheet)
        {if (m_bEventEnabled[EventEnabled_TabScrolling]) FireEvent(eventidTabScrolling,EVENT_PARAM(VTS_I2  VTS_I2), OldLeftSheet, NewLeftSheet);}
    void FireTabScrolled(short OldLeftSheet, short NewLeftSheet)
        {if (m_bEventEnabled[EventEnabled_TabScrolled]) FireEvent(eventidTabScrolled,EVENT_PARAM(VTS_I2  VTS_I2), OldLeftSheet, NewLeftSheet);}
    void FireBeforeEditMode(long Col, long Row, long UserAction, VARIANT FAR* CursorPos, VARIANT FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_BeforeEditMode]) FireEvent(eventidBeforeEditMode,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_PVARIANT  VTS_PVARIANT), Col, Row, UserAction, CursorPos, Cancel);}
    void FireDataLoaded()
        {if (m_bEventEnabled[EventEnabled_DataLoaded]) FireEvent(eventidDataLoaded,EVENT_PARAM(VTS_NONE));}
    void FireBeforeScrollTip(BOOL IsVertical, long Index, BSTR FAR *Text, VARIANT FAR *Width)
        {if (m_bEventEnabled[EventEnabled_BeforeScrollTip]) FireEvent(eventidBeforeScrollTip,EVENT_PARAM(VTS_BOOL  VTS_I4  VTS_PBSTR VTS_PVARIANT), IsVertical, Index, Text, Width);}
    void FireBeforeColMove(long Col, long Col2, long ColDest, VARIANT FAR *Cancel)
        {if (m_bEventEnabled[EventEnabled_BeforeColMove]) FireEvent(eventidBeforeColMove,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4 VTS_PVARIANT), Col, Col2, ColDest, Cancel);}
    void FireBeforeRowMove(long Row, long Row2, long RowDest, VARIANT FAR *Cancel)
        {if (m_bEventEnabled[EventEnabled_BeforeRowMove]) FireEvent(eventidBeforeRowMove,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4 VTS_PVARIANT), Row, Row2, RowDest, Cancel);}
#endif // SS_V70
#endif

#ifdef SS_V80
    void FireCircularFormula(short Sheet, long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_CircularFormula]) FireEvent(eventidCircularFormula, EVENT_PARAM(VTS_I2 VTS_I4 VTS_I4), Sheet, Col, Row); }

    void FireCTCancelEditing(LPCTSTR Name, VARIANT *EditorControl, short Sheet, long Col, long Row)
        {if (m_bEventEnabled[EventEnabled_CTCancelEditing]) FireEvent(eventidCTCancelEditing, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4), Name, EditorControl, Sheet, Col, Row); }
    void FireCTCreateEditorControl(LPCTSTR Name, long Style, long CtrlId, VARIANT *EditorControl)
        {if (m_bEventEnabled[EventEnabled_CTCreateEditorControl]) FireEvent(eventidCTCreateEditorControl, EVENT_PARAM(VTS_BSTR VTS_I4 VTS_I4 VTS_PVARIANT), Name, Style, CtrlId, EditorControl); }
    void FireCTCreateRendererControl(LPCTSTR Name, long Style, long CtrlId, VARIANT *RendererControl)
        {if (m_bEventEnabled[EventEnabled_CTCreateRendererControl]) FireEvent(eventidCTCreateRendererControl, EVENT_PARAM(VTS_BSTR VTS_I4 VTS_I4 VTS_PVARIANT), Name, Style, CtrlId, RendererControl); }
    void FireCTGetEditorValue(LPCTSTR Name, VARIANT *EditorControl, short Sheet, long Col, long Row, VARIANT *Value)
        {if (m_bEventEnabled[EventEnabled_CTGetEditorValue]) FireEvent(eventidCTGetEditorValue, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_PVARIANT), Name, EditorControl, Sheet, Col, Row, Value); }
    void FireCTInitializeControl(LPCTSTR Name, VARIANT *Ctrl, short Sheet, long Col, long Row, CCTAppearance *Appearance)
        {if (m_bEventEnabled[EventEnabled_CTInitializeControl]) FireEvent(eventidCTInitializeControl, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_DISPATCH), Name, Ctrl, Sheet, Col, Row, Appearance); }
    void FireCTIsReservedKey(LPCTSTR Name, VARIANT *RendererControl, short Sheet, long Col, long Row, short KeyCode, short Shift, BOOL *IsReserved)
        {if (m_bEventEnabled[EventEnabled_CTIsReservedKey]) FireEvent(eventidCTIsReservedKey, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_I2 VTS_I2 VTS_PBOOL), Name, RendererControl, Sheet, Col, Row, KeyCode, Shift, IsReserved); }
    void FireCTIsReservedLocation(LPCTSTR Name, VARIANT *RendererControl, short Sheet, long Col, long Row, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y, OLE_XPOS_PIXELS CellLeft, OLE_YPOS_PIXELS CellTop, OLE_XPOS_PIXELS CellRight, OLE_YPOS_PIXELS CellBottom, CCTAppearance *Appearance, VARIANT *Value, long *ReservedCursorId)
        {if (m_bEventEnabled[EventEnabled_CTIsReservedLocation]) FireEvent(eventidCTIsReservedLocation, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_DISPATCH VTS_VARIANT VTS_PI4), Name, RendererControl, Sheet, Col, Row, x, y, CellLeft, CellTop, CellRight, CellBottom, Appearance, Value, ReservedCursorId); }
    void FireCTIsValid(LPCTSTR Name, VARIANT *EditorControl, VARIANT *Value, BOOL *IsValid)
        {if (m_bEventEnabled[EventEnabled_CTIsValid]) FireEvent(eventidCTIsValid, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_PBOOL), Name, EditorControl, Value, IsValid); }
    void FireCTGetPreferredSize(LPCTSTR Name, VARIANT *RendererControl, short Sheet, long Col, long Row, OLE_HANDLE hDC, OLE_XPOS_PIXELS CellLeft, OLE_YPOS_PIXELS CellTop, OLE_XPOS_PIXELS CellRight, OLE_YPOS_PIXELS CellBottom, CCTAppearance *Appearance, VARIANT *Value, OLE_XSIZE_PIXELS *PreferredWidth, OLE_YSIZE_PIXELS *PreferredHeight)
        {if (m_bEventEnabled[EventEnabled_CTGetPreferredSize]) FireEvent(eventidCTGetPreferredSize, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_HANDLE VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_DISPATCH VTS_VARIANT VTS_PXSIZE_PIXELS VTS_PYSIZE_PIXELS), Name, RendererControl, Sheet, Col, Row, hDC, CellLeft, CellTop, CellRight, CellBottom, Appearance, Value, PreferredWidth, PreferredHeight); }
    void FireCTGetReservedCursor(LPCTSTR Name, VARIANT *EditorControl, long ReservedCursorId, VARIANT *ReservedCursor)
        {if (m_bEventEnabled[EventEnabled_CTGetReservedCursor]) FireEvent(eventidCTGetReservedCursor, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I4 VTS_PVARIANT), Name, EditorControl, ReservedCursorId, ReservedCursor); }
    void FireCTPaintCell(LPCTSTR Name, VARIANT *RendererControl, short Sheet, long Col, long Row, VARIANT *Value, OLE_HANDLE hDC, OLE_XPOS_PIXELS CellLeft, OLE_YPOS_PIXELS CellTop, OLE_XPOS_PIXELS CellRight, OLE_YPOS_PIXELS CellBottom, CCTAppearance *Appearance, BOOL IsSelected, BOOL IsLocked, BOOL *Handled)
        {if (m_bEventEnabled[EventEnabled_CTPaintCell]) FireEvent(eventidCTPaintCell, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_VARIANT VTS_HANDLE VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_DISPATCH VTS_BOOL VTS_BOOL VTS_PBOOL), Name, RendererControl, Sheet, Col, Row, Value, hDC, CellLeft, CellTop, CellRight, CellBottom, Appearance, IsSelected, IsLocked, Handled); }
    void FireCTSetValue(LPCTSTR Name, VARIANT *Ctrl, short Sheet, long Col, long Row, VARIANT *Value, BOOL *Handled)
        {if (m_bEventEnabled[EventEnabled_CTSetValue]) FireEvent(eventidCTSetValue, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_VARIANT VTS_PBOOL), Name, Ctrl, Sheet, Col, Row, Value, Handled); }
    void FireCTStartEditing(LPCTSTR Name, VARIANT *EditorControl, short Sheet, long Col, long Row, int Msg, int wParam, int lParam, BOOL SelectAll, BOOL AutoClipboard, BOOL *Handled)
        {if (m_bEventEnabled[EventEnabled_CTStartEditing]) FireEvent(eventidCTStartEditing, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BOOL VTS_BOOL VTS_PBOOL), Name, EditorControl, Sheet, Col, Row, Msg, wParam, lParam, SelectAll, AutoClipboard, Handled); }
    void FireCTStopEditing(LPCTSTR Name, VARIANT *EditorControl, short Sheet, long Col, long Row, BOOL *Cancel)
        {if (m_bEventEnabled[EventEnabled_CTStopEditing]) FireEvent(eventidCTStopEditing, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_I2 VTS_I4 VTS_I4 VTS_PBOOL), Name, EditorControl, Sheet, Col, Row, Cancel); }
    void FireCTStringFormat(LPCTSTR Name, VARIANT *RendererControl, VARIANT *Value, long Style, BSTR *Formatted)
        {if (m_bEventEnabled[EventEnabled_CTStringFormat]) FireEvent(eventidCTStringFormat, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_I4 VTS_PBSTR), Name, RendererControl, Value, Style, Formatted); }
    void FireCTStringUnformat(LPCTSTR Name, VARIANT *RendererControl, LPCTSTR Formatted, long Style, VARIANT *Value)
        {if (m_bEventEnabled[EventEnabled_CTStringUnformat]) FireEvent(eventidCTStringUnformat, EVENT_PARAM(VTS_BSTR VTS_VARIANT VTS_BSTR VTS_I4 VTS_PVARIANT), Name, RendererControl, Formatted, Style, Value); }
#endif

#ifdef SS_V35
    void FireScriptCustomFunction(LPCTSTR FunctionName, short ParameterCnt, long Col, long Row, VARIANT FAR* Status)
        {if (m_bEventEnabled[EventEnabled_CustomFunction]) FireEvent(eventidScriptCustomFunction,EVENT_PARAM(VTS_BSTR  VTS_I2  VTS_I4  VTS_I4  VTS_PVARIANT), FunctionName, ParameterCnt, Col, Row, Status);}
    void FireScriptDataFill(long Col, long Row, short DataType, short fGetData, VARIANT FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_DataFill]) FireEvent(eventidScriptDataFill,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I2  VTS_I2  VTS_PVARIANT), Col, Row, DataType, fGetData, Cancel);}
    void FireScriptDragDropBlock(long Col, long Row, long Col2, long Row2, long NewCol, long NewRow, long NewCol2, long NewRow2, BOOL Overwrite, VARIANT FAR* Action, VARIANT FAR* DataOnly, VARIANT FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_DragDropBlock]) FireEvent(eventidScriptDragDropBlock,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_BOOL  VTS_PVARIANT  VTS_PVARIANT  VTS_PVARIANT), Col, Row, Col2, Row2, NewCol, NewRow, NewCol2, NewRow2, Overwrite, Action, DataOnly, Cancel);}
    void FireScriptLeaveCell(long Col, long Row, long NewCol, long NewRow, VARIANT FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_LeaveCell]) FireEvent(eventidScriptLeaveCell,EVENT_PARAM(VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_PVARIANT), Col, Row, NewCol, NewRow, Cancel);}
    void FireScriptLeaveRow(long Row, BOOL RowWasLast, BOOL RowChanged, BOOL AllCellsHaveData, long NewRow, long NewRowIsLast, VARIANT FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_LeaveRow]) FireEvent(eventidScriptLeaveRow,EVENT_PARAM(VTS_I4  VTS_BOOL  VTS_BOOL  VTS_BOOL  VTS_I4  VTS_I4  VTS_PVARIANT), Row, RowWasLast, RowChanged, AllCellsHaveData, NewRow, NewRowIsLast, Cancel);}
    void FireScriptPrintAbort(VARIANT FAR* Abort)
        {if (m_bEventEnabled[EventEnabled_PrintAbort]) FireEvent(eventidScriptPrintAbort,EVENT_PARAM(VTS_PVARIANT), Abort);}
    void FireScriptPrintMsgBox(LPCTSTR Text, long PrintID, BOOL OpenMsgBox, VARIANT FAR* Processed)
        {if (m_bEventEnabled[EventEnabled_PrintMsgBox]) FireEvent(eventidScriptPrintMsgBox,EVENT_PARAM(VTS_BSTR VTS_I4 VTS_BOOL VTS_PVARIANT), Text, PrintID, OpenMsgBox, Processed);}
    void FireScriptQueryAdvance(BOOL AdvanceNext, VARIANT FAR* Cancel)
        {if (m_bEventEnabled[EventEnabled_QueryAdvance]) FireEvent(eventidScriptQueryAdvance,EVENT_PARAM(VTS_BOOL  VTS_PVARIANT), AdvanceNext, Cancel);}
    void FireScriptQueryData(long Row, long RowsNeeded, VARIANT FAR* RowsLoaded, short Direction, VARIANT FAR* AtTop, VARIANT FAR* AtBottom)
        {if (m_bEventEnabled[EventEnabled_QueryData]) FireEvent(eventidScriptQueryData,EVENT_PARAM(VTS_I4  VTS_I4  VTS_PVARIANT  VTS_I2  VTS_PVARIANT  VTS_PVARIANT), Row, RowsNeeded, RowsLoaded, Direction, AtTop, AtBottom);}
    void FireScriptTextTipFetch(long Col, long Row, VARIANT FAR *MultiLine, VARIANT FAR *TipWidth, VARIANT FAR *TextTip, VARIANT FAR *Show)
        {if (m_bEventEnabled[EventEnabled_TextTipFetch]) FireEvent(eventidScriptTextTipFetch,EVENT_PARAM(VTS_I4  VTS_I4  VTS_PVARIANT  VTS_PVARIANT  VTS_PVARIANT  VTS_PVARIANT), Col, Row, MultiLine, TipWidth, TextTip, Show);}
    void FireScriptBeforeUserSort(long Col, long State, VARIANT FAR *DefaultAction)
        {if (m_bEventEnabled[EventEnabled_BeforeUserSort]) FireEvent(eventidScriptBeforeUserSort,EVENT_PARAM(VTS_I4  VTS_I4  VTS_PVARIANT), Col, State, DefaultAction);}
    void FireScriptKeyPress(VARIANT FAR *KeyAscii)
        {if (m_bEventEnabled[EventEnabled_KeyPress]) FireEvent(eventidScriptKeyPress,EVENT_PARAM(VTS_PVARIANT), KeyAscii);}
    void FireScriptKeyDown(VARIANT FAR *KeyCode, short Shift)
        {if (m_bEventEnabled[EventEnabled_KeyDown]) FireEvent(eventidScriptKeyDown,EVENT_PARAM(VTS_PVARIANT VTS_I2), KeyCode, Shift);}
    void FireScriptKeyUp(VARIANT FAR *KeyCode, short Shift)
        {if (m_bEventEnabled[EventEnabled_KeyUp]) FireEvent(eventidScriptKeyUp,EVENT_PARAM(VTS_PVARIANT VTS_I2), KeyCode, Shift);}

#endif   // SS_V35

  DECLARE_EVENT_MAP()
// fix for SEL5004
	BOOL IsPictMetaFile(LPPICTUREDISP pPictDisp)
	{
		IPicture* pPict;
		if (pPictDisp && S_OK == pPictDisp->QueryInterface(IID_IPicture, (LPVOID FAR*)&pPict))
		{
			short nType;
			pPict->get_Type(&nType);
			if (PICTYPE_METAFILE == nType || PICTYPE_ENHMETAFILE == nType)
			{
				pPict->Release();
				return TRUE;
			}
			pPict->Release();
		}
		return FALSE;
	}
// fix for SEL5004
#if SS_V80
   void DoSuperclassPaint(CDC* pDC, const CRect& rcBounds);
#endif

};

// Spread Designer
#define SSM_SPREADDSGEXIT        (WM_USER + 110)
#define SSM_GETCOLUMNNAMES       (WM_USER + 111)
#define SSM_SPREADDSGHEREIAM     (WM_USER + 112)
// Spread Designer

#if SS_V80
extern "C"
{
  // callbacks
  BOOL CALLBACK CT_CancelEditing(LPSS_CT_CANCELEDITING_ARGS pArgs);
  BOOL CALLBACK CT_CreateEditorControl(LPSS_CT_CREATEEDITORCONTROL_ARGS pArgs);
  BOOL CALLBACK CT_CreateRendererControl(LPSS_CT_CREATERENDERERCONTROL_ARGS pArgs);
  BOOL CALLBACK CT_GetEditorValue(LPSS_CT_GETEDITORVALUE_ARGS pArgs);
  BOOL CALLBACK CT_InitializeControl(LPSS_CT_INITIALIZECONTROL_ARGS pArgs);
  BOOL CALLBACK CT_IsReservedKey(LPSS_CT_ISRESERVEDKEY_ARGS pArgs);
  BOOL CALLBACK CT_IsReservedLocation(LPSS_CT_ISRESERVEDLOCATION_ARGS pArgs);
  BOOL CALLBACK CT_IsValid(LPSS_CT_ISVALID_ARGS pArgs);
  BOOL CALLBACK CT_GetPreferredSize(LPSS_CT_GETPREFERREDSIZE_ARGS pArgs);
  BOOL CALLBACK CT_GetReservedCursor(LPSS_CT_GETRESERVEDCURSOR_ARGS pArgs);
  BOOL CALLBACK CT_PaintCell(LPSS_CT_PAINTCELL_ARGS pArgs);
  BOOL CALLBACK CT_SetValue(LPSS_CT_SETVALUE_ARGS pArgs);
  BOOL CALLBACK CT_StartEditing(LPSS_CT_STARTEDITING_ARGS pArgs);
  BOOL CALLBACK CT_StopEditing(LPSS_CT_STOPEDITING_ARGS pArgs);
  BOOL CALLBACK CT_StringFormat(LPSS_CT_STRINGFORMAT_ARGS pArgs);
  BOOL CALLBACK CT_StringUnformat(LPSS_CT_STRINGUNFORMAT_ARGS pArgs);
  // conversion functions
  void CT_ValueToVariant(VARIANT *Variant, SS_CT_VALUE *Value);
  BOOL CT_VariantToValue(VARIANT *Variant, SS_CT_VALUE *Value);
}

#endif