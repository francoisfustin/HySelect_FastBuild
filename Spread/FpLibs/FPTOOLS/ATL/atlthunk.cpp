// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#include <windows.h>
#include <atlstdthunk.h>
 
#define ULongToPtr( ul ) ((VOID *)(ULONG*)((unsigned long)ul))
#define UlongToPtr(ul) ULongToPtr(ul)

//FORCEINLINE
PVOID
//#if !defined(_M_CEE_PURE)
__cdecl
//#endif
__InlineInterlockedCompareExchangePointer (
    PVOID volatile *Destination,
    PVOID ExChange,
    PVOID Comperand
    )
{
    return((PVOID)(LONG*)InterlockedCompareExchange((void**)Destination, (void*)(LONG*)ExChange, (void*)(LONG*)Comperand));
}

#define InterlockedCompareExchangePointer __InlineInterlockedCompareExchangePointer

#ifndef _SLIST_HEADER_
#define _SLIST_HEADER_

#if defined(_WIN64)

//
// The type SINGLE_LIST_ENTRY is not suitable for use with SLISTs.  For
// WIN64, an entry on an SLIST is required to be 16-byte aligned, while a
// SINGLE_LIST_ENTRY structure has only 8 byte alignment.
//
// Therefore, all SLIST code should use the SLIST_ENTRY type instead of the
// SINGLE_LIST_ENTRY type.
//

#pragma warning(push)
#pragma warning(disable:4324)   // structure padded due to align()
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY *PSLIST_ENTRY;
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY {
    PSLIST_ENTRY Next;
} SLIST_ENTRY;
#pragma warning(pop)

#else

#define SLIST_ENTRY SINGLE_LIST_ENTRY
#define _SLIST_ENTRY _SINGLE_LIST_ENTRY
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY

#endif // _WIN64

#if defined(_WIN64)

typedef union DECLSPEC_ALIGN(16) _SLIST_HEADER {
    struct {  // original struct
        ULONGLONG Alignment;
        ULONGLONG Region;
    };
    struct {  // 8-byte header
        ULONGLONG Depth:16;
        ULONGLONG Sequence:9;
        ULONGLONG NextEntry:39;
        ULONGLONG HeaderType:1; // 0: 8-byte; 1: 16-byte
        ULONGLONG Init:1;       // 0: uninitialized; 1: initialized
        ULONGLONG Reserved:59;
        ULONGLONG Region:3;
    } Header8;
    struct {  // 16-byte header
        ULONGLONG Depth:16;
        ULONGLONG Sequence:48;
        ULONGLONG HeaderType:1; // 0: 8-byte; 1: 16-byte
        ULONGLONG Init:1;       // 0: uninitialized; 1: initialized
        ULONGLONG Reserved:2;
        ULONGLONG NextEntry:60; // last 4 bits are always 0's
    } Header16;
} SLIST_HEADER, *PSLIST_HEADER;

#else

typedef union _SLIST_HEADER {
    ULONGLONG Alignment;
    struct {
        SLIST_ENTRY Next;
        WORD   Depth;
        WORD   Sequence;
    };
} SLIST_HEADER, *PSLIST_HEADER;

#endif // _WIN64

#endif // _SLIST_HEADER_


