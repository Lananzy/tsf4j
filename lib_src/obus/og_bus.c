
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include "oi_log.h"
#include "og_ini.h"
#include "og_shm.h"
#include "og_bus.h"



SectionDesc astBusConfigDesc[] = {
	{"Channel", CFG_SECTION, offsetof(BusConfig, stChnCfg), {
		{"Key", CFG_INT, offsetof(ChannelConfig, iKey), "0"},
		{"Up", CFG_IP, offsetof(ChannelConfig, iUp), "0"},
		{"UpRelay", CFG_IP, offsetof(ChannelConfig, iUpRelay), "0"},
		{"UpRelayType", CFG_STRING, offsetof(ChannelConfig, sUpRelayType), ""},
		{"UpSize", CFG_INT, offsetof(ChannelConfig, iUpSize), "0"},
		{"Down", CFG_IP, offsetof(ChannelConfig, iDown), "0"},
		{"DownRelay", CFG_IP, offsetof(ChannelConfig, iDownRelay), "0"},
		{"DownRelayType", CFG_STRING, offsetof(ChannelConfig, sDownRelayType), ""},
		{"DownSize", CFG_INT, offsetof(ChannelConfig, iDownSize), "0"}
	}},
	{"Log", CFG_SECTION, offsetof(BusConfig, stLog), {
		{"LogPath", CFG_STRING, offsetof(BusLog, sLogPath), "/tmp/buslog"}
	}},
	{""}
};

//static int s_iLog	=	1;
static int s_iLog	=	0;		//glory,2008-03-27,去掉不必要日志

int BusGetLog(void)
{
	return s_iLog;
}

int BusSetLog(int iLog)
{
	int iOldLog;

	iOldLog	=	s_iLog;
	s_iLog	=	iLog;

	return iOldLog;
}

unsigned short BusHeadLen(BusPkgHead* pstHead)
{
	return pstHead->cHeadLen * 4;
}

unsigned int BusBodyLen(BusPkgHead* pstHead)
{
	return ntohl(*(unsigned int *)pstHead->sBodyLen);
}

unsigned int BusPkgLen(BusPkgHead* pstHead)
{
	return BusHeadLen(pstHead) + BusBodyLen(pstHead);
}

unsigned short BusHeadCheckSum(BusPkgHead* pstHead)
{
unsigned short shSum = 0;
int i;

	for (i = 0; i < sizeof(BusPkgHead)/2; i++) shSum ^= *(short *)((char *)pstHead + i * 2);
	return shSum;
}

int DetachOneChannel(Channel *pstChannel)
{
	if( !pstChannel->pstHead )
		return 0;

	shmdt(pstChannel->pstHead);
	pstChannel->pstHead	=	NULL;

	return 0;
}

int AttachOneChannel(Channel *pstChannel, ChannelConfig *pstChnCfg)
{
int iRet, iFlag;
char *sShm;

	iFlag = 0666 | IPC_CREAT;
	// if (iChannelFlag & CHANNEL_CREATE) iFlag |= IPC_CREAT;
	iRet = GetShm3(&sShm, pstChnCfg->iKey, sizeof(ChannelHead) + pstChnCfg->iUpSize + pstChnCfg->iDownSize, iFlag);
	if (iRet < 0) return -1;
	pstChannel->pstHead = (ChannelHead *)sShm;
	pstChannel->sUp = sShm + sizeof(ChannelHead);
	pstChannel->sDown = sShm + sizeof(ChannelHead) + pstChnCfg->iUpSize;
	if (iRet == 0) {
		if (pstChannel->pstHead->iUp != pstChnCfg->iUp || pstChannel->pstHead->iDown != pstChnCfg->iDown) {
			printf("Channel config ERROR!\n");
			printf("Shm: Up: %s   ", inet_ntoa(*(struct in_addr *)&pstChannel->pstHead->iUp));
			printf("Down: %s\n", inet_ntoa(*(struct in_addr *)&pstChannel->pstHead->iDown));
			printf("Cfg: Up: %s   ", inet_ntoa(*(struct in_addr *)&pstChnCfg->iUp));
			printf("Down: %s\n", inet_ntoa(*(struct in_addr *)&pstChnCfg->iDown));
		}
		return 0;
	}

	memset(pstChannel->pstHead, 0, sizeof(ChannelHead));
	pstChannel->pstHead->iUp = pstChnCfg->iUp;
	pstChannel->pstHead->iDown = pstChnCfg->iDown;
	pstChannel->pstHead->iUpRelay = pstChnCfg->iUpRelay;
	pstChannel->pstHead->iDownRelay = pstChnCfg->iDownRelay;
	pstChannel->pstHead->stDown.iSize = pstChnCfg->iDownSize;
	pstChannel->pstHead->stUp.iSize = pstChnCfg->iUpSize;
	pstChannel->pstHead->stDown.iSize = pstChnCfg->iDownSize;

	return 0;
}

