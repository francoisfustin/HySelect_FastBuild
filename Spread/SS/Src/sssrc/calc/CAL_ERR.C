//--------------------------------------------------------------------
//
//  File: cal_err.c
//
//  Description: Calculation error codes
//

#include "calc.h"
#include "cal_err.h"

//--------------------------------------------------------------------
//
//  The CALC_ERROR structure holds information about a calculuation
//  error.
//

typedef struct tag_CALC_ERROR
{
  LPTSTR lpszText;
} CALC_ERROR, FAR* LPCALC_ERROR;

//--------------------------------------------------------------------
//
//  The ErrorTable[] array holds information about allowable
//  calculation errors.
//
//  Note: The order of the ErrorTable table must math the order of the
//        CALC_ERR_? constants which are defined in CAL_ERR.h file.
//        This allows quicker lookups.
//
//  Note: Valid calculation errors are always represented by a nonzero
//        id code.  Thus, ErrorTable[0] is not used.
//

const CALC_ERROR ErrorTable[] =
{
  {_T("")},
  {_T("#DIV/0!")},
  {_T("#N/A")},
  {_T("#NAME?")},
  {_T("#NULL!")},
  {_T("#NUM!")},
  {_T("#REF!")},
  {_T("#VALUE!")},
};

const int sizeErrorTable = sizeof(ErrorTable) / sizeof(ErrorTable[0]);

//--------------------------------------------------------------------
//
//  The ErrLookup() function determines if the given text represents
//  a valid calculation error.  If so, the function reurns the id
//  of the  error.  Otherwise, the function returns zero.

int ErrLookup(LPTSTR lpszText, int nTextLen)
{
  int i;

  for (i = 0; i < sizeErrorTable; i++)
    if (_tcsnicmp(ErrorTable[i].lpszText, lpszText, nTextLen) == 0)
      return i;
  return 0;
}

//--------------------------------------------------------------------
//
//  The ErrGetText() function retrieves the text representation of
//  the given calculuation error.
//

int ErrGetText(int nId, LPTSTR lpszText, int nLen)
{
  int nErrorLen;
  int nResult = 0;

  if (0 < nId && nId < sizeErrorTable)
  {
    nErrorLen = (int)_tcslen(ErrorTable[nId].lpszText);
    if (nErrorLen < nLen)
    {
      _tcscpy(lpszText, ErrorTable[nId].lpszText);
      nResult = nErrorLen;
    }
  }
  if (0 == nResult && nLen > 0)
    _tcscpy(lpszText, _T(""));
  return nResult;
}

//--------------------------------------------------------------------
//
//  The ErrGetTextLen() function retrieves the length of the text
//  representation of the given calculation error.
//

int ErrGetTextLen(int nId)
{
  int nResult = 0;

  if (0 < nId && nId < sizeErrorTable)
    nResult = (int)_tcslen(ErrorTable[nId].lpszText);
  return nResult;
}
