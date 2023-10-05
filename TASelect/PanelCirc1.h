#pragma once


#include "afxcmn.h"
#include "hydromod.h"
#include "SSheetPanelCirc1.h"
#include "ExtComboBox.h"
#include "afxwin.h"
#include "SetDpi.h"
#include "XGroupBox.h"

class CPanelCirc1 : public CDlgWizard
{
public:
	enum { IDD = IDD_PANELCIRC1 };

	CPanelCirc1( CWnd *pParent = NULL );
	virtual ~CPanelCirc1();

	CDB_CircuitScheme *GetSelectedSch() { return m_pFPSheet->GetSelectedSch(); }
	CDB_CircuitScheme::eBALTYPE GetBalancingType();
	bool IsTerminalUnitChecked() { return m_pFPSheet->IsTuChecked(); }
	bool IsDirectReturn();
	bool IsCvOnOff();
	CDB_ControlProperties::CvCtrlType GetControlType();
	bool IsTACv();
	void EnableButtonNext();
	void DoNext() { GetWizMan()->DoNext(); }
	void DisableButtons( int iButton ) { GetWizMan()->DisableButtons( iButton ); }
	CSSheetPanelCirc1 *GetFPSpreadSheet() { return m_pFPSheet; }

	// To avoid to make 'OnWizNext' public.
	BOOL GoWizNext() { return OnWizNext(); }

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnSize( CRect rectParentSize );
	afx_msg void OnClickedRadioBalancingTypeNoBalance();
	afx_msg void OnClickedRadioBalancingTypeManualBalancing();
	afx_msg void OnClickedRadioBalancingTypeDiffPressController();
	afx_msg void OnClickedRadioBalancingTypeElectronicBalancing();
	afx_msg void OnClickedRadioBalancingTypeElectronicDpC();
	afx_msg void OnClickedRadioTUNone();
	afx_msg void OnClickedRadioTUTerminalUnit();
	afx_msg void OnClickedRadioTURadiator();
	afx_msg void OnClickedRadioReturnTypeDirect();
	afx_msg void OnClickedRadioReturnTypeReverse();
	afx_msg void OnClickedRadioWorkWithControlValvesKvsValues();
	afx_msg void OnClickedRadioWorkWithControlValvesIMIValves();
	afx_msg void OnClickedRadioControlTypeNone();
	afx_msg void OnClickedRadioControlTypeProportional();
	afx_msg void OnClickedRadioControlTypeOnOff();
	afx_msg void OnClickedRadioControlType3Pts();
	afx_msg void OnClickedRadioControlValveTypeStandard();
	afx_msg void OnClickedRadioControlValveTypeAdjustable();
	afx_msg void OnClickedRadioControlValveTypeAdjustableAndMeasurable();
	afx_msg void OnClickedRadioControlValveTypePressureIndependent();
	afx_msg void OnClickedRadioControlValveTypeSmart();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' virtual methods.
	virtual BOOL OnWizNext();
	virtual BOOL OnWizFinish();
	virtual bool OnInitialActivate();
	virtual bool OnActivate();
	virtual bool OnAfterActivate();
	virtual bool OnEscapeKeyPressed( void ) { return ( TRUE == OnWizFinish() ) ? true : false; }
	virtual int OnWizButFinishTextID() { return IDS_CANCEL; }
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UpdateCVTypeGroup();
	// Update balancing group regarding control type group
	void UpdateBalancingTypeGroup();
	void InitSpreadSheet();
	void VerifyTerminalUnitGroup();
	void VerifyRadiosReturnType();
	void verifyRadioControlValveType();
	void ClickRadioReturnTypeDirect();
	void ClickRadioReturnTypeReverse();
	void ClickRadioWorkWithControlValvesKvsValues();
	void ClickRadioWorkWithControlValvesIMIValves();
	void ClickRadioControlTypeNone();
	void ClickRadioControlTypeProportional();
	void ClickRadioControlTypeOnOff();
	void ClickRadioControlType3points();
	void ClickRadioControlValveTypeStandard();
	void ClickRadioControlValveTypeAdjustable();
	void ClickRadioControlValveTypeAdjustableAndMeasurable();
	void ClickRadioControlValveTypePressureIndependent();
	void ClickRadioControlValveTypeSmart();

protected:
	bool m_bEditDoNext;
	bool m_bOnActivateInCourse;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CTable *m_pInitialTab;
	CDS_HydroMod *m_pHM;
	CSSheetPanelCirc1 *m_pFPSheet;
	CButton m_RadioBalancingTypeNone;
	CButton m_RadioBalancingTypeManual;
	CButton m_RadioBalancingTypeDiffPress;
	CButton m_RadioBalancingTypeElectronic;
	CButton m_RadioBalancingTypeElectronicDpC;
	CButton m_RadioTUNone;
	CButton m_RadioTUTerminalUnit;
	CButton m_RadioTURadiator;
	CButton m_RadioReturnTypeDirect;
	CButton m_RadioReturnTypeReverse;
	CButton m_RadioControlValveTypeStandard;
	CButton m_RadioControlValveTypeAdjustable;
	CButton m_RadioControlValveTypeAdjustableMeasurable;
	CButton m_RadioControlValveTypePressureIndependent;
	CButton m_RadioControlValveTypeSmart;
	CButton m_RadioWorkForControlValvesKvsValues;
	CButton m_RadioWorkForControlValvesIMIValves;
	CButton m_RadioControlTypeNone;
	CButton m_RadioControlTypeProportional;
	CButton m_RadioControlTypeOnOFF;
	CButton m_RadioControlType3Points;
	CSetDPI dpi;
	CXGroupBox m_GroupReturnType;
	CXGroupBox m_GroupTerminalUnit;
	CXGroupBox m_GroupBalancingType;
	CXGroupBox m_GroupWorkForControlValves;
	CXGroupBox m_GroupControlType;
	CXGroupBox m_GroupControlValveType;
	UINT_PTR m_nTimer;
};
