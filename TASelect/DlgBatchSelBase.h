#pragma once


#include "ExtButton.h"
#include "ExtNumEdit.h"
#include "HydroMod.h"
#include "XGroupBox.h"
#include "ExtComboBox.h"
#include "DlgBatchSelectionOutput.h"

#define DLGBATCHSELBASE_SELECTIONBACKCOLOR				_IMI_GRAY_XXLIGHT
#define DLGBATCHSELBASE_BLOCKSELECTIONSTYLE				SS_BORDERSTYLE_SOLID
#define DLGBATCHSELBASE_BLOCKSELECTIONCOLOR				_BLACK
#define DLGBATCHSELBASE_BACKCOLOR_NORMAL				_WHITE
#define DLGBATCHSELBASE_BACKCOLOR_DATAERROR				_RED // _TAH_ORANGE_XLIGHT
#define DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR		RGB( 0xC0, 0xC0, 0xC0 )
#define DLGBATCHSELBASE_COLOR_HEADERROWBELOW			_BLACK
#define DLGBATCHSELBASE_COLOR_DATAROWSEPARATOR			RGB( 0xC0, 0xC0, 0xC0 )
#define DLBBATCHSELBASE_ROWHEIGHT_FIRSTLINE				( 12.75 * 0.5 )
#define DLBBATCHSELBASE_ROWHEIGHT_MAINHEADER			( 12.75 * 1.5 )
#define DLBBATCHSELBASE_ROWHEIGHT_HEADERNAME			( 12.75 * 2 )
#define DLBBATCHSELBASE_ROWHEIGHT_HEADERUNIT			( 12.75 * 1.2 )
#define DLBBATCHSELBASE_ROWHEIGHT_DATA					( 12.75 )

class CMainFrame;
class CDlgBatchSelBase : public CDlgSelectionBase, public CDlgBatchSelectionOutput::IBSNotificationHandler
{
public:
	// Remark: These values correspond to the radios order in the dialog. Make sure that the tab order starts well first on the 'Flow'
	//         and goes after on the 'Power' radio. Otherwise the values below are no more valid.
	enum RadioFlowOrPowerDT
	{
		Radio_Flow	= 0,
		Radio_PowerDT = 1
	};

	// To be available for 'DlgBatchSelDpC_DpCTab' and 'DlgBatchSelDpC_DpCTab'.
	typedef enum AboveOrBelow
	{
		Below65 = false,
		Above50 = true
	};

	typedef enum RowStatus
	{
		BS_RS_NotYetDone			= 0x00,
		BS_RS_FindOneSolution		= 0x01,
		BS_RS_FindSolutions			= 0x02,
		BS_RS_FindNoSolution		= 0x03,
		BS_RS_FindAlternative		= 0x10,
		BS_RS_FindOneSolutionAlter	= ( BS_RS_FindAlternative + BS_RS_FindOneSolution ),	// 0x11
		BS_RS_FindSolutionsAlter	= ( BS_RS_FindAlternative + BS_RS_FindSolutions )		// 0x12
	};

	// This class allows us to save all data that are available for one row to do a selection.
	class BSRowParameters
	{
	public:
		long m_lRow;
		DlgOutputHelper::mapIntCellBase m_mapCellData;
		RowStatus m_eStatus;
		CBatchSelectBaseList *m_pclBatchResults;
		CSelectedBase *m_pclSelectedProduct;
		CSelectedBase *m_pclSelectedSecondaryProduct;		// For Mv on DpC for example.
		CDB_Actuator *m_pclSelectedActuator;
		CDB_Product *m_pclSelectedAdapActuator;      // HYS-956: Display adapter between CV and actuator
		CDS_SSel *m_pclCDSSelSelected;		// To ease programming when editing a solution.
		CString m_strReference1;
		CString m_strReference2;
		double m_dFlow;
		double m_dDp;
		double m_dPower;
		double m_dDT;
		// For Dp selection.
		double m_dKvs;
		double m_dDpBranch;
		
