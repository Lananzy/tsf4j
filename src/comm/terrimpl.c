#include "pal/pal.h"
#include "comm/terr.h"
#include "comm/terrimpl.h"
#include "scew/scew.h"
#include "comm/tparser.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif /* WIN32 */

const char* TERRIMPL_XML_ERRLIB			= "errlib";
const char* TERRIMPL_XML_ERRSET			= "errset";
const char* TERRIMPL_XML_ERROR			= "error";

const char* TERRIMPL_XML_VERSION		= "version";
const char* TERRIMPL_XML_TIME_FORMAT	= "time-format";
const char* TERRIMPL_XML_OUTPUT_FORMAT	= "output-format";
const char* TERRIMPL_XML_USE_MSECS		= "use-msecs";

const char* TERRIMPL_XML_LANG			= "lang";
const char* TERRIMPL_XML_ID				= "id";
const char* TERRIMPL_XML_NAME			= "name";
const char* TERRIMPL_XML_DESC			= "desc";

TERRLIB* terrimpl_alloc(void)
{
	TERRLIB* pstLib;

	pstLib	=	(TERRLIB*) calloc( 1, sizeof(TERRLIB) );

	if( pstLib )
	{
		terrimpl_init(pstLib);
	}

	return pstLib;
}

void terrimpl_free(TERRLIB* pstLib)
{
	int i;
	TERRSET* pstSet; 

	assert( pstLib );

	for(i=0; i<TERRIMPL_MAX_SETS; i++)
	{
		pstSet	=	pstLib->sets + i;

		if( pstSet->pstErrors )
			free( pstSet->pstErrors );
	}

	if( pstLib->pstTree )
		scew_tree_free( pstLib->pstTree );

	free( pstLib );
}

void terrimpl_init(TERRLIB* pstLib)
{
	assert( pstLib );

	terrimpl_set_output_format(pstLib, TERRIMPL_DEF_OUTPUT_FORMAT);
	terrimpl_set_time_format(pstLib, TERRIMPL_DEF_TIME_FORMAT);
}

HERR terrimpl_set_output_format(TERRLIB* pstLib, const char* pszFormat)
{
	int iLen;

	assert( pstLib && pszFormat );

	iLen	= strlen( pszFormat );

	if( iLen >= TERRIMPL_MAX_FORMAT )
		return TERRIMPLE_BUFF;

	strcpy( pstLib->szOutput, pszFormat );

	return 0;
}

HERR terrimpl_set_time_format(TERRLIB* pstLib, const char* pszFormat)
{
	int iLen;

	assert( pstLib && pszFormat );

	iLen	= strlen( pszFormat );

	if( iLen >= TERRIMPL_MAX_FORMAT )
		return TERRIMPLE_BUFF;

	strcpy( pstLib->szTime, pszFormat );

	return 0;
}

int terrimpl_set_flags(TERRLIB* pstLib, int iFlags)
{
	int iOldFlags;

	assert( pstLib );

	iOldFlags	=	pstLib->iFlags;

	pstLib->iFlags	=	iFlags;

	return iOldFlags;
}

HERR terrimpl_get_output_format(TERRLIB* pstLib, char* pszFormat, int *piLen)
{
	int iStr;

	assert( pstLib && piLen );

	iStr	=	strlen(pstLib->szOutput);

	if( !pszFormat )
	{
		*piLen	=	iStr;
		return 0;
	}

	if( *piLen < iStr )
	{
		*piLen	=	iStr;
		return TERRIMPLE_BUFF;
	}

	*piLen	=	iStr;

	strcpy( pszFormat, pstLib->szOutput);

	return 0;
}

HERR terrimpl_get_time_format(TERRLIB* pstLib, char* pszFormat, int *piLen)
{
	int iStr;

	assert( pstLib && piLen );

	iStr	=	strlen(pstLib->szTime);

	if( !pszFormat )
	{
		*piLen	=	iStr;
		return 0;
	}

	if( *piLen < iStr )
	{
		*piLen	=	iStr;
		return TERRIMPLE_BUFF;
	}

	*piLen	=	iStr;

	strcpy( pszFormat, pstLib->szTime);

	return 0;
}

int terrimpl_get_flags(TERRLIB* pstLib)
{
	assert( pstLib );

	return pstLib->iFlags;
}

