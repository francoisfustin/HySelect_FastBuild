/************
* SS_TYPE.H
************/

#ifndef SS_TYPE_H
#define SS_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define SS_TYPE_ALLOC_CNT 4

#define SS_VALUE_TEXT  0
#define SS_VALUE_VALUE 1
#define SS_VALUE_FLOAT 2
#define SS_VALUE_INT   3
#if defined(_WIN64) || defined(IA64)
#define SS_VALUE_LONG_PTR 4
#endif
//#if SS_V80
//#define SS_VALUE_HANDLE 4
//#endif

/******************************
* Declare function prototypes
******************************/

#ifdef SS_V80
BOOL           SS_TypeControlRemove(CT_HANDLE hCT, LPTBGLOBALHANDLE lphControls,
                                    LPSHORT lpControlsAllocCnt, LPSHORT lpControlsCnt);
#endif // SS_V80
short          SS_CreateControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, BOOL fRenderer);
LPSS_CELLTYPE  SSx_RetrieveCellType(LPSPREADSHEET lpSS,
                                    LPSS_CELLTYPE CellType, LPSS_COL lpCol,
                                    LPSS_ROW lpRow, LPSS_CELL Cell,
                                    SS_COORD CellCol, SS_COORD CellRow);
GLOBALHANDLE   SS_CreateTextHandle(LPCTSTR lpszText, short Len);
LPSS_CELLTYPE  SS_SetCellTypeStruct(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                      LPSS_CELL Cell, SS_COORD CellCol, SS_COORD CellRow);
int            SS_StrReplaceCh(LPTSTR szStr, TCHAR chOld, TCHAR chNew);
void           SS_FloatFormat(LPSPREADSHEET lpSS,
                              LPSS_CELLTYPE lpCellType, double dfVal,
                              LPTSTR lpszText, BOOL fValue);
BOOL           SS_FormatData(LPSPREADSHEET lpSS, LPSS_DATA lpData,
                             SS_COORD Col, SS_COORD Row,
                             LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                             BOOL fValue, BOOL fCheckRange, BOOL fCheckLen);
TBGLOBALHANDLE SS_UnFormatData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, 
                               LPSS_CELLTYPE lpCellType, LPTSTR lpszText);
TBGLOBALHANDLE SS_FormatComboBox(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText,
                                 LPTSTR lpszTextFmt, BOOL fValue, BOOL fCheckLen);
BOOL           SS_FormatDataDate(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                                 LPCTSTR lpszText, LPTSTR lpszTextFmt,
                                 BOOL fValue);
BOOL           SS_FormatDataTime(LPSS_CELLTYPE lpCellType,
                                 LPCTSTR lpszText, LPTSTR lpszTextFmt,
                                 BOOL fValue);
short          SS_TypeGetControlType(LPSPREADSHEET lpSS, short CtrlID);
HWND           SS_GetControlhWnd(LPSPREADSHEET lpSS, short CtrlID);
#if SS_V80
FARPROC        SS_GetControlWndProc(LPSPREADSHEET lpSS, HWND hWnd);
void           SS_SetControlWndProc(LPSPREADSHEET lpSS, HWND hWnd, FARPROC wndProc);
#endif
HWND           SS_RetrieveControlhWnd(LPSPREADSHEET lpSS,
                                      SS_COORD Col, SS_COORD Row);
LPSS_CELLTYPE SS_SetTypeButton(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                               long  Style, LPCTSTR Text,
                               LPCTSTR Picture, short nPictureType,
                               LPCTSTR PictureDown, short nPictureDownType,
                               short ButtonType, short nShadowSize,
                               LPSUPERBTNCOLOR lpColor);