		BSRowParameters()
		{
			m_lRow = 1;						// Coordinates are relative and are 1-indexed.
			m_eStatus = RowStatus::BS_RS_NotYetDone;
			m_pclBatchResults = NULL;
			m_pclSelectedProduct = NULL;
			m_pclSelectedSecondaryProduct = NULL;
			m_pclSelectedActuator = NULL;
			m_pclSelectedAdapActuator = NULL;
			m_pclCDSSelSelected = NULL;
			m_strReference1 = _T("");
			m_strReference2 = _T("");
			m_dFlow = 0.0;
			m_dDp = 0.0;
			m_dPower = 0.0;
			m_dDT = 0.0;
			m_dKvs = 0.0;
			m_dDpBranch = 0.0;
		}
	};

	CDlgBatchSelBase( CBatchSelectionParameters &clBatchSelParams, UINT nID = 0, CWnd *pParent = NULL );

	virtual ~CDlgBatchSelBase();

	void Suggest( void ) { OnBnClickedSuggest(); }

	bool IsButtonValidateEnabled( void );

	virtual CExtNumEditComboBox *GetpComboTypeBelow65( void ) { return &m_ComboTypeBelow65; }
	virtual CExtNumEditComboBox *GetpComboFamilyBelow65( void ) { return &m_ComboFamilyBelow65; }
	virtual CExtNumEditComboBox *GetpComboBdyMatBelow65( void ) { return &m_ComboBodyMaterialBelow65; }
	virtual CExtNumEditComboBox *GetpComboConnectBelow65( void ) { return &m_ComboConnectBelow65; }
	virtual CExtNumEditComboBox *GetpComboVersionBelow65( void ) { return &m_ComboVersionBelow65; }
	virtual CExtNumEditComboBox *GetpComboPNBelow65( void ) { return &m_ComboPNBelow65; }

	virtual CExtNumEditComboBox *GetpComboTypeAbove50( void ) { return &m_ComboTypeAbove50; }
	virtual CExtNumEditComboBox *GetpComboFamilyAbove50( void )  { return &m_ComboFamilyAbove50; }
	virtual CExtNumEditComboBox *GetpComboBdyMatAbove50( void ) { return &m_ComboBodyMaterialAbove50; }
	virtual CExtNumEditComboBox *GetpComboConnectAbove50( void ) { return &m_ComboConnectAbove50; }
	virtual CExtNumEditComboBox *GetpComboVersionAbove50( void ) { return &m_ComboVersionAbove50; }
	virtual CExtNumEditComboBox *GetpComboPNAbove50( void ) { return &m_ComboPNAbove50; }
	
