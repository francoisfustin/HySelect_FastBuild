#pragma once


#include "DlgBatchSelBase.h"
#include "DlgBatchSelDpC_DpCTab.h"
#include "DlgBatchSelDpC_BvTab.h"
#include "ProductSelectionParameters.h"

#define DLBBATCHSELDPC_ROWHEIGHT_MAINHEADER			( 12.75 )

class CDlgBatchSelDpC_DpcBvCtrlTab : public CMFCTabCtrl
{
public:
	// Overrides some 'CMFCBaseTabCtrl' to avoid drag to be moved.
	virtual DROPEFFECT OnDragEnter( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual DROPEFFECT OnDragOver( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ){ return DROPEFFECT_NONE; }
	virtual void SwapTabs( int nFisrtTabID, int nSecondTabID ) {}
	virtual void MoveTab( int nSource, int nDest ) {}
};

class CDlgBatchSelDpC : public CDlgBatchSelBase
{
public:
	enum { IDD = IDD_DLGBATCHSELDPC };
	CDlgBatchSelDpC( CWnd *pParent = NULL );
	virtual ~CDlgBatchSelDpC();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DialogExt' public virtual methods.
	virtual void SetApplicationBackground( COLORREF cBackColor );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption()	{ return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_DPC ); }
	virtual void SaveSelectionParameters();
	virtual void ActivateLeftTabDialog( void );
	virtual bool ResetRightView( void );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgBatchSelBase' public virtual methods.
	virtual CExtNumEditComboBox *GetpComboTypeBelow65() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboFamilyBelow65() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboBdyMatBelow65() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboConnectBelow65() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboVersionBelow65() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboPNBelow65()	{ return NULL; }
						 
	virtual CExtNumEditComboBox *GetpComboTypeAbove50() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboFamilyAbove50() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboBdyMatAbove50() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboConnectAbove50() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboVersionAbove50() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboPNAbove50()	{ return NULL; }
	
	virtual CXGroupBox *GetpGroupValveBelow65() { return NULL; }
	virtual CXGroupBox *GetpGroupValveAbove50() { return NULL; }

	virtual bool OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList );
	virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct );
	virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative );
	// End of overriding 'DlgBatchSelBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Methods for 'CDlgBatchSelDpC_BvTab' and 'CDlgBatchSelDpC_DpCTab' dialogs.
	bool DpCBvCtrlTab_BS_ComboChange( bool bIsLast = false ) { return BS_ComboChange( bIsLast ); }
	void DpCBvCtrlTab_NotificationCbnChange( void );

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	virtual afx_msg void OnBnClickedCheckDpBranchOrKvs();
	virtual afx_msg void OnBnClickedCheckDpMax();
	virtual afx_msg void OnCbnSelChangeDpStab();
	virtual afx_msg void OnCbnSelChangeDpCLoc();
	virtual afx_msg void OnCbnSelChangeMvLoc();
	virtual afx_msg void OnEnChangeDpMax();
	virtual afx_msg void OnEnKillFocusDpMax();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedValidate();
	virtual afx_msg void OnBnClickedCancel();
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgBatchSelBase' protected methods.
	virtual CButton *GetpBtnSuggest() const {	return ( (CButton *)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnValidate() const { return ( (CButton *)GetDlgItem( IDC_BUTTONVALIDATE ) ); }

	virtual void BS_PrepareColumnList( void );
	virtual bool BS_InitMainHeaders( void );
	virtual bool BS_InitColumns( void );
	virtual bool BS_Validate( void );
	virtual void BS_EnableRadios( bool bEnable );
	virtual void BS_EnableCombos( bool bEnable );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters );
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
	virtual void BS_UpdateInputColumnLayout( int iUpdateWhat );
	virtual void BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter );
	virtual void BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter );
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	virtual void UpdateFlowOrPowerDTState();
	// End of overriding 'CDlgBatchSelBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected methods.
	virtual void ClearAll( void );
	// End of overriding 'CDlgSelectionBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	typedef enum BS_DpCUpdateInputColumn
	{
		BS_DpCUIC_DpBranch = BS_UpdateInputColumn::BS_UIC_Last,
		BS_DpCUIC_Kvs,
		BS_DpCUIC_DpMax,
		BS_DpCUIC_Last
	};

