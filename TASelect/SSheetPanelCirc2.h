#pragma once


#include "SSheet.h"
#include "Select.h"
#include "TerminalUnit.h"
#include "MessageManager.h"

#define  _BoxType CDB_CircuitScheme::eBoxes

#define  PC2_COLWIDTH_FIXED				1
#define  PC2_COLWIDTH_LOCK				3
#define  PC2_COLWIDTH_DESCRIPTION		13
#define  PC2_COLWIDTH_VALUE				15
#define  PC2_COLWIDTH_UNIT				6
#define  PC2_COLUMPROPERTY_FIXED		(0x2000)

class CDlgComboBoxHM; 
class CDlgSelectActuator;
class CTermUnit;

class CSSheetPanelCirc2 : public CSSheet
{
protected:
	// Allow to save type of variable linked to the cell.
	enum _BoxTU
	{
		Title,
		Des,
		Flow,
		Power,
		DeltaT,
		SupplyT,
		ReturnT,
		Dp,
		Qref,
		DpRef,
		Kv,
		Cv,
		SmartControlValve
	};

	enum eBCol
	{
		Butt = -1,
		Lock = 0,
		Desc = 1,
		Val = 2,
		Unit = 3
	};

	typedef struct _sBox
	{
		int	nRows; //= 38,
		int	nCols; //= 15,

		int	TU_Top; //= 3,				// Terminal Unit
		int	TU_nRows; //= 9,
		int	TU_Des; //= 1,
		int	TU_Flow; //= 2,
		int		TU_Power; //= 1,
		int		TU_DeltaT; //= 2,
		int	TU_Dp; //= 3,
		int		TU_QRef; //= 1,
		int		TU_DpRef; //= 2,

		int	Pipe_Top; //= 13,			// Pipe
		int	Pipe_Left; //= 2,
		int	Pipe_nRows; //= 6,
		int	Pipe_Serie; //= 1,
		int	Pipe_Size; //= 2,
		int	Pipe_Length; //= 3,
		int	Pipe_Dp; //= 4,
		int	Pipe_Acc; //= 5,

		int	DpC_Top; //= 20,			// Dp Controller
		int	DpC_Left; //= 2,
		int	DpC_nRows; //= 9,
		int	DpC_Name; //= 1,
		int		DpC_Type; //= 1,
		int		DpC_Connect; //= 2,
		int		DpC_PN; //=3,
		int	DpC_Setting; //= 2,
		int	DpC_Dp; //= 3,
		int	DpC_DpL; //= 4,

		int	PrimaryValues_Top; //= 27,			// Primary values
		int	PrimaryValues_Left; //= 7,
		int	PrimaryValues_nRows; //= 3,
		int	PrimaryValues_Flow; //= 1,
		int	PrimaryValues_Hmin; //= 2,
		int	PrimaryValues_AvailH; //= 3,
		int	PrimaryValues_Tsp; //= 4,		Circuit supply temperature at the primary side
		int	PrimaryValues_Trp; //= 5,		Circuit return temperature at the primary side

		int	CVKV_Top; //= 10,			// control valve KV Mode
		int	CVKV_Left; //= 12,
		int	CVKV_nRows; //= 6,
		int CV_Loc; // = 1	// For Double mixing 3Winj circuit 
		int	CVKV_Des; //= 1,
		int	CVKV_Kvsmax; //= 2,
		int	CVKV_Kvs; //= 3,
		int	CVKV_Dp; //= 4,
		int	CVKV_Auth; //= 5,

		int	CVTA_Top; //= 10,			// control valve TA Mode
		int	CVTA_Left; //= 12,
		int	CVTA_nRows; //= 8,
		//int CV_Loc; // = 1	// For Double mixing 3Winj circuit 
		int	CVTA_Name; //= 1,
		int		CVTA_Conn; //= 1,
		int		CVTA_Vers; //= 2,
		int		CVTA_PN; //= 3,		
		int	CVTA_Preset; //= 1,
		int	CVTA_Dp; //= 2,
		int	CVTA_Auth; //= 3,
		int CVTA_Actuator;//=4

		int	Rem_Top; //= 30,			// Remark
		int	Rem_Left; //= 2,
		int	Rem_nRows; //= 2,
		int	Rem_l1; //= 1,

		int	Pic_Top; //= 1,
		int	Pic_Left; //= 6,
		int	Pic_nRows; //= 28,
		int	Pic_nCols; //= 5
		struct _sBox()
		{
			Reset();
		}
		void Reset()
		{
			nRows = 0;
			nCols = 0;
			TU_Top = 0;
			TU_nRows = 0;
			TU_Des = 0;
			TU_Flow = 0;
			TU_Power = 0;
			TU_DeltaT = 0;
			TU_Dp = 0;
			TU_QRef = 0;
			TU_DpRef = 0;
			Pipe_Top = 0;
			Pipe_Left = 0;
			Pipe_nRows = 0;
			Pipe_Serie = 0;
			Pipe_Size = 0;
			Pipe_Length = 0;
			Pipe_Dp = 0;
			Pipe_Acc = 0;
			DpC_Top = 0;
			DpC_Left = 0;
			DpC_nRows = 0;
			DpC_Name = 0;
			DpC_Type = 0;
			DpC_Connect = 0;
			DpC_PN = 0;
			DpC_Setting = 0;
			DpC_Dp = 0;
			DpC_DpL = 0;
			PrimaryValues_Top = 0;
			PrimaryValues_Left = 0;
			PrimaryValues_nRows = 0;
			PrimaryValues_Flow = 0;
			PrimaryValues_Hmin = 0;
			PrimaryValues_AvailH = 0;
			CVKV_Top = 0;
			CVKV_Left = 0;
			CVKV_nRows = 0;
			CV_Loc = 0;
			CVKV_Des = 0;
			CVKV_Kvsmax = 0;
			CVKV_Kvs = 0;
			CVKV_Dp = 0;
			CVKV_Auth = 0;
			CVTA_Top = 0;
			CVTA_Left = 0;
			CVTA_nRows = 0;
			CV_Loc = 0;
			CVTA_Name = 0;
			CVTA_Conn = 0;
			CVTA_Vers = 0;
			CVTA_PN = 0;
			CVTA_Preset = 0;
			CVTA_Dp = 0;
			CVTA_Auth = 0;
			CVTA_Actuator = 0;
			Rem_Top = 0;
			Rem_Left = 0;
			Rem_nRows = 0;
			Rem_l1 = 0;
			Pic_Top = 0;
			Pic_Left = 0;
			Pic_nRows = 0;
			Pic_nCols = 0;
		}
	}; 
	_sBox m_Box;

