#pragma once


#include "SSheet.h"
#include "HydroMod.h"
#include "afxwin.h"
#include "ExtListCtrl.h"
#include "ExtStatic.h"

#define DLGSRACTR_LEFTCOLUMNINDEX		0
#define DLGSRACTR_RIGHTCOLUMNINDEX		1
#define DLGSRACTR_LEFTCOLUMNWIDTH		93
#define DLGSRACTR_RIGHTCOLUMNWIDTH		236
#define DLGSRACTR_AJUSTCOORDS( Left, Right)	{ Left = max( Left, Right); Right = Left; }


class CDlgSearchAndReplaceActuator : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgSearchAndReplaceActuator )

public:
	enum { IDD = IDD_DLGSEARCHREPLACEACTUATOR };

	enum WorkingOn
	{
		WO_HM,
		WO_IndSel
	};
	enum WorkingMode
	{
		WM_Add,
		WM_Remove
	};

	CDlgSearchAndReplaceActuator( CWnd* pParent = NULL );
	virtual ~CDlgSearchAndReplaceActuator();

	void SetWorkingMode( WorkingMode eWorkingMode ) { m_eWorkingMode = eWorkingMode; }
	bool SetParams( std::vector<LPARAM>* pvecParams );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	afx_msg void OnMove( int x, int y );
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnClickedButApply();
	afx_msg void OnClickedButApplyAll();
	afx_msg void OnClickedButPrevious();
	afx_msg void OnClickedButNext();
	afx_msg void OnClickedButClose();

// Private methods.
private:
	enum WhichPart
	{
		WP_Left,
		WP_Right
	};
	void _FillSrcList( void );
	void _FillDstList( void );
	void _FillDstListNotAvailableOrDeleted( void );
	void _FillDstListSetNoBreakable( void );
	void _FillDstListNoSolution( void );

	void _FillCVArticleReferenceNoSet( CDB_ControlValve* pclControlValve, CExtListCtrl* pList );
	void _FillCVArticleReferenceSet( CDB_Set* pCVActSet, CExtListCtrl* pList );
	void _FillCVCommonPart( CDB_ControlValve* pclControlValve, CExtListCtrl* pList );
	void _FillAccessoryPart( CDB_Product* pclAccessory, bool fForSet, bool fByPair, CExtListCtrl* pList );
	void _FillAdapterPart( CDB_Product* pclAdapter, bool fForSet, CExtListCtrl* pList );
	void _FillActuatorPart( CDB_Actuator* pclActuator, CDB_ControlValve* pclControlValve, bool fForSet, CExtListCtrl* pList );
	void _FillCVActrAccSetPart( CDB_Set* pCVActSet, CExtListCtrl* pList );
	void _FillQuantity( int iQuantity, CExtListCtrl* pList );

	bool _GetFirstSelection( void );
	bool _GetPreviousSelection( void );
	bool _IsPreviousSelectionExist( void );
	bool _GetNextSelection( void );
	bool _IsNextSelectionExist( void );

	void _ShowSolution( void );
	bool _GetFirstSolution( void );
	bool _GetNextSolution( void );
	bool _IsNextSolutionExist( void );
	void _GetSolution( int iIndex );
	int _GetNumberOfSolution( void );

	void _GetAdapter( void );

	bool _FindSolution( bool fUseParams = false, IDPTR PowerSupplyIDPtr = _NULL_IDPTR, IDPTR InputSignalIDPtr = _NULL_IDPTR, 
						CDB_ControlValve::DRPFunction eDRPFunctionToCheck = CDB_ControlValve::DRPFunction::drpfUndefined, bool fDowngradeFunctionalities = false );

	void _ApplyOnIndSel( void );
	void _ApplyOnHM( void );
	void _ApplyAll( void );
	void _ShowFinalMessage( void );

private:
	CWnd *m_pParent;
	WorkingOn m_eWorkingOn;
	WorkingMode m_eWorkingMode;
	bool m_bInitialized;
	CExtStatic m_StaticCVSrc;
	CExtStatic m_StaticCVTrg;
	CRect m_rectEditCVTrg;
	CExtStatic m_StaticNoSolution;
	CExtListCtrl m_ListCVSrc;
	CExtListCtrl m_ListCVTrg;
	CButton m_ButApply;
	CButton m_ButApplyAll;
	CButton m_ButPrevious;
	CButton m_ButNext;
	CButton m_ButClose;
	CExtListCtrl m_ListChangeDone;
	CScrollBar m_ScrollBar;
	int m_iListSrcTitleID;
	int m_iListTrgTitleID;
	int m_iNbrRow;
	int m_iLeftPos;
	int m_iRightPos;
	bool m_bSetNotAvailableOrDeleted;
	COLORREF m_cTextForeColor;
	COLORREF m_cTextBackColor;

	int m_iCurrentSourceSelected;									// Set the current source selected.
	
	std::vector<CDS_HydroMod *> *m_pvecHMList;						// Contains all HM selections submitted to the dialog.
	std::vector<CDS_HydroMod *> m_vecHMListCopy;					// Contains a copy to display original when selection has been changed.
	bool m_bHMSrcSelectedAsaPackage;
	CDB_Set *m_pHMCvActrSet;

	std::vector<CDS_SSelCtrl *> *m_pvecSelCtrlList;					// Contains all individual selections submitted to the dialog.
	std::vector<CDS_SSelCtrl *> m_vecSelCtrlListCopy;				// Contains a copy to display original when selection has been changed.
	std::map<int, bool> m_mapChangeStatus;							// Set for each control valve if user has changed something (click on 'Apply') or not.

	IDPTR *m_pPowerSupplyIDPtr;										// Contains power supply IDPTR that user has chosen for actuator.
	IDPTR *m_pInputSignalIDPtr;										// Contains input signal IDPTR that user has chosen for actuator.
	bool m_bFailSafeFct;											// Contains fail-safe function that user has chosen for actuator.
											
	bool m_bDowngradeFunctionalities;								// Set to 'yes' if user allows actuator has its functionality downgraded.
	bool m_bForceSetWhenPossible;									// Set to 'yes' if user wants to force a set when it is possible.

	std::vector<CDB_Actuator *>	m_vecActuatorSolution;				// Contains a list of actuator for the current control valve.
	std::vector<CDB_Set *>	m_vecCVActrSetSolution;					// Contains a list of control valve actuator set for the current control valve.
	int m_iCurrentSolution;											// Index of the current solution in the above vector.
	bool m_bSetForced;												// 'true' if solution has been forced to be a set.
	std::map<CDB_Actuator *, bool> m_mapDowngradeStatus;				// Keep in memory the actuators for which functionality has been downgraded.

	CDB_Product *m_pclAdapter;									// Current adapter link to control valve and actuator.
	bool m_bAdapterBelongsToSet;
	std::vector<CDB_Product *> m_vecCVSetAccessoryList;			// Contains list of accessories linked to the current control valve and actuator set.

	int m_iTotalChange;												// Memorize total number of change from the user.
	CDB_ControlValve::DRPFunction m_eDefaultReturnPosFct; 	// Contains fail safe function that user has chosen for actuator.
};