int AttachChannelCmp(const void *p1, const void *p2)
{
AttachChannel *pstAch1, * pstAch2;

	pstAch1 = (AttachChannel *)p1;
	pstAch2 = (AttachChannel *)p2;
	return pstAch1->iDst - pstAch2->iDst;
}

int BusConnectInternal(BusInterface *pstBus, char *sFile, int iProcID)
{
FILE *f;
BusConfig stBusCfg;
char sID[MAX_CFG_ID_LEN], sNext[MAX_CFG_ID_LEN];
AttachChannel stAch;

	if (!(f = fopen(sFile, "r"))) {
		printf("Error open bus config: %s\n", sFile);
		return -1;
	}

	if( !pstBus->iProcID )
	{
		pstBus->iProcID = iProcID;
	}
	else if( pstBus->iProcID!=iProcID )
	{
		printf("Error BindedProcID(%d) diff with ToBindProcID(%d)\n", pstBus->iProcID, iProcID);
		return -1;
	}

	InitSection(&stBusCfg, astBusConfigDesc);
	sNext[0] = 0;
	while (!feof(f)) {
		memset(&stBusCfg.stChnCfg, 0, sizeof(stBusCfg.stChnCfg));
		if (ReadSection(f, &stBusCfg, astBusConfigDesc, sID, sNext) != 0) continue;
		if (!cistrcmp(sID, "Channel")) {
			if (stBusCfg.stChnCfg.iUpRelay == 0 && iProcID == stBusCfg.stChnCfg.iUp) {
				stAch.iDst = stBusCfg.stChnCfg.iDown;
				stAch.stChannel.iSide = CHANNEL_SIDE_UP;
			}
			else if (stBusCfg.stChnCfg.iDownRelay == 0 && iProcID == stBusCfg.stChnCfg.iDown) {
				stAch.iDst = stBusCfg.stChnCfg.iUp;
				stAch.stChannel.iSide = CHANNEL_SIDE_DOWN;
			}
			else continue;
			if (pstBus->iAttach >= MAX_ATTACH_CHANNEL) {
				break;
			}
			printf("Attach: %d   %s --> ", stBusCfg.stChnCfg.iKey, inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iDown));
			printf("%s   ", inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iUp));
			if (AttachOneChannel(&stAch.stChannel, &stBusCfg.stChnCfg) == 0)
			{
				if( MyBInsert(&stAch, pstBus->astAttach, &pstBus->iAttach, sizeof(AttachChannel), 1, AttachChannelCmp) )
				{
					printf("Done. %d      %d\n", pstBus->iAttach, pstBus);
				}
				else
				{
					DetachOneChannel(&stAch.stChannel);
					printf("Duplicate, Ignore. %d      %d\n", pstBus->iAttach, pstBus);
				}
			}
			else printf("Fail\n");
		}
	}

	fclose(f);

	return 0;
}

int BusConnect(BusInterface *pstBus, char *sFile, int iProcID)
{
	char sLogFile[80];
	int iRet;

	memset(pstBus, 0, sizeof(BusInterface));

	iRet	=	BusConnectInternal(pstBus, sFile, iProcID);

	if( iRet<0 )
		return -1;

	if( pstBus->iAttach<=0 )
	{
		printf("Error: No channel attached\n");
		return -1;
	}

	sprintf(sLogFile, "%s/%s", "/tmp/buslog", inet_ntoa(*(struct in_addr *)&iProcID));
	printf("log %s\n", sLogFile);
	InitLogFile(&pstBus->stLog, sLogFile, 0, 5, 1000000);

	return 0;
}

