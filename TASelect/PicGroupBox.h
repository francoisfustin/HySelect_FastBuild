#pragma once

#include "afxwin.h"
#include "EnBitmap.h"

class CPicGroupBox :
	public CStatic
{
protected:
	CEnBitmap m_eBmp;
	CPoint	  m_PicPos;
	bool	  m_bTransparent;
public:
	CPicGroupBox();
	~CPicGroupBox(void);

	void SetPicture(int PicID, CPoint ptPos = CPoint(5,0), bool bTransparent = true);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};
