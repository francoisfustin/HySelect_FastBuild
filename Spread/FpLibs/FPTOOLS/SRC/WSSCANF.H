/************************************************************************
 *
 *    Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *
 *-----------------------------------------------------------------------
 *
 *      Module:  wsscanf.h
 *
 *      Author:  baw
 *
 *     Remarks:  header file for wsscanf.c
 *
 *   Revisions:  
 *     01.00.000  2/23/91 baw   Initial version, initial build
 *
 ************************************************************************/

// constant definitions

#define ALLOW_RANGE                        // allow "%[a-z]" - style 
                                           // scansets

#define LEFT_BRACKET  ('[' | ('a' - 'A'))  // 'lowercase' version

#define ASCII         32                   // # of bytes needed to 
                                           // hold 256 bits

// macro definitions

#define INC()            (++nCharCount, Inc( (LPSTR FAR *) &lpPtrBuffer ))
#define UN_INC( chr )    (--nCharCount, UnInc( chr, (LPSTR FAR *) &lpPtrBuffer ))
#define EAT_WHITE()      WhiteOut( (int FAR *) &nCharCount, (LPSTR FAR *) &lpPtrBuffer )
#define HEXTODEC( chr )  HexToDec( chr )

#define MUL10(x)         ( (((x)<<2) + (x))<<1 )

// function definitions

int FAR _cdecl _export wsscanf( LPSTR, LPSTR, ... ) ;
char NEAR HexToDec( char ) ;
char NEAR Inc( LPSTR FAR * ) ;
VOID NEAR UnInc( char, LPSTR FAR * ) ;
char NEAR WhiteOut( int FAR *, LPSTR FAR * ) ;

/************************************************************************
 * End of File: wsscanf.h
 ************************************************************************/
