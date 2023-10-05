#pragma once


#include "MyToolTipCtrl.h"
#include "XGroupBox.h"
#include "ExtButton.h"
#include "ExtComboBox.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "afxwin.h"

class CDlgCOWaterChar : public CDialogEx
{
public:
	class INotificationHandler
	{
	public:
		virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgCOWaterChar *pclDlgWaterChar ) = 0;
	};

	enum { IDD = IDD_DLGCOWATERCHAR };

	enum eGroupApplication
	{
		eCooling = 0,
		eHeating,
		eLast = eHeating
	};

	CDlgCOWaterChar( CProductSelelectionParameters *m_pclProductSelParams = NULL, INotificationHandler *pfnVerifyFluidCharacteristics = NULL, CWnd *pParent = NULL );
	//~CDlgCOWaterChar();

	// Opens the 'Water Characteristics' dialog window.
	// Caption specifies the text to set in the title bar.
	// Origin specifies from which origin the dialog is started.
	// Returns the same value as domodal().
	int Display( LPCTSTR lptstrCaption = NULL, WMUserWaterCharWParam eOrigin = WMUserWaterCharWParam::WM_UWC_WP_ForProductSel );

	CString GetAdditiveFamilyID( void ) { return m_AdditFamID; }
	CString GetAdditiveID( void ) { return m_AdditID; }
	double GetPCWeight( void ) { return m_dPCWeight; }
	double GetCoolingTemp( void ) { return m_dCoolingTemp; }
	double GetHeatingTemp( void ) { return m_dHeatingTemp; }
	double GetCoolingDT( void ) { return m_dDefaultCoolingDT; }
	double GetHeatingDT( void ) { return m_dDefaultHeatingDT; }
	double GetCoolingDensity( void ) { return m_dCoolingDensity; }
	double GetHeatingDensity( void ) { return m_dHeatingDensity; }
	double GetCoolingKinVisc( void ) { return m_dCoolingKinVisc; }
	double GetHeatingKinVisc( void ) { return m_dHeatingKinVisc; }
	double GetCoolingSpecificHeat( void ) { return m_dCoolingSpecifHeat; }
	double GetHeatingSpecificHeat( void ) { return m_dHeatingSpecifHeat; }
	double GetCoolingFrozenTemperature( void ) { return m_dCoolingTempfreez; }
	double GetHeatingFrozenTemperature( void ) { return m_dHeatingTempfreez; }

	// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual void OnOK();
	afx_msg void OnDestroy();

	afx_msg void OnChangeEditCoolingTemperature();
	afx_msg void OnChangeEditHeatingTemperature();
	afx_msg void OnChangeEditCoolingDT();
	afx_msg void OnChangeEditHeatingDT();
	afx_msg void OnCbnSelChangeAdditiveFamily();
	afx_msg void OnCbnSelChangeAdditiveName();
	afx_msg void OnChangeEditPCWeight();
	afx_msg void OnButtonRecommendation();
	afx_msg void OnEditEnterCoolingTemperature( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnEnKillFocusCoolingTemperature();
	afx_msg void OnEditEnterHeatingTemperature( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnEnKillFocusHeatingTemperature();
	afx_msg void OnEnKillFocusCoolingDT();
	afx_msg void OnEnKillFocusHeatingDT();
	afx_msg void OnEnKillFocusPCWeight();

	afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );


	// Update fields for unknown state.
	void SetUnknownState( eGroupApplication eGroup );

	// Update Water Phase bitmap.
	void UpdateWaterBitmap( enum WaterPhase_enum phase, eGroupApplication eGroup );

	// Update all fields.
	void UpdateAllCooling();	
	void UpdateAllHeating();	
	void UpdateAll();	
	// Fill the 2 combo boxes of the additive group.
	// 'Family' and 'Name' specify the default selections. If NULL, the "** No Add ..." and empty strings are respectively selected.
	void _FillComboAdditFam( LPCTSTR ptstrFamily = NULL );
	void _FillComboAdditName( CString strID );
	void _DisplayWarningWithID( bool bActivate, eGroupApplication eGroup, CExtNumEdit *pExtEdit = NULL, UINT IDMsg = 0, double dValue = 0.0, int iUnit = -1 );
	void _DisplayWarningWithString( bool bActivate, eGroupApplication eGroup, CExtNumEdit *pExtEdit = NULL, CString strMsg = _T( "" ), double dValue = 0.0, int iUnit = -1 );

	// Protected variables.
private:
	CWaterChar m_clWCChangeOverCopy;
	CExtButton m_clButtonOK;
	CMFCButton m_clButtonRecommendation;
	CExtNumEdit m_EditCoolingTemp;
	CExtNumEdit m_EditHeatingTemp;
	CExtNumEdit m_EditDefaultCoolingDT;
	CExtNumEdit m_EditDefaultHeatingDT;
	CExtNumEdit m_EditPcWeight;
	CExtNumEdit m_EditCoolingDens;
	CExtNumEdit m_EditHeatingDens;
	CExtNumEdit m_EditCoolingSpecifHeat;
	CExtNumEdit m_EditHeatingSpecifHeat;
	CExtNumEdit m_EditCoolingKinVisc;
	CExtNumEdit m_EditHeatingKinVisc;
	CExtNumEdit m_EditCoolingFreezPoint;
	CExtNumEdit m_EditHeatingFreezPoint;
	CExtNumEdit m_StaticCoolingWarning;
	CExtNumEdit m_StaticHeatingWarning;

	CExtNumEditComboBox m_ComboAdditName;
	CExtNumEditComboBox m_ComboAdditFam;

	CXGroupBox m_GroupCoolingRho;
	CXGroupBox m_GroupCooling;
	CXGroupBox m_GroupAddit;
	CXGroupBox m_GroupHeatingRho;
	CXGroupBox m_GroupHeating;

	CProductSelelectionParameters *m_pclProductSelParams;
	INotificationHandler *m_pfnVerifyFluidCharacteristics;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	LPCTSTR m_ptstrCaption;					// Dialog caption string
	WMUserWaterCharWParam m_eOrigin;				// Dialog origin (see 'global.h' for details).
	CMyToolTipCtrl m_ToolTip;				// ToolTipCtrl for additive family combo box
	CBitmap m_BitmapCoolingLiquid;					// Bitmap for liquid phase
	CBitmap m_BitmapCoolingIce;					// Bitmap for iced water
	CBitmap m_BitmapCoolingUnknown;				// Bitmap for unknown state
	CBitmap m_BitmapHeatingLiquid;					// Bitmap for liquid phase
	CBitmap m_BitmapHeatingIce;					// Bitmap for iced water
	CBitmap m_BitmapHeatingUnknown;				// Bitmap for unknown state
	CString m_AdditFamID;					// ID of selected additive family
	CString m_AdditID;						// ID of selected additive
	ProjectType m_PrjType;					// Cooling//Heating//Solar
	double m_dCoolingTemp;
	double m_dPCWeight;
	double m_dCoolingDensity;
	double m_dCoolingKinVisc;
	double m_dCoolingSpecifHeat;
	double m_dCoolingTempfreez;
	double m_dDefaultCoolingDT;
	double m_dHeatingTemp;
	double m_dPCHeatingWeight;
	double m_dHeatingDensity;
	double m_dHeatingKinVisc;
	double m_dHeatingSpecifHeat;
	double m_dHeatingTempfreez;
	double m_dDefaultHeatingDT;

	CTable *m_pAdditFamTab;					// Pointer on selected additive family
	CDB_AdditCharacteristic *m_pAdditChar;	// Additive characteristic

};