int BusConnectDir(BusInterface *pstBus, char *sDir, int iProcID, char* sSuffix)
{
	int iLen;
	int iDir;
	int iFile;
	int iSuffix;
	DIR* pstDir;
	struct dirent* pstEntry;
	char sFile[80];
	char sLogFile[80];

	memset(pstBus, 0, sizeof(BusInterface));

	if( sDir )
		iLen	=	strlen(sDir);
	else
		iLen	=	0;

	if( iLen<=0 )
	{
		printf("Error Dir specified: %s\n", sDir);
		return -1;
	}

	if( sSuffix )
		iSuffix	=	strlen(sSuffix);
	else
		iSuffix	=	0;
	
	pstDir	=	opendir(sDir);

	if( !pstDir )
	{
		printf("Error open bus config dir: %s\n", sDir);
		return -1;
	}

	while(pstEntry=readdir(pstDir))
	{
		iFile	=	strlen(pstEntry->d_name);

		if( '.'==pstEntry->d_name[0] )
		{
			if( !pstEntry->d_name[1] || ('.'==pstEntry->d_name[1] && !pstEntry->d_name[2]) )
			continue;
		}

		if( iSuffix && (iFile<iSuffix || strcasecmp(pstEntry->d_name+iFile-iSuffix, sSuffix)) )
			continue;

		if( sDir[iLen-1]=='/' )
			sprintf(sFile, "%s%s", sDir, pstEntry->d_name);
		else
			sprintf(sFile, "%s/%s", sDir, pstEntry->d_name);

		BusConnectInternal(pstBus, sFile, iProcID);
	}

	if( pstBus->iAttach<=0 )
	{
		printf("Error: No channel attached\n");
		return -1;
	}

	sprintf(sLogFile, "%s/%s", "/tmp/buslog", inet_ntoa(*(struct in_addr *)&iProcID));
	printf("log %s\n", sLogFile);
	InitLogFile(&pstBus->stLog, sLogFile, 0, 5, 1000000);

	return 0;
}

int BusConnectRelayServer(BusInterface *pstBus, char *sFile, int iProcID)
{
FILE *f;
BusConfig stBusCfg;
char sID[MAX_CFG_ID_LEN], sNext[MAX_CFG_ID_LEN];
AttachChannel *pstAch;

	if (!(f = fopen(sFile, "r"))) {
		printf("Error open bus config: %s\n", sFile);
		return -1;
	}
	memset(pstBus, 0, sizeof(BusInterface));
	pstBus->iProcID = iProcID;
	InitSection(&stBusCfg, astBusConfigDesc);
	sNext[0] = 0;
	while (!feof(f)) {
		if (ReadSection(f, &stBusCfg, astBusConfigDesc, sID, sNext) != 0) continue;
		if (pstBus->iAttach >= MAX_ATTACH_CHANNEL) {
			break;
		}
		pstAch = &pstBus->astAttach[pstBus->iAttach];
		pstAch->iDst = -1;
		if (!cistrcmp(sID, "Channel")) {
			if (iProcID == stBusCfg.stChnCfg.iUpRelay && !cistrcmp(stBusCfg.stChnCfg.sUpRelayType, "server")) {
				pstAch->stChannel.iSide = CHANNEL_SIDE_UP;
			}
			else if (iProcID == stBusCfg.stChnCfg.iDownRelay && !cistrcmp(stBusCfg.stChnCfg.sDownRelayType, "server")) {
				pstAch->stChannel.iSide = CHANNEL_SIDE_DOWN;
			}
			else {
				InitMember(&stBusCfg.stChnCfg, astBusConfigDesc[0].stMember);
				continue;
			}
			printf("Relay server attach: %d   %s --> ", stBusCfg.stChnCfg.iKey, inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iDown));
			printf("%s   ", inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iUp));
			if (AttachOneChannel(&pstAch->stChannel, &stBusCfg.stChnCfg) == 0) {
				pstBus->iAttach++;
				printf("Done. %d\n", pstBus->iAttach);
			} else printf("Fail\n");
			InitMember(&stBusCfg.stChnCfg, astBusConfigDesc[0].stMember);
		}
	}
	printf("...%d....\n", pstBus->iAttach);
	fclose(f);
	sprintf(sFile, "%s/%s", stBusCfg.stLog.sLogPath, inet_ntoa(*(struct in_addr *)&iProcID));
	printf("log %s\n", sFile);
	InitLogFile(&pstBus->stLog, sFile, 0, 5, 1000000);
	return 0;
}

