// editnum.h

#include "usernum.h"

// Get rid of fEmpty and iPrevStrLen and style
typedef struct
{
   HWND         hWnd;
   HWND         hWndSpinBtn;
   double       dfMinVal;                     // the range value
   double       dfMaxVal;                     // the range value
   double       dfVal;                        // the current value
   LONG         style;
   BYTE         fNegative;
   BYTE         fEmpty;                       // this is true if the buffer is empty
	BYTE         nWidthSufPos;                 // Size of Suffix for positive numbers
	BYTE         nWidthSufNeg;                 // Size of Suffix for negative numbers
   HWND         hWndParent;                   // the parent window, if any
   int          iPrevStrLen;
	SSNUM_FORMAT Format;
	WPARAM       wKeyDownVirtualKeyCode;
} EDITNUMINFO, *LPEDITNUMINFO;

#define  LockNumHeader(lpInfo,hInfo,hWnd)  \
   HANDLE          (hInfo);\
   LPEDITNUMINFO   (lpInfo);\
   (hInfo) = GetWindowNumField(hWnd);\
   (lpInfo) = (LPEDITNUMINFO)GlobalLock((hInfo))\

#define  UnlockNumHeader(hInfo)     GlobalUnlock((hInfo))

// function prototypes

void Num_GetDisplayFormat(LPSSNUM_FORMAT lpFormatSrc, LPSSNUM_FORMAT lpFormat);
void Num_GetValue(LPEDITNUMINFO lpInfo, LPDOUBLE lpdfVal);
BOOL Num_SetRange(LPEDITNUMINFO lpInfo, double dfMinVal, double dfMaxVal);
BOOL Num_StringToValue(LPSSNUM_FORMAT lpFormat, LPCTSTR szString, LPDOUBLE lpdfVal);
BOOL Num_ValueToString(LPSSNUM_FORMAT lpFormat, double dfVal, LPTSTR szString);
BOOL Num_ProcessWMChar(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL Num_ProcessWMCreate(HWND hWnd, LPARAM lParam);
BOOL Num_CheckRange(LPEDITNUMINFO lpInfo, double dfVal);
LONG Num_ProcessEMValidate(HWND hWnd, WPARAM wParam, LPARAM lParam);
void Num_ProcessWMDestroy(HWND hWnd);
void Num_SetValue(LPEDITNUMINFO lpInfo, double dfVal);
BOOL Num_ProcessWMSetText(HWND hWnd, WPARAM wParam, LPCTSTR lParam);
void Num_ProcessWMKillFocus(HWND hWnd);
void Num_SetZero(LPEDITNUMINFO lpInfo);
BOOL Num_ProcessEMCaretPos(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL Num_ProcessWMKeyDown(HWND hWnd, WPARAM wParam);
void Num_ProcessWMKeyUp(HWND hWnd, WPARAM wParam);
void Num_SetInitialCaretPosition(HWND hWnd);
void Num_ProcessWMIniChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL Num_ProcessEMSetFormat(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL Num_ProcessEMGetFormat(HWND hWnd, LPARAM lParam);
void Num_ResetPrevLen(HWND hWnd);
void Num_ReplaceText(HWND hWnd, LPARAM lParam);
void Num_ProcessWMSize(HWND hWnd, LPARAM lParam);
void Num_ResetSpinBtn(LPEDITNUMINFO lpInfo);
void Num_ProcessWMVScroll(HWND hWnd, int nScrollCode);
void Num_ProcessWMEnable(HWND hWnd, WPARAM wParam);