void          SSx_SetTypeCheckBoxDefaults(LPSS_BOOK lpBook, LPSS_CELLTYPE lpCellType);
LPSS_CELLTYPE SS_SetTypeCheckBox(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                                 long lStyle, LPCTSTR lpText,
                                 LPCTSTR lpPictUp, WORD wPictUpType,
                                 LPCTSTR lpPictDown, WORD wPictDownType,
                                 LPCTSTR lpPictFocusUp, WORD wPictFocusUpType,
                                 LPCTSTR lpPictFocusDown,
                                 WORD wPictFocusDownType,
                                 LPCTSTR lpPictDisabledUp,
                                 WORD wPictDisabledUpType,
                                 LPCTSTR lpPictDisabledDown,
                                 WORD wPictDisabledDownType);
LPSS_CELLTYPE SS_SetTypeComboBox(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                 long Style, LPCTSTR lpItems, short dMaxRows,
                                 short dMaxEditLen, short dComboWidth,
                                 HWND hWndDropDown, BYTE bAutoSearch);
LPSS_CELLTYPE SS_SetTypeDate(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                             long Style, LPDATEFORMAT lpFormat,
                             LPTB_DATE lpMin, LPTB_DATE lpMax);
LPSS_CELLTYPE SS_SetTypeEdit(LPSS_CELLTYPE CellType,
                             long Style, int Len, short ChrSet,
                             short ChrCase);
LPSS_CELLTYPE SS_SetTypeFloatExt(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                 long Style, short Left, short Right,
                                 double Min, double Max,
                                 LPFLOATFORMAT lpFormat);
LPSS_CELLTYPE SS_SetTypeIntegerExt(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                   long lStyle, long lMin, long lMax,
                                   BOOL fSpinWrap, long lSpinInc);
LPSS_CELLTYPE SS_SetTypeOwnerDraw(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                  long lStyle);
LPSS_CELLTYPE SS_SetTypePic(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                            long Style, LPCTSTR Mask);
LPSS_CELLTYPE SS_SetTypePicture(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                long Style, LPCTSTR PictName);
LPSS_CELLTYPE SS_SetTypePictureHandle(LPSPREADSHEET lpSS,
                                      LPSS_CELLTYPE CellType,
                                      long Style, HANDLE hPict,
                                      HPALETTE hPal, BOOL fDeleteHandle);
LPSS_CELLTYPE SS_SetTypeStaticText(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                                   long TextStyle);
LPSS_CELLTYPE SS_SetTypeTime(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                             long Style, LPTIMEFORMAT lpFormat,
                             LPTIME lpMin, LPTIME lpMax);
LRESULT SS_ComboBoxSendMessage(LPSPREADSHEET lpSS, SS_COORD Col,
                               SS_COORD Row, UINT uMsg, WPARAM wParam,
                               LPARAM lParam);
void SS_StringToNum(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPCTSTR lpszText, LPDOUBLE lpdfVal);
void SS_StringToFloat(LPSPREADSHEET lpSS, LPCTSTR nptr, LPDOUBLE lpdfVal);
#ifdef SS_V40
LPSS_CELLTYPE SS_SetTypeCurrency(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, BYTE fShowSeparator, BYTE fShowCurrencySymbol,
                 LPTSTR lpszDecimal, LPTSTR lpszSeparator, LPTSTR lpszCurrency,
                 BYTE bLeadingZero, BYTE bNegCurrencyStyle, BYTE bPosCurrencyStyle,
                 BYTE fSpin, BYTE fSpinWrap, double SpinInc);
LPSS_CELLTYPE SS_SetTypeNumber(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, BYTE fShowSeparator, LPTSTR lpszDecimal,
                 LPTSTR lpszSeparator, BYTE bLeadingZero, BYTE bNegStyle,
                 BYTE fSpin, BYTE fSpinWrap, double SpinInc);
LPSS_CELLTYPE SS_SetTypePercent(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, LPTSTR lpszDecimal, BYTE bLeadingZero, BYTE bNegStyle,
                 BYTE fSpin, BYTE fSpinWrap, double SpinInc);