int BusConnectRelayClient(BusInterface *pstBus, char *sFile, int iProcID)
{
FILE *f;
BusConfig stBusCfg;
char sID[MAX_CFG_ID_LEN], sNext[MAX_CFG_ID_LEN];
AttachChannel *pstAch;
int iFound;

	if (!(f = fopen(sFile, "r"))) {
		printf("Error open bus config: %s\n", sFile);
		return -1;
	}
	memset(pstBus, 0, sizeof(BusInterface));
	pstBus->iProcID = iProcID;
	InitSection(&stBusCfg, astBusConfigDesc);
	sNext[0] = 0;
	pstAch = &pstBus->astAttach[0];
	iFound = 0;
	while (!feof(f)) {
		if (ReadSection(f, &stBusCfg, astBusConfigDesc, sID, sNext) != 0) continue;
		if (pstBus->iAttach >= MAX_ATTACH_CHANNEL) {
			break;
		}
		if (!cistrcmp(sID, "Channel")) {
			if (!cistrcmp(stBusCfg.stChnCfg.sUpRelayType, "client")) {
				pstAch->stChannel.iSide = CHANNEL_SIDE_UP;
				pstAch->iDst = stBusCfg.stChnCfg.iDown;
				if (iProcID == 0) {
					iFound++;
					pstBus->iProcID = stBusCfg.stChnCfg.iUpRelay;
					if (fork() == 0) goto found;
				} else if (iProcID == stBusCfg.stChnCfg.iUpRelay) goto found;
			}

			if (!cistrcmp(stBusCfg.stChnCfg.sDownRelayType, "client")) {
				pstAch->stChannel.iSide = CHANNEL_SIDE_DOWN;
				pstAch->iDst = stBusCfg.stChnCfg.iUp;
				if (iProcID == 0) {
					iFound++;
					pstBus->iProcID = stBusCfg.stChnCfg.iDownRelay;
					if (fork() == 0) goto found;
				} else if (iProcID == stBusCfg.stChnCfg.iDownRelay) goto found;
			}
			InitMember(&stBusCfg.stChnCfg, astBusConfigDesc[0].stMember);
			continue;
		}
	}
	if (!iFound) printf("No valid relay client channel.\n");
	return -1;

found:
	fclose(f);
	printf("Relay client attach: %d   %s --> ", stBusCfg.stChnCfg.iKey, inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iDown));
	printf("%s   ", inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iUp));
	if (AttachOneChannel(&pstAch->stChannel, &stBusCfg.stChnCfg) == 0) {
		pstBus->iAttach = 1;
		printf("Done. %d %s\n", pstBus->iAttach, inet_ntoa(*(struct in_addr *)&pstAch->stChannel.pstHead->iDown));
	} else {
		printf("Fail\n");
		return -1;
	}
	sprintf(sFile, "%s/%s", stBusCfg.stLog.sLogPath, inet_ntoa(*(struct in_addr *)&pstBus->iProcID));
	printf("log %s\n", sFile);
	InitLogFile(&pstBus->stLog, sFile, 0, 5, 1000000);
	return 0;
}

int BusConfigIsRepeat(int iShmKey)
{
	static int aiExistShmKey[1000];
	static int iExistNum = 0;
	int i;

	for (i = 0; i < iExistNum; i++) {
		if (aiExistShmKey[i] == iShmKey) {
			break;
		}
	}

	if (i == iExistNum) {
		aiExistShmKey[iExistNum] = iShmKey;
		iExistNum ++;
		return 0;
	}
	else {
		return 1;
	}
}

