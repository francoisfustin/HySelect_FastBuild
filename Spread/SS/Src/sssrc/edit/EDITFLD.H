#include "toolbox.h"

#define  MAXFIELD 160
/*
#define  N_PRINTABLE_CHARS 95
*/
#define  N_PRINTABLE_CHARS 256

#define  LEFT  1
#define  RIGHT 2

#define  GLOBAL   TRUE
#define  LOCAL    FALSE

#define  UPPERCASE 1
#define  LOWERCASE 2

#define  XMARGIN 2   // Pixels
#define  YMARGIN 1

#define  EDIT_HALIGN_LEFT  0
#define  EDIT_HALIGN_RIGHT 1

#define  EDIT_FIELD_CODES  (DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_HASSETSEL)

typedef struct tagEditField
   {
   TCHAR        lpszString[MAXFIELD + 1];          // The contents of the field
   BYTE         nCharWidthTable[N_PRINTABLE_CHARS - ' ' + 1];// Table of widths for this font
   int          yHeight;         // Height (in pixels) of this font
   int          tmOverhang;      // Overhang (in pixels) of this font
   int          xRightNonClientXtra; // Xtra Non-Client area (in pixels) of the of right side of rectangle
   int          nChars;          // Number of characters in the field
   int          iCurrentPos;     // Number of chars to the left of the caret
                                 //    0 <= nCurrentPos <= nChars
   BOOL         bModified;       // The user has modified the text
   BOOL         bHighlighted;    // True if the field is currently highlighted
   int          iHiStart;        // Starting position of the current highlight
   int          iHiEnd;          // Ending position of the current highlight
   BOOL         bOverstrike;     // True if Overstrike mode is selected
   BYTE         bHAlign;         // EDIT_HALIGN_LEFT EDIT_HALIGN_RIGHT
   BYTE         bHAlignActual;   // EDIT_HALIGN_LEFT EDIT_HALIGN_RIGHT
   BOOL         bMouseDown;      // True if the left mouse button is down
   BYTE         nCase;           // Indicates whether to force upper or lower case
   BYTE         fPassWord;       // Show text as a password
   HBITMAP      hBitmapCaret;
   HFONT        hFont;           // NULL if system font is used
   short        nMaxChars;       // Maximum number of characters to be entered
   BOOL         bCharBeingInserted;
   BOOL         f3D;
   EDIT3DFORMAT Edit3DFormat;
   HWND         hWnd;
   BOOL         fSuspendNotify;
   int          nMaxCharWidth;     //- Added in by Masanori Iwasa
   BOOL         bProcessingBackspace; // BJO 27Jun97 GIL708 - Added
   BOOL         bProcessingDelete;    // BJO 27Jun97 GIL708 - Added
   BOOL         bShiftDown;   // The shift key is currently held down.
   int          iAnchor;      // Start, or anchor, position for highlighting
   int          iPrevious;    // Previous caret position for highlighting
   BOOL         bFirstMove;   // True if this is the first move of a highlighting
   int          KType;
   BOOL         bLocalOverstrike;
   short        nLeftChar;
	BOOL         fAutoHScroll;
	BOOL fEnhanced;
   } EDITFIELD, FAR *LPEDITFIELD;

extern LPCTSTR lpszSpinClassName;

