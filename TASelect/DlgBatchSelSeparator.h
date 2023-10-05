#pragma once


#include "DlgBatchSelBase.h"
#include "ProductSelectionParameters.h"

#define DLBBATCHSELSEPARATOR_ROWHEIGHT_MAINHEADER			( 12.75 )

class CDlgBatchSelSeparator : public CDlgBatchSelBase
{
public:
	enum { IDD = IDD_DLGBATCHSELSEPARATOR };
	CDlgBatchSelSeparator( CWnd *pParent = NULL );
	virtual ~CDlgBatchSelSeparator();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString(IDS_LEFTTABTITLE_WATERQUALITY); }
	virtual void SaveSelectionParameters();
	virtual bool ResetRightView( void );
	virtual void ActivateLeftTabDialog( void );

	// Remark: This method is called by the 'CDlgWaterChar' dialog to know it it can validate the fluid characteristic that
	// user has defined.
	virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar );
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
	virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct );
	virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative );
	// End of overriding 'DlgBatchSelBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	virtual afx_msg void OnComboSelChangeType();
	virtual afx_msg void OnComboSelChangeFamily();
	virtual afx_msg void OnComboSelChangeConnection();
	virtual afx_msg void OnComboSelChangeVersion();

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
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );
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
	virtual void BS_EnableCombos( bool fEnable );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage* pclBSMessage, BSRowParameters* pclRowParameters );
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
	virtual void BS_ReadAllColumnWidth( CDS_BatchSelParameter* pclBatchSelParameter );
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
	void _FillComboBoxType( CString strTypeID = _T("") );
	void _FillComboBoxFamily( CString strFamilyID = _T("") );
	void _FillComboBoxConnect( CString strConnectID = _T("")  );
	void _FillComboBoxVersion( CString strVersionID = _T("") );

	bool _BS_ApplySelection();
	CDS_SSelAirVentSeparator *_BS_CreateSSelSeparator( BSRowParameters *pclRowParameters );
	void _BS_DisplaySolutions( void );
	void _BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID );

	// Allow to fill 'm_clIndSelSeparatorParams' with needed data from 'm_clBatchSelSeparatorParams' to show results in the 'RViewSSelSeparator' when
	// user double clicks on a batch result.
	bool _CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters );

	void _VerifyFluidCharacteristics( bool bShowErrorMsg );
	void _VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar = NULL );

// Private variables.
private:
	// To make batch selection.
	CBatchSelSeparatorParams m_clBatchSelSeparatorParams;

	// To allow to view all available results for one row (When double-clicking on it) in the 'RViewSSelSeparator' view.
	CIndSelSeparatorParams m_clIndSelSeparatorParams;

	CExtNumEditComboBox m_ComboType;
	CExtNumEditComboBox m_ComboFamily;
	CExtNumEditComboBox m_ComboConnect;
	CExtNumEditComboBox m_ComboVersion;

	// VARIABLES FOR BATCH SELECTION.
	typedef enum BS_SeparatorColumnID
	{
		BS_SEPARATORCID_First = BS_ColumnID::BS_CID_Input_Last,
		BS_SEPARATORCID_Name = BS_SEPARATORCID_First,
		BS_SEPARATORCID_Type,
		BS_SEPARATORCID_Material,
		BS_SEPARATORCID_Connection,
		BS_SEPARATORCID_Version,
		BS_SEPARATORCID_PN,
		BS_SEPARATORCID_Size,
		BS_SEPARATORCID_Dp,
		BS_SEPARATORCID_TemperatureRange,
		BS_SEPARATORCID_PipeSize,
		BS_SEPARATORCID_PipeLinDp,
		BS_SEPARATORCID_PipeV,
		BS_SEPARATORCID_Last 
	};
};