int BusConnectRelayServerDir(BusInterface *pstBus, char *sDir, int iProcID)
{
char sFile[80];
FILE *f;
BusConfig stBusCfg;
char sID[MAX_CFG_ID_LEN], sNext[MAX_CFG_ID_LEN];
AttachChannel *pstAch;
DIR* pstDir;
struct dirent* pstEntry;
int iLen;

	memset(pstBus, 0, sizeof(BusInterface));
	pstBus->iProcID = iProcID;
	InitSection(&stBusCfg, astBusConfigDesc);
	sNext[0] = 0;
	
	pstDir	=	opendir(sDir);

	if( !pstDir )
	{
		printf("Error open bus config dir: %s\n", sDir);
		return -1;
	}

	while(pstEntry=readdir(pstDir))
	{
		iLen = strlen(pstEntry->d_name);
		if ((iLen <= 4) || (strcmp(pstEntry->d_name+(iLen-4), ".bus") != 0)) continue;
		if( sDir[strlen(sDir)-1]=='/' )
			sprintf(sFile, "%s%s", sDir, pstEntry->d_name);
		else
			sprintf(sFile, "%s/%s", sDir, pstEntry->d_name);
			
		if (!(f = fopen(sFile, "r"))) {
			printf("Error open bus config: %s\n", sFile);
			return -1;
		}
		
		while (!feof(f)) {
			if (ReadSection(f, &stBusCfg, astBusConfigDesc, sID, sNext) != 0) continue;
			if (pstBus->iAttach >= MAX_ATTACH_CHANNEL) {
				break;
			}
			pstAch = &pstBus->astAttach[pstBus->iAttach];
			pstAch->iDst = -1;
			if (!cistrcmp(sID, "Channel")) {
				if (iProcID == stBusCfg.stChnCfg.iUpRelay && !cistrcmp(stBusCfg.stChnCfg.sUpRelayType, "server") && !BusConfigIsRepeat(stBusCfg.stChnCfg.iKey)) {
					pstAch->stChannel.iSide = CHANNEL_SIDE_UP;
				}
				else if (iProcID == stBusCfg.stChnCfg.iDownRelay && !cistrcmp(stBusCfg.stChnCfg.sDownRelayType, "server") && !BusConfigIsRepeat(stBusCfg.stChnCfg.iKey)) {
					pstAch->stChannel.iSide = CHANNEL_SIDE_DOWN;
				}
				else {
					InitMember(&stBusCfg.stChnCfg, astBusConfigDesc[0].stMember);
					continue;
				}
				printf("Relay server attach: %d   %s --> ", stBusCfg.stChnCfg.iKey, inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iDown));
				printf("%s   ", inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iUp));
				if (AttachOneChannel(&pstAch->stChannel, &stBusCfg.stChnCfg) == 0) {
					pstBus->iAttach++;
					printf("Done. %d\n", pstBus->iAttach);
				} else printf("Fail\n");
				InitMember(&stBusCfg.stChnCfg, astBusConfigDesc[0].stMember);
			}
		}
		fclose(f);
	}
	printf("...%d....\n", pstBus->iAttach);
	closedir(pstDir);
	sprintf(sFile, "%s/%s", stBusCfg.stLog.sLogPath, inet_ntoa(*(struct in_addr *)&iProcID));
	printf("log %s\n", sFile);
	InitLogFile(&pstBus->stLog, sFile, 0, 5, 1000000);
	return 0;
}

int BusConnectRelayClientDir(BusInterface *pstBus, char *sDir, int iProcID)
{
char sFile[80];
FILE *f;
BusConfig stBusCfg;
char sID[MAX_CFG_ID_LEN], sNext[MAX_CFG_ID_LEN];
AttachChannel *pstAch;
int iFound;
DIR* pstDir;
struct dirent* pstEntry;
int iLen;

	memset(pstBus, 0, sizeof(BusInterface));
	pstBus->iProcID = iProcID;
	InitSection(&stBusCfg, astBusConfigDesc);
	sNext[0] = 0;
	pstAch = &pstBus->astAttach[0];
	iFound = 0;
	
	pstDir	=	opendir(sDir);

	if( !pstDir )
	{
		printf("Error open bus config dir: %s\n", sDir);
		return -1;
	}

	while(pstEntry=readdir(pstDir))
	{
		iLen = strlen(pstEntry->d_name);
		if ((iLen <= 4) || (strcmp(pstEntry->d_name+(iLen-4), ".bus") != 0)) continue;
		if( sDir[strlen(sDir)-1]=='/' )
			sprintf(sFile, "%s%s", sDir, pstEntry->d_name);
		else
			sprintf(sFile, "%s/%s", sDir, pstEntry->d_name);
			
		if (!(f = fopen(sFile, "r"))) {
			printf("Error open bus config: %s\n", sFile);
			return -1;
		}
		while (!feof(f)) {
			if (ReadSection(f, &stBusCfg, astBusConfigDesc, sID, sNext) != 0) continue;
			if (pstBus->iAttach >= MAX_ATTACH_CHANNEL) {
				break;
			}
			if (!cistrcmp(sID, "Channel")) {
				if (!cistrcmp(stBusCfg.stChnCfg.sUpRelayType, "client")) {
					pstAch->stChannel.iSide = CHANNEL_SIDE_UP;
					pstAch->iDst = stBusCfg.stChnCfg.iDown;
					if (iProcID == 0 && !BusConfigIsRepeat(stBusCfg.stChnCfg.iKey)) {
						iFound++;
						pstBus->iProcID = stBusCfg.stChnCfg.iUpRelay;
						if (fork() == 0) goto found;
					} else if (iProcID == stBusCfg.stChnCfg.iUpRelay) goto found;
				}
	
				if (!cistrcmp(stBusCfg.stChnCfg.sDownRelayType, "client")) {
					pstAch->stChannel.iSide = CHANNEL_SIDE_DOWN;
					pstAch->iDst = stBusCfg.stChnCfg.iUp;
					if (iProcID == 0 && !BusConfigIsRepeat(stBusCfg.stChnCfg.iKey)) {
						iFound++;
						pstBus->iProcID = stBusCfg.stChnCfg.iDownRelay;
						if (fork() == 0) goto found;
					} else if (iProcID == stBusCfg.stChnCfg.iDownRelay) goto found;
				}
				InitMember(&stBusCfg.stChnCfg, astBusConfigDesc[0].stMember);
				continue;
			}
		}
		fclose(f);
	}
	closedir(pstDir);
	if (!iFound) printf("No valid relay client channel.\n");
	return -1;

found:
	fclose(f);
	closedir(pstDir);
	printf("Relay client attach: %d   %s --> ", stBusCfg.stChnCfg.iKey, inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iDown));
	printf("%s   ", inet_ntoa(*(struct in_addr *)&stBusCfg.stChnCfg.iUp));
	if (AttachOneChannel(&pstAch->stChannel, &stBusCfg.stChnCfg) == 0) {
		pstBus->iAttach = 1;
		printf("Done. %d %s\n", pstBus->iAttach, inet_ntoa(*(struct in_addr *)&pstAch->stChannel.pstHead->iDown));
	} else {
		printf("Fail\n");
		return -1;
	}
	sprintf(sFile, "%s/%s", stBusCfg.stLog.sLogPath, inet_ntoa(*(struct in_addr *)&pstBus->iProcID));
	printf("log %s\n", sFile);
	InitLogFile(&pstBus->stLog, sFile, 0, 5, 1000000);
	return 0;
}

