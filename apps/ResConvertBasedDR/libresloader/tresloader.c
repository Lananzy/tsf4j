#include "tresloader.h"
#include "tdr/tdr.h"
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include "../comm/ResConv.h"
#include <stdlib.h>

#define RES_HEAD_META_NAME "ResHead"

extern unsigned char g_szMetalib_ResConv[];

static int rl_check_head(LPRESHEAD pstHead, int iData);
//static void rl_set_head(LPRESHEAD pstHead, int iCount, int iUnit);
static int rl_load_file(const char* pszPath, char** ppszBuff, int* piSize);

char g_szResLoaderResDir[RL_MAX_PATH_LEN];
LPTDRMETALIB g_pstResLoaderMetaLib;
char g_cResLoaderLoadMode = RL_LOADMODE_XMLV1;

#ifdef WIN32
#define  snprintf _snprintf
#endif


int rl_comp_func(const void* pv1, const void* pv2)
{
	return (*(int*)pv1) - (*(int*)pv2);
}

int rl_comp_func_ll(const void* pv1, const void* pv2)
{
	tdr_longlong diff = (*(tdr_longlong *)pv1) - (*(tdr_longlong *)pv2);

	if (diff > 0) return 1;
	else if (diff < 0) return -1;
	else return 0;
}

int rl_comp_func_n(const void* pv1, const void* pv2)
{
	return (*(short*)pv1) - (*(short*)pv2);
}


int rl_check_head(LPRESHEAD pstHead, int iData)
{
	assert( pstHead );

	if( RES_FILE_MAGIC!=pstHead->iMagic )
		return -1;

	if( RES_TRANSLATE_VERSION!=pstHead->iVersion )
		return -1;

	if( pstHead->iUnit<=0 )
		return -1;

	if( pstHead->iCount<0 )
		return -1;

	if( (int)(sizeof(RESHEAD) + pstHead->iUnit*pstHead->iCount) != iData )
		return -1;

	return 0;
}

/*
void rl_set_head(LPRESHEAD pstHead, int iCount, int iUnit)
{
	assert( pstHead );

	pstHead->iMagic	=	RES_FILE_MAGIC;
	pstHead->iVersion=	RES_TRANSLATE_VERSION;
	pstHead->iCount =	iCount;
	pstHead->iUnit	=	iUnit;
}
*/

int rl_sload(const char* pszPath, char* pszBuff, int iBuff, int iUnit)
{
	LPRESHEAD pstHead;
	char* pszData;
	char* pszSrc;
	char* pszDst;
	int iData;
	int iCount;
	int iSrcUnit;
	int i;
	int iRet = 0;

	iRet = rl_load_file(pszPath, &pszData, &iData);
	if (iRet<0 || !pszData )
		return -1;	/* can not read. */

	pstHead	=	(LPRESHEAD)pszData;
	if (0 == iUnit)
	{
		iUnit = pstHead->iUnit;
	}
	
	if( rl_check_head(pstHead, iData)<0 || 
	    pstHead->iUnit>iUnit || pstHead->iCount*iUnit>iBuff )
	{
		free(pszData);
		return -1;
	}
	
	iCount	=	pstHead->iCount;
	iSrcUnit=	pstHead->iUnit;
	pszSrc	=	pszData + sizeof(RESHEAD);
	pszDst	=	pszBuff;

	for(i=0; i<iCount; i++)
	{
		memcpy(pszDst, pszSrc, iSrcUnit);
		
		pszDst	+=	iUnit;
		pszSrc	+=	iSrcUnit;
	}

	free( pszData );

	return iCount;
}

