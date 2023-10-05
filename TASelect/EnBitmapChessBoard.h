
#include "EnBitmap.h"


class CEnBitmapChessBoard: public CEnBitmap
{
// this class is used to work with a kind of CEnBitmap chessboard.
// the number of line and column are a variable set with the constructor or update with SetDimension
// the size of a standard image is the size of the first image in the list.
// the number of columns or line begin with 0. (For the loop, with start at the position 0 to length-1)
// After a modification of a subimage you need to update the global image by call Compute_CEnBitmap.
// if an image is greater than the first image in the list then the pixels outside the range of the 
// first image are not taken in the calculation.
// After a modification of a subimage you need to update the global image by call Compute_CEnBitmap.

public:
	// constructor
	// set a chessboard with l lines and c columns with a CenBitmap in color crbkgnd for each squares.
	CEnBitmapChessBoard(int l,int c,COLORREF crBkgnd= RGB(255, 255, 255),int height=0,int width=0);
	// set a chessboard with l lines and c columns with the reference image (refCenBitmap) for each squares.
	CEnBitmapChessBoard(int l,int c,CEnBitmap* refCEnBitmap,COLORREF crBkgnd= RGB(255, 255, 255));
	// set a chessboard with l lines and c columns with the reference image (szImagePath) for each squares.
	CEnBitmapChessBoard(int l,int c,LPCTSTR szImagePath, COLORREF crBack = -1); 
	// set a chessboard with l lines and c columns with the reference image (see the function (loadimage) in the CEnBitmap) for each squares.
	CEnBitmapChessBoard(int l,int c,UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst = NULL, COLORREF crBack = -1); 
	
	virtual ~CEnBitmapChessBoard();

	int GetLine(){return m_line;};
	int GetColumn(){return m_column;};
	void SetDimension(int l,int c){m_line=l;m_column=c;};

	// add a CEnBitmap in the list of image (in the last postion)
	void AddCEnBitmap(CEnBitmap* newCEnBitmap);
	// Replace the element in the line and col with the new element (newCEnBitmap).
	// WARNING : line and col begin with '0'
	void AddCEnBitmap(int line,int col,CEnBitmap* newCEnBitmap);
	void AddCEnBitmap(int line,int col,UINT uIDRes, LPCTSTR szResourceType, HMODULE hInst = NULL, COLORREF crBack = -1); 
	// Replace the 'pos'th element in the list with the new element (newCEnBitmap).
	void ReplaceCEnBitmap(int pos,CEnBitmap* newCEnBitmap);
	// rebuild the global image. To take effect of modification.
	void Compute_CEnBitmap();

	// make a rotation of the element in the line and in the col of nDegrees.
	// WARNING : line and col begin with '0'
	void RotateImage(int line,int col,int nDegrees, BOOL bEnableWeighting=0);
	// Flip the element in the line and in the col.
	// WARNING : line and col begin with '0'
	BOOL FlipImage(int line,int col, BOOL bHorz, BOOL bVert = 0);
	

private:
	CArray<LPARAM> m_ListCEnBitmap;	
	int m_line, m_column;
};