HERR terrimpl_load_error(TERRSET* pstErrSet, scew_element* pstError)
{
	scew_attribute* pstAttr;
	TERRDESC* pstErrDesc;
	const XML_Char* pszName;
	const XML_Char* pszValue;

	assert(pstErrSet && pstError && pstErrSet->iErrs<pstErrSet->iMaxErrs);

	pszName	=	scew_element_name( pstError );

	if( !pszName || 0!=strcmp(pszName, TERRIMPL_XML_ERROR ) )
		return 0;

	pstErrDesc	=	pstErrSet->pstErrors + pstErrSet->iErrs;

	pstAttr	=	scew_attribute_by_name(pstError, TERRIMPL_XML_ID);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	if( !pszValue ) 
		return TERRIMPLE_NEEDID;

	pstErrDesc->iId		=	atoi(pszValue);

	pstAttr	=	scew_attribute_by_name(pstError, TERRIMPL_XML_DESC);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	pstErrDesc->pszDesc	=	(char*)pszValue;

	pstErrSet->iErrs++;

	return 0;
}

int terrimpl_compare(const void* pv1, const void* pv2)
{
	const TERRDESC* pstEle1;
	const TERRDESC* pstEle2;

	pstEle1	=	(const TERRDESC*) pv1;
	pstEle2	=	(const TERRDESC*) pv2;

	return pstEle1->iId - pstEle2->iId;
}

HERR terrimpl_load_set(TERRLIB* pstLib, scew_element* pstSet, const char* pszLang)
{
	scew_element* pstError;
	scew_attribute* pstAttr;
	TERRSET* pstErrSet;
	int iErrors;
	int iId;
	const XML_Char* pszName;
	const XML_Char* pszValue;

	assert( pstLib && pstSet );

	pszName	=	scew_element_name( pstSet );

	if( !pszName || 0!=strcmp(pszName, TERRIMPL_XML_ERRSET) )
		return 0;

	pstAttr	=	scew_attribute_by_name(pstSet, TERRIMPL_XML_LANG);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	if( pszValue && pszLang && 0!=strcmp(pszValue, pszLang) )
		return 0;

	pstAttr	=	scew_attribute_by_name(pstSet, TERRIMPL_XML_ID);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	if( !pszValue )
		return TERRIMPLE_NEEDID;

	iId	=	atoi(pszValue);

	if( iId<0 && iId>=TERRIMPL_MAX_SETS )
		return TERRIMPLE_BADMOD;

	pstAttr	=	scew_attribute_by_name(pstSet, TERRIMPL_XML_NAME);

	pszName	=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	pstErrSet=	pstLib->sets + iId;

	if( pstErrSet->pstErrors )
		return TERRIMPLE_REPEAT;

	iErrors	=	(int) scew_element_count(pstSet);

	pstErrSet->pstErrors	=	(TERRDESC*) calloc(1, sizeof(TERRDESC)*iErrors);

	if( !pstErrSet->pstErrors )
		return TERRIMPLE_NOMEM;

	pstLib->iSets++;
	
	pstErrSet->pszName	=	(char*)pszName;
	pstErrSet->iSet		=	iId;
	pstErrSet->iMaxErrs	=	iErrors;
	pstErrSet->iErrs	=	0;

	pstError=	scew_element_next(  pstSet, NULL );

	while( pstError )
	{
		terrimpl_load_error( pstErrSet, pstError );

		pstError	=	scew_element_next( pstSet, pstError );
	}

	if( pstErrSet->iErrs>1 )
	{
		qsort(pstErrSet->pstErrors, pstErrSet->iErrs, sizeof(TERRDESC), terrimpl_compare);
	}

	return 0;
}

HERR terrimpl_load_tree(TERRLIB* pstLib, scew_tree* pstTree, const char* pszLang)
{
	scew_element* pstRoot;
	scew_element* pstSet;
	scew_attribute* pstAttr;
	const XML_Char* pszName;
	const XML_Char* pszValue;

	assert( pstLib && pstTree );

	pstLib->iSets	=	0;

	pstRoot	=	scew_tree_root( pstTree );

	if( !pstRoot )
		return 0;

	tparser_convert_element(pstRoot);

	pszName	=	scew_element_name( pstRoot );

	if( 0!=strcmp(pszName, TERRIMPL_XML_ERRLIB) )
		return TERRIMPLE_BADROOT;

	pstAttr	=	scew_attribute_by_name(pstRoot, TERRIMPL_XML_VERSION);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	if( pszValue && atoi(pszValue)>TERRIMPL_MAJOR )
		return TERRIMPLE_BADVER;

	pstAttr	=	scew_attribute_by_name(pstRoot, TERRIMPL_XML_OUTPUT_FORMAT);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	if( pszValue )
		terrimpl_set_output_format(pstLib, pszValue);

	pstAttr	=	scew_attribute_by_name(pstRoot, TERRIMPL_XML_TIME_FORMAT);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	if( pszValue )
		terrimpl_set_time_format(pstLib, pszValue);
	
	pstAttr	=	scew_attribute_by_name(pstRoot, TERRIMPL_XML_USE_MSECS);

	pszValue=	pstAttr ? scew_attribute_value( pstAttr ) : NULL;

	if( pszValue && atoi(pszValue) )
		terrimpl_set_flags(pstLib, TERRIMPL_FLAGS_USE_MSECS);

	pstSet	=	scew_element_next( pstRoot, NULL );

	while( pstSet )
	{
		terrimpl_load_set( pstLib, pstSet, pszLang );

		pstSet	=	scew_element_next( pstRoot, pstSet );
	}

	pstLib->iPid	=	(int)getpid();
	pstLib->fInited	=	1;

	return 0;
}

