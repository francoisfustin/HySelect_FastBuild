#pragma once


#include "CEditString.h"

class CCbiRestrEdit : public CEdit, public CCbiRestrString
{
public:
	CCbiRestrEdit();
	virtual ~CCbiRestrEdit();

// Generated message map functions.
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg virtual void OnCBIRestEditChange();
	afx_msg LRESULT OnImeStartComposition( WPARAM wParam = 0, LPARAM lParam = 0);
};
