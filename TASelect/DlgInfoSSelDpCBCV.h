#pragma once


#include "ProductSelectionParameters.h"

class CProductSelelectionParameters;
class CIndSelDpCBCVParams;
class CEnBitmapPatchWork;
class CDlgInfoSSelDpCBCV : public CDialog
{
public:
	enum { IDD = IDD_DLGINFOSSELDPCBCV };

	CDlgInfoSSelDpCBCV( CProductSelelectionParameters *pclProdSelParams, CWnd *pParent = NULL );
	virtual ~CDlgInfoSSelDpCBCV();

	BOOL Create();
	void SetSchemePict( double dDpBranch, bool bWithSTS );
	void UpdateInfoDpCBCV( CDB_DpCBCValve *pclDpCBCValve, double dDpMin, bool bValidDplRange, double dFlow, double dRho );
	void UpdateInfoSTS( CDB_ShutoffValve *pclShutoffValve, double dDpFullyOpenedSTS );
	void Reset();
	virtual void OnOK();
	bool PrepareImage( CDC &dc, CEnBitmapPatchWork *pEnBmp );

// Public variables.
public:
	CEdit m_EditInfoDpCBCV;

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	void Init();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMove( int x, int y );
	afx_msg void OnDestroy();
	afx_msg void OnPaint();

// Private variables
private:
	CIndSelDpCBCVParams *m_pclIndSelDpCBCVParams;
	IDPTR m_DpCBCVIDPtr;
	IDPTR m_STSIDPtr;
	CWnd *m_pParentWnd;
	CString m_strSectionName;				// Name of the section in the registry
	bool m_bInitialized;					// Set to true when OnInitDialog performed
	bool m_bWithSTS;
	double m_dFlow;
	double m_dRho;
	double m_dDpMin;						// Minimum Dp of the DpCBCV
	double m_dDpBranch;
	double m_dDpFullyOpenedSTS;			// Pressure drop fully opened
	bool m_bDpCBCVSelected;				// Set to true when a DpCBCV is selected
	bool m_bSTSSelected;					// Set to true when a STS is selected
	bool m_bValidDplRange;
	CDB_DpCBCVCharacteristic *m_pDpCBCVChar;
	CString m_strDplRange;
	bool m_bSetSchemePictCalled;
};
