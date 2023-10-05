/*
$Revision:   1.5  $
*/


#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <ctype.h>
#include <toolbox.h>
#include <limits.h>
#include <mbstring.h>

// #ifdef NOASSEMBLER

/*
 *              memcpy for far strings
 */
LPVOID DLLENTRY MemCpy(LPVOID sv1, const void FAR* sv2, WORD n)
{
  LPSTR  s1 = (LPSTR)sv1;
  LPCSTR s2 = (LPCSTR)sv2;

  LPSTR   s = sv1;

  while (n--)
    *s1++ = *s2++;

  return s;
}


/*
 *              memset for far strings
 */



LPVOID DLLENTRY MemSet(LPVOID sv1, char c, WORD n)
{
  LPSTR   s1 = sv1;
  LPSTR   s = sv1;

  while (n--)
    *s1++ = c;

  return s;
}


// #endif  /* #ifdef NOASSEMBLER   */


/*
 *    atoi for far strings
 */
// rdw - rewrite to detect overflow 05/10/93
TCHAR chZero = '0';  // prevent optimization that causes overflow

int   DLLENTRY StringToInt(str)
LPCTSTR str;
{
   int   val;
   BOOL  neg;

   while (_istspace (*str))
      str++;

   str += (neg = *str == '-'); 

   val = 0;
   while(_istdigit (*str))
      {
      if (val < TB_INT_MAX / 10)
   val = val*10 + (int)(*str++ - '0');
      else if (val == TB_INT_MAX / 10) 
   {
   val = val*10;
   if (val <= TB_INT_MAX - (int)(*str - chZero))
      val = val + (int)(*str++ - '0');
   else // overflow detected
      return TB_INT_OVERFLOW;  
   }
      else // overflow detected
   return TB_INT_OVERFLOW;     
      }

   return neg ? -val : val;
}

/*
 *    atol for far strings
 */
// rdw - rewrite to detect overflow 05/10/93
LONG  DLLENTRY StringToLong(str)
LPCTSTR str;
{
   LONG  val;
   BOOL  neg;

   while (_istspace (*str))
      str++;

   str += (neg = *str == '-'); 

   val = 0;
   while(_istdigit (*str))
      {
      if (val < TB_LONG_MAX / 10)
       val = val*10 + (int)(*str++ - '0');
      else if (val == TB_LONG_MAX / 10) 
   {
   val = val*10;
   if (val <= TB_LONG_MAX - (int)(*str - chZero))
      val = val + (int)(*str++ - '0');
   else
      return TB_LONG_OVERFLOW; // overflow detected
   }
      else
   return TB_LONG_OVERFLOW; // overflow detected
      }

   return neg ? -val : val;
}

/*
 *    itoa for far strings
 */
LPTSTR   DLLENTRY IntToString(Val, Str)
int      Val;
LPTSTR   Str;
{
   LPTSTR s = Str;
   BOOL   neg;

   if (neg = Val < 0)
      Val = -Val;
   while (Val || s==Str)
   {
      *Str++ = (TCHAR)(Val%10 + (int)'0');
      Val/=10;
   }
   if (neg)
      *Str++ = '-';
   *Str = '\0';

   return StrRev (s);
}

/*
 *    ltoa for far strings
 */
LPTSTR DLLENTRY LongToString (Val, Str)
LONG    Val;
LPTSTR  Str;
{
   LPTSTR s = Str;
   BOOL   neg;

   if (neg = Val < 0)
      Val = -Val;
   while (Val || s==Str)
   {
      *Str++ = (TCHAR)(Val%10 + (long)'0');
      Val/=10;
   }
   if (neg)
      *Str++ = '-';
   *Str = '\0';

   return StrRev (s);
}

#if defined(_WIN64) || defined(IA64)
/*
 *    ltoa for far strings
 */
LPTSTR DLLENTRY LongPtrToString (Val, Str)
LONG_PTR Val;
LPTSTR  Str;
{
   LPTSTR s = Str;
   BOOL   neg;

   if (neg = Val < 0)
      Val = -Val;
   while (Val || s==Str)
   {
      *Str++ = (TCHAR)(Val%10 + (long)'0');
      Val/=10;
   }
   if (neg)
      *Str++ = '-';
   *Str = '\0';

   return StrRev (s);
}
#endif
/*
 *              Remove leading and trailing blanks
 */
LPTSTR  DLLENTRY StrTrim (Str)
LPTSTR  Str;
{
  return StrLTrim (StrRTrim (Str));
}

/*
 *    Remove Leading blanks
 */
LPTSTR  DLLENTRY StrLTrim(Str)
LPTSTR  Str;
{
  LPTSTR  s = Str;

#ifdef _UNICODE
  while (_istspace (*Str))
    Str++;
#else
  BOOL fDone = FALSE;
// 96' 5/16 BOC Gao. for a problem in case of Kanji.
//  while (isspace (*Str) && !IsDBCSLeadByte(*Str))
//    Str++;

  while (!fDone)
  {
    if (_TIsDBCSLeadByte((BYTE)*Str))
    {
      if (_ismbcspace(MAKEWORD((BYTE)*(Str+1), (BYTE)*Str)))
        Str += 2;
      else
        fDone = TRUE;
    }
    else if (_istspace(*Str))
      Str++;
    else
      fDone = TRUE;
  }
#endif
// -----------------------------------------------<<

if (s != Str)  
  return lstrcpy (s, Str);
else
  return (s);
}

