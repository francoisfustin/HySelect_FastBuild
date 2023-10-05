#pragma once


#include "DlgBatchSelBase.h"
#include "ProductSelectionParameters.h"

#define DLBBATCHSELSMARTCONTROLVALVE_ROWHEIGHT_MAINHEADER			( 12.75 )

class CDlgBatchSelSmartControlValve : public CDlgBatchSelBase
{
public:
	enum { IDD = IDD_DLGBATCHSELSMARTCONTROLVALVE };
	CDlgBatchSelSmartControlValve( CWnd *pParent = NULL );
	virtual ~CDlgBatchSelSmartControlValve();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_SMARTCONTROLVALVE ); }
	virtual void SaveSelectionParameters();
	virtual bool ResetRightView( void );
	virtual void ActivateLeftTabDialog( void );
	// End of overriding 'DlgSelectionBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgBatchSelBase' public virtual methods.
	virtual CExtNumEditComboBox *GetpComboTypeBelow65() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboFamilyBelow65() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboVersionBelow65()	{ return NULL; }
	virtual CExtNumEditComboBox *GetpComboTypeAbove50() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboFamilyAbove50() { return NULL; }
	virtual CExtNumEditComboBox *GetpComboVersionAbove50()	{ return NULL; }
	virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct );
	virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative );
	// End of overriding 'DlgBatchSelBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual BOOL OnInitDialog();

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

	virtual void EnableSuggestButton( bool bEnable );

	virtual void BS_PrepareColumnList( void );
	virtual bool BS_InitColumns( void );
	virtual bool BS_Validate( void );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage* pclBSMessage, BSRowParameters* pclRowParameters );
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
	virtual void BS_ReadAllColumnWidth( CDS_BatchSelParameter* pclBatchSelParameter );
	virtual void BS_WriteAllColumnWidth( CDS_BatchSelParameter *pclBatchSelParameter );
	
	virtual void GetMaterialList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetConnectList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	virtual void GetPNList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow );
	
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
	CDS_SSelSmartControlValve *_BS_CreateSmartControlValve( BSRowParameters *pclRowParameters );
	void _BS_DisplaySolutions( void );
	void _BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID );

	// Allow to fill 'm_clIndSelSmartControlValveParams' with needed data from 'm_clBatchSelSmartControlValveParams' to show results in the 'RViewSSelSmartControlValve' when
	// user double clicks on a batch result.
	bool _CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters );

// Private variables.
private:
	// To make batch selection.
	CBatchSelSmartControlValveParams m_clBatchSelSmartControlValveParams;

	// To allow to view all available results for one row (When double-clicking on it) in the 'RViewSSelSmartControlValve' view.
	CIndSelSmartControlValveParams m_clIndSelSmartControlValveParams;

	// VARIABLES FOR BATCH SELECTION.
	typedef enum BS_SmartControlValveColumnID
	{
		BS_SMARTCONTROLVALVECID_First = BS_ColumnID::BS_CID_Input_Last,
		BS_SMARTCONTROLVALVECID_Name = BS_SMARTCONTROLVALVECID_First,
		BS_SMARTCONTROLVALVECID_Material,
		BS_SMARTCONTROLVALVECID_Connection,
		BS_SMARTCONTROLVALVECID_PN,
		BS_SMARTCONTROLVALVECID_Size,
		BS_SMARTCONTROLVALVECID_DpMin,
		BS_SMARTCONTROLVALVECID_TemperatureRange,
		BS_SMARTCONTROLVALVECID_PipeSize,
		BS_SMARTCONTROLVALVECID_PipeLinDp,
		BS_SMARTCONTROLVALVECID_PipeV,
		BS_SMARTCONTROLVALVECID_Last 
	};
};
