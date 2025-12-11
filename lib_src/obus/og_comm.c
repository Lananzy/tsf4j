
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/procfs.h>
#include <errno.h>
#include "og_pkg.h"

const char* ZONEIDFILE = "/usr/local/mmog/zone/.ZoneID";
const char* MULTIZONEFILE = "/usr/local/mmog/zone/.MultiZone";

#define SHORT2INT(INT_addr, SH_laddr, SH_haddr) \
		do {*(short*)INT_addr=*SH_laddr; *(((short*)INT_addr)+1)=*SH_haddr; } while(0)

#define INT2SHORT(INT_addr, SH_laddr, SH_haddr) \
		do {*SH_laddr=*(short*)INT_addr; *SH_haddr=*(((short*)INT_addr)+1); } while(0)

int GetZoneID()
{
FILE *f;
char sBuf[100];

	f = fopen(ZONEIDFILE, "r");
	if (!f) return -1;
	fgets(sBuf, sizeof(sBuf), f);
	fclose(f);
	return atoi(sBuf);
}

int GetMultiZone()
{
FILE *f;
char sBuf[100];

	f = fopen(MULTIZONEFILE, "r");
	if (!f) return -1;
	fgets(sBuf, sizeof(sBuf), f);
	fclose(f);
	return atoi(sBuf);
}

unsigned short HeadCheckSum(void* sHead)
{
unsigned short shSum = 0;
int i;

	for (i = 0; i < 8; i++) shSum ^= *(short *)((char *)sHead + i * 2);
	return shSum;
}

int AddString(char* p, char* sStr)
{
	*(int *)p = htonl(strlen(sStr) + 1);
	strcpy(p + 4, sStr);
	return strlen(sStr) + 5;
}

int AddString2(char* p, char* sStr)
{
	strcpy(p, sStr);
	return strlen(sStr) + 1;
}

char* GetString(char* p, char* sStr, int iStrLen, int *piGetLen, int iDataLen)
{
int iLen;
char *p1;

	p1 = p + *piGetLen;
	*piGetLen += 4;
	if (*piGetLen > iDataLen) return NULL;
	iLen = ntohl(*(int *)p1);
	*piGetLen += iLen;
	if (*piGetLen > iDataLen) return NULL;
	if (iLen > iStrLen) return NULL;
	strncpy(sStr, p1 + 4, iLen);
	return sStr;
}

char* AddStr(char* p, int *piLen, char *s)
{
	strcpy(p + *piLen, s);
	*piLen += strlen(s);
	return p;
}

char* AddStr2(char* p, int *piLen, ...)
{
va_list ap;
char *s;

	va_start(ap, piLen);
	s = va_arg(ap, char *);
	while (s) {
		strcpy(p + *piLen, s);
		*piLen += strlen(s);
		s = va_arg(ap, char *);
	}
	va_end(ap);
	return p;
}

int AddChar(char* p, int *piLen, char c)
{
	*(p + *piLen) = c;
	(*piLen)++;
	return 1;
}

char GetChar(char* p, int *piGetLen, int iDataLen)
{
char *p1;

	p1 = p + *piGetLen;
	if (++(*piGetLen) > iDataLen) return 0;
	return *p1;
}

int AddShort(char* p, int *piLen, short sh)
{
	*(short *)(p + *piLen) = htons(sh);
	*piLen += 2;
	return 2;
}

int AddShort2(char* p, int *piLen, short sh)
{
	*(short *)(p + *piLen) = sh;
	*piLen += 2;
	return 2;
}

short GetShort(char* p, int *piGetLen, int iDataLen)
{
char *p1;

	p1 = p + *piGetLen;
	*piGetLen += sizeof(short);
	if (*piGetLen > iDataLen) return 0;
	return ntohs(*(short *)p1);
}

short GetShort2(char* p, int *piGetLen, int iDataLen)
{
char *p1;

	p1 = p + *piGetLen;
	*piGetLen += sizeof(short);
	if (*piGetLen > iDataLen) return 0;
	return *(short *)p1;
}

int AddInt(char* p, int *piLen, int i)
{
	*(int *)(p + *piLen) = htonl(i);
	*piLen += 4;
	return 4;
}

int AddInt2(char* p, int *piLen, int i)
{
	*(int *)(p + *piLen) = i;
	*piLen += 4;
	return 4;
}

int GetInt(char* p, int *piGetLen, int iDataLen)
{
char *p1;

	p1 = p + *piGetLen;
	*piGetLen += sizeof(int);
	if (*piGetLen > iDataLen) return 0;
	return ntohl(*(int *)p1);
}

int GetInt2(char* p, int *piGetLen, int iDataLen)
{
char *p1;

	p1 = p + *piGetLen;
	*piGetLen += sizeof(int);
	if (*piGetLen > iDataLen) return 0;
	return *(int *)p1;
}

int AddBin(char* p, int *piLen, char *sSrc, int iSrcLen)
{
	memcpy(p + *piLen, sSrc, iSrcLen);
	*piLen += iSrcLen;
	return iSrcLen;
}

char* GetBin(char* p, int *piGetLen, char* sDst, int iLen, int iDataLen)
{
char *p1;

	p1 = p + *piGetLen;
	*piGetLen += iLen;
	if (*piGetLen > iDataLen) return 0;
	memcpy(sDst, p1, iLen);
	return sDst;
}

unsigned short PkgLen(void* sHead)
{
	return ntohs(*(short *)sHead) + ntohs(*(short *)(sHead + 2));
}