// Private methods.
private:
	void _FillComboDpStab( eDpStab eDpStabilizationOn );
	void _FillComboDpCLoc( eDpCLoc eDpCLocation );
	void _FillComboMvLoc( eMvLoc eMvLocation );

	bool _BS_ApplySelection();
	CDS_SSelDpC *_BS_CreateSSelDpC( BSRowParameters *pclRowParameters );
	void _BS_DisplaySolutions();
	void _BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID );

	// HYS-1188: We split 'm_iDpCCheckDpKvs' in two variables.
	void _UpdateDpBranchCheckboxState();
	void _UpdateDpBranchColumn();
	void _UpdateKvsCheckboxState();
	void _UpdateKvsColumn();

	void _UpdateDpMaxFieldState();
	void _UpdateDpMaxColumn();

	// Called when user changes the Dp stabilization type (On a branch or on a control valve).
	// We need in this case to switch between Dp or Kv.
	void _UpdateDpStabilizedOn();

	// Called when user changes the DpC location.
	void _UpdateDpCLocation();

	// Called when user changes the Mv location.
	void _UpdateMvLocation();

	// Allow to fill 'm_clIndSelDpCParams' with needed data from 'm_clBatchSelDpCParam' to show results in the 'RViewSSelDpC' when
	// user double clicks on a batch result.
	bool _CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters );

// Private variables.
private:
	// To make batch selection.
	CBatchSelDpCParams m_clBatchSelDpCParams;

	// To allow to view all available results for one row (When double-clicking on it) in the 'RViewSSelDpC' view.
	CIndSelDpCParams m_clIndSelDpCParams;

	CButton	m_clCheckboxDpBranchOrKvs;
	CButton m_clCheckboxDpMax;
	CExtNumEdit m_clExtEditDpMax;

	CExtNumEditComboBox m_ComboDpStab;
	CExtNumEditComboBox m_ComboDpCLoc;
	CExtNumEditComboBox m_ComboMvLoc;

	typedef enum TabIndex
	{
		etiDpC = 0,
		etiBv
	};
	CDlgBatchSelDpC_DpcBvCtrlTab m_clDpcBvCtrlTab;
	CDlgBatchSelDpC_DpCTab *m_pclDlgBatchSelDpC_DpCTab;
	CDlgBatchSelDpC_BvTab *m_pclDlgBatchSelDpC_BvTab;

	CXGroupBox m_clGroupDpCScheme;

	// Variables used when editing a batch result.
	CRankEx m_DpCList;

	// VARIABLES FOR BATCH SELECTION.
	typedef enum BS_DPCMainHeaderID
	{
		BS_DPCHMID_First = BS_MainHeaderID::BS_MHID_Last,
		BS_DPCMHID_DpCBv = BS_DPCHMID_First,
		BS_DPCMHID_Last
	};

	typedef enum BS_DPCMainHeaderSubID
	{
		BS_DPCMHSUBID_DpController,
		BS_DPCMHSUBID_RegulatingValve,
		BS_DPCMHSUBID_Last
	};

	typedef enum BS_DpCColumnID
	{
		BS_DpCCID_First = BS_ColumnID::BS_CID_Input_Last,
		BS_DpCCID_Name = BS_DpCCID_First,
		BS_DpCCID_Material,
		BS_DpCCID_Connection,
		BS_DpCCID_Version,
		BS_DpCCID_PN,
		BS_DpCCID_Size,
		BS_DpCCID_DpMin,
		BS_DpCCID_DpRange,
		BS_DpCCID_DpMax,
		BS_DpCCID_TemperatureRange,
		BS_DpCCID_PipeSize,
		BS_DpCCID_PipeLinDp,
		BS_DpCCID_PipeV,
		BS_DpCCID_BvName,
		BS_DpCCID_BvIn,					// Primary or secondary.
		BS_DpCCID_BvMaterial,
		BS_DpCCID_BvConnection,
		BS_DpCCID_BvVersion,
		BS_DpCCID_BvPN,
		BS_DpCCID_BvSize,
		BS_DpCCID_BvPreset,
		BS_DpCCID_BvDpSignal,
		BS_DpCCID_BvDp,
		BS_DpCCID_BvDpFullOpening,
		BS_DpCCID_BvDpHalfOpening,
		BS_DpCCID_BvTemperatureRange,
		BS_DpCCID_BvPipeSize,
		BS_DpCCID_BvPipeLinDp,
		BS_DpCCID_BvPipeV,
		BS_DpCCID_Last
	};
};