HERR terrimpl_load_buffer(TERRLIB* pstLib, const char* pszBuff, int iBuff, const char* pszLang)
{
	scew_parser* pstParser;
	scew_tree* pstTree;

	assert( pstLib && pszBuff );

	pstParser	=	scew_parser_create();

	if( NULL==pstParser )
		return TERRIMPLE_PARSER;

	tparser_set_default_encoding(pstParser);

	if( 0==scew_parser_load_buffer(pstParser, pszBuff, (unsigned int)iBuff) )
		return TERRIMPLE_LOADXML;

	pstTree	=	scew_parser_tree( pstParser );

	scew_parser_free(pstParser);

	return terrimpl_load_tree( pstLib, pstTree, pszLang);
}

HERR terrimpl_load_fp(TERRLIB* pstLib, FILE* fp, const char* pszLang)
{
	scew_parser* pstParser;
	scew_tree* pstTree;

	assert( pstLib && fp );

	pstParser	=	scew_parser_create();

	if( NULL==pstParser )
		return TERRIMPLE_PARSER;

	tparser_set_default_encoding(pstParser);

	if( 0==scew_parser_load_file_fp(pstParser, fp) )
		return TERRIMPLE_LOADXML;

	pstTree	=	scew_parser_tree( pstParser );

	scew_parser_free(pstParser);

	return terrimpl_load_tree( pstLib, pstTree, pszLang);
}

HERR terrimpl_load_file(TERRLIB* pstLib, const char* pszPath, const char* pszLang)
{
	scew_parser* pstParser;
	scew_tree* pstTree;

	assert( pstLib && pszPath );

	pstParser	=	scew_parser_create();

	if( NULL==pstParser )
		return TERRIMPLE_PARSER;

	tparser_set_default_encoding(pstParser);

	if( 0==scew_parser_load_file(pstParser, pszPath) )
		return TERRIMPLE_LOADXML;

	pstTree	=	scew_parser_tree( pstParser );

	scew_parser_free(pstParser);

	return terrimpl_load_tree( pstLib, pstTree, pszLang);
}

void terrimpl_unload(TERRLIB* pstLib)
{
	int i;
	TERRSET* pstSet;

	assert( pstLib );

	for(i=0; i<TERRIMPL_MAX_SETS; i++)
	{
		pstSet	=	pstLib->sets + i;

		if( pstSet->pstErrors )
		{
			free( pstSet->pstErrors );

			pstSet->pstErrors	=	NULL;
		}

		memset( pstSet, 0, sizeof(TERRSET) );
	}

	if( pstLib->pstTree )
	{
		scew_tree_free( pstLib->pstTree );

		pstLib->pstTree	=	NULL;
	}
}

HERR terrimpl_get_info(TERRLIB* pstLib, HERR hErr, TERRINFO* pstInfo)
{
	TERRDESC stDesc;
	int iSet;
	TERRSET* pstSet;
	TERRDESC* pstFind;

	assert( pstLib && pstInfo );

	pstInfo->pstLib	=	pstLib;
	pstInfo->pstSet	=	NULL;
	pstInfo->pstDesc=	NULL;

	stDesc.iId	=	TERR_GET_ERRNO( hErr );
	stDesc.pszDesc	=	NULL;

	iSet		=	TERR_GET_MOD( hErr );

	if( iSet<0 || iSet>TERRIMPL_MAX_SETS )
		return TERRIMPLE_BADMOD;

	pstSet		=	pstLib->sets + iSet;

	pstInfo->pstSet	=	pstSet;

	if( pstSet->iErrs<1 )
		return TERRIMPLE_SEARCH;

	pstFind	= (TERRDESC*) bsearch( &stDesc, pstSet->pstErrors, pstSet->iErrs, sizeof(TERRDESC), terrimpl_compare);

	if( !pstFind )
		return TERRIMPLE_NONEXIST;

	pstInfo->pstDesc=	pstFind;

	return 0;
}

