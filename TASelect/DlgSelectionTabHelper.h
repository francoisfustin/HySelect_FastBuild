#pragma once


#include <map>
class CDlgSelectionTabHelper
{
public:
	enum SetFocusWhere
	{
		First		= 0x00,
		Last		= 0x01,
		WhereMask	= 0x0F,
		Edit		= 0x10,
		Combo		= 0x20,
		TypeMask	= 0xF0
	};
	CDlgSelectionTabHelper( CWnd* pWndOwner ) { m_pWndOwner = pWndOwner; }
	virtual ~CDlgSelectionTabHelper() {}

	// Set the focus on the first available edit control.
	virtual void SetFocusOnControl( int iWhere );
	
	// This method is called when left tab dialog has no more the focus while user presses 'TAB' or 'SHIFT+TAB'.
	// Param: if 'fNext' that means user presses 'TAB', otherwise it's 'SHIFT+TAB'.
	// Return 'false' if the focus must stay on the left tab, otherwise the individual, batch or wizard selection in the right view has taken the focus.
	virtual bool LeftTabKillFocus( bool fNext ) { return false; }

// Protected methods.
protected:
	BOOL OnPreTranslateMessage( MSG* pMsg );

// Private methods.
private:
	HWND _GetActiveControl( CWnd *pWnd, UINT uiGetWhat );

// Private variables.
private:
	CWnd* m_pWndOwner;
};
