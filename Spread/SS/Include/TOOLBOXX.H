//-----------------------------------------------------------------
//
//  toolboxx.h - Window message crackers for tbSpread
//
//-----------------------------------------------------------------

//
//  void On (HWND hWnd, UINT idCtl)
//
#define HANDLE_SSM_(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), ()(lParam)), 0L)
#define FORWARD_SSM_(hWnd, idCtl, fn) \
  (void)(fn)((hWnd), SSM_, (WPARAM)(idCtl), (LPARAM)())

//
//  void OnCellLocked (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
//
#define HANDLE_SSM_CELLLOCKED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_CELLLOCKED(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_CELLLOCKED, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//
//  BOOL OnClick (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
//
#define HANDLE_SSM_CLICK(hWnd, wParam, lParam, fn) \
  (LRESULT)(fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam))
#define FORWARD_SSM_CLICK(hWnd, idCtl, fn) \
  (BOOL)(fn)((hWnd), SSM_CLICK, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//
//  void OnColRestricted (HWND hWnd, UINT idCtl, SS_COORD Col)
//
#define HANDLE_SSM_COLRESTRICTED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (SS_COORD)(lParam)), 0L)
#define FORWARD_SSM_COLRESTRICTED(hWnd, idCtl, Col, fn) \
  (void)(fn)((hWnd), SSM_COLRESTRICTED, (WPARAM)(idCtl), (LPARAM)(Col))

//
//  void OnColRowRestricted (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
//
#define HANDLE_SSM_COLROWRESTRICTED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_COLROWRESTRICTED(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_COLROWRESTRICTED, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//
//  void OnColWidthChange (HWND hWnd, UINT idCtl, LPSS_COORDRANGE lpCoordRange)
//
#define HANDLE_SSM_COLWIDTHCHANGE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_COORDRANGE)(lParam)), 0L)
#define FORWARD_SSM_COLWIDTHCHANGE(hWnd, idCtl, lpCoordRange, fn) \
  (void)(fn)((hWnd), SSM_COLWIDTHCHANGE, (WPARAM)(idCtl), (LPARAM)(lpCoordRange))

//
//  void OnDataChange (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
//
#define HANDLE_SSM_DATACHANGE(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_DATACHANGE(hWnd, idCtl, lpCellCoord, fn) \
    (void)(fn)((hWnd), SSM_DATACHANGE, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//
//  BOOL OnDblClk (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
//
#define HANDLE_SSM_DBLCLK(hWnd, wParam, lParam, fn) \
  (LRESULT)(fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam))
#define FORWARD_SSM_DBLCLK(hWnd, idCtl, lpCellCoord, fn) \
  (BOOL)(fn)((hWnd), SSM_DBLCLK, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//
//  void OnDragDrop (HWND hWnd, UINT idCtl, LPSS_DRAGDROP)
//
#define HANDLE_SSM_DRAGDROP(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_DRAGDROP)(lParam)), 0L)
#define FORWARD_SSM_DRAGDROP(hWnd, idCtl, lpDragDrop, fn) \
  (void)(fn)((hWnd), SSM_DRAGDROP, (WPARAM)(idCtl), (LPARAM)(lpDragDrop))

//
//  void OnDrawItem (HWND hWnd, LPSS_DRAWITEMSTRUCT lpDrawItem)
//
#define HANDLE_SSM_DRAWITEM(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (LPSS_DRAWITEMSTRUCT)(lParam)), 0L)
#define FORWARD_SSM_DRAWITEM(hWnd, lpDrawItem, fn) \
  (void)(fn)((hWnd), SSM_DRAWITEM, (WPARAM)0, (LPARAM)(lpDrawItem))

//
//  void OnEditModeOff (HWND hWnd, UINT idCtl, LPSS_EDITMODEOFF lpEditModeOff)
//
#define HANDLE_SSM_EDITMODEOFF(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_EDITMODEOFF)(lParam)), 0L)
#define FORWARD_SSM_EDITMODEOFF(hWnd, idCtl, lpEditModeOff, fn) \
  (void)(fn)((hWnd), SSM_EDITMODEOFF, (WPARAM)(idCtl), (LPARAM)(lpEditModeOff))

//
//  void OnEditModeOn (HWND hWnd, UINT idCtl, LPSS_CELLCOORD lpCellCoord)
//
#define HANDLE_SSM_EDITMODEON(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_CELLCOORD)(lParam)), 0L)
#define FORWARD_SSM_EDITMODEON(hWnd, idCtl, lpCellCoord, fn) \
  (void)(fn)((hWnd), SSM_EDITMODEON, (WPARAM)(idCtl), (LPARAM)(lpCellCoord))

//
//  void OnEnterRow (HWND hWnd, UINT idCtl, LPSS_ENTERROW lpEnterRow)
//
#define HANDLE_SSM_ENTERROW(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_ENTERROW)(lParam)), 0L)
#define FORWARD_SSM_ENTERROW(hWnd, idCtl, lpEnterRow, fn) \
  (void)(fn)((hWnd), SSM_ENTERROW, (WPARAM)(idCtl), (LPARAM)(lpEnterRow))

//
//  BOOL OnKeyDown (HWND hWnd, UINT idCtl, UINT VirtualKey, UINT Flags)
//
#define HANDLE_SSM_KEYDOWN(hWnd, wParam, lParam, fn) \
  (fn)((hWnd), (UINT)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))
#define FORWARD_SSM_KEYDOWN(hWnd, idCtl, VirtualKey, Flags, fn) \
  (BOOL)(fn)((hWnd), SSM_KEYDOWN, (WPARAM)(idCtl), MAKELPARAM((VirtualKeys),(Flags)))

//
//  void OnKeyPress (HWND hWnd, UINT idCtl, UINT* lpKey)
//
#define HANDLE_SSM_KEYPRESS(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (UINT*)(lParam)), 0)
#define FORWARD_SSM_KEYPRESS(hWnd, idCtl, lpKey, fn) \
  (void)(fn)((hWnd), SM_KEYPRESS, (WPARAM)(idCtl), (LPARAM)(lpKey))

//
//  void OnKeyUp (HWND hWnd, UINT idCtl, UNIT VirtualKey, UINT Flags);
//
#define HANDLE_SSM_KEYUP(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)), 0)
#define FORWARD_SSM_KEYUP(hWnd, idCtl, VirtualKey, Flags, fn) \
  (void)(fn)((hWnd), SSM_KEYUP, (WPARAM)(idCtl), MAKELPARAM((VirtualKeys),(Flags)))

//
//  BOOL OnLeaveCell (HWND hWnd, UINT idCtl, LPSS_LEAVECELL lpLeaveCell)
//
#define HANDLE_SSM_LEAVECELL(hWnd, wParam, lParam, fn) \
  (LRESULT)(fn)((hWnd), (UINT)(wParam), (LPSS_LEAVECELL)(lParam))
#define FORWARD_SSM_LEAVECELL(hWnd, idCtl, lpLeaveCell, fn) \
  (BOOL)(fn)((hWnd), SSM_LEAVECELL, (WPARAM)(idCtl), (LPARAM)(lpLeaveCell))

//
//  void OnLeaveRow (HWND hWnd, UINT idCtl, LPSS_LEAVEROW lpLeaveRow)
//
#define HANDLE_SSM_LEAVEROW(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_LEAVEROW)(lParam)), 0L)
#define FORWARD_SSM_LEAVEROW(hWnd, idCtl, lpLeaveRow, fn) \
  (void)(fn)((hWnd), SSM_LEAVEROW, (WPARAM)(idCtl), (LPARAM)(lpLEaveRow))

//
//  void OnRButton (HWND hWnd, UINT idCtl, LPSS_RBUTTON lpRButton)
//
#define HANDLE_SSM_RBUTTON(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_RBUTTON)(lParam)), 0L)
#define FORWARD_SSM_RBUTTON(hWnd, idCtl, lpRButton, fn) \
  (void)(fn)((hWnd), SSM_RBUTTON, (WPARAM)(idCtl), (LPARAM)(lpRButton))

//
//  void OnRowHeightChange (HWND hWnd, UINT idCtl, LPSS_COORDRANGE lpCoordRange)
//
#define HANDLE_SSM_ROWHEIGHTCHANGE(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_COORDRANGE)(lParam)), 0L)
#define FORWARD_SSM_ROWHEIGHTCHANGE(hWnd, idCtl, lpCoordRange, fn) \
  (void)(fn)((hWnd), SSM_ROWHEIGHTCHANGE, (WPARAM)(idCtl), (LPARAM)(lpCoordRange))

//
//  void OnRowRestricted (HWND hWnd, UINT idCtl, SS_COORD Row)
//
#define HANDLE_SSM_ROWRESTRICTED(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (SS_COORD)(lParam)), 0L)
#define FORWARD_SSM_ROWRESTRICTED(hWnd, idCtl, Row, fn) \
  (void)(fn)((hWnd), SSM_ROWRESTRICTED, (WPARAM)(idCtl), (LPARAM)(Row))

//
//  BOOL OnVClearData (HWND hWnd, UNIT idCtl, LPSS_VCLEARDATA lpVClearData)
//
#define HANDLE_SSM_VCLEARDATA(hWnd, wParam, lParam, fn) \
  (fn)((hWnd), (UINT)(wParam), (LPSS_VCLEARDATA)(lParam))
#define FORWARD_SSM_VCLEARDATA(hWnd, idCtl, lpVQueryData, fn) \
  (BOOL)(fn)((hWnd), SSM_VCLEARDATA, (WPARAM)(idCtl), (LPARAM)(lpVClearData))
  
//
//  void OnVQueryData (HWND hWnd, UINT idCtl, LPSS_VQUERYDATA lpVQueryData)
//
#define HANDLE_SSM_VQUERYDATA(hWnd, wParam, lParam, fn) \
  ((fn)((hWnd), (UINT)(wParam), (LPSS_VQUERYDATA)(lParam)), 0L)
#define FORWARD_SSM_VQUERYDATA(hWnd, idCtl, lpVQueryData, fn) \
  (void)(fn)((hWnd), SSM_VQUERYDATA, (WPARAM)(idCtl), (LPARAM)(lpVQueryData))

//-------- Alternate porting layer macros --------------------------

#define GET_SSM_CELLLOCKED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_CELLLOCKED_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_CELLLOCKED_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_CLICK_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_CLICK_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_CLICK_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_COLRESTRICTED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COLRESTRICTED_COL(wParam, lParam) (SS_COORD)(lParam)
#define GET_SSM_COLRESTRICTED_MPS(idCtl, Col) \
        (WPARAM)(idCtl), (LPARAM)(Col)

#define GET_SSM_COLROWRESTRICTED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COLROWRESTRICTED_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_COLROWRESTRICTED_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_COLWIDTHCHANGE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_COLWIDTHCHANGE_LPCOORDRANGE(wParam, lParam) (LPSS_COORDRANGE)(lParam)
#define GET_SSM_COLWIDTHCHANGE_MPS(idCtl, lpCoordRange) \
        (WPARAM)(idCtl), (LPARAM)(lpCoordRange)

#define GET_SSM_DATACHANGE_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_DATACHANGE_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_DATACHANGE_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_DBLCLK_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_DBLCLK_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_DBLCLK_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_DRAGDROP_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_DRAGDROP_LPDRAGDROP(wParam, lParam) (LPSS_DRAGDROP)(lParam)
#define GET_SSM_DRAGDROP_MPS(idCtl, lpDragDrop) \
        (WPARAM)(idCtl), (LPARAM)(lpDragDrop)

#define GET_SSM_DRAWITEM_LPDRAWITEM(wParam, lParam) (LPSS_DRAWITEMSTRUCT)(lParam)
#define GET_SSM_DRAWITEM_MPS(lpDrawItem) \
        (WPARAM)(0), (LPARAM)(lpDrawItem)

#define GET_SSM_EDITMODEOFF_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_EDITMODEOFF_LPEDITMODEOFF(wParam, lParam) (LPSS_EDITMODEOFF)(lParam)
#define GET_SSM_EDITMODEOFF_MPS(idCtl, lpEditModeOff) \
        (WPARAM)(idCtl), (LPARAM)(lpEditModeOff)

#define GET_SSM_EDITMODEON_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_EDITMODEON_LPCELLCOORD(wParam, lParam) (LPSS_CELLCOORD)(lParam)
#define GET_SSM_EDITMODEON_MPS(idCtl, lpCellCoord) \
        (WPARAM)(idCtl), (LPARAM)(lpCellCoord)

#define GET_SSM_ENTERROW_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_ENTERROW_LPENTERROW(wParam, lParam) (LPSS_ENTERROW)(lParam)
#define GET_SSM_ENTERROW_MPS(idCtl, lpEnterRow) \
        (WPARAM)(idCtl), (LPARAM)(lpEnterRow)

#define GET_SSM_KEYDOWN_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_KEYDOWN_VIRTUALKEY(wParam, lParam) (UINT)LOWORD(lParam)
#define GET_SSM_KEYDOWN_FLAGS(wParam, lParam) (UINT)HIWORD(lParam)
#define GET_SSM_KEYDOWN_MPS(idCtl, VirtualKey, Flags) \
        (WPARAM)(idCtl), MAKELPARAM(VirtualKey, Flags)

#define GET_SSM_LEAVECELL_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_LEAVECELL_LPLEAVECELL(wParam, lParam) (LPSS_LEAVECELL)(lParam)
#define GET_SSM_LEAVECELL_MPS(idCtl, lpLeaveCell) \
        (WPARAM)(idCtl), (LPARAM)(lpLeaveCell)

#define GET_SSM_LEAVEROW_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_LEAVEROW_LPLEAVEROW(wParam, lParam) (LPSS_LEAVEROW)(lParam)
#define GET_SSM_LEAVEROW_MPS(idCtl, lpLeaveRow) \
        (WPARAM)(idCtl), (LPARAM)(lpLeaveRow)

#define GET_SSM_RBUTTON_IDCTL(wParam, lParam) (UNIT)(wParam)
#define GET_SSM_RBUTTON_LPRBUTTON(wParam, lParam) (LPSS_RBUTTON)(lParam)
#define GET_SSM_RBUTTON_MPS(idCtl, lpRButton) \
        (WPARAM)(idCtl), (LPARAM)(lpRButton)

#define GET_SSM_ROWHEIGHTCHANGE_IDCTL(wParam, lParam) (UINT)(idCtl)
#define GET_SSM_ROWHEIGHTCHANGE_LPCOORDRANGE(wParam, lParam) (LPSS_COORDRANGE)(lParam)
#define GET_SSM_ROWHEIGHTCHANGE_MPS(idCtl, lpCoordRange) \
        (WPARAM)(idCtl), (LPARAM)(lpCoordRange)

#define GET_SSM_ROWRESTRICTED_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_ROWRESTRICTED_ROW(wParam, lParam) (SS_COORD)(lParam)
#define GET_SSM_ROWRESTRICTED_MPS(idCtl, Row) \
        (WPARAM)(idCtl), (LPARAM)(Row)

#define GET_SSM_VCLEARDATA_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_VCLEARDATA_LPVCLEARDATA(wParam, lParam) (LPSS_VCLEARDATA)(lParam)
#define GET_SSM_VCLEARDATA_MPS(idCtl, lpVClearData) \
        (WPARAM)(idCtl), (LPARAM)(lpVClearData)

#define GET_SSM_VQUERYDATA_IDCTL(wParam, lParam) (UINT)(wParam)
#define GET_SSM_VQUERYDATA_LPVQUERYDATA(wParam, lParam) (LPSS_VQUERYDATA)(lParam)
#define GET_SSM_VQUERYDATA_MPS(idCtl, lpVQueryData) \
        (WPARAM)(idCtl), (LPARAM)(lpVQueryData)