unsigned short PkgLenInner(void* sHead)
{
	return *(short *)sHead + *(short *)(sHead + 2);
}

unsigned short PkgLen2(void* sHead, int* piHeadLen, int* piBodyLen)
{
	*piHeadLen = ntohs(*(short *)sHead);
	*piBodyLen = ntohs(*(short *)(sHead + 2));
	return *piHeadLen + *piBodyLen;
}

unsigned int MsPass(struct timeval* pstTv1, struct timeval* pstTv2)
{
int iSec;
	iSec = pstTv1->tv_sec - pstTv2->tv_sec;
	if (iSec < 0 || iSec >100000) iSec = 100000;
	return iSec * 1000 + (pstTv1->tv_usec - pstTv2->tv_usec)/1000;
}

struct timeval* TvAddMs(struct timeval* pstTv, int iMs)
{
	pstTv->tv_sec += iMs / 1000;
	pstTv->tv_usec += (iMs % 1000) * 1000;
	if (pstTv->tv_usec >= 1000000) {
		pstTv->tv_usec -= 1000000;
		pstTv->tv_sec ++;
	}
	return pstTv;
}

int TvBefore(struct timeval* pstTv1, struct timeval* pstTv2)
{
	if (pstTv1->tv_sec < pstTv2->tv_sec) return 1;
	if (pstTv1->tv_sec > pstTv2->tv_sec) return 0;
	if (pstTv1->tv_usec < pstTv2->tv_usec) return 1;
	return 0;
}

int IntCompare(const void *p1, const void *p2)
{
register int *pI1, *pI2;

	pI1 = (int *)p1;
	pI2 = (int *)p2;
	return *pI1 - *pI2;
}

int KillPre(int iProcID) 
{
char sFile[80];

	sprintf(sFile, "/tmp/%s.pid", inet_ntoa(iProcID));
	if (KillPrevious(sFile) < 0) {
		printf("Can not kill previous process. exit\n");
		exit(0);
	}
	WritePid(sFile);
	return 0;
}

int GetPidByEntity( int iEntityId) 
{
	char szProcFile[128] ;
	char sPid[10] ;
	char sMyName[50] ; 
	char sKillName[50];
	FILE* pPidFile = NULL ;
	int iPid = -1 ;

	prpsinfo_t prpsinfo;
	
	memset( szProcFile , 0 , sizeof(szProcFile)) ;
	sprintf(szProcFile , "/tmp/%s.pid" , inet_ntoa(iEntityId) ) ;
	pPidFile = fopen(szProcFile , "r") ;
	if( !pPidFile)
	{
		perror(szProcFile) ;
		return -1 ;
	}

	memset( sPid , 0 ,sizeof(sPid)) ;
	if (!fgets(sPid, sizeof(sPid) - 1, pPidFile)) {
		fclose(pPidFile);
		return -1;
	}
	
	fclose(pPidFile);
	iPid = atoi(sPid);

	if (GetPrpsinfo(getpid(), &prpsinfo) < 0) 
	{
		return -1;
	}
	
	memset(sMyName, 0, sizeof(sMyName));
	strncpy(sMyName, prpsinfo.pr_fname, sizeof(sMyName) - 1);

	if (GetPrpsinfo(iPid, &prpsinfo) < 0) 
	{
		return -1 ;	
	}
		
	memset(sKillName, 0, sizeof(sKillName));
	strncpy(sKillName, prpsinfo.pr_fname, sizeof(sKillName) - 1);

	if (strcmp(sMyName, sKillName)) 
	{
		return -1 ;
	}
			
	return iPid ;
}
int SendSig2Entity( int iEntityId , int sig)
{
	int iPid = -1 ;
	int iRetCode = 0 ;
	
	iPid = GetPidByEntity( iEntityId) ;
	if( iPid < 0 )
	{
		return 0 ;
	}

	iRetCode = kill( iPid , sig) ;
	if( iRetCode < 0 )
	{
		return -1 ;
	}

	return 0 ;
}

int StopEntity( int iEntityId)
{
	int iRetCode = 0 ;
	int iPid = -1 ;
	int i = 0 ;

	iRetCode = SendSig2Entity( iEntityId, SIGUSR1) ;
	if( iRetCode < 0 )
	{
		return -1 ;
	}
	/*
	iPid = GetPidByEntity( iEntityId) ;
	if( iPid < 0 )
	{
		return -1 ;
	}

	for( i = 0 ; i < 5 ; i++ )
	{
		if( kill( iPid , 0 ) < 0 )
		{
			switch(errno)	
			{
				case ESRCH:
					return 0 ;
					break ;
				case EPERM :
					return -1 ;
					break ;
				default :
					return -1 ;
					
			}
		}

		sleep(1) ;
	}*/
	
	return 0 ;
}

int KillEntity(int iEntityId)
{
	int iPid = - 1 ;

	iPid = GetPidByEntity( iEntityId) ;
	if( iPid < 0 )
	{
		return -1 ;
	}

	KillProcByPid( iPid ) ;
	
	return 0 ;
}

int TerminateEntity( int iEntityId)
{
	int iPid = - 1 ;
	int iRetCode = 0 ;

	iPid = GetPidByEntity( iEntityId) ;
	if( iPid < 0 )
	{
		return -1 ;
	}

	iRetCode = SendSig2Entity( iEntityId , SIGTERM) ;
	if( iRetCode < 0 )
	{
		return -1 ;
	}

	return WaitProcExit(iPid) ;
	
	
}

