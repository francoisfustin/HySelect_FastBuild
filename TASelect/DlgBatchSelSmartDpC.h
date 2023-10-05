#pragma once


#include "DlgBatchSelBase.h"
#include "ProductSelectionParameters.h"

#define DLBBATCHSELSMARTDPC_ROWHEIGHT_MAINHEADER			( 12.75 )

class CDlgBatchSelSmartDpC : public CDlgBatchSelBase
{
public:
	enum { IDD = IDD_DLGBATCHSELSMARTDPC };
	CDlgBatchSelSmartDpC( CWnd *pParent = NULL );
	virtual ~CDlgBatchSelSmartDpC();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionBase' public virtual methods.
	virtual CString GetDlgCaption() { return TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_SMARTDPC ); }
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
	
	virtual bool OnBatchSelectionGetColumnList( CDlgOutput::mapColData &mapColumnList );
	virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct );
	virtual bool OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative );
	// End of overriding 'DlgBatchSelBase' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	virtual afx_msg void OnBnClickedCheckDpBranch();
	virtual afx_msg void OnBnClickedCheckDpMax();
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
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	// End of overriding 'CDlgSelectionBase' protected afx_msg methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgBatchSelBase' protected methods.
	virtual CButton *GetpBtnSuggest() const { return ( (CButton*)GetDlgItem( IDC_BUTTONSUGGEST ) ); }
	virtual CButton *GetpBtnValidate() const { return ( (CButton*)GetDlgItem( IDC_BUTTONVALIDATE ) ); }

	virtual void EnableSuggestButton( bool bEnable );

	virtual void BS_PrepareColumnList( void );
	virtual bool BS_InitMainHeaders( void );
	virtual bool BS_InitColumns( void );
	virtual bool BS_Validate( void );
	virtual void BS_EnableRadios( bool bEnable );
	virtual void BS_PrepareOutput( CDlgBatchSelectionOutput::CBSMessage* pclBSMessage, BSRowParameters* pclRowParameters );
	virtual bool BS_SuggestSelection( void );
	virtual bool BS_AddOneColumn( int iColumnID );
	virtual void BS_UpdateInputColumnLayout( int iUpdateWhat );
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

	typedef enum BS_SmartDpCUpdateInputColumn
	{
		BS_SmartDpCUIC_DpBranch = BS_UpdateInputColumn::BS_UIC_Last,
		BS_SmartDpCUIC_DpMax,
		BS_SmartDpCUIC_Last
	};

// Private methods.
private:
	bool _BS_ApplySelection();
	CDS_SSelSmartDpC *_BS_CreateSmartDpC( BSRowParameters *pclRowParameters );
	void _BS_DisplaySolutions( void );
	void _BS_FillCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID );

	void _UpdateDpBranchCheckboxState();
	void _UpdateDpBranchColumn();

	void _UpdateDpMaxFieldState();
	void _UpdateDpMaxColumn();

	// Allow to fill 'm_clIndSelSmartDpCParams' with needed data from 'm_clBatchSelSmartDpCParams' to show results in the 'RViewSSelSmartDpC' when
	// user double clicks on a batch result.
	bool _CopyBatchSelParamsToIndSelParams( BSRowParameters *pclRowParameters );

// Private variables.
private:
	// To make batch selection.
	CBatchSelSmartDpCParams m_clBatchSelSmartDpCParams;

	// To allow to view all available results for one row (When double-clicking on it) in the 'RViewSSelSmartDpC' view.
	CIndSelSmartDpCParams m_clIndSelSmartDpCParams;

	CButton	m_clCheckboxDpBranch;
	CButton m_clCheckboxDpMax;
	CExtNumEdit m_clExtEditDpMax;

	typedef enum BS_SmartDpCMainHeaderID
	{
		BS_SMARTDPCHMID_First = BS_MainHeaderID::BS_MHID_Last,
		BS_SMARTDPCMHID_SmartDpCDpSensor = BS_SMARTDPCHMID_First,
		BS_SMARTDPCMHID_Last
	};

	typedef enum BS_SmartDpCMainHeaderSubID
	{
		BS_SMARTDPCMHSUBID_SmartDpC,
		BS_SMARTDPCMHSUBID_DpSensor,
		BS_SMARTDPCMHSUBID_Last
	};

	typedef enum BS_SmartDpCColumnID
	{
		BS_SMARTDPCCID_First = BS_ColumnID::BS_CID_Input_Last,
		BS_SMARTDPCCID_Name = BS_SMARTDPCCID_First,
		BS_SMARTDPCCID_Material,
		BS_SMARTDPCCID_Connection,
		BS_SMARTDPCCID_PN,
		BS_SMARTDPCCID_Size,
		BS_SMARTDPCCID_Kvs,
		BS_SMARTDPCCID_Qnom,
		BS_SMARTDPCCID_DpMin,
		BS_SMARTDPCCID_DpMax,
		BS_SMARTDPCCID_TemperatureRange,
		BS_SMARTDPCCID_PipeSize,
		BS_SMARTDPCCID_PipeLinDp,
		BS_SMARTDPCCID_PipeV,
		BS_SMARTDPCCID_DpSensorName,
		BS_SMARTDPCCID_DpSensorDpl,
		BS_SMARTDPCCID_DpSensorBurstPressure,
		BS_SMARTDPCCID_DpSensorTemperatureRange,
		BS_SMARTDPCCID_Last 
	};
};
