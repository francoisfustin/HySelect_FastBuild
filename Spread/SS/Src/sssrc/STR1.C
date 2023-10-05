/*
$Revision:   1.5  $
*/


#include	<windows.h>
#include <stdio.h>
#include	<ctype.h>
#include	<toolbox.h>
#include <tchar.h>

//#ifdef NOASSEMBLER



/*
 *    strrchr for far strings
 */
LPTSTR	DLLENTRY StrrChr(s1,c)
LPCTSTR	s1;      
TCHAR		c;        
{
   LPCTSTR s2=s1;

   while(*s2++);
   
   while (*--s2 != c)
      if (s2 == s1)
         return NULL;

	return (LPTSTR)s2;       
}



/*
 *		strrev for far strings
 */
LPTSTR	DLLENTRY StrRev (s1)
LPTSTR	s1;
{
	LPTSTR	s = s1;
   LPTSTR s2 = s1 + StrLen (s1);
   TCHAR  c;

   while (s1 < s2)
   {
      c = *s1;
      *s1++ = *--s2;
      *s2 = c;
   }

	return s;
}


/*
 *		strnicmp for far strings
 */
int	DLLENTRY StrniCmp (s1, s2, len)
LPCTSTR	s1;
LPCTSTR	s2;
int	len;
{
	if (!len--)
		return 0;

	while (len-- && *s1 && *s2 && toupper(*s1) == toupper(*s2))
	{
		s1++; s2++;
	}

	return toupper(*s1) - toupper(*s2);
}


/*
 *		strstr for far strings
 */
LPTSTR	DLLENTRY StrStr (Str, Find)
LPCTSTR Str;
LPCTSTR Find;
{
	int	Len;

	Len = StrLen(Find);
	while (*Str)
		if (!StrnCmp (Str, Find, Len))
			return (LPTSTR)Str;
      else
         Str++;

	return NULL;
}



/*
 *		strlen for far strings
 */
int	DLLENTRY StrLen (s)
LPCTSTR	s;
{
	int	len = 0;

	while (*s++)
		len++;

	return len;
}

/*
 *    strchr for far strings
 */
LPTSTR	DLLENTRY StrChr(s1,c)
LPCTSTR	s1;      
TCHAR     c;        
{
   while (*s1 != c)
      if (!*s1++)
         return NULL;   

	return (LPTSTR)s1;       
}

/*
 *    strichr for far strings
 */
LPTSTR  DLLENTRY StriChr(s1,c)
LPCTSTR	s1;      
TCHAR     c;        
{
   while (tolower(*s1) != tolower(c))
      if (!*s1++)
         return NULL;   

	return (LPTSTR)s1;       
}


/*
 *		strcat for far strings
 */
LPTSTR	DLLENTRY StrCat (s1, s2)
LPTSTR  s1;
LPCTSTR s2;
{
	LPTSTR	s = s1;

	while (*s1) s1++;
	while (*s1++ = *s2++);

	return s;
}


/*
 *		strcmp for far strings
 */

int  DLLENTRY 	StrCmp (s1, s2)
LPCTSTR	s1;
LPCTSTR	s2;
{
	while (*s1 && *s2 && *s1 == *s2)
	{
		s1++; s2++;
	}

	return *s1 - *s2;
}

/*
 *		strtok for far strings
 */
LPTSTR	DLLENTRY StrTok (s1, s2)
LPTSTR  s1;
LPCTSTR s2;
{
	LPTSTR	s;
  static LPTSTR Last = 0L;

	if (s1)			/* new string of tokens */
		Last = s1;

    if (!Last)
       return (NULL);

	while (StrChr (s2, *Last))					/* while separator */
		if (*Last)
			Last++;
		else
			return NULL;								/* no more tokens */

	s = Last;		/* start of token */

	while (*Last && !StrChr (s2, *Last))		/* while not separator */
		Last++;

	if (*Last)		/* null terminate token if not already */
		*Last++ = '\0';
			
	return s;
}


/*
 *		stricmp for far strings
 */
int  DLLENTRY 	StriCmp (s1, s2)
LPCTSTR	s1;
LPCTSTR	s2;
{
	while (*s1 && *s2 && toupper(*s1) == toupper(*s2))
	{
		s1++; s2++;
	}

	return toupper(*s1) - toupper(*s2);
}


/*
 *		strcpy for far strings
 */
LPTSTR	DLLENTRY StrCpy (s1, s2)
LPTSTR	s1;
LPCTSTR	s2;
{
	LPTSTR	s = s1;

	while (*s1++ = *s2++);

	return s;
}

