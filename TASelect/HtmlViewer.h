//
// RViewInfo.h : interface of the CHtmlViewer class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <direct.h>
#include <afxhtml.h>


class CHtmlViewer : public CHtmlView
{
public: // create from serialization only
	CHtmlViewer();


private:
	CString m_OriginalPage;

// Operations
public:
	// Display an html page.
	// Either the url (beginning with "http://") or the relative 
	// path of the file from the installation directory must be 
	// given in const char* url.
	void DisplayPage(CString url);
	// This virtual function is called when the user clicks on a link
	// of the displayed html page.
	virtual void OnClickLink(){/*AfxMessageBox("");*/};
	virtual void OnInitialUpdate();

// Overrides
	// ClassWizard generated virtual function overrides
	public:
	virtual void OnDraw(CDC* pDC) {} ;  // overridden to draw this view

// Implementation
public:
	virtual ~CHtmlViewer();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnMouseActivate( CWnd* pDesktopWnd, UINT nHitTest, UINT message );

};
