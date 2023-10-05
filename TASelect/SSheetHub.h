#pragma once


#include "HydroMod.h" 
#include "SSheet.h"
#include "DlgComboBoxHM.h"

// This is the same sheet for hub and loops. When 'CRViewSSelHub::Create' is called, 'CSSheetTAHub' class is
// created twice: one for the main part and other one for the loop part.
//
// When 'CRViewSSelHub::SetRedraw' is called, 'CSSheetTAHub::Init' is called with first parameter set to 'true' to specify
// that CSSheetTAHub has to be considered as the main part. And set to 'false' for the loop part.

class CDlgComboBoxHub; 
class CDlgHubCvCB;
class CSSheetTAHub : public CSSheet
{
public:
	struct _HeaderRow 
	{
		const static int GroupName = 1;     
		const static int ButDlg = GroupName + 1;
		const static int ColName = ButDlg + 1;
		const static int Unit = ColName + 1;
		const static int Picture = Unit + 1;
		const static int FirstCirc = Picture + 1;
	} m_HeaderRow;

	struct _Sheet
	{
/*1*/	const static int Sub = 1;
		const static int Pos = Sub + 1;
		const static int CheckBox = Pos + 1;
		const static int LabelID = CheckBox + 1;
/*5*/	const static int LoopDesc = LabelID + 1;
		const static int LoopQ = LoopDesc + 1;
		const static int LoopP = LoopQ + 1;
		const static int LoopDT = LoopP + 1;
		const static int LoopDp = LoopDT + 1;
/*10*/	const static int LoopSep = LoopDp + 1;

		const static int SupName = LoopSep + 1;
		const static int SupDp	 = SupName + 1;
		const static int SupSep	 = SupDp + 1;

		const static int BvType = SupSep + 1;						// Return side
		const static int BvCtrlType = BvType + 1;
		const static int BvName = BvCtrlType + 1;
		const static int BvPreSet = BvName + 1;
		const static int BvDp = BvPreSet + 1;
		const static int BvDpSignal = BvDp + 1;
/*20*/	const static int BvDpMin = BvDpSignal + 1;
		const static int BvSep = BvDpMin + 1;

		const static int ActuatorType = BvSep + 1;
		const static int ActuatorSep = ActuatorType + 1;

		const static int DpCName = ActuatorSep + 1;
		const static int DpCDpLr = DpCName + 1;
		const static int DpCSet = DpCDpLr + 1;
		const static int DpCDp = DpCSet + 1;
		const static int DpCDpl = DpCDp + 1;
		const static int DpCDpMin = DpCDpl + 1;
/*30*/	const static int DpCSep = DpCDpMin + 1;

		const static int UnitDp = DpCSep + 1;
		const static int UnitQref = UnitDp + 1;
		const static int UnitDpref = UnitQref + 1;
		const static int UnitSep = UnitDpref + 1;

		const static int CVType = UnitSep + 1;
		const static int CVName = CVType + 1;
		const static int CVDesc = CVName + 1;
		const static int CVKvsMax = CVDesc + 1;
		const static int CVKvs = CVKvsMax + 1;
/*40*/	const static int CVDp = CVKvs + 1;
		const static int CVAuth = CVDp + 1;
		const static int CVSep = CVAuth + 1;

		const static int PipeSerie = CVSep + 1;
		const static int PipeSize = PipeSerie + 1;
		const static int PipeL = PipeSize + 1;
		const static int PipeDp = PipeL + 1;
		const static int PipeLinDp = PipeDp + 1;
		const static int PipeV = PipeLinDp + 1;
		const static int PipeSep = PipeV + 1;

/*50*/	const static int AccDesc = PipeSep + 1;
		const static int AccDp = AccDesc + 1;
		const static int AccSep = AccDp + 1;
		const static int Pointer = AccSep + 1;
	} m_Sheet;

public:
	CSSheetTAHub( );
	virtual ~CSSheetTAHub();

	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );

	void Init( bool fHubMode, CDS_HmHub *pHub, bool fPrint = false );
	CDS_HydroMod *GetpHM( long lRow );
	void FillSheetHub();
	void FillSheetHubStations();
	CDS_HmHub * GetpHub() { return m_pHub; }
	void CellClicked( long lColumn, long lRow, CSSheetTAHub *pclOtherTAHub, bool *pfMustResize );
	void ComboDropDown( long lColumn, long lRow );
	void CellDblClick( long lColumn, long lRow );
	void ButtonClickedFpspreadhubStation( long lColumn, long lRow, short nButtonDown );
	void DragDropBlock( long lOldColumnFrom, long lOldRowFrom, long lOldColumnTo, long lOldRowTo, long lNewColumnFrom, long lNewRowFrom, long lNewColumnTo, long lNewRowTo, BOOL &fCancel );
	void TextTipFetch( long lColumn, long lRow, WORD* pwMultiLine, SHORT* pnTipWidth, TCHAR* pstrTipText, BOOL* pfShowTip );
	void LeaveCell( long lOldColumn, long lOldRow, long lNewColumn, long lNewRow, BOOL* pfCancel );
	void SheetChanged( short nOldSheet, short nNewSheet );
	void RightClick( short nClickType, long lColumn, long lRow, long lMouseX, long lMouseY) {}
	void EditChange( long lColumn, long lRow );
	void KeyPress( int *piKeyAscii );
	void KeyDown( WORD wKeyCode, WORD wShiftStatus );
	
	void ExpandAll();
	void CollapseAll();

	bool SelectRow( long lRow );
	void RestoreColExtended();
	void RefreshCollapseExpandButton( CSSheetTAHub * pclOtherTAHub );

	CDS_HydroMod::eHMObj GetHMObj ( long lColumn, long lRow );
	bool IsEditAccAvailable( long lColumn, long lRow );

// Protected members
protected:
	void InitSheet( CDS_HmHub *pHub );
	bool CheckValidity( double dVal );
	void SheetHubHideColumns();
	void SheetStationHideColumns();
	void SetRowSeparator( int iNumberOfStation );
	void FillDpControllerColumns( long lRow );
	void CloseDialogSCB( CDialogExt *pDlgSCB, bool fModified );
	bool IsDDAvailable( long lColumn, long lRow );	
	bool SaveAndCompute( long lColumn, long lRow, bool fComputeAll = true );
	void FillBvColumns( CDS_HydroMod *pHM, long lCurRow );
	bool ValidateCell();
	
	// If column has no data and can be hide then do it
	void HideEmptyColumns( void );

private:
	// Expand or collapse all in regards to current state.
	// Returns 'true' if expanded occurs otherwise 'false'.
	bool _ExpandCollapseAll();

	// Allow to retrieve which is the last visible column
	long _GetLastVisibleColumn( void );
	
	// Allow to correctly draw last black horizontal line below all
	void _DrawBlackLine( void );

// Protected variables
protected:
	bool m_fPrinting;
	bool m_bCellModified;
	bool m_fHubMode;
	bool m_fModificationInCourse;
	bool m_fComboEditMode;
	long m_lRow, m_lCol;
	double m_dCellDouble;
	CString m_strCellText;
	LPARAM m_CellLParam;
	CDlgComboBoxHM *m_pSCB;
	CDlgHubCvCB *m_pSCvCB;
	CTADatabase* m_pTADB;
	CTADatastruct *m_pTADS;
	CDS_HmHub *m_pHub;
	CArray <long> m_arlHub, m_arlHubStation;		// Contains columns in expand state
};