	CDlgComboBoxHM *m_pHMSCB;
	CDlgSelectActuator *m_pHMSelectActuator;

	// Box position (top and left)
	class sBox
	{
	public:
		sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox ) { m_eBoxType = eBoxType; ResetPos(); }
		void ResetPos() { pos = CPoint( 0, 0 ); }
		CPoint pos;
		CDB_CircuitScheme::eBoxes m_eBoxType;
	};
	sBox m_BoxPrimaryValues;
	sBox m_BoxRem;
	sBox m_BoxPic;
	
	class sBoxDpC : public sBox
	{
	public:
		sBoxDpC() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox ) { sBox::Reset( eBoxType ); bExt = false; }
		bool bExt;
	};
	sBoxDpC m_BoxDpC;

	class sBoxTu : public sBox
	{
	public:
		sBoxTu() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			m_iDpRow = 0;
			m_eQType = CTermUnit::_QType::Undefined;
			m_eDPType = CDS_HydroMod::eDpType::None;
		}
		int m_iDpRow;
		CTermUnit::_QType m_eQType;
		CDS_HydroMod::eDpType m_eDPType;
	};
	sBoxTu m_BoxTu;

	class sBoxCv : public sBox
	{
	public:
		sBoxCv() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			bExt = false;
			rPreSet = 0;
			rDp = 0;
			rAuth = 0;
			rActuator = 0;
			bKvEditMode = false;
			bLocalisationExist = false;
		}
		bool bExt;
		int rPreSet;
		int rDp;
		int rAuth;
		int rActuator;
		bool bKvEditMode;
		bool bLocalisationExist;
	};
	sBoxCv m_BoxCv;

