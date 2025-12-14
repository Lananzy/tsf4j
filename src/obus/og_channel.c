#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oi_shm.h"
#include "og_shm.h"
#include "og_bus.h"
#include "og_pkg.h"

/*
int GetZoneConnectChannel(Channel* pstChannel, int iZoneID, int iMultiZone, int iChannelFlag)
{
int iRet, iFlag;
key_t	iKey;
ChannelZoneConnect *pstZoneConnect;

	iKey = CHANNEL_ZONE_CONNECT_SHM_KEY;
	if (iMultiZone) iKey += iZoneID * MAX_SHM_PER_ZONE;
	iFlag = 0666;
	if (iChannelFlag & CHANNEL_CREATE) iFlag |= IPC_CREAT;
	iRet = GetShm3(&pstZoneConnect, iKey, sizeof(ChannelZoneConnect), iFlag);
	if (iRet < 0) return -1;
	pstChannel->iSide = iChannelFlag & CHANNEL_SIDE_UP;
	if (!pstChannel->iSide) pstChannel->iSide |= CHANNEL_SIDE_DOWN;
	pstChannel->pstHead = &pstZoneConnect->stHead;
	pstChannel->sUp = pstZoneConnect->sUp;
	pstChannel->sDown = pstZoneConnect->sDown;
	if (iRet == 0) return 0;

	memset(&pstZoneConnect->stHead, 0, sizeof(pstZoneConnect->stHead));
	pstZoneConnect->stHead.stUp.iSize = CHANNEL_ZONE_CONN_UP_SIZE;
	pstZoneConnect->stHead.stDown.iSize = CHANNEL_ZONE_CONN_DOWN_SIZE;

	return 0;
}

int GetZoneCtrlChannel(Channel* pstChannel, int iZoneID, int iMultiZone, int iChannelFlag)
{
int iRet, iFlag;
key_t	iKey;
ChannelZoneCtrl *pstZoneCtrl;

	iKey = CHANNEL_ZONE_CTRL_SHM_KEY;
	if (iMultiZone) iKey += iZoneID * MAX_SHM_PER_ZONE;
	iFlag = 0666;
	if (iChannelFlag & CHANNEL_CREATE) iFlag |= IPC_CREAT;
	iRet = GetShm3(&pstZoneCtrl, iKey, sizeof(ChannelZoneCtrl), iFlag);
	if (iRet < 0) return -1;
	pstChannel->iSide = iChannelFlag & CHANNEL_SIDE_UP;
	if (!pstChannel->iSide) pstChannel->iSide |= CHANNEL_SIDE_DOWN;
	pstChannel->pstHead = &pstZoneCtrl->stHead;
	pstChannel->sUp = pstZoneCtrl->sUp;
	pstChannel->sDown = pstZoneCtrl->sDown;
	if (iRet == 0) return 0;

	memset(&pstZoneCtrl->stHead, 0, sizeof(pstZoneCtrl->stHead));
	pstZoneCtrl->stHead.stUp.iSize = CHANNEL_ZONE_CTRL_UP_SIZE;
	pstZoneCtrl->stHead.stDown.iSize = CHANNEL_ZONE_CTRL_DOWN_SIZE;

	return 0;
}

int GetZoneDbChannel(Channel* pstChannel, int iZoneID, int iMultiZone, int iChannelFlag)
{
int iRet, iFlag;
key_t	iKey;
ChannelZoneDB *pstZoneDB;

	iKey = CHANNEL_ZONE_DB_SHM_KEY;
	if (iMultiZone) iKey += iZoneID * MAX_SHM_PER_ZONE;
	iFlag = 0666;
	if (iChannelFlag & CHANNEL_CREATE) iFlag |= IPC_CREAT;
	iRet = GetShm3(&pstZoneDB, iKey, sizeof(ChannelZoneDB), iFlag);
	if (iRet < 0) return -1;
	pstChannel->iSide = iChannelFlag & CHANNEL_SIDE_UP;
	if (!pstChannel->iSide) pstChannel->iSide |= CHANNEL_SIDE_DOWN;
	pstChannel->pstHead = &pstZoneDB->stHead;
	pstChannel->sUp = pstZoneDB->sUp;
	pstChannel->sDown = pstZoneDB->sDown;
	if (iRet == 0) return 0;

	memset(&pstZoneDB->stHead, 0, sizeof(pstZoneDB->stHead));
	pstZoneDB->stHead.stUp.iSize = CHANNEL_ZONE_DB_UP_SIZE;
	pstZoneDB->stHead.stDown.iSize = CHANNEL_ZONE_DB_DOWN_SIZE;
	pstZoneDB->stHead.stUp.cAuth = 1;

	return 0;
}
*/

