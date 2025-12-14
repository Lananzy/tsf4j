#include <assert.h>
#include "../comm/ResConv.h"
#include "resprint.h"
#include "tdr/tdr.h"
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define snprintf _snprintf
#else
#define snprintf snprintf
#endif

#define RES_HEAD_META_NAME "ResHead"
extern unsigned char g_szMetalib_ResConv[];

static int read_res_head(FILE *fp, RESHEAD *pstHead);

int res_print(const char *pszBinFile, const char *pszXMlFile, const char *pszMetalibFile, const char *pszMeta,
			  const char *pszMetaSetName, const char *pszMetaCountName)
{
	int iRet =0;
	LPTDRMETALIB pstLib = NULL;
	LPTDRMETA pstMeta;
	FILE *fpFile = NULL;
	FILE *fpXML = NULL;
	RESHEAD stHead;
	TDRDATA stData;
	char *pszDataBuf = NULL;
	char szRootName[TDR_NAME_LEN*2];


	assert(NULL != pszBinFile);
	assert(NULL != pszMetalibFile);
	assert(NULL != pszMeta);
	assert(NULL != pszXMlFile);


	iRet = tdr_load_metalib(&pstLib, pszMetalibFile);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("从二进制元数据文件加载元数据库失败, 原因是: %s\n", tdr_error_string(iRet));
	}
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		pstMeta = tdr_get_meta_by_name(pstLib, pszMeta);
		if (NULL == pstMeta)
		{
			printf("二进制元数据库中找不到名字为%s的元数据\n", pszMeta);
			iRet = -1;
		}
	}

	/*打开文件*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		fpFile = fopen(pszBinFile, "rb");
		if (NULL == fpFile)
		{
			printf("打开资源文件%s失败\n", pszBinFile);
			iRet = -2;
		}
	}
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		fpXML = fopen(pszXMlFile, "w");
		if (NULL == fpXML)
		{
			printf("打开资源文件%s失败\n", pszBinFile);
			iRet = -3;
		}
	}

	
	/*写xml头部*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if ((NULL != pszMetaSetName) && (*pszMetaSetName != '\0'))
		{
			snprintf(szRootName, sizeof(szRootName), "%s", pszMetaSetName);
		}else
		{
			snprintf(szRootName, sizeof(szRootName), "Res_%s", tdr_get_meta_name(pstMeta));
		}
		fprintf(fpXML, "<?xml version=\"1.0\" encoding=\"GBK\" standalone=\"yes\" ?>\n");
		fprintf(fpXML, "<%s>\n", szRootName);
		iRet = read_res_head(fpFile, &stHead);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("从资源文件中读取头部信息失败,ret :%d\n", iRet);
		}
	}
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if ((NULL != pszMetaCountName) && (*pszMetaCountName != '\0'))
		{
			fprintf(fpXML, "<%s>%d</%s>\n", pszMetaCountName, stHead.iCount, pszMetaCountName);
		}else
		{
			LPTDRMETA pstMetaHead = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_ResConv, RES_HEAD_META_NAME);
			if ((NULL != pstMetaHead) && (tdr_get_meta_size(pstMetaHead) == (int)sizeof(stHead)))
			{
				stData.iBuff = (int)sizeof(stHead);
				stData.pszBuff = (char *)&stHead;
				tdr_output_fp(pstMetaHead, fpXML, &stData, tdr_get_meta_current_version(pstMetaHead), TDR_IO_OLD_XML_VERSION);
			}
		}		
	}

	/*分配内存*/
	if (!TDR_ERR_IS_ERROR(iRet))
	{
		pszDataBuf = malloc(stHead.iUnit);
		if (NULL == pszDataBuf)
		{
			printf("为读取资源信息分配内存失败\n");
			iRet = -4;
		}	
	}

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		int i;
		int iRead;
		
		for (i = 0; i < stHead.iCount; i++)
		{
			iRead = fread(pszDataBuf, 1, stHead.iUnit, fpFile);
			if (iRead != stHead.iUnit)
			{
				printf("读取资源信息失败\n");
				break;
			}

			stData.iBuff = stHead.iUnit;
			stData.pszBuff = pszDataBuf;
			iRet = tdr_output_fp(pstMeta, fpXML, &stData, 0, TDR_IO_OLD_XML_VERSION);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				printf("将资源信息以XML格式输出失败:%s\n", tdr_error_string(iRet));
				break;
			}
		}/*for (i = 0; i < stHead.iCount; i++)*/
	}/*if (!TDR_ERR_IS_ERROR(iRet))*/

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		fprintf(fpXML, "</%s>", szRootName);
	}
	
	tdr_free_lib(&pstLib);
	if (NULL != pszDataBuf)
	{
		free(pszDataBuf);
	}
	fclose(fpXML);
	fclose(fpFile);	
	
	return iRet;
}



int read_res_head(FILE *fp, RESHEAD *pstHead)
{
	int iRead;

	assert(NULL != fp);
	assert(NULL != pstHead);

	iRead = fread(pstHead, sizeof(RESHEAD), 1, fp);
	if (iRead != 1)
	{
		return -1;
	}

	if (pstHead->iMagic != RES_FILE_MAGIC)
	{
		return -2;
	}

	if (pstHead->iVersion != RES_TRANSLATE_VERSION)
	{
		return -3;
	}

	return 0;
}

