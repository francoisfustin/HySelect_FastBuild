#pragma once


#include "afxcmn.h"
#include "MyToolTipCtrl.h"
#include "afxwin.h"
#include "ExtComboBox.h"
#include "XGroupBox.h"
#include "DialogExt.h"

class CDlgCustPipe : public CDialogExt
{
public:
	enum { IDD = IDD_DLGCUSTPIPE };

	CDlgCustPipe( CWnd *pParent = NULL );
	virtual ~CDlgCustPipe();
	BOOL PreTranslateMessage(MSG* pMsg);

	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display();

	// Create a unique ID for new pipe.
	CString BuildPipeID( bool fPipeSerie = false );

	// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	// Delete pipes in user db, copy pipes from BakDB and delete BackDB.
	virtual void OnCancel();

	afx_msg void OnKillFocusPipeSeries( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnSetFocusPipeSeries( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTreeSelChangedPipesSeries( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTreeClickPipesSeries( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTreeDoubleClickPipesSeries( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTreeRightClickPipesSeries( NMHDR *pNMHDR, LRESULT *pResult );

	// Change the series name.
	// Remark: check if series is unlocked, if new name isn't empty and if new name does not already exist.
	afx_msg void OnTreeBeginLabelEditPipeSeries( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTreeEndLabelEditPipeSeries( NMHDR *pNMHDR, LRESULT *pResult );

	afx_msg void OnSetFocusListpipe( NMHDR *pNMHDR, LRESULT *pResult );

	// Remark: if user clicks on empty part of the pipe list we unselect pipe.
	afx_msg void OnListClickPipe( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnListDoubleClickPipe( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnListRightClickPipe( NMHDR *pNMHDR, LRESULT *pResult );

	// User select another pipe in the current pipe series.
	afx_msg void OnListItemChanged( NMHDR *pNMHDR, LRESULT *pResult );

	// Before user select another pipe in the current pipe series.
	afx_msg void OnListItemChangingSavePipe(NMHDR *pNMHDR, LRESULT *pResult);

	// Change pipe name.
	afx_msg void OnListEndLabelEditPipe( NMHDR *pNMHDR, LRESULT *pResult );

	// Selection change on corresponding valve.
	afx_msg void OnCbnSelChangeValve();
	afx_msg void OnEnKillFocusInternalDiameter();
	afx_msg void OnEnKillFocusRoughness();
	afx_msg void OnEnKillFocusMaxPressure();
	afx_msg void OnEnKillFocusMaxTemp();

	afx_msg void OnBnClickedUnhideAllPipeSeries();
	afx_msg void OnBnClickedHideAllPipeSeries();
	afx_msg void OnBnClickedDefaultPipeSeries();
	afx_msg void OnBnClickedExpTxt();
	afx_msg void OnButtonCreate();
	afx_msg void OnButtonDelete();

	afx_msg void OnFltMenuPipesEdit();
	afx_msg void OnFltMenuPipesDelete();
	afx_msg void OnFltMenuPipesAdd();

	afx_msg void OnFltMenuSeriesEdit();
	afx_msg void OnFltMenuSeriesSelect();
	afx_msg void OnFltMenuSeriesUnselect();
	afx_msg void OnFltMenuSeriesDefault();
	afx_msg void OnFltMenuSeriesDelete();
	afx_msg void OnFltMenuSeriesAdd();

	// User click on the tree. Display information according to the tree state.
	LRESULT OnCheckStateChange( WPARAM wParam, LPARAM lParam );

	// Protected variables.
protected:
	CTADatabase *m_pTADB;
	CImageList *m_pclTreeImageList;
	CXGroupBox m_GroupPipe;
	CListCtrl m_ListPipe;
	CTreeCtrl m_Tree;
	short m_TreeItemHeight;
	CEdit m_EditPressure;
	CEdit m_EditTemp;
	CEdit m_EditRoughness;
	CEdit m_EditDiameter;
	CExtNumEditComboBox m_ComboValve;
	CButton m_ButtonDelete;
	CButton m_ButtonCreate;
	CButton m_ButHide;
	CButton m_ButUnhide;
	CButton m_ButtonSetAsDefaultPipeSeries;
	CMyToolTipCtrl m_ToolTip;

	// Private methods.
private:
	enum OverlayMaskIndex
	{
		OMI_Undefined		= 0,
		OMI_SetAsDefault	= 1
	};

	enum ButtonType
	{
		Nothing,
		Series,
		Pipe,
	};
	
	enum class eIsLockedBy
	{
		eUnlocked = 0,
		eMainPipe, //GEN_STEEL pipe
		eTechParamDefaultPipe,
		eHydraulicNetwork,
	};

	// Reset list with user DB pipe series.
	// Remark: if 'pStr' is defined then select the corresponding item.
	void _ResetAll( CString *pStr );

	// Enable/Disable, clear all edit box, Enable/Disable combobox valve.
	// If 'fClear' is set to 'true' then we reset the selected item.
	void _SetEditor( bool fEnabled, bool fClear );

	// Enable or disable buttons 'Create' and 'Delete' and set captions for these buttons.
	void _SetButtons( ButtonType eButtonCreate, ButtonType eButtonDelete, bool fEnableCreate, bool fEnableDelete );

	// Search in a ListCtrl for the first free name based on an fixed string *pStr and an index 1 ... n.
	// Return : 'true' if a string is found, *pStr contains the new string.
	//		    'false' if index must be >= 100.
	bool _FindFirstFreeName( CListCtrl *pclList, CString *pStr );

	// Delete a selected pipe...
	// Returns '0' if OK or '-1' if bad selection or pipe is fixed.
	int	 _DeleteSelPipe( int iPipe );

	// Check one edit box.
	// Returns 'true' or 'false'.
	bool _CheckEditBox( CEdit *pclEdit, double *pdValueSI, int iPhysType );

	// Check and remove empty series.
	// if 'fDelete' is set to 'false' the method returns true immediately when a empty series is found.
	// if 'fDelete' is set to 'true' the method deletes this empty series.
	bool _CheckEmptySeries( CPipeUserDatabase *pclUserDatabase, bool fDelete = false );

	void _FillListPipe( CTable *pclTable );
	void _EnableButtonDefaultPipeSeries( CTable *pclTable );
	void _FillUsedPipeSeriesList( CTable *pclTable );
	eIsLockedBy _CanHidePipeSeries( IDPTR IDPtrSerie );

	// The function return true if the IDPtr exist in 'pclTable'.
	bool _CheckPipeUsed( IDPTR IDPtrSerie, IDPTR IDPtrSize, CTable *pclTable );

	// Force a pipe refresh in each hydromod.
	void _RefreshPipeUsed( CTable *pclTable );

	void _SaveDefaultPipeSeries( CString strPipeID );

	void _SetState( void );

	void _CreateSeries( void );
	void _CreatePipe( void );

	void _DeleteSeries( void );
	void _DeletePipe( void );

	// Private variables.
private:
	CPipeUserDatabase	m_BackupDB;
	CUnitDatabase *m_pUnitDB;
	double m_dIntDiameter;
	double m_dRoughness;
	double m_dTemperature;
	double m_dPressure;
	CString m_DefaultPipeSeriesID;
	bool m_bModified;
	ButtonType m_eButtonCreateType;
	ButtonType m_eButtonDeleteType;
	CMenu m_FltMenuSeries;
	CMenu m_FltMenuPipes;
	CDS_TechnicalParameter *m_pTechParam;
	CStringList m_UsedPipeSeries;
};
