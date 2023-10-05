/*    PortTool v2.2     FPFILE.C          */

#include "fptools.h"

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <errno.h>    
#include <string.h>

#if (!defined(_MSC_VER) | (_MSC_VER <= 600))
#define _open open
#define _close close
#define _lseek lseek
#define _access access
#endif

static char szFileName[256];

/*
 *    open file - TRUE for success
 */
BOOL  DLLENTRY FileOpen (Name, OFlag, PMode, File)
LPSTR Name;
int   OFlag;
int   PMode;
LPINT File;
{
   _fstrcpy (szFileName, Name);

    *File = _open(szFileName, OFlag, PMode);

   return *File != -1;
}

/*
 *    close file - TRUE for success
 */
BOOL  DLLENTRY FileClose (File)
int   File;
{
    return !_close (File);
}

/*
 *    seek in file - TRUE for success
 */
BOOL  DLLENTRY FileSeek (File, Pos, Mode)
int   File;
long  Pos;
WORD  Mode;
{
    return _lseek (File, Pos, Mode) != -1;
}

/*
 *    read data from file
 */
#if defined(_WIN64) || defined(_IA64)
WORD DLLENTRY fpFileRead (LPVOID Buff, WORD Size, WORD Num, int File)
#else
WORD DLLENTRY FileRead (Buff,Size,Num,File)
LPVOID   Buff; /* buffer to read */
WORD     Size;    /* size of elements */
WORD     Num;     /* number of elements */
int      File;    /* input file handle */
#endif
{
   WORD  Count = 0;

   errno = 0;
   if (Num && Size)
   {
#if defined(_WIN64) || defined(_IA64)
      errno = fpSysRead(File, Buff, Size * Num, &Count);
#else
      errno = SysRead(File, Buff, Size * Num, &Count);
#endif
      if (errno == 5)         // Acces denied
         errno = EACCES;

      if (errno == 6)         // invalid file handle
         errno = EBADF;
      return(Count / Size);
   }
   return(0);
}

#if defined(_WIN64) || defined(_IA64)
/*
 *    get character from file
 */
BOOL  DLLENTRY FileGetc (Value,File)
LPSTR Value;   /* location to read to */
int   File;    /* input file handle */
{
   return fpFileRead (Value,sizeof(char),1,File);
}
/*
 *    get word from file
 */
BOOL  DLLENTRY FileGetw (Value, File)
LPINT Value;   /* location to read to */
int   File;    /* input file handle */
{
   return fpFileRead (Value,sizeof(int),1,File);
}
#else
/*
 *    get character from file
 */
BOOL  DLLENTRY FileGetc (Value,File)
LPSTR Value;   /* location to read to */
int   File;    /* input file handle */
{
   return FileRead (Value,sizeof(char),1,File);
}
/*
 *    get word from file
 */
BOOL  DLLENTRY FileGetw (Value, File)
LPINT Value;   /* location to read to */
int   File;    /* input file handle */
{
   return FileRead (Value,sizeof(int),1,File);
}
#endif

/*
 *    returns errno - last error as in errno.h
 */
int   DLLENTRY ErrorNo()
{
   return errno;
}

/*
 *    Returns TRUE if the named file exists, FALSE otherwise.
 */
BOOL  DLLENTRY FileExists (Name)
LPSTR Name;
{              
   _fstrcpy (szFileName, Name);

   return !_access (szFileName, 0);
}

