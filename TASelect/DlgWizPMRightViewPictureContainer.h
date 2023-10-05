#pragma once


#include "DialogExt.h"

#define DLGWIZARDPM_RVPICTURE_SEPARATORCOLOR					RGB( 0, 120, 215 )

class CDlgWizardPM_RightViewPictureContainer : public CDialogExt
{
public:
	enum { IDD = IDD_DLGWIZPM_RVIEWPICTURECONTAINER };

	CDlgWizardPM_RightViewPictureContainer();

	void UpdatePicture( int iImageIDB );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	virtual afx_msg BOOL OnEraseBkgnd( CDC *pDC );
	virtual afx_msg void OnPaint();

	// Private variables.
private:
	int m_iCurrentImageIDB;
	HBITMAP m_hCurrentImage;
	CStatic m_clPicture;
};

