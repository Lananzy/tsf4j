#include "stdafx.h"
#include <assert.h>
#include "ResSorter.h"
#include "../comm/error_report.h"
#include "ResBinFile.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_metalib_kernel_i.h"

CResSorter::CResSorter(void)
{

}
CResSorter::~CResSorter(void)
{

}

int CResSorter::Sort(IN LPTDRMETALIB pstMetalib, IN const CString &szMetaName, IN const CString &szBinFile, SORTMETHOD enSort)
{
	CResBinFile stBinFile;
	int iRet = 0;
	LPTDRMETA pstMeta = NULL;
	TDRDATA stData = {0,NULL};
	unsigned char *pszDatabuf = NULL;

	assert(NULL != pstMetalib);

	if (enSort ==  SORT_METHOD_NO)
	{
		return 0;
	}

	if( !stBinFile.Open(szBinFile, CFile::modeReadWrite|CFile::typeBinary))
	{
		WLogInfo(LOG_LEVEL_SEVERE, _T("无法打开资源文件 %s 读取信息\r\n"), szBinFile);
		return -1;
	}

	pstMeta = tdr_get_meta_by_name(pstMetalib, (LPCTSTR)szMetaName);
	if (NULL == pstMeta)
	{
		WLogInfo(LOG_LEVEL_ERROR, "Error: 元数据描述数据库没有定义名字为%s的资源数据\r\n", szMetaName);
		iRet = -2;
	}


	RESHEAD stHead;	
	if (0 == iRet)
	{
		iRet = stBinFile.ReadResHead(stHead);
		if (0 != iRet)
		{
			WLogInfo(LOG_LEVEL_ERROR, _T("读取资源文件的头部信息失败<ret:%d>,请确定%s为有效的二进制资源文件."), szBinFile);
		}else if (pstMeta->iHUnitSize != stHead.iUnit)
		{
			WLogInfo(LOG_LEVEL_ERROR, _T("资源文件的单个资源信息的大小<%d字节>与元数据描述中定义的大小<%d字节>不同,请确定资源文件<%s>和元数据描述文件版本一致."),
				stHead.iUnit, pstMeta->iHUnitSize, szBinFile);
			iRet = -3;
		}
	}

	stData.iBuff = stHead.iUnit * stHead.iCount;
	if (0 == iRet)
	{
		try
		{
			stData.pszBuff = new  char[stData.iBuff];
		}
		catch (CException* e)
		{
			WLogInfo(LOG_LEVEL_SEVERE,"Error: 分配内存空间[%d]失败\r\n", stData.iBuff);
			iRet = -4;
		}
	}	

	/*将资源数据输出来*/
	if (0 == iRet)
	{
		iRet = stBinFile.ReadBinData((unsigned char *)stData.pszBuff, stData.iBuff);
		if (0 != iRet)
		{
			WLogInfo(LOG_LEVEL_ERROR, _T("从二进制资源文件<%s>中读取资源信息失败."), szBinFile);
		}		
	}/*if (0 == iRet)*/

	if ((0 == iRet) && (stHead.iCount > 1))
	{
		switch(enSort)
		{
		case SORT_METHOD_ASC:
			iRet = tdr_sort_metas(pstMeta, &stData, stHead.iCount, TDR_SORTMETHOD_ASC_SORT, TDR_MAX_VERSION);
			break;
		case SORT_METHOD_DESC:
			iRet = tdr_sort_metas(pstMeta, &stData, stHead.iCount, TDR_SORTMETHOD_DSC_SORT, TDR_MAX_VERSION);
			break;
		default:
			break;
		}	
		if (TDR_ERR_IS_ERROR(iRet))
		{
			WLogInfo(LOG_LEVEL_ERROR, _T("对二进制资源文件<%s>中的资源信息排序失败: %s."), szBinFile, tdr_error_string(iRet));
		}
	}/*if (0 == iRet)*/

	if (0 == iRet)
	{
		stBinFile.SeekToBegin();
		iRet = stBinFile.WriteResHead(stHead);
		if (0 != iRet)
		{
			WLogInfo(LOG_LEVEL_ERROR, _T("将资源头部写到二进制资源文件<%s>中失败."), szBinFile);
		}else
		{
			iRet = stBinFile.AppendWriteBinData((const unsigned char *)stData.pszBuff, stData.iBuff);
			if (0 != iRet)
			{
				WLogInfo(LOG_LEVEL_ERROR, _T("将资源写到二进制资源文件<%s>中失败."), szBinFile);
			}
		}/*if (0 != iRet)*/		
	}/*if (0 != iRet)*/

	/*释放资源*/	
	stBinFile.Close();
	if (NULL != stData.pszBuff)
	{
		delete stData.pszBuff;
	}

	return iRet;
}
