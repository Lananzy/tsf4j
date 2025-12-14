#include "pal/pal.h"
#include "tbus_dyemsg.h"
#include "tbus_kernel.h"
#include "tbus_log.h"


extern LPTLOGCATEGORYINST	g_pstBusLogCat;
extern TBUSGLOBAL g_stBusGlobal ;

/** 根据染色标志位和前一个消息对消息进行染色
*/
int tbus_dye_pkg(IN LPTBUSHEAD a_pstHead, IN LPTBUSHEAD a_pstPreHead, int a_iFlag)
{
	unsigned int dwDyeID;


	assert(NULL != a_pstHead);

	if (!(TBUS_FLAG_START_DYE_MSG & a_iFlag) && !(TBUS_FLAG_KEEP_DYE_MSG & a_iFlag))
	{
		return TBUS_SUCCESS;
	}
	if (TBUS_HEAD_CMD_TRANSFER_DATA != a_pstHead->bCmd)
	{
		return TBUS_SUCCESS;
	}

	if (TBUS_FLAG_KEEP_DYE_MSG & a_iFlag)
	{	/*保持染色*/
		if (NULL == a_pstPreHead)
		{
			return TBUS_SUCCESS;
		}
		if (!(a_pstPreHead->bFlag & TBUS_HEAD_FLAG_TACE))
		{
			return TBUS_SUCCESS;
		}
		if (TBUS_HEAD_CMD_TRANSFER_DATA != a_pstPreHead->bCmd)
		{
			return TBUS_SUCCESS;
		}

		/*取出特征码*/
		dwDyeID = a_pstPreHead->stExtHead.stDataHead.iDyedID;
	}else
	{
		/*启动染色，生成特征码*/
		g_stBusGlobal.dwDyedMsgID++;
		dwDyeID = g_stBusGlobal.dwDyedMsgID;
	}

	a_pstHead->bFlag |= TBUS_HEAD_FLAG_TACE;
	a_pstHead->stExtHead.stDataHead.chIsDyed = 1;
	a_pstHead->stExtHead.stDataHead.iDyedID = dwDyeID;

	return 0;
}

unsigned int tbus_get_dyedmsgid()
{
	return g_stBusGlobal.dwDyedMsgID;
}

void tbus_log_dyedpkg(IN LPTBUSHEAD a_pstHead, IN const char *a_pszOperation)
{
#define TBUS_IPSTR_LEN	32
#define TBUS_MAX_ROUTE_TRACE	10

	char szSrc[TBUS_IPSTR_LEN];
	char szDst[TBUS_IPSTR_LEN];
	char szTmpIP[TBUS_IPSTR_LEN];
	int *pRouteTab;
	char szRoute[TBUS_IPSTR_LEN*TBUS_MAX_ROUTE_TRACE]={0};
	int i;
	struct timeval stStart;
	int iRouteCnt;

	assert(NULL != a_pstHead);

	if (!(a_pstHead->bFlag & TBUS_HEAD_FLAG_TACE))
	{
		return ;
	}

	/*获取最后10个route节点*/
	pRouteTab = &a_pstHead->stExtHead.stDataHead.routeAddr[0];
	iRouteCnt = (int)a_pstHead->stExtHead.stDataHead.bRoute;
	for (i =  iRouteCnt-1; (i >= 0) ; i--)
	{
		int iLen = strlen(szRoute);
		if (i < iRouteCnt - TBUS_MAX_ROUTE_TRACE)
		{
			break;
		}
		STRNCPY(szTmpIP, tbus_addr_ntoa(pRouteTab[i]), sizeof(szTmpIP));
		snprintf(&szRoute[iLen], sizeof(szRoute)-iLen, "%s,", szTmpIP);
	}

	if (NULL == a_pszOperation)
	{
		a_pszOperation = "";
	}
	STRNCPY(szSrc, tbus_addr_ntoa(a_pstHead->iSrc), sizeof(szSrc));
	STRNCPY(szDst, tbus_addr_ntoa(a_pstHead->iDst), sizeof(szDst));


	gettimeofday(&stStart, NULL);
	tbus_log(TLOG_PRIORITY_FATAL,"%-10ld.%06ld %-16s DyedMsg:	%-10lu  %s->%s %s\n", 
		stStart.tv_sec, stStart.tv_usec, a_pszOperation, a_pstHead->stExtHead.stDataHead.iDyedID, 
		szSrc, szDst, szRoute);

}
