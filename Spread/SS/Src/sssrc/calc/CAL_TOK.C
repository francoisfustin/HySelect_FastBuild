//--------------------------------------------------------------------
//
//  File: cal_tok.c
//
//  Description: Expression parsing routines.
//

#include <ctype.h>
#include <string.h>

#include "calc.h"
#include "cal_mem.h"
#include "cal_oper.h"
#include "cal_tok.h"
#include "toolbox.h"

//--------------------------------------------------------------------

#define TOKENARRAY_INCSIZE 32

//--------------------------------------------------------------------
//
//  The TokenArrayInit() function initializes a token array.  If
//  succesful, the function returns TRUE.  Otherwise, the function
//  returns FALSE.
//

BOOL TokenArrayInit(LPCALC_TOKENARRAY lpArray)
{
  lpArray->nSize = 0;
  lpArray->nMaxSize = 0;
  lpArray->hItem = 0;
  lpArray->lpItem = 0;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The TokenArrayFree() function frees all memory associated with
//  the token array.
//

BOOL TokenArrayFree(LPCALC_TOKENARRAY lpArray)
{
  if( lpArray->lpItem )
    CalcMemUnlock(lpArray->hItem);
  if( lpArray->hItem )
    CalcMemFree(lpArray->hItem);
  lpArray->nSize = 0;
  lpArray->nMaxSize = 0;
  lpArray->hItem = 0;
  lpArray->lpItem = 0;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The TokenArrayPeek() function retrieves a token from the top of
//  the stack.  The stack is not modified (ie. the token remains on
//  the stack).  If successful, the function returns TRUE.  Otherwise,
//  the function returns FALSE (ie. when the stack is empty).
//

static BOOL TokenArrayPeek(LPCALC_TOKENARRAY lpStack, LPCALC_TOKEN lpItem)
{
  BOOL bResult = FALSE;
  
  if( lpStack->nSize > 0 )
  {
    *lpItem = lpStack->lpItem[lpStack->nSize-1];
    bResult = TRUE;
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The TokenArrayPop() function removes a token from the top of the
//  stack.  If successful, the functions returns TRUE.  Otherwise,
//  the function returns FALSE (ie. when stack underflow occurs).
//

static BOOL TokenArrayPop(LPCALC_TOKENARRAY lpStack, LPCALC_TOKEN lpItem)
{
  BOOL bResult = FALSE;
  
  if( lpStack->nSize > 0 )
  {
    lpStack->nSize--;
    *lpItem = lpStack->lpItem[lpStack->nSize];
    bResult = TRUE;
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The TokenArrayPush() function adds a token to the top of the
//  stack.  If successful, the functions returns TRUE.  Otherwise,
//  the function returns FALSE (ie. when stack overflow occurs).
//

static BOOL TokenArrayPush(LPCALC_TOKENARRAY lpStack, LPCALC_TOKEN lpItem)
{
  BOOL bResult = FALSE;

  if( lpStack->nSize >= lpStack->nMaxSize )
  {
    lpStack->nMaxSize += TOKENARRAY_INCSIZE;
    if( lpStack->lpItem )
      CalcMemUnlock(lpStack->hItem);
    if( lpStack->hItem )
      lpStack->hItem = CalcMemReAlloc(lpStack->hItem,lpStack->nMaxSize*sizeof(CALC_TOKEN));
    else
      lpStack->hItem = CalcMemAlloc(lpStack->nMaxSize*sizeof(CALC_TOKEN));
    if( !lpStack->hItem || !(lpStack->lpItem = CalcMemLock(lpStack->hItem)) )
    {
      lpStack->nSize = 0;
      lpStack->nMaxSize = 0;
      lpStack->hItem = 0;
      lpStack->lpItem = 0;
    }
  }  
  if( lpStack->nSize < lpStack->nMaxSize )
  {
    lpStack->lpItem[lpStack->nSize] = *lpItem;
    lpStack->nSize++;
    bResult = TRUE;
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The TokenArrayClear() function removes all items from the stack.
//

static void TokenArrayClear(LPCALC_TOKENARRAY lpStack)
{
  lpStack->nSize = 0;
}

//--------------------------------------------------------------------
//
//  The SepInfo[] array holds information on allowable separators.
//
//  Note: The order of the SepInfo table must match the order of the
//        CALC_SEP_? constants.  This allows quicker lookups.
//

const CALC_SEPARATOR SepInfo[] =
{
  {_T(""),  CALC_SEP_NULL},
  {_T("("), CALC_SEP_LPAR},
  {_T(")"), CALC_SEP_RPAR},
  {_T(","), CALC_SEP_COMMA},
};

const int sizeSepInfo = sizeof(SepInfo) / sizeof(SepInfo[0]);

//--------------------------------------------------------------------
//
//  The SepGetText() function retrieves the text representation of
//  the given separator.
//

int SepGetText(int nId, LPTSTR lpszText, int nLen)
{
  int nSepLen;
  int nResult = 0;
  
  if( 0 < nId && nId < sizeSepInfo )
  {
    nSepLen = lstrlen(SepInfo[nId].lpszSep);
    if( nSepLen < nLen )
    {
      lstrcpy(lpszText, SepInfo[nId].lpszSep);
      nResult = nSepLen;
    }
  }
  return nResult;
}

//--------------------------------------------------------------------
//
//  The SepGetTextLen() function retrieves the length of the text
//  representation of the given separator.
//

int SepGetTextLen(int nId)
{
  int nRet;
  
  if( 0 < nId && nId < sizeSepInfo )
    nRet = lstrlen(SepInfo[nId].lpszSep);
  return nRet;
}

//--------------------------------------------------------------------
//
//  The SetInvalid() function makes the token into an invalid token.
//

static void SetInvalid(LPCALC_TOKEN lpToken)
{
  lpToken->nType = CALC_TOKEN_INVALID;
}

//--------------------------------------------------------------------
//
//  The GetSepInfo() function retrieves a pointer to the information
//  for the given separator.
//

static const CALC_SEPARATOR FAR* GetSepInfo(int nId)
{
  int i;
  
  for( i = 0; i < sizeSepInfo; i++ )
    if( nId == SepInfo[i].nId )
      return &SepInfo[i];
  return NULL;
}

//--------------------------------------------------------------------
//
//  The LookupOper() function determines if the given text represents
//  a valid operator.  If valid, the function also determines the type
//  and precedence of the operator.
//
//  Note: It is assumed that operators are at most 2 characters long.
//

static BOOL LookupOper(LPCALC_TOKEN lpToken, BOOL bBinary)
{
  TCHAR szText[3];
  int nId;
  int nLen;
  BOOL bRet = FALSE;

  nLen = min(lpToken->nLen, sizeof(szText)/sizeof(TCHAR) - 1);  
  lstrcpyn(szText, lpToken->lpszText, nLen + 1);
  szText[nLen] = '\0';
  if( nId = OperLookup(szText, bBinary) )
  {
    lpToken->u.Oper.nId = nId;
    lpToken->u.Oper.nPred = OperGetPred(nId);
    bRet = TRUE;
  }
  else
    SetInvalid(lpToken);
  return bRet;
}

//--------------------------------------------------------------------
//
//  The LookupSep() function determines if the text represents a
//  valid separator.  If valid, the function also determines the
//  type of separator.
//
//  Note: It is assumed that all separators are single characters.
//

static BOOL LookupSep(LPCALC_TOKEN lpToken)
{
  int i;
  
  for(i = 0; i < sizeSepInfo; i++)
    if( lpToken->lpszText[0] == SepInfo[i].lpszSep[0] )
    {
      lpToken->u.Sep.nId = SepInfo[i].nId;
      return TRUE;
    }
  SetInvalid(lpToken);
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The IsNameChar() function determines if the given character is
//  a valid name character.
//
//  Note: A# style cell references use '#' to indicate a relative
//        cell reference.
//
//  Note: A1 style cell references use '$' to indicate a absolute
//        cell reference.
//
//  Note: R1C1 style cell references use '[' and ']' to indicate
//        a relative cell reference.
//
//  Note: External cell references use '!' to separate sheet name
//        from cell name.
//

static BOOL IsNameChar(TCHAR ch)
{
  return _istalnum((_TUCHAR)ch) || ch == '_'
         || ch == '$' || ch == '[' || ch == ']'
         || ch == '#' || ch == '!' || ch == '.';
}

//--------------------------------------------------------------------
//
//  The IsSepChar() function determines if the given character is
//  a valid separator character.
//

static BOOL IsSepChar(TCHAR ch)
{
  return ch == '(' || ch == ')' || ch == ',';
}

//--------------------------------------------------------------------
//
//  The IsOperChar() function determines if the given character is
//  a valid operator character.
//

static BOOL IsOperChar(TCHAR ch)
{
  return ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
         ch == '^' || ch == '&' || ch == '|' || ch == '!' ||
         ch == '<' || ch == '>' || ch == '=' || ch == ':';
}

//--------------------------------------------------------------------
//
//  The StartsWithError() function determines if the given text
//  starts with an error code.
//

static LPCTSTR StartsWithError(LPCTSTR lpszText)
{
  static LPCTSTR szErrors[] = {_T("#DIV/0!"), _T("#N/A"), _T("#NAME?"), _T("#NULL!"), _T("#NUM!"), _T("#REF!"), _T("#VALUE!")};
  int i;

  for (i = 0; i < sizeof(szErrors) / sizeof(szErrors[0]); i++)
    if (_tcsnicmp(lpszText, szErrors[i], _tcslen(szErrors[i])) == 0)
      return szErrors[i];
  return NULL;
}

//--------------------------------------------------------------------
//
//  The ParseNextToken() function finds the next token in the given
//  text.  If a token is found then the function returns the position
//  of the first unparsed character and lpToken contains the position,
//  length, and type of the token. If no token is found, the function
//  returns NULL.
//

static LPTSTR ParseNextToken(LPCTSTR lpszText, BOOL bAllowBinOp,
                             LPCALC_TOKEN lpToken)
{
  int nLen = 0;                 // length of token
  int nLenExtra = 0;            // length of extra characters after token
  int nType = CALC_TOKEN_NULL;  // type of token
  LPCTSTR lpszError = NULL;

  while( _istspace((_TUCHAR)*lpszText) )
    lpszText++;
  if( *lpszText == '\0' )
  {
    lpszText = NULL;
  }
  else if( _istdigit((_TUCHAR)*lpszText) || *lpszText == '.' )
  {
    nType = CALC_TOKEN_LONG;
    while( _istdigit((_TUCHAR)lpszText[nLen]) )
      nLen++;
    if( lpszText[nLen] == '.' )
    {
      nLen++;
      while( _istdigit((_TUCHAR)lpszText[nLen]) )
        nLen++;
      nType = CALC_TOKEN_DOUBLE;
    }
    if( lpszText[nLen] == 'e' || lpszText[nLen] == 'E' )
    {
      nLen++;
      if( lpszText[nLen] == '-' || lpszText[nLen] == '+' )
        nLen++;
      while( _istdigit((_TUCHAR)lpszText[nLen]) )
        nLen++;
      nType = CALC_TOKEN_DOUBLE;
    }
  }
  else if (lpszError = StartsWithError(lpszText))
  {
    nLen = (int)_tcslen(lpszError);
    nType = CALC_TOKEN_ERROR;
  }
  else if (_istalpha((_TUCHAR)*lpszText) || *lpszText == '_' ||
           *lpszText == '$'  || *lpszText == '#' || *lpszText == '\'' ||
           (_TIsDBCSLeadByte((BYTE)*lpszText) && _istalpha(MAKEWORD((BYTE)*(lpszText+1), (BYTE)*lpszText))))
  {
    if (*lpszText == '\'')
    {
      nLen++;
      while ((lpszText[nLen] && lpszText[nLen] != '\'') || (lpszText[nLen] == '\'' && lpszText[nLen+1] == '\''))
      {
        if (lpszText[nLen] == '\'')
          nLen++;
        nLen++;
      }
      if (lpszText[nLen] == '\'')
        nLen++;
    }
    while (IsNameChar(lpszText[nLen]) || (_TIsDBCSLeadByte(lpszText[nLen]) && _istalnum(MAKEWORD((BYTE)lpszText[nLen+1], (BYTE)lpszText[nLen]))))
    {
      if (lpszText[nLen] == '[' && lpszText[nLen+1] == '-')
        nLen++;
      else if (_TIsDBCSLeadByte((BYTE)lpszText[nLen]))
        nLen++;
      nLen++;
    }
    if (lpszText[nLen] == '(') 
    {
      nLenExtra = 1;
      nType = CALC_TOKEN_FUNCTION;
    }
    else
      nType = CALC_TOKEN_NAME;
  }
  else if( *lpszText == '"' )
  {
    lpszText++;
    while( lpszText[nLen] && lpszText[nLen] != '"' )
      nLen++;
    if( lpszText[nLen] == '"' )
    {
      nLenExtra = 1;
      nType = CALC_TOKEN_STRING;
    }
    else
    {
      lpszText--;
      nLen = 1;
      nLenExtra = 0;
      nType = CALC_TOKEN_INVALID;
    }
  }
  else if( IsSepChar(*lpszText) )
  {
    nLen = 1;
    nType = CALC_TOKEN_SEPARATOR;
  }
  else if( IsOperChar(*lpszText) )
  {
    BOOL bTwoChar;
    bTwoChar = lpszText[0] == '<' && lpszText[1] == '>';
    bTwoChar |= lpszText[0] == '<' && lpszText[1] == '=';
    bTwoChar |= lpszText[0] == '>' && lpszText[1] == '=';
    nLen = bTwoChar ? 2 : 1;
    nType = CALC_TOKEN_OPERATOR;
  }
  else
  {
    nLen = 1;
    nType = CALC_TOKEN_INVALID;
  }
  lpToken->lpszText = (LPTSTR)lpszText;
  lpToken->nLen = nLen;
  lpToken->nType = nType;
  if( nType == CALC_TOKEN_SEPARATOR )
    LookupSep(lpToken);
  else if( nType == CALC_TOKEN_OPERATOR )
    LookupOper(lpToken, bAllowBinOp);
  return (LPTSTR)(lpszText + nLen + nLenExtra);
}

//--------------------------------------------------------------------
//
//  The TransferOper() function transfers tokens from the temporary
//  stack to the primary stack when an operator is encountered in
//  the expression being parsed.
//

static void TransferOper(int nPred,
                         LPCALC_TOKENARRAY lpStack,
                         LPCALC_TOKENARRAY lpTemp)
{
  CALC_TOKEN token;
  
  while( TokenArrayPeek(lpTemp, &token)
         && token.nType == CALC_TOKEN_OPERATOR
         && token.u.Oper.nPred >= nPred )
  {
    TokenArrayPop(lpTemp, &token);
    TokenArrayPush(lpStack, &token);
  }
}

//--------------------------------------------------------------------
//
//  The TransferRpar() function transfers tokens from the temporary
//  stack to the primary stack when a ")" is encountered in the
//  expression being parsed.  If a matching function or "(" is found,
//  the function returns TRUE.  Otherwise, the function returns FALSE
//

static BOOL TransferRpar(LPCALC_TOKENARRAY lpStack, LPCALC_TOKENARRAY lpTemp,
                         int nPrevType, LPCALC_TOKEN lpCurrent)
{
  CALC_TOKEN token;
  CALC_TOKEN tokenComma;
  BOOL bError = FALSE;
  BOOL bMatchFound = FALSE;
  int nCommas = 0;
  int nMinArgs = nPrevType == CALC_TOKEN_FUNCTION ? 0 : 1;
  
  while( !bError && !bMatchFound && TokenArrayPop(lpTemp,&token) )
  {
    switch( token.nType )
    {
      case CALC_TOKEN_OPERATOR:
        TokenArrayPush(lpStack, &token);
        break;
      case CALC_TOKEN_SEPARATOR:
        if( token.u.Sep.nId  == CALC_SEP_COMMA )
        {
          tokenComma = token;
          nCommas++;
        }
        else if( token.u.Sep.nId == CALC_SEP_LPAR )
        {
          if( nCommas == 0 )
          {
            TokenArrayPush(lpStack, lpCurrent);
            bMatchFound = TRUE;
          }
          else
          {
            SetInvalid(&tokenComma);
            *lpCurrent = tokenComma;
            bError = TRUE;
          }
        }
        break;
      case CALC_TOKEN_FUNCTION:
        token.u.Func.nArgs = nMinArgs + nCommas;
        TokenArrayPush(lpStack, &token);
        bMatchFound = TRUE;
        break;
    }
  }
  if( !bMatchFound )
    SetInvalid(lpCurrent);
  return !bError && bMatchFound;
}

//--------------------------------------------------------------------
//
//  The TransferComma() function transfers tokens from the temporary
//  stack to the primary stack when a "," is encountered in the
//  expression being parsed.  All operators between the current ","
//  and previous function, "(", or ","  are moved to the primary stack.
//

static void TransferComma(LPCALC_TOKENARRAY lpStack, LPCALC_TOKENARRAY lpTemp)
{
  CALC_TOKEN token;
  
  while( TokenArrayPeek(lpTemp, &token) && token.nType == CALC_TOKEN_OPERATOR )
  {
    TokenArrayPop(lpTemp, &token);
    TokenArrayPush(lpStack, &token);
  }
}

//--------------------------------------------------------------------
//
//  The TransferEnd() function transfers tokens from the temporary
//  stack to the primary stack when the end of the expression being
//  parsed is encountered.  If successful the function returns TRUE.
//  Otherwise, the functions returns FALSE and lpError contains the
//  token in question.  Reasons for failure include an unclosed
//  function, an unclosed "(", or a "," separator outside of a
//  function call.
//

static BOOL TransferEnd(LPCALC_TOKENARRAY lpStack, LPCALC_TOKENARRAY lpTemp,
                        LPCALC_TOKEN lpError)
{
  CALC_TOKEN token;
  BOOL bResult = TRUE;
  
  while( TokenArrayPop(lpTemp,&token) )
  {
    if( token.nType == CALC_TOKEN_OPERATOR )
      TokenArrayPush(lpStack, &token);
    else
    {
      SetInvalid(&token);
      *lpError = token;
      bResult = FALSE;
    }
  }
  return bResult;
}

//--------------------------------------------------------------------
//
//  The TokenParse() function parses the given text into a stack of
//  tokens.  If sucessful, the function returns TRUE and the lpStack
//  array contains a reverse polish representation of the orginal
//  expression.  Otherwise, the function returns FALSE and the lpStack
//  array contains a single error token.
//
//  A temporary stack is used to hold operators and functions until
//  there place in the reverse polish representation can be found.
//  The temporary stack also holds "(" and "," separator characters
//  until the closing ")" is found.
//
//  Setting bAllowBinOp = TRUE indicates that a operand was been
//  encountered and that binary operators and the "," separator are
//  now allowed.
//
//  Setting bNeedOperand = TRUE indicates that a binary operator or
//  "," separator has been encountered and that a operand is now
//  needed.
//

BOOL TokenParse(LPCTSTR lpszText, LPCALC_TOKENARRAY lpStack)
{
  CALC_TOKEN token;
  CALC_TOKENARRAY temp;
  BOOL bAllowBinOp = FALSE;
  BOOL bNeedOperand = FALSE;
  BOOL bError = FALSE;
  int nPrevType = CALC_TOKEN_NULL;

  if( TokenArrayInit(&temp) )
  {
    while( !bError && (lpszText=ParseNextToken(lpszText,bAllowBinOp,&token)) )
    {
      switch(token.nType)
      {
        case CALC_TOKEN_LONG:
        case CALC_TOKEN_DOUBLE:
        case CALC_TOKEN_STRING:
        case CALC_TOKEN_NAME:
        case CALC_TOKEN_ERROR:
          if( bAllowBinOp )
          {
            SetInvalid(&token);
            bError = TRUE;
          }
          else
            TokenArrayPush(lpStack, &token);
          bAllowBinOp = TRUE;
          bNeedOperand = FALSE;
          break;
        case CALC_TOKEN_FUNCTION:
          TokenArrayPush(&temp, &token);
          bAllowBinOp = FALSE;
          bNeedOperand = FALSE;
          break;
        case CALC_TOKEN_OPERATOR:
          if( bAllowBinOp )
            TransferOper(token.u.Oper.nPred, lpStack, &temp);
          TokenArrayPush(&temp, &token);
          bAllowBinOp = FALSE;
          bNeedOperand = TRUE;
          break;
        case CALC_TOKEN_SEPARATOR:
          switch( token.u.Sep.nId )
          {
            case CALC_SEP_LPAR:
              if( bAllowBinOp )
              {
                SetInvalid(&token);
                bError = TRUE;
              }
              else
              {
                TokenArrayPush(lpStack, &token);
                TokenArrayPush(&temp, &token);
              }
              bAllowBinOp = FALSE;
              bNeedOperand = TRUE;
              break;
            case CALC_SEP_COMMA:
              if( bAllowBinOp )
              {
                TransferComma(lpStack, &temp);
                TokenArrayPush(&temp, &token);
              }
              else
              {
                SetInvalid(&token);
                bError = TRUE;
              }
              bAllowBinOp = FALSE;
              bNeedOperand = TRUE;
              break;
            case CALC_SEP_RPAR:
              if( bNeedOperand )
              {
                SetInvalid(&token);
                bError = TRUE;
              }
              else if( !TransferRpar(lpStack, &temp, nPrevType, &token) )
                bError = TRUE;
              bAllowBinOp = TRUE;
              bNeedOperand = FALSE;
              break;
          }
          break;
        case CALC_TOKEN_INVALID:
          bError = TRUE;
          break;
      }
      nPrevType = token.nType;
    }
    if( !bError & bNeedOperand )
    {
      SetInvalid(&token);
      bError = TRUE;
    }
    if( !bError && !TransferEnd(lpStack, &temp, &token) )
      bError = TRUE;
    if( bError )
    {
      TokenArrayClear(lpStack);
      TokenArrayPush(lpStack, &token);
    }
    TokenArrayFree(&temp);
  }
  return !bError;
}

//--------------------------------------------------------------------
