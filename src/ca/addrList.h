

#include "envDefs.h"  

#ifdef __cplusplus
extern "C" {
#endif

epicsShareFunc void epicsShareAPI configureChannelAccessAddressList 
        (ELLLIST *pList, SOCKET sock, unsigned short port);

epicsShareFunc void epicsShareAPI addAddrToChannelAccessAddressList 
    (ELLLIST *pList, const ENV_PARAM *pEnv, unsigned short port);

epicsShareFunc void epicsShareAPI printChannelAccessAddressList (const ELLLIST *pList);

epicsShareFunc void epicsShareAPI setPortAndRemoveDuplicates 
    (ELLLIST *pDestList, ELLLIST *pSrcList, unsigned short port);

#ifdef __cplusplus
}
#endif

