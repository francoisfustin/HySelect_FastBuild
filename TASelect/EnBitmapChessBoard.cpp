#include "stdafx.h"
#include "EnBitmapChessBoard.h"
// this class is used to work with a kind of chessboard of CEnBitmap.
// the number of line and column are a variable set with the constructor or update with SetDimension
// the size of a standard image is the size of the first image in the list.
// the number of columns or line begin with 0. (For the loop, with start at the position 0 to length-1)
// After a modification of a subimage you need to update the global image by call Compute_CEnBitmap.
// if an image is greater than the first image in the list then the pixels outside the range of the 
// first image are not taken in the calculation.

CEnBitmapChessBoard::CEnBitmapChessBoard(int l,int c,COLORREF crBkgnd,int height,int width)
{
	m_crBkgnd = crBkgnd;
	m_line = l;
	m_column = c;
	// construct image of reference.
	if (height*width)
	{
		BOOL bRes = TRUE;
		CEnBitmap* cEnBitmapRef=new CEnBitmap(crBkgnd);

		// fill the bitmap with the value of the background;
		RGBX* pDestPixels = new RGBX[height*width];
		for (int i=0;i<(height*width);i++)
		{
			pDestPixels[i].btBlue=	GetBValue(crBkgnd) ;
			pDestPixels[i].btGreen=GetGValue(crBkgnd);
			pDestPixels[i].btRed=GetRValue(crBkgnd);
		}

		HDC hdc = GetDC(NULL);
		// create the bitmap.
		HBITMAP hbmSrc = ::CreateCompatibleBitmap(hdc, width, height);

		if (hbmSrc)
		{
			BITMAPINFO bi;

			if (cEnBitmapRef->PrepareBitmapInfo32(bi, hbmSrc))
			{
				if (SetDIBits(hdc, hbmSrc, 0, height, pDestPixels, &bi, DIB_RGB_COLORS))
				{
					// delete the bitmap and attach new
					DeleteObject();
					bRes = cEnBitmapRef->Attach(hbmSrc);
				}
			}
			// delete temporary components.
			::ReleaseDC(NULL, hdc);
			if (!bRes)
				::DeleteObject(hbmSrc);
			for (int i=0;i<(l*c);i++) // build the list with the image of reference.
				AddCEnBitmap(cEnBitmapRef);

		}
		delete[] pDestPixels;
		cEnBitmapRef->DeleteObject();
	}
}