	virtual CXGroupBox *GetpGroupValveBelow65( void ) { return &m_clGroupValveBelow65; }
	virtual CXGroupBox *GetpGroupValveAbove50( void ) { return &m_clGroupValveAbove50; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' public virtual methods.
	// Set the focus on the first available edit control.
	virtual	void SaveSelectionParameters( void );
	virtual	void LeaveLeftTabDialog( void );
	virtual void ActivateLeftTabDialog( void );

	// Called by 'CDlgLeftTabSelManager' when user change application type.
	virtual void ChangeApplicationType( ProjectType eProductSelectionApplicationType );

	virtual bool IsRightViewInEdition( void );
	virtual void OnRViewSSelSelectProduct( void );
	virtual void OnRViewSSelSelectionChanged( bool bSelectionActive );
	virtual void OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey );
	virtual bool ResetRightView( void ) { return false; }
	virtual CMainFrame::RightViewList GetRViewID( void );
	// End of overriding 'CDlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Overrides CDlgBatchSelectionOutput::INotificationHandler.
	virtual void OnBatchSelectionButtonClearAll( void );
	virtual void OnBatchSelectionButtonClearResults( void );
	virtual void OnBatchSelectionPasteData( void );
	virtual void OnBatchSelectionDeleteRow( long lStartRowRelative, long lEndRowRelative );
	virtual bool OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList );
	virtual void OnBatchSelectionAddColumn( int iColumnID );
	virtual void OnBatchSelectionRemoveColumn( int iColumnID );
	virtual bool OnBatchSelectionKeyboardVirtualKey( int iVirtualKey );
	virtual bool OnBatchSelectionIsDataExist( bool &bDataExist, bool &bDataValidated );
	virtual bool OnBatchSelectionIsResultExist( bool &bResultExist, bool &bResultValidated );
	virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct );
	virtual bool OnBatchSelectionDelete( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative );
	virtual bool OnBatchSelectionEdit( long lRowRelative );
	virtual bool OnBatchSelectionSuggest( void );
	virtual bool OnBatchSelectionValidate( void );
	virtual bool OnDlgOutputCellClicked( int iOutputID, int iColumnID, long lRowRelative );
	virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative );
	virtual bool OnDlgOutputCellEditModeOff( int iOutputID, int iColumnID, long lRowRelative, bool bChangeMade );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	virtual afx_msg void OnDestroy() {}

	virtual afx_msg void OnCbnSelChangeTypeBelow65();
	virtual afx_msg void OnCbnSelChangeFamilyBelow65();
	virtual afx_msg void OnCbnSelChangeBdyMatBelow65();
	virtual afx_msg void OnCbnSelChangeConnectBelow65();
	virtual afx_msg void OnCbnSelChangeVersionBelow65();
	virtual afx_msg void OnCbnSelChangePNBelow65();
		
	virtual afx_msg void OnCbnSelChangeTypeAbove50();
	virtual afx_msg void OnCbnSelChangeFamilyAbove50();
	virtual afx_msg void OnCbnSelChangeBdyMatAbove50();
	virtual afx_msg void OnCbnSelChangeConnectAbove50();
	virtual afx_msg void OnCbnSelChangeVersionAbove50();
	virtual afx_msg void OnCbnSelChangePNAbove50();

	virtual afx_msg void OnBnClickedSuggest() {}
	virtual afx_msg void OnBnClickedValidate();
	virtual afx_msg void OnBnClickedCancel() {}

	virtual afx_msg void OnBnClickedRadioFlowPower();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected methods.
	virtual void OnAfterNewDocument( void );
	// End of overriding 'CDlgSelectionBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Access to check Dp.
	virtual CButton *GetpCheckboxDp() const { return NULL; }
	virtual CButton *GetpBtnSuggest() const { return NULL; }
	virtual CButton *GetpBtnValidate() const { return NULL; }

	// This method can be overridden by inherited class to check if the button can be enable or not.
	// Typically, it's the case for separator.
	virtual void EnableSuggestButton( bool bEnable );

	// Methods for batch selection.

	// Allow to clear all previously working data.
	// Param: 'fForceDelete' is set to 'true' if we must delete ourself all 'CDS_HydroMod' solutions and their temporary
	//        parent table (a 'CTableHM' is created in CTADataStruct to maintain all 'CDS_HydroMod' while working on the batch selection).
	//        Typically this value is set to 'false' when this method is called from 'OnNewDocument' because all objects are already deleted.
	virtual void BS_ClearAllData( bool bForceDelete = true );
	
	virtual void BS_PrepareColumnList( void );
	virtual bool BS_InitMainHeaders( void );
	virtual bool BS_InitColumns( void );
	virtual void BS_VerifyAllDataInput( void );

	class BSRowParameters;
	virtual void BS_VerifyOneDataInput( DlgOutputHelper::CCellBase *pclCellToVerify, BSRowParameters *pclBSRowParameters );

	// Allow to read data that are in the 'pmapCellData' and transfer them in the 'pclBSRowParameters'.
	virtual void BS_ReadInputData( DlgOutputHelper::mapIntCellBase *pmapCellData, BSRowParameters *pclBSRowParameters );
	
	virtual bool BS_Validate( void );
	virtual void BS_EnableRadios( bool bEnable );
	virtual void BS_EnableCombos( bool bEnable );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters ) {}
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
	virtual bool BS_RemoveOneColumn( int iColumnID );
	virtual void BS_UpdateUnit( void );
	virtual void BS_UpdateIOColumnLayout( int iUpdateWhat );
	virtual bool BS_ComboChange( bool bIsLast = false );
	virtual void BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter ) {}
	virtual void BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter ) {}

	void FillComboBoxType( AboveOrBelow eAboveOrBelow, CString strTypeID = _T("") );
	void FillComboBoxFamily( AboveOrBelow eAboveOrBelow, CString strFamilyID = _T("") );
	void FillComboBoxBodyMaterial( AboveOrBelow eAboveOrBelow, CString strBdyMatID = _T("") );
	void FillComboBoxConnect( AboveOrBelow eAboveOrBelow, CString strConnectID = _T("") );
	void FillComboBoxVersion( AboveOrBelow eAboveOrBelow, CString strVersionID = _T("") );
	void FillComboBoxPN( AboveOrBelow eAboveOrBelow, CString strPNID = _T("") );

	virtual void GetTypeList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetFamilyList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetMaterialList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetConnectList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetVersionList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetPNList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );

	// Notify the inherited classes that the treatment for the combo change is finished.
	// It allows the inherited classes to do some work (Like change actuator combos for control batch).
	virtual void EndOfComboChange() { return; }

	void SetModificationMode( bool bFlag = true );

	virtual CRViewSSelSS *GetLinkedRightViewSSel( void ) { return NULL; }

	// Allow to update the 'm_pclBatchSelParams->m_eFlowOrPowerDTMode' in regards to the 'm_iRadioFlowPowerDT' variable.
	virtual void UpdateFlowOrPowerDTState();

	void CheckIfWeNeedToClearResults( void );
	// HYS-825: To update input data when changing default units
	void BS_UpdateFlowPowerDpCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnIDint, int iRow );

	// HYS-1995: Add calculated flow in PDT mode.
	void BS_UpdateFlowCell( int iOutputID, long lRow );

