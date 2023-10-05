/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Thu Oct 16 15:55:34 2008
 */
/* Compiler settings for SS30PP.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __SS30PP_h__
#define __SS30PP_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __PPGeneral_FWD_DEFINED__
#define __PPGeneral_FWD_DEFINED__

#ifdef __cplusplus
typedef class PPGeneral PPGeneral;
#else
typedef struct PPGeneral PPGeneral;
#endif /* __cplusplus */

#endif 	/* __PPGeneral_FWD_DEFINED__ */


#ifndef __PPPage_FWD_DEFINED__
#define __PPPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class PPPage PPPage;
#else
typedef struct PPPage PPPage;
#endif /* __cplusplus */

#endif 	/* __PPPage_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __SS30PPLib_LIBRARY_DEFINED__
#define __SS30PPLib_LIBRARY_DEFINED__

/* library SS30PPLib */
/* [helpstring][helpfile][version][uuid] */ 


EXTERN_C const IID LIBID_SS30PPLib;

EXTERN_C const CLSID CLSID_PPGeneral;

#ifdef __cplusplus

class DECLSPEC_UUID("12F0D61F-E79A-4879-B038-5781A2E4C0B8")
PPGeneral;
#endif

EXTERN_C const CLSID CLSID_PPPage;

#ifdef __cplusplus

class DECLSPEC_UUID("E409C3B7-1BF7-470a-B7FC-B4352A73310F")
PPPage;
#endif
#endif /* __SS30PPLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