Channel* FindChannel(BusInterface *pstBus, int iDst)
{
int i;

	for (i = 0; i < pstBus->iAttach; i++) {
		if (pstBus->astAttach[i].iDst == iDst) return &pstBus->astAttach[i].stChannel;
	}
	return NULL;
}

int LogBus(BusInterface *pstBus, char *sFormat, ...)
{
va_list ap;

	va_start(ap, sFormat);
	Logv(&pstBus->stLog, sFormat, ap);
	va_end(ap);
}

int BusCheckFlag(BusInterface *pstBus, BusPkg *pstPkg, const char *sDesc)
{
short shFlag;
char sBuf[100];

	if( !s_iLog )
		return 0;

	shFlag = ntohs(*(short *)pstPkg->stHead.sFlag);
	if (shFlag & BUS_TRACE) {
		sprintf(sBuf, "check Flag: %s --> ", inet_ntoa(*(struct in_addr *)pstPkg->stHead.sSrc));
		strcat(sBuf, inet_ntoa(*(struct in_addr *)pstPkg->stHead.sDst));
		// Log(&pstBus->stLog, 2, "Bus trace %s,  %s\n%s", sDesc, sBuf, HexString((char *)pstPkg, BusPkgLen(&pstPkg->stHead)));
		LogBus(pstBus, "Bus trace %s,  %s\n%s", sDesc, sBuf, HexString((char *)pstPkg, BusPkgLen(&pstPkg->stHead)));
	}

	return 0;
}

int BusSend(BusInterface *pstBus, int iDst, char *sData, int iSize, short shFlag)
{
BusPkg *pstPkg;
Channel *pstChannel;

	if (iSize > MAX_BUS_PKG_DATA - 4) return -1;
	if (!(pstChannel = FindChannel(pstBus, iDst))) return -2;
	pstPkg = &pstBus->stSendPkg;
	memset(pstPkg, 0, sizeof(BusPkgHead));
	pstPkg->stHead.cVer = BUS_VERSION;
	pstPkg->stHead.cHeadLen = sizeof(BusPkgHead) / 4 + 1;
	*(int *)pstPkg->stHead.sBodyLen = htonl(iSize);
	*(int *)pstPkg->stHead.sDst = iDst;
	*(int *)pstPkg->sData = *(int *)pstPkg->stHead.sSrc = pstBus->iProcID;
	*(short *)pstPkg->stHead.sFlag = htons(shFlag);
	pstPkg->stHead.cRoute = 1;
	memcpy(pstPkg->sData + 4, sData, iSize);
	if (ChannelPush(pstChannel, pstPkg) < 0) return -3;
	BusCheckFlag(pstBus, pstPkg, "Send");
	return 0;
}

