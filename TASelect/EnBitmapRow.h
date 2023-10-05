#pragma once
#include "EnBitmap.h"

class CEnBitmapRow: public CEnBitmap
{
// this class is used to work with a line of bitmap with different height and different width.
// The height of the computed image is the maximum height of all subimage.
// After a modification of a subimage you need to update the global image by call Compute_CEnBitmap.

public:
	// constructor
	CEnBitmapRow(COLORREF crBkgnd= RGB(255, 255, 255));
	virtual ~CEnBitmapRow();

	// add a CEnBitmap in the list of image (in the last postion)
	void AddCEnBitmap(CEnBitmap* newCEnBitmap);
	void AddCEnBitmap(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst = NULL, COLORREF crBack = -1); 
	// Replace the 'pos'th element in the list with the new element (newCEnBitmap).
	void ReplaceCEnBitmap(int pos,CEnBitmap* newCEnBitmap);
	void ReplaceCEnBitmap(int pos, UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack=-1);

	// rebuild the global image. To take effect of modification.
	void Compute_CEnBitmap();

	BOOL FlipImage(int pos, BOOL bHorz, BOOL bVert = 0);
	int GetWidth(){return m_width;};
	int GetHeight(){return m_height;};
private:
	CArray<LPARAM> m_ListCEnBitmap;
	int m_width,m_height;
};