#define BV_NROWS	9
	class sBoxBv : public sBox
	{
	public:
		sBoxBv() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			bExt = false;
			bDpSignalExist = false;
			bMeasuringValve = false;
			BV_nRows = 0;
			BV_Loc = 0;
			BV_Name = 0;
			BV_OverFlow = 0;
			BV_Type = 0;
			BV_Connect = 0;
			BV_Version = 0;
			BV_PN = 0;
			BV_Setting = 0;
			BV_Dp = 0;
			BV_DpSignal = 0;
		}
		bool bExt;
		bool bDpSignalExist;
		bool bMeasuringValve;	
		int	BV_nRows; //= 9,
		int BV_Loc;//=1,
		int	BV_Name; //= 1,
		int	BV_OverFlow; //= 1,
		int		BV_Type; //= 1,		// Relative position from name
		int		BV_Connect; //= 2,
		int		BV_Version; //= 3,
		int		BV_PN; //= 4,
		int	BV_Setting; //= 2,
		int	BV_Dp; //= 3,
		int	BV_DpSignal; //= 4,
	};
	sBoxBv m_BoxBvP;
	sBoxBv m_BoxBvB;
	sBoxBv m_BoxBvS;					// Box BV primary, bypass and secondary

#define PUMPNROWS 3
	class sBoxPump : public sBox
	{
	public:
		sBoxPump() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			nRows = 0;
			iHAvailable = 0;
			iHmin = 0;
			iUserHead = 0;
			iFlow = 0;
		}
		int	nRows; //= 3 or 4,
		int iHAvailable;			// It's the H available for the children (For auto-adaptive with decouypling bypass circuit,
									// it's the Hmin + Hbypass).
		int iHmin;
		int iUserHead;
		int iFlow;
	};
	sBoxPump m_BoxPump;										// Box Pump

// HYS-1716: we add on row for the computed return temperature.
// HYS-1882: Remouve design return temperature. 
#define SECONDARYVALUESNROWS	3
	class sBoxSecondaryValues : public sBox
	{
	public:
		sBoxSecondaryValues() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			nRows = 0;
			m_iDesignSecondarySupplyTemperature = 0;
			m_iCompoutedSecondaryReturnTemperature = 0;
		}
		int	nRows; //= 2,
		int m_iDesignSecondarySupplyTemperature;
		int m_iCompoutedSecondaryReturnTemperature;
	};
	sBoxSecondaryValues m_BoxSecondaryValues;										// Box secondary values

	class sBoxPipe : public sBox
	{
	public:
		sBoxPipe() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			pPipe = NULL;
		}
		CPipes *pPipe;
	};
	sBoxPipe m_BoxCircuitPrimaryPipe;
	sBoxPipe m_BoxCircuitSecondaryPipe;
	sBoxPipe m_BoxDistributionSupplyPipe;
	sBoxPipe m_BoxDistributionReturnPipe;

#define SHUTOFFVALVENROWS 6
	class sBoxShutoffValve : public sBox
	{
	public:
		sBoxShutoffValve() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			bExt = false;
			nRows = 0;
			SV_Name = 0;
			SV_Type = 0;
			SV_Connect = 0;
			SV_Version = 0;
			SV_PN = 0;
			SV_Dp = 0;
		}
		bool bExt;
		int	nRows; //= 7,
		int	SV_Name;
		int	SV_Type;
		int	SV_Connect;
		int	SV_Version;
		int	SV_PN;
		int	SV_Dp;
	};
	sBoxShutoffValve m_BoxShutoffValve;										// Box ShutoffValve

