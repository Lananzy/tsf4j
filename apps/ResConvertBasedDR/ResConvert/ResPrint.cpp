#include "stdafx.h"
#include <assert.h>
#include <stdio.h>
#include "ResPrint.h"
#include "ResBinFile.h"
#include "../comm/error_report.h"
#include "tdr/tdr_define_i.h"
#include "tdr/tdr_metalib_kernel_i.h"


extern "C" unsigned char g_szMetalib_ResConv[];


CResPrint::CResPrint(void)
{
}

CResPrint::~CResPrint(void)
{
}

int CResPrint::Print(IN LPTDRMETALIB pstMetalib, IN const CString &szMetaName, IN const CString &szBinFile,
					 IN const CString &szRecordSetName, IN const CString &szRecordCountName)
{
	CString szXmlFile;
	FILE *fpXml = NULL;
	CString	szLine;
	CResBinFile stBinFile;
	int iRet = 0;
	LPTDRMETA pstMeta;
	LPTDRMETA pstMetaHead;
	TDRDATA stData;
	CString szRootName;
	int iPos;

	assert(NULL != pstMetalib);

	if( !stBinFile.Open(szBinFile, CFile::modeRead))
	{
		WLogInfo(LOG_LEVEL_ERROR, _T("无法打开资源文件 %s 读取信息\r\n"), szBinFile);
		iRet = -1;
	}

	szXmlFile = szBinFile;
	iPos = szXmlFile.Find('.', 0);
	if (iPos > 0)
	{
		szXmlFile = szXmlFile.Left(iPos);
	}
	szXmlFile +=  ".xml";
	
	
	fpXml = fopen(szXmlFile, "w");
	if(NULL == fpXml)
	{
		WLogInfo(LOG_LEVEL_ERROR, _T("无法打开文件 %s 以XML格式输出二进制资源文件！\r\n"), szXmlFile);
		iRet = -2;
	}

	if (0 == iRet)
	{
		pstMeta = tdr_get_meta_by_name(pstMetalib, (LPCTSTR)szMetaName);
		if (NULL == pstMeta)
		{
			WLogInfo(LOG_LEVEL_ERROR, "Error: 元数据描述数据库没有定义名字为%s的资源数据\r\n", szMetaName);
			iRet = -4;
		}
	}

	if (!szRecordSetName.IsEmpty())
	{
		szRootName = szRecordSetName;
	}else
	{
		szRootName = "Res_";
		szRootName += pstMeta->szName;
	}

	if (0 == iRet)
	{
		fprintf(fpXml, "<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n");
		fprintf(fpXml, "<%s>\n", szRootName);
	}
	

	RESHEAD stHead;
	if (0 == iRet)
	{
		iRet = stBinFile.ReadResHead(stHead);
		if (0 != iRet)
		{
			WLogInfo(LOG_LEVEL_ERROR, _T("读取资源文件的头部信息失败<ret:%d>,请确定%s为有效的二进制资源文件！\r\n"), iRet, szBinFile);
		}

		/*输出头部*/
		if (szRecordCountName.IsEmpty())
		{
			pstMetaHead = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_ResConv, RES_HEAD_META_NAME);
			if ((NULL != pstMetaHead) && (pstMetaHead->iHUnitSize == (int)sizeof(stHead)))
			{
				stData.iBuff = (int)sizeof(stHead);
				stData.pszBuff = (char *)&stHead;
				tdr_output_fp(pstMetaHead, fpXml, &stData, TDR_MAX_VERSION, TDR_IO_OLD_XML_VERSION);
			}
		}else
		{
			fprintf(fpXml, "<%s>%d</%s>\n", szRecordCountName, stHead.iCount, szRecordCountName);
		}		
	}	


	unsigned char *pszDatabuf = NULL;
	if (0 == iRet)
	{
		try
		{
			pszDatabuf = new unsigned char[stHead.iUnit];
		}
		catch (CException* e)
		{
			WLogInfo(LOG_LEVEL_ERROR, "Error: 分配内存空间[%d]失败\r\n", stHead.iUnit);
			iRet = -3;
		}
	}	
	
	/*将资源数据输出来*/
	if (0 == iRet)
	{
		for (int i = 0; i < stHead.iCount; i++)
		{
			iRet = stBinFile.ReadBinData(pszDatabuf, stHead.iUnit);
			if (0 != iRet)
			{
				break;
			}
			stData.iBuff = stHead.iUnit;
			stData.pszBuff = (char *)pszDatabuf;
			iRet = tdr_output_fp(pstMeta, fpXml, &stData, TDR_MAX_VERSION, TDR_IO_OLD_XML_VERSION);
			if (0 != iRet)
			{
				break;
			}
		}/*for (int i = 0; i < stHead.iCount; i++)*/
	}/*if (0 == iRet)*/

	if (0 == iRet)
	{
		fprintf(fpXml, "</%s>", szRootName);
	}

	/*释放资源*/
	fclose(fpXml);
	stBinFile.Close();
	if (NULL != pszDatabuf)
	{
		delete pszDatabuf;
	}

	return iRet;
}	

