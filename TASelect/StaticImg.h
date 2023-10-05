#pragma once
class CStaticImg : public CStatic
{
protected:
	//CEnBitmap m_Img;
	CImage m_Img;
public:
	CStaticImg():CStatic(){};
	void SetImageID(int ID);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};
