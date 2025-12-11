/*
**  @file logdump.c
**  this is one of tsf4g tools, used to dump local bin log to stout or to net
**  Id
**  @author Author  kent
**  @date Date  2009.1.20
**  @version Revision
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "tapp/tapp.h"
#include "tdr/tdr.h"
#include "tlog/tlog.h"
#include "tlog/tlogbin.h"
#include "tloghelp/tlogload.h"
#include "logdump_conf_desc.h"
#include "version.h"
#include <ftw.h>


#define DUMP_START "dumpstart"
#define DUMP_END "dumpend"
#define DUMP_LOGPATH "logpath"

#define FTW_MAX_FD 48
#define DEFAULT_BUFF 204800

typedef struct tagLogFileEnt	LOGFILEENT;
typedef struct tagLogFileEnt	*LLOGFILEENT;
struct tagLogFileEnt
{
	char szLogFilePath[MAX_PATH];
	int iFileSize;
	LOGFILEENT *pstNext;	
};

static char gs_szBuff[0x10000];

struct taglogdumpEnv
{
	LOGDUMPCONF *pstConf;
	TLOGCATEGORYINST* pstLogCat;
	TLOGNET stLogNet;
	int iTickProcNum;
	LOGFILEENT *pstLogFileLinkHead;
	LOGFILEENT *pstLogFileLinkPtr;
	char szLogPath[MAX_PATH];
	struct timeval stStart;
	struct timeval stEnd;
	FILE *fp;
	int iDumpTotal; 
	int iError; 
	struct timeval stLast;//record the last bin log time  
	int iTickTotal; 
	int iLimit; 
	char* pszBuff;
	int iBuff;
	int iOff;
	int iData;
	int iMinHeadLen;
	LPTDRMETA pstLogMeta; //bin log 中真正内容的meta
	char *pszHost;
	int iHost;
};
typedef struct taglogdumpEnv		LOGDUMPENV;
typedef struct taglogdumpEnv		*LLOGDUMPENV;


static TAPPCTX gs_stAppCtx;
extern unsigned char g_szMetalib_logdump[];

LOGDUMPENV gs_stEnv;

/**获取字符串模式(x,y)中的两个数值
*@param[in]       pszArg  输入字符串
*@param[in,out]   piFirst  第一个参数的指针   
*@param[in,out]   piSecond  第二个参数的指针 
*@retval 0   处理成功
*@retval <0  处理失败
*/
int dumparg_get(char *pszArg, long *piFirst, long *piSecond)
{
	char szTmp[128];
	char *ptr1 = NULL;
	char *ptr2 = NULL;
	int iLen = 0;

	ptr1 = strchr(pszArg, '(');
	if (NULL == ptr1)
	{
		return -1;
	}
	
	ptr2 = strchr(ptr1, ',');
	if (NULL == ptr2 || ptr1 >= ptr2)
	{
		return -1;
	}

	iLen = ptr2-ptr1+1;
	memcpy(szTmp, ptr1+1, iLen);
	szTmp[iLen] = 0;
	*piFirst = atol(szTmp);

	ptr1 = ptr2;
	ptr2 = strchr(ptr2, ')');
	if (NULL == ptr2 || ptr1 >= ptr2)
	{
		return -1;
	}
	
	iLen = ptr2-ptr1+1;
	memcpy(szTmp, ptr1+1, iLen);
	szTmp[iLen] = 0;
	*piSecond= atol(szTmp);
	
	return 0;
}

/**记录遍历的目录中的bin log 文件信息
*@param[in]   file  文件路径 
*@param[in]   sb    文件的stat 结构指针
*@param[in]   flag  文件标志
*@retval 0   处理成功
*@retval <0  处理失败
*/
int logdump_getfile(const char *file, const struct stat *sb, int flag)
{
	LOGFILEENT *pstLogFileEnt = NULL;
		
	if (FTW_F == flag)
	{
		pstLogFileEnt = malloc(sizeof(LOGFILEENT));
		if (NULL == pstLogFileEnt)
		{
			exit(-1);
		}
		STRNCPY(pstLogFileEnt->szLogFilePath, file, sizeof(pstLogFileEnt->szLogFilePath));
		pstLogFileEnt->iFileSize = sb->st_size;
		pstLogFileEnt->pstNext = NULL;
		
		if (NULL == gs_stEnv.pstLogFileLinkPtr)
		{
			gs_stEnv.pstLogFileLinkPtr = pstLogFileEnt;			
		}
		else
		{
			gs_stEnv.pstLogFileLinkPtr->pstNext = pstLogFileEnt;
		}

		if (NULL == gs_stEnv.pstLogFileLinkHead)
		{
			gs_stEnv.pstLogFileLinkHead = gs_stEnv.pstLogFileLinkPtr;
		}
	}
	
	return 0;
}