int ChannelGet(Channel* pstChannel, BusPkg *pstPkg)
{
ChannelVar *pstVar;
char *sData;
int iPkgLen, iDataLen, iTailLen;
unsigned short shPkgHeadLen, shPkgBodyLen;

	if (pstChannel->iSide & CHANNEL_SIDE_UP)  {
		pstVar = &pstChannel->pstHead->stUp;
		sData = pstChannel->sUp;
	} else {
		pstVar = &pstChannel->pstHead->stDown;
		sData = pstChannel->sDown;
	}
	if (pstVar->cAuth) {
		// pstVar->iHead = pstVar->iGet;
		pstVar->cAuth = 0;
	}
	// printf("channel head %d tail %d\n", pstVar->iHead, pstVar->iTail);
	if (pstVar->iHead == pstVar->iTail) return ERR_CHANNEL_EMPTY;
	iDataLen = pstVar->iTail - pstVar->iHead;
	if (iDataLen < 0) iDataLen += pstVar->iSize;
	if (iDataLen < sizeof(BusPkgHead)) {
		pstVar->iHead = pstVar->iTail;
		return ERR_CHANNEL_CONFUSE;
	}
	iTailLen = pstVar->iSize - pstVar->iHead;
	if (iTailLen < sizeof(BusPkgHead)) {
		memcpy(pstPkg, sData + pstVar->iHead, iTailLen);
		memcpy((char *)pstPkg + iTailLen, sData, sizeof(BusPkgHead) - iTailLen);
	} else {
		memcpy(pstPkg, sData + pstVar->iHead, sizeof(BusPkgHead));
	}
	shPkgHeadLen = BusHeadLen(&pstPkg->stHead);
	shPkgBodyLen = BusBodyLen(&pstPkg->stHead);
	iPkgLen = shPkgHeadLen + shPkgBodyLen;
	if (shPkgHeadLen < sizeof(BusPkgHead) || BusHeadCheckSum(&pstPkg->stHead) || iDataLen < iPkgLen) {
		// printf("check: %d  head len: %d\n", BusHeadCheckSum(&pstPkg->stHead), shPkgHeadLen);
		// HexShow(&pstPkg->stHead, shPkgHeadLen, 0);
		pstVar->iHead = pstVar->iTail;
		return ERR_CHANNEL_CONFUSE;
	}
	if (iTailLen < iPkgLen) {
		memcpy(pstPkg, sData + pstVar->iHead, iTailLen);
		memcpy((char *)pstPkg + iTailLen, sData, iPkgLen - iTailLen);
	} else  {
		memcpy(pstPkg, sData + pstVar->iHead, iPkgLen);
	}
	pstVar->iHead = (pstVar->iHead + iPkgLen) % pstVar->iSize;
	return 0;
}

