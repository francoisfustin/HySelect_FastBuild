#pragma once


#define DLBBATCHSELBCV_ROWHEIGHT_MAINHEADER			( 12.75 )

class CDlgBatchSelBCv : public CDlgBatchSelCtrlBase
{
public:
	enum { IDD = IDD_DLGBATCHSELBCV };
	CDlgBatchSelBCv( CWnd *pParent = NULL );
	virtual ~CDlgBatchSelBCv();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_BCV ); }
	virtual void SaveSelectionParameters();
	virtual bool ResetRightView( void );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgBatchSelBase' public virtual methods.
	virtual bool OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList );
	virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct );
	virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative );
	// End of overriding 'DlgBatchSelBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCheckDp();
	afx_msg void OnBnClickedCheckStrict();
	afx_msg void OnCbnSelChange2w3w();
	afx_msg void OnCbnSelChangeCtrlType();

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
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgBatchSelBase' protected methods.
	virtual CButton *GetpCheckboxDp() const { return ( (CButton *)GetDlgItem( IDC_CHECKDP ) ); }
	virtual CButton *GetpBtnSuggest() const { return ( (CButton *)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnValidate() const { return ( (CButton *)GetDlgItem( IDC_BUTTONVALIDATE ) ); }

	virtual void BS_PrepareColumnList( void );
	virtual bool BS_InitMainHeaders( void );
	virtual bool BS_InitColumns( void );
	virtual bool BS_Validate( void );
	virtual void BS_EnableCombos( bool bEnable );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters );
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
	virtual void BS_UpdateInputColumnLayout( int iUpdateWhat );
	virtual void BS_ReadAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter );
	virtual void BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter );
	virtual CRViewSSelSS *GetLinkedRightViewSSel( void );
	// End of overriding 'CDlgBatchSelBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected methods.
	virtual void ClearAll( void );
	// End of overriding 'CDlgSelectionBase' protected methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// HYS-1458 : Rename column
	virtual void RenameColHeader();
// Private methods.
private:
	void _FillCombo2w3w( CDB_ControlProperties::CV2W3W eCv2w3w = CDB_ControlProperties::CV2W3W::CV2W );
	void _FillComboCtrlType( CDB_ControlProperties::CvCtrlType eCvCtrlType = CDB_ControlProperties::eCvProportional );

	bool _BS_ApplySelection();
	CDS_SSelBCv *_BS_CreateSSelBCv( BSRowParameters *pclRowParameters );
	void _BS_DisplaySolutions();
	void _BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID );

	// Allow to fill 'm_clIndSelBCVParams' with needed data from 'm_clBatchSelBCVParams' to show results in the 'RViewSSelBCV' when
	// user double clicks on a batch result.
	bool _CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters );

// Private variables.
private:
	// To make batch selection.
	CBatchSelBCVParams m_clBatchSelBCVParams;
	
	// To allow to view all available results for one row (When double-clicking on it) in the 'RViewSSelBv' view.
	CIndSelBCVParams m_clIndSelBCVParams;
	
	CButton m_clCheckboxDp;
	CButton m_clCheckboxStrict;
	CExtNumEditComboBox m_Combo2w3w;
	CExtNumEditComboBox m_ComboCtrlType;

	// VARIABLES FOR BATCH SELECTION.
	typedef enum BS_BCVMainHeaderID
	{
		BS_BCVHMID_First = BS_MainHeaderID::BS_MHID_Last,
		BS_BCVMHID_ValveActuator = BS_BCVHMID_First,
		BS_BCVMHID_Last
	};

	typedef enum BS_BCVMainHeaderSubID
	{
		BS_BCVMHSUBID_Valve,
		BS_BCVMHSUBID_Actuator,
		BS_BCVMHSUBID_Last
	};

	typedef enum BS_BCVColumnID
	{
		BS_BCVCID_First = BS_ColumnID::BS_CID_Input_Last,
 		BS_BCVCID_Name = BS_BCVCID_First,
 		BS_BCVCID_Material,
 		BS_BCVCID_Connection,
 		BS_BCVCID_Version,
		BS_BCVCID_PN,
		BS_BCVCID_Size,
 		BS_BCVCID_Rangeability,
 		BS_BCVCID_LeakageRate,
 		BS_BCVCID_Stroke,
		BS_BCVCID_ImgCharacteristic,
		BS_BCVCID_ImgPushClose,
 		BS_BCVCID_Preset,
		BS_BCVCID_Dp,
		BS_BCVCID_DpFullOpening,
		BS_BCVCID_DpHalfOpening,
 		BS_BCVCID_TemperatureRange,
 		BS_BCVCID_PipeSize,
 		BS_BCVCID_PipeLinDp,
		BS_BCVCID_PipeV,
		BS_BCVCID_ActrName,
		BS_BCVCID_ActrCloseOffDp,
		BS_BCVCID_ActrMaxInletPressure,
		BS_BCVCID_ActrActuatingTime,
		BS_BCVCID_ActrIP,
		BS_BCVCID_ActrPowSupply,
		BS_BCVCID_ActrInputSig,
		BS_BCVCID_ActrOutputSig,
		BS_BCVCID_ActrFailSafe,
		BS_BCVCID_ActrDRP,
		BS_BCVCID_ActrAdapterName,
		BS_BCVCID_Last 
	};
};