/*
 *              Remove trailing blanks
 */
LPTSTR  DLLENTRY StrRTrim(Str)
LPTSTR  Str;
{
  LPTSTR  s = Str;
  LPTSTR  p;
  int     Len;

  Len = StrLen(Str);
  p = Str+Len-1;

#ifdef _UNICODE
  while(Len-- && _istspace (*p))
	  p--;

  p[1] = '\0';
#else

// 96' 5/16 BOC Gao. for a problem in case of Kanji.
//  while(Len-- && isspace (*p))
//#ifndef SPREAD_JPN
//  while(Len-- && isspace (*p) && !IsDBCSLeadByte(*p))
//	  p--;
//#else
//Modify by BOC 99.6.11 (hyt)--------------------------
//for if first char not DBCS and last char is special kanji
//like "Œã" it will trim  error
  p = NULL;
  while(*s!='\0' && Len--)
  {
	  if(_TIsDBCSLeadByte((BYTE)*s))
	  {
       if (_ismbcspace(MAKEWORD((BYTE)*(s+1), (BYTE)*s)))
       {
         if(!p)
			  p = s;
       }
       else
		   p = NULL;

		 s++;
		 Len--;
	  }
	  else if(_istspace(*s))
	  {
		  if(!p)
			  p = s;
	  }
	  else
		  p = NULL;
	  s++;
  }
  if(p)
	  *p='\0';
  return Str;
//--------------------------------------------------------------
//#endif
#endif
// -----------------------------------------------<<
	
  return s;
}


LPTSTR StrSkipSpace(LPTSTR Str)
{
LPTSTR  s = Str;

while(*s!='\0')
	{
	if((_TIsDBCSLeadByte((BYTE)*s) && _ismbcspace(MAKEWORD((BYTE)*(s+1), (BYTE)*s))) || _istspace(*s))
		s = CharNext(s);
	else
		break;
	}

return s;
}

/*
 * Remove spaces between words so there is only one space between words.
 */
LPTSTR DLLENTRY StrTrimAll(LPTSTR Str)
{
LPTSTR  s;
LPTSTR  p;
BOOL    fStrip = FALSE;

Str = StrTrim(Str);
s = Str;

while(*s!='\0')
	{
	if(_TIsDBCSLeadByte((BYTE)*s))
		{
		if (_ismbcspace(MAKEWORD((BYTE)*(s+1), (BYTE)*s)))
			fStrip = TRUE;
		}
	else if(_istspace(*s))
		fStrip = TRUE;

	s = CharNext(s);

	if (fStrip)
		{
		fStrip = FALSE;
		p = StrSkipSpace(s);

		if (p != s)
			lstrcpy(s, p);
		}
	}

return Str;
}


int DLLENTRY   StrnCmp (lpStr1, lpStr2, Length)
LPCTSTR lpStr1;
LPCTSTR lpStr2;
int     Length;
{
   if (!Length)
      return 0;

    while (--Length && *lpStr1 && *lpStr2 && *lpStr1 == *lpStr2)
    {
    lpStr1++;
    lpStr2++;
    }

   return(*lpStr1 - *lpStr2);
}


int DLLENTRY   StrnCmpi (lpStr1, lpStr2, Length)
LPCTSTR lpStr1;
LPCTSTR lpStr2;
int     Length;
{
   if (!Length)
      return 0;

    while (--Length && *lpStr1 && *lpStr2 && toupper(*lpStr1) == toupper(*lpStr2))
    {
    lpStr1++;
    lpStr2++;
    }

   return(toupper(*lpStr1) - toupper(*lpStr2));
}

LPTSTR DLLENTRY StrpBrk(lpStr,lpChars)
LPCTSTR lpStr;
LPCTSTR lpChars;
{
   while(*lpStr && !StrChr(lpChars,*lpStr))
      lpStr++;

   if (!*lpStr)
      return NULL;

   return (LPTSTR)lpStr;
}


HPVOID DLLENTRY MemHugeCpy(HPVOID hpMem1, HPCVOID hpMem2, long Size)
{
#ifdef WIN32

return _fmemcpy(hpMem1, hpMem2, (size_t)Size);

#else //WIN16

char huge *lpMem1 = hpMem1;
const char huge *lpMem2 = hpMem2;
char  huge * p = lpMem1;

while(Size--)
   *lpMem1++ = *lpMem2++;

return p;

#endif
}


HPVOID   DLLENTRY MemHugeSet(HPVOID hpMem, char Value, WORD Size)
{
#ifdef WIN32
return _fmemset(hpMem, (BYTE)Value, (size_t)Size);

#else
   HPBYTE hpMem1 = hpMem;
   HPBYTE p = hpMem;

   while(Size--)
      *hpMem1++ = (BYTE)Value;

   return p;
#endif
}
