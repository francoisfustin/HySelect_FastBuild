#pragma once

class CMyMFCTabCtrl : public CMFCTabCtrl
{
public:
	
	typedef enum TabIndex
	{
		etiUndefined = -1,
		etiProj	= 0,
		etiSSel	= 1,
		etiSelP	= 2,
		etiInfo	= 3,
		etiLast	= 4
	};

	CMyMFCTabCtrl() {}
	virtual ~CMyMFCTabCtrl() {}
	
	virtual DROPEFFECT OnDragEnter( COleDataObject* pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual DROPEFFECT OnDragOver( COleDataObject* pDataObject, DWORD dwKeyState, CPoint point ) { return DROPEFFECT_NONE; }
	virtual void SwapTabs( int nFisrtTabID, int nSecondTabID ) {}
	virtual void MoveTab( int nSource, int nDest) {}
};

class CDlgLeftTabBase;
class CDlgLTtabctrl : public CDockablePane
{
	DECLARE_DYNAMIC( CDlgLTtabctrl )

public:
	class INotificationHandler
	{
	public:
		enum
		{
			NH_OnLeaveTab	= 0x01,
			NH_OnEnterTab	= 0x02
		};
		INotificationHandler() {}
		~INotificationHandler() {}
		virtual void OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex ) = 0;
 		virtual void OnLTTabCtrlLeaveTab( CMyMFCTabCtrl::TabIndex eTabIndex ) = 0;
	};

	CDlgLTtabctrl();
	virtual ~CDlgLTtabctrl();
	
	void SetActiveTab( CMyMFCTabCtrl::TabIndex eTabIndex );
	CMyMFCTabCtrl::TabIndex GetActiveTab( void ) { return m_eTabIndex; }

	void RegisterNotificationHandler( CMyMFCTabCtrl::TabIndex eTabIndex, INotificationHandler *pclHandler, short nNotificationHandlerFlags );
	void UnregisterNotificationHandler( CMyMFCTabCtrl::TabIndex eTabIndex, INotificationHandler *pclHandler );

	CMFCTabCtrl* GetMFCTabCtrl() { return &m_wndTabs; }
	void OnApplicationLook();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg LRESULT OnChangeActiveTab( WPARAM wp, LPARAM lp );
	afx_msg LRESULT OnChangingActiveTab( WPARAM wp, LPARAM lp );

private:
	void _ClearTabs();

// Private variables.
private:
	CMyMFCTabCtrl				m_wndTabs;
	CMyMFCTabCtrl::TabIndex		m_eTabIndex;
	CMyMFCTabCtrl::TabIndex		m_ePrevTabIndex;
	CImageList					m_TabCtrlImageList;
	typedef std::map<short, INotificationHandler*> mapShortNotif;
	typedef mapShortNotif::iterator mapShortNotifIter;
	std::map<CMyMFCTabCtrl::TabIndex, mapShortNotif> m_mapNotificationHandlerList;
	std::map<CMyMFCTabCtrl::TabIndex, CDlgLeftTabBase*> m_mapLeftTabList;
};

extern CDlgLTtabctrl *pDlgLTtabctrl;
