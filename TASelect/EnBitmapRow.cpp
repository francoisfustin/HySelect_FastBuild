#include "stdafx.h"
#include "EnBitmapRow.h"
// this class is used to work with a line of bitmap of same height but different width.
// After a modification of a subimage you need to update the global image by call Compute_CEnBitmap.

CEnBitmapRow::CEnBitmapRow(COLORREF crBkgnd)
{
	m_crBkgnd = crBkgnd;
	m_width = 0;
	m_height = 0;
}

CEnBitmapRow::~CEnBitmapRow()
{
	for (int i=0;i<m_ListCEnBitmap.GetCount();i++)
	{
		((CEnBitmap *)m_ListCEnBitmap.GetAt(i))->DeleteObject();  //delete the object in the list.
		delete ((CEnBitmap *)m_ListCEnBitmap.GetAt(i));
	}
	m_ListCEnBitmap.RemoveAll(); // empty the list
}
void CEnBitmapRow::AddCEnBitmap(CEnBitmap* newCEnBitmap){

	CEnBitmap* EnBmp=new CEnBitmap();
	BITMAP BM;
	newCEnBitmap->GetObject(sizeof(BM), &BM);
	m_width += BM.bmWidth;   // calculation of the new width of the global bitmap.
	m_height = max( m_height , BM.bmHeight ) ;

	EnBmp->CopyImage(*newCEnBitmap);
	m_ListCEnBitmap.Add((LPARAM) EnBmp); //add the element in the list.
}
void CEnBitmapRow::AddCEnBitmap(UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	CEnBitmap EnBmp;
	EnBmp.LoadImage(uIDRes, szResourceType, hInst, crBack); // build the CenBitmap
	BITMAP BM;
	EnBmp.GetObject(sizeof(BM), &BM);
	m_width += BM.bmWidth; // calculation of the new width of the global bitmap.
	m_height = max( m_height , BM.bmHeight ) ;
	m_ListCEnBitmap.Add((LPARAM) &EnBmp); //add the element in the list.
}
void CEnBitmapRow::ReplaceCEnBitmap(int pos, UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	CEnBitmap EnBmp;
	EnBmp.LoadImage(uIDRes, szResourceType, hInst, crBack); // build the CenBitmap
	ReplaceCEnBitmap(pos,&EnBmp);							//replace the element in the list.
}

void CEnBitmapRow::ReplaceCEnBitmap(int pos,CEnBitmap* newCEnBitmap)
{
	CEnBitmap* EnBmp=new CEnBitmap();
	EnBmp->CopyImage(*newCEnBitmap);
	BITMAP BM;
	EnBmp->GetObject(sizeof(BM), &BM);
	m_width += BM.bmWidth;  // calculation of the new width of the global bitmap.
	m_height = max( m_height , BM.bmHeight ) ;
	((CEnBitmap *)m_ListCEnBitmap.GetAt(pos))->GetObject(sizeof(BM), &BM);
	m_width -= BM.bmWidth;	// calculation of the new width of the global bitmap. remove the old image.
	((CEnBitmap *)m_ListCEnBitmap.GetAt(pos))->DeleteObject();  // delete the old element
	m_ListCEnBitmap.RemoveAt(pos);								// remove the ptr in the list
	m_ListCEnBitmap.InsertAt(pos,(LPARAM) EnBmp);				// add the new element in the good pos.
}
void CEnBitmapRow::Compute_CEnBitmap()
{
	int rectHeight, rectWidth;
	int T;

	//initialize variables
	BITMAP BM;
	((CEnBitmap *)m_ListCEnBitmap.GetAt(0))->GetObject(sizeof(BM), &BM);			// Retrieve size of Bitmap
	rectHeight=  m_height;
	rectWidth= m_width;
	T=0;

	BITMAPINFOHEADER bih;
	
	// Initialize the bitmapinfoheader
	bih.biSize               = sizeof(BITMAPINFOHEADER);
	bih.biWidth              = m_width ;
	bih.biHeight             = 	m_height;
	bih.biPlanes             = 1;
	bih.biBitCount           = BM.bmPlanes * BM.bmBitsPixel;
	bih.biSizeImage          = 0;
	bih.biXPelsPerMeter      = 0;
	bih.biYPelsPerMeter      = 0;
	bih.biClrUsed            = 0;
	bih.biClrImportant       = 0;

	// set the bits
	HDC hdc = GetDC(NULL);
	HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);

	BOOL bRes = TRUE;

	// initialize with the color of the background.
	RGBX* pDestPixels = new RGBX[bih.biWidth * bih.biHeight];
	for (int i=0;i<(bih.biWidth * bih.biHeight);i++)
	{
		pDestPixels[i].btBlue=	GetBValue(m_crBkgnd) ;
		pDestPixels[i].btGreen=GetGValue(m_crBkgnd);
		pDestPixels[i].btRed=GetRValue(m_crBkgnd);
	}

	for (int i=0;i<m_ListCEnBitmap.GetCount();i++) // for each image in the list
	{
		

		//get the pixel value of the current image
		RGBX* pSrcPixels = ((CEnBitmap *)m_ListCEnBitmap.GetAt(i))->GetDIBits32();
		BITMAP BMSRC;	
	
		// get the size of the current image.
		((CEnBitmap *)m_ListCEnBitmap.GetAt(i))->GetObject(sizeof(BMSRC), &BMSRC);

		for (int stepY=0;stepY<BMSRC.bmHeight ;stepY++) // for each line
		{
			for (int stepX=0;stepX<BMSRC.bmWidth ;stepX++) // for each columns
			{		
					int dst=0, src=0;	
					dst = T + stepY * rectWidth + stepX; 
					src = stepY * BMSRC.bmWidth + stepX; // The position of the current value is the number of line  calculated * the width + the number of columns calculated.
					if (dst<(rectWidth*rectHeight))						
						pDestPixels[dst] = pSrcPixels[src];
					else 
					{
						i=m_ListCEnBitmap.GetCount();
						stepY=BMSRC.bmHeight;
						stepX=BMSRC.bmWidth;
					}
					//TRACE("dst : %d \n",dst);
			}
		}
		T += BMSRC.bmWidth;  
		free(pSrcPixels);
	}

	// attach the bitmap.
	if (hbmSrc)
	{
		BITMAPINFO bi;

		if (PrepareBitmapInfo32(bi, hbmSrc))
		{
			if (SetDIBits(hdc, hbmSrc, 0, bih.biHeight, pDestPixels, &bi, DIB_RGB_COLORS))
			{
				// delete the bitmap and attach new
				DeleteObject();
				bRes = Attach(hbmSrc);
			}
		}
		// delete the temporary elements.
		::ReleaseDC(NULL, hdc);

		if (!bRes)
			::DeleteObject(hbmSrc);
	}

	
	delete[] pDestPixels;
}

BOOL CEnBitmapRow::FlipImage(int pos, BOOL bHorz, BOOL bVert)
{
	return ((CEnBitmap *)m_ListCEnBitmap.GetAt(pos))->FlipImage(bHorz,bVert); // flip the element in the pos in the list.
}