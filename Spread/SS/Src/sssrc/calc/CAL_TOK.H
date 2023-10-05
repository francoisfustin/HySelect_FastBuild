//--------------------------------------------------------------------
//
//  File: cal_tok.h
//
//  Description: Expression parsing routines.
//

#ifndef CAL_TOK_H
#define CAL_TOK_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following constants represent the various separator tokens.
//
//  Note: The order of the constants must match the SepInfo array
//        in the CAL_TOK.C file.  This allows quick lookups.
//

#define CALC_SEP_NULL  0
#define CALC_SEP_LPAR  1
#define CALC_SEP_RPAR  2
#define CALC_SEP_COMMA 3

//--------------------------------------------------------------------
//
//  The CALC_SEPARATOR structure holds information about a separator.
//

typedef struct tagCALC_SEPARATOR
{
  LPCTSTR lpszSep;  // text representation of the separator
  int nId;          // id code of the separator
} CALC_SEPARATOR, far *LPCALC_SEPARATOR;

//--------------------------------------------------------------------
//
//  The following functions return information about separator tokens.
//

int SepGetText(int nId, LPTSTR lpszText, int nLen);
int SepGetTextLen(int nId);

//--------------------------------------------------------------------
//
//  The following constants represent the various catagories of
//  tokens.
//

#define CALC_TOKEN_NULL      0
#define CALC_TOKEN_OPERATOR  1
#define CALC_TOKEN_SEPARATOR 2
#define CALC_TOKEN_LONG      3
#define CALC_TOKEN_DOUBLE    4
#define CALC_TOKEN_NAME      5
#define CALC_TOKEN_STRING    6
#define CALC_TOKEN_FUNCTION  7
#define CALC_TOKEN_ERROR     8
#define CALC_TOKEN_INVALID   9

//--------------------------------------------------------------------
//
//  The CALC_TOKEN structure contains information on a single token.
//  The lpszText field points into the orginal string which is being
//  parsed (i.e. lpszText is not NULL terminated and can not be used
//  once the orginal string is deallocated).
//

typedef struct tagCALC_TOKEN
{
  LPTSTR lpszText;
  int nLen;
  int nType;
  union
  {
    struct
    {
      int nId;
      int nPred;
    } Oper;
    struct
    {
      int nId;
    } Sep;
    struct
    {
      int nArgs;
    } Func;
  } u;
} CALC_TOKEN, FAR* LPCALC_TOKEN;

//--------------------------------------------------------------------
//
//  The CALC_TOKENARRAY structure represents a stack (or array) of
//  tokens.  The array automatically expands to hold as many items
//  as needed.
//
//  Note:  The memory will remain locked (i.e. lpItem != NULL)
//         anytime that nSize > 0.
//

typedef struct tabCALC_TOKENARRAY
{
  int nSize;            // current number of items in the array
  int nMaxSize;         // maxium number of items the array can hold
  CALC_HANDLE hItem;    // handle to array of items
  LPCALC_TOKEN lpItem;  // array of items
} CALC_TOKENARRAY, far *LPCALC_TOKENARRAY;

//--------------------------------------------------------------------

BOOL TokenArrayInit(LPCALC_TOKENARRAY lpArray);
BOOL TokenArrayFree(LPCALC_TOKENARRAY lpArray);

//--------------------------------------------------------------------
//
//  The TokenParse() function will parse the given text into a token
//  array.  If successful, the function returns TRUE. Otherwise, the
//  function returns FALSE.
//

BOOL TokenParse(LPCTSTR lpszText, LPCALC_TOKENARRAY lpToken);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