int ChannelPush(Channel* pstChannel, BusPkg *pstPkg)
{
ChannelVar *pstVar;
char *sData;
int iPkgLen, iRoom, iTailRoom;

	if (pstChannel->iSide & CHANNEL_SIDE_UP)  {
		pstVar = &pstChannel->pstHead->stDown;
		sData = pstChannel->sDown;
	} else {
		pstVar = &pstChannel->pstHead->stUp;
		sData = pstChannel->sUp;
	}
	iRoom = pstVar->iHead - pstVar->iTail - 1;
	if (iRoom < 0) iRoom += pstVar->iSize;
	iPkgLen = BusPkgLen(&pstPkg->stHead);
	if (iPkgLen < sizeof(BusPkgHead)) return ERR_CHANNEL_PKG;
	if (iPkgLen > sizeof(BusPkg)) return ERR_CHANNEL_PKG;
	if (iRoom < iPkgLen) {
		// printf("channel head: %d tail: %d\n", pstVar->iHead, pstVar->iTail);
		return ERR_CHANNEL_FULL;
	}
	*(int *)pstPkg->stHead.sSeq = htonl(pstVar->iSeq);
	*(short *)pstPkg->stHead.sCheckSum = 0;
	*(short *)pstPkg->stHead.sCheckSum = BusHeadCheckSum(&pstPkg->stHead);
	if (pstVar->iTail + iPkgLen > pstVar->iSize) {
		iTailRoom = pstVar->iSize - pstVar->iTail;
		memcpy(sData + pstVar->iTail, (char *)pstPkg, iTailRoom);
		memcpy(sData, (char *)pstPkg + iTailRoom, iPkgLen - iTailRoom);
		pstVar->iTail = iPkgLen - iTailRoom;
	} else {
		memcpy(sData + pstVar->iTail, (char *)pstPkg, iPkgLen);
		pstVar->iTail = (pstVar->iTail + iPkgLen) % pstVar->iSize;
	}
	// printf("channel head: %d tail: %d\n", pstVar->iHead, pstVar->iTail);
	pstVar->iSeq++;
	return 0;
}

int ChannelAuthPos(Channel* pstChannel, int* piPos)
{
	ChannelVar *pstVar;
	char *sData;
	int iPkgLen, iDataLen, iTailLen;
	unsigned short shPkgHeadLen, shPkgBodyLen;
	
	if (pstChannel->iSide & CHANNEL_SIDE_UP)  {
		pstVar = &pstChannel->pstHead->stUp;
		sData = pstChannel->sUp;
	} else {
		pstVar = &pstChannel->pstHead->stDown;
		sData = pstChannel->sDown;
	}
	if (!pstVar->cAuth) {
		pstVar->iGet = pstVar->iHead;
		pstVar->cAuth = 1;
	}
	
	if (pstVar->iGet == pstVar->iTail) return ERR_CHANNEL_EMPTY;
	iDataLen = pstVar->iTail - pstVar->iGet;
	if (iDataLen < 0) iDataLen += pstVar->iSize;
	
	if (iDataLen < sizeof(BusPkgHead))
	{
		pstVar->iGet = pstVar->iHead = pstVar->iTail;
		return ERR_CHANNEL_CONFUSE;
	}

	*piPos	=	pstVar->iGet;

	return 0;
}

int ChannelAuthPeekByPos(Channel* pstChannel, int iPos, BusPkg *pstPkg)
{
	ChannelVar *pstVar;
	char *sData;
	int iPkgLen, iDataLen, iTailLen;
	unsigned short shPkgHeadLen, shPkgBodyLen;
	
	if (pstChannel->iSide & CHANNEL_SIDE_UP)  {
		pstVar = &pstChannel->pstHead->stUp;
		sData = pstChannel->sUp;
	} else {
		pstVar = &pstChannel->pstHead->stDown;
		sData = pstChannel->sDown;
	}

	if (iPos == pstVar->iTail) return ERR_CHANNEL_EMPTY;
	iDataLen = pstVar->iTail - iPos;
	if (iDataLen < 0) iDataLen += pstVar->iSize;
	if (iDataLen < sizeof(BusPkgHead))
	{
		return ERR_CHANNEL_CONFUSE;
	}

	iTailLen = pstVar->iSize - iPos;
	if (iTailLen < sizeof(BusPkgHead))
	{
		memcpy(pstPkg, sData + iPos, iTailLen);
		memcpy((char *)pstPkg + iTailLen, sData, sizeof(BusPkgHead) - iTailLen);
	}
	else
	{
		memcpy(pstPkg, sData + iPos, sizeof(BusPkgHead));
	}

	shPkgHeadLen = BusHeadLen(&pstPkg->stHead);
	shPkgBodyLen = BusBodyLen(&pstPkg->stHead);
	iPkgLen = shPkgHeadLen + shPkgBodyLen;
	if (shPkgHeadLen < sizeof(BusPkgHead) || BusHeadCheckSum(&pstPkg->stHead) || iDataLen < iPkgLen)
	{
		return ERR_CHANNEL_CONFUSE;
	}

	if (iTailLen < iPkgLen)
	{
		memcpy(pstPkg, sData + iPos, iTailLen);
		memcpy((char *)pstPkg + iTailLen, sData, iPkgLen - iTailLen);
	} else  {
		memcpy(pstPkg, sData + iPos, iPkgLen);
	}

	return 0;
}

