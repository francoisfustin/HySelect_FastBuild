#pragma once


#include "DialogExt.h"
#include "ExtStatic.h"

#define DLGWIZARDPM_RVTEXT_HEADERTEXTSIZE					( 10 ) 
#define DLGWIZARDPM_RVTEXT_NORMALTEXTSIZE					( 10 )
#define DLGWIZARDPM_RVTEXT_HEADERTEXTBOLD					( true )		
#define DLGWIZARDPM_RVTEXT_NORMALTEXTBOLD					( false )
#define DLGWIZARDPM_RVTEXT_SPACEBETWEENHEADERNORMALTEXT		( 2 )
#define DLGWIZARDPM_RVTEXT_SPACEBETWEENTWOTEXT				( 10 )

#define DLGWIZARDPM_RVTEXT_SEPARATORCOLOR					RGB( 0, 120, 215 )

class CRViewWizardSelPM;
class CDlgWizardPM_RightViewTextContainer : public CDialogExt
{
public:
	enum { IDD = IDD_DLGWIZPM_RVIEWTEXTCONTAINER };

	CDlgWizardPM_RightViewTextContainer();
	virtual ~CDlgWizardPM_RightViewTextContainer();

	void UpdateText( CArray<CRViewWizardSelPM::CTextIDValue> *parTextIDSList );
	virtual afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();

	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnPaint();
	virtual afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar );
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

// Private methods.
private:
	void _Clear( void );
	int _ComputeHeightNeeded( int iWidth, int iHeight );

// Private variables.
private:
	CArray<CExtStatic *> m_arpExtStaticList;
	CArray<CRViewWizardSelPM::CTextIDValue> m_arTextIDSList;
};

