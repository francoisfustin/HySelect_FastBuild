/**********
* EDITPIC
**********/

#ifndef EDITPIC_H
#define EDITPIC_H

#define UNDEFINED          -1L

#define PICFMT_NUM         '9'
#define PICFMT_ALPHA       'A'
#define PICFMT_ALPHANUM    'N'
#define PICFMT_ASCII       'X'
#define PICFMT_LOWER       'L'
#define PICFMT_UPPER       'U'
#define PICFMT_HEX         'H'
#define PICFMT_CALLBACK    '?'
#define PICFMT_CONTROL     '/'

//#ifdef  SPREAD_JPN
#define PICFMT_KANJI       '&'
//#endif

typedef struct tagPicField
   {
   FARPROC lpfnCallBack;
   HANDLE  hMask;
   HANDLE  hDefText;
   } PICFIELD, FAR *LPPICFIELD;

typedef WORD (far pascal *PICCALLBACK)(HWND, WORD, short, short);

BOOL  PicCreateField(HWND, LPARAM);
void  PicInitializeField(HWND, LPARAM);
void  PicSetMask(HWND hWnd, LPCTSTR szMaskNew);
BOOL  PicSetText(HWND hWnd, LPCTSTR lpszText, LPTSTR lpszTextFmt, BOOL fSetValue);
BOOL  PicGetMask(HWND hWnd, LPTSTR szMask, WORD Len);
void  PicDestroyField(HWND);
BOOL  PicProcessKey(HWND hWnd, WORD Key, BOOL fShift);
TCHAR PicProcessCharacter(HWND hWnd, WPARAM wParam);
void  PicSetCallBack(HWND hWnd, FARPROC lParam);
void  PicSetSelection(HWND hWnd, int iStart, int iEnd);

BOOL DLLENTRY PicFmtText(HWND hWnd, HANDLE hDefText, HANDLE hMask,
                         LPCTSTR lpszText, LPTSTR lpszTextFmt, BOOL fSetValue);
BOOL DLLENTRY PicIsNULL(HWND hWnd);
BOOL DLLENTRY PicGetValue(HWND hWnd, LPTSTR lpValString);
BOOL DLLENTRY PicToValue(LPCTSTR lpszPicFmt, LPCTSTR lpszPicMask,
                         LPTSTR lpszPicValue);
void          PicClipboardPaste(HWND hWnd);
void          PicClipboardCut(HWND hWnd);

#endif
