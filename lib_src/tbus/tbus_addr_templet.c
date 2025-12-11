#include <assert.h>
#include <string.h>
#include <errno.h>
#include "tbus/tbus.h"
#include "tbus/tbus_macros.h"
#include "tbus_addr_templet.h"
#include "tbus_log.h"
#include "tbus/tbus_error.h"
#include "tbus_desc.h"
#include "tbus_misc.h"

/** 从通信地址模板串某一节的字符串中分析出此节存储所需的位信息
*/
int tbus_add_addrtemplet_segment(OUT LPTBUSADDRTEMPLET a_pstAddrTemplet, IN char *a_pszSeg);


///////////////////////////////////////////////////////////////////////////////////////////////
int tbus_parse_addrtemplet(OUT LPTBUSADDRTEMPLET a_pstAddrTemplet, IN const char *a_pszAddTemplet)
{
	char *pchDot = NULL;
	char *pszAddrTempletDup = NULL;
	int iRet ;
	int iBits;
	int i;

	assert(NULL != a_pstAddrTemplet);
	assert(NULL != a_pszAddTemplet);

	a_pstAddrTemplet->iSegNum = 0;
	tbus_log(TLOG_PRIORITY_DEBUG,"AddrTemplet: %s", a_pszAddTemplet);	
	pszAddrTempletDup = strdup(a_pszAddTemplet);
	if (NULL == pszAddrTempletDup)
	{
		tbus_log(TLOG_PRIORITY_ERROR,"strdup address templet %s failed, %s", a_pszAddTemplet, strerror(errno));
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PARSE_ADDR_TEMPLET);
	}

	/*从模板串向前解析点分法的每一节的信息*/
	do 
	{
		char *pszSeg = NULL;
		pchDot = strrchr(pszAddrTempletDup, TBUS_SPLIT_SEGMENT_CHARACTER);
		if (NULL != pchDot)
		{
			pszSeg = pchDot + 1;
			*pchDot = '\0';
		}else
		{
			pszSeg = pszAddrTempletDup;
		}
		iRet = tbus_add_addrtemplet_segment(a_pstAddrTemplet, pszSeg);
		if (TBUS_ERR_IS_ERROR(iRet))
		{
			break;
		}
	} while(NULL != pchDot);

	
	if (NULL != pszAddrTempletDup)
	{
		free(pszAddrTempletDup);
	}

	iBits = 0;
	for (i = 0; i< a_pstAddrTemplet->iSegNum; i++)
	{
		iBits += a_pstAddrTemplet->astSeg[i].bBitNum;
	}
	if ((0 >= iBits)|| (iBits > TBUS_ADDR_TEMPLET_MAX_BITS_NUM))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid address templet %s, total bits num must be in the scope[1,%d]", 
			a_pszAddTemplet, TBUS_ADDR_TEMPLET_MAX_BITS_NUM);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PARSE_ADDR_TEMPLET);
	}

	return iRet ;
}

int tbus_add_addrtemplet_segment(OUT LPTBUSADDRTEMPLET a_pstAddrTemplet, IN char *a_pszSeg)
{
	char *pchDesc;
	LPTBUSHANDLEEG pstSeg; 
	int iBitsNum;
	int i;

	assert(NULL != a_pstAddrTemplet);
	assert(NULL != a_pszSeg);

	if (a_pstAddrTemplet->iSegNum >= (int)(sizeof(a_pstAddrTemplet->astSeg)/sizeof(a_pstAddrTemplet->astSeg[0])))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"address segment num reach max segments<%d> limits, cannot add any  segment", 
			a_pstAddrTemplet->iSegNum);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PARSE_ADDR_TEMPLET);
	}

	pchDesc = strchr(a_pszSeg, TBUS_SPLIT_SEGMENT_DESCRITION_CHARACTER);
	if (NULL == pchDesc)
	{
		pchDesc = a_pszSeg;
	}else
	{
		pchDesc++;
	}

	iBitsNum = strtol(pchDesc, NULL, 10);
	if ((0 >= iBitsNum) || (iBitsNum > TBUS_ADDR_TEMPLET_MAX_BITS_NUM))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid address segment bits num<%s>, its value must be 1-%d", 
			pchDesc, TBUS_ADDR_TEMPLET_MAX_BITS_NUM);
		return TBUS_ERR_MAKE_ERROR(TBUS_ERROR_PARSE_ADDR_TEMPLET);
	}
	pstSeg = &a_pstAddrTemplet->astSeg[a_pstAddrTemplet->iSegNum];
	pstSeg->bBitNum = (unsigned char)iBitsNum;

	pstSeg->dwMaxVal = 1;
	for (i = 0; i < iBitsNum -1; i++)
	{
		pstSeg->dwMaxVal <<=1;
		pstSeg->dwMaxVal += 1;
	}
	

	pstSeg->bStartBit = 0;
	for (i = 0; i < a_pstAddrTemplet->iSegNum; i++)
	{
		pstSeg->bStartBit += a_pstAddrTemplet->astSeg[i].bBitNum;
	}

	
	a_pstAddrTemplet->iSegNum++;
	tbus_log(TLOG_PRIORITY_FATAL,"address segment[%d]: maxval:%u bitsnum:%d startbits:%d",
		a_pstAddrTemplet->iSegNum, pstSeg->dwMaxVal, pstSeg->bBitNum, pstSeg->bStartBit);

	return TBUS_SUCCESS;
}