#define DPCBCVNROWS 8
	class sBoxDpCBCV : public sBox
	{
	public:
		sBoxDpCBCV() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			bExt = false;
			nRows = 0;
			DpCBCV_Name = 0;
			DpCBCV_Connect = 0;
			DpCBCV_Version = 0;
			DpCBCV_PN = 0;
			DpCBCV_Dp = 0;
		}
		bool bExt;
		int	nRows; //= 8,
		int	DpCBCV_Name;
		int	DpCBCV_Connect;
		int	DpCBCV_Version;
		int	DpCBCV_PN;
		int	DpCBCV_Presetting;
		int	DpCBCV_Dp;
		int	DpCBCV_Actuator;
	};
	sBoxDpCBCV m_BoxDpCBCV;										// Box DpCBCV

// Dynamic in this case because we have 10 rows when the control mode is flow and we have 10 rows
// when the control mode is power.
#define SMARTCONTROLVALVENROWS 10
	class sBoxSmartControlValve : public sBox
	{
	public:
		sBoxSmartControlValve() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			bExt = false;
			eLocationChoice = SmartValveLocalization::SmartValveLocSupply;
			eControlModeChoice = SmartValveControlMode::SCVCM_Flow;
			nRows = 0;
			iName = 0;
			iBodyMaterial = 0;
			iConnect = 0;
			iPN = 0;
			iLocation = 0;
			iControlMode = 0;
			iFlowMax = 0;
			iPowerMax = 0;
			iDp = 0;
			iDpMin = 0;
		}
		bool bExt;
		SmartValveLocalization eLocationChoice;
		SmartValveControlMode eControlModeChoice;
		int	nRows; //= 10,
		int	iName;
		int	iBodyMaterial;
		int	iConnect;
		int	iPN;
		int iLocation;
		int	iControlMode;
		int	iFlowMax;
		int	iPowerMax;
		int	iDp;
		int	iDpMin;
	};
	sBoxSmartControlValve m_BoxSmartControlValve;

#define SMARTDPCNROWS 11	
	class sBoxSmartDpC : public sBox
	{
	public:
		sBoxSmartDpC() : sBox() { Reset(); }
		void Reset( CDB_CircuitScheme::eBoxes eBoxType = CDB_CircuitScheme::eBoxes::ebNoBox )
		{ 
			sBox::Reset( eBoxType );
			bExt = false;
			eLocationChoice = SmartValveLocalization::SmartValveLocSupply;
			nRows = 0;
			iName = 0;
			iBodyMaterial = 0;
			iConnect = 0;
			iPN = 0;
			iLocation = 0;
			iDp = 0;
			iDpMin = 0;
			iSets = 0;
			iDpl = 0;
		}
		bool bExt;
		SmartValveLocalization eLocationChoice;
		int	nRows; //= 8,
		int	iName;
		int	iBodyMaterial;
		int	iConnect;
		int	iPN;
		int iLocation;
		int	iDp;
		int	iDpMin;
		int iSets;
		int	iDpl;
	};
	sBoxSmartDpC m_BoxSmartDpC;

	enum InputValuesOrigin
	{
		InputValuesComeFromUser,
		InputValuesComeFromHydroMod
	};