CEnBitmapChessBoard::CEnBitmapChessBoard(int l,int c,CEnBitmap* refCEnBitmap,COLORREF crBkgnd)
{
	m_crBkgnd = crBkgnd;
	m_line = l;
	m_column = c;
	for (int i=0;i<l*c;i++)  // build the list with the image of reference.
	{
		m_ListCEnBitmap.Add((LPARAM) refCEnBitmap);
	}
}
CEnBitmapChessBoard::CEnBitmapChessBoard(int l,int c,LPCTSTR szImagePath, COLORREF crBack)
{
	m_crBkgnd = crBack;
	m_line = l;
	m_column = c;
	CEnBitmap* refCEnBitmap=new CEnBitmap(crBack); //Set image of reference.
	BOOL flag = refCEnBitmap->Attach(LoadImageFile(szImagePath, crBack == -1 ? m_crBkgnd : crBack));
	for (int i=0;i<l*c;i++)  // build the list with the image of reference.
	{
		m_ListCEnBitmap.Add((LPARAM) refCEnBitmap);
	}
	refCEnBitmap->DeleteObject();
}
CEnBitmapChessBoard::CEnBitmapChessBoard(int l,int c,UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	m_crBkgnd = crBack;
	m_line = l;
	m_column = c;

	CEnBitmap* refCEnBitmap=new CEnBitmap(crBack); // set image of reference.
	refCEnBitmap->Attach(LoadImageResource(uIDRes, szResourceType, hInst, crBack == -1 ? m_crBkgnd : crBack));
	for (int i=0;i<l*c;i++)  // build the list with the image of reference.
	{
		m_ListCEnBitmap.Add((LPARAM) refCEnBitmap);
	}
	refCEnBitmap->DeleteObject();
}
CEnBitmapChessBoard::~CEnBitmapChessBoard()
{
	for (int i=0;i<m_ListCEnBitmap.GetCount();i++)
	{
		((CEnBitmap *)m_ListCEnBitmap.GetAt(i))->DeleteObject();  //delete the object in the list.
	}
	m_ListCEnBitmap.RemoveAll(); // empty the list
}
void CEnBitmapChessBoard::AddCEnBitmap(CEnBitmap* newCEnBitmap){
	CEnBitmap* EnBmp=new CEnBitmap();
	EnBmp->CopyImage(*newCEnBitmap);
	m_ListCEnBitmap.Add((LPARAM) EnBmp); //add the element in the list.
}
void CEnBitmapChessBoard::AddCEnBitmap(int line,int col,CEnBitmap* newCEnBitmap)
{
	int pos = col + line * m_column; // get the position in the list.
	ReplaceCEnBitmap(pos,newCEnBitmap); //replace the element in the list.
}
void CEnBitmapChessBoard::AddCEnBitmap(int line,int col,UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst, COLORREF crBack)
{
	int pos = col + line * m_column; // get the position in the list.
	CEnBitmap EnBmp;
	EnBmp.LoadImage(uIDRes, szResourceType, hInst, crBack); // build the CenBitmap
	ReplaceCEnBitmap(pos,&EnBmp);							//replace the element in the list.
}
void CEnBitmapChessBoard::ReplaceCEnBitmap(int pos,CEnBitmap* newCEnBitmap)
{
	CEnBitmap* EnBmp=new CEnBitmap();
	EnBmp->CopyImage(*newCEnBitmap);
	((CEnBitmap *)m_ListCEnBitmap.GetAt(pos))->DeleteObject();  // delete the old element
	m_ListCEnBitmap.RemoveAt(pos);								// remove the ptr in the list
	m_ListCEnBitmap.InsertAt(pos,(LPARAM) EnBmp);				// add the new element in the good pos.
}
void CEnBitmapChessBoard::Compute_CEnBitmap()
{
	int rectHeight, rectWidth;
	int T;

	//initialize variables
	BITMAP BM;
	((CEnBitmap *)m_ListCEnBitmap.GetAt(0))->GetObject(sizeof(BM), &BM);			// Retrieve size of Bitmap
	rectHeight=  BM.bmHeight;
	rectWidth= BM.bmWidth;
	T=0;

	BITMAPINFOHEADER bih;
	
	// Initialize the bitmapinfoheader
	bih.biSize               = sizeof(BITMAPINFOHEADER);
	bih.biWidth              = rectWidth *m_column ;
	bih.biHeight             = rectHeight *m_line ;
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

		// calculate the position of the top left point of the current image in the global image.

		// ---------------------
		// I   I   I   I   I   I
		// I   I   I   I   I   I
		// Y-------X------------   X is the position of T  
		// I   I   I...I   I   I   The position of Y is : The number of line calculated * the surface of standard image.
		// I   I   I...I   I   I				==>   (column * (i/column)) *  (rectHeight * rectWidth)
		// ---------------------									--> int / int --> int
		// I   I   I   I   I   I	The position of X is : The position of Y + the number of column calculated for this line * the width of standard image.
		// I   I   I   I   I   I
		// ---------------------   
		

		T = rectHeight * rectWidth * m_column * (i/m_column) +  rectWidth * (i%m_column);  

		//get the pixel value of the current image
		RGBX* pSrcPixels = ((CEnBitmap *)m_ListCEnBitmap.GetAt(i))->GetDIBits32();
		BITMAP BMSRC;	
	
		// get the size of the current image.
		((CEnBitmap *)m_ListCEnBitmap.GetAt(i))->GetObject(sizeof(BMSRC), &BMSRC);

		for (int stepY=0;stepY<rectHeight ;stepY++) // for each line
		{
			if (stepY<BMSRC.bmHeight) // if the height of the current image is less than the height of standard square.
			{
				for (int stepX=0;stepX<rectWidth ;stepX++) // for each columns
				{		
					if (stepX<BMSRC.bmWidth) // if the width of the current image is less than the width of standard square.
					{
						int dst=0, src=0;	

						// ---------------------
						// I   I   I   I   I   I
						// I   I   I   I   I   I
						// Y-------T............   
						// I.......X.s.I   I   I The position of the current destination (s, src) is 
						// I   I   I...I   I   I the position of X = T + the number of calculated line (for the current image) * the width * the number of columns in a line 
						// --------------------- The position of s = X + the number of pixel calculated for the current line.
						// I   I   I   I   I   I
						// I   I   I   I   I   I
						// ---------------------   
						dst = T + stepY * rectWidth * m_column + stepX; 
						src = stepY * BMSRC.bmWidth + stepX; // The position of the current value is the number of line  calculated * the width + the number of columns calculated.
						pDestPixels[dst] = pSrcPixels[src];
						//TRACE("dst : %d \n",dst);
					}
					else
						stepX=rectWidth;
				}
			}
			else
				stepY=rectHeight;
		}
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

void CEnBitmapChessBoard::RotateImage(int line,int col,int nDegrees, BOOL bEnableWeighting)
{
	int pos = col + line * m_column; // get the position in the list.
	((CEnBitmap *)m_ListCEnBitmap.GetAt(pos))->RotateImage(nDegrees,bEnableWeighting); // make a rotation of nDegrees of the element in the pos in the list.
}

BOOL CEnBitmapChessBoard::FlipImage(int line,int col, BOOL bHorz, BOOL bVert)
{
	int pos = col + line * m_column; // get the position in the list.
	return ((CEnBitmap *)m_ListCEnBitmap.GetAt(pos))->FlipImage(bHorz,bVert); // flip the element in the pos in the list.
}