// Protected variables.
protected:
	CBatchSelectionParameters *m_pclBatchSelParams;

	CXGroupBox m_clGroupInputs;
	CXGroupBox m_clGroupValveBelow65;
	CXGroupBox m_clGroupValveAbove50;
	CExtButton m_clButtonSuggest;
	CButton m_clButtonValidate;				// Also used for 'Apply' cancel when editing a selection.
	CButton m_clButtonCancel;

	int m_iDN50;
	int m_iDN65;

	// Valves below DN 65.
	CExtNumEditComboBox m_ComboTypeBelow65;
	CExtNumEditComboBox m_ComboFamilyBelow65;
	CExtNumEditComboBox m_ComboBodyMaterialBelow65;
	CExtNumEditComboBox m_ComboConnectBelow65;
	CExtNumEditComboBox m_ComboVersionBelow65;
	CExtNumEditComboBox m_ComboPNBelow65;

	// Valves above DN 50.
	CExtNumEditComboBox m_ComboTypeAbove50;
	CExtNumEditComboBox m_ComboFamilyAbove50;
	CExtNumEditComboBox m_ComboBodyMaterialAbove50;
	CExtNumEditComboBox m_ComboConnectAbove50;
	CExtNumEditComboBox m_ComboVersionAbove50;
	CExtNumEditComboBox m_ComboPNAbove50;

	// VARIABLES FOR BATCH SELECTION.
	typedef enum BS_InputsVerificationStatus
	{
		BS_IVS_NotYetDone,				// Verification of inputs has not been yet executed.
		BS_IVS_OK,						// All input data are correct.
		BS_IVS_Error,					// There are one or mode errors with input data.
		BS_IVS_Empty					// All cells are empty.
	};

	typedef enum BS_SuggestionStatus
	{
		BS_SS_NotYetDone,
		BS_SS_Done
	};

	typedef enum BS_ValidationStatus
	{
		BS_VS_NotYetDone,
		BS_VS_Done
	};

	typedef enum BS_MainHeaderID
	{
		BS_MHID_InputOutput,
		BS_MHID_Last
	};

	typedef enum BS_MainHeaderSubID
	{
		BS_MHSUBID_Input,
		BS_MHSUBID_Output,
		BS_MHSUBID_Last
	};

	typedef enum BS_ColumnID
	{
		BS_CID_Input_FirstColumn = 0,
		BS_CID_Input_Status,
		BS_CID_Input_Reference1,
		BS_CID_Input_Reference2,
		BS_CID_Input_Flow,
		BS_CID_Input_Power,
		BS_CID_Input_DT,
		BS_CID_Input_Dp,
		BS_CID_Input_DpBranch,
		BS_CID_Input_Kvs,
		BS_CID_Input_Separator,
		BS_CID_Input_Last
	};

	typedef enum BS_RowRelativePos
	{
		BS_RID_MainHeader = 1,
		BS_RID_HeaderName = 1,
		BS_RID_HeaderUnit = 2,
		BS_RID_FirstData  = 1
	};

	typedef enum BS_CellStatus
	{
		BS_CS_OK,
		BS_CS_Error
	};

	typedef enum BS_UpdateInputColumn
	{
		BS_UIC_FlowOrPowerDT,
		BS_UIC_Dp,
		BS_UIC_Last
	};

	bool m_bInitialized;

	CDlgBatchSelectionOutput::CBSInterface m_clInterface;
	
	bool m_bAtLeastOneRowWithBestSolution;
	bool m_bAtLeastOneRowWithSolutionButNotBest;
	bool m_bAtLeastOneRowWithNoSolution;
	bool m_bAtLeastOneRowWithSolutionButAlternative;

	// Set to 'true' when user changes a combo and the question "Are you sure to delete current results and do a selection with new parameters?" has been
	// already submitted. This allow us to change following combos without asking again the same question.
	bool m_bMsgComboChangeDone;

	// Set to 'true' to do nothing when we change the last combo.
	bool m_bMsgDoNotClearResult;					
		
	bool m_bRadiosEnabled;
	int m_iRadioFlowPowerDT;
	BS_InputsVerificationStatus m_eInputsVerificationStatus;
	BS_SuggestionStatus m_eSuggestionStatus;
	BS_ValidationStatus m_eValidationStatus;

	typedef std::map<long, BSRowParameters> mapLongRowParams;
	typedef mapLongRowParams::iterator mapLongRowParamsIter;

	mapLongRowParams m_mapAllRowData;
	BSRowParameters *m_pclCurrentRowParameters;

	// This class allows us to save all data for the column definitions.
	class BSColParameters
	{
	public:
		int m_iColumnID;
		// 'false' is this column can't be used.
		bool m_bEnabled;
		// 'true' if this column is fixed and can't be removed.
		bool m_bCantRemove;
		// 'true' if this column is currently displayed.
		bool m_bDisplayed;
		// 'true' if user has intentionally chosen to display this column or not.
		// It's for example the batch BCV case. In regards to user has checked or not the 'Dp' checkbox in the left panel, we must show 'Dp' column or
		// 'Dp full opening' and 'Dp half opening'. If user chooses to input a 'Dp' value, and intentionally adds the 'Dp' column and after that uncheck 'Dp'
		// we must no more show the 'Dp' column. But if user checks again the 'Dp', we must keep in memory his choice and display or not 'Dp'.
		eBool3 m_eb3UserChoice;
		DlgOutputHelper::CColDef m_clHeaderColDef;
		DlgOutputHelper::CFontDef m_clHeaderFontDef;
		DlgOutputHelper::CColDef m_clDataColDef;
		DlgOutputHelper::CFontDef m_clDataFontDef;
		CString m_strHeaderName;
		int m_iHeaderUnit;
		CString m_strHeaderUnit;
		bool m_bLineBelowHeader;
		BSColParameters()
		{
			m_iColumnID = 0;
			m_bEnabled = true;
			m_bCantRemove = false;
			m_bDisplayed = false;
			m_eb3UserChoice = eBool3::eb3Undef;
			m_strHeaderName = _T("");
			m_iHeaderUnit = -1;
			m_strHeaderUnit = _T("");
			m_bLineBelowHeader = false;
		};
	};

	typedef std::map<int, BSColParameters> mapLongColParams;
	typedef mapLongColParams::iterator mapLongColParamsIter;
	mapLongColParams m_mapColumnList;

// Private methods.
private:
	void _BSFirstTimeInit( void );
	void _BSResizingColumnInit( void );

	// Allow to read one cell data and transfer value in the 'pclBSRowParameters'.
	void _BS_ReadOneInputData( DlgOutputHelper::CCellBase *pclCellData, BSRowParameters *pclBSRowParameters );

	void _UpdateFlowCellProperties( bool bStatus );
// Private variables.
private:
	class techParams
	{
	public:
		double m_dTargetLinearDp;
		double m_dMaxLinearDp;
		double m_dTargetWaterVelocity;
		double m_dMaxWaterVelocity;
		int m_iTargetWaterVelocity;
		int m_iSizeShiftAbove;
		int m_iSizeShiftBelow;
		techParams()
		{
			m_dTargetLinearDp=0;
			m_dMaxLinearDp=0;
			m_dTargetWaterVelocity=0;
			m_dMaxWaterVelocity=0;
			m_iTargetWaterVelocity=0;
			m_iSizeShiftAbove=0;
			m_iSizeShiftBelow=0;
		}
	};
	techParams m_rTechParamSaved;
};