NTSYSAPI
VOID
NTAPI
RtlInitializeSListHead (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlFirstEntrySList (
    IN const SLIST_HEADER *ListHead
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
WORD  
NTAPI
RtlQueryDepthSList (
    IN PSLIST_HEADER ListHead
    );


extern "C"
{
typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

struct _PEB;
typedef struct _PEB * PPEB;

typedef struct _TEB {
    NT_TIB NtTib;
    PVOID EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    PPEB ProcessEnvironmentBlock;
    /* .... Don't need any thing below this*/
} TEB, *PTEB;
 
_inline struct _TEB * Atl_NtCurrentTeb( void ) { __asm mov eax, fs:[0x18] }
 
}

#if !defined(_X86_)
#error Unsupported platform
#endif

#if !defined(PAGE_SIZE)
#define PAGE_SIZE 4096
#endif

#if !defined(DECLSPEC_NOINLINE)
#define DECLSPEC_NOINLINE __declspec(noinline)
#endif

#define ATL_THUNKS_PER_PAGE (PAGE_SIZE / sizeof(ATL_THUNK_ENTRY))

//
// Local function prototypes and typedefs
//

BOOL
static
__InitializeThunkPool (
    VOID
    );

typedef
PSINGLE_LIST_ENTRY
(__stdcall *PINTERLOCKED_PUSH_ENTRY_SLIST) (
     PSLIST_HEADER ListHead,
     PSINGLE_LIST_ENTRY ListEntry
    );

typedef
PSINGLE_LIST_ENTRY
(__stdcall *PINTERLOCKED_POP_ENTRY_SLIST) (
     PSLIST_HEADER ListHead
    );

//
// An ATL thunk structure, used to manage free thunks in the pool
//

typedef union _ATL_THUNK_ENTRY {
    SLIST_ENTRY SListEntry;
	struct ATL::_stdcallthunk Thunk;
} ATL_THUNK_ENTRY, *PATL_THUNK_ENTRY;

//
// Pointer to the process-wide ATL thunk slist.
//

PSLIST_HEADER __AtlThunkPool = NULL;

//
// Special value for __AtlThunkPool indicating that the standard
// heap should be used for thunk allocation.
//

#define ATLTHUNK_USE_HEAP_VALUE (PSLIST_HEADER)UlongToPtr(1)
#define ATLTHUNK_USE_HEAP()     (__AtlThunkPool == ATLTHUNK_USE_HEAP_VALUE)

PINTERLOCKED_PUSH_ENTRY_SLIST __AtlInterlockedPushEntrySList = NULL;
PINTERLOCKED_POP_ENTRY_SLIST  __AtlInterlockedPopEntrySList = NULL;


PVOID
__AllocStdCallThunk_cmn (
    VOID
    )

/*++

Routine Description:

    This function is called by ATL to allocate a thunk structure from
    executable memory.

Arguments:

    None.

Return Value:

    Returns a pointer to a thunk structure on success.  Raises an exception
    on failure.

--*/

{
    PATL_THUNK_ENTRY lastThunkEntry;
    PATL_THUNK_ENTRY thunkEntry;
    PVOID thunkPage;

    //
    // Perform initialization if this is the first time through.
    //

    if (__AtlThunkPool == NULL) {
        if (__InitializeThunkPool() == FALSE) {
            goto outOfMemory;
        }
    }

    if (ATLTHUNK_USE_HEAP()) {

        //
        // On a non-NX capable platform, use the standard heap.
        //

        thunkEntry = (PATL_THUNK_ENTRY)HeapAlloc(GetProcessHeap(),
                                                 0,
												 sizeof(ATL::_stdcallthunk));
        if (thunkEntry == NULL) {
            goto outOfMemory;
        }

        return thunkEntry;
    }

    //
    // Attempt to pop a thunk structure from the list and return it
    // 

    thunkEntry = (PATL_THUNK_ENTRY)__AtlInterlockedPopEntrySList(__AtlThunkPool);
    if (thunkEntry != NULL) {
        return &thunkEntry->Thunk;
    }

    //
    // The thunk list was empty.  Allocate a new page of executable
    // memory.
    //

    thunkPage = (PATL_THUNK_ENTRY)VirtualAlloc(NULL,
                                               PAGE_SIZE,
                                               MEM_COMMIT,
                                               PAGE_EXECUTE_READWRITE);
    if (thunkPage == NULL) {
        goto outOfMemory;
    }

    //
    // See if another thread has replenished the pool while we were off
    // allocating memory.  This does not close the window but makes it much
    // smaller.
    //
    // The volatile reference moves the overhead of making the page present
    // outside of the window.
    //

    *(DWORD volatile *)thunkPage;
    thunkEntry = (PATL_THUNK_ENTRY)__AtlInterlockedPopEntrySList(__AtlThunkPool);
    if (thunkEntry != NULL) {

        //
        // The pool has been replenished.  Free the page and use the thunk
        // entry that we just received.
        //

        VirtualFree(thunkPage,0,MEM_RELEASE);
        return thunkEntry;
    }

    //
    // Create an array of thunk structures on the page and insert all but
    // the last into the free thunk list.
    //
    // The last is kept out of the list and represents the thunk allocation.
    //

    thunkEntry = (PATL_THUNK_ENTRY)thunkPage;
    lastThunkEntry = thunkEntry + ATL_THUNKS_PER_PAGE - 1;
    do {
        __AtlInterlockedPushEntrySList(__AtlThunkPool,&thunkEntry->SListEntry);
        thunkEntry += 1;
    } while (thunkEntry < lastThunkEntry);

    return thunkEntry;

outOfMemory:

    return NULL;
}


VOID
__FreeStdCallThunk_cmn (
    IN PVOID Thunk
    )

/*++

Routine Description:

    This function is called by ATL to release a thunk structure back to the
    process-wide free thunk pool.

Arguments:

    Thunk - supplies a pointer to a thunk structure that was allocated with
            __AllocStdCallThunk().

Return Value:

    None.

--*/

{
    PATL_THUNK_ENTRY thunkEntry;

    if (ATLTHUNK_USE_HEAP()) {

        //
        // On a non-NX capable platform, use the standard heap.
        //                              

        HeapFree(GetProcessHeap(),0,Thunk);

    } else {

        //
        // Simply push the free thunk structure back onto the pool
        //
    
        thunkEntry = (PATL_THUNK_ENTRY)Thunk;
        __AtlInterlockedPushEntrySList(__AtlThunkPool,&thunkEntry->SListEntry);
    }
}


BOOL
static
//DECLSPEC_NOINLINE
__InitializeThunkPool (
    VOID
    )

/*++

Routine Description:

    This function is called on the first invocation of __AllocStdCallThunk().
    It retrieves a pointer to the process-wide thunk pool SLIST_HEADER, if
    one already exists, otherwise this routine supplies an initialized
    SLIST_HEADER.

Arguments:

    None.

Return Value:

    Returns TRUE if initialization succeeded, FALSE otherwise.

--*/

{
    #define PEB_POINTER_OFFSET 0x34

    PSLIST_HEADER *atlThunkPoolPtr;
    PSLIST_HEADER atlThunkPool;

    //
    // On Win64, a per-process ATL thunk "heap" (anchored in the PEB) is always
    // mantained as an SLIST.
    //
    // On X86, such a heap is conditional.  If the OS is < 5.1 (Windows XP) then
    // thunks are allocated/freed from/to the heap, otherwise they are mantained
    // as they would be on Win64.
    //
    // Two reasons for this:
    //
    // - We can't guarantee that the SLIST slot in the PEB is available downlevel
    // - Downlevel OSs may not offer the SLIST functionality
    // 

    HMODULE kernel32Module;
    BOOL result;

    result = IsProcessorFeaturePresent( 12 /*PF_NX_ENABLED*/ );
    if (result == FALSE) {

        //
        // NX execution is not happening on this machine.
        //
        // Indicate that the regular heap should be used by setting
        // __AtlThunkPool to a special value.
        //

        __AtlThunkPool = ATLTHUNK_USE_HEAP_VALUE;
        return TRUE;
    }

    //
    // We are running on Windows NT5.1 or later.  Get the kernel32 pointers to
    // InterlockedPushEntrySList and InterlockedPopEntrySList.  They can't be
    // simply imported as this library may run in environments without those
    // routines.
    // 

    kernel32Module = LoadLibrary( "kernel32.dll" );
    if (kernel32Module != NULL) {

        __AtlInterlockedPushEntrySList = (PINTERLOCKED_PUSH_ENTRY_SLIST)
            GetProcAddress( kernel32Module, "InterlockedPushEntrySList" );

        __AtlInterlockedPopEntrySList = (PINTERLOCKED_POP_ENTRY_SLIST)
            GetProcAddress( kernel32Module, "InterlockedPopEntrySList" );
    }

    if (__AtlInterlockedPushEntrySList == NULL ||
        __AtlInterlockedPopEntrySList == NULL) {

        //
        // If either address could not be retrieved then fail the
        // initialization.
        //

        return FALSE;
    }

    atlThunkPoolPtr =
        (PSLIST_HEADER *)((PCHAR)(Atl_NtCurrentTeb()->ProcessEnvironmentBlock) + PEB_POINTER_OFFSET);

    atlThunkPool = *atlThunkPoolPtr;
    if (atlThunkPool == NULL) {

        //
        // The pool list has not yet been initialized.  Try to use ours.
        //
        // Normally we would simply call InitializeSListHead() to initialize
        // the SLIST_HEADER.  However, this creates linkage that conflicts with
        // modules (such as duser) which also link to ntslist.lib.
        //
        // So to avoid that, the SLIST_HEADER is initialized manually.  This
        // code is platform-specific.
        // 
    
        atlThunkPool = (PSLIST_HEADER)HeapAlloc( GetProcessHeap(),
                                                 0,
                                                 sizeof(SLIST_HEADER) );
        if (atlThunkPool == NULL) {
            return FALSE;
        }

        //InitializeSListHead(atlThunkPool);
        atlThunkPool->Alignment = 0;

        if (InterlockedCompareExchangePointer( (PVOID *)atlThunkPoolPtr,
                                               atlThunkPool,
                                               NULL ) != NULL) {

            //
            // Another thread was initializing as well, and won the race.
            // Free our slist header and use the one that is now there.
            //

            HeapFree( GetProcessHeap(),
                      0,
                      atlThunkPool );
        }

        atlThunkPool = *atlThunkPoolPtr;
    }

    __AtlThunkPool = atlThunkPool;
    return TRUE;
}

//
// Now create the actual routines, one pair within an ATL namespace and one
// without.
// 

PVOID
__stdcall __AllocStdCallThunk (
    VOID
    )
{
    return __AllocStdCallThunk_cmn();
}

VOID
__stdcall __FreeStdCallThunk (
    IN PVOID Thunk
    )
{
    __FreeStdCallThunk_cmn(Thunk);
}

namespace ATL {

PVOID
__stdcall __AllocStdCallThunk (
    VOID
    )
{
    return __AllocStdCallThunk_cmn();
}

VOID
__stdcall __FreeStdCallThunk (
    IN PVOID Thunk
    )
{
    __FreeStdCallThunk_cmn(Thunk);
}

}   // namespace ATL


