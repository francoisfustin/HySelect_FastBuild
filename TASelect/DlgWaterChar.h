#pragma once

#include "MyToolTipCtrl.h"
#include "XGroupBox.h"
#include "ExtButton.h"
#include "ExtComboBox.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "afxwin.h"

enum WaterPhase_enum
{
	WP_LIQUID,
	WP_ICE,
	WP_UNKNOWN
};

class CDlgWaterChar : public CDialogEx
{
public:
	class INotificationHandler
	{
	public:
		virtual void OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar ) = 0;
	};

	enum { IDD = IDD_DLGWATERCHAR };

	CDlgWaterChar( INotificationHandler *pfnVerifyFluidCharacteristics = NULL, CWnd *pParent = NULL );

	// Opens the 'Water Characteristics' dialog window.
	// 'lptstrCaption' specifies the text to set in the title bar.
	// 'eOrigin' specifies from which origin the dialog is started.
	// 'eApplicationTypeToForce' if set, we force the application type combo to this value.
	// Returns the same value as domodal().
	enum DlgWaterCharOrigin
	{
		DlgWaterChar_ForProductSelection,
		DlgWaterChar_ForHMCalc,
		DlgWaterChar_ForTools
	};

	int Display( LPCTSTR lptstrCaption = NULL, DlgWaterCharOrigin eOrigin = DlgWaterCharOrigin::DlgWaterChar_ForProductSelection, bool bModificationEnabled = true,
			ProjectType eApplicationTypeToForce = ProjectType::InvalidProjectType );

	ProjectType GetInitialApplicationType( void ) { return m_eGlobalProjectType; }
	ProjectType GetCurrentApplicationType( void ) { return m_eCurrentProjectType; }
	CString GetAdditiveFamilyID( ProjectType eProjectType = ProjectType::InvalidProjectType );
	CString GetAdditiveID( ProjectType eProjectType = ProjectType::InvalidProjectType );
	double GetTemp( ProjectType eProjectType = ProjectType::InvalidProjectType );
	double GetDT( ProjectType eProjectType = ProjectType::InvalidProjectType );
	double GetPCWeight( ProjectType eProjectType = ProjectType::InvalidProjectType );
	double GetDensity( ProjectType eProjectType = ProjectType::InvalidProjectType );
	double GetKinematicViscosity( ProjectType eProjectType = ProjectType::InvalidProjectType );
	double GetSpecificHeat( ProjectType eProjectType = ProjectType::InvalidProjectType );
	double GetFrozenTemperature( ProjectType eProjectType = ProjectType::InvalidProjectType );
	
	// Retrieve the water char for the current application type (The application type when the dialog has been opened!).
	void GetWaterChar( CWaterChar &clWaterChar, double &dDT );
	
	// Retrieve the water char for a specific application type.
	// Returns 'false' if there is no water characteristic for the 'eProjectType'. Otherwise, returns 'true' and 
	// returns the corresponding water characteristic in 'clWaterChar'.
	bool GetWaterChar( ProjectType eProjectType, CWaterChar &clWaterChar, double &dDT );

	// Returns 'true' if something has been changed in the water characteristic for the current application type.
	bool IsSomethingChanged( void );

	// Returns 'true' if something has been changed in the water characteristic for a specific application type.
	bool IsSomethingChanged( ProjectType eProjectType );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual void OnOK();
	afx_msg void OnDestroy();

	afx_msg void OnChangeEditTemperature();
	afx_msg void OnChangeEditDT();
	afx_msg void OnCbnSelChangeApplicationType();
	afx_msg void OnCbnSelChangeAdditiveFamily();
	afx_msg void OnCbnSelChangeAdditiveName();
	afx_msg void OnCheckSpecificHeat();
	afx_msg void OnChangeEditPCWeight();
	afx_msg void OnButtonRecommendation();
	afx_msg void OnChangeEditDensity();
	afx_msg void OnChangeEditKinematicViscosity();
	afx_msg void OnChangeEditSpecificHeat();

	afx_msg void OnEditEnterTemperature( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnEnKillFocusTemperature();
	afx_msg void OnEnKillFocusDT();
	afx_msg void OnEnKillFocusPCWeight();
	afx_msg void OnEnKillFocusDensity();
	afx_msg void OnEnKillFocusKinematicViscosity();
	afx_msg void OnEnKillFocusSpecificHeat();

	afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	
	// Update fields for unknown state.
	void SetUnknownState();
	
	// Update Water Phase bitmap.
	void UpdateWaterBitmap( enum WaterPhase_enum phase );
	
	// Update all fields.
	void UpdateAll();

	void _FillComboApplicationType( ProjectType eProjectType );

	// Fill the 2 combo boxes of the additive group.
	// 'Family' and 'Name' specify the default selections. If NULL, the "** No Add ..." and empty strings are respectively selected.
	void _FillComboAdditFam( LPCTSTR ptstrFamily = NULL );
	void _FillComboAdditName( CString strID );

	void _DisplayWarningWithID( bool bActivate, CExtNumEdit *pExtEdit = NULL, UINT IDMsg = 0, double dValue = 0.0, int iUnit = -1 );
	void _DisplayWarningWithString( bool bActivate, CExtNumEdit *pExtEdit = NULL, CString strMsg = _T(""), double dValue = 0.0, int iUnit = -1 );

// Protected variables.
private:
	std::map<ProjectType, CWaterChar> m_mapWaterCharWorking;
	std::map<ProjectType, double> m_mapDTWorking;
	std::map<ProjectType, bool> m_mapIsSomethingChanged;

	std::map<ProjectType, CWaterChar> m_mapWaterCharCopies;
	std::map<ProjectType, double> m_mapDTCopies;

	CExtButton m_clButtonOK;
	CMFCButton m_clButtonRecommendation;
	CButton m_clCheckSpecifHeat;
	CExtNumEdit m_EditTemp;
	CExtNumEdit m_EditDefaultDT;
	CExtNumEdit m_EditSpecifHeat;
	CExtNumEdit m_EditKinVisc;
	CExtNumEdit m_EditDens;
	CExtNumEdit m_EditPcWeight;
	CExtNumEdit m_EditFreezPoint;
	CExtNumEdit m_StaticWarning;

	CExtNumEditComboBox m_ComboApplicationType;
	CExtNumEditComboBox m_ComboAdditName;
	CExtNumEditComboBox m_ComboAdditFam;

	CXGroupBox m_GroupApplicationType;
	CXGroupBox m_GroupRho;
	CXGroupBox m_GroupPT;
	CXGroupBox m_GroupAddit;
	
	INotificationHandler *m_pfnVerifyFluidCharacteristics;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	LPCTSTR m_ptstrCaption;					// Dialog caption string
	DlgWaterCharOrigin m_eOrigin;			// Who called the dialog.
	CMyToolTipCtrl m_ToolTip;				// ToolTipCtrl for additive family combo box
	CBitmap m_BitmapLiquid;					// Bitmap for liquid phase
	CBitmap m_BitmapIce;					// Bitmap for iced water
	CBitmap m_BitmapUnknown;				// Bitmap for unknown state
	
	// Global project type as it was when creating the dialog.
	ProjectType m_eGlobalProjectType;

	// Current choice of the user with the application type combo.
	ProjectType m_eCurrentProjectType;

	// Forced value asked when calling 'Display' method (Can be the same as the global).
	ProjectType m_eForcedProjectType;

	bool m_bModificationEnabled;

	CTable *m_pAdditFamTab;					// Pointer on selected additive family
	CDB_AdditCharacteristic *m_pAdditChar;	// Additive characteristic

	CFont m_font;
};