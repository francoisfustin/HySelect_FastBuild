#pragma once


#include "SSheet.h"
#include "HydroMod.h"
#include "afxwin.h"
#include "DialogExt.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"

// CDlgSearchAndReplaceCircuits dialog
enum _eWorkForCV
{
	ewcvKvs,
	ewcvTA,
	ewcvLast
};

class CDlgSearchAndReplaceCircuits : public CDialogExt, public CSSheet::INotificationHandler
{
	DECLARE_DYNAMIC( CDlgSearchAndReplaceCircuits )

public:
	enum { IDD = IDD_DLGSEARCHREPLACECIRCUITS };

	CDlgSearchAndReplaceCircuits( CWnd *pParent = NULL );
	virtual ~CDlgSearchAndReplaceCircuits();

	void UpdateData ( CDS_HydroMod *pHMSource );
	void DisableFindNextBtn();

	// Set what the user has choosen to check.
	void SetFlagTarget( bool bBalType, bool bWorkForCV, bool bCVCtrlType, bool bCtrlValvType );
	
	// Set the values user has set for the variable to check.
	void SetTarget(	CDB_CircuitScheme::eBALTYPE BalType, CDB_CircSchemeCateg *pclCircSchemeCategoriesSelected, _eWorkForCV WorkForCtrlValv, 
			CDB_ControlProperties::CvCtrlType CtrlType, CDB_ControlProperties::eCVFUNC CtrlValvType );

	void SetHMLeftTreeWidth( CRect &rect );

	void ApplyAll();
	void SearchAndReplaceFinished( std::map<WPARAM, CDS_HydroMod *> *pMap );

	// Overrides the 'CMultiSpreadBase' virtual public methods.
	virtual void SSheetOnBeforePaint( void );
	virtual void SSheetOnAfterPaint( void );

	class CCircuit
	{
	public:
		CCircuit();
		CCircuit( CDS_HydroMod *pHM );
		CCircuit &operator = ( CCircuit &Circ );
		bool operator == ( CCircuit &Circ );

		bool HMmatching( CDS_HydroMod *pHM );
		bool Init( CDS_HydroMod *pHM );

		// These variables tell us if we must check balancing type, circuit type and so on.
		bool m_bIsBalTypeMustBeChecked;
		bool m_bIsValveTypeMustBeChecked;
		bool m_bIsCtrlTypeMustBeChecked;
		bool m_bIsCtrlValveTypeMustBeChecked;

		// If we must check, these variables contain what exactly to check.
		CDB_CircuitScheme::eBALTYPE m_eBalType;				// No (Straight pipe), manual balancing or Dp controller.
		CDB_CircSchemeCateg *m_pclCircSchemeCategory;		// Circuit scheme selected.
		_eWorkForCV m_eValveType;						// IMI control valve or Kvs.
		CDB_ControlProperties::CvCtrlType m_eControlType;		// On/Off, 3 points or proportional.
		CDB_ControlProperties::eCVFUNC m_eControlValveType;		// Control only, presettable, presettable and point tests or both presettable or presettable and point tests.

		CString m_strHMName;

		// This variable contains the circuit scheme of the circuit saved in the 'm_mapAvailSolutions' and 'm_vecSolutionsKept' maps.
		CDB_CircuitScheme *m_pclCircuitScheme;

		bool m_bFoundSolution;
	};

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedApplyAll();
	afx_msg void OnClose();

	void _InitializeSSheetSrc( CSSheet *pSheet );
	void _InitializeSSheetTrg( CSSheet *pSheet );
	void _UpdateHMTrg();
	void _DrawSrcHMSchema();
	void _DrawTrgHMSchema( UINT nPos );
	void _AddOneAdditionalRow();
	void _RemoveAdditionalRows();
	void _ReplaceCircuit();
	int _FindPreviousUserChoice();
	void _ClearAllTrgHMSchema();
	void _SetFocusOnTrgHMSchema();
	void _ManageLButtonDown( CPoint point );
	
private:
	CWnd *m_pParent;
	CRect m_rectHMLeftTree;
	CStatic m_StaticLeftRect;
	CStatic m_StaticRightRect;
	CButton m_ButNext;
	CButton m_ButApply;
	CButton m_ButApplyAll;
	CButton m_ButClose;
	CExtStatic m_StaticFindWhat;
	CExtStatic m_StaticReplaceWith;
	CExtNumEdit m_EditHMName;
	CListBox m_ResultList;
	UINT m_uiIndexOfSelectedScheme;
	CSSheet *m_pclSSheetSrc;
	CSSheet *m_pclSSheetTrg;
	CDS_HydroMod *m_pHMSrc;
	std::map<int, CRect> m_mapCellCoords;

	CCircuit m_Target;

	// This allows to save a list of solutions already done. For each kind of source circuit, we have only one target if user
	// has clicked on 'Apply all'.
	std::vector<std::pair<CCircuit, CCircuit> > m_vecSolutionsKept;

	// For each source, this vector contains a list of solutions.
	std::vector<CCircuit> m_vecAvailSolutions;

	// Contains all the replacements applied
	std::vector<std::pair<CCircuit, CCircuit> > m_vecChangesList;
};
