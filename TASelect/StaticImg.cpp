#include "stdafx.h"
#include "StaticImg.h"

void CStaticImg::SetImageID(int ID)
{
	if (!m_Img.IsNull())
		m_Img.Destroy();
	m_Img.LoadFromResource(AfxGetApp()->m_hInstance, ID);
}

void CStaticImg::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	m_Img.TransparentBlt(lpDrawItemStruct->hDC,0,0,m_Img.GetWidth(),m_Img.GetHeight(),0xFF000000);
}