int rl_xload_with_head(IN char* pszBuff, IN int iBuff, IN int iUnit, IN const char* pszXMLFilePath, IN int iIOVersion, IN LPTDRMETALIB pstMetalib, IN const char *pszMeta)
{
	LPTDRMETA pstMeta = NULL;
	int iSrcCount;
	int iSrcUnit;
	TDRDATA stHost;
	char *pszSrc;
	char *pszDst;
	int i;
	RESHEAD stHead;
	int iData;
	int iRet = 0;

	assert(NULL != pszBuff);
	assert(NULL != pszXMLFilePath);
	assert(NULL != pstMetalib);
	assert(NULL != pszMeta);

	if ((TDR_IO_OLD_XML_VERSION != iIOVersion) && (TDR_IO_NEW_XML_VERSION != iIOVersion))
	{
		iIOVersion = TDR_IO_NEW_XML_VERSION;
	}

	/*读取资源头部*/
	pstMeta = tdr_get_meta_by_name((LPTDRMETALIB)g_szMetalib_ResConv, RES_HEAD_META_NAME);
	if (NULL == pszMeta)
	{
		return -1;
	}
	if (tdr_get_meta_size(pstMeta) != (int)sizeof(stHead))
	{
		return -2; /*结构存储空间不一致*/
	}

	stHost.pszBuff = (char*)&stHead;
	stHost.iBuff = sizeof(stHead);
	iRet = tdr_input_file(pstMeta, &stHost, pszXMLFilePath, 0, iIOVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		return -3;
	}
	
	/*读取数据，先获得资源结构体的元数据描述*/
	pstMeta = tdr_get_meta_by_name(pstMetalib, pszMeta);
	if (NULL == pstMeta)
	{
		return -5;
	}
	iSrcUnit = tdr_get_meta_size(pstMeta);
	if (0 == iUnit)
	{
		iUnit = iSrcUnit;
	}

	/*校验资源头部和缓冲区*/
	iData	=	stHead.iCount * stHead.iUnit + sizeof(stHead);
	if( rl_check_head(&stHead, iData)<0 || (iSrcUnit != stHead.iUnit) ||
	    (stHead.iUnit>iUnit) || (stHead.iCount*iUnit > iBuff) )
	{
		return -4;
	}





	stHost.iBuff = stHead.iCount * stHead.iUnit ;
	stHost.pszBuff = (char*)calloc(1, stHost.iBuff);
	if (!stHost.pszBuff)
	{
		return -7;
	}
	iRet = tdr_input_file(pstMeta, &stHost, pszXMLFilePath, 0, iIOVersion);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		free(stHost.pszBuff);
		return -7;
	}
	
	iSrcCount = stHost.iBuff / iSrcUnit;
	pszSrc = stHost.pszBuff;
	pszDst = pszBuff;
 	for (i = 0; i < iSrcCount; i++)
	{
		memcpy(pszDst, pszSrc, iSrcUnit);
		
		pszDst += iUnit;
		pszSrc += iSrcUnit;
	}

	free(stHost.pszBuff);


	return iSrcCount;	
}

int rl_cload(void** ppvBuff, int *piBuff, int *piUnit, const char* pszPath)
{
	RESHEAD stHead;
	char* pszData;
	FILE *fpFile;
	int iData;
	int iRet = 0;

	assert(NULL != pszPath);
	assert(NULL != piBuff);
	assert(NULL != piUnit);

	fpFile	= fopen(pszPath, "rb");
	if( NULL == fpFile )
		return -1;

	if( 1 != fread((char*)&stHead, sizeof(stHead), 1, fpFile))
	{
		fclose(fpFile);
		return -1;
	}

	iData	=	stHead.iCount * stHead.iUnit + sizeof(stHead);
	if( rl_check_head(&stHead, iData)<0 )
	{
		fclose(fpFile);
		return -2;
	}

	if( piBuff )
		*piBuff	=	stHead.iCount * stHead.iUnit;

	if( piUnit )
		*piUnit	=	stHead.iUnit;

	pszData	=	(char*) malloc(stHead.iCount*stHead.iUnit);
	if( NULL==pszData )
	{
		fclose(fpFile);
		return -1;
	}

	iData	= fread(pszData, 1, stHead.iCount*stHead.iUnit, fpFile);
	fclose(fpFile);
	if( iData!=stHead.iCount*stHead.iUnit )
	{
		free(pszData);
		iRet =  -1;
	}
	else
	{
		*ppvBuff	=	pszData;
		iRet =  0;
	}

	return iRet;
}

char* rl_find(char* pszBuff, int iCount, int iUnit, int iKey)
{
	if (iCount < 0) return NULL;
	return (char*)bsearch(&iKey, pszBuff, iCount, iUnit, rl_comp_func);
}

char* rl_find_ll(char* pszBuff, int iCount, int iUnit, tdr_longlong llKey)
{
	if (iCount < 0) return NULL;
	return (char*)bsearch(&llKey, pszBuff, iCount, iUnit, rl_comp_func_ll);
}

char* rl_find_n(char* pszBuff, int iCount, int iUnit, short nKey)
{
	if (iCount < 0) return NULL;
	return (char*)bsearch(&nKey, pszBuff, iCount, iUnit, rl_comp_func_n);
}