void SS_CreateCurrencyFormatStruct(LPSS_CELLTYPE lpCellType, LPSSNUM_FORMAT lpFormat);
void SS_CreateNumberFormatStruct(LPSS_CELLTYPE lpCellType, LPSSNUM_FORMAT lpFormat);
void SS_CreatePercentFormatStruct(LPSS_CELLTYPE lpCellType, LPSSNUM_FORMAT lpFormat);
#endif // SS_V40

#ifdef SS_V70
LPSS_CELLTYPE SS_SetTypeScientific(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, long Style,
                 BYTE Right, double Min, double Max, LPTSTR lpszDecimal);
double SS_ScientificToFloat(LPTSTR lpszText, LPSS_CELLTYPE lpCellType);
#endif // SS_V70

//- These functions were added to support JPN Dates.(Masanori Iwasa)
BOOL SS_DateValidate(LPCTSTR lpszText, LPTB_DATE lpDate, DATEFORMAT df);
BOOL SS_DateToString(LPTSTR lpszText, LPTB_DATE lpDate, DATEFORMAT df);
BOOL SS_TimeValidate(LPCTSTR lpszText, LPTIME lpTime);
BOOL SS_TimeToString(LPTSTR lpszText, LPTIME lpTime);
long SS_GetCellAlign(LPSS_CELLTYPE lpCellType);

#ifdef SS_V80
BOOL SS_AddCustomCellType(LPSS_BOOK lpBook, LPCTSTR lpszName, BOOL bEditable, BOOL bCanOverflow, BOOL bCanBeOverflown, BOOL bUseRendererControl, LPSS_CT_PROCS lpProcs);
BOOL SS_CustCellTypeLookup(LPSS_BOOK lpBook, LPCTSTR lpszName, BOOL FAR* lpbEditable, BOOL FAR* lpbCanOverflow, BOOL FAR* lpbCanBeOverflown, BOOL FAR* lpbUseRendererControl, LPSS_CT_PROCS lpProcs);
BOOL SS_RemoveCustomCellType(LPSS_BOOK lpBook, LPCTSTR lpszName);
BOOL SS_EnumCustomCellType(LPSS_BOOK lpBook, LPCTSTR lpszPrevName, LPTSTR lpszName, int nLen);
int SS_EnumCustomCellTypeLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevName);
int SS_CustCellTypeFirst(LPSS_BOOK lpBook, LPTSTR lpszName, int nLen);
int SS_CustCellTypeFirstLen(LPSS_BOOK lpBook);
int SS_CustCellTypeNext(LPSS_BOOK lpBook, LPCTSTR lpszPrevName, LPTSTR lpszName, int nLen);
int SS_CustCellTypeNextLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevName);
LPSS_CELLTYPE SS_SetTypeCustom(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType,
                            LPCTSTR lpszName, long lStyle, LPSS_CT_VALUE ItemData);
