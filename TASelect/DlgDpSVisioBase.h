#pragma once


#include "afxdialogex.h"

class CDlgDpSVisioBase : public CDialogEx
{
public:
	CDlgDpSVisioBase( UINT nIDTemplate, CDlgDpSVisio *pParent );
	virtual ~CDlgDpSVisioBase() {}

// Public methods.
public:
	virtual void SetParam( LPARAM lpParam ) {}
	virtual void Stop( void ) {}
	virtual void OnDpSVisioConnected( CDlgDpSVisio::WhichDevice eWhichDevice ) {}
	virtual void OnDpSVisioDisconnected( CDlgDpSVisio::WhichDevice eWhichDevice ) {}

// Protected methods.
protected:
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual void OnOK();
	virtual void OnCancel();

protected:
	CDlgDpSVisio *m_pParent;
};