int rl_load_file(const char* pszPath, char** ppszBuff, int* piSize)
{
	FILE *fp;

	long lSize;
	int iRead;
	int iRet = 0;
	char* pszData;
	
	assert( pszPath && piSize );
	assert(ppszBuff);

	fp = fopen(pszPath, "rb");
	if (NULL == fp)
	{
		*ppszBuff	=	NULL;
		*piSize		=	0;
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	if (0 >= lSize)
	{
		*ppszBuff	=	NULL;
		*piSize		=	0;
		fclose(fp);
		return -2;
	}
	rewind(fp);

	*piSize	= (int)lSize;
	pszData	=	(char*) calloc(1, (size_t)(lSize + 1));
	if( NULL==pszData )
	{
		*ppszBuff	=	NULL;
		*piSize		=	0;
		fclose(fp);
		return -3;
	}
	
	iRead = fread(pszData, 1, lSize, fp);
	if( iRead== (int)lSize )
	{
		*ppszBuff	= pszData;
		pszData[lSize]	=	'\0';
	}else
	{
		free(pszData);
		iRet =	-4;
	}

	fclose(fp);

	return iRet;
}

int rl_init(const char *pszDir, const char *pszTDRFile, char cLoadMode)
{

	char szTDRPath[1024];

	if (!pszDir || !pszTDRFile ||
		strlen(pszDir) >= RL_MAX_PATH_LEN-1 || strlen(pszDir)+strlen(pszTDRFile) >= 1024-1)
		return -1;

	if (cLoadMode == RL_LOADMODE_BIN) 
	{
		g_cResLoaderLoadMode = RL_LOADMODE_BIN;
	}else if (cLoadMode == RL_LOADMODE_XMLV1)
	{
		g_cResLoaderLoadMode = RL_LOADMODE_XMLV1;
	}else if (cLoadMode == RL_LOADMODE_XMLV2) 
	{
		g_cResLoaderLoadMode = RL_LOADMODE_XMLV2;
	}else 
	{
		g_cResLoaderLoadMode = RL_LOADMODE_XMLV1;
	}

	memset(g_szResLoaderResDir, 0, sizeof(g_szResLoaderResDir));
	memset(szTDRPath, 0, sizeof(szTDRPath));

	strncpy(g_szResLoaderResDir, pszDir, sizeof(g_szResLoaderResDir) - 1);

	snprintf(szTDRPath, sizeof(szTDRPath), "%s/%s", g_szResLoaderResDir, pszTDRFile);
	if (0 > tdr_load_metalib(&g_pstResLoaderMetaLib, szTDRPath))
	{
		printf("load metalib %s fail.\n", szTDRPath);
		return -1;
	}

	return 0;
}

int rl_xload(const char* pszResName, const char *pszMetaName, char *pszBuff, int iBuff, int iUnit)
{
	char szPath[1024];
	LPTDRMETA pstMeta = NULL;
	int iIndex = -1;
	LPTDRMETAENTRY pstEntry = NULL;
	int iSrcCount;
	int iSrcUnit;
	TDRDATA stHost;
	char *pszSrc;
	char *pszDst;
	int i;

	if (!pszResName || !pszMetaName || !pszBuff 
		|| strlen(pszResName) >= RL_MAX_PATH_LEN-1 
		|| strlen(pszMetaName) >= RL_MAX_PATH_LEN-1)
		return -1;

	if (g_cResLoaderLoadMode == RL_LOADMODE_BIN)
	{
		snprintf(szPath, sizeof(szPath), "%s/%s.bin", g_szResLoaderResDir, pszResName);
		return rl_sload(szPath, pszBuff, iBuff, iUnit);
	}

	/*读取xml文件格式的资源文件*/
	snprintf(szPath, sizeof(szPath), "%s/%s.xml", g_szResLoaderResDir, pszResName);
	pstMeta = tdr_get_meta_by_name(g_pstResLoaderMetaLib, pszMetaName);
	if (!pstMeta)
	{
		printf("find meta %s fail.\n", pszMetaName);
		return -1;
	}

	if (tdr_get_entry_by_id(&iIndex, pstMeta, RES_ID_ARRAY) < 0 || iIndex < 0)
	{
		printf("meta %s is invalid resource format.\n", pszMetaName);
		return -1;
	}

	pstEntry = tdr_get_entry_by_index(pstMeta, iIndex);
	if (!pstEntry)
	{
		printf("entry %s not found.\n", pszMetaName);
		return -1;
	}

	iSrcCount = -1;
	iSrcUnit = tdr_get_entry_unitsize(pstEntry);

	stHost.pszBuff = (char*)&iSrcCount;
	stHost.iBuff = sizeof(iSrcCount);

	tdr_input_file(pstMeta, &stHost, szPath, 0, (g_cResLoaderLoadMode==RL_LOADMODE_XMLV1?1:0));

	if (stHost.iBuff < (int)sizeof(iSrcCount) || iSrcCount <= 0)
	{
		printf("resource %s contain nothing.\n", pszResName);
		return -1;
	}

	if(iSrcUnit > iUnit || iSrcCount*iUnit>iBuff)
	{
		printf("resource buffer for %s not enough.\n", pszResName);
		return -1;
	}

	stHost.iBuff = sizeof(int) + iSrcCount * iSrcUnit;
	stHost.pszBuff = (char*)calloc(1, stHost.iBuff);

	if (!stHost.pszBuff)
	{
		printf("alloc memory for resource %s fail.\n", pszResName);
		free(stHost.pszBuff);
		return -1;
	}

	if (0 > tdr_input_file(pstMeta, &stHost, szPath, 0, (g_cResLoaderLoadMode==RL_LOADMODE_XMLV1?1:0)))
	{
		printf("load resource %s fail.\n", pszResName);
		free(stHost.pszBuff);
		return -1;
	}

	pszSrc = stHost.pszBuff + sizeof(iSrcCount);
	pszDst = pszBuff;

	for (i = 0; i < iSrcCount; i++)
	{
		memcpy(pszDst, pszSrc, iSrcUnit);

		pszDst += iUnit;
		pszSrc += iSrcUnit;
	}

	free(stHost.pszBuff);
	stHost.pszBuff = NULL;

	return iSrcCount;

}