HWND SS_GetControlRenderer(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
//HWND SS_RetrieveControlRendererhWnd(LPSPREADSHEET lpSS, SS_COORD Col,
//                            SS_COORD Row);
HWND SS_GetCustomEditor(LPSPREADSHEET lpSS, LPCTSTR Name, long lStyle);
HWND SS_GetCustomRenderer(LPSPREADSHEET lpSS, LPCTSTR lpszName, long lStyle);
void SS_CT_RefEditor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
void SS_CT_RefRenderer(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
void SS_CT_Ref(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
void SS_CT_UnrefEditor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
void SS_CT_UnrefRenderer(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
void SS_CT_Unref(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
LPSS_CELLTYPE SS_CT_LockCellType(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
void SS_CT_UnlockCellType(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);

// callback wrappers
BOOL SS_CT_CancelEditing(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow);
BOOL SS_CT_StopEditing(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow);
BOOL SS_CT_CreateEditorControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, int CtrlId, HWND *phWndCtrl);
BOOL SS_CT_CreateRendererControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, int CtrlId, HWND *phWndCtrl);
BOOL SS_CT_InitializeEditorControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow);
BOOL SS_CT_InitializeRendererControl(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow);
BOOL SS_CT_IsReservedKey(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, WPARAM wParam);
BOOL SS_CT_IsReservedLocation(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, LPRECT lpRectCell, SS_COORD Col, SS_COORD Row, int MouseX, int MouseY, HCURSOR *lpCursor);
BOOL SS_CT_IsValid(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow, LPSS_DATA DataItem);
BOOL SS_CT_GetEditorValue(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HWND hWndCtrl, SS_COORD CellCol, SS_COORD CellRow, LPSS_DATA DataItem);
BOOL SS_CT_GetPreferredSize(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HDC hDC, LPRECT lpRect, SS_COORD CellCol, SS_COORD CellRow, LPSIZE lpSize);
BOOL SS_CT_GetReservedCursor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow, long lReservedCursorId, HCURSOR *phCursor);
BOOL SS_CT_PaintCell(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT Rect, SS_COORD CellCol, SS_COORD CellRow, BOOL fSelected);
BOOL SS_CT_SetEditorValue(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow);
BOOL SS_CT_SetRendererValue(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow);
BOOL SS_CT_StartEditing(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL SS_CT_StringFormat(LPSPREADSHEET  lpSS, LPSS_CELLTYPE CellType, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE lpValue, HGLOBAL *phszDataFormatted);
BOOL SS_CT_StringUnformat(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, SS_COORD Col, SS_COORD Row, LPTSTR lpszFormatted, LPSS_DATA DataItem);
// dispatch pointers are stored in a window property named PDISPATCH
#define PDISPATCH _T("PDISPATCH")
#define CT_GetIDispatch(hWnd) (LPDISPATCH)GetProp(hWnd, PDISPATCH)
#define SS_CT_LockName(CellType) (CellType->Type == SS_TYPE_CUSTOM ? (LPTSTR)GlobalLock(CellType->Spec.Custom.hName) : NULL)
#define SS_CT_UnlockName(CellType) if( CellType->Type == SS_TYPE_CUSTOM ) GlobalUnlock(CellType->Spec.Custom.hName);
#if SS_OCX
long SS_GetControlDispatchFromHwnd(LPSPREADSHEET lpSS, HWND hWnd);
long SS_GetControlDispatch(LPSPREADSHEET lpSS, short CtrlID);
void SS_UpdateControlHwnd(LPSPREADSHEET lpSS, short CtrlID, HWND hWndNew);
void SS_CT_UpdateControlHwnd(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
#endif
HWND SS_CT_GetControlhWnd(LPSPREADSHEET lpSS, CT_HANDLE hCT, long lStyle, BOOL fRenderer);
BOOL SS_CT_IsEditorControl(LPSPREADSHEET lpSS, HWND hWnd);
BOOL SS_GetCustomItemData(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData);
BOOL SS_SetCustomItemData(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LPSS_CT_VALUE ItemData);
// in ss_edit.c
void SS_CT_SubclassEditor(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
void SS_CT_UnsubclassEditor(LPSPREADSHEET lpSS, HWND hWndCtrl);

//--------------------------------------------------------------------
//
//  The CUSTCELLTYPETABLE structure represents a table of custom
//  cell types.
//

typedef struct ss_custcelltypetable
{
  TBGLOBALHANDLE hCellTypes;  // handle to an array of SS_CUSTOMCELLTYPE
  int nElemCnt;       // number of cell types in array
  int nAllocCnt;      // number array slots which have been allocated
  int nAllocInc;      // number of additional array slots on each ReAlloc
} SS_CUSTCELLTYPETABLE, FAR* LPSS_CUSTCELLTYPETABLE;
#endif // SS_V80


#ifdef __cplusplus
}
#endif

#endif
