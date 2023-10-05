// pvctl.h : Declaration of the CPvCtrl OLE control class.
#ifndef VC8
#include "fpidptbl.h"                                    //MYDISP
#endif

#ifdef FP_OLEDRAG
#include "..\..\..\fplibs\fptools\src\fpdatobj.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPvCtrl : See pvctl.cpp for implementation.

class CPvCtrl : public COleControl
{
	DECLARE_DYNCREATE(CPvCtrl)

// Constructor
public:
	CPvCtrl();
        BOOL PX_Props(CPropExchange* pPX);
        void fpSetProp(WORD wPropID, long lValue, long lSetIndex);
        long fpGetProp(WORD wPropID, long lGetIndex);
        HRESULT MyOnActivateInPlace(BOOL bUIActivate, LPMSG pMsg);
        BOOL MyCreateControlWindow(HWND hWndParent, const CRect& rcPos);

// Overrides

	// Drawing function
        virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
        virtual void OnDrawMetafile(CDC* pdc, const CRect& rcBounds);

	// Persistence
	virtual void DoPropExchange(CPropExchange* pPX);

	// Reset control state
	virtual void OnResetState();

public:
    DECLARE_INTERFACE_MAP()

#define FPMYDISP_CTRL_NAME  CPvCtrl                     //MYDISP
#ifndef VC8
#include "fpmydisp.h"
#endif
#ifdef FP_OLEDRAG
#include "..\sssrc\spread\pvoledrg.h"
#endif
#undef FPMYDISP_CTRL_NAME

// property categories -scl
#ifdef SS_V40
  BEGIN_INTERFACE_PART(CategorizeProperties, ICategorizeProperties)
   INIT_INTERFACE_PART(CPvCtrl, CategorizeProperties)
	STDMETHOD(PropertyToCategory)(DISPID dispid, PROPCAT *ppropcat);
	STDMETHOD(GetCategoryName)(PROPCAT propcat, LCID lcid, BSTR *pbstrName);
  END_INTERFACE_PART(CategorizeProperties)
#endif

// Implementation
protected:
	~CPvCtrl();

        BOOL OnCommand(WPARAM wParam, LPARAM lParam);

        BEGIN_OLEFACTORY(CPvCtrl)        // Class factory and guid
          virtual BOOL VerifyUserLicense();
          virtual BOOL GetLicenseKey(DWORD, BSTR FAR*);
      // CODESENT
          virtual BOOL VerifyLicenseKey(BSTR bstrKey);
        END_OLEFACTORY(CPvCtrl)

//        DECLARE_OLECREATE_EX(CPvCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CPvCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CPvCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CPvCtrl)		// Type name and misc status

	// VBX port support
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	WNDPROC* GetSuperWndProcAddr(void);
   virtual void OnMousePointerChanged();

   virtual void OnFreezeEvents(BOOL bFreeze);
	BOOL OnVerbUiActivate(LPMSG lpMsg, HWND, LPCRECT lprcPosRect);
	BOOL OnVerbInPlaceActivate(LPMSG lpMsg, HWND, LPCRECT lprcPosRect);
	BOOL OnVerbPrimary(LPMSG lpMsg, HWND, LPCRECT lprcPosRect);

   virtual BOOL PreTranslateMessage(LPMSG lpMsg);

protected:

	short ShiftState();
	virtual void OnClick(USHORT iButton);
	void OnButtonUp(USHORT nButton, UINT nFlags, CPoint point);
	void OnButtonDown(USHORT nButton, UINT nFlags, CPoint point);
	void OnButtonDblClk(USHORT nButton, UINT nFlags, CPoint point);
	void ButtonDown(USHORT iButton, UINT nFlags, CPoint point);
	void ButtonUp(USHORT iButton, UINT nFlags, CPoint point);
	void ButtonDblClk(USHORT iButton, UINT nFlags, CPoint point);

// Message maps
        //{{AFX_DISPATCH(CPvCtrl)
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
// fix for #8705 -scl
  LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
        //{{AFX_DISPATCH(CPvCtrl)
#ifdef SS_V35
      afx_msg BOOL GetScriptEnhanced();
      afx_msg void SetScriptEnhanced(BOOL bNewValue);
#endif