const char* terrimpl_err2str(TERRLIB* pstLib, HERR hErr)
{
	TERRINFO stInfo;
	HERR hRet;

	assert( pstLib );

	hRet	=	terrimpl_get_info( pstLib, hErr, &stInfo );

	if( TERR_IS_ERROR(hRet) )
		return NULL;

	if( stInfo.pstDesc )
		return stInfo.pstDesc->pszDesc;
	else
		return NULL;
}

HERR terrimpl_translate(TERRLIB* pstLib, HERR hErr, char* pszBuff, int* piBuff)
{
	char szTime[TERRIMPL_MAX_FORMAT + TERRIMPL_MAX_NAME];
	char szDesc[TERRIMPL_MAX_NAME];
	char szModName[TERRIMPL_MAX_NAME];
	TERRINFO stInfo;
	struct tm stTm;
	time_t tNow;
	int iPid;
	int iMaxLen;
	int iLen;
	HERR hRet;
	char* pszTimeFormat;
	char* pszOutputFormat;
	char* pszMod;
	char* pszErr;

	assert( pszBuff && piBuff );

	pszBuff[0]	=	'\0';

	hRet	=	terrimpl_get_info(pstLib, hErr, &stInfo);

	if( stInfo.pstSet )
		pszMod	=	stInfo.pstSet->pszName;
	else
		pszMod	=	NULL;

	if( stInfo.pstDesc )
		pszErr	=	stInfo.pstDesc->pszDesc;
	else
		pszErr	=	NULL;

	if( !pszMod )
	{
		sprintf(szModName, "mod(#%d)", (int)TERR_GET_MOD(hErr) );
		
		pszMod	=	szModName;
	}

	if( !pszErr )
	{
		sprintf(szDesc, "errno(#%d)", (int)TERR_GET_ERRNO(hErr) );
		
		pszErr	=	szDesc;
	}

	iMaxLen	=	*piBuff;

	if( pstLib->fInited )
	{
		iPid		=	pstLib->iPid;
		pszTimeFormat	=	pstLib->szTime;
		pszOutputFormat	=	pstLib->szOutput;
	}
	else
	{
		iPid		=	(int) getpid();
		
		if( pstLib->iFlags & TERRIMPL_FLAGS_USE_MSECS )
			pszTimeFormat	=	TERRIMPL_MSECS_FORMAT;
		else
			pszTimeFormat	=	TERRIMPL_DEF_TIME_FORMAT;

		pszOutputFormat		=	TERRIMPL_DEF_OUTPUT_FORMAT;
	}

	time(&tNow);

#ifdef WIN32
	{
		struct tm *ptm;
		ptm	=	localtime(&tNow);
		memcpy(&stTm, ptm, sizeof(stTm));
	}
#else
	localtime_r(&tNow, &stTm);
#endif /* WIN32 */

	if( pstLib->iFlags & TERRIMPL_FLAGS_USE_MSECS )
	{
		snprintf(szTime, sizeof(szTime), pszTimeFormat, (int)tNow);
	}
	else
	{
		snprintf(szTime, sizeof(szTime), pszTimeFormat, stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday, stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
	}

	iLen	=	snprintf(pszBuff, iMaxLen, pszOutputFormat, iPid, szTime, pszMod, pszErr);

	if( -1==iLen )
		iLen	=	iMaxLen - 1;

	*piBuff	=	iLen;

	return 0;
}

void terrimpl_dump(FILE* fp, TERRLIB* pstLib)
{
	int i,j;
	TERRSET* pstSet;

	fprintf(fp, "_____________________________________________________\n");
	fprintf(fp, "Status:(inited=%d) ", pstLib->fInited);
	
	if( pstLib->fInited )
		fprintf(fp, "TERRSETs=%d ", pstLib->iSets);

	fprintf(fp, "\n");

	for(i=0; i<TERRIMPL_MAX_SETS; i++)
	{
		pstSet	=	pstLib->sets + i;

		if( !pstSet->pstErrors || pstSet->iErrs<=0 )
			continue;

		fprintf(fp, "TERRSET: Id=%d Name=%s Errs=%d\n", pstSet->iSet, pstSet->pszName, pstSet->iErrs);

		for(j=0; j<pstSet->iErrs; j++)
		{
			fprintf(fp, "\tError: Id=%d Desc=%s\n", pstSet->pstErrors[j].iId, pstSet->pstErrors[j].pszDesc);
		}
	}

	fprintf(fp, "_____________________________________________________\n");
}
