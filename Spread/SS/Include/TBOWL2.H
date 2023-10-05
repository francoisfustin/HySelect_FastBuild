//--------------------------------------------------------------------
//
//  FarPoint Technologies  (C) Copyright 1995,  All Rights Reserved
//
//  32-Bit version of ToolBox spreadsheet OWL wrapper.
//

#ifndef __TBOWL_H
#define __TBOWL_H

#ifndef __OWL_H
#include <owl\owlpch.h>
#endif

#ifndef __EDIT_H
#include <owl\edit.h>
#endif

#ifndef __LISTBOX_H
#include <owl\listbox.h>
#endif

#ifndef TOOLBOX_H
#include <toolbox.h>
#endif


class TSpread: public TControl
{
public:

  TSpread (TWindow* AParent, int AnId, LPCSTR ATitle, int X,
		int Y, int W, int H, TModule* AModule = NULL)
	: TControl(AParent, AnId, ATitle, X, Y, W, H, AModule)
  {Attr.Style |= WS_BORDER | WS_HSCROLL | WS_VSCROLL;}


  TSpread (TWindow* AParent, int ResourceId, TModule* AModule = NULL)
	: TControl(AParent, ResourceId, AModule) {}

  BOOL AddCustomFunction(LPCSTR FunctionName, short ParameterCnt,
			 FARPROC lpfnFunctionProc)
	{ return SSAddCustomFunction(HWindow, FunctionName,
		 ParameterCnt, lpfnFunctionProc); }

  BOOL AddMultiSelBlocks(LPSS_SELBLOCK lpSelBlock)
	{ return SSAddMultiSelBlocks(HWindow, lpSelBlock); }

  BOOL BuildDependencies(void)
	{ return SSBuildDependencies(HWindow); }

  BOOL Clear(SS_COORD Col, SS_COORD Row)
	{ return SSClear(HWindow, Col, Row); }

  BOOL ClearData(SS_COORD Col, SS_COORD Row)
	{ return SSClearData(HWindow, Col, Row); }

  BOOL ClearDataRange(SS_COORD Col,  SS_COORD Row,
		       SS_COORD Col2, SS_COORD Row2)
	{ return SSClearDataRange(HWindow, Col, Row, Col2, Row2); }

  BOOL ClearRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2)
	{ return SSClearRange(HWindow, Col, Row, Col2, Row2); }

  BOOL ClipboardCopy(void)
	{ return (BOOL)SSClipboardCopy(HWindow); }

  BOOL ClipboardCut(void)
	{ return SSClipboardCut(HWindow); }

  BOOL ClipboardPaste(void)
	{ return SSClipboardPaste(HWindow); }

  BOOL ClipIn(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
	      LPSTR Buffer, long BufferLen)
	{ return SSClipIn(HWindow, Col, Row, Col2, Row2, Buffer, BufferLen); }

  GLOBALHANDLE ClipOut(SS_COORD Col, SS_COORD Row,
		       SS_COORD Col2,SS_COORD Row2)
	{ return SSClipOut(HWindow, Col, Row, Col2, Row2); }

  BOOL ClipValueIn(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
	      LPSTR Buffer, long BufferLen)
	{ return SSClipValueIn(HWindow, Col, Row, Col2, Row2, Buffer, BufferLen); }

  GLOBALHANDLE ClipValueOut(SS_COORD Col, SS_COORD Row,
			    SS_COORD Col2,SS_COORD Row2)
	{ return SSClipValueOut(HWindow, Col, Row, Col2, Row2); }

  int ColWidthToLogUnits (double dfColWidth)
	{ return SSColWidthToLogUnits(HWindow, dfColWidth); }

  LRESULT ComboBoxSendMessage (SS_COORD Col, SS_COORD Row, UINT Msg, WPARAM wParam, LPARAM lParam)
	{ return SSComboBoxSendMessage(HWindow, Col, Row, Msg, wParam, lParam); }

  BOOL CopyRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                 SS_COORD Row2, SS_COORD ColDest, SS_COORD RowDest)
	{ return SSCopyRange(HWindow, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL DelCol(SS_COORD Col)
	{ return SSDelCol(HWindow, Col); }

  BOOL DelColRange(SS_COORD Col, SS_COORD Col2)
	{ return SSDelColRange(HWindow, Col, Col2); }

  BOOL DelRow(SS_COORD Row)
	{ return SSDelRow(HWindow, Row); }

  BOOL DelRowRange(SS_COORD Row, SS_COORD Row2)
	{ return SSDelRowRange(HWindow, Row, Row2); }

  BOOL DeSelectBlock(void)
	{ return SSDeSelectBlock (HWindow); }

  void FloatFormat(LPSS_CELLTYPE lpCellType, double dfVal, LPSTR lpszText, BOOL fValue)
	{ SSFloatFormat(HWindow, lpCellType, dfVal, lpszText, fValue); }

  BOOL GetActiveCell (LPSS_COORD Col, LPSS_COORD Row)
	{ return SSGetActiveCell(HWindow, Col, Row); }

  void GetAutoSizeVisible (LPSS_COORD lpVisibleCols, LPSS_COORD lpVisibleRows)
	{ SSGetAutoSizeVisible(HWindow, lpVisibleCols, lpVisibleRows); }

  BOOL GetBool(short nIndex)
	{ return SSGetBool(HWindow, nIndex); }

  BOOL GetBorder(SS_COORD Col, SS_COORD Row, LPWORD lpwBorderType,
		 LPWORD lpwBorderStyle, LPCOLORREF lpColor)
	{ return SSGetBorder(HWindow, Col, Row, lpwBorderType, lpwBorderStyle, lpColor); }

  BOOL GetBottomRightCell(LPSS_COORD ACol, LPSS_COORD ARow)
	{ return SSGetBottomRightCell(HWindow, ACol, ARow); }

  WORD GetButtonDrawMode(void)
	{ return SSGetButtonDrawMode(HWindow); }

  BOOL GetButtonText(SS_COORD Col, SS_COORD Row, LPSTR lpText)
	{ return SSGetButtonText(HWindow, Col, Row, lpText); }

  WNDPROC GetCallBack(void)
	{ return SSGetCallBack(HWindow); }

  BOOL GetCellFromPixel(LPSS_COORD lpCol, LPSS_COORD lpRow,
			 int MouseX, int MouseY)
	{ return SSGetCellFromPixel(HWindow, lpCol, lpRow, MouseX, MouseY); }

  BOOL GetCellRect(SS_COORD Col, SS_COORD Row, LPRECT lpRect)
	{ return SSGetCellRect(HWindow, Col, Row, lpRect); }

  void GetCellSendingMsg(LPSS_COORD lpCol, LPSS_COORD lpRow)
	{ SSGetCellSendingMsg(HWindow, lpCol, lpRow); }

  BOOL GetCellType(SS_COORD Col, SS_COORD Row, LPSS_CELLTYPE CellType)
	{ return SSGetCellType(HWindow, Col, Row, CellType); }

  void GetClientRect(LPRECT lpRect)
	{ SSGetClientRect(HWindow, lpRect); }

  BOOL GetColPageBreak(SS_COORD Col)
       { return SSGetColPageBreak(HWindow, Col); }

  BOOL GetColor(SS_COORD Col, SS_COORD Row, LPCOLORREF lpBackground,
		 LPCOLORREF lpForeground)
	{ return SSGetColor(HWindow, Col, Row, lpBackground, lpForeground); }

  BOOL GetColUserData(SS_COORD Col,LPLONG lplUserData)
	{ return SSGetColUserData(HWindow, Col, lplUserData); }

  BOOL GetColWidth(SS_COORD Col, LPDOUBLE lpColWidth)
	{ return SSGetColWidth(HWindow, Col, lpColWidth);  }

  BOOL GetColWidthInPixels(SS_COORD Col, LPINT lpdWidthPixels)
	{ return SSGetColWidthInPixels(HWindow, Col, lpdWidthPixels); }

  BOOL GetCurrSelBlockPos(LPSS_COORD CurrentCol, LPSS_COORD CurrentRow)
	{ return SSGetCurrSelBlockPos(HWindow, CurrentCol, CurrentRow); }

  HCURSOR GetCursor(WORD wType)
	{ return SSGetCursor(HWindow, wType); }
	
  int GetData(SS_COORD Col, SS_COORD Row, LPSTR Data)
	{ return SSGetData(HWindow, Col, Row, Data); }

  BOOL GetDataCnt(LPSS_COORD lpColCnt, LPSS_COORD lpRowCnt)
	{ return SSGetDataCnt(HWindow, lpColCnt, lpRowCnt); }

  int GetDataLen(SS_COORD Col, SS_COORD Row)
	{ return SSGetDataLen(HWindow, Col, Row); }

  BOOL GetDefFloatFormat(LPFLOATFORMAT lpFloatFormat)
	{ return SSGetDefFloatFormat(HWindow, lpFloatFormat); }

  WORD GetEditEnterAction(void)
	 { return SSGetEditEnterAction(HWindow); }

  BOOL GetEditMode(void)
	{ return SSGetEditMode(HWindow); }

  BOOL GetFirstValidCell(LPSS_COORD lpCol, LPSS_COORD lpRow)
       { return SSGetFirstValidCell(HWindow, lpCol, lpRow); }

  BOOL GetFloat(SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfValue)
	{ return SSGetFloat(HWindow, Col, Row, lpdfValue); }

  HFONT GetFont(SS_COORD Col, SS_COORD Row)
	{ return SSGetFont(HWindow, Col, Row); }

  short GetFormula(SS_COORD Col, SS_COORD Row, LPSTR lpFormula)
	{ return SSGetFormula(HWindow, Col, Row, lpFormula); }

  short GetFormulaLen(SS_COORD Col, SS_COORD Row)
	{ return SSGetFormulaLen(HWindow, Col, Row); }

  BOOL GetFreeze(LPSS_COORD lpColsFrozen, LPSS_COORD lpRowsFrozen)
	{ return SSGetFreeze(HWindow, lpColsFrozen, lpRowsFrozen); }

  void GetGrayAreaColor(LPCOLORREF lpBackground, LPCOLORREF lpForeground)
	{ SSGetGrayAreaColor(HWindow, lpBackground, lpForeground); }

  WORD GetGridType(void)
	{ return SSGetGridType(HWindow); }

  COLORREF GetGridColor(void)
	{ return SSGetGridColor(HWindow); }

  BOOL GetInteger(SS_COORD Col, SS_COORD Row, LPLONG lplValue)
	{ return SSGetInteger(HWindow, Col, Row, lplValue); }

  BOOL GetLastValidCell(LPSS_COORD lpCol, LPSS_COORD lpRow)
       { return SSGetLastValidCell(HWindow, lpCol, lpRow); }

  BOOL GetLock(SS_COORD Col, SS_COORD Row)
	{ return SSGetLock(HWindow, Col, Row); }

  void GetLockColor(LPCOLORREF lpBackground, LPCOLORREF lpForeground)
	{ SSGetLockColor(HWindow, lpBackground, lpForeground); }

  SS_COORD GetMaxCols(void)
	{ return SSGetMaxCols (HWindow); }

  SS_COORD GetMaxRows(void)
	{ return SSGetMaxRows(HWindow); }

  BOOL GetMaxTextCellSize(SS_COORD Col, SS_COORD Row, LPDOUBLE lpdfWidth, LPDOUBLE lpdfHeight)
	{ return SSGetMaxTextCellSize(HWindow, Col, Row, lpdfWidth, lpdfHeight); }

  BOOL GetMaxTextColWidth(SS_COORD Col, LPDOUBLE lpdfColWidth)
	{ return SSGetMaxTextColWidth(HWindow, Col, lpdfColWidth); }

  BOOL GetMaxTextRowHeight(SS_COORD Row, LPDOUBLE lpdfRowHeight)
	{ return SSGetMaxTextRowHeight(HWindow, Row, lpdfRowHeight); }

  GLOBALHANDLE GetMultiSelBlocks(LPSHORT lpdSelBlockCnt)
	{ return SSGetMultiSelBlocks(HWindow, lpdSelBlockCnt); }

  WORD GetOperationMode(void)
	{ return SSGetOperationMode(HWindow); }

  HWND GetOwner(void)
	{ return SSGetOwner(HWindow); }

  BOOL GetRowHeight(SS_COORD Row, LPDOUBLE lpHeight)
	{ return SSGetRowHeight(HWindow, Row, lpHeight);  }

  BOOL GetRowHeightInPixels(SS_COORD Row, LPINT lpdHeightPixels)
	{ return SSGetRowHeightInPixels(HWindow, Row, lpdHeightPixels); }

  BOOL GetRowPageBreak(SS_COORD Row)
       { return SSGetRowPageBreak(HWindow, Row); }

  BOOL GetRowUserData(SS_COORD Row, LPLONG lplUserData)
	{ return SSGetRowUserData(HWindow, Row, lplUserData); }

  BOOL GetSel(LPSHORT SelStart, LPSHORT SelEnd)
	{ return SSGetSel (HWindow, SelStart, SelEnd); }

  WORD GetSelBlockOptions(void)
	{ return SSGetSelBlockOptions(HWindow); }

  BOOL GetSelectBlock(LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
	{ return SSGetSelectBlock (HWindow, CellUL, CellLR); }

  GLOBALHANDLE GetSelText (void)
	{ return SSGetSelText (HWindow); }

  void GetShadowColor(LPCOLORREF lpShadowColor, LPCOLORREF lpShadowText,
		      LPCOLORREF lpShadowDark,  LPCOLORREF lpShadowLight)
	{ SSGetShadowColor (HWindow, lpShadowColor, lpShadowText,
			    lpShadowDark, lpShadowLight); }

  short GetUserResizeCol(SS_COORD Col)
	{ return SSGetUserResizeCol(HWindow, Col); }

  short GetUserResizeRow(SS_COORD Row)
	{ return SSGetUserResizeRow(HWindow, Row); }

  BOOL GetTopLeftCell(LPSS_COORD ACol, LPSS_COORD ARow)
	{ return SSGetTopLeftCell (HWindow, ACol, ARow); }

  WORD GetUnitType(void)
	{ return SSGetUnitType(HWindow); }

  LONG GetUserData(void)
	{ return SSGetUserData(HWindow); }

  WORD GetUserResize(void)
	{ return SSGetUserResize(HWindow); }

  int GetValue(SS_COORD Col, SS_COORD Row, LPSTR lpBuffer)
	{ return SSGetValue (HWindow, Col, Row, lpBuffer); }

  int GetValueLen(SS_COORD Col, SS_COORD Row)
	{ return SSGetValueLen (HWindow, Col, Row); }

  BOOL GetVisible(SS_COORD Col, SS_COORD Row, short Visible)
	{ return SSGetVisible (HWindow, Col, Row, Visible); }

  BOOL InsCol(SS_COORD Col)
	{ return SSInsCol (HWindow, Col); }

  BOOL InsColRange(SS_COORD Col, SS_COORD Col2)
	{ return SSInsColRange(HWindow, Col, Col2); }

  BOOL InsRow(SS_COORD Row)
	{ return SSInsRow (HWindow, Row); }

  BOOL InsRowRange(SS_COORD Row, SS_COORD Row2)
	{ return SSInsRowRange(HWindow, Row, Row2); }

  BOOL IsCellInSelection(SS_COORD Col, SS_COORD Row)
	{ return SSIsCellInSelection(HWindow, Col, Row); }

  BOOL LoadFromBuffer(GLOBALHANDLE hBuffer, long lBufferLen)
	{ return SSLoadFromBuffer (HWindow, hBuffer, lBufferLen); }

  BOOL LoadFromFile(LPCSTR lpFileName)
	{ return SSLoadFromFile (HWindow, lpFileName); }

  void LogUnitsToColWidth(short dUnits, LPDOUBLE lpWidth)
	{ SSLogUnitsToColWidth(HWindow, dUnits, lpWidth); }

  void LogUnitsToRowHeight(SS_COORD Row, short dUnits, LPDOUBLE lpHeight)
	{ SSLogUnitsToRowHeight(HWindow, Row, dUnits, lpHeight); }

  BOOL MoveRange(SS_COORD Col,SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
		 SS_COORD ColDest, SS_COORD RowDest)
	{ return SSMoveRange(HWindow, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL Print(LPCSTR lpszPrintJobName, LPSS_PRINTFORMAT lpPrintFormat,
	     FARPROC lpfnPrintProc, LONG lAppData, FARPROC lpfnAbortProc)
	{ return SSPrint(HWindow, lpszPrintJobName, lpPrintFormat,
		 lpfnPrintProc, lAppData, lpfnAbortProc); }

  BOOL ReCalc(void)
	 { return SSReCalc (HWindow); }

  BOOL ReplaceSel(LPCSTR lpText)
	{ return SSReplaceSel (HWindow, lpText); }

  short RowHeightToLogUnits(SS_COORD Row, double dfRowHeight)
	{ return RowHeightToLogUnits (Row, dfRowHeight); }

  GLOBALHANDLE SaveToBuffer(long far *lpBufferLen, BOOL fDataOnly)
	{ return SSSaveToBuffer (HWindow, lpBufferLen, fDataOnly); }
		    
  BOOL SaveToFile(LPCSTR lpFileName, BOOL fDataOnly)
	{ return SSSaveToFile (HWindow, lpFileName, fDataOnly); }

  long SelModeSendMessage(UINT uMsg, long lParam1, long lParam2, long lParam3)
       {  return ::SSSelModeSendMessage(HWindow, uMsg, lParam1, lParam2, lParam3); }

  BOOL SetActiveCell(SS_COORD Col, SS_COORD Row)
	{ return SSSetActiveCell (HWindow, Col, Row); }

  void SetAutoSizeVisible(SS_COORD VisibleCols, SS_COORD VisibleRows)
	 { SSSetAutoSizeVisible (HWindow, VisibleCols, VisibleRows); }

  BOOL SetBool(short nIndex, BOOL bNewVal)
	{ return SSSetBool (HWindow, nIndex, bNewVal); }

  BOOL SetBorder(SS_COORD Col, SS_COORD Row, WORD wBorderType,
		 WORD wBorderStyle, COLORREF Color)
	{ return SSSetBorder(HWindow, Col, Row, wBorderType, wBorderStyle, Color); }

  BOOL SetBorderRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
		      SS_COORD Row2, WORD wBorderType, WORD wBorderStyle,
		      COLORREF Color)
	{ return SSSetBorderRange(HWindow, Col, Row, Col2, Row2, wBorderType, wBorderStyle, Color); }

  WORD SetButtonDrawMode(WORD wMode)
	{ return SSSetButtonDrawMode(HWindow, wMode); }

  BOOL SetButtonText(SS_COORD Col, SS_COORD Row, LPCSTR lpText)
	{ return SSSetButtonText(HWindow, Col, Row, lpText); }

  WNDPROC SetCallBack(WNDPROC lpfnCallBack)
	{ return SSSetCallBack(HWindow, lpfnCallBack); }

  BOOL SetCellType(SS_COORD Col, SS_COORD Row, LPSS_CELLTYPE CellType)
	{ return SSSetCellType(HWindow, Col, Row, CellType); }

  BOOL SetCellTypeRange(SS_COORD Col,  SS_COORD Row, SS_COORD Col2,
			SS_COORD Row2, LPSS_CELLTYPE CellType)
	{ return SSSetCellTypeRange(HWindow, Col, Row, Col2, Row2, CellType); }

  BOOL SetColor(SS_COORD Col, SS_COORD Row, COLORREF Background,
		COLORREF Foreground)
	{ return SSSetColor(HWindow, Col, Row, Background, Foreground); }

  BOOL SetColorRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
		     COLORREF Background, COLORREF Foreground)
	{ return SSSetColorRange(HWindow, Col, Row, Col2, Row2, Background, Foreground); }

  BOOL SetColPageBreak(SS_COORD Col, BOOL fPageBreak)
       { return SSSetColPageBreak(HWindow, Col, fPageBreak); }

  BOOL SetColUserData(SS_COORD Col, long lUserData)
	{ return SSSetColUserData(HWindow, Col, lUserData); }

  BOOL SetColWidth(SS_COORD Col, double Width)
	{ return SSSetColWidth(HWindow, Col, Width); }

  BOOL SetColWidthInPixels(SS_COORD Col, int dWidth)
	{ return SSSetColWidthInPixels(HWindow, Col, dWidth); }

  BOOL SetCursor(WORD wType, HCURSOR hCursor)
	{ return SSSetCursor(HWindow, wType, hCursor); }

  BOOL SetData(SS_COORD Col, SS_COORD Row, LPCSTR lpData)
	{ return SSSetData (HWindow, Col, Row, lpData); }

  BOOL SetDataRange(SS_COORD Col, SS_COORD Row,
		    SS_COORD Col2,SS_COORD Row2, LPCSTR lpData)
	{ return SSSetDataRange (HWindow, Col, Row, Col2, Row2, lpData); }

  BOOL SetDefFloatFormat(LPFLOATFORMAT lpFloatFormat)
	{ return SSSetDefFloatFormat(HWindow, lpFloatFormat); }

  WORD SetEditEnterAction(WORD wAction)
	 { return SSSetEditEnterAction(HWindow, wAction); }

  BOOL SetEditMode(BOOL fEditModeOn)
	{ return SSSetEditMode(HWindow, fEditModeOn); }

  BOOL SetFont(SS_COORD Col, SS_COORD Row, HFONT hFont, BOOL fDeleteFont)
	{ return SSSetFont(HWindow, Col, Row, hFont, fDeleteFont); }

  BOOL SetFontRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2,
		    SS_COORD Row2, HFONT hFont, BOOL fDeleteFont)
	{ return SSSetFontRange(HWindow, Col, Row, Col2, Row2, hFont, fDeleteFont); }

  BOOL SetFormula(SS_COORD Col, SS_COORD Row, LPCSTR Formula,
		  BOOL BuildDependencies)
	{ return SSSetFormula (HWindow, Col, Row, Formula, BuildDependencies); }

  BOOL SetFormulaRange(SS_COORD Col, SS_COORD Row,
		       SS_COORD Col2,SS_COORD Row2,
		       LPCSTR Formula, BOOL BuildDependencies)
	{ return SSSetFormulaRange(HWindow, Col, Row, Col2, Row2, Formula,
				   BuildDependencies); }

  BOOL SetFloat(SS_COORD Col, SS_COORD Row, double dfValue)
	{ return SSSetFloat(HWindow, Col, Row, dfValue); }

  BOOL SetFloatRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, double dfValue)
	{ return SSSetFloatRange(HWindow, Col, Row, Col2, Row2, dfValue); }

  BOOL SetFreeze(SS_COORD ColsFrozen, SS_COORD RowsFrozen)
	 { return SSSetFreeze (HWindow, ColsFrozen, RowsFrozen); }

  void SetGrayAreaColor(COLORREF Background, COLORREF Foreground)
	{ SSSetGrayAreaColor(HWindow, Background, Foreground); }

  WORD SetGridType(WORD wGridType)
	{ return SSSetGridType(HWindow, wGridType); }

  COLORREF SetGridColor(COLORREF Color)
	{ return SSSetGridColor(HWindow, Color); }

  BOOL SetInteger(SS_COORD Col, SS_COORD Row, long lValue)
	{ return SSSetInteger(HWindow, Col, Row, lValue); }

  BOOL SetIntegerRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, long lValue)
	{ return SSSetIntegerRange(HWindow, Col, Row, Col2, Row2, lValue); }

  BOOL SetLock(SS_COORD Col, SS_COORD Row, BOOL Lock)
	{ return SSSetLock (HWindow, Col, Row, Lock); }

  void SetLockColor(COLORREF Background, COLORREF Foreground)
	 { SSSetLockColor(HWindow, Background, Foreground); }

  BOOL SetLockRange(SS_COORD Col,  SS_COORD Row,
		    SS_COORD Col2, SS_COORD Row2, BOOL Lock)
	{ return SSSetLockRange (HWindow, Col, Row, Col2, Row2, Lock); }

  void SetMaxCols(SS_COORD MaxCols)
	{ SSSetMaxCols(HWindow, MaxCols); }

  void SetMaxRows(SS_COORD MaxRows)
	{ SSSetMaxRows(HWindow, MaxRows); }

  BOOL SetMultiSelBlocks(LPSS_SELBLOCK lpSelBlockList, short dSelBlockCnt)
	{ return SSSetMultiSelBlocks(HWindow, lpSelBlockList, dSelBlockCnt); }

  WORD SetOperationMode(WORD wMode)
	{ return SSSetOperationMode(HWindow, wMode); }
  
  HWND SetOwner(HWND hWndOwner)
	{ return SSSetOwner(HWindow, hWndOwner); }

  BOOL SetRowHeight(SS_COORD Row, double Height)
	{ return SSSetRowHeight (HWindow, Row, Height); }

  BOOL SetRowHeightInPixels(SS_COORD Row, int dHeight)
	{ return SSSetRowHeightInPixels(HWindow, Row, dHeight); }

  BOOL SetRowPageBreak(SS_COORD Row, BOOL fPageBreak)
       { return SSSetRowPageBreak(HWindow, Row, fPageBreak); }

  BOOL SetRowUserData(SS_COORD Row, long lUserData)
	{ return SSSetRowUserData(HWindow, Row, lUserData); }

  BOOL SetSel(short SelStart, short SelEnd)
	{ return SSSetSel(HWindow, SelStart, SelEnd); }

  BOOL SetSelectBlock(LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
	{ return SSSetSelectBlock (HWindow, CellUL, CellLR); }

  void SetShadowColor(COLORREF ShadowColor, COLORREF ShadowText,
		      COLORREF ShadowDark,  COLORREF ShadowLight)
	{ SSSetShadowColor(HWindow, ShadowColor, ShadowText,
			   ShadowDark, ShadowLight); }

  LPSS_CELLTYPE SetTypeButton(LPSS_CELLTYPE CellType, long Style,
			      LPCSTR Text, LPCSTR Picture,
			      short nPictureType, LPCSTR PictureDown,
			      short nPictureDownType, short ButtonType,
			      short nShadowSize, LPSUPERBTNCOLOR lpColor)
	{ return SSSetTypeButton(HWindow, CellType, Style, 
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
	{ return SSSetTypeCheckBox(HWindow, lpCellType, lStyle, lpText, lpPictUp, wPictUpType,
				   lpPictDown, wPictDownType, lpPictFocusUp, wPictFocusUpType,
				   lpPictFocusDown, wPictFocusDownType, lpPictDisabledUp, wPictDisabledUpType,
				   lpPictDisabledDown, wPictDisabledDownType) ; }

  LPSS_CELLTYPE SetTypeComboBox(LPSS_CELLTYPE CellType, long Style,
				LPCSTR lpItems)
	{ return SSSetTypeComboBox(HWindow, CellType, Style, lpItems); }

  LPSS_CELLTYPE SetTypeDate(LPSS_CELLTYPE CellType, long Style,
				 LPDATEFORMAT lpFormat, LPTB_DATE lpMin,
				 LPTB_DATE lpMax)
	{ return SSSetTypeDate(HWindow, CellType, Style, lpFormat, lpMin, lpMax); }

  LPSS_CELLTYPE SetTypeEdit(LPSS_CELLTYPE CellType, long Style, short Len,
			    short ChrSet, char ChrCase)
	{ return SSSetTypeEdit(HWindow, CellType, Style, Len, ChrSet, ChrCase); }


  LPSS_CELLTYPE SetTypeFloat(LPSS_CELLTYPE CellType, long Style,
			     short Left, short Right, double Min,
			     double Max)
	{ return SSSetTypeFloat(HWindow, CellType, Style, Left, Right, Min, Max); }

  LPSS_CELLTYPE SetTypeFloatExt(LPSS_CELLTYPE CellType, long Style,
			     short Left, short Right, double Min,
			     double Max, LPFLOATFORMAT lpFormat)
	{ return SSSetTypeFloatExt(HWindow, CellType, Style, Left, Right, Min, Max, lpFormat); }

  LPSS_CELLTYPE SetTypeInteger(LPSS_CELLTYPE CellType, long lMin, long lMax)
	{ return SSSetTypeInteger(HWindow, CellType, lMin, lMax); }

  LPSS_CELLTYPE SetTypeIntegerExt(LPSS_CELLTYPE CellType, long lStyle, long lMin,
				  long lMax, BOOL fSpinWrap, long lSpinInc)
	{ return SSSetTypeIntegerExt(HWindow, CellType, lStyle, lMin, lMax, fSpinWrap, lSpinInc); }

  LPSS_CELLTYPE SetTypeOwnerDraw(LPSS_CELLTYPE lpCellType, long lStyle)
	{ return SSSetTypeOwnerDraw(HWindow, lpCellType, lStyle); }

  LPSS_CELLTYPE SetTypePic(LPSS_CELLTYPE CellType, long Style, LPCSTR Mask)
	{ return SSSetTypePic(HWindow, CellType, Style, Mask); }

  LPSS_CELLTYPE SetTypePicture(LPSS_CELLTYPE CellType, long Style,
                               LPCSTR PictName)
	{ return SSSetTypePicture(HWindow, CellType, Style, PictName); }

  LPSS_CELLTYPE SetTypePictureHandle(LPSS_CELLTYPE CellType, long Style, HANDLE hPict,
				     HPALETTE hPal, BOOL fDeleteHandle)
	{ return SSSetTypePictureHandle(HWindow, CellType, Style, hPict, hPal, fDeleteHandle); }

  LPSS_CELLTYPE SetTypeStaticText(LPSS_CELLTYPE CellType, short TextStyle)
	{ return SSSetTypeStaticText(HWindow, CellType, TextStyle); }

  LPSS_CELLTYPE SetTypeTime(LPSS_CELLTYPE CellType, long Style,
			    LPTIMEFORMAT lpFormat, LPTIME lpMin,
			    LPTIME lpMax)
	{ return SSSetTypeTime (HWindow, CellType, Style, lpFormat, lpMin, lpMax); }

  WORD SetUnitType(WORD wUnitType)
	{ return SSSetUnitType(HWindow, wUnitType); }

  LONG SetUserData(LONG lUserData)
	{ return SSSetUserData(HWindow, lUserData); }

  short SetUserResizeCol(SS_COORD Col, short dOption)
	{ return SSSetUserResizeCol(HWindow, Col, dOption); }

  short SetUserResizeRow(SS_COORD Row, short dOption)
	{ return SSSetUserResizeRow(HWindow, Row, dOption); }

  WORD SetUserResize(WORD wUserResize)
	{ return SSSetUserResize(HWindow, wUserResize); }

  WORD SetSelBlockOptions(WORD wOption)
	{ return SSSetSelBlockOptions(HWindow, wOption); }

  BOOL SetValue(SS_COORD Col, SS_COORD Row, LPCSTR lpData)
	 { return SSSetValue (HWindow, Col, Row, lpData); }

  BOOL SetValueRange(SS_COORD Col,  SS_COORD Row,
		     SS_COORD Col2, SS_COORD Row2, LPCSTR lpData)
	 { return SSSetValueRange (HWindow, Col, Row, Col2, Row2, lpData); }

  BOOL ShowActiveCell(short Position)
	{ return SSShowActiveCell (HWindow, Position); }

  BOOL ShowCell(SS_COORD Col, SS_COORD Row, short Position)
	{ return SSShowCell (HWindow, Col, Row, Position); }

  BOOL ShowCol(SS_COORD Col, BOOL fShow)
	{ return SSShowCol(HWindow, Col, fShow); }

  BOOL ShowRow(SS_COORD Row, BOOL fShow)
	{ return SSShowRow(HWindow, Row, fShow); }

  BOOL Sort(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPSS_SORT lpSort)
	{ return SSSort(HWindow, Col, Row, Col2, Row2, lpSort); }

  BOOL SwapRange(SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2,
		 SS_COORD ColDest, SS_COORD RowDest)
	{ return SSSwapRange(HWindow, Col, Row, Col2, Row2, ColDest, RowDest); }

  BOOL ValidateFormula (LPCSTR lpszFormula)
	{ return SSValidateFormula (HWindow, lpszFormula); }

  void VGetBufferSize(LPSS_COORD lpdBufferSize, LPSS_COORD lpdOverlap)
	{ SSVGetBufferSize(HWindow, lpdBufferSize, lpdOverlap); }

  SS_COORD VGetMax()
	{ return SSVGetMax(HWindow); }

  SS_COORD VGetPhysBufferSize(void)
	{ return SSVGetPhysBufferSize(HWindow); }

  SS_COORD VGetPhysBufferTop(void)
	{ return SSVGetPhysBufferTop(HWindow); }

  LONG VGetStyle(void)
	{ return SSVGetStyle(HWindow); }

  BOOL VScrollGetSpecial(LPWORD lpwOptions)
	{ return SSVScrollGetSpecial(HWindow, lpwOptions); }

  BOOL VScrollSetSpecial(BOOL fUseSpecialVScroll, WORD wOptions)
	{ return SSVScrollSetSpecial(HWindow, fUseSpecialVScroll, wOptions); }

  void VSetBufferSize(SS_COORD dBufferSize, SS_COORD dOverlap)
	{ SSVSetBufferSize(HWindow, dBufferSize, dOverlap); }

  SS_COORD VSetMax(SS_COORD Max)
	{ return SSVSetMax(HWindow, Max); }

  LONG VSetStyle(LONG lStyle)
	{ return SSVSetStyle(HWindow, lStyle); }


//protected:
public:

  virtual LPSTR GetClassName()
	{ return "TBSpread"; }

  void EvHScroll(UINT scrollCode, UINT thumbPos, HWND hWndCtl);

  void EvVScroll(UINT scrollCode, UINT thumbPos, HWND hWndCtl);

  DECLARE_RESPONSE_TABLE(TSpread);

}; // end class TSpread

#endif