public:
	CSSheetPanelCirc2();
	virtual ~CSSheetPanelCirc2();
	
	void Init( CDS_HydroMod *pHM, CPoint ptInitialSize = CPoint( -1, -1 ), bool bPrinting = false );	

	// RefreshDispatch is called when DP for a box is computed.
	// This function call other box compute function.
	void RefreshDispatch( CDB_CircuitScheme::eBoxes FromWho = (CDB_CircuitScheme::eBoxes)-1 );
	bool IsReady() { return m_bReady; }

	// Called when cell content change.
	void SaveAndCompute();

	void PrintThisSheet();
	virtual void CloseDialogSCB( CDialogEx *pDlgSCB, bool bModified );

	void OptimizeColumnWidth() { _ChangeColumnSize(); }

	///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	// For unit test: verify all circuit scheme positions.
	enum
	{
		Status_OK,
		Status_XOutOfRange,
		Status_YOutOfRange,
		Status_PositionBusy
	};

	bool TestHydronicSchemesBoxPosition(IDPTR &idptrFail);
	int  TestHydronicSchemesBoxPositionGetStatus() { return m_iTestHydronicSchemesBoxPositionStatus; }
	int  TestHydronicSchemesBoxPositionGetTotalCols() { return m_iTestHydronicSchemesBoxPositionCols; }
	int  TestHydronicSchemesBoxPositionGetTotalRows() { return m_iTestHydronicSchemesBoxPositionRows; }
	int  TestHydronicSchemesBoxPositionGetBoxNumber() { return m_iTestHydronicSchemesBoxPositionErrorBox; }
	CStringA  TestHydronicSchemesBoxPositionGetBoxName() { return m_mapKeyName.at( m_iTestHydronicSchemesBoxPositionErrorBox ); }
	int  TestHydronicSchemesBoxPositionGetXValue() { return m_iTestHydronicSchemesBoxPositionErrorXValue; }
	int  TestHydronicSchemesBoxPositionGetYValue() { return m_iTestHydronicSchemesBoxPositionErrorYValue; }
#endif
	///////////////////////////////////////////////////////////////////////////////////////////////////