int BusForward(BusInterface *pstBus, int iDst, char *sData, int iSize, short shFlag)
{
BusPkg *pstPkg;
Channel *pstChannel;
int iHeadLen;

	iHeadLen = BusHeadLen(pstBus->pstHead);
	if (iSize > sizeof(BusPkg) - iHeadLen - 4) return -1;
	if (!(pstChannel = FindChannel(pstBus, iDst))) return -2;
	pstPkg = &pstBus->stSendPkg;
	memcpy(pstPkg, pstBus->pstHead, iHeadLen);
	*(int *)pstPkg->stHead.sBodyLen = htonl(iSize);
	*(int *)pstPkg->stHead.sDst = iDst;
	*(int *)((char *)pstPkg + iHeadLen) = *(int *)pstPkg->stHead.sSrc = pstBus->iProcID;
	*(short *)pstPkg->stHead.sFlag |= htons(shFlag);
	pstPkg->stHead.cRoute++;
	pstPkg->stHead.cHeadLen++;
	memcpy((char *)pstPkg + iHeadLen + 4, sData, iSize);
	if (ChannelPush(pstChannel, pstPkg) < 0) return -3;
	BusCheckFlag(pstBus, pstPkg, "Forward");
	return 0;
}

int GetBackDst(BusInterface *pstBus)
{
	if (!pstBus->pstHead->cRoute) return 0;
	return *(int *)(pstBus->stRecvPkg.sData + (pstBus->pstHead->cRoute - 1) * 4);
}

int BusBackward(BusInterface *pstBus, char *sData, int iSize, short shFlag)
{
BusPkg *pstPkg;
Channel *pstChannel;
int iHeadLen, iDst;

	if (!(iDst = GetBackDst(pstBus))) return -1;
	iHeadLen = BusHeadLen(pstBus->pstHead) - 4;
	if (iSize > sizeof(BusPkg) - iHeadLen) return -2;
	if (!(pstChannel = FindChannel(pstBus, iDst))) return -3;
	pstPkg = &pstBus->stSendPkg;
	memcpy(pstPkg, pstBus->pstHead, iHeadLen);
	*(int *)pstPkg->stHead.sBodyLen = htonl(iSize);
	*(int *)pstPkg->stHead.sDst = iDst;
	*(int *)pstPkg->stHead.sSrc = pstBus->iProcID;
	*(short *)pstPkg->stHead.sFlag |= htons(shFlag);
	pstPkg->stHead.cRoute--;
	pstPkg->stHead.cHeadLen--;
	memcpy((char *)pstPkg + iHeadLen, sData, iSize);
	// printf("%d %s\n", iHeadLen, sData);
	if (ChannelPush(pstChannel, pstPkg) < 0) return -4;
	BusCheckFlag(pstBus, pstPkg, "Backward");
	return 0;
}

int BusDeliver(BusInterface *pstBus, int iDst, char *sData, int iSize, short shFlag)
{
BusPkg *pstPkg;
Channel *pstChannel;
int iHeadLen;

	iHeadLen = BusHeadLen(pstBus->pstHead);
	if (iSize > sizeof(BusPkg) - iHeadLen) return -1;
	if (!(pstChannel = FindChannel(pstBus, iDst))) return -2;
	pstPkg = &pstBus->stSendPkg;
	memcpy(pstPkg, pstBus->pstHead, iHeadLen);
	*(int *)pstPkg->stHead.sBodyLen = htonl(iSize);
	*(int *)pstPkg->stHead.sDst = iDst;
	*(int *)pstPkg->stHead.sSrc = pstBus->iProcID;
	*(short *)pstPkg->stHead.sFlag |= htons(shFlag);
	memcpy(pstBus->sData, sData, iSize);
	if (ChannelPush(pstChannel, pstPkg) < 0) return -3;
	BusCheckFlag(pstBus, pstPkg, "Deliver");
	return 0;
}

