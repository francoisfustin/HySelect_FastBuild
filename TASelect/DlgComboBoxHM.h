#pragma once


#include "DlgSpreadCB.h"

class CDlgComboBoxHM :public CDlgSpreadCB
{
	DECLARE_DYNAMIC(CDlgComboBoxHM)

public:
	enum eSCB
	{
		SCB_CircuitPrimaryPipeSerie,
		SCB_CircuitPrimaryPipeSize,
		SCB_CircuitSecondaryPipeSerie,
		SCB_CircuitSecondaryPipeSize,
		SCB_CircuitBypassPipeSerie,
		SCB_CircuitBypassPipeSize,
		SCB_DistributionSupplyPipeSerie,
		SCB_DistributionSupplyPipeSize,
		SCB_DistributionReturnPipeSerie,
		SCB_DistributionReturnPipeSize,
		SCB_BvLoc,
		SCB_BvPName,
		SCB_BvPType,
		SCB_BvPConnection,
		SCB_BvPVersion,
		SCB_BvPPN,
		SCB_BvSName,
		SCB_BvSType,
		SCB_BvSConnection,
		SCB_BvSVersion,
		SCB_BvSPN,
		SCB_BvBName,
		SCB_BvBType,
		SCB_BvBConnection,
		SCB_BvBVersion,
		SCB_BvBPN,
		SCB_CvKvs,
		SCB_Cv,
		SCB_CvLoc,
		SCB_CvConnection,
		SCB_CvVersion,
		SCB_CvPN,
		SCB_Actuator,
		SCB_DpCType,
		SCB_DpCConnection,
		SCB_DpCPN,
		SCB_DpC,
		SCB_ShutOffValve,
		SCB_ShutOffValveType,
		SCB_ShutOffValveConnection,
		SCB_ShutOffValveVersion,
		SCB_ShutOffValvePN,
		SCB_DpCBCValve,
		SCB_DpCBCValveConnection,
		SCB_DpCBCValveVersion,
		SCB_DpCBCValvePN,
		SCB_SmartControlValve,
		SCB_SmartControlValveBody,
		SCB_SmartControlValveConnection,
		SCB_SmartControlValvePN,
		SCB_SmartDpC,
		SCB_SmartDpCBody,
		SCB_SmartDpCConnection,
		SCB_SmartDpCPN,
		SCB_DS_HubValveName,				// DS for direct selection, list comes from directly from DB
		SCB_DS_HubDpCName,
		SCB_DS_StationValveType,
		SCB_DS_StationValveCtrlType,
		SCB_DS_StationValveName,
		SCB_DS_StationActuatorType,
		SCB_DS_ShutOffValve,
		SCB_TU_Flow,
		SCB_TU_Dp,
		SCB_Last
	};

	enum { IDD = IDD_DLGSPREADCB };

// Public members.
public:
	CDlgComboBoxHM( CSSheet *pOwnerSSheet, CWaterChar *pWC = NULL );
	virtual ~CDlgComboBoxHM( void );

	void OpenDialogSCB( CDS_HydroMod *pHM, eSCB eSCBtype, long lColumn, long lRow );
	eSCB GetSCBType() { return m_eSCB; }
	CDS_HydroMod *GetSelectedHM() { return m_pHM; }

	// This method allow to retrieve what is the LParam linked to the user choice.
	// Remark: LParam is set when calling 'm_pCB->SetRowParameters()' method.
	LPARAM GetLParamSelected( void );

	// Initialize list of Hub object, needed for direct selection 
	void InitList( CDS_HydroMod *pHM, eSCB eSCBtype );
	bool GetFirstListItem( CString &str, LPARAM &itemdata );
	void Close();
	
// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	virtual void Refresh();

	afx_msg void OnPaint();
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );

	// Spread DLL message handlers.
	// Remark: Why implementing these methods in the 'CDlgComboBoxHM' class and not directly in the 'CSpreadComboBox' class?
	//         When the '_TSpread' class sends the 'LeaveCell' event, this one is directly sent to the 'CSpreadComboBox' class
	//         and also to its parent (here the 'CDlComboBoxHM' class). If we implement handlers only in the 'CSpreadComboBox', 
	//		   class this, this one will return 'TRUE' or 'FALSE' in regards to its need. After that, the same event is sent
	//         to the owner of the 'CSpreadComboBox' class that is the 'CDlgComboBoxHM' class. By default, if there is no 
	//         handler in the 'CDlgComboBoxHM' class, the return value will automatically be set to 'FALSE' and then will
	//         overwrite the previous value set by the 'CSpreadComboBox' class. This is why we voluntary don't catch
	//         '_TSpread' event in the 'CSpreadComboBox' class, but we do it only in 'CDlgComboBoxHM'.
	afx_msg LRESULT ClickFpspread( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT DblClickFpspread( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT LeaveCellFpspread( WPARAM wParam, LPARAM lParam );

// Private methods.
private:
	void _CloseDialogSCB();
	void _ShowCBPipeSize();
	void _ShowCBPipeSerie();
	void _ShowCBBvLoc( CDS_HydroMod *pHM );
	void _ShowCBBvName( CDS_HydroMod::CBV *pHMBV );
	void _ShowCBBvType( CDS_HydroMod::CBV *pHMBV );
	void _ShowCBBvConnect( CDS_HydroMod::CBV *pHMBV );
	void _ShowCBBvVersion( CDS_HydroMod::CBV *pHMBV );
	void _ShowCBBvPN( CDS_HydroMod::CBV *pHMBV );
	void _ShowCBDpC();
	void _ShowCBDpCType();
	void _ShowCBDpCConnect();
	void _ShowCBDpCPN();
	void _ShowCBShutOffValve();
	void _ShowCBShutOffValveType();
	void _ShowCBShutOffValveConnect();
	void _ShowCBShutOffValveVersion();
	void _ShowCBShutOffValvePN();
	void _ShowCBDpCBCValve();
	void _ShowCBDpCBCValveConnect();
	void _ShowCBDpCBCValveVersion();
	void _ShowCBDpCBCValvePN();
	void _ShowCBSmartControlValve();
	void _ShowCBSmartControlValveBodyMaterial();
	void _ShowCBSmartControlValveConnect();
	void _ShowCBSmartControlValvePN();
	void _ShowCBSmartDpC();
	void _ShowCBSmartDpCBodyMaterial();
	void _ShowCBSmartDpCConnect();
	void _ShowCBSmartDpCPN();

	long _FillRowCv( CDB_RegulatingValve *pRV, bool bKvs = false, bool bForSet = false );
	void _ShowCBCvLoc( CDS_HydroMod *pHM );
	void _ShowCBCv();
	void _ShowCBCvKvs();
	void _ShowCBCvConnect();
	void _ShowCBCvVersion();
	void _ShowCBCvPN();
	void _ShowCBTUFlow();
	void _ShowCBTUDp();

	void _ShowList( long lColumn, long lRow );
	void _HideUnnecessaryCol( long lFromColumn, long lToColumn );
	void _InitListStationActuatorType();
	void _InitListStationValveType();
	void _InitListStationValveCtrlType();
	void _InitListStationValveName();
	void _InitListHubValveName();
	void _InitListHubDpCName();
	void _InitListShutOffValve();

// Private variables.
protected:
	eSCB m_eSCB;
	CRank *m_pRankList;
	CSpreadComboBox *m_pCB;
};
