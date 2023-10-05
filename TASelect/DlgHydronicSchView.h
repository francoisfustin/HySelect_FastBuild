#pragma once


#include "afxcmn.h"
#include "afxwin.h"
#include <afxcview.h>
#include "EnBitmapComponent.h"
#include "EnBitmapPatchWork.h"
#include "DlgHydronicSchView_BaseTab.h"

#define _2W				_T("2-Way")
#define _2WINJ			_T("2-Way injection")
#define _3W				_T("3-Way")
#define _3WINJMIX		_T("3-Way inj. mixing")
#define _3WINJDB		_T("3-Way inj. dec. bypass")
#define _DC				_T("Distribution circuit")
#define _DiffPresCtrl	_T("Diff. pressure control")
#define _ManBalanc		_T("Manual balancing")
#define _Electronic		_T("Electronic")
#define _NoBalanc		_T("No balancing")
#define _PDG			_T("Pending circuit")
#define _PUMP			_T("Pump")
#define _SPipes			_T("Straight pipes")
#define _ReturnDirect	_T("Direct")
#define _ReturnReverse	_T("Reverse")
#define _Supply			_T("Supply")
#define _Return			_T("Return")

#ifdef UNICODE
#define _string std::wstring 
#else
#define _string std::string 
#endif

class CDlgHydronicSchView_Tabs : public CMFCTabCtrl
{
public:

	// Overrides some 'CMFCBaseTabCtrl' to avoid drag to be moved.
	virtual DROPEFFECT OnDragEnter( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual DROPEFFECT OnDragOver( COleDataObject *pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual void SwapTabs( int nFisrtTabID, int nSecondTabID ) {}
	virtual void MoveTab( int nSource, int nDest ) {}
};

class CDlgHydronicSchView : public CDialogEx
{
public:
	DECLARE_DYNAMIC( CDlgHydronicSchView )

	enum { IDD = IDD_DLGHYDRONICSCHVIEW };

	CDlgHydronicSchView( CWnd *pParent = NULL );
	virtual ~CDlgHydronicSchView();

	void VerifyCircuitSheme( void );
	void RedrawSchemes();
	
	CDB_CircuitScheme::eBALTYPE GetBalancingType() { return m_eBalancingType; }
	CDS_HydroMod::ReturnType GetReturnType() { return m_eReturnType; }
	bool GetTerminalUnit() { return m_bTermUnit; }
	bool GetShowDistributionPipe() { return m_bShowDistributionPipe; }
	CDB_ControlProperties::CvCtrlType GetControlType() { return m_eControlType; }
	CDB_ControlProperties::eCVFUNC GetCvType() { return m_eCvType; }

	eMvLoc GetMvLoc();
	eDpStab GetDpStab();
	bool GetBvIn3WBypass();
	SmartValveLocalization GetSmartValveLocatization();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnCbnSelChangeCircuitSchemeCategory();
	afx_msg void OnCbnSelChangeBalancingType();
	afx_msg void OnCbnSelChangeCircuitScheme();
	afx_msg void OnCbnSelChangeReturnType();
	afx_msg void OnBnClickedCheckTerminalUnit();
	afx_msg void OnBnClickedCheckShowDistributionPipe();
	afx_msg void OnCbnSelChangeControlType();
	afx_msg void OnCbnSelComboControlValveType();
	afx_msg void OnBnClickedLargeViewSymbol();
	afx_msg void OnBnClickedLargeViewPicture();
	afx_msg void OnBnClickedButtonDraw();
	
	void InitComboBox();
	void FillComboBoxCircuitSchemeCategory();
	void FillComboBoxBalancingType();
	void FillComboBoxCircuitScheme();
	void FillComboBoxReturnType();
	void FillComboBoxControlType();
	void FillComboBoxControlValveType();

	void VerifyTerminalUnit();
	void VerifyShowDistributionPipe();
	void VerifyControlValveType();

	void AddHMChildren( CTable *pTab );

	typedef enum ProdType 
	{	
		Undefined = -1,
		Bv = 0,
		DpC,
		Cv
	};
	
	typedef enum eCirSchCateg
	{
		PDG,			// Pending (SCHCATEG_PDG)
		PUMP,			// Pump (SCHCATEG_PUMP)
		SP,				// Straight pipes (SCHCATEG_SP)
		DC,				// Distribution circuit (SCHCATEG_DC)
		CV_2W,			// 2-way with control (SCHCATEG_2WD)
		CV_2WINJ,		// 2-way injection with control (SCHCATEG_2W_INJ)
		CV_3W,			// 3-way with control (SCHCATEG_3WD)
		CV_3WINJMIX,	// 3-way injection mixing with control (SCHCATEG_3WINJ_MIX)
		CV_3WINJDB,		// 3-way injection with decoupling bypass (SCHCATEG_3WINJ_DB)
		NOCAT
	};
	
	typedef enum eDrawingType
	{
		OldScheme,	// 0
		DynScheme,	// 1
		DynTAScheme	// 2
	};

	// Retrieve a rectangle with the original size for the image if possible, otherwise take the largest window's size.
	// The image will not be deformed.
	CRect GetGoodSizeRect( eDrawingType eSchemeType );

	void DrawDynScheme( bool bForDynSchTASymbol );

// Private methods.
private:

// Private variables.
private:
	CTADatabase *m_pTADB;
	ProdType m_eProdType;

	CComboBox m_ComboCircuitSchemeCategory;
	CComboBox m_ComboBalancingType;
	CComboBox m_ComboCircuitScheme;
	CComboBox m_ComboReturnType;
	CComboBox m_ComboControlType;
	CComboBox m_ComboControlValveType;

	CButton m_CheckTerminalUnit;
	CButton m_CheckShowDistributionPipe;
	
	CStatic m_stDynSchTA;
	CStatic m_stDynSch;

	CDB_CircSchemeCateg *m_pclCircuitSchemeCategory;
	CDB_CircuitScheme::eBALTYPE m_eBalancingType;
	CDS_HydroMod::ReturnType m_eReturnType;
	bool m_bTermUnit;
	bool m_bShowDistributionPipe;
	CDB_ControlProperties::CvCtrlType m_eControlType;
	CDB_ControlProperties::eCVFUNC m_eCvType;

	CProdPic *m_pProdPic;
	CEnBitmapPatchWork m_DynBmp;
	CEnBitmapPatchWork m_DynTASymbolBmp;
	CDB_CircuitScheme *m_pCircuitScheme;
	CTable *m_pTabSch;
	CTable *m_pDpCSchemeTab;

	CDlgHydronicSchView_Tabs m_clHydronicSchViewCtrlTab;
	std::map<CAnchorPt::eFunc, CDlgHydronicSchView_BaseTab *> m_mapTabList;

	CArray<CDB_CircSchemeCateg *> m_arCDBSchemeCategory;
	std::vector<CDB_CircuitScheme *> m_vecCircuitSchemeList;
};
