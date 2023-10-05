// ssocx.h : main header file for SSOCX.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

//- Wei Feng add for working arround a bug in MFC
#ifdef	SPREAD_JPN
#ifndef	WIN32
class CParkingWnd : public CWnd
{
public:
	CParkingWnd()
		{ CreateEx(0, _T("AfxWnd"), NULL, WS_OVERLAPPED, 0, 0, 1, 1, NULL, 0); }

	~CParkingWnd()
	{ /*OutputDebugString("Destruct of Parking\n");*/}
};
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CSsocxApp : See ssocx.cpp for implementation.

class CSsocxApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;
/////////////////////////////////////////////////////////////////////////////
// _DSpread wrapper class
#if 0
class _DSpread : public COleDispatchDriver
{
// Attributes
public:
	short GetAction();
	void SetAction(short);
	long GetActiveCol();
	void SetActiveCol(long);
	long GetActiveRow();
	void SetActiveRow(long);
	BOOL GetAllowCellOverflow();
	void SetAllowCellOverflow(BOOL);
	BOOL GetAllowDragDrop();
	void SetAllowDragDrop(BOOL);
	BOOL GetAllowMultiBlocks();
	void SetAllowMultiBlocks(BOOL);
	BOOL GetAllowUserFormulas();
	void SetAllowUserFormulas(BOOL);
	BOOL GetArrowsExitEditMode();
	void SetArrowsExitEditMode(BOOL);
	BOOL GetAutoCalc();
	void SetAutoCalc(BOOL);
	BOOL GetAutoClipboard();
	void SetAutoClipboard(BOOL);
	BOOL GetAutoSize();
	void SetAutoSize(BOOL);
	BOOL GetBlockMode();
	void SetBlockMode(BOOL);
	short GetButtonDrawMode();
	void SetButtonDrawMode(short);
	BOOL GetCalcDependencies();
	void SetCalcDependencies(BOOL);
	// property 'CellBorderColor' not emitted because of invalid type
	short GetCellBorderStyle();
	void SetCellBorderStyle(short);
	short GetCellBorderType();
	void SetCellBorderType(short);
	short GetCellType();
	void SetCellType(short);
	BOOL GetChangeMade();
	void SetChangeMade(BOOL);
	CString GetClip();
	void SetClip(LPCTSTR);
	CString GetClipValue();
	void SetClipValue(LPCTSTR);
	long GetCol();
	void SetCol(long);
	long GetCol2();
	void SetCol2(long);
	short GetColHeaderDisplay();
	void SetColHeaderDisplay(short);
	BOOL GetColHidden();
	void SetColHidden(BOOL);
	BOOL GetColPageBreak();
	void SetColPageBreak(BOOL);
	long GetColsFrozen();
	void SetColsFrozen(long);
	short GetCursorStyle();
	void SetCursorStyle(short);
	short GetCursorType();
	void SetCursorType(short);
	long GetDataColCnt();
	void SetDataColCnt(long);
	CString GetDataField();
	void SetDataField(LPCTSTR);
	BOOL GetDataFillEvent();
	void SetDataFillEvent(BOOL);
	long GetDataRowCnt();
	void SetDataRowCnt(long);
	BOOL GetDAutoCellTypes();
	void SetDAutoCellTypes(BOOL);
	BOOL GetDAutoFill();
	void SetDAutoFill(BOOL);
	BOOL GetDAutoHeadings();
	void SetDAutoHeadings(BOOL);
	BOOL GetDAutoSave();
	void SetDAutoSave(BOOL);
	BOOL GetDAutoSizeCols();
	void SetDAutoSizeCols(BOOL);
	long GetDestCol();
	void SetDestCol(long);
	long GetDestRow();
	void SetDestRow(long);
	BOOL GetDInformActiveRowChange();
	void SetDInformActiveRowChange(BOOL);
	BOOL GetDisplayColHeaders();
	void SetDisplayColHeaders(BOOL);
	BOOL GetDisplayRowHeaders();
	void SetDisplayRowHeaders(BOOL);
	short GetEditEnterAction();
	void SetEditEnterAction(short);
	BOOL GetEditMode();
	void SetEditMode(BOOL);
	BOOL GetEditModePermanent();
	void SetEditModePermanent(BOOL);
	BOOL GetEditModeReplace();
	void SetEditModeReplace(BOOL);
	short GetFileNum();
	void SetFileNum(short);
	short GetFloatDefCurrencyChar();
	void SetFloatDefCurrencyChar(short);
	short GetFloatDefDecimalChar();
	void SetFloatDefDecimalChar(short);
	short GetFloatDefSepChar();
	void SetFloatDefSepChar(short);
	CString GetFormula();
	void SetFormula(LPCTSTR);
	// property 'GrayAreaBackColor' not emitted because of invalid type
	// property 'GridColor' not emitted because of invalid type
	BOOL GetGridShowHoriz();
	void SetGridShowHoriz(BOOL);
	BOOL GetGridShowVert();
	void SetGridShowVert(BOOL);
	BOOL GetGridSolid();
	void SetGridSolid(BOOL);
	short GetHDCPrinter();
	void SetHDCPrinter(short);
	short GetInterfaceDesigner();
	void SetInterfaceDesigner(short);
	BOOL GetIsBlockSelected();
	void SetIsBlockSelected(BOOL);
	long GetLeftCol();
	void SetLeftCol(long);
	BOOL GetLock();
	void SetLock(BOOL);
	// property 'LockBackColor' not emitted because of invalid type
	// property 'LockForeColor' not emitted because of invalid type
	long GetMaxCols();
	void SetMaxCols(long);
	long GetMaxRows();
	void SetMaxRows(long);
	double GetMaxTextCellHeight();
	void SetMaxTextCellHeight(double);
	double GetMaxTextCellWidth();
	void SetMaxTextCellWidth(double);
	BOOL GetMoveActiveOnFocus();
	void SetMoveActiveOnFocus(BOOL);
	long GetMultiSelCount();
	void SetMultiSelCount(long);
	long GetMultiSelIndex();
	void SetMultiSelIndex(long);
	BOOL GetNoBeep();
	void SetNoBeep(BOOL);
	BOOL GetNoBorder();
	void SetNoBorder(BOOL);
	short GetOperationMode();
	void SetOperationMode(short);
	short GetPosition();
	void SetPosition(short);
	CString GetPrintAbortMsg();
	void SetPrintAbortMsg(LPCTSTR);
	BOOL GetPrintBorder();
	void SetPrintBorder(BOOL);
	BOOL GetPrintColHeaders();
	void SetPrintColHeaders(BOOL);
	BOOL GetPrintColor();
	void SetPrintColor(BOOL);
	CString GetPrintFooter();
	void SetPrintFooter(LPCTSTR);
	BOOL GetPrintGrid();
	void SetPrintGrid(BOOL);
	CString GetPrintHeader();
	void SetPrintHeader(LPCTSTR);
	CString GetPrintJobName();
	void SetPrintJobName(LPCTSTR);
	long GetPrintMarginBottom();
	void SetPrintMarginBottom(long);
	long GetPrintMarginLeft();
	void SetPrintMarginLeft(long);
	long GetPrintMarginRight();
	void SetPrintMarginRight(long);
	long GetPrintMarginTop();
	void SetPrintMarginTop(long);
	short GetPrintPageEnd();
	void SetPrintPageEnd(short);
	short GetPrintPageStart();
	void SetPrintPageStart(short);
	BOOL GetPrintRowHeaders();
	void SetPrintRowHeaders(BOOL);
	BOOL GetPrintShadows();
	void SetPrintShadows(BOOL);
	short GetPrintType();
	void SetPrintType(short);
	BOOL GetPrintUseDataMax();
	void SetPrintUseDataMax(BOOL);
	BOOL GetProcessTab();
	void SetProcessTab(BOOL);
	BOOL GetProtect();
	void SetProtect(BOOL);
	BOOL GetReDraw();
	void SetReDraw(BOOL);
	BOOL GetRestrictCols();
	void SetRestrictCols(BOOL);
	BOOL GetRestrictRows();
	void SetRestrictRows(BOOL);
	BOOL GetRetainSelBlock();
	void SetRetainSelBlock(BOOL);
	long GetRow();
	void SetRow(long);
	long GetRow2();
	void SetRow2(long);
	short GetRowHeaderDisplay();
	void SetRowHeaderDisplay(short);
	BOOL GetRowHidden();
	void SetRowHidden(BOOL);
	BOOL GetRowPageBreak();
	void SetRowPageBreak(BOOL);
	long GetRowsFrozen();
	void SetRowsFrozen(long);
	BOOL GetScrollBarExtMode();
	void SetScrollBarExtMode(BOOL);
	BOOL GetScrollBarMaxAlign();
	void SetScrollBarMaxAlign(BOOL);
	short GetScrollBars();
	void SetScrollBars(short);
	BOOL GetScrollBarShowMax();
	void SetScrollBarShowMax(BOOL);
	long GetSelBlockCol();
	void SetSelBlockCol(long);
	long GetSelBlockCol2();
	void SetSelBlockCol2(long);
	long GetSelBlockRow();
	void SetSelBlockRow(long);
	long GetSelBlockRow2();
	void SetSelBlockRow2(long);
	short GetSelectBlockOptions();
	void SetSelectBlockOptions(short);
	long GetSelLength();
	void SetSelLength(long);
	long GetSelModeIndex();
	void SetSelModeIndex(long);
	long GetSelModeSelCount();
	void SetSelModeSelCount(long);
	BOOL GetSelModeSelected();
	void SetSelModeSelected(BOOL);
	long GetSelStart();
	void SetSelStart(long);
	CString GetSelText();
	void SetSelText(LPCTSTR);
	// property 'ShadowColor' not emitted because of invalid type
	// property 'ShadowDark' not emitted because of invalid type
	// property 'ShadowText' not emitted because of invalid type
	short GetSortBy();
	void SetSortBy(short);
	long GetStartingColNumber();
	void SetStartingColNumber(long);
	long GetStartingRowNumber();
	void SetStartingRowNumber(long);
	long GetTopRow();
	void SetTopRow(long);
	short GetTypeButtonAlign();
	void SetTypeButtonAlign(short);
	// property 'TypeButtonBorderColor' not emitted because of invalid type
	// property 'TypeButtonColor' not emitted because of invalid type
	// property 'TypeButtonDarkColor' not emitted because of invalid type
	// property 'TypeButtonLightColor' not emitted because of invalid type
	// property 'TypeButtonPicture' not emitted because of invalid type
	// property 'TypeButtonPictureDown' not emitted because of invalid type
	short GetTypeButtonShadowSize();
	void SetTypeButtonShadowSize(short);
	CString GetTypeButtonText();
	void SetTypeButtonText(LPCTSTR);
	// property 'TypeButtonTextColor' not emitted because of invalid type
	short GetTypeButtonType();
	void SetTypeButtonType(short);
	BOOL GetTypeCheckCenter();
	void SetTypeCheckCenter(BOOL);
	CString GetTypeCheckText();
	void SetTypeCheckText(LPCTSTR);
	short GetTypeCheckTextAlign();
	void SetTypeCheckTextAlign(short);
	short GetTypeComboBoxCount();
	void SetTypeComboBoxCount(short);
	short GetTypeComboBoxCurSel();
	void SetTypeComboBoxCurSel(short);
	BOOL GetTypeComboBoxEditable();
	void SetTypeComboBoxEditable(BOOL);
	short GetTypeComboBoxIndex();
	void SetTypeComboBoxIndex(short);
	CString GetTypeComboBoxList();
	void SetTypeComboBoxList(LPCTSTR);
	CString GetTypeComboBoxString();
	void SetTypeComboBoxString(LPCTSTR);
	BOOL GetTypeDateCentury();
	void SetTypeDateCentury(BOOL);
	short GetTypeDateFormat();
	void SetTypeDateFormat(short);
	CString GetTypeDateMax();
	void SetTypeDateMax(LPCTSTR);
	CString GetTypeDateMin();
	void SetTypeDateMin(LPCTSTR);
	short GetTypeDateSeparator();
	void SetTypeDateSeparator(short);
	short GetTypeEditCharCase();
	void SetTypeEditCharCase(short);
	short GetTypeEditCharSet();
	void SetTypeEditCharSet(short);
	long GetTypeEditLen();
	void SetTypeEditLen(long);
	BOOL GetTypeEditMultiLine();
	void SetTypeEditMultiLine(BOOL);
	BOOL GetTypeEditPassword();
	void SetTypeEditPassword(BOOL);
	short GetTypeFloatCurrencyChar();
	void SetTypeFloatCurrencyChar(short);
	short GetTypeFloatDecimalChar();
	void SetTypeFloatDecimalChar(short);
	short GetTypeFloatDecimalPlaces();
	void SetTypeFloatDecimalPlaces(short);
	double GetTypeFloatMax();
	void SetTypeFloatMax(double);
	double GetTypeFloatMin();
	void SetTypeFloatMin(double);
	BOOL GetTypeFloatMoney();
	void SetTypeFloatMoney(BOOL);
	BOOL GetTypeFloatSeparator();
	void SetTypeFloatSeparator(BOOL);
	short GetTypeFloatSepChar();
	void SetTypeFloatSepChar(short);
	short GetTypeHAlign();
	void SetTypeHAlign(short);
	long GetTypeIntegerMax();
	void SetTypeIntegerMax(long);
	long GetTypeIntegerMin();
	void SetTypeIntegerMin(long);
	long GetTypeIntegerSpinInc();
	void SetTypeIntegerSpinInc(long);
	BOOL GetTypeIntegerSpinWrap();
	void SetTypeIntegerSpinWrap(BOOL);
	long GetTypeOwnerDrawStyle();
	void SetTypeOwnerDrawStyle(long);
	CString GetTypePicDefaultText();
	void SetTypePicDefaultText(LPCTSTR);
	CString GetTypePicMask();
	void SetTypePicMask(LPCTSTR);
	BOOL GetTypePictCenter();
	void SetTypePictCenter(BOOL);
	BOOL GetTypePictMaintainScale();
	void SetTypePictMaintainScale(BOOL);
	// property 'TypePictPicture' not emitted because of invalid type
	BOOL GetTypePictStretch();
	void SetTypePictStretch(BOOL);
	BOOL GetTypeSpin();
	void SetTypeSpin(BOOL);
	short GetTypeTextAlignVert();
	void SetTypeTextAlignVert(short);
	BOOL GetTypeTextPrefix();
	void SetTypeTextPrefix(BOOL);
	BOOL GetTypeTextShadow();
	void SetTypeTextShadow(BOOL);
	BOOL GetTypeTextShadowIn();
	void SetTypeTextShadowIn(BOOL);
	BOOL GetTypeTextWordWrap();
	void SetTypeTextWordWrap(BOOL);
	BOOL GetTypeTime24Hour();
	void SetTypeTime24Hour(BOOL);
	CString GetTypeTimeMax();
	void SetTypeTimeMax(LPCTSTR);
	CString GetTypeTimeMin();
	void SetTypeTimeMin(LPCTSTR);
	BOOL GetTypeTimeSeconds();
	void SetTypeTimeSeconds(BOOL);
	short GetTypeTimeSeparator();
	void SetTypeTimeSeparator(short);
	short GetUnitType();
	void SetUnitType(short);
	short GetUserResize();
	void SetUserResize(short);
	short GetUserResizeCol();
	void SetUserResizeCol(short);
	short GetUserResizeRow();
	void SetUserResizeRow(short);
	CString GetValue();
	void SetValue(LPCTSTR);
	long GetVirtualCurRowCount();
	void SetVirtualCurRowCount(long);
	long GetVirtualCurTop();
	void SetVirtualCurTop(long);
	long GetVirtualMaxRows();
	void SetVirtualMaxRows(long);
	BOOL GetVirtualMode();
	void SetVirtualMode(BOOL);
	long GetVirtualOverlap();
	void SetVirtualOverlap(long);
	long GetVirtualRows();
	void SetVirtualRows(long);
	BOOL GetVirtualScrollBuffer();
	void SetVirtualScrollBuffer(BOOL);
	long GetVisibleCols();
	void SetVisibleCols(long);
	long GetVisibleRows();
	void SetVisibleRows(long);
	BOOL GetVScrollSpecial();
	void SetVScrollSpecial(BOOL);
	short GetVScrollSpecialType();
	void SetVScrollSpecialType(short);
	// property 'BackColor' not emitted because of invalid type
	short GetBorderStyle();
	void SetBorderStyle(short);
	BOOL GetEnabled();
	void SetEnabled(BOOL);
	// property 'Font' not emitted because of invalid type
	// property 'ForeColor' not emitted because of invalid type
	short GetHWnd();
	void SetHWnd(short);
	CString GetText();
	void SetText(LPCTSTR);

// Operations
public:
	double GetColWidth(long lCol);
	void SetColWidth(long lCol, double newValue);
	double GetMaxTextColWidth(long lCol);
	void SetMaxTextColWidth(long lCol, double newValue);
	double GetMaxTextRowHeight(long lRow);
	void SetMaxTextRowHeight(long lRow, double newValue);
	double GetRowHeight(long lRow);
	void SetRowHeight(long lRow, double newValue);
	long GetSortKey(short nIndex);
	void SetSortKey(short nIndex, long nNewValue);
	short GetSortKeyOrder(short nIndex);
	void SetSortKeyOrder(short nIndex, short nNewValue);
	// method 'GetTypeCheckPicture' not emitted because of invalid return type
	// method 'SetTypeCheckPicture' not emitted because of invalid return type
	BOOL AddCustomFunction(LPCTSTR FunctionName, short ParameterCnt);
	double CFGetDoubleParam(short Param);
	long CFGetLongParam(short Param);
	BOOL CFGetParamInfo(short Param, short* Type, short* Status);
	CString CFGetStringParam(short Param);
	void CFSetResult(VARIANT* Var);
	CString ColNumberToLetter(long HeaderNumber);
	void ColWidthToTwips(float Width, long* Twips);
	void GetBottomRightCell(long* Col, long* Row);
	BOOL GetCellDirtyFlag(long Col, long Row);
	void GetCellFromScreenCoord(long* Col, long* Row, long x, long y);
	BOOL GetCellPos(long Col, long Row, long* x, long* y, long* Width, long* Height);
	void GetClientArea(long* Width, long* Height);
	long GetColItemData(long Col);
	BOOL GetDataFillData(VARIANT* Var, short VarType);
	void GetFirstValidCell(long* Col, long* Row);
	long GetItemData();
	void GetLastValidCell(long* Col, long* Row);
	long GetMultiSelItem(long SelPrev);
	long GetRowItemData(long Row);
	BOOL GetText(long Col, long Row, VARIANT* Var);
	BOOL IsCellSelected(long Col, long Row);
	BOOL IsFormulaValid(LPCTSTR Formula);
	BOOL IsVisible(long Col, long Row, BOOL Partial);
	BOOL LoadFromFile(LPCTSTR FileName);
	BOOL LoadTabFile(LPCTSTR FileName);
	void RowHeightToTwips(long Row, float Height, long* Twips);
	BOOL SaveTabFile(LPCTSTR FileName);
	BOOL SaveToFile(LPCTSTR FileName, BOOL DataOnly);
	void SetColItemData(long Col, long Value);
	BOOL SetDataFillData(VARIANT* Var);
	BOOL SetCellDirtyFlag(long Col, long Row, BOOL Dirty);
	void SetItemData(long Value);
	void SetRowItemData(long Row, long Value);
	void SetText(long Col, long Row, VARIANT* Var);
	void TwipsToColColWidth(long Twips, float* ColWidth);
	void TwipsToRowHeight(long Row, long Twips, float* RowHeight);
	void Refresh();
	void AboutBox();
#ifdef FP_OLEDRAG
	void OLEDrag(void);
#endif
#ifdef SS_V35
	short GetColUserSortIndicator(long lCol);
	void SetColUserSortIndicator(long lCol, short newValue);
	short GetUserColAction();
	void SetUserColAction(short newValue);
	BOOL GetScriptEnhanced();
	void SetScriptEnhanced(BOOL newValue);
#endif
#ifdef SS_V40
	short GetColMerge();
	void SetColMerge(short newValue);
	short GetRowMerge();
	void SetRowMerge(short newValue);
	CString GetColID();
	void SetColID(LPCTSTR);
	long GetRowHeaderCols();
	void SetRowHeaderCols(long newValue);
	long GetRowHeadersAutoTextIndex();
	void SetRowHeadersAutoTextIndex(long newValue);
	long GetColHeaderRows();
	void SetColHeaderRows(long newValue);
	long GetColHeadersAutoTextIndex();
	void SetColHeadersAutoTextIndex(long newValue);
	long GetColHeadersUserSortIndex();
	void SetColHeadersUserSortIndex(long newValue);
   short GetTypeComboBoxAutoSearch(void);
   void SetTypeComboBoxAutoSearch(short lNewValue);
   double GetTypeSpinInc(void);
   void SetTypeSpinInc(double newValue);
   BOOL GetTypeSpinWrap(void);
   void SetTypeSpinWrap(BOOL newValue);
   BOOL GetTypeNegRed(void);
   void SetTypeNegRed(BOOL newValue);
   double GetTypeCurrencyMin(void);
   void SetTypeCurrencyMin(double newValue);
   double GetTypeCurrencyMax(void);
   void SetTypeCurrencyMax(double newValue);
   BSTR GetTypeCurrencyDecimal(void);
   void SetTypeCurrencyDecimal(LPCTSTR newValue);
   BSTR GetTypeCurrencySeparator(void);
   void SetTypeCurrencySeparator(LPCTSTR newValue);
   BSTR GetTypeCurrencySymbol(void);
   void SetTypeCurrencySymbol(LPCTSTR newValue);
   short GetTypeCurrencyLeadingZero(void);
   void SetTypeCurrencyLeadingZero(short newValue);
   short GetTypeCurrencyDecPlaces(void);
   void SetTypeCurrencyDecPlaces(short newValue);
   short GetTypeCurrencyNegStyle(void);
   void SetTypeCurrencyNegStyle(short newValue);
   short GetTypeCurrencyPosStyle(void);
   void SetTypeCurrencyPosStyle(short newValue);
   BOOL GetTypeCurrencyShowSep(void);
   void SetTypeCurrencyShowSep(BOOL newValue);
   BOOL GetTypeCurrencyShowSymbol(void);
   void SetTypeCurrencyShowSymbol(BOOL newValue);
   double GetTypeNumberMin(void);
   void SetTypeNumberMin(double newValue);
   double GetTypeNumberMax(void);
   void SetTypeNumberMax(double newValue);
   BSTR GetTypeNumberDecimal(void);
   void SetTypeNumberDecimal(LPCTSTR newValue);
   BSTR GetTypeNumberSeparator(void);
   void SetTypeNumberSeparator(LPCTSTR newValue);
   short GetTypeNumberLeadingZero(void);
   void SetTypeNumberLeadingZero(short newValue);
   short GetTypeNumberDecPlaces(void);
   void SetTypeNumberDecPlaces(short newValue);
   short GetTypeNumberNegStyle(void);
   void SetTypeNumberNegStyle(short newValue);
   BOOL GetTypeNumberShowSep(void);
   void SetTypeNumberShowSep(BOOL newValue);
   double GetTypePercentMin(void);
   void SetTypePercentMin(double newValue);
   double GetTypePercentMax(void);
   void SetTypePercentMax(double newValue);
   BSTR GetTypePercentDecimal(void);
   void SetTypePercentDecimal(LPCTSTR newValue);
   short GetTypePercentDecPlaces(void);
   void SetTypePercentDecPlaces(short newValue);
   short GetTypePercentNegStyle(void);
   void SetTypePercentNegStyle(short newValue);
   short GetTypePercentLeadingZero(void);
   void SetTypePercentLeadingZero(short newValue);
   short GetTypeTextOrient(void);
   void SetTypeTextOrient(short newValue);
	CString GetCellTag();
	void SetCellTag(LPCTSTR);
	BOOL GetEventEnabled(short Event);
	void SetEventEnabled(short Event, BOOL IsEnabled);
	BOOL GetAllowEditOverflow(void);
	void SetAllowEditOverflow(BOOL newValue);
#endif // SS_V40
};