int ChannelAuthGet(Channel* pstChannel, BusPkg *pstPkg)
{
ChannelVar *pstVar;
char *sData;
int iPkgLen, iDataLen, iTailLen;
unsigned short shPkgHeadLen, shPkgBodyLen;

	if (pstChannel->iSide & CHANNEL_SIDE_UP)  {
		pstVar = &pstChannel->pstHead->stUp;
		sData = pstChannel->sUp;
	} else {
		pstVar = &pstChannel->pstHead->stDown;
		sData = pstChannel->sDown;
	}
	if (!pstVar->cAuth) {
		pstVar->iGet = pstVar->iHead;
		pstVar->cAuth = 1;
	}
	if (pstVar->iGet == pstVar->iTail) return ERR_CHANNEL_EMPTY;
	iDataLen = pstVar->iTail - pstVar->iGet;
	if (iDataLen < 0) iDataLen += pstVar->iSize;
	if (iDataLen < sizeof(BusPkgHead)) {
		pstVar->iGet = pstVar->iHead = pstVar->iTail;
		// printf("1: %d %d\n", iDataLen, sizeof(BusPkgHead));
		return ERR_CHANNEL_CONFUSE;
	}
	iTailLen = pstVar->iSize - pstVar->iGet;
	if (iTailLen < sizeof(BusPkgHead)) {
		memcpy(pstPkg, sData + pstVar->iGet, iTailLen);
		memcpy((char *)pstPkg + iTailLen, sData, sizeof(BusPkgHead) - iTailLen);
	} else {
		memcpy(pstPkg, sData + pstVar->iGet, sizeof(BusPkgHead));
	}
	shPkgHeadLen = BusHeadLen(&pstPkg->stHead);
	shPkgBodyLen = BusBodyLen(&pstPkg->stHead);
	iPkgLen = shPkgHeadLen + shPkgBodyLen;
	if (shPkgHeadLen < sizeof(BusPkgHead) || BusHeadCheckSum(&pstPkg->stHead) || iDataLen < iPkgLen) {
		pstVar->iGet = pstVar->iHead = pstVar->iTail;
		printf("2: %d %d %d %d %d %d\n", shPkgHeadLen, sizeof(BusPkgHead), shPkgBodyLen, BusHeadCheckSum(&pstPkg->stHead), iDataLen, iPkgLen);
		return ERR_CHANNEL_CONFUSE;
	}
	if (iTailLen < iPkgLen) {
		memcpy(pstPkg, sData + pstVar->iGet, iTailLen);
		memcpy((char *)pstPkg + iTailLen, sData, iPkgLen - iTailLen);
	} else  {
		memcpy(pstPkg, sData + pstVar->iGet, iPkgLen);
	}
	pstVar->iGet = (pstVar->iGet + iPkgLen) % pstVar->iSize;
	if (!pstVar->cGStart) pstVar->cGStart = 1;
	//printf("auth get head: %d  get: %d tail: %d\n", pstVar->iHead, pstVar->iGet, pstVar->iTail);

	// tmp code
	// pstVar->iHead = pstVar->iGet;
	return 0;
}

BusPkgHead *GetChannelPkgHead(char *sData, ChannelVar *pstVar, int iPos,  BusPkgHead *pstHead)
{
int iPkgLen, iDataLen, iTailLen;

	iTailLen = pstVar->iSize - iPos;
	if (iTailLen < sizeof(BusPkgHead)) {
		memcpy(pstHead, sData + iPos, iTailLen);
		memcpy((char *)pstHead + iTailLen, sData, sizeof(BusPkgHead) - iTailLen);
	} else {
		memcpy(pstHead, sData + iPos, sizeof(BusPkgHead));
	}
	return pstHead;
}

