#pragma once


#include "DlgSelectionComboHelper.h"

#define DLBBATCHSELPICV_ROWHEIGHT_MAINHEADER			( 12.75 )

class CDlgBatchSelPICv : public CDlgBatchSelCtrlBase
{
public:
	enum { IDD = IDD_DLGBATCHSELPICV };
	CDlgBatchSelPICv( CWnd *pParent = NULL );
	virtual ~CDlgBatchSelPICv();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString(IDS_LEFTTABTITLE_PIBCV); }
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
	
	// HYS-1458 : Rename column
	virtual void RenameColHeader();
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnCbnSelChangeCtrlType();
	afx_msg void OnBnClickedCheckStrict();

	////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgBatchSelBase' protected virtual methods.
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
	////////////////////////////////////////////////////////////////////////

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
	virtual CButton *GetpBtnSuggest() const { return ( (CButton*)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnValidate() const { return ( (CButton*)GetDlgItem( IDC_BUTTONVALIDATE ) ); }

	virtual void BS_PrepareColumnList( void );
	virtual bool BS_InitMainHeaders( void );
	virtual bool BS_InitColumns( void );
	virtual bool BS_Validate( void );
	virtual void BS_EnableCombos( bool bEnable );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters );
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
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

// Private methods.
private:
	bool _BS_ApplySelection();
	CDS_SSelPICv *_BS_CreateSSelPICv( BSRowParameters *pclRowParameters );
	void _BS_DisplaySolutions();
	void _BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID );

	// Allow to fill 'm_clIndSelPIBCVParams' with needed data from 'm_clBatchSelPIBCVParams' to show results in the 'RViewSSelPICV' when
	// user double clicks on a batch result.
	bool _CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters );

// Private variables.
private:
	// To make batch selection.
	CBatchSelPIBCVParams m_clBatchSelPIBCVParams;

	// To allow to view all available results for one row (When double-clicking on it) in the 'RViewSSelPICV' view.
	CIndSelPIBCVParams m_clIndSelPIBCVParams;

	CDlgSelComboHelperPIBCV *m_pclSelComboHelperPIBCV;

	CExtNumEditComboBox m_ComboCtrlType;
	CButton m_clCheckboxStrict;

	// VARIABLES FOR BATCH SELECTION.
	typedef enum BS_PICVMainHeaderID
	{
		BS_PICVHMID_First = BS_MainHeaderID::BS_MHID_Last,
		BS_PICVMHID_ValveActuator = BS_PICVHMID_First,
		BS_PICVMHID_Last
	};

	typedef enum BS_PICVMainHeaderSubID
	{
		BS_PICVMHSUBID_Valve,
		BS_PICVMHSUBID_Actuator,
		BS_PICVMHSUBID_Last
	};

	typedef enum BS_PICVColumnID
	{
		BS_PICVCID_First = BS_ColumnID::BS_CID_Input_Last,
		BS_PICVCID_Name = BS_PICVCID_First,
		BS_PICVCID_FC,
		BS_PICVCID_Material,
		BS_PICVCID_Connection,
		BS_PICVCID_Version,
		BS_PICVCID_PN,
		BS_PICVCID_Size,
		BS_PICVCID_Rangeability,
		BS_PICVCID_LeakageRate,
		BS_PICVCID_Stroke,
		BS_PICVCID_ImgCharacteristic,
		BS_PICVCID_ImgPushClose,
		BS_PICVCID_Preset,
		BS_PICVCID_DpMin,
		BS_PICVCID_TemperatureRange,
		BS_PICVCID_PipeSize,
		BS_PICVCID_PipeLinDp,
		BS_PICVCID_PipeV,
		BS_PICVCID_ActrName,
		BS_PICVCID_ActrCloseOffDp,
		BS_PICVCID_ActrMaxInletPressure,
		BS_PICVCID_ActrActuatingTime,
		BS_PICVCID_ActrIP,
		BS_PICVCID_ActrPowSupply,
		BS_PICVCID_ActrInputSig,
		BS_PICVCID_ActrOutputSig,
		BS_PICVCID_ActrFailSafe,
		BS_PICVCID_ActrDRP,
		BS_PICVCID_ActrAdapterName,
		BS_PICVCID_Last
	};
};
