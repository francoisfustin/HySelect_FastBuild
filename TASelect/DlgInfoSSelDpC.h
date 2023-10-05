#pragma once


#include "ProductSelectionParameters.h"

class CProductSelelectionParameters;
class CIndSelDpCParams;
class CEnBitmapPatchWork;
class CDlgInfoSSelDpC : public CDialog
{
public:
	enum { IDD = IDD_DLGINFOSSELDPC };
	CDlgInfoSSelDpC( CProductSelelectionParameters *pclProdSelParams, CWnd *pParent = NULL );

	BOOL Create();
	void SetSchemePict( eDpStab eDpStabilzedOn, eMvLoc eMvLocation, eDpCLoc eDpCLocation, bool bKvDpOK, double dDpBranch, double dKvCv, bool bForSet = false );
	void UpdateInfoMV( CDB_TAProduct *pDev, double dDpComputedOnMv, double dDpFullyOpenedMv, double dSettingMv );
	void UpdateInfoDpC( CDB_DpController *pDev, double dDpMin, bool bValidDplRange );
	void Reset();
	virtual void OnOK();
	bool PrepareImage( CDC &dc, CEnBitmapPatchWork *pEnBmp );

// Public variables.
public:
	CEdit m_EditInfoDpC;

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange *pDX );
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	void Init();
	virtual afx_msg void OnMove( int x, int y );
	virtual afx_msg void OnDestroy();
	virtual afx_msg void OnPaint();

// Private variables.
private:
	CIndSelDpCParams *m_pclIndSelDpCParams;
	CWnd *m_pParentWnd;
	IDPTR m_DpCIDPtr;
	IDPTR m_ValveIDPtr;
	CString m_strSectionName;				// Name of the section in the registry
	bool m_bInitialized;					// Set to true when OnInitDialog performed
	eDpStab m_eDpStab;
	eMvLoc m_eMvLoc;
	eDpCLoc m_eDpCLoc;
	bool m_bKvDpOK;
	double m_dDpBranch;
	double m_dKvCv;
	bool m_bForSet;
	double m_dDpComputedOnMv;				// Pressure drop for the computed setting
	double m_dDpFullyOpenedMv;				// Pressure drop fully opened
	double m_dSettingMv;
	bool m_bDpCSelected;					// Set to true when a DpC is selected
	bool m_bMvSelected;					// Set to true when a Mv is selected
	double m_dDpMin;						// Minimum Dp of the DpC
	bool m_bValidDplRange;
	CDB_DpCCharacteristic *m_pDpCChar;
	double m_dBPDpC;						// Proportional band of the DpC
	bool m_bAbsoluteBP;
	CString m_strDplRange;
	bool m_bSetSchemePictCalled;
};
