#pragma once


#include <direct.h>
#include <afxhtml.h>
#include "TASelectDoc.h"


class CRViewInfo : public CHtmlView
{
	DECLARE_DYNCREATE( CRViewInfo )

public:
	CRViewInfo() {}
	virtual ~CRViewInfo();

	CTASelectDoc *GetDocument();

	// Display an html page.
	// Either the url (beginning with "http://") or the relative path of the file from the installation directory must be 
	// given in const char* url.
	void DisplayPage( LPCTSTR ptcURL );

	// Overrides 'CView' method.
	virtual void OnInitialUpdate();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();

	// Overridden to draw this view.
	virtual void OnDraw( CDC *pDC ) {}

private:
	CString m_strOriginalPage;
};
 
extern CRViewInfo *pRViewInfo;

#ifndef _DEBUG  // debug version in RightViewSelP.cpp
inline CTASelectDoc* CRViewInfo::GetDocument()
   { return (CTASelectDoc*)m_pDocument; }
#endif