int logdump_init(TAPPCTX *pstAppCtx, LOGDUMPENV *pstEnv)
{
	int opt = 0;
	LPTDRMETALIB pstLib = NULL;
			
	if (NULL == pstAppCtx->stConfData.pszBuff || 0 == pstAppCtx->stConfData.iMeta 
		|| NULL == pstAppCtx->pszConfFile )
	{
		return -1;
	}
	pstEnv->pstConf = (LOGDUMPCONF *)pstAppCtx->stConfData.pszBuff;

	if (0 > tapp_get_category(TLOG_DEF_CATEGORY_TEXTROOT, (int *)(&pstEnv->pstLogCat)))	
	{		
		printf("tapp_get_category run fail\n");
		return -1;	
	}

	if (pstEnv->pstConf->iMaxBuff > 0)
	{
		pstEnv->iBuff = pstEnv->pstConf->iMaxBuff;
	}
	else
	{
		pstEnv->iBuff = DEFAULT_BUFF;
	}

	pstEnv->pszBuff = malloc(pstEnv->iBuff);
	if (NULL == pstEnv->pszBuff)
	{
		return -1;
	}

	pstEnv->iLimit = pstEnv->pstConf->iDumpTotal;

	if (DUMP_NET == pstEnv->pstConf ->iDumpFlag)
	{
		if (0 > tlognet_init(&pstEnv->stLogNet, &pstEnv->pstConf->stDumpDst.stDumpNet))
		{
			printf("net init fail, please check net config\n");
			return -1;
		}
	}
	else if (DUMP_STDOUT == pstEnv->pstConf ->iDumpFlag)
	{
		int iLen;

		iLen = strlen(pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaLibPath);
		if (0 == strncmp(pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaLibPath+iLen-4, ".xml", 4) )
		{
			if (0 > tdr_create_lib_file(&pstLib, pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaLibPath, 1, stderr))
			{
				printf("load lib from %s fail\n", pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaLibPath);
				return -1;
			}
		}
		else
		{
			if (0 > tdr_load_metalib(&pstLib, pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaLibPath))
			{
				printf("load lib from %s fail\n", pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaLibPath);
				return -1;
			}
		}
		
		pstEnv->pstLogMeta = tdr_get_meta_by_name(pstLib, pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaName);
		if (NULL == pstEnv->pstLogMeta)
		{
			printf("get meta %s from %s fail\n", pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaName, pstEnv->pstConf->stDumpDst.stDumpStdout.szMetaLibPath);
			return -1;
		}

		pstEnv->iHost = tdr_get_meta_size(pstEnv->pstLogMeta);
		pstEnv->pszHost = malloc(pstEnv->iHost);
		if (NULL == pstEnv->pszHost)
		{
			return -1;
		}
	}

	STRNCPY(pstEnv->szLogPath, pstEnv->pstConf->szLogPath, sizeof(pstEnv->szLogPath));

	if (pstAppCtx->iTimer > 0) //进行频率控制
	{
		pstEnv->iTickProcNum = pstEnv->pstConf->iDumpSpeed / (1000.0/pstAppCtx->iTimer);
	}
	if (0 != pstEnv->pstConf->iDumpSpeed && 0 == pstEnv->iTickProcNum)
	{
		pstEnv->iTickProcNum = 1;
	}

	for( opt=optind; opt<pstAppCtx->argc; opt++ ) //处理额外的启动参数
	{
		if(strstr(pstAppCtx->argv[opt], DUMP_START) )
		{
				if (0 > dumparg_get(pstAppCtx->argv[opt], &pstEnv->stStart.tv_sec, &pstEnv->stStart.tv_usec))
				{
					printf("argv %s error\n", pstAppCtx->argv[opt]);
					return -1;
				}
		}
		else if (strstr(pstAppCtx->argv[opt], DUMP_END))
		{
			if (0 > dumparg_get(pstAppCtx->argv[opt], &pstEnv->stEnd.tv_sec, &pstEnv->stEnd.tv_usec))
			{
				printf("argv %s error\n", pstAppCtx->argv[opt]);
				return -1;
			}
		}
		else if (strstr(pstAppCtx->argv[opt], DUMP_LOGPATH))
		{
			STRNCPY(pstEnv->szLogPath, pstAppCtx->argv[opt], sizeof(pstEnv->szLogPath));
		}
		else
		{
			continue;
		}
	}

	pstEnv->iMinHeadLen = sizeof(TLOGBINBASE);
	

	printf("start=(%ld, %ld), end=(%ld,%ld), logpath=%s\n", pstEnv->stStart.tv_sec, pstEnv->stStart.tv_usec,
		pstEnv->stEnd.tv_sec, pstEnv->stEnd.tv_usec, pstEnv->szLogPath);

	pstEnv->pstLogFileLinkHead = NULL;
	pstEnv->pstLogFileLinkPtr = NULL;
	if (0 > ftw(pstEnv->pstConf->szLogPath, logdump_getfile, FTW_MAX_FD))
	{
		printf("error in log path %s\n", pstEnv->pstConf->szLogPath);
		return -1;
	}

	pstEnv->pstLogFileLinkPtr = pstEnv->pstLogFileLinkHead;
	pstEnv->fp = NULL;
		
	printf("logdump start\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "logdump start");

	return 0;
}

int logdump_fini(TAPPCTX *pstAppCtx, LOGDUMPENV *pstEnv)
{
	if (pstEnv->fp)
	{
		fclose(pstEnv->fp);
		pstEnv->fp = NULL;
	}

	if (DUMP_NET == pstEnv->pstConf ->iDumpFlag)
	{
		tlognet_fini(&pstEnv->stLogNet);
	}

	printf("dump total=%d, last log timestamp=(%ld,%ld)", pstEnv->iDumpTotal, pstEnv->stLast.tv_sec, pstEnv->stLast.tv_usec);
	tlog_info(pstEnv->pstLogCat, 0, 0, "dump total=%d, last log timestamp=(%ld,%ld)", 
				pstEnv->iDumpTotal, pstEnv->stLast.tv_sec, pstEnv->stLast.tv_usec);
		
	printf("logdump finish\n");
	tlog_info(pstEnv->pstLogCat, 0, 0, "logdump finish");
	return 0;
}


/**针对每个bin log 进行过滤
*@param[in]   pstAppCtx  Tapp指针
*@param[in]   pstEnv     环境变量指针
*@param[in]   pstLogBinHead  bin log 的头部信息
*@retval 1    需要被过滤掉
*@retval 0    需要被dump
*/
int logdump_filter(TAPPCTX *pstAppCtx, LOGDUMPENV *pstEnv, TLOGBINHEAD *pstLogBinHead)
{
	if(pstEnv->pstConf->stFilterVec.iCount > 0 && 0!=tlogfilter_match_vec(&pstEnv->pstConf->stFilterVec, pstLogBinHead->iID, pstLogBinHead->iCls) )
	{
		return 1;
	}

	if ((pstEnv->stStart.tv_sec != 0 && 
			(pstLogBinHead->stTime.iSec < pstEnv->stStart.tv_sec || 
				(pstLogBinHead->stTime.iSec == pstEnv->stStart.tv_sec && pstLogBinHead->stTime.iUsec < pstEnv->stStart.tv_usec)) ) || 
		(pstEnv->stEnd.tv_sec != 0 && 
			(pstLogBinHead->stTime.iSec > pstEnv->stEnd.tv_sec ||
				(pstLogBinHead->stTime.iSec == pstEnv->stEnd.tv_sec && pstLogBinHead->stTime.iUsec > pstEnv->stEnd.tv_usec))))
	{
		return 1;
	}

	return 0;
}

/**匹配一条完整的bin log 并处理
*@param[in]   pstAppCtx  Tapp指针
*@param[in]   pstEnv     环境变量指针
*@retval 0    处理成功
*@retval <0   处理失败
*/
int logdump_proc_msgone(TAPPCTX *pstAppCtx, LOGDUMPENV *pstEnv)
{
	char* pszMsg = NULL;
	int iLen = 0;
	TLOGBINHEAD stBinHead;
	int iRet = 0;
		
/*	char szTmp[1024];


	if (pstEnv->iData <= 0)
	{
		return -1;
	}
	
	pszMsg = pstEnv->pszBuff + pstEnv->iOff;
	for (i=0; i<pstEnv->iData; i++)
	{
		if (*(pszMsg+i) == '\n')
		{
			iLen = i+1;
			if (iLen < (int )sizeof(szTmp))
			{
				memcpy(szTmp, pszMsg, iLen);
				szTmp[iLen] = 0;
				printf("%s", szTmp);
			}
			pstEnv->iData -= iLen;
			pstEnv->iOff += iLen;
			return 0;
		}
	}*/

	if (pstEnv->iData < pstEnv->iMinHeadLen)
	{
		return -1;
	}

	pszMsg	=	pstEnv->pszBuff + pstEnv->iOff;
	if (0 > tlogbin_ntoh_head(&stBinHead, pszMsg, pstEnv->iData))
	{
		printf("log msg confuse in %s log file\n", pstEnv->pstLogFileLinkPtr->szLogFilePath);
		tlog_error(pstEnv->pstLogCat, 0, 0, "log msg confuse in %s log file", pstEnv->pstLogFileLinkPtr->szLogFilePath);
		pstEnv->iError = 1;
		return -1;
	}

	iLen = stBinHead.chHeadLen + stBinHead.iBodyLen;
	if( pstEnv->iData<iLen )
	{
		return -1;
	}

	if (logdump_filter(pstAppCtx, pstEnv, &stBinHead))
	{
		pstEnv->iOff	+=	iLen;
		pstEnv->iData	-=	iLen;
		return 0;
	}

	if (DUMP_NET == pstEnv->pstConf->iDumpFlag)
	{
		iRet = tlognet_write(&pstEnv->stLogNet, pszMsg, iLen);
		if (iRet != iLen)
		{
			
			iRet = -1;
		}
		else
		{
			iRet = 0;
		}
	}
	else if (DUMP_STDOUT == pstEnv->pstConf->iDumpFlag)
	{
		
		TDRDATA stHost;
		TDRDATA stNet;
				
		iRet = 0;
			
		stNet.pszBuff = pszMsg + stBinHead.chHeadLen;
		stNet.iBuff = iLen - stBinHead.chHeadLen;
		
		stHost.pszBuff	=	pstEnv->pszHost;
		stHost.iBuff	=	pstEnv->iHost;

		if (0 == tdr_ntoh(pstEnv->pstLogMeta, &stHost, &stNet, stBinHead.iBodyVer))
		{
			tdr_output_fp(pstEnv->pstLogMeta, stdout, &stHost, 0, 0);
		}
	}
		
	if (0 > iRet)
	{
		pstEnv->iError = 1;
		return -1;
	}
	else
	{
		pstEnv->iTickTotal++;
		pstEnv->iDumpTotal++;
		pstEnv->stLast.tv_sec = stBinHead.stTime.iSec;
		pstEnv->stLast.tv_usec = stBinHead.stTime.iUsec;
		pstEnv->iOff	+=	iLen;
		pstEnv->iData	-=	iLen;
	}
	
	return 0;
}

/**在一个bin log 文件中匹配多个bin log 并处理
*@param[in]   pstAppCtx  Tapp指针
*@param[in]   pstEnv     环境变量指针
*@retval 0    处理成功
*@retval <0   处理失败
*/
int logdump_proc_msgbatch(TAPPCTX *pstAppCtx, LOGDUMPENV *pstEnv)
{
	int iBuff = 0;
	int iRead = 0;
	int iRet = 0;

	iBuff	=	pstEnv->iBuff - pstEnv->iData;

	iRead	=	fread(pstEnv->pszBuff+pstEnv->iData, 1, iBuff, pstEnv->fp);

	if( iRead<0 )
	{
		printf("fread %s log file error\n", pstEnv->pstLogFileLinkPtr->szLogFilePath);
		tlog_error(pstEnv->pstLogCat, 0, 0, "fread %s log file error", pstEnv->pstLogFileLinkPtr->szLogFilePath);
		pstEnv->iError = 1;
		return -1;
	}

	pstEnv->iData +=	iRead;

	while( 1 )
	{
		iRet	=	logdump_proc_msgone(pstAppCtx, pstEnv);

		if( iRet<0 )
		{
			break;
		}

		if ((pstEnv->iTickProcNum && pstEnv->iTickTotal >= pstEnv->iTickProcNum) ||
			(pstEnv->iLimit && pstEnv->iDumpTotal >= pstEnv->iLimit))
		{
			break;
		}
	}

	if( pstEnv->iData>0 )
	{
		memmove(pstEnv->pszBuff, pstEnv->pszBuff+pstEnv->iOff, pstEnv->iData);
	}
	pstEnv->iOff	=	0;

	return 0;
}

int logdump_proc_file(TAPPCTX *pstAppCtx, LOGDUMPENV *pstEnv)
{
	int iRet = 0;
	
	while( 1 )
	{
		if( ftell(pstEnv->fp)>=pstEnv->pstLogFileLinkPtr->iFileSize)
		{
			fclose(pstEnv->fp);
			pstEnv->fp = NULL;
			pstEnv->pstLogFileLinkPtr = pstEnv->pstLogFileLinkPtr->pstNext;
			break;
		}

		iRet = logdump_proc_msgbatch(pstAppCtx, pstEnv);
		if (iRet < 0)
		{
			fclose(pstEnv->fp);
			pstEnv->fp = NULL;
			tapp_exit_mainloop();
			return -1;
		}
		else if (pstEnv->iError)
		{
			tapp_exit_mainloop();
			return -1;
		}

		if( feof(pstEnv->fp) )
		{
			fclose(pstEnv->fp);
			pstEnv->fp = NULL;
			pstEnv->pstLogFileLinkPtr = pstEnv->pstLogFileLinkPtr->pstNext;
			break;
		}

		if ((pstEnv->iTickProcNum && pstEnv->iTickTotal >= pstEnv->iTickProcNum) ||
			(pstEnv->iLimit && pstEnv->iDumpTotal >= pstEnv->iLimit))
		{
			break;
		}
	
	}

	return 0;
}

int logdump_tick(TAPPCTX *pstAppCtx, LOGDUMPENV *pstEnv)
{
	pstEnv->iTickTotal = 0;

	while(1)
	{
		if (NULL == pstEnv->fp)
		{
			if (NULL == pstEnv->pstLogFileLinkPtr)
			{
				tapp_exit_mainloop();
				return 0;
			}
			else
			{
				pstEnv->fp = fopen(pstEnv->pstLogFileLinkPtr->szLogFilePath, "rb");
				if (NULL == pstEnv->fp)
				{
					pstEnv->pstLogFileLinkPtr = pstEnv->pstLogFileLinkPtr->pstNext;
					continue;
				}
				setvbuf(pstEnv->fp, gs_szBuff, _IOFBF, sizeof(gs_szBuff));
				pstEnv->iData = 0;
				pstEnv->iOff = 0;
			}
		}

		if (0 > logdump_proc_file(pstAppCtx, pstEnv))
		{
			break;
		}

		if (pstEnv->iLimit && pstEnv->iDumpTotal >= pstEnv->iLimit)
		{
			tapp_exit_mainloop();
			return 0;
		}

		if (pstEnv->iTickProcNum && pstEnv->iTickTotal >= pstEnv->iTickProcNum) 
		{
			break;
		}
	}
	
	return 0;
}

int main(int argc, char* argv[])
{
	int iRet = 0;
	void* pvArg	=	&gs_stEnv;

	memset(&gs_stAppCtx, 0, sizeof(gs_stAppCtx));
	memset(&gs_stEnv, 0, sizeof(gs_stEnv));

	gs_stAppCtx.argc	=	argc;
	gs_stAppCtx.argv	=	argv;

	gs_stAppCtx.pfnInit	=	(PFNTAPPFUNC)logdump_init;
	
	gs_stAppCtx.iLib = (int)g_szMetalib_logdump;
	gs_stAppCtx.stConfData.pszMetaName = "logdumpconf";
	gs_stAppCtx.uiVersion = TAPP_MAKE_VERSION(MAJOR, MINOR, REV, BUILD);
		
	iRet	=	tapp_def_init(&gs_stAppCtx, pvArg);
	if(0 > iRet)
	{
		printf("Error: app Initialization failed.\n");
		return iRet;
	}

	
	gs_stAppCtx.pfnFini	=	(PFNTAPPFUNC)logdump_fini;
	gs_stAppCtx.pfnProc	=	(PFNTAPPFUNC)NULL;
	gs_stAppCtx.pfnTick	=	(PFNTAPPFUNC)logdump_tick;
	gs_stAppCtx.pfnReload	=	(PFNTAPPFUNC)NULL;
	gs_stAppCtx.pfnStop  =   (PFNTAPPFUNC)NULL;
	
	iRet	=	tapp_def_mainloop(&gs_stAppCtx, pvArg);

	tapp_def_fini(&gs_stAppCtx, pvArg);
	
	return iRet;
}


