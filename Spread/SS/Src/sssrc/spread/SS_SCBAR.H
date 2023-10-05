/************
* SCRLBAR.H
************/

#ifndef SCRLBAR_H
#define SCRLBAR_H

#ifdef __cplusplus
extern "C" {
#endif

#define TBSCRLBAR _T("tbScrlBar")

typedef struct scrlbaritem
   {
   HBITMAP hBmp;
   LPTSTR   lpBmpName;
   HBITMAP hBmpD;
   LPTSTR   lpBmpNameD;
   } SCRLBARITEM, FAR *LPSCRLBARITEM;

typedef struct scrlbar
   {
   short       dBtnDown;
   short       dHeight;
   SCRLBARITEM ItemUp;
   SCRLBARITEM ItemDn;
   SCRLBARITEM ItemPageUp;
   SCRLBARITEM ItemPageDn;
   SCRLBARITEM ItemHome;
   SCRLBARITEM ItemEnd;
   BOOL        fIs95;
   BOOL        fIsHorizontal;
#ifdef SS_V80 
   BOOL       fTrackMouse;
   BOOL       fMouseIsDown;
   long       trackSize;
   long       ptLast;
   long       ptStart;
   long       ptCur;
   long       ptBase;
   long       ptOffset;
   long	     lCurPos;
   long       lLastPos;
   long       lFirstPos;
   long		  lCntPage;
   RECT		  rectThumb;
   short	     dWidth;
   RECT		  mouseBtnRect;
   short	     nStyle;
   COLORREF   clrTrack;
   COLORREF   clrArrow;
   COLORREF   clrUpperNormalStart;
   COLORREF   clrUpperNormalEnd;
   COLORREF   clrLowerNormalStart;
   COLORREF   clrLowerNormalEnd;
   COLORREF   clrUpperHoverStart;
   COLORREF   clrUpperHoverEnd;
   COLORREF   clrLowerHoverStart;
   COLORREF   clrLowerHoverEnd;
   COLORREF   clrUpperPushedStart;
   COLORREF   clrUpperPushedEnd;
   COLORREF   clrLowerPushedStart;
   COLORREF   clrLowerPushedEnd;
   COLORREF   clrHoverBorderColor;
   COLORREF   clrBorderColor;
#endif
   } SCRLBAR, FAR *LPSCRLBAR;

#define SBR_UP      _T("sbrup")
#define SBR_UPD     _T("sbrupd")
#define SBR_DN      _T("sbrdn")
#define SBR_DND     _T("sbrdnd")
#define SBR_PAGEUP  _T("sbrpgup")
#define SBR_PAGEUPD _T("sbrpgupd")
#define SBR_PAGEDN  _T("sbrpgdn")
#define SBR_PAGEDND _T("sbrpgdnd")
#define SBR_HOME    _T("sbrhome")
#define SBR_HOMED   _T("sbrhomed")
#define SBR_END     _T("sbrend")
#define SBR_ENDD    _T("sbrendd")

#define SBRS_HOMEEND  0x0100L
#define SBRS_UPDN     0x0200L
#define SBRS_PAGEUPDN 0x0400L

#ifdef SS_V80
#define SBRS_THUMB    0x0800L

#define SBR_LF      _T("sbrlf")
#define SBR_LFD     _T("sbrlfd")
#define SBR_RT      _T("sbrrt")
#define SBR_RTD     _T("sbrrtd")
#define SBR_PAGELF  _T("sbrpglf")
#define SBR_PAGELFD _T("sbrpglfd")
#define SBR_PAGERT  _T("sbrpgrt")
#define SBR_PAGERTD _T("sbrpgrtd")
#define SBR_HOMEH    _T("sbrhomeh")
#define SBR_HOMEHD   _T("sbrhomehd")
#define SBR_ENDH     _T("sbrendh")
#define SBR_ENDHD    _T("sbrendhd")

#endif

#define SCRLBAR_TIMEDELAY    50
#define SCRLBAR_INITIALDELAY 320

#ifdef SS_V80
#define SBR_SETSTYLE		WM_USER+1
#define SBR_SETPAGESIZE		WM_USER+2
#define SBR_SETPOS			WM_USER+3
#define SBR_TRACKMOUSE      WM_USER+4

void ScrlBarEnhancedScrollBarColors(HWND hWnd, COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor);

#endif
#ifdef __cplusplus
}
#endif

#endif
