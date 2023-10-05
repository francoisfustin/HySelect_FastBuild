#pragma once


#include "SSheet.h"
#include "HydroMod.h"
#include "afxwin.h"
#include "ExtStatic.h"
#include "ExtListCtrl.h"

#define DLGSRSET_LEFTCOLUMNINDEX		0
#define DLGSRSET_RIGHTCOLUMNINDEX		1
#define DLGSRSET_AJUSTCOORDS( Left, Right)	{ Left = max( Left, Right); Right = Left; }

class CDlgSearchAndReplaceSet : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgSearchAndReplaceSet )

public:
	enum { IDD = IDD_DLGSEARCHREPLACESET };

	enum WorkingOn
	{
		WO_HM,
		WO_IndSel
	};
	enum WorkingMode
	{
		WM_Group,
		WM_Split
	};

	CDlgSearchAndReplaceSet( CWnd* pParent = NULL );
	virtual ~CDlgSearchAndReplaceSet();

	void SetWorkingMode( WorkingMode eWorkingMode ) { m_eWorkingMode = eWorkingMode; }
	bool SetParams( std::vector<LPARAM>* pvecParams );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	afx_msg void OnMove( int x, int y );
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
	void _FillSrcList();
	void _FillDstList();
	void _FillDstListNotAvailableOrDeleted( void );
	void _FillDstListSetNoBreakable( void );
	void _FillChangeDoneList();
	
	void _FillCVArticleReferenceNoSet( CDB_ControlValve* pclControlValve, CExtListCtrl* pList );
	void _FillCVArticleReferenceSet( CDB_Set* pCVActSet, CExtListCtrl* pList );
	void _FillCVCommonPart( CDB_ControlValve* pclControlValve, CExtListCtrl* pList );
	void _FillAccessoryPart( CDB_Product* pclAccessory, bool fForSet, bool fByPair, CExtListCtrl* pList );
	void _FillActuatorPart( CDB_Actuator* pclActuator, CDB_ControlValve* pclControlValve, bool fForSet, CExtListCtrl* pList );
	void _FillAdapterPart( CDB_Product* pclAdapter, bool fForSet, CExtListCtrl* pList );
	void _FillCVActrAccSetPart( CDB_Set* pCVActSet, CExtListCtrl* pList );
	void _FillQuantity( int iQuantity, CExtListCtrl* pList );

	bool _GetFirstSelection( void );
	bool _GetPreviousSelection( void );
	bool _IsPreviousSelectionExist( void );
	bool _GetNextSelection( void );
	bool _IsNextSelectionExist( void );
	
	void _ShowSolution( void );
	bool _GetFirstCVActrSetSolution( void );
	bool _GetNextCVActrSetSolution( void );
	bool _IsNextCVActrSetSolutionExist( void );
	void _GetCVActrSetSolution( int iIndex );
	int _GetNumberOfSolution( void );

	void _GetAdapter();
	
	bool _FindSolution();

	void _ApplyOnHM( void );
	void _ApplyOnIndSel( void );
	void _ApplyAll( void );
	void _ShowFinalMessage();

private:
	CWnd *m_pParent;
	WorkingOn m_eWorkingOn;
	WorkingMode m_eWorkingMode;
	bool m_bInitialized;
	CExtStatic m_StaticCVSrc;
	CExtStatic m_StaticCVTrg;
	CExtStatic m_StaticNoSolution;
	CExtListCtrl m_ListCVSrc;
	CExtListCtrl m_ListCVTrg;
	CButton m_ButApply;
	CButton m_ButApplyAll;
	CButton m_ButPrevious;
	CButton m_ButNext;
	CButton m_ButClose;
	CExtListCtrl m_ListChangeDone;
	int m_iListSrcTitleID;
	int m_iListTrgTitleID;
	int m_iNbrRow;
	int m_iLeftPos;
	int m_iRightPos;
	bool m_bSetNotAvailableOrDeleted;
	COLORREF m_cTextForeColor;
	COLORREF m_cTextBackColor;

	int m_iCurrentSourceSelected;									// Set the current source selected.

	std::vector<CDS_HydroMod *> *m_pvecHMList;						// Contains all HM selections submitted on the dialog.
	std::vector<CDS_HydroMod *> m_vecHMListCopy;					// Contains a copy to display original when selection has been changed.
	bool m_bHMSrcSelectedAsaPackage;
	CDB_Set *m_pHMCvActrSet;

	std::vector<CDS_SSelCtrl *> *m_pvecSelCtrlList;					// Contains all individual selections submitted to the dialog.
	std::vector<CDS_SSelCtrl *> m_vecSelCtrlListCopy;				// Contains a copy to display original when selection has been changed.
	std::map<int, bool> m_mapChangeStatus;							// Set for each control valve if user has changed something (click on 'Apply') or not.

	std::vector<CDB_Set *> m_vecCVActrSetSolution;
	int m_iCurrentCVActrSet;

	CDB_Product *m_pclAdapter;
	std::vector<CDB_Product *> m_vecCVSetAccessoryList;

	int m_iTotalChange;
};