/////////////////////////////////////////////////////////////////////////////
// _DSpreadEvents wrapper class

class _DSpreadEvents : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	void Advance(BOOL AdvanceNext);
	void BlockSelected(long BlockCol, long BlockRow, long BlockCol2, long BlockRow2);
	void ButtonClicked(long Col, long Row, long ButtonDown);
	void Change(long Col, long Row);
	void Click(long Col, long Row);
	void ColWidthChange(long Col1, long Col2);
	void CustomFunction(LPCTSTR FunctionName, short ParameterCnt, long Col, long Row, short* Status);
	void DataAddNew();
	void DataColConfig(long Col, LPCTSTR DataField, short DataType);
	void DataFill(long Col, long Row, short DataType, BOOL fGetData, short* Cancel);
	void DblClick(long Col, long Row);
	void DragDropBlock(long Col, long Row, long Col2, long Row2, long NewCol, long NewRow, long NewCol2, long NewRow2, BOOL Overwrite, short* Action, BOOL* DataOnly, BOOL* Cancel);
	void DrawItem(long Col, long Row, short hDC, long Left, long Top, long Right, long Bottom, long Style);
	void EditError(long Col, long Row, short EditError);
	void EditMode(long Col, long Row, short Mode, BOOL ChangeMade);
	void EnterRow(long Row, long RowIsLast);
	void LeaveCell(long Col, long Row, long NewCol, long NewRow, BOOL* Cancel);
	void LeaveRow(long Row, BOOL RowWasLast, BOOL RowChanged, BOOL AllCellsHaveData, long NewRow, long NewRowIsLast, BOOL* Cancel);
	void PrintAbort(BOOL* Abort);
	void QueryAdvance(BOOL AdvanceNext, BOOL* Cancel);
	void QueryData(long Row, long RowsNeeded, long* RowsLoaded, short Direction, BOOL* AtTop, BOOL* AtBottom);
	void RightClick(short ClickType, long Col, long Row, long MouseX, long MouseY);
	void RowHeightChange(long Row1, long Row2);
	void SelChange(long BlockCol, long BlockRow, long BlockCol2, long BlockRow2, long CurCol, long CurRow);
	void TopLeftChange(long OldLeft, long OldTop, long NewLeft, long NewTop);
	void UserFormulaEntered(long Col, long Row);
	void VirtualClearData(long Row, long RowsBeingCleared);
	void KeyDown(short* KeyCode, short Shift);
	void KeyPress(short* KeyAscii);
	void KeyUp(short* KeyCode, short Shift);
	void MouseDown(short Button, short Shift, long x, long y);
	void MouseMove(short Button, short Shift, long x, long y);
	void MouseUp(short Button, short Shift, long x, long y);
#ifdef FP_OLEDRAG
	void OLECompleteDrag(long* Effect);
	void OLEDragDrop(LPDATAOBJECT* DataObject, long* Effect, short* Button, short* Shift, float* X, float* Y);
	void OLEDragOver(LPDATAOBJECT* DataObject, long* Effect, short* Button, short* Shift, float* X, float* Y, short* State);
	void OLEGiveFeedback(long* Effect, BOOL* DefaultCursors);
	void OLESetData(LPDATAOBJECT* DataObject, short* DataFormat);
	void OLEStartDrag(LPDATAOBJECT* DataObject, long* AllowedEffects);
#endif
};

#endif // 0