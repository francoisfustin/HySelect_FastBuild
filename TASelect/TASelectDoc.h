//
// TASelectDoc.h : interface of the CTASelectDoc class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CTASelectDoc : public CDocument
{
protected: // create from serialization only
	CTASelectDoc();
	DECLARE_DYNCREATE( CTASelectDoc )

	// Implementation
public:
	virtual ~CTASelectDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

	void AutoSave();
	virtual void SetTitle( LPCTSTR lpszTitle );
	bool CloseTASelect();
	void FileSave( LPCTSTR lpszPathName, bool bAutoSave = false );

	// ClassWizard generated virtual function overrides.
	virtual BOOL OnNewDocument();

	// MRU file and drag and drop file ...
	virtual BOOL OnOpenDocument( LPCTSTR lpszPathName );
	virtual void OnCloseDocument();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	LRESULT OnCloseApplication( WPARAM wParam, LPARAM lParam );
};
extern CTASelectDoc *pTASelectDoc;