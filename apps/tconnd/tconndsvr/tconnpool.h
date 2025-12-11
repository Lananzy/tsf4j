/*
**  @file $RCSfile: tconnpool.h,v $
**  general description of this module
**  $Id: tconnpool.h,v 1.4 2009/01/21 10:59:09 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009/01/21 10:59:09 $
**  @version $Revision: 1.4 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TCONNPOOL_H
#define TCONNPOOL_H

#include "pal/pal.h"
#include "comm/tmempool.h"
#include "apps/tcltapi/tqqdef.h"
#include "apps/tcltapi/tpdudef.h"
#include "apps/tconnapi/tframehead.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagTConnInst		TCONNINST;
typedef struct tagTConnInst		*LPTCONNINST;



struct tagTConnInst
{
	int fValid;
	short fListen;
	short fStream;
	int iIdx;
	int iID;

	//add by hardway
	int iSynSent;
	int iVIPFlag;
	int iRelayFlag;
	int iRelayType;
	int iOldIdx;
	int iOldID;

	int s;
	int iLisLoc;
	int iTransLoc;
	int iUseTimeStamp;

	int iBuff;
	int iOff;
	int iData;
	int fWaitFirstPkg;

	int iMinHeadLen;
	int iHeadLen;
	int iPkgLen;

	/*does not used currently*/
	//int iFailedMsg;
	//int iSendMsg;
	//int iSendByte;

	int iRecvMsg;
	int iRecvByte;

	time_t tLastRecvMsgCheck;
	char szRes1[16-sizeof(time_t)];

	struct sockaddr stAddr;
	char szRes2[32-sizeof(struct sockaddr)];

	time_t tCreate;
	char szRes3[16-sizeof(time_t)];

	time_t tLastRecv;
	char szRes4[16-sizeof(time_t)];

	unsigned int uiQueueToken;
	int iIsInQueue;
	int iQueuePrev;
	int iQueueNext;

	time_t tLastQueueNotify;
	char szRes5[16-sizeof(time_t)];

	int iSendFirstFrame;
	int iSendFirstPkg;

	int iAuthPass;
	int iAuthType;
	TFRAMEAUTHDATA stAuthData;
	char szRes6[128 - sizeof(TFRAMEAUTHDATA)];

	int iNeedFree;
	int iNotify;

	int iEncMethod;
	int iEncKeyLen;
	char szEncKey[128];

	char szIdent[TQQ_IDENT_LEN];

	char szBuff[1];
};



typedef TMEMPOOL				TCONNPOOL;
typedef TMEMPOOL				*LPTCONNPOOL;

#define tconnd_init_pool(ppstPool, iMax, iUnit)	tmempool_new(ppstPool, iMax, iUnit)
#define tconnd_fini_pool(ppstPool)				tmempool_destroy(ppstPool)
#define tconnd_get_inst(pstPool, iIdx)			(TCONNINST*)tmempool_get(pstPool, iIdx)
#define tconnd_alloc_inst(pstPool)				tmempool_alloc(pstPool)
#define tconnd_free_inst(pstPool, iIdx)			tmempool_free(pstPool, iIdx)

#ifdef __cplusplus
}
#endif

#endif /* TCONNPOOL_H */