// Protected members.
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	// Spread DLL message handlers.
	afx_msg LRESULT ComboDropDownFpspread( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT ComboSelChangeFpspread( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT EditChangeFpspread( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT EditModeOnFpspread( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT EditModeOffFpspread( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT KeyDownFpspread( WPARAM  wParam, LPARAM lParam );
	afx_msg LRESULT TextTipFetchFpspread( WPARAM wParam, LPARAM lParam );

	// 'MessageManager' message handler.
	afx_msg LRESULT MM_OnMessageReceive( WPARAM wParam, LPARAM lParam );

	void SetRedraw( bool fRedraw = true );
	enum _BoxType WhatBox( long lColumn, long lRow );
	bool IsCellCanbeEdited( long lColumn, long lRow );
	void FormatEditText( long lColumn, long lRow,  CString strText = _T(""), long lStyle = SSS_ALIGN_RIGHT, short nMaxEditLen = 64 );
	void FormatEditDouble( long lColumn, long lRow, CString strText = _T("") );

	// Draw a lock 'lock' or unlock in the center of cell col, row.
	void SetPictureLock( long lColumn, long lRow, bool fSet, COLORREF BackgroundColor = 0 );
	
	// 0 no button, 1 Lock/unLock, 2 button.
	void FormatBox( long lColumn, long lRow, int iRows, int iTitleID, COLORREF TitleColor = 0, bool fShowLockButton = false, int iCols = 4 );
	
	void FormatBoxTU( bool bRedraw = false );
	void RefreshBoxTU( InputValuesOrigin eInputValuesOrigin );
	
	void FormatBoxPipe( sBoxPipe *prBoxPipe );
	void RefreshBoxPipe( sBoxPipe *prBoxPipe, InputValuesOrigin eInputValuesOrigin );
	void ResetPipeSerie( sBoxPipe *prBoxPipe );
	void ChangePipeSize( sBoxPipe *prBoxPipe );

	void FormatBoxBV( CDS_HydroMod::eHMObj locate, bool fRedraw = true );
	sBoxBv *InitpBoxBv( CDS_HydroMod::eHMObj locate, CDS_HydroMod::CBV *pBV );
	void ChangeBvType( CDS_HydroMod::CBV *pBV );
	void ChangeBvConnect( CDS_HydroMod::CBV *pBV );
	void ChangeBvVersion( CDS_HydroMod::CBV *pBV );
	void ChangeBvPN( CDS_HydroMod::CBV *pBV );
	void ChangeBvLoc( CDS_HydroMod::CBV *pBV );
	void ChangeBv( CDS_HydroMod::CBV *pBV );

	void FormatBoxCVKV();
	void ChangeCvKvs();

	void FormatBoxCVTA();
	void RefreshBoxCV( InputValuesOrigin eInputValuesOrigin );
	void ChangeCvConnect();
	void ChangeCvVersion();
	void ChangeCvPN();
	void ChangeCv();

	void FormatBoxRemark();
	void ChangeRemark( InputValuesOrigin eInputValuesOrigin );

	void FormatBoxPrimaryValues();
	void RefreshBoxPrimaryValues();

	void FormatBoxDpC();
	void ChangeDpCType();
	void ChangeDpCConnect();
	void ChangeDpCPN();
	void ChangeDpC();

	void FormatBoxPicture(); 

	void FormatBoxPump();
	void RefreshBoxPump( InputValuesOrigin eInputValuesOrigin );

	void FormatBoxSecondaryValues();
	void RefreshBoxSecondaryValues( InputValuesOrigin eInputValuesOrigin );

	void FormatBoxSV();
	void ChangeSVType();
	void ChangeSVConnect();
	void ChangeSVVersion();
	void ChangeSVPN();
	void ChangeSV();

	void FormatBoxDpCBCV();
	void ChangeDpCBCVConnect();
	void ChangeDpCBCVVersion();
	void ChangeDpCBCVPN();
	void ChangeDpCBCV();

	void FormatBoxSmartControlValve();
	void ChangeSmartControlValveBodyMaterial();
	void ChangeSmartControlValveConnect();
	void ChangeSmartControlValvePN();
	void ChangeSmartControlValveLocalization();
	void ChangeSmartControlValveControlMode();
	void ChangeSmartControlValve();

	void FormatBoxSmartDpC();
	void ChangeSmartDpCBodyMaterial();
	void ChangeSmartDpCConnect();
	void ChangeSmartDpCPN();
	void ChangeSmartDpCLocalization();
	void ChangeSmartDpCSets();
	void ChangeSmartDpC();

	bool CheckValidity( double dVal );

	void ArrowKeyDown( long lFromColumn, long lFromRow );
	void ArrowKeyUp( long lFromColumn, long lFromRow );

	void CellClicked( long lColumn, long lRow );
	void ComboDropDown( long lColumn, long lRow );
	void ComboSelChange( long lColumn, long lRow );
	void ComboCloseUp( long lColumn, long lRow, short nSelChange );
	void EditChange( long lColumn, long lRow );
	void TextTipFetch( long lColumn, long lRow, WORD* pwMultiLine, SHORT* pnTipWidth, TCHAR* pstrTipText, BOOL* pfShowTip );

// Private methods.
private:
	void _DrawCheckbox( long lColumn, long lRow, CDS_HydroMod::eUseProduct eState );
	void _FillCVLoc();
	void _ChangeColumnSize( void );
	void InitDefaultBox( int iFirstCVRow );

	// To manage params with combo box and errors.
	void _ClearRange( enum _BoxType eBoxType, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2 );
	void _FormatComboList( enum _BoxType eBoxType, long lColumn, long lRow );
	void _AddComboBoxParam( enum _BoxType eBoxType, long lColumn, long lRow, LPARAM lpParam );
	LPARAM _GetComboBoxParam( enum _BoxType eBoxType, long lColumn, long lRow, int iIndex );
	void _VerifyComboBoxError();
	
// Protected variables.
protected:
	WORD m_wLastKeyPressed;
	WORD m_wLastShiftCtrlStatus;
	long m_lRow;
	long m_lCol;
	CString m_strCellText;
	double m_dCellDouble;
	bool m_bCellModified;
	bool m_bReady;
	bool m_bRedrawAllowed;
	CPrjParams *m_pPrjParams, *m_pPrjParamsSaved;
	float m_dFontFactor;
	bool m_bRefreshIncourse;
	bool m_bPrinting;
	bool m_bRefreshBoxPicture;							// Boolean than avoid multiple refreshing when the Init function is called
	std::map<short, short> m_mapColWidth;
	bool m_bManageEditOffSpreadNotification;

	// TADB Access
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CUnitDatabase *m_pUnitDB;
	CDS_HydroMod *m_pHM;
	CWaterChar *m_pWC;
	CTableDN *m_pclTableDN;
	CDS_TechnicalParameter *m_pTechParam;
	SS_CELLCOORD m_rCellCoord;

	// To manage content of a TSpread combo box (There is no way in native function to set a param for each line of a combo).
	class classcomp { public: bool operator() ( const CPoint &pt1, const CPoint &pt2) const { if( pt1.x == pt2.x ) { return ( pt1.y > pt2.y ); } return ( pt1.x < pt2.x ); } };
	typedef std::vector<LPARAM> vecparam;
	typedef std::map<CPoint, vecparam, classcomp> mappointparam;

	// In each box type, we can have a list of combo box defined by their position (CPoint).
	// And for each combo box, we can have a list or parameters links to each combo line.
	std::map<enum _BoxType, mappointparam> m_mapComboBoxList;

	typedef std::pair<CPoint, vecparam> pairptvecparam;
	typedef std::pair< enum _BoxType, mappointparam> pairboxtypemapptparam;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// For unit test: verify all circuit scheme positions.
#ifdef _DEBUG
	int m_iTestHydronicSchemesBoxPositionStatus;
	int m_iTestHydronicSchemesBoxPositionCols;
	int m_iTestHydronicSchemesBoxPositionRows;
	int m_iTestHydronicSchemesBoxPositionErrorBox;
	int m_iTestHydronicSchemesBoxPositionErrorXValue;
	int m_iTestHydronicSchemesBoxPositionErrorYValue;

	enum
	{
		Box_PrimaryValues,
		Box_PrimaryPipe,
		Box_DistributionPipe,
		Box_TerminalUnit,
		Box_ControlValve,
		Box_SmartControlValve,
		Box_BalancingValveOnPrimary,
		Box_BalancingValveOnBypass,
		Box_DifferentialPressureController,
		Box_Remark,
		Box_Pump,
		Box_BalancingValveOnSecondary,
		Box_SecondaryPipe,
		Box_SecondaryValues,
		Box_ReversePipe,
		Box_ShutoffValve,
		Box_CombinedDpCControlAndBalancingValve,
		Box_SmartDifferentialPressureController
	};

	std::map<int, CStringA> m_mapKeyName = {
		{ Box_PrimaryValues, "Primary values" },
		{ Box_PrimaryPipe, "Circuit pipe at the primary side" },
		{ Box_DistributionPipe, "Distribution pipe" },
		{ Box_TerminalUnit, "Terminal unit" },
		{ Box_ControlValve, "Control valve" },
		{ Box_SmartControlValve, "Smart control valve" },
		{ Box_BalancingValveOnPrimary, "Balancing valve on the primary side" },
		{ Box_BalancingValveOnBypass, "Balancing valve on the bypass" },
		{ Box_DifferentialPressureController, "Differential pressure controller" },
		{ Box_Remark, "Remark" },
		{ Box_Pump, "Pump" },
		{ Box_BalancingValveOnSecondary, "Balancing valve on the secondary side" },
		{ Box_SecondaryPipe, "Circuit pipe at the secondary side" },
		{ Box_SecondaryValues, "Secondary values" },
		{ Box_ReversePipe, "Reverse pipe" },
		{ Box_ShutoffValve, "Shutoff valve" },
		{ Box_CombinedDpCControlAndBalancingValve, "Combined Dp controller, control and balancing valve" },
		{ Box_SmartDifferentialPressureController, "Smart differential pressure controller" } };

#endif
	///////////////////////////////////////////////////////////////////////////////////////////////////

};
