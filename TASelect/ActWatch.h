// ActWatch.h : header file
//

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CActivationWatcher thread

class CActivationWatcher : public CWinThread
{
	DECLARE_DYNCREATE(CActivationWatcher)
protected:
	CActivationWatcher();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActivationWatcher)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CActivationWatcher();

	// Generated message map functions
	//{{AFX_MSG(CActivationWatcher)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