        afx_msg LPPICTUREDISP GetMouseIcon();
        afx_msg void SetMouseIcon(LPPICTUREDISP newValue);
	//}}AFX_DISPATCH
//        DECLARE_DISPATCH_MAP()
#if VC8
  DECLARE_DISPATCH_MAP()
#else
	FP_DECLARE_DISPATCH_MAP()                            //MYDISP
#endif
	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CPvCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
        void FirePageChange(long Page)
                { FireEvent(eventidPageChange, EVENT_PARAM( VTS_I4), Page); }
        void FireZoom(void)
                { FireEvent(eventidZoom, EVENT_PARAM( VTS_NONE)); }

	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CPvCtrl)
		// NOTE: ClassWizard will add and remove enumeration elements here.
		//    DO NOT EDIT what you see in these blocks of generated code !

        dispidAllowUserZoom = 1L,
	dispidGrayAreaColor = 2L,
	dispidGrayAreaMarginH = 3L,
	dispidGrayAreaMarginType = 4L,
	dispidGrayAreaMarginV = 5L,
	dispidHWndSpread = 6L,
	dispidPageBorderColor = 7L,
	dispidPageBorderWidth = 8L,
	dispidPageShadowColor = 9L,
	dispidPageShadowWidth = 10L,
	dispidPageViewPercentage = 11L,
	dispidPageViewType = 12L,
	dispidScrollBarH = 13L,
	dispidScrollBarV = 14L,
	dispidScrollIncH = 15L,
	dispidScrollIncV = 16L,
        dispidPageMultiCntH = 17L,
        dispidPageMultiCntV = 18L,
        dispidPageGutterH = 19L,
        dispidPageGutterV = 20L,
        dispidZoomState = 21L,
        dispidPageCurrent = 22L,
        dispidPagesPerScreen = 23L,
        dispidPagePercentageActual = 24L,

        dispidMousePointer = 50L,
        dispidMouseIcon = 51L,
#ifdef FP_OLEDRAG
		dispidOLEDropMode = 52L,
		dispidOLEDrag = 53L,
#endif        
#ifdef SS_V35
		dispidScriptEnhanced = 54L,
#endif
		eventidPageChange = 1L,
		eventidZoom = 2L,
#ifdef FP_OLEDRAG
		eventidOLECompleteDrag = 3L,
		eventidOLEDragDrop = 4L,
		eventidOLEDragOver = 5L,
		eventidOLEGiveFeedback = 6L,
		eventidOLESetData = 7L,
		eventidOLEStartDrag = 8L,
#ifdef SS_V35
		eventidScriptOLECompleteDrag = 9L,
		eventidScriptOLEDragDrop = 10L,
		eventidScriptOLEDragOver = 11L,
		eventidScriptOLEGiveFeedback = 12L,
		eventidScriptOLEStartDrag = 13L,
#endif
#endif
	//}}AFX_DISP_ID
	};

//---------------------------------------------------------------------
// private members
//---------------------------------------------------------------------
private:
   FP_HUGEBUFFER  PrePropList;
    
   // State flags
   BOOL           m_bFreezeEvents;
   BOOL           m_bEventAdvise;

   CPictureHolder m_pictMouseIcon;
   short          m_enumMousePointer;
};

#define IDC_NODROP              "IDC_NODROP"  // NoDrop cursor name

#define CUSTOM_CURSOR           99            // Custom cursor indicator  
#define MAX_CURSOR_INDEX        15            // index of last nonstandard cursor  

#ifdef WIN32
#define MAX_STANDARD_INDEX      15            // index of last standard Windows cursor
#else
#define MAX_STANDARD_INDEX      11            // index of last standard Windows cursor

#define IDC_NO                  "IDC_NODROP"         // NODROP cursor name
#define IDC_HELP                "IDC_ARROWQUESTION"  // ARROWQUESTION cursor name
#define IDC_APPSTARTING         "IDC_ARROWHOURGLASS" // ARROWHOURGLASS cursor name
#define IDC_SIZEALL             "IDC_SIZEALL"        // SIZEALL cursor name
#endif

#define IDS_PV               1
#define IDS_PV_PPG           2
#define IDS_PV_PPG_CAPTION   101

#define IDD_ABOUTBOX_PV      100
#define IDD_PROPPAGE_PV      101
#define IDB_PV               2
#define IDI_ABOUTDLL         10

#define _APS_NEXT_RESOURCE_VALUE        201
#define _APS_NEXT_CONTROL_VALUE         201
#define _APS_NEXT_SYMED_VALUE           101
#define _APS_NEXT_COMMAND_VALUE         32768