// in FieldMgr   
BOOL        CreateField          (HWND, LPARAM);
void        OpenField            (HWND);
void        HighlightField       (HWND hWnd, LPEDITFIELD lpField);
void        PaintField           (HWND, HDC);
void        CloseField           (HWND);
void        DestroyField         (HWND);
LPEDITFIELD LockField            (HWND);
void        UnlockField          (HWND);
BOOL        ValidateText         (HWND, LPCTSTR, int);
void        EditFieldModifed     (HWND);
BOOL        GetModify            (HWND hWnd);
void        SetModify            (HWND hWnd, BOOL Modify);
BOOL        ReplaceSel           (HWND hWnd, LPARAM lParam);
BOOL        EditSet3DParams(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL        EditGet3DParams(HWND hWnd, LPARAM lParam);
void        ProcessSize(HWND hWnd, LPARAM lParam);
int         EditGetStartXPos(LPEDITFIELD lpField);
void        EditGetEditRect(LPEDITFIELD lpField, LPRECT lpRect);
void        EditScrollCurrentPosIntoView(LPEDITFIELD lpField);
void        EditScrollCharIntoView(LPEDITFIELD lpField, short nViewChar);
void        EditScrollHome(LPEDITFIELD lpField);
void        EditScrollEnd(LPEDITFIELD lpField);
void        EditScrollLeft(LPEDITFIELD lpField);
void        EditScrollRight(LPEDITFIELD lpField);
short       EditGetRightVisCharPos(LPEDITFIELD lpField);
void        EditScroll(LPEDITFIELD lpField, short nNewLeftChar);
BYTE        EditGetHAlign(LPEDITFIELD lpField);
short       EditGetLastValidCaretPos(LPEDITFIELD lpField);
void        EditGetRect(HWND hWnd, LPARAM lParam);

// in Caret
void        HomeCaret            (HWND, LPEDITFIELD);
void        EndCaret             (HWND, LPEDITFIELD);
void        PositionCaret        (HWND, LPEDITFIELD, int);
void        LeftCaret            (HWND, LPEDITFIELD);
void        RightCaret           (HWND, LPEDITFIELD);
int         CaretPosOfCharacter  (LPEDITFIELD, int);
void        CreateModeCaret      (HWND, LPEDITFIELD);
BOOL        CaretPosition        (HWND hWnd, WPARAM nChar, LPARAM lParam);
void        SetCaretPosition     (HWND, WPARAM);
int         GetCaretPosition     (HWND);
void        SetCaretXPos         (HWND, int);
int         GetCaretXPos         (HWND);
void        PositionCaretAfterPaint (HWND);

// in Char
HFONT       ProcessEditSetFont   (HWND, WPARAM, LPARAM);
HFONT       ProcessEditGetFont   (HWND hWnd);
BOOL        GetFontWidth         (HWND, HFONT);
int         LeftOf               (LPEDITFIELD, int);
int         RightOf              (LPEDITFIELD, int);
int         GetSingleCharWidth   (LPEDITFIELD, TCHAR);
void        InsertCharacter      (HWND, LPEDITFIELD, TCHAR);    //- Masanori Iwasa
void        ReplaceCharacter     (HWND, LPEDITFIELD, TCHAR);
int         EditGetCharFromPos(HWND hWnd, LPARAM lParam);
int         FindNearestCharacter (LPEDITFIELD, int, BOOL);
int         FieldLength (LPEDITFIELD lpField, short nStartPos);
int         GetStringLength      (HDC hDC, LPCTSTR lpString, short Length);

// in HiLight
void        MouseDown            (HWND, LPARAM);
void        MouseMove            (HWND, LPARAM);
void        MouseUp              (HWND);
void        ShiftDown            (HWND);
void        ShiftMove            (HWND);
void        ShiftUp              (HWND hWnd);
BOOL        IsShiftDown          (HWND hWnd);
void        SetSelection         (HWND, int, int);
LONG        GetSelection         (HWND);
void        ClearSelection       (HWND);
void        DeselectText         (HWND, LPEDITFIELD);
void        DeleteSelectedText   (HWND, LPEDITFIELD);
void        ReplaceSelectedText  (HWND, LPEDITFIELD, LPCTSTR);
void        Invert               (HWND, LPEDITFIELD, int, int);

// in KeyBoard
TCHAR       ProcessCharacter     (HWND, WPARAM);
WORD        ProcessKeystroke     (HWND, WPARAM);
WORD        ProcessKeyUp         (HWND, WPARAM);
void        ClipboardCopy        (HWND hWnd);
void        ClipboardCut         (HWND hWnd);
void        ClipboardPaste       (HWND hWnd);
BOOL        SendTextToClipboard  (HWND hWnd, LPCTSTR lpszText, short dLen);
HANDLE      GetTextFromClipboard (HWND hWnd);
BOOL        Edit_IsActionKey(WORD wAction, WORD wKey);

// in Text
void        SetFieldText         (HWND, LPARAM);   
short       GetFieldText         (HWND, WPARAM, LPARAM);
LONG        GetTextLength        (HWND);   
void        AdjustLength         (HWND);
void        InsertText           (HWND hWnd, LPEDITFIELD lpField,
                                  LPCTSTR lpNewText);

//- Newly added function to support DBCS. (Masanori Iwasa)
int         IsCharDbcs           (LPEDITFIELD lpField, int Last );

// 96' 6/25 Added by BOC MASA & Gao. 
// - for get DBCS width
int		GetDBCSWidth	(LPEDITFIELD lpField, int nStart, int nLength);

