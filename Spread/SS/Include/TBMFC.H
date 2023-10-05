//***********************************************************************
//* FarPoint Technologies  (C) Copyright 1995  All Rights Reserved
//*
//*  32-Bit version of ToolBox spreadsheet MFC wrapper.
//***********************************************************************
         
#ifndef TBMFC_H
#define TBMFC_H 1

#include <afxwin.h>

#ifndef TOOLBOX_H
#include "toolbox.h"
#endif


class TSpread : public CWnd
{
  DECLARE_DYNAMIC(TSpread)

// Constructors
public:  
  TSpread() {}
  BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
  BOOL GetBool(short nIndex)
        { return SSGetBool(m_hWnd, nIndex); }
 
  BOOL SetBool(short nIndex, BOOL bNewVal)
        { return SSSetBool (m_hWnd, nIndex, bNewVal); }

// Operations
  BOOL AddCustomFunction(LPCSTR FunctionName, short ParameterCnt,
                         FARPROC lpfnFunctionProc)
        { return SSAddCustomFunction(m_hWnd, FunctionName,
                 ParameterCnt, lpfnFunctionProc); }

  BOOL AddMultiSelBlocks(LPSS_SELBLOCK lpSelBlock)
        { return SSAddMultiSelBlocks(m_hWnd, lpSelBlock); }

  BOOL BuildDependencies()
        { return SSBuildDependencies(m_hWnd); }

  BOOL Clear()
        { return SSClear(m_hWnd, SS_ALLCOLS, SS_ALLROWS); }

  BOOL Clear(SS_COORD Col, SS_COORD Row)
        { return SSClear(m_hWnd, Col, Row); }

  BOOL ClearData(SS_COORD Col, SS_COORD Row)
        { return SSClearData(m_hWnd, Col, Row); }

  BOOL ClearDataRange(SS_COORD Col,  SS_COORD Row, 
                       SS_COORD Col2, SS_COORD Row2)
        { return SSClearDataRange(m_hWnd, Col, Row, Col2, Row2); }

