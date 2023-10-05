#pragma once


#include "DlgBatchSelBase.h"
#include "ProductSelectionParameters.h"

class CDlgBatchSelBv : public CDlgBatchSelBase
{
public:
	enum { IDD = IDD_DLGBATCHSELBV };
	CDlgBatchSelBv( CWnd *pParent = NULL );
	virtual ~CDlgBatchSelBv() {}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_REGULATINGVALVE ); }
	virtual void SaveSelectionParameters();
	virtual bool ResetRightView( void );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgBatchSelBase' public virtual methods.
	virtual bool OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList );
	virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &fCatExist, CData *&pclProduct );
	virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative );
	// End of overriding 'DlgBatchSelBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnBnClickedCheckDp();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelBase' protected afx_msg methods.
	virtual afx_msg void OnBnClickedSuggest();
	virtual afx_msg void OnBnClickedValidate();
	virtual afx_msg void OnBnClickedCancel();
	// HYS-1263 : Added to update application type if it is necessary
	virtual void ActivateLeftTabDialog();
	// End of overriding 'CDlgIndSelBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgSelectionBase' protected afx_msg methods.
	virtual afx_msg LRESULT OnNewDocument( WPARAM wParam = 0, LPARAM lParam = 0 );

	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgBatchSelBase' protected methods.
	virtual CButton *GetpCheckboxDp() const { return ( (CButton*)GetDlgItem( IDC_CHECKDP ) ); }
	virtual CButton *GetpBtnSuggest() const { return ( (CButton*)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnValidate() const { return ( (CButton*)GetDlgItem( IDC_BUTTONVALIDATE ) ); }

	virtual void BS_PrepareColumnList( void );
	virtual bool BS_InitColumns( void );
	virtual bool BS_Validate( void );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters );
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
	virtual void BS_UpdateIOColumnLayout( int iUpdateWhat );
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
	CDS_SSelBv *_BS_CreateSSelBv( BSRowParameters *pclRowParameters );
	void _BS_DisplaySolutions( void );
	void _BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID );

	// Allow to fill 'm_clIndSelBVParams' with needed data from 'm_clBatchSelBVParams' to show results in the 'RViewSSelBv' when
	// user double clicks on a batch result.
	bool _CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters );

// Private variables.
private:
	// To make batch selection.
	CBatchSelBVParams m_clBatchSelBVParams;

	// To allow to view all available results for one row (When double-clicking on it) in the 'RViewSSelBv' view.
	CIndSelBVParams m_clIndSelBVParams;

	CButton	m_clCheckboxDp;

	// VARIABLES FOR BATCH SELECTION.
	typedef enum BS_BVColumnID
	{
		BS_BVCID_First = BS_ColumnID::BS_CID_Input_Last,
		BS_BVCID_Name = BS_BVCID_First,
		BS_BVCID_Material,
		BS_BVCID_Connection,
		BS_BVCID_Version,
		BS_BVCID_PN,
		BS_BVCID_Size,
		BS_BVCID_Preset,
		BS_BVCID_DpSignal,
		BS_BVCID_Dp,
		BS_BVCID_DpFullOpening,
		BS_BVCID_DpHalfOpening,
		BS_BVCID_TemperatureRange,
		BS_BVCID_PipeSize,
		BS_BVCID_PipeLinDp,
		BS_BVCID_PipeV,
		BS_BVCID_Last 
	};
};
