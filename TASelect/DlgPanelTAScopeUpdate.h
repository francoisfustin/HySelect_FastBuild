#pragma once


#include "wizard.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "StaticImg.h"
#include "XGroupBox.h"
#include "FileTableMng.h"


class CDlgPanelTAScopeUpdate : public CDlgWizard
{
public:
	enum { IDD = IDD_PANEL_TASCOPE_UPDATE };

	CDlgPanelTAScopeUpdate( CWnd* pParent = NULL );
	virtual ~CDlgPanelTAScopeUpdate() {}

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	// Used to refresh the progress bar.
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizCancel();
	virtual BOOL OnWizNext();
	virtual BOOL OnWizFinish();
	virtual bool OnActivate();
	virtual bool OnAfterActivate();
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool InitFileList();
	void AddCategoryItem( UINT uiIDS, CFileTable* pSFT );

// Protected variables.
protected:
	enum eUpdateIcon
	{
		Incourse = 0,
		Failed,
		Success,
		Wait,
		Last
	};

	enum eProcessingZBUpdate
	{
		No = 0,
		Requested,
		InCourse,
		FinishOk,
		FinishFail,
		Reported,
		LastZBUpdate
	};
	
	CBrush			m_brWhiteBrush;
	CDlgWizTAScope *m_pParent;
	CImageList		m_SmallImageList;
	CListCtrl		m_FileList;
	CProgressCtrl	m_Progress;
	CProgressCtrl	m_ProgressAll;
	CEdit			m_EditInfos;
	CStatic			m_ConnectionPicture;
	CEnBitmap		m_Bmp;
	CXGroupBox		m_GroupVersion;
	CXGroupBox		m_GroupSFTVersion;
	CXGroupBox		m_GroupGBInfo;
	CFileTable		m_UseToConvertDoubleString;

	double			m_dTotalSize;
	double			m_dAlreadyUplploaded;
	double			m_dAlreadyUploadedCateg;
	bool			m_fUploadInCourse;
	bool			m_fUploadHFT;
	bool			m_fRefreshingHFTNeeded;
	int				m_iCurrentCategory;
	eProcessingZBUpdate m_eUpdateZBHH;
	eProcessingZBUpdate m_eUpdateZBDPS;
	UINT_PTR		m_nTimer;
	
#define MAXFILESBYCATEGORY	25
	struct CategoryItem
	{
		int			IDS;
		UINT		arCount;						// number of element into the POSITION array	
		POSITION	arPos[MAXFILESBYCATEGORY];		// Maximum MAXFILESBYCATEGORY files by category	
		double		TotalFileSize;
	};
	CArray<CategoryItem> m_ArCateg;

// Private methods.
private:
	void _CleanVersionField( void );
};