  BOOL ClearRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2)
        { return SSClearRange(m_hWnd, Col, Row, Col2, Row2); }

  BOOL ClipboardCopy(void)
        { return (BOOL)SSClipboardCopy(m_hWnd); }

  BOOL ClipboardCut(void)
        { return SSClipboardCut(m_hWnd); }

  BOOL ClipboardPaste(void)
        { return SSClipboardPaste(m_hWnd); }

  BOOL ClipIn(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
              LPSTR Buffer, long BufferLen)
        { return SSClipIn(m_hWnd, Col, Row, Col2, Row2, Buffer, BufferLen); }

  GLOBALHANDLE ClipOut(SS_COORD Col, SS_COORD Row,
                       SS_COORD Col2,SS_COORD Row2)
        { return SSClipOut(m_hWnd, Col, Row, Col2, Row2); }

  BOOL ClipValueIn (SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPSTR lpBuffer,
      LONG lBufferLen)
      { return (BOOL)SSClipValueIn(m_hWnd, Col,Row,Col2,Row2,lpBuffer,lBufferLen); }

  GLOBALHANDLE ClipValueOut (SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2)
      { return (GLOBALHANDLE)SSClipValueOut(m_hWnd, Col,Row,Col2,Row2); }
      
  long ColWidthToLogUnits (double dfColWidth)
        { return SSColWidthToLogUnits(m_hWnd, dfColWidth); }

  LRESULT ComboBoxSendMessage (SS_COORD Col, SS_COORD Row, UINT Msg, WPARAM wParam, LPARAM lParam)
        { return SSComboBoxSendMessage(m_hWnd, Col, Row, Msg, wParam, lParam); }

  BOOL CopyRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
         SS_COORD Row2, SS_COORD ColDest, SS_COORD RowDest)
        { return SSCopyRange(m_hWnd, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL DelCol(SS_COORD Col)
        { return SSDelCol(m_hWnd, Col); }

  BOOL DelColRange(SS_COORD Col, SS_COORD Col2)
        { return SSDelColRange(m_hWnd, Col, Col2); }

  BOOL DelRow(SS_COORD Row)
        { return SSDelRow(m_hWnd, Row); }

  BOOL DelRowRange(SS_COORD Row, SS_COORD Row2)
        { return SSDelRowRange(m_hWnd, Row, Row2); }

  BOOL DeSelectBlock(void)
        { return SSDeSelectBlock (m_hWnd); }

  void FloatFormat(LPSS_CELLTYPE lpCellType, double dfVal, LPSTR lpszText, BOOL fValue)
        { SSFloatFormat(m_hWnd, lpCellType, dfVal, lpszText, fValue); }

  BOOL GetActiveCell (LPSS_COORD Col, LPSS_COORD Row)
        { return SSGetActiveCell(m_hWnd, Col, Row); }

  void GetAutoSizeVisible (LPSS_COORD lpVisibleCols, LPSS_COORD lpVisibleRows)
        { SSGetAutoSizeVisible(m_hWnd, lpVisibleCols, lpVisibleRows); }

  BOOL GetBorder(SS_COORD Col, SS_COORD Row, LPWORD lpwBorderType,
         LPWORD lpwBorderStyle, LPCOLORREF lpColor)
        { return SSGetBorder(m_hWnd, Col, Row, lpwBorderType, lpwBorderStyle, lpColor); }

  BOOL GetBottomRightCell(LPSS_COORD ACol, LPSS_COORD ARow)
        { return SSGetBottomRightCell(m_hWnd, ACol, ARow); }

  WORD GetButtonDrawMode(void)
        { return SSGetButtonDrawMode(m_hWnd); }

  BOOL GetButtonText(SS_COORD Col, SS_COORD Row, LPSTR lpText)
        { return SSGetButtonText(m_hWnd, Col, Row, lpText); }

  WNDPROC GetCallBack(void)
        { return SSGetCallBack(m_hWnd); }

  BOOL GetCellFromPixel(LPSS_COORD lpCol, LPSS_COORD lpRow,
                         int MouseX, int MouseY)
        { return SSGetCellFromPixel(m_hWnd, lpCol, lpRow, MouseX, MouseY); }

  BOOL GetCellRect(SS_COORD Col, SS_COORD Row, LPRECT lpRect)
        { return SSGetCellRect(m_hWnd, Col, Row, lpRect); }

  void GetCellSendingMsg(LPSS_COORD lpCol, LPSS_COORD lpRow)
        { SSGetCellSendingMsg(m_hWnd, lpCol, lpRow); }

  BOOL GetCellType(SS_COORD Col, SS_COORD Row, LPSS_CELLTYPE CellType)
        { return SSGetCellType(m_hWnd, Col, Row, CellType); }

  void GetClientRect(LPRECT lpRect)
        { SSGetClientRect(m_hWnd, lpRect); }
                  
  BOOL GetColPageBreak(SS_COORD col)
        { return (BOOL)SSGetColPageBreak(m_hWnd, col); }

  BOOL SetColPageBreak(SS_COORD col)
        { return (BOOL)SSGetColPageBreak(m_hWnd, col); }

  BOOL GetColor(SS_COORD Col, SS_COORD Row, LPCOLORREF lpBackground,
                 LPCOLORREF lpForeground)
        { return SSGetColor(m_hWnd, Col, Row, lpBackground, lpForeground); }

  BOOL GetColUserData(SS_COORD Col,LPLONG lplUserData)
        { return SSGetColUserData(m_hWnd, Col, lplUserData); }

  BOOL GetColWidth(SS_COORD Col, LPDOUBLE lpColWidth)
        { return SSGetColWidth(m_hWnd, Col, lpColWidth); }

  BOOL GetColWidthInPixels(SS_COORD Col, LPINT lpdWidthPixels)
        { return SSGetColWidthInPixels(m_hWnd, Col, lpdWidthPixels); }

  BOOL GetCurrSelBlockPos(LPSS_COORD CurrentCol, LPSS_COORD CurrentRow)
        { return SSGetCurrSelBlockPos(m_hWnd, CurrentCol, CurrentRow); }

  HCURSOR GetCursor(WORD wType)
        { return SSGetCursor(m_hWnd, wType); }
        
  int GetData(SS_COORD Col, SS_COORD Row, LPSTR Data)
        { return SSGetData(m_hWnd, Col, Row, Data); }

  BOOL GetDataCnt(LPSS_COORD lpColCnt, LPSS_COORD lpRowCnt)
        { return SSGetDataCnt(m_hWnd, lpColCnt, lpRowCnt); }

  int GetDataLen(SS_COORD Col, SS_COORD Row)
        { return SSGetDataLen(m_hWnd, Col, Row); }

  BOOL GetDefFloatFormat(LPFLOATFORMAT lpFloatFormat)
        { return SSGetDefFloatFormat(m_hWnd, lpFloatFormat); }

  WORD GetEditEnterAction(void)
        { return SSGetEditEnterAction(m_hWnd); }

  BOOL GetEditMode(void)
        { return SSGetEditMode(m_hWnd); }
                       
  BOOL GetFirstValidCell(LPSS_COORD lpCol, LPSS_COORD lpRow)
        { return SSGetFirstValidCell(m_hWnd, lpCol, lpRow); }

  BOOL GetLastValidCell(LPSS_COORD lpCol, LPSS_COORD lpRow)
        { return SSGetLastValidCell(m_hWnd, lpCol, lpRow); }
        
  BOOL GetRowPageBreak(SS_COORD row)
        { return SSGetRowPageBreak(m_hWnd, row); }
  
  
  BOOL GetFloat(SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfValue)
        { return SSGetFloat(m_hWnd, Col, Row, lpdfValue); }

  HFONT GetFont(SS_COORD Col, SS_COORD Row)
        { return SSGetFont(m_hWnd, Col, Row); }

  short GetFormula(SS_COORD Col, SS_COORD Row, LPSTR lpFormula)
        { return SSGetFormula(m_hWnd, Col, Row, lpFormula); }

  short GetFormulaLen(SS_COORD Col, SS_COORD Row)
        { return SSGetFormulaLen(m_hWnd, Col, Row); }

  BOOL GetFreeze(LPSS_COORD lpColsFrozen, LPSS_COORD lpRowsFrozen)
        { return SSGetFreeze(m_hWnd, lpColsFrozen, lpRowsFrozen); }

  void GetGrayAreaColor(LPCOLORREF lpBackground, LPCOLORREF lpForeground)
        { SSGetGrayAreaColor(m_hWnd, lpBackground, lpForeground); }

  WORD GetGridType(void)
        { return SSGetGridType(m_hWnd); }

  COLORREF GetGridColor(void)
        { return SSGetGridColor(m_hWnd); }

  BOOL GetInteger(SS_COORD Col, SS_COORD Row, LPLONG lplValue)
        { return SSGetInteger(m_hWnd, Col, Row, lplValue); }

  BOOL GetLock(SS_COORD Col, SS_COORD Row)
        { return SSGetLock(m_hWnd, Col, Row); }

  void GetLockColor(LPCOLORREF lpBackground, LPCOLORREF lpForeground)
        { SSGetLockColor(m_hWnd, lpBackground, lpForeground); }

  SS_COORD GetMaxCols()
        { return SSGetMaxCols (m_hWnd); }

  SS_COORD GetMaxRows()
        { return SSGetMaxRows(m_hWnd); }

  BOOL GetMaxTextCellSize(SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfWidth, LPDOUBLE lpdfHeight)
        { return SSGetMaxTextCellSize(m_hWnd, Col, Row, lpdfWidth, lpdfHeight); }

  BOOL GetMaxTextColWidth(SS_COORD Col, LPDOUBLE lpdfColWidth)
        { return SSGetMaxTextColWidth(m_hWnd, Col, lpdfColWidth); }

  BOOL GetMaxTextRowHeight(SS_COORD Row, LPDOUBLE lpdfRowHeight)
        { return SSGetMaxTextRowHeight(m_hWnd, Row, lpdfRowHeight); }

  GLOBALHANDLE GetMultiSelBlocks(LPSHORT lpdSelBlockCnt)
        { return SSGetMultiSelBlocks(m_hWnd, lpdSelBlockCnt); }

  WORD GetOperationMode()
        { return SSGetOperationMode(m_hWnd); }

  HWND GetOwner()
        { return SSGetOwner(m_hWnd); }

  BOOL  GetRowHeight(SS_COORD Row, LPDOUBLE lpHeight)
        { return (BOOL)SSGetRowHeight(m_hWnd, Row, lpHeight); }

  BOOL GetRowHeightInPixels(SS_COORD Row, LPINT lpdHeightPixels)
        { return SSGetRowHeightInPixels(m_hWnd, Row, lpdHeightPixels); }

  BOOL GetRowUserData(SS_COORD Row, LPLONG lplUserData)
        { return SSGetRowUserData(m_hWnd, Row, lplUserData); }

  BOOL GetSel(LPINT SelStart, LPINT SelEnd)
        { return SSGetSel (m_hWnd, SelStart, SelEnd); }

  WORD GetSelBlockOptions()
        { return SSGetSelBlockOptions(m_hWnd); }

  BOOL GetSelectBlock(LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
        { return SSGetSelectBlock (m_hWnd, CellUL, CellLR); }

  GLOBALHANDLE GetSelText()
        { return SSGetSelText (m_hWnd); }

  void GetShadowColor(LPCOLORREF lpShadowColor, LPCOLORREF lpShadowText,
                      LPCOLORREF lpShadowDark,  LPCOLORREF lpShadowLight)
        { SSGetShadowColor (m_hWnd, lpShadowColor, lpShadowText,
                            lpShadowDark, lpShadowLight); }

  short GetUserResizeCol(SS_COORD Col)
        { return SSGetUserResizeCol(m_hWnd, Col); }

  short GetUserResizeRow(SS_COORD Row)
        { return SSGetUserResizeRow(m_hWnd, Row); }

  BOOL GetTopLeftCell(LPSS_COORD ACol, LPSS_COORD ARow)
        { return SSGetTopLeftCell (m_hWnd, ACol, ARow); }

  WORD GetUnitType()
        { return SSGetUnitType(m_hWnd); }

  LONG GetUserData()
        { return SSGetUserData(m_hWnd); }

  WORD GetUserResize()
        { return SSGetUserResize(m_hWnd); }

  int GetValue(SS_COORD Col, SS_COORD Row, LPSTR lpBuffer)
        { return SSGetValue (m_hWnd, Col, Row, lpBuffer); }

  int GetValueLen(SS_COORD Col, SS_COORD Row)
        { return SSGetValueLen (m_hWnd, Col, Row); }

  BOOL GetVisible(SS_COORD Col, SS_COORD Row, short Visible)
        { return SSGetVisible (m_hWnd, Col, Row, Visible); }

  BOOL InsCol(SS_COORD Col)
        { return SSInsCol (m_hWnd, Col); }

  BOOL InsColRange(SS_COORD Col, SS_COORD Col2)
        { return SSInsColRange(m_hWnd, Col, Col2); }

  BOOL InsRow(SS_COORD Row)
        { return SSInsRow (m_hWnd, Row); }

  BOOL InsRowRange(SS_COORD Row, SS_COORD Row2)
        { return SSInsRowRange(m_hWnd, Row, Row2); }

  BOOL IsCellInSelection(SS_COORD Col, SS_COORD Row)
        { return SSIsCellInSelection(m_hWnd, Col, Row); }

  BOOL LoadFromBuffer(GLOBALHANDLE hBuffer, long lBufferLen)
        { return SSLoadFromBuffer (m_hWnd, hBuffer, lBufferLen); }

  BOOL LoadFromFile(LPCSTR lpFileName)
        { return SSLoadFromFile (m_hWnd, lpFileName); }

  void LogUnitsToColWidth(short dUnits, LPDOUBLE lpWidth)
        { SSLogUnitsToColWidth(m_hWnd, dUnits, lpWidth); }

  void LogUnitsToRowHeight(SS_COORD Row, short dUnits, LPDOUBLE lpHeight)
        { SSLogUnitsToRowHeight(m_hWnd, Row, dUnits, lpHeight); }

  BOOL MoveRange(SS_COORD Col,SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
         SS_COORD ColDest, SS_COORD RowDest)
        { return SSMoveRange(m_hWnd, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL Print(LPCSTR lpszPrintJobName, LPSS_PRINTFORMAT lpPrintFormat,
             SS_PRINTFUNC lpfnPrintProc, LONG lAppData, ABORTPROC lpfnAbortProc)
        { return SSPrint(m_hWnd, lpszPrintJobName, lpPrintFormat,
                 lpfnPrintProc, lAppData, lpfnAbortProc); }

  BOOL ReCalc()
        { return SSReCalc (m_hWnd); }

  BOOL ReplaceSel(LPCSTR lpText)
        { return SSReplaceSel (m_hWnd, lpText); }

  short RowHeightToLogUnits(SS_COORD Row, double dfRowHeight)
        { return RowHeightToLogUnits (Row, dfRowHeight); }

  GLOBALHANDLE SaveToBuffer(long far *lpBufferLen, BOOL fDataOnly)
        { return SSSaveToBuffer (m_hWnd, lpBufferLen, fDataOnly); }
                    
  BOOL SaveToFile(LPCSTR lpFileName, BOOL fDataOnly)
        { return SSSaveToFile (m_hWnd, lpFileName, fDataOnly); }

  long SelModeSendMessage(UINT uMsg, long lParam1, long lParam2, long lParam3)
        { return SSSelModeSendMessage(m_hWnd, uMsg, lParam1, lParam2, lParam3); }

  BOOL SetActiveCell(SS_COORD Col, SS_COORD Row)
        { return SSSetActiveCell (m_hWnd, Col, Row); }

  void SetAutoSizeVisible(SS_COORD VisibleCols, SS_COORD VisibleRows)
        { SSSetAutoSizeVisible (m_hWnd, VisibleCols, VisibleRows); }

  BOOL SetBorder(SS_COORD Col, SS_COORD Row, WORD wBorderType,
         WORD wBorderStyle, COLORREF Color)
        { return SSSetBorder(m_hWnd, Col, Row, wBorderType, wBorderStyle, Color); }

  BOOL SetBorderRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                      SS_COORD Row2, WORD wBorderType, WORD wBorderStyle,
              COLORREF Color)
        { return SSSetBorderRange(m_hWnd, Col, Row, Col2, Row2, wBorderType, wBorderStyle, Color); }

  WORD SetButtonDrawMode(WORD wMode)
        { return SSSetButtonDrawMode(m_hWnd, wMode); }

  BOOL SetButtonText(SS_COORD Col, SS_COORD Row, LPCSTR lpText)
        { return SSSetButtonText(m_hWnd, Col, Row, lpText); }

  WNDPROC SetCallBack(WNDPROC lpfnCallBack)
        { return SSSetCallBack(m_hWnd, lpfnCallBack); }

  BOOL SetCellType(SS_COORD Col, SS_COORD Row, LPSS_CELLTYPE CellType)
        { return SSSetCellType(m_hWnd, Col, Row, CellType); }

  BOOL SetCellTypeRange(SS_COORD Col,  SS_COORD Row, SS_COORD Col2,
                        SS_COORD Row2, LPSS_CELLTYPE CellType)
        { return SSSetCellTypeRange(m_hWnd, Col, Row, Col2, Row2, CellType); }

  BOOL SetColor(SS_COORD Col, SS_COORD Row, COLORREF Background,
                COLORREF Foreground)
        { return SSSetColor(m_hWnd, Col, Row, Background, Foreground); }

  BOOL SetColorRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
                     COLORREF Background, COLORREF Foreground)
        { return SSSetColorRange(m_hWnd, Col, Row, Col2, Row2, Background, Foreground); }

  BOOL SetColUserData(SS_COORD Col, long lUserData)
        { return SSSetColUserData(m_hWnd, Col, lUserData); }

  BOOL SetColWidth(SS_COORD Col, double Width)
        { return SSSetColWidth(m_hWnd, Col, Width); }

  BOOL SetColWidthInPixels(SS_COORD Col, int dWidth)
        { return SSSetColWidthInPixels(m_hWnd, Col, dWidth); }

  BOOL SetCursor(WORD wType, HCURSOR hCursor)
        { return SSSetCursor(m_hWnd, wType, hCursor); }

  BOOL SetData(SS_COORD Col, SS_COORD Row, LPCSTR lpData)
        { return SSSetData (m_hWnd, Col, Row, lpData); }

  BOOL SetDataRange(SS_COORD Col, SS_COORD Row,
                    SS_COORD Col2,SS_COORD Row2, LPCSTR lpData)
        { return SSSetDataRange (m_hWnd, Col, Row, Col2, Row2, lpData); }

  BOOL SetDefFloatFormat(LPFLOATFORMAT lpFloatFormat)
        { return SSSetDefFloatFormat(m_hWnd, lpFloatFormat); }

  WORD SetEditEnterAction(WORD wAction)
        { return SSSetEditEnterAction(m_hWnd, wAction); }

  BOOL SetEditMode(BOOL fEditModeOn)
        { return SSSetEditMode(m_hWnd, fEditModeOn); }

  BOOL SetFont(SS_COORD Col, SS_COORD Row, HFONT hFont, BOOL fDeleteFont)
        { return SSSetFont(m_hWnd, Col, Row, hFont, fDeleteFont); }

  BOOL SetFontRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                    SS_COORD Row2, HFONT hFont, BOOL fDeleteFont)
        { return SSSetFontRange(m_hWnd, Col, Row, Col2, Row2, hFont, fDeleteFont); }

  BOOL SetFormula(SS_COORD Col, SS_COORD Row, LPCSTR Formula,
                  BOOL BuildDependencies)
        { return SSSetFormula (m_hWnd, Col, Row, Formula, BuildDependencies); }

  BOOL SetFormulaRange(SS_COORD Col, SS_COORD Row,
                       SS_COORD Col2,SS_COORD Row2,
                       LPCSTR Formula, BOOL BuildDependencies)
        { return SSSetFormulaRange(m_hWnd, Col, Row, Col2, Row2, Formula, 
                                   BuildDependencies); }

  BOOL SetFloat(SS_COORD Col, SS_COORD Row, double dfValue)
        { return SSSetFloat(m_hWnd, Col, Row, dfValue); }

  BOOL SetFloatRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, double dfValue)
        { return SSSetFloatRange(m_hWnd, Col, Row, Col2, Row2, dfValue); }

  BOOL SetFreeze(SS_COORD ColsFrozen, SS_COORD RowsFrozen)
         { return SSSetFreeze (m_hWnd, ColsFrozen, RowsFrozen); }

  void SetGrayAreaColor(COLORREF Background, COLORREF Foreground)
        { SSSetGrayAreaColor(m_hWnd, Background, Foreground); }

  WORD SetGridType(WORD wGridType)
        { return SSSetGridType(m_hWnd, wGridType); }

  COLORREF SetGridColor(COLORREF Color)
        { return SSSetGridColor(m_hWnd, Color); }

  BOOL SetInteger(SS_COORD Col, SS_COORD Row, long lValue)
        { return SSSetInteger(m_hWnd, Col, Row, lValue); }

  BOOL SetIntegerRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, long lValue)
        { return SSSetIntegerRange(m_hWnd, Col, Row, Col2, Row2, lValue); }

  BOOL SetLock(SS_COORD Col, SS_COORD Row, BOOL Lock)
        { return SSSetLock (m_hWnd, Col, Row, Lock); }

  void SetLockColor(COLORREF Background, COLORREF Foreground)
        { SSSetLockColor(m_hWnd, Background, Foreground); }

  BOOL SetLockRange(SS_COORD Col,  SS_COORD Row,
                    SS_COORD Col2, SS_COORD Row2, BOOL Lock)
        { return SSSetLockRange (m_hWnd, Col, Row, Col2, Row2, Lock); }

  void SetMaxCols(SS_COORD MaxCols)
        { SSSetMaxCols(m_hWnd, MaxCols); }

  void SetMaxRows(SS_COORD MaxRows)
        { SSSetMaxRows(m_hWnd, MaxRows); }

  BOOL SetMultiSelBlocks(LPSS_SELBLOCK lpSelBlockList, short dSelBlockCnt)
        { return SSSetMultiSelBlocks(m_hWnd, lpSelBlockList, dSelBlockCnt); }

  WORD SetOperationMode(WORD wMode)
        { return SSSetOperationMode(m_hWnd, wMode); }
  
  HWND SetOwner(HWND hWndOwner)
        { return SSSetOwner(m_hWnd, hWndOwner); }

  BOOL SetRowHeight(SS_COORD Row, double Height)
        { return SSSetRowHeight (m_hWnd, Row, Height); }

  BOOL SetRowHeightInPixels(SS_COORD Row, int dHeight)
        { return SSSetRowHeightInPixels(m_hWnd, Row, dHeight); }

  BOOL SetRowUserData(SS_COORD Row, long lUserData)
        { return SSSetRowUserData(m_hWnd, Row, lUserData); }

  BOOL SetSel(short SelStart, short SelEnd)
        { return SSSetSel(m_hWnd, SelStart, SelEnd); }

  BOOL SetSelectBlock(LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
        { return SSSetSelectBlock (m_hWnd, CellUL, CellLR); }

  void SetShadowColor(COLORREF ShadowColor, COLORREF ShadowText,
                      COLORREF ShadowDark,  COLORREF ShadowLight)
        { SSSetShadowColor(m_hWnd, ShadowColor, ShadowText,
                           ShadowDark, ShadowLight); }

  LPSS_CELLTYPE SetTypeButton(LPSS_CELLTYPE CellType, long Style,
                              LPCSTR Text, LPCSTR Picture, 
                              short nPictureType, LPCSTR PictureDown,
                              short nPictureDownType, short ButtonType,
                              short nShadowSize, LPSUPERBTNCOLOR lpColor)
        { return SSSetTypeButton(m_hWnd, CellType, Style, 
                                 Text, Picture, nPictureType,
                                 PictureDown, nPictureDownType,
                                 ButtonType, nShadowSize, lpColor); }
 
  LPSS_CELLTYPE SetTypeCheckBox(LPSS_CELLTYPE lpCellType,
                long lStyle, LPCSTR lpText,
                LPCSTR lpPictUp, WORD wPictUpType,
                LPCSTR lpPictDown, WORD wPictDownType,
                LPCSTR lpPictFocusUp, WORD wPictFocusUpType,
                LPCSTR lpPictFocusDown, WORD wPictFocusDownType,
                LPCSTR lpPictDisabledUp, WORD wPictDisabledUpType,
                LPCSTR lpPictDisabledDown, WORD wPictDisabledDownType)
        { return SSSetTypeCheckBox(m_hWnd, lpCellType, lStyle, lpText, lpPictUp, wPictUpType,
                   lpPictDown, wPictDownType, lpPictFocusUp, wPictFocusUpType,
                   lpPictFocusDown, wPictFocusDownType, lpPictDisabledUp, wPictDisabledUpType,
                   lpPictDisabledDown, wPictDisabledDownType) ; }

  LPSS_CELLTYPE SetTypeComboBox(LPSS_CELLTYPE CellType, long Style,
                                LPCSTR lpItems)
        { return SSSetTypeComboBox(m_hWnd, CellType, Style, lpItems); }

  LPSS_CELLTYPE SetTypeDate(LPSS_CELLTYPE CellType, long Style,
                            LPDATEFORMAT lpFormat, LPTB_DATE lpMin, 
                            LPTB_DATE lpMax)
        { return SSSetTypeDate(m_hWnd, CellType, Style, lpFormat, lpMin, lpMax); }

  LPSS_CELLTYPE SetTypeEdit(LPSS_CELLTYPE CellType, long Style, short Len,
                            short ChrSet, char ChrCase)
        { return SSSetTypeEdit(m_hWnd, CellType, Style, Len, ChrSet, ChrCase); }


  LPSS_CELLTYPE SetTypeFloat(LPSS_CELLTYPE CellType, long Style,
                             short Left, short Right, double Min, 
                             double Max)
        { return SSSetTypeFloat(m_hWnd, CellType, Style, Left, Right, Min, Max); }

  LPSS_CELLTYPE SetTypeFloatExt(LPSS_CELLTYPE CellType, long Style,
                             short Left, short Right, double Min, 
                             double Max, LPFLOATFORMAT lpFormat)
        { return SSSetTypeFloatExt(m_hWnd, CellType, Style, Left, Right, Min, Max, lpFormat); }

  LPSS_CELLTYPE SetTypeInteger(LPSS_CELLTYPE CellType, long Min, long Max)
        { return SSSetTypeInteger(m_hWnd, CellType, Min, Max); }

  LPSS_CELLTYPE SetTypeIntegerExt(LPSS_CELLTYPE CellType, long lStyle, long lMin,
                  long lMax, BOOL fSpinWrap, long lSpinInc)
        { return SSSetTypeIntegerExt(m_hWnd, CellType, lStyle, lMin, lMax, fSpinWrap, lSpinInc); }

  LPSS_CELLTYPE SetTypeOwnerDraw(LPSS_CELLTYPE lpCellType, long lStyle)
        { return SSSetTypeOwnerDraw(m_hWnd, lpCellType, lStyle); }

  LPSS_CELLTYPE SetTypePic(LPSS_CELLTYPE CellType, long Style, LPCSTR Mask)
        { return SSSetTypePic(m_hWnd, CellType, Style, Mask); }

  LPSS_CELLTYPE SetTypePicture(LPSS_CELLTYPE CellType, long Style,
                               LPCSTR PictName)
        { return SSSetTypePicture(m_hWnd, CellType, Style, PictName); }

  LPSS_CELLTYPE SetTypePictureHandle(LPSS_CELLTYPE CellType, long Style, HANDLE hPict,
                     HPALETTE hPal, BOOL fDeleteHandle)
        { return SSSetTypePictureHandle(m_hWnd, CellType, Style, hPict, hPal, fDeleteHandle); }

  LPSS_CELLTYPE SetTypeStaticText(LPSS_CELLTYPE CellType, short TextStyle)
        { return SSSetTypeStaticText(m_hWnd, CellType, TextStyle); }

  LPSS_CELLTYPE SetTypeTime(LPSS_CELLTYPE CellType, long Style,
                            LPTIMEFORMAT lpFormat, LPTIME lpMin,
                            LPTIME lpMax)
        { return SSSetTypeTime (m_hWnd, CellType, Style, lpFormat, lpMin, lpMax); }

  WORD SetUnitType(WORD wUnitType)
        { return SSSetUnitType(m_hWnd, wUnitType); }

  LONG SetUserData(LONG lUserData)
        { return SSSetUserData(m_hWnd, lUserData); }

  short SetUserResizeCol(SS_COORD Col, short dOption)
        { return SSSetUserResizeCol(m_hWnd, Col, dOption); }

  short SetUserResizeRow(SS_COORD Row, short dOption)
        { return SSSetUserResizeRow(m_hWnd, Row, dOption); }

  WORD SetUserResize(WORD wUserResize)
        { return SSSetUserResize(m_hWnd, wUserResize); }

  WORD SetSelBlockOptions(WORD wOption)
        { return SSSetSelBlockOptions(m_hWnd, wOption); }

  BOOL SetValue(SS_COORD Col, SS_COORD Row, LPCSTR lpData)
        { return SSSetValue (m_hWnd, Col, Row, lpData); }

  BOOL SetValueRange(SS_COORD Col,  SS_COORD Row,
                     SS_COORD Col2, SS_COORD Row2, LPCSTR lpData)
        { return SSSetValueRange (m_hWnd, Col, Row, Col2, Row2, lpData); }

  BOOL ShowActiveCell(short Position)
        { return SSShowActiveCell (m_hWnd, Position); }

  BOOL ShowCell(SS_COORD Col, SS_COORD Row, short Position)
        { return SSShowCell (m_hWnd, Col, Row, Position); }

  BOOL ShowCol(SS_COORD Col, BOOL fShow)
        { return SSShowCol(m_hWnd, Col, fShow); }

  BOOL ShowRow(SS_COORD Row, BOOL fShow)
        { return SSShowRow(m_hWnd, Row, fShow); }

  BOOL Sort(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPSS_SORT lpSort)
        { return SSSort(m_hWnd, Col, Row, Col2, Row2, lpSort); }

  BOOL SwapRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
         SS_COORD ColDest, SS_COORD RowDest)
        { return SSSwapRange(m_hWnd, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL ValidateFormula (LPCSTR lpszFormula)
        { return SSValidateFormula (m_hWnd, lpszFormula); }

  void VGetBufferSize(LPSS_COORD lpdBufferSize, LPSS_COORD lpdOverlap)
        { SSVGetBufferSize(m_hWnd, lpdBufferSize, lpdOverlap); }

  SS_COORD VGetMax()
        { return SSVGetMax(m_hWnd); }

  SS_COORD VGetPhysBufferSize(void)
        { return SSVGetPhysBufferSize(m_hWnd); }

  SS_COORD VGetPhysBufferTop(void)
        { return SSVGetPhysBufferTop(m_hWnd); }

  LONG VGetStyle(void)
        { return SSVGetStyle(m_hWnd); }

  BOOL VScrollGetSpecial(LPWORD lpwOptions)
        { return SSVScrollGetSpecial(m_hWnd, lpwOptions); }

  BOOL VScrollSetSpecial(BOOL fUseSpecialVScroll, WORD wOptions)
        { return SSVScrollSetSpecial(m_hWnd, fUseSpecialVScroll, wOptions); }

  void VSetBufferSize(SS_COORD dBufferSize, SS_COORD dOverlap)
        { SSVSetBufferSize(m_hWnd, dBufferSize, dOverlap); }

  SS_COORD VSetMax(SS_COORD Max)
        { return SSVSetMax(m_hWnd, Max); }

  LONG VSetStyle(LONG lStyle)
        { return SSVSetStyle(m_hWnd, lStyle); }

// Implementation
  virtual ~TSpread() {};
  virtual WNDPROC* GetSuperWndProcAddr();

}; // end class TSpread

#endif