/*
 *		strncpy for far strings
 */
LPTSTR	DLLENTRY StrnCpy (s1, s2, n)
LPTSTR	s1;
LPCTSTR	s2;
int	n;
{
	LPTSTR	s = s1;

	while (n-- && (*s1++ = *s2++))
		;

	return s;
}

/*
 *		strncpy for far strings
 */
LPTSTR	DLLENTRY StrnCat (s1, s2, n)
LPTSTR  s1;
LPCTSTR s2;
int	n;
{
	LPTSTR	s = s1;

	while (n && *s1)
		s1++;
	while (n-- && (*s1++ = *s2++))
		;

	return s;
}



// #endif  /* #ifdef NOASSEMBLER   */

/*
 *		return pointer to last char of far string
 */
LPTSTR	DLLENTRY StrLast (s)
LPTSTR	s;
{
   if (!*s) return s;

	while (*s) s++;

	return s-1;
}

/*
 *		strncpy for far strings with null termination
 */
LPTSTR	DLLENTRY StrnnCat (s1, s2, n)
LPTSTR	s1;
LPCTSTR	s2;
int	n;
{
	LPTSTR	s = s1;

	if (!n--)
		return s;
	while (n && *s1)
		n--,s1++;
	while (n-- && (*s1++ = *s2++));
	*s1 = '\0';

	return s;
}



/*
 *		same as strncpy with null termination
 */
LPTSTR	DLLENTRY StrnnCpy (s1, s2, n)
LPTSTR  s1;
LPCTSTR s2;
int	n;		/* maximum size of s1 including null termination */
{
	LPTSTR	s = s1;

	if (!n--)
		return s;
	while (n-- && (*s1++ = *s2++));
	*s1 = '\0';

	return s;
}

/*
 *		fstrstr with ignore case
 */
LPTSTR	DLLENTRY StriStr (Str, Find)
LPCTSTR Str;
LPCTSTR Find;
{
	int	Len;

	Len = lstrlen(Find);
	while (*Str)
//GAB 07/12/04 	14495	if (!StrniCmp (Str, Find, Len))
	  if (!_tcsnicmp (Str, Find, Len))
		return (LPTSTR)Str;
      else
         Str = _tcsinc(Str);
         /* RFW - 2/9/05 - 15719
         Str++;
         */

	return NULL;
}

	
/*
 *		strupr for far strings
 */
LPTSTR	DLLENTRY StrUpr(Str)
LPTSTR	Str;
{
	LPTSTR	s = Str;

	for(;*Str ;Str++)
        *Str = (TCHAR)toupper(*Str);

	return s;
}

/*
 *		strlwr for far strings
 */
LPTSTR	DLLENTRY StrLwr(Str)
LPTSTR	Str;
{
	LPTSTR	s = Str;

	for(;*Str ;Str++)
        *Str = (TCHAR)tolower(*Str);

	return s;
}


LPTSTR DLLENTRY StrInsertChar (TCHAR cChar, LPTSTR lpszString, int iInsertPos)
{
   int   i;
   LPTSTR lptmp = lpszString;
        
   if (lptmp)
   {
      // The new character is inserted into the string at index iInsertPos.

      // Loop backwards from the end of the string to the insert position
      // moving each character (including the terminator) RIGHT one place.  

      for (i = lstrlen(lpszString); i >= iInsertPos; i--)
         lpszString[i+1] = lpszString[i];

      // Finally insert the new character

      lpszString[++i] = cChar;
   }
   return(lptmp);
}

//--------------------------------------------------------------------------//

LPTSTR DLLENTRY StrDeleteChar(LPTSTR lpszString, register int i)
{
   LPTSTR lptmp = lpszString;

   if (lptmp && i < StrLen( lptmp))
   {
      // Loop from the character following the deleted character to the end of
      // the string moving each character (including the terminator) LEFT one 
      // place.
      do
      {
         lpszString[i] = lpszString[i+1];
      } while (lpszString[++i]);
   }
   return(lptmp);
}

//--------------------------------------------------------------------------//

LPTSTR DLLENTRY StrRemove(LPTSTR lpszString, register int i, register int j)
{
   int k;
   LPTSTR lptmp = lpszString;

   if (lptmp)
   {
      // If the end comes before the start, swap them.

      if (i > j)
      {
         k = i;
         i = j;
         j = k;
      }

      // Loop from the character following the substring to the end of the 
      // string moving each character (including the terminator) LEFT by the 
      // number of characters in the substring.

      do
      {
         lpszString[i++] = lpszString[++j];
      } while (lpszString[j]);
   }
   return(lptmp);
}