int GetChannelHeadSeq(Channel* pstChannel)
{
ChannelVar *pstVar;
char *sData;
BusPkgHead stHead;

	if (pstChannel->iSide & CHANNEL_SIDE_UP)  {
		pstVar = &pstChannel->pstHead->stUp;
		sData = pstChannel->sUp;
	} else {
		pstVar = &pstChannel->pstHead->stDown;
		sData = pstChannel->sDown;
	}
	if (pstVar->iHead == pstVar->iTail) return pstVar->iSeq;
	GetChannelPkgHead(sData, pstVar, pstVar->iHead, &stHead);
	return ntohl(*(int *)stHead.sSeq);
}

int ChannelAuth(Channel* pstChannel, int iAuthSeq)
{
ChannelVar *pstVar;
char *sData;
int iPkgLen, iDataLen, iTailLen, iHeadSeq, iGetSeq;
BusPkgHead stHead;

	if (pstChannel->iSide & CHANNEL_SIDE_UP)  {
		pstVar = &pstChannel->pstHead->stUp;
		sData = pstChannel->sUp;
	} else {
		pstVar = &pstChannel->pstHead->stDown;
		sData = pstChannel->sDown;
	}
	if (pstVar->iHead == pstVar->iGet) return ERR_CHANNEL_EMPTY;
	if (pstVar->iTail == pstVar->iGet) {
		iGetSeq = pstVar->iSeq;
	} else {
		GetChannelPkgHead(sData, pstVar, pstVar->iGet, &stHead);
		iGetSeq = ntohl(*(int *)stHead.sSeq);
	}
	while (1) {
		GetChannelPkgHead(sData, pstVar, pstVar->iHead, &stHead);
		iHeadSeq = ntohl(*(int *)stHead.sSeq);
		//printf("a %d h %d g %d\n", iAuthSeq, iHeadSeq, iGetSeq);
		if (iAuthSeq - iHeadSeq < 0 || iAuthSeq - iGetSeq >= 0) return ERR_CHANNEL_AUTH;
		iPkgLen = BusPkgLen(&stHead);
		//printf("head: %d tail: %d get: %d len: %d\n", pstVar->iHead, pstVar->iTail, pstVar->iGet, iPkgLen);
		if (!iPkgLen) {
			HexShow(sData + pstVar->iHead, 250, 0);
			pstVar->iGet = pstVar->iHead = pstVar->iTail;
			printf("channel err!!!\n");
			return ERR_CHANNEL_CONFUSE;
			// exit(0);
		}
		pstVar->iHead = (pstVar->iHead + iPkgLen) % pstVar->iSize;
		if (iHeadSeq == iAuthSeq) return 0;
	}
}

ChannelVar *ChannelVarGet(Channel* pstChannel)
{
	if (pstChannel->iSide & CHANNEL_SIDE_UP)  return  &pstChannel->pstHead->stUp;
	return &pstChannel->pstHead->stDown;
}

ChannelVar *ChannelVarPush(Channel* pstChannel)
{
	if (pstChannel->iSide & CHANNEL_SIDE_UP)  return  &pstChannel->pstHead->stDown;
	return &pstChannel->pstHead->stUp;
}

int ChannelAuthPush(Channel* pstChannel, BusPkg *pstPkg, int iRecvSeq)
{
ChannelVar *pstVar;
int iRet;

	pstVar = ChannelVarPush(pstChannel);
	if (iRecvSeq != pstVar->iRecvSeq + 1) return ERR_CHANNEL_RECV;
	iRet = ChannelPush(pstChannel, pstPkg);
	if (iRet < 0) return iRet;
	pstVar->iRecvSeq++;
	if (!pstVar->cPStart) pstVar->cPStart = 1;
	return 0;
}

/*
int ConstructChannelPkgHead(ChannelPkg *pstPkg, int iHeadExt, int iBodyLen, short shCmd)
{
	memset(&pstPkg->stHead, 0, sizeof(pstPkg->stHead));
	*(short *)pstPkg->stHead.sHeadLen = sizeof(pstPkg->stHead) + iHeadExt;
	*(short *)pstPkg->stHead.sBodyLen = iBodyLen;
	*(short *)pstPkg->stHead.sCmd = shCmd;
	return 0;
}
*/