void tbus_dump_addrtemplet(IN LPTBUSADDRTEMPLET a_pstAddrTemplet, IN FILE *a_fpDump)
{
	int i;
	assert(NULL != a_pstAddrTemplet);

	if (NULL == a_fpDump)
	{
		a_fpDump = stdout;
	}
	
	fprintf(a_fpDump, "addr segments num: %d  content:\n", a_pstAddrTemplet->iSegNum);
	for (i = a_pstAddrTemplet->iSegNum - 1; i >= 0; i--)
	{
		LPTBUSHANDLEEG pstSeg = &a_pstAddrTemplet->astSeg[i];
		fprintf(a_fpDump, "%d(%d/%d).", pstSeg->dwMaxVal, 
			(int)pstSeg->bBitNum, (int)pstSeg->bStartBit);
	}
	fprintf(a_fpDump,"\n");
}



/**根据地址模板设置将内部通信地址 转换成点分十进制表示的地址串
*@param[in] a_pstTemplet 通信地址模板结构的指针
*@param[in] a_iAddr	通信地址
*@return 成点分十进制表示的地址串
*@note 返回的成点分十进制表示的地址串保存在一个静态缓冲区中，后一次调用会覆盖前一次调用时获取的信息
*/
char *tbus_addr_nota_by_addrtemplet(IN LPTBUSADDRTEMPLET a_pstTemplet,IN TBUSADDR a_iAddr)
{
	static char szAddrBuff[TBUS_MAX_ADDR_STRING_LEN];
	char *pch;
	int iWrite;
	int iLeft;
	int i;

	assert(NULL != a_pstTemplet);

	if (0 >= a_pstTemplet->iSegNum)
	{
		return "";
	}

	a_iAddr = ntohl(a_iAddr);
	pch  = &szAddrBuff[0];
	iLeft = sizeof(szAddrBuff);
	for (i = a_pstTemplet->iSegNum - 1; i >= 0; i--)
	{
		LPTBUSHANDLEEG pstSeg = &a_pstTemplet->astSeg[i];
		iWrite = snprintf(pch, iLeft, "%d.", ((a_iAddr>>(int)pstSeg->bStartBit) & pstSeg->dwMaxVal));
		if ((0 > iWrite) || (iWrite >= iLeft))
		{
			break;
		}
		pch += iWrite;
		iLeft -= iWrite;
	}

	/*去掉最后的'.'*/
	pch--;
	*pch = '\0';

	return &szAddrBuff[0];
}

/**根据通信地址模板将点分十进制表示的地址串转换成内部表示通信地址
*@param[in] a_pstTemplet 通信地址模板结构的指针
*@param[in] a_pszAddr 点分十进制表示的地址串缓冲去指针
*@param[out] a_piAddr 保存通信地址的指针
*@retval	0 成功
*@retval <0	失败
*/
int tbus_addr_aton_by_addrtemplet(IN LPTBUSADDRTEMPLET a_pstTemplet, IN const char *a_pszAddr, OUT LPTBUSADDR a_piAddr)
{
	int iAdd;
	int iSegVal;
	char szAddrBuff[TBUS_MAX_ADDR_STRING_LEN];
	int i;
	char *pszSeg = NULL;
	int iRet = TBUS_SUCCESS;

	assert(NULL != a_pstTemplet);
	assert(NULL != a_pszAddr);
	assert(NULL != a_pszAddr);

	STRNCPY(szAddrBuff, a_pszAddr, sizeof(szAddrBuff));
	tbus_trim_str(szAddrBuff);
	tbus_log(TLOG_PRIORITY_TRACE,"addr string:%s", szAddrBuff);

	if (strcasecmp(szAddrBuff,"0") == 0)
	{
		*a_piAddr = 0; /*"0" 表示0.0.0.0..这个特殊地址*/
		return 0;
	}

	pszSeg = &szAddrBuff[0];
	iAdd =0 ;
	for (i = a_pstTemplet->iSegNum - 1; i >= 0; i-- )
	{
		LPTBUSHANDLEEG pstSeg = &a_pstTemplet->astSeg[i];
		char *pchDot = strchr(pszSeg, TBUS_SPLIT_SEGMENT_CHARACTER);
		if (NULL != pchDot)
		{
			*pchDot = '\0';
		}
		if ('\0' == *pszSeg)
		{
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
			tbus_log(TLOG_PRIORITY_ERROR,"invalid addr string<%s> do not match addr templet, for addr segment num is %d",
				szAddrBuff, a_pstTemplet->iSegNum);
			break;
		}
		iSegVal = strtol(pszSeg, NULL, 10);
		if ((0 > iSegVal) || (iSegVal > (int)pstSeg->dwMaxVal))
		{
			iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
			tbus_log(TLOG_PRIORITY_ERROR," %dth segment value scope is 0-%u, but get value is %d do not match addr templet, for addr segment num is %d",
				pstSeg->dwMaxVal, a_pstTemplet->iSegNum);
			break;
		}
		
		iAdd += iSegVal<<pstSeg->bStartBit;
		if (NULL != pchDot)
		{
			pszSeg = pchDot + 1;
		}else
		{
			*pszSeg = '\0';
		}
	}/*for (i = a_pstTemplet->iSegNum - 1; i >= 0; i-- )*/

	/*如果模板都匹配完毕，地处串还有剩余*/
	if ((0 > i) &&
		(0 < strnlen(pszSeg, sizeof(szAddrBuff) - (pszSeg - &szAddrBuff[0]))))
	{
		tbus_log(TLOG_PRIORITY_ERROR,"invalid addr string<%s> do not match addr templet, for addr segment num is %d",
			szAddrBuff, a_pstTemplet->iSegNum);

		iRet = TBUS_ERR_MAKE_ERROR(TBUS_ERROR_INVALID_ADDR_STRING);
	}

	if (0 == iRet)
	{
		*a_piAddr = htonl(iAdd);
		tbus_log(TLOG_PRIORITY_DEBUG,"convert addr string <%s> to addr %x",
			a_pszAddr, iAdd);
	}
	

	return iRet;	
}