int BusRealRecv(BusInterface *pstBus, char cType)
{
static int iGetPkg = 0;
int i, iStart, iRet;
char sBuf[100];

	if (pstBus->iAttach <= 0) return ERR_CHANNEL_EMPTY;
	if( iGetPkg < 10 )
	{
		i	=	pstBus->iLast ;
	}
	else
	{
		i	=	(pstBus->iLast + 1) % pstBus->iAttach;
		iGetPkg	=	0;
	}

	iStart	=	i;

//	i = iStart = iGetPkg < 10 ? pstBus->iLast : (pstBus->iLast + 1) % pstBus->iAttach;
	while (1) {
		if (cType) {
			if (pstBus->astAttach[i].iDst >= 0) iRet = ChannelAuthGet(&pstBus->astAttach[i].stChannel, &pstBus->stRecvPkg);
			else iRet = ERR_CHANNEL_EMPTY;
		} else iRet = ChannelGet(&pstBus->astAttach[i].stChannel, &pstBus->stRecvPkg);
		if (iRet == 0) {
			iGetPkg ++;
			pstBus->pstHead = &pstBus->stRecvPkg.stHead;
			pstBus->sData = (char *)&pstBus->stRecvPkg + BusHeadLen(pstBus->pstHead);
			pstBus->iSize = BusBodyLen(pstBus->pstHead);
			pstBus->iSide = pstBus->astAttach[i].stChannel.iSide;
			pstBus->iSrc = *(int *)pstBus->stRecvPkg.stHead.sSrc;
			pstBus->iLast = i;
			BusCheckFlag(pstBus, &pstBus->stRecvPkg, "Recv");
			return 0;
		} else if (iRet != ERR_CHANNEL_EMPTY) printf("get ret: %d\n", iRet);
		i = (i + 1) % pstBus->iAttach;
		iGetPkg = 0;
		if (i == iStart) return ERR_CHANNEL_EMPTY;
	}
}

int BusRealRecvChannel(BusInterface *pstBus, int iChannel, char cType)
{
	static int iGetPkg = 0;
	int i, iStart, iRet;
	char sBuf[100];
	
	if (pstBus->iAttach <= 0) return ERR_CHANNEL_EMPTY;
	if ( iChannel>=pstBus->iAttach ) return ERR_CHANNEL_EMPTY;

	i = iChannel;

	if (cType)
	{
		if (pstBus->astAttach[i].iDst >= 0) 
			iRet = ChannelAuthGet(&pstBus->astAttach[i].stChannel, &pstBus->stRecvPkg);
		else
			iRet = ERR_CHANNEL_EMPTY;
	} 
	else
		iRet = ChannelGet(&pstBus->astAttach[i].stChannel, &pstBus->stRecvPkg);

	if (iRet == 0)
	{
		pstBus->pstHead = &pstBus->stRecvPkg.stHead;
		pstBus->sData = (char *)&pstBus->stRecvPkg + BusHeadLen(pstBus->pstHead);
		pstBus->iSize = BusBodyLen(pstBus->pstHead);
		pstBus->iSide = pstBus->astAttach[i].stChannel.iSide;
		pstBus->iSrc = *(int *)pstBus->stRecvPkg.stHead.sSrc;
		pstBus->iLast = i;
		BusCheckFlag(pstBus, &pstBus->stRecvPkg, "Recv");
		return 0;
	}
	else if (iRet != ERR_CHANNEL_EMPTY) 
		printf("get ret: %d\n", iRet);

	return ERR_CHANNEL_EMPTY;
}

int BusRecv(BusInterface *pstBus)
{
	return BusRealRecv(pstBus, BUS_PROC);
}

//
int GetChannelIdxByEntity( BusInterface *pstBus , int iDst ) 
{
	int iIndex = 0 ;
	for( iIndex = 0 ; iIndex < pstBus->iAttach ; iIndex++ )
	{
		if (pstBus->astAttach[iIndex].iDst == iDst)
		{
			return iIndex ;
		}
	}
	if( iIndex >= pstBus->iAttach)
	{
		return -1 ;
	}

	return -1 ;
}


int BusRecvSpec( BusInterface *pstBus , int iDst ) 
{
	int iChannel = 0 ;
	int iRetVal = 0 ;

	if( !pstBus || iDst == 0 )
	{
		return -1 ;
	}
	
	iChannel = GetChannelIdxByEntity( pstBus , iDst );
	if( iChannel < 0 )
	{
		return -2 ;
	}
	
	iRetVal = BusRealRecvChannel(pstBus , iChannel , BUS_PROC) ;	
	if( iRetVal < 0 )
	{
		return -3 ;
	}
 
	return 0 ;
}

