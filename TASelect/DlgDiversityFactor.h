#pragma once


#include "afxdialogex.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "afxbutton.h"
#include "extcombobox.h"

#define _TREECONTAINER_SLIDERHEIGHT		25

class CDlgTreeContainer : public CDialogEx
{
public:
	CDlgTreeContainer();

	void GiveEvent( CEvent *pclEvent ) { m_pclEvent = pclEvent; }
	void SetHydroMod( CDS_HydroMod* pHM );
	double GetDiversityFactor( void ) { return m_dDiversityFactor; }
	void ChangeColorMode( bool fSet );
	void ResetTree ( void );
	CHMTreeListCtrl* GetpTree( void ) { return &m_Tree; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam );
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	
// Private methods.
private:
	void _FillTree( void );
	void _RefreshSliderControl( void );
	void _MoveSliderControl( void );
	void _SliderCtrlRightMove( void );
	void _SliderCtrlLeftMove( void );

// Private variables.
private:
	CEvent*				m_pclEvent;
	CDS_HydroMod*		m_pHM;
	CSliderCtrl			m_SliderCtrl;
	UINT				m_uiPrevSliderPos;
	CHMTreeListCtrl		m_Tree;
	CTCImageList		m_TreeImageList;
	// Don't use a CTCImageList for status, image appear shifted
	CImageList			m_TreeCheckBox;
	int					m_iSliderLeftPos;
	bool				m_bInitialized;
	double				m_dDiversityFactor;
};

class CDlgDiversityFactor : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgDiversityFactor )

	class CMyRichEditCtrl : public CRichEditCtrl
	{
		DECLARE_MESSAGE_MAP()
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
		afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnRButtonDblClk( UINT nFlags, CPoint point );
		afx_msg void OnMButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnMButtonDblClk( UINT nFlags, CPoint point );
		afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );
	};

	class CMyStatic : public CStatic
	{
		virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	};

public:
	enum { IDD = IDD_DLGDIVERSITYFACTOR };

	CDlgDiversityFactor( CWnd* pParent = NULL );
	virtual ~CDlgDiversityFactor( void );

	// Must be called by 'CMainFrame::OnRbnPPPFreeze()' to signal that we can or not enable 'Apply' button.
	void CheckInternalState( void );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand( UINT nID, LPARAM lParam ); 
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedApply();
	afx_msg void OnEnChangeEditDF();
	afx_msg void OnCbnSelChangeModule();
	afx_msg void OnBnClickedButtonrst();

// Private methods.
private:
	void _TakeSnapshot( void );
	void _RestoreSnapshot( void );
	bool _ApplyDiversityFactor( void );
	void _FillComboChooseRoot( void );
	void _CleanAllDiversityFactor(  CDS_HydroMod *pHM = NULL );

// Private variables.
private:
	CDlgTreeContainer m_TreeContainer;
	WORD m_arwTemplate[1024];
	WORD m_arwTemplate2[1024];
	CEdit m_cEditDF;
	double m_dDF;
	CMyRichEditCtrl m_cStaticText;
	CMFCButton m_clApplyButton;
	CExtNumEditComboBox m_clComboChooseRoot;
	bool m_bIsChanges;
	CTADatastruct *m_pclTADatastruct;
	CDS_HydroMod *m_pCurrentModule;
	
	// To take snapshot about diversity when entering in diversity factor dialog.
	typedef std::map<CDS_HydroMod*, double > mapHMDouble;
	typedef mapHMDouble::iterator mapHMDoubleIter;
	mapHMDouble m_mapDFSnapshot;
	CMyStatic m_LegendNoDFSquare;
	CMyStatic m_LegendDFSquare;
};
