// ss40db.h : Spread 4 function exports for DAO binding

#ifndef _SS40DAO_H
#define _SS40DAO_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL DataManager_GetAmbientUserMode(VOID *pDataManager);
IOleClientSite* DataManager_GetClientSite(VOID *pDataManager);
ULONG DataManager_ExternalAddRef(VOID *pDataManager);
ULONG DataManager_ExternalRelease(VOID *pDataManager);
LPOLESTR DataManager_CoTaskCopyString(LPCTSTR psz);

#ifdef __cplusplus
}
#endif

#endif // _SS40DAO_H