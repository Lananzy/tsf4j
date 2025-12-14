#include "ov_os.h"
#include "ov_dr.h"

/*****************************************************************

XML Sample:

<typelib >
	<type name="Point" id="10" >
		<item name="x" desc="xPos" type="int" count="1" />
		<item name="y" desc="yPos" type="int" count="1" />
	</type>

	<type name="Rect" id="11" >
		<item name="left" desc="Left" type="int" count="1" />
		<item name="top" desc="Top" type="int" count="1" />
		<item name="right" desc="Right" type="int" count="1" />
		<item name="bottom" desc="Bottom" type="int" count="1" />
	</type>

	<type name="Body" class="union" >
		<item name="pt" type="Point" count="1" select="0" />
		<item name="rc" type="Rect" count="1" select="1" />
	</type>

	<type name="Package" id="12" >
		<item name="type" desc="Type" type="int" count="1" />
		<item name="data" desc="Data" type="Body" select="type" />
	</type>
</typelib>

********************************************************************/

static DRTYPEINFO	s_types[] = 
{
/* typename, 		cname,		sprefix, mPrefix,	type,			size,	flags. */
{ DR_NAME_UNION,	"union",	"st",	 "",		DR_TYPE_UNION,		0,	0}, 
{ DR_NAME_STRUCT,	"struct",	"st",	 "",		DR_TYPE_STRUCT,		0,	0},
{ DR_NAME_CHAR,		"char",		"c",	 "sz",		DR_TYPE_CHAR,		1,	0},
{ DR_NAME_UCHAR,	"unsigned char","b",	 "sz",		DR_TYPE_UCHAR,		1,	0},
{ DR_NAME_SHORT,	"short",	"n",	 "",		DR_TYPE_SHORT,		2,	DR_FLAG_NETORDER},
{ DR_NAME_USHORT,	"unsigned short","w",	 "",		DR_TYPE_USHORT,		2,	DR_FLAG_NETORDER},
{ DR_NAME_INT,		"int",		"i",	 "",		DR_TYPE_INT,		4,	DR_FLAG_NETORDER},
{ DR_NAME_UINT,		"unsigned int",	"dw",	 "",		DR_TYPE_UINT,		4,	DR_FLAG_NETORDER},
{ DR_NAME_LONG,		"long",		"l",	 "",		DR_TYPE_INT,		4,	DR_FLAG_NETORDER},
{ DR_NAME_ULONG,	"unsigned long","ul",	 "",		DR_TYPE_UINT,		4,	DR_FLAG_NETORDER},
{ DR_NAME_LONGLONG,	"longlong",	"ll",	 "",		DR_TYPE_LONGLONG,	8,	DR_FLAG_NETORDER},
{ DR_NAME_ULONGLONG,	"ulonglong",	"ull",	 "",		DR_TYPE_ULONGLONG,	8,	DR_FLAG_NETORDER},
{ DR_NAME_DATE,		"int",		"t",	 "",		DR_TYPE_DATE,		4,	DR_FLAG_NETORDER},
{ DR_NAME_TIME,		"int",		"t",	 "",		DR_TYPE_TIME,		4,	DR_FLAG_NETORDER},
{ DR_NAME_DATETIME,	"int",		"t",	 "",		DR_TYPE_DATETIME,	4,	DR_FLAG_NETORDER},
{ DR_NAME_MONEY,	"int",		"m",	 "",		DR_TYPE_MONEY,	4,	DR_FLAG_NETORDER},
{ DR_NAME_FLOAT,	"float",	"f",	 "",		DR_TYPE_FLOAT,		4,	DR_FLAG_NETORDER},
{ DR_NAME_DOUBLE,	"double",	"d",	 "",		DR_TYPE_DOUBLE,		8,	DR_FLAG_NETORDER},
{ DR_NAME_STRING,	"char",		"sz",	 "sz",		DR_TYPE_STRING,		1,	0},
};

/* the function which has a suffix "_i" means that it is used internal. */

int dr_typename_to_idx(const char* pszName)
{
	int i;

	assert(pszName);

	for(i=0; i<sizeof(s_types)/sizeof(DRTYPEINFO); i++)
	{
		if( 0==stricmp(s_types[i].pszName, pszName) )
			return i;
	}

	return -1;
}

LPDRTYPEINFO dr_idx_to_typeinfo(int iIdx)
{
	assert( iIdx>=0 && iIdx<sizeof(s_types)/sizeof(DRTYPEINFO) );
	
	return s_types + iIdx;
}

int dr_typeid_to_idx(int iTypeID)
{
	int i;

	for(i=0; i<sizeof(s_types)/sizeof(DRTYPEINFO); i++)
	{
		if( s_types[i].iType==iTypeID )
			return i;
	}

	return -1;
}

void dr_meta_to_hpp(DRMETA* pstMeta, FILE* fp, const char* pszLeading)
{
	DRMETALIB* pstLib;
	const char* pszRealLeading;

	LPDRMETAENTRY pstEntry;
	LPDRMETA pstType;
	LPDRTYPEINFO pstInfo;
	LPDRMACRO pstMacro;

	const char* pszPrefix;
	const char* pszClass;

	char szType[DR_NAME_LEN];
	char szName[DR_NAME_LEN];
	char szMacro[DR_MACRO_LEN];
	char szArray[DR_MACRO_LEN + 4];
	char szVersion[DR_MACRO_LEN + 20];
	char szID[DR_MACRO_LEN + 20];
	
	char szBuff[DR_NAME_LEN + DR_MACRO_LEN + 8 + 16];

	int i;
	int n;

	assert( pstMeta && fp );

	if( NULL==pszLeading )
		pszRealLeading	=	"";
	else
		pszRealLeading	=	pszLeading;
	
	n	=	24;

	pstLib	=	DR_META_TO_LIB(pstMeta);
	
	pstMacro=	DR_GET_MACRO_TABLE(pstLib);
	
	if( DR_TYPE_UNION==pstMeta->iType )
		pszClass	=	DR_TAG_UNION;
	else
		pszClass	=	DR_TAG_STRUCT;

	fprintf(fp, "%s tag%s\n", pszClass, pstMeta->szName);

	fprintf(fp, "{\n");

	for(i=0; i<pstMeta->iEntries; i++)
	{
		pstEntry	=	pstMeta->entries + i;
		
		strcpy(szName, pstEntry->szName);
		
		if( -1!=pstEntry->idxCount )
		{
			strcpy(szMacro, pstMacro[pstEntry->idxCount].szMacro);
			strupr(szMacro);
			sprintf(szArray, "[%s]", szMacro);
		}
		else if( 1==pstEntry->iCount )
			sprintf(szArray, "");
		else if( 0==pstEntry->iCount )
			sprintf(szArray, "[1]");
		else
			sprintf(szArray, "[%d]", pstEntry->iCount);

		if( -1!=pstEntry->ptrMeta )
		{
			pstType	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			strcpy(szType, pstType->szName);
			strupr(szType);
		}
		else
		{
			pstInfo	=	s_types + pstEntry->idxType;

			STRNCPY(szType, pstInfo->pszCName, sizeof(szType));			
		}

		if( NULL==pszLeading )
			pszPrefix	=	"";
		else
		{
			if( 1==pstEntry->iCount )
				pszPrefix	=	s_types[pstEntry->idxType].pszSPrefix;
			else
				pszPrefix	=	s_types[pstEntry->idxType].pszMPrefix;

			if( strlen(pstEntry->szName)<=1 )
				pszPrefix	=	"";

			if( pszPrefix[0] )
				szName[0]	=	toupper(szName[0]);
			else
				szName[0]	=	tolower(szName[0]);
		}
		
		snprintf(szBuff, sizeof(szBuff)-1, "%s%s%s%s;", pszRealLeading, pszPrefix, szName, szArray);
		
		szBuff[sizeof(szBuff)-1]	=	'\0';
		
		if( pstEntry->iVersion>0 && pstEntry->iVersion!=pstLib->iVersion )
			snprintf(szVersion, sizeof(szVersion)-1, "Ver.%d", pstEntry->iVersion);
		else
			szVersion[0]	=	'\0';
			
		szVersion[sizeof(szVersion)-1]	=	'\0';

		if( -1!=pstEntry->idxID )
		{
			snprintf(szID, sizeof(szID)-1,"%s,", pstMacro[pstEntry->idxID].szMacro);
			szID[sizeof(szID)-1]	=	'\0';
		}
		else
			szID[0]	=	'\0';

		if( pstEntry->szDesc[0] || szVersion[0] || szID[0] )
			fprintf(fp, "\t%s %-*s \t/* %s %s %s */\n", szType, n, szBuff, szID, szVersion, pstEntry->szDesc);
		else
			fprintf(fp, "\t%s %-*s\n", szType, n, szBuff);
	}

	fprintf(fp, "};\n");
}

int dr_off_to_name_i(LPDRMETA pstMeta, int iEntry, int iOff, char* pszBuff, int iBuff)
{
	DRMETALIB* pstLib;
	DRMETAENTRY* pstEntry;

	int iCur;
	int i;

	const char* pszName;
	
	int iTotal;
	int iSize;
	int iError;
	int fFirst;
	
	assert( pstMeta && iOff>=0 && pszBuff );

	if( iBuff<=0 )
		return 0;

	pstLib	=	DR_META_TO_LIB(pstMeta);
		
	i	=	0;
	iCur	=	0;
	
	iTotal	=	iBuff;
	
	fFirst	=	1;
	
	while( iCur<iOff )
	{
		if( i>=pstMeta->iEntries )  
		{
			i	=	0;
			break;
		}
		
		pstEntry	=	pstMeta->entries + i;
		
		if( iCur+pstEntry->iOff<iOff &&
			iCur + pstEntry->iOff + pstEntry->iSize<=iOff )
		{
			i++;
			continue;
		}

		if( i==iEntry )
			pszName	=	DR_TAG_THIS;
		else
			pszName	=	pstEntry->szName;
		
		if( fFirst )
		{
			iSize	=	snprintf(pszBuff, iBuff, "%s", pszName);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			fFirst	=	0;
		}
		else
		{
			iSize	=	snprintf(pszBuff, iBuff, ".%s", pszName);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}
		
		iCur	+=	pstEntry->iOff;

		iEntry	=	-1;
		
		if( -1==pstEntry->ptrMeta )
		{
			pstMeta	=	NULL;
			break;
		}
		else
		{
			pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			i	=	0;
		}
	}

	if( iCur!=iOff )
		return (iTotal - iBuff);
	
	while( pstMeta )
	{
		pstEntry	=	pstMeta->entries;

		if( i==iEntry )
			pszName	=	DR_TAG_THIS;
		else
			pszName	=	pstEntry->szName;
		
		if( fFirst )
		{
			iSize	=	snprintf(pszBuff, iBuff, "%s", pszName);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			fFirst	=	0;
		}
		else
		{
			iSize	=	snprintf(pszBuff, iBuff, ".%s", pszName);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}

		iEntry	=	-1;
		
		if( -1==pstEntry->ptrMeta )
		{
			pstMeta	=	NULL;
			break;
		}
		else
			pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
	}
	
	return (iTotal - iBuff);
}

int dr_off_to_name_fp_i(LPDRMETA pstMeta, int iEntry, int iOff, FILE* fp)
{
	DRMETALIB* pstLib;
	DRMETAENTRY* pstEntry;

	int iCur;
	int i;

	const char* pszName;
	
	int fFirst;
	
	assert( pstMeta && iOff>=0 && fp );

	pstLib	=	DR_META_TO_LIB(pstMeta);
		
	i	=	0;
	iCur	=	0;
	
	fFirst	=	1;
	
	while( iCur<iOff )
	{
		if( i>=pstMeta->iEntries )  
		{
			i	=	0;
			break;
		}
		
		pstEntry	=	pstMeta->entries + i;
		
		if( iCur+pstEntry->iOff<iOff &&
			iCur + pstEntry->iOff + pstEntry->iSize<=iOff )
		{
			i++;
			continue;
		}

		if( i==iEntry )
			pszName	=	DR_TAG_THIS;
		else
			pszName	=	pstEntry->szName;
		
		if( fFirst )
		{
			fprintf(fp, "%s", pszName);
			fFirst	=	0;
		}
		else
			fprintf(fp, ".%s", pszName);
		
		iCur	+=	pstEntry->iOff;

		iEntry	=	-1;
		
		if( -1==pstEntry->ptrMeta )
		{
			pstMeta	=	NULL;
			break;
		}
		else
		{
			pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
			i	=	0;
		}
	}

	if( iCur!=iOff )
		return 0;
	
	while( pstMeta )
	{
		pstEntry	=	pstMeta->entries;

		if( i==iEntry )
			pszName	=	DR_TAG_THIS;
		else
			pszName	=	pstEntry->szName;
		
		if( fFirst )
		{
			fprintf(fp, "%s", pszName);
			fFirst	=	0;
		}
		else
			fprintf(fp, ".%s", pszName);

		iEntry	=	-1;
		
		if( -1==pstEntry->ptrMeta )
		{
			pstMeta	=	NULL;
			break;
		}
		else
			pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
	}
	
	return 0;
}

void dr_dump(FILE* fp, LPDRMETALIB pstLib)
{
	int i;
	int j;

	const char* pszSizeType;

	LPDRMETA pstMeta;
	LPDRMETAENTRY pstEntry;
	LPDRMACRO pstMacro;

	assert( fp && pstLib );

	fprintf(fp, "\nLIB(\"%s\"): ID=%d ver=%d Metas %d/%d Macros:%d/%d pad:%d:\n\n", pstLib->szName, 
		pstLib->iID, pstLib->iVersion, pstLib->iCurMetas, pstLib->iMaxMetas,
		pstLib->iCurMacros, pstLib->iMaxMacros, pstLib->iPadding);

	pstMacro	=	DR_GET_MACRO_TABLE(pstLib);

	for(i=0; i<pstLib->iCurMacros; i++)
	{
		fprintf(fp, "Macro Name=\"%s\" id=%d\n", pstMacro[i].szMacro, pstMacro[i].iID);
	}

	for(i=0; i<pstLib->iCurMetas; i++)
	{
		pstMeta	=	DR_IDX_TO_META(pstLib, i);

		DR_GET_SIZETYPE(pszSizeType, pstMeta->iSizeType);

		fprintf(fp, "Meta(size=\"%s\") Name=\"%s\" idxType=%d id=%d entries=%d data=%d.\n", 
			pszSizeType,
			pstMeta->szName, 
			pstMeta->idxType, 
			pstMeta->iID, 
			pstMeta->iEntries,
			pstMeta->iData);

		for(j=0; j<pstMeta->iEntries; j++)
		{
			pstEntry	=	pstMeta->entries + j;

			fprintf(fp, "\tEntry:\tName=\"%s\" ID=%d Desc=\"%s\" type=%d meta=%d off=%d count=%d\n",
				pstEntry->szName, pstEntry->iID,
				pstEntry->szDesc, pstEntry->idxType,
				pstEntry->ptrMeta, pstEntry->iOff,
				pstEntry->iCount );

			fprintf(fp, "\t\t unit=%d size=%d select=(%d, %d) refer=(%d, %d) target=(%d, %d)\n",
				pstEntry->iUnit, pstEntry->iSize,
				pstEntry->iSelectOff, pstEntry->iSelectUnit,
				pstEntry->iReferOff, pstEntry->iReferUnit,
				pstEntry->iTargetOff, pstEntry->iTargetUnit);
		}
	}

	fprintf(fp, "\n");
}

int dr_init_metalib(DRMETALIB* pstLib, DRLIBPARAM* pstParam)
{
	assert( pstLib && pstParam );
	
	if( pstParam->iSize<=0 || pstParam->iMetas<=0 || pstParam->iMacros<0 ||
		DR_CALC_MIN_SIZE(pstParam->iMetas, pstParam->iMacros) >= (size_t)pstParam->iSize )
		return -1;
		
	memset(pstLib, 0, pstParam->iSize);

	pstLib->wMagic		=	DR_MAGIC;
	pstLib->nBuild		=	DR_BUILD;

	pstLib->iPadding	=	pstParam->iPadding;
	pstLib->iID		=	pstParam->iID;

	pstLib->iSize		=	pstParam->iSize;
	pstLib->iVersion	=	pstParam->iVersion;

	pstLib->iMaxMetas	=	pstParam->iMetas;
	pstLib->iCurMetas	=	0;

	pstLib->iMaxID		=	0;
	
	memcpy(pstLib->szName, pstParam->szName, sizeof(pstLib->szName)-1);

	pstLib->iMaxMacros	=	pstParam->iMacros;
	pstLib->iCurMacros	=	0;

	pstLib->ptrMacro	=	DR_CALC_MACRO_PTR(pstParam);

	pstLib->ptrID		=	DR_CALC_ID_PTR(pstParam);
	pstLib->ptrName	=	DR_CALC_NAME_PTR(pstParam);
	pstLib->ptrMap		=	DR_CALC_MAP_PTR(pstParam);
	pstLib->ptrMeta	=	DR_CALC_META_PTR(pstParam);

	pstLib->ptrCur		=	pstLib->ptrMeta;

	return 0;
}

void dr_free_metalib(LPDRMETALIB pstLib)
{
	if( NULL==pstLib )
		return;
	
	free( pstLib );
}

int dr_size(LPDRMETALIB pstLib)
{
	return pstLib->iSize;
}

LPDRMETALIB dr_clone(void* pvDst, int iBuff, LPDRMETALIB pstSrc)
{
	if( iBuff<pstSrc->iSize )
		return NULL;

	memcpy(pvDst, pstSrc, pstSrc->iSize);

	return (LPDRMETALIB) pvDst;
}

int dr_load_bin_fp(LPDRMETALIB* ppstLib, FILE* fp)
{
	long lPos;
	size_t lSize;
	LPDRMETALIB pstLib;
	
	fseek(fp, 0, SEEK_END);
	
	lPos	=	ftell(fp);
	
	if( lPos<=0 )
	{
		fclose(fp);
		return -1;
	}
	
	rewind(fp);
			
	pstLib	=	(LPDRMETALIB) calloc(1, (size_t)lPos);
	
	if( NULL==pstLib )
		return -1;
		
	lSize	=	fread(pstLib, 1, lPos, fp);
	
	if( pstLib->iSize!=(int)lSize || DR_MAGIC!=pstLib->wMagic ||
	    DR_BUILD!=pstLib->nBuild )
	{
		free(pstLib);
		return -1;
	}
	else
	{
		*ppstLib	=	pstLib;
		return 0;
	}
}

int dr_load_bin_file(LPDRMETALIB* ppstLib, const char* pszFile)
{
	FILE* fp;
	int iRet;
	
	fp	=	fopen(pszFile, "rb");
	
	if( NULL==fp )
		return -1;
	
	iRet	=	dr_load_bin_fp(ppstLib, fp);
	
	fclose(fp);
	
	return iRet;
}

int dr_save_bin_fp(LPDRMETALIB pstLib, FILE* fp)
{
	int iSize;
	
	assert( pstLib && fp );
		
	iSize	=	fwrite(pstLib, 1, pstLib->iSize, fp);
	
	if( iSize!=pstLib->iSize )
		return -1;
	else
		return 0;
}

int dr_save_bin_file(LPDRMETALIB pstLib, const char* pszFile)
{
	FILE* fp;
	int iRet;
	
	assert( pstLib && pszFile );
	
	fp	=	fopen(pszFile, "wb" );
	
	if( NULL==fp )
		return -1;
	
	iRet	=	dr_save_bin_fp(pstLib, fp);
	
	fclose(fp);

	return iRet;	
}

int dr_save_metalib(LPDRMETALIB pstLib, char* pszXml, int *piXml)
{
	LPDRMETA pstMeta;
	LPDRMETAENTRY pstEntry;

	LPDRMETA pstType;
	LPDRMACRO pstMacro;

	const char* pszSizeType;

	char* pszBuff;
	int iBuff;
	int iSize;
	int iError;

	int i;
	int j;

	assert( pstLib && pszXml && piXml && *piXml>1 );

	pszBuff	=	pszXml;
	iBuff	=	*piXml - 1;
	iSize	=	0;
	iError	=	0;

	pszBuff[0]	=	0;
	pszBuff[iBuff]	=	'\0';

	iSize	=	snprintf(pszBuff, iBuff, "<%s", DR_TAG_TYPELIB);
	DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

	if( pstLib->szName[0] )
	{
		iSize	=	snprintf(pszBuff, iBuff, " %s=\"%s\"", DR_TAG_NAME, pstLib->szName);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
	}

	if( -1!=pstLib->iID )
	{
		iSize	=	snprintf(pszBuff, iBuff, " %s=\"%d\"", DR_TAG_ID, pstLib->iID);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
	}

	if( pstLib->iVersion>0 )
	{
		iSize	=	snprintf(pszBuff, iBuff, " %s=\"%d\"", DR_TAG_VERSION, pstLib->iVersion);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
	}

	if( pstLib->iPadding>0 )
	{
		iSize	=	snprintf(pszBuff, iBuff, " %s=\"%d\"", DR_TAG_PADDING, pstLib->iPadding);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
	}
	
	iSize	=	snprintf(pszBuff, iBuff, " >\n");
	DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

	if( pstLib->iCurMacros>0 )
	{
		iSize	=	snprintf(pszBuff, iBuff, "\t<%s>\n", DR_TAG_MACROS);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

		pstMacro	=	DR_GET_MACRO_TABLE(pstLib);

		for(i=0; i<pstLib->iCurMacros; i++)
		{
			iSize	=	snprintf(pszBuff, iBuff, "\t\t<%s", DR_TAG_MACRO);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%s\"", 
						DR_TAG_NAME, pstMacro[i].szMacro);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%d\"", 
						DR_TAG_ID, pstMacro[i].iID);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

			iSize	=	snprintf(pszBuff, iBuff, " />\n");
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}

		iSize	=	snprintf(pszBuff, iBuff, "\t</%s>\n", DR_TAG_MACROS);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
	}

	for(i=0; i<pstLib->iCurMetas; i++)
	{
		iSize	=	snprintf(pszBuff, iBuff, "\t<%s", DR_TAG_TYPE);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

		pstMeta	=	DR_IDX_TO_META(pstLib, i);

		if( pstMeta->szName[0] )
		{
			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%s\"", 
						DR_TAG_NAME, pstMeta->szName);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}

		if( pstMeta->iSizeType>0 && pstMeta->idxSize>=0 && 
		    pstMeta->idxSize<sizeof(s_types)/sizeof(DRTYPEINFO) )
			pszSizeType=	s_types[pstMeta->idxSize].pszName;
		else
			pszSizeType=	"";

		if(pszSizeType[0])
		{
			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%s\"", 
						DR_TAG_SIZETYPE, pszSizeType);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}

		if( -1!=pstMeta->idxID )
		{
			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%s\"", 
				DR_TAG_ID, pstMacro[pstMeta->idxID].szMacro);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}
		else if( -1!=pstMeta->iID )
		{
			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%d\"", 
				DR_TAG_ID, pstMeta->iID);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}

		if( DR_TYPE_UNION==pstMeta->iType )
		{
			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%s\"", 
						DR_TAG_CLASS, DR_TAG_UNION);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}
		else
		{
			iSize	=	snprintf(pszBuff, iBuff, " %s=\"%s\"", 
						DR_TAG_CLASS, DR_TAG_STRUCT);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}
		
		if( -1!=pstMeta->idxVersion )
		{
			iSize	=	snprintf(pszBuff, iBuff, 
			" %s=\"%s\"", DR_TAG_VERSION, pstMacro[pstMeta->idxVersion].szMacro);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}
		else if( pstMeta->iVersion>0 && pstMeta->iVersion!=pstLib->iVersion )
		{
			iSize	=	snprintf(pszBuff, iBuff, 
			" %s=\"%d\"", DR_TAG_VERSION, pstMeta->iVersion);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}

		iSize	=	snprintf(pszBuff, iBuff, " >\n");
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);


		for(j=0; j<pstMeta->iEntries; j++)
		{
			pstEntry	=	pstMeta->entries + j;

			iSize	=	snprintf(pszBuff, iBuff, "\t\t<%s", DR_TAG_ITEM);
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

			if( pstEntry->szName[0] )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_NAME, pstEntry->szName);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( -1!=pstEntry->idxID )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_ID, pstMacro[pstEntry->idxID].szMacro);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}
			else if( -1!=pstEntry->iID )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%d\"", DR_TAG_ID, pstEntry->iID);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( -1!=pstEntry->idxCount )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_COUNT, pstMacro[pstEntry->idxCount].szMacro);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}
			else if( 1!=pstEntry->iCount && pstEntry->iCount>=0 )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%d\"", DR_TAG_COUNT, pstEntry->iCount);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( -1!=pstEntry->idxVersion )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_VERSION, pstMacro[pstEntry->idxVersion].szMacro);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}
			else if( pstEntry->iVersion>0 && pstEntry->iVersion!=pstMeta->iVersion )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%d\"", DR_TAG_VERSION, pstEntry->iVersion);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( -1!=pstEntry->idxIO)
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_IO, pstMacro[pstEntry->idxIO].szMacro);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}
			else if( pstEntry->iIO )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%d\"", DR_TAG_IO, pstEntry->iIO);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}
			
			if( -1!=pstEntry->ptrMeta )
			{
				pstType	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_TYPE, pstType->szName);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}
			else if( -1!=pstEntry->idxType )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_TYPE, 
				s_types[pstEntry->idxType].pszName);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( pstEntry->iTargetUnit>0 )
			{
				iSize	=	snprintf(pszBuff, iBuff, " %s=\"", DR_TAG_TARGET);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

				iSize	=	dr_off_to_name_i(pstMeta, j, pstEntry->iTargetOff, pszBuff, iBuff);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

				iSize	=	snprintf(pszBuff, iBuff, "\"");
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( pstEntry->iReferUnit>0 )
			{
				iSize	=	snprintf(pszBuff, iBuff, " %s=\"", DR_TAG_REFER);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

				iSize	=	dr_off_to_name_i(pstMeta, j, pstEntry->iReferOff, pszBuff, iBuff);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

				iSize	=	snprintf(pszBuff, iBuff, "\"");
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( pstEntry->iSelectUnit>0 )
			{
				iSize	=	snprintf(pszBuff, iBuff, " %s=\"", DR_TAG_SELECT);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

				iSize	=	dr_off_to_name_i(pstMeta, j, pstEntry->iSelectOff, pszBuff, iBuff);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

				iSize	=	snprintf(pszBuff, iBuff, "\"");
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			if( pstEntry->szDesc[0] )
			{
				iSize	=	snprintf(pszBuff, iBuff, 
				" %s=\"%s\"", DR_TAG_DESC, pstEntry->szDesc);
				DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
			}

			iSize	=	snprintf(pszBuff, iBuff, " />\n");
			DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
		}

		iSize	=	snprintf(pszBuff, iBuff, "\t</%s>\n", 
					DR_TAG_TYPE);
		DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);
	}

	iSize	=	snprintf(pszBuff, iBuff, "</%s>\n", DR_TAG_TYPELIB);
	DR_CHECK_BUFF(pszBuff, iBuff, iSize, iError);

	*piXml	=	pszBuff - pszXml + strlen( pszBuff );

	if( iError )
		return -1;
	else
		return 0;
}

int dr_save_metalib_fp(LPDRMETALIB pstLib, FILE* fp)
{
	LPDRMETA pstMeta;
	LPDRMETAENTRY pstEntry;

	LPDRMETA pstType;
	LPDRMACRO pstMacro;

	const char* pszSizeType;

	int i;
	int j;

	assert( pstLib && fp );

	fprintf(fp, "<%s", DR_TAG_TYPELIB);
	
	if( pstLib->szName[0] )
		fprintf(fp, " %s=\"%s\"", DR_TAG_NAME, pstLib->szName);
	
	if( -1!=pstLib->iID )
		fprintf(fp, " %s=\"%d\"", DR_TAG_ID, pstLib->iID);
	
	if( pstLib->iVersion>0 )
		fprintf(fp, " %s=\"%d\"", DR_TAG_VERSION, pstLib->iVersion);

	if( pstLib->iPadding>0 )
		fprintf(fp, " %s=\"%d\"", DR_TAG_PADDING, pstLib->iPadding);
	
	fprintf(fp, " >\n");

	if( pstLib->iCurMacros>0 )
	{
		fprintf(fp, "\t<%s>\n", DR_TAG_MACROS);

		pstMacro	=	DR_GET_MACRO_TABLE(pstLib);

		for(i=0; i<pstLib->iCurMacros; i++)
		{
			fprintf(fp, "\t\t<%s", DR_TAG_MACRO);
			fprintf(fp, " %s=\"%s\"", DR_TAG_NAME, pstMacro[i].szMacro);
			fprintf(fp, " %s=\"%d\"", DR_TAG_ID, pstMacro[i].iID);
			fprintf(fp, " />\n");
		}

		fprintf(fp, "\t</%s>\n", DR_TAG_MACROS);
	}

	for(i=0; i<pstLib->iCurMetas; i++)
	{
		fprintf(fp, "\t<%s", DR_TAG_TYPE);

		pstMeta	=	DR_IDX_TO_META(pstLib, i);

		if( pstMeta->szName[0] )
			fprintf(fp, " %s=\"%s\"", DR_TAG_NAME, pstMeta->szName);

		if( pstMeta->iSizeType>0 && pstMeta->idxSize>=0 && 
		    pstMeta->idxSize<sizeof(s_types)/sizeof(DRTYPEINFO) )
			pszSizeType=	s_types[pstMeta->idxSize].pszName;
		else
			pszSizeType=	"";
		
		if( pszSizeType[0] )
			fprintf(fp, " %s=\"%s\"", DR_TAG_SIZETYPE, pszSizeType);

		if( -1!=pstMeta->idxID )
			fprintf(fp, " %s=\"%s\"", DR_TAG_ID, pstMacro[pstMeta->idxID].szMacro);
		else if( -1!=pstMeta->iID )
			fprintf(fp, " %s=\"%d\"", DR_TAG_ID, pstMeta->iID);

		if( DR_TYPE_UNION==pstMeta->iType )
			fprintf(fp, " %s=\"%s\"", DR_TAG_CLASS, DR_TAG_UNION);
		else
			fprintf(fp, " %s=\"%s\"", DR_TAG_CLASS, DR_TAG_STRUCT);

		fprintf(fp, " >\n");


		for(j=0; j<pstMeta->iEntries; j++)
		{
			pstEntry	=	pstMeta->entries + j;

			fprintf(fp, "\t\t<%s", DR_TAG_ITEM);

			if( pstEntry->szName[0] )
				fprintf(fp, " %s=\"%s\"", DR_TAG_NAME, pstEntry->szName);

			if( -1!=pstEntry->idxID )
				fprintf(fp, " %s=\"%s\"", DR_TAG_ID, pstMacro[pstEntry->idxID].szMacro);
			else if( -1!=pstEntry->iID )
				fprintf(fp, " %s=\"%d\"", DR_TAG_ID, pstEntry->iID);

			if( -1!=pstEntry->idxCount )
				fprintf(fp, " %s=\"%s\"", DR_TAG_COUNT, pstMacro[pstEntry->idxCount].szMacro);
			else if( 1!=pstEntry->iCount && pstEntry->iCount>=0 )
				fprintf(fp, " %s=\"%d\"", DR_TAG_COUNT, pstEntry->iCount);

			if( -1!=pstEntry->idxVersion )
				fprintf(fp, " %s=\"%s\"", DR_TAG_VERSION, pstMacro[pstEntry->idxVersion].szMacro);
			else if( pstEntry->iVersion>0 && pstEntry->iVersion!=pstLib->iVersion )
				fprintf(fp, " %s=\"%d\"", DR_TAG_VERSION, pstEntry->iVersion);

			if( -1!=pstEntry->idxIO )
				fprintf(fp, " %s=\"%s\"", DR_TAG_IO, pstMacro[pstEntry->idxIO].szMacro);
			else if( pstEntry->iIO )
				fprintf(fp, " %s=\"%d\"", DR_TAG_IO, pstEntry->iIO);

			if( -1!=pstEntry->ptrMeta )
			{
				pstType	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				fprintf(fp, " %s=\"%s\"", DR_TAG_TYPE, pstType->szName);
			}
			else if( -1!=pstEntry->idxType )
				fprintf(fp, " %s=\"%s\"", DR_TAG_TYPE, s_types[pstEntry->idxType].pszName);
			
			if( pstEntry->iTargetUnit>0 )
			{
				fprintf(fp, " %s=\"", DR_TAG_TARGET);
				dr_off_to_name_fp_i(pstMeta, j, pstEntry->iTargetOff, fp);
				fprintf(fp, "\"");
			}

			if( pstEntry->iReferUnit>0 )
			{
				fprintf(fp, " %s=\"", DR_TAG_REFER);
				dr_off_to_name_fp_i(pstMeta, j, pstEntry->iReferOff, fp);
				fprintf(fp, "\"");
			}

			if( pstEntry->iSelectUnit>0 )
			{
				fprintf(fp, " %s=\"", DR_TAG_SELECT);
				dr_off_to_name_fp_i(pstMeta, j, pstEntry->iSelectOff, fp);
				fprintf(fp, "\"");
			}

			if( pstEntry->szDesc[0] )
				fprintf(fp, " %s=\"%s\"", DR_TAG_DESC, pstEntry->szDesc);

			fprintf(fp, " />\n");
		}

		fprintf(fp, "\t</%s>\n", DR_TAG_TYPE);
	}

	fprintf(fp, "</%s>\n", DR_TAG_TYPELIB);

	return 0;
}

int dr_save_metalib_file(LPDRMETALIB pstLib, const char* pszFile)
{
	FILE* fp;
	int iRet;

	assert( pstLib && pszFile );

	fp	=	fopen(pszFile, "w");

	if( NULL==fp )
		return -1;

	iRet	=	dr_save_metalib_fp(pstLib, fp);

	fclose( fp );

	return iRet;
}

int dr_macro_by_name(DRMETALIB* pstLib, const char* pszName, LPDRMACRO* ppstMacro)
{
	LPDRMACRO pstMacro;

	int iRet;
	int iMin;
	int iMax;
	int i;

	assert( pstLib && ppstMacro);

	pstMacro	=	DR_GET_MACRO_TABLE(pstLib);

	iMin    =       0;
	iMax    =	pstLib->iCurMacros - 1;

	while(iMin<=iMax)
	{
		i       =       (iMin+iMax)>>1;

		iRet    =       strcmp(pszName, pstMacro[i].szMacro);

		if( iRet>0 )
			iMin    =       i + 1;
		else if( iRet<0 )
			iMax    =       i - 1;
		else
		{
			*ppstMacro	=	pstMacro + i;

			return 0;
		}
	}

	*ppstMacro	=	NULL;

	return -1;
}

int dr_value_by_name(DRMETALIB* pstLib, const char* pszName, int* piID)
{
	LPDRMACRO pstMacro;

	if( dr_macro_by_name(pstLib, pszName, &pstMacro)<0 )
	{
		*piID	=	-1;
		return -1;
	}
	else
	{
		*piID	=	pstMacro->iID;
		return 0;
	}
}

int dr_meta_by_name(DRMETALIB* pstLib, const char* pszName, LPDRMETA* ppstMeta)
{
	LPDRNAMEENTRY pstName;

	int iRet;
	int iMin;
	int iMax;
	int i;

	assert( pstLib && ppstMeta );

	pstName	=	DR_GET_NAME_TABLE(pstLib);

	iMin    =       0;
	iMax    =	pstLib->iCurMetas-1;

	while(iMin<=iMax)
	{
		i       =       (iMin+iMax)>>1;

		iRet    =       strcmp(pszName, pstName[i].szName);

		if( iRet>0 )
			iMin    =       i + 1;
		else if( iRet<0 )
			iMax    =       i - 1;
		else
		{
			*ppstMeta	=	DR_IDX_TO_META(pstLib, pstName[i].iIdx);

			return 0;
		}
	}

	*ppstMeta	=	NULL;

	return -1;
}

int dr_meta_by_id(DRMETALIB* pstLib, int iID, LPDRMETA* ppstMeta)
{
	LPDRIDENTRY pstID;

	int iRet;
	int iMin;
	int iMax;
	int i;

	assert( pstLib && ppstMeta );

	pstID	=	DR_GET_ID_TABLE(pstLib);

	iMin    =       0;
	iMax    =	pstLib->iCurMetas - 1;

	while(iMin<=iMax)
	{
		i       =       (iMin+iMax)>>1;

		iRet    =       iID - pstID[i].iID;

		if( iRet>0 )
				iMin    =       i + 1;
		else if( iRet<0 )
				iMax    =       i - 1;
		else
		{
			*ppstMeta	=	DR_IDX_TO_META(pstLib, pstID[i].iIdx);

			return 0;
		}
	}

	*ppstMeta	=	NULL;

	return -1;
}


int dr_entry_by_id(DRMETA* pstMeta, int iID, int* piIdx)
{
	int i;
	int iMin;
	int iMax;
	int iRet;

	assert( pstMeta && piIdx );

	*piIdx	=	-1;

	if( DR_TYPE_UNION==pstMeta->iType )	/* binary search for entries sorted. */
	{
		iMin	=	0;
		iMax	=	pstMeta->iEntries - 1;

		while(iMin<=iMax)
		{
			i	=	(iMin + iMax)>>1;

			iRet	=	iID - pstMeta->entries[i].iID;

			if( 0==iRet )
				return i;
			else if( iRet<0 )
				iMax	=	i - 1;
			else
				iMin	=	i + 1;
		}
	}
	else
	{
		for(i=0; i<pstMeta->iEntries; i++)
		{
			if( pstMeta->entries[i].iID==iID )
			{
				*piIdx	=	i;
				return 0;
			}
		}
	}

	return -1;
}

int dr_fprintf(DRMETA* pstMeta, FILE* fp, DRDATA* pstHost, int iVersion)
{
	DRSTACK s;
	int n;

	LPDRSTACKITEM pstTop;
	LPDRMETALIB pstLib;
	LPDRMETAENTRY pstEntry;
	
	DRMETA* pstOldMeta;

	char* pszHostStart;
	char* pszHostEnd;
	
	char* pszPtr;

	int iCount;
	int iRet;

	int i;
	int iID;
	
	int fChange;
	
	int iEntry;
	int iLen;

	assert( DR_TYPE_UNION!=pstMeta->iType && pstMeta->iEntries>0 );
	
	if( !iVersion )
		iVersion	=	DR_MAX_VERSION;

	if( pstMeta->iVersion>iVersion )
		return -1;

	pstLib	=	DR_META_TO_LIB(pstMeta);
	
	pszHostEnd	=	pstHost->pszBuff + pstHost->iBuff;

	s[0].pszHostBase=	pstHost->pszBuff;
	s[0].pszNetBase	=	NULL;
	s[0].pszTarget	=	NULL;
	s[0].pstMeta	=	pstMeta;
	s[0].iUnit	=	0;
	s[0].iEntry	=	0;
	s[0].iCount	=	1;

	n		=	1;

	pstTop		=	s;
	
	fChange		=	0;
	
	iRet		=	0;
	
	fprintf(fp, "[%s]:", pstMeta->szName);

	while( n )
	{		
		/* process the count. */
		if( pstTop->iCount<=0 )
		{
			pstTop--;
			n--;
			
			if( n )
			{
				pstMeta		=	pstTop->pstMeta;
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
			}
			
			continue;
		}
			
		pstEntry	=	pstMeta->entries + pstTop->iEntry;
		
		if( pstEntry->iVersion > iVersion )
		{
			/* DR_DEC_COUNT(pstTop, pstMeta, fChange); */
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		pszHostStart	=	pstTop->pszHostBase + pstEntry->iOff;
		
		if( pstEntry->iReferUnit>0 )
		{
			pszPtr	=	pstTop->pszHostBase + pstEntry->iReferOff;
			DR_GET_INT(iCount, pstEntry->iReferUnit, pszPtr);
			
			if( iCount<0 )	/* the value is too big. */
			{
				iRet	=	-1;
				break;
			}
		}
		else
			iCount	=	pstEntry->iCount;
				
		if( pstEntry->iType<=DR_TYPE_COMPOSITE )
		{
			if( n>=DR_STACK_SIZE ) /* stack full. */
			{
				iRet	=	-1;
				break;
			}
				
			if( iCount<=0 )
			{
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
				continue;
			}
			
			if( DR_TYPE_UNION==pstEntry->iType )
			{
				pszPtr	=	pstTop->pszHostBase + pstEntry->iSelectOff;
				
				DR_GET_INT(iID, pstEntry->iSelectUnit, pszPtr);
				
				pstOldMeta	=	pstMeta;
	
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
	
				DR_GET_ENTRY(iEntry, pstMeta->entries, pstMeta->iEntries, iID);
				
				if( -1==iEntry || pstMeta->entries[iEntry].iVersion>iVersion )
				{
					pstMeta	=	pstOldMeta;
					DR_INC_ENTRY(pstTop, pstMeta, fChange);
					continue;
				}
			}
			else
			{
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iEntry	=	0;
			}

			pstTop++;
			n++;
			
			pstTop->iEntry		=	iEntry;
			pstTop->pstMeta		=	pstMeta;
			pstTop->iCount		=	iCount;
			pstTop->pszHostBase	=	pszHostStart;
			
			pstTop->iUnit		=	pstEntry->iRealUnit;
			pstTop->iOff		=	pstEntry->iRealOff;
		}	
		else if( DR_TYPE_STRING==pstEntry->iType  )
		{
			if( 0==pstEntry->iCount )
				iCount	=	pszHostEnd - pszHostStart;
			
			iLen	=	strnlen(pszHostStart, iCount);
			
			if( iLen>=iCount ) /* the string has no space for null. */
			{
				iRet	=	-1;
				break;
			}
			
			iLen++;
			
			fprintf(fp, " %s=\"%s\"", pstEntry->szName, pszHostStart);
						
			pszHostStart	+=	iCount;
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
		else
		{
			if( pszHostStart + pstEntry->iUnit*iCount>pszHostEnd )
			{
				iRet	=	-1; /* the buffer is too small. */
				break;
			}
			
			if( iCount>1 )
				fprintf(fp, " %s[%d]=", pstEntry->szName, iCount);
			else
				fprintf(fp, " %s=", pstEntry->szName);

			for(i=0; i<iCount; i++ )
			{
				switch( pstEntry->iType )
				{
				case DR_TYPE_LONG:
				case DR_TYPE_INT:
					fprintf(fp, "%d ", *(int*)pszHostStart);
					break;

				case DR_TYPE_ULONG:
				case DR_TYPE_UINT:
					fprintf(fp, "%u ", *(int*)pszHostStart);
					break;
					
				case DR_TYPE_SHORT:
					fprintf(fp, "%d ", (int)*(short*)pszHostStart);
					break;
				case DR_TYPE_USHORT:
					fprintf(fp, "%u ", (unsigned int)*(unsigned short*)pszHostStart);
					break;
					
				case DR_TYPE_FLOAT:
					fprintf(fp, "%f ", *(float*)pszHostStart);
					break;
				case DR_TYPE_DOUBLE:
					fprintf(fp, "%f ", *(double*)pszHostStart);
					break;
		
				case DR_TYPE_CHAR:
					if( pszHostStart[0]>' ' && pszHostStart[0]<0x7f )
						fprintf(fp, "%d[%c] ", (int)(unsigned char)pszHostStart[0], pszHostStart[0]);
					else
						fprintf(fp, "%d ", (int)(unsigned char)pszHostStart[0]);
					break;
				case DR_TYPE_UCHAR:
					if( pszHostStart[0]>' ' && pszHostStart[0]<0x7f )
						fprintf(fp, "%x[%c] ", (int)(unsigned char)pszHostStart[0], pszHostStart[0]);
					else
						fprintf(fp, "%x ", (int)(unsigned char)pszHostStart[0]);

					break;
		
				default:	/* must be 8 bytes. */
					break;
				}
				
				pszHostStart	+=	pstEntry->iUnit;
			}
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
	}
	
	pstHost->iBuff	=	pszHostStart - pstHost->pszBuff;
	fprintf(fp, "\n");

	return iRet;
}

int dr_sprintf(DRMETA* pstMeta, DRDATA* pstOut, DRDATA* pstHost, int iVersion)
{
	DRSTACK s;
	int n;

	LPDRSTACKITEM pstTop;
	LPDRMETALIB pstLib;
	LPDRMETAENTRY pstEntry;
	
	DRMETA* pstOldMeta;

	char* pszHostStart;
	char* pszHostEnd;
	
	char* pszPtr;
	
	char* pszStart;
	int iBuff;
	int iSize;
	int iError;

	int iCount;
	int iRet;

	int i;
	int iID;
	
	int fChange;
	
	int iEntry;
	int iLen;

	assert( pstMeta && pstOut && pstHost );
	
	if( !iVersion )
		iVersion	=	DR_MAX_VERSION;
	
	if( pstMeta->iVersion>iVersion )
		return -1;

	pstLib	=	DR_META_TO_LIB(pstMeta);
	
	iBuff	=	pstOut->iBuff - 1;
	pszStart=	pstOut->pszBuff;
	pszStart[0]	=	'\0';
	pszStart[iBuff]	=	'\0';
	
	pszHostEnd	=	pstHost->pszBuff + pstHost->iBuff;

	s[0].pszHostBase=	pstHost->pszBuff;
	s[0].pszNetBase	=	NULL;
	s[0].pszTarget	=	NULL;
	s[0].pstMeta	=	pstMeta;
	s[0].iUnit	=	0;
	s[0].iEntry	=	0;
	s[0].iCount	=	1;

	n		=	1;

	pstTop		=	s;
	
	fChange		=	0;
	
	iRet		=	0;
	iError		=	0;
	
	iSize	=	snprintf(pszStart, iBuff, "[%s]:", pstMeta->szName);
	DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);

	while( n )
	{
		/* process the count. */
		if( pstTop->iCount<=0 )
		{
			pstTop--;
			n--;
			
			if( n )
			{
				pstMeta		=	pstTop->pstMeta;
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
			}
			
			continue;
		}
			
		pstEntry	=	pstMeta->entries + pstTop->iEntry;
		
		if( pstEntry->iVersion > iVersion )
		{
			/* DR_DEC_COUNT(pstTop, pstMeta, fChange); */
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		pszHostStart	=	pstTop->pszHostBase + pstEntry->iOff;
		
		if( pstEntry->iReferUnit>0 )
		{
			pszPtr	=	pstTop->pszHostBase + pstEntry->iReferOff;
			DR_GET_INT(iCount, pstEntry->iReferUnit, pszPtr);
			
			if( iCount<0 )	/* the value is too big. */
			{
				iRet	=	-1;
				break;
			}
		}
		else
			iCount	=	pstEntry->iCount;
				
		if( pstEntry->iType<=DR_TYPE_COMPOSITE )
		{
			if( n>=DR_STACK_SIZE ) /* stack full. */
			{
				iRet	=	-1;
				break;
			}
				
			if( iCount<=0 )
			{
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
				continue;
			}
			
			if( DR_TYPE_UNION==pstEntry->iType )
			{
				pszPtr	=	pstTop->pszHostBase + pstEntry->iSelectOff;
				
				DR_GET_INT(iID, pstEntry->iSelectUnit, pszPtr);
				
				pstOldMeta	=	pstMeta;
	
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
	
				DR_GET_ENTRY(iEntry, pstMeta->entries, pstMeta->iEntries, iID);
				
				if( -1==iEntry || pstMeta->entries[iEntry].iVersion>iVersion )
				{
					pstMeta	=	pstOldMeta;
					DR_INC_ENTRY(pstTop, pstMeta, fChange);
					continue;
				}
			}
			else
			{
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iEntry	=	0;
			}

			pstTop++;
			n++;
			
			pstTop->iEntry		=	iEntry;
			pstTop->pstMeta		=	pstMeta;
			pstTop->iCount		=	iCount;
			pstTop->pszHostBase	=	pszHostStart;
			
			pstTop->iUnit		=	pstEntry->iRealUnit;
			pstTop->iOff		=	pstEntry->iRealOff;
		}	
		else if( DR_TYPE_STRING==pstEntry->iType  )
		{
			if( 0==pstEntry->iCount )
				iCount	=	pszHostEnd - pszHostStart;
			
			iLen	=	strnlen(pszHostStart, iCount);
			
			if( iLen>=iCount ) /* the string has no space for null. */
			{
				iRet	=	-1;
				break;
			}
			
			iLen++;
			
			iSize	=	snprintf(pszStart, iBuff, " %s=\"%s\"", pstEntry->szName, pszHostStart);
			DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);

			pszHostStart	+=	iCount;
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
		else
		{
			if( pszHostStart + pstEntry->iUnit*iCount>pszHostEnd )
			{
				iRet	=	-1; /* the buffer is too small. */
				break;
			}
			
			if( iCount>1 )
				iSize	=	snprintf(pszStart, iBuff, " %s[%d]=", pstEntry->szName, iCount);
			else
				iSize	=	snprintf(pszStart, iBuff, " %s=", pstEntry->szName);
			
			DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);

			for(i=0; i<iCount; i++ )
			{
				switch( pstEntry->iType )
				{
				case DR_TYPE_LONG:
				case DR_TYPE_INT:
					iSize	=	snprintf(pszStart, iBuff, "%d ", *(int*)pszHostStart);
					break;

				case DR_TYPE_ULONG:
				case DR_TYPE_UINT:
					iSize	=	snprintf(pszStart, iBuff, "%u ", *(int*)pszHostStart);
					break;
					
				case DR_TYPE_SHORT:
					iSize	=	snprintf(pszStart, iBuff, "%d ", (int)*(short*)pszHostStart);
					break;
				case DR_TYPE_USHORT:
					iSize	=	snprintf(pszStart, iBuff, "%u ", 
							(unsigned int)*(unsigned short*)pszHostStart);
					break;
					
				case DR_TYPE_FLOAT:
					iSize	=	snprintf(pszStart, iBuff, "%f ", *(float*)pszHostStart);
					break;
				case DR_TYPE_DOUBLE:
					iSize	=	snprintf(pszStart, iBuff, "%f ", *(double*)pszHostStart);
					break;
		
				case DR_TYPE_CHAR:
					if( pszHostStart[0]>' ' && pszHostStart[0]<0x7f )
						iSize	=	snprintf(pszStart, iBuff, "%d[%c] ", 
								(int)(unsigned char)pszHostStart[0], pszHostStart[0]);
					else
						iSize	=	snprintf(pszStart, iBuff, "%d ", 
								(int)(unsigned char)pszHostStart[0]);
					break;
				case DR_TYPE_UCHAR:
					if( pszHostStart[0]>' ' && pszHostStart[0]<0x7f )
						iSize	=	snprintf(pszStart, iBuff, "0x%x[%c] ", 
								(int)(unsigned char)pszHostStart[0], pszHostStart[0]);
					else
						iSize	=	snprintf(pszStart, iBuff, "0x%x ", 
								(int)(unsigned char)pszHostStart[0]);

					break;
		
				default:	/* must be 8 bytes. */
					iSize	=	snprintf(pszStart, iBuff, "%lld ", 
								(longlong) pszHostStart[0]);
					break;
				}
				
				DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
				
				pszHostStart	+=	pstEntry->iUnit;
			}
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
	}
	
	pstHost->iBuff	=	pszHostStart - pstHost->pszBuff;
/*	
	iSize	=	snprintf(pszStart, iBuff, "\n");
	DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
*/	
	pstOut->iBuff	=	pszStart - pstOut->pszBuff;

	if( iError || iRet<0 )
		return -1;
	else
		return 0;
}

int dr_hton(DRMETA* pstMeta, DRDATA* pstNet, DRDATA* pstHost, int iVersion)
{
	DRSTACK s;
	int n;

	LPDRSTACKITEM pstTop;
	LPDRMETALIB pstLib;
	LPDRMETAENTRY pstEntry;
	
	DRMETA* pstOldMeta;

	char* pszNetStart;
	char* pszNetEnd;
	char* pszHostStart;
	char* pszHostEnd;
	
	char* pszPtr;

	int iCount;
	int iRet;

	int fChange;
	int iID;
	
	int iEntry;
	int iLen;

	assert( DR_TYPE_UNION!=pstMeta->iType && pstMeta->iEntries>0 );
	
	if( !iVersion )
		iVersion	=	DR_MAX_VERSION;
		
	if( pstMeta->iVersion>iVersion )
	{
		pstNet->iBuff	=	0;
		pstHost->iBuff	=	0;
		return -1;
	}

	pstLib	=	DR_META_TO_LIB(pstMeta);
	
	pszNetStart	=	pstNet->pszBuff;
	pszNetEnd	=	pszNetStart + pstNet->iBuff;
	
	pszHostEnd	=	pstHost->pszBuff + pstHost->iBuff;

	pstTop		=	s;
	n			=	1;

	s[0].pszHostBase=	pstHost->pszBuff;
	s[0].pszNetBase	=	pszNetStart;
	s[0].pstMeta	=	pstMeta;
	s[0].iEntry	=	0;
	s[0].iCount	=	1;

	if( pstMeta->iSizeType )
	{
		pstTop->iUnit		=	pstMeta->iSizeType;
		pstTop->iOff		=	-pstMeta->iSizeType;
		DR_CALC_TARGET(pstTop, pszNetStart);			
	}
	else
	{
		s[0].iUnit	=	0;
		s[0].iOff	=	0;
		s[0].pszTarget	=	NULL;
	}

	fChange		=	0;
	
	iRet		=	0;

	while( n )
	{
		if( fChange )
		{
			if( pszNetStart > pszNetEnd )	/* check size. */
			{
				iRet	=	-1;
				break;
			}
			
			DR_SET_INT_NET(pstTop->pszTarget, pstTop->iUnit, pszNetStart-pstTop->pszNetBase);
			
			fChange	=	0;
			
			if( pstTop->iCount )
			{
				DR_CALC_TARGET(pstTop, pszNetStart);
			}
		}
		
		/* process the count. */
		if( pstTop->iCount<=0 )
		{
			pstTop--;
			n--;
			
			if( n )
			{
				pstMeta		=	pstTop->pstMeta;
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
			}
			
			continue;
		}
			
		pstEntry	=	pstMeta->entries + pstTop->iEntry;
		
		if( pstEntry->iVersion > iVersion )
		{
			/* DR_DEC_COUNT(pstTop, pstMeta, fChange); */
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		pszHostStart	=	pstTop->pszHostBase + pstEntry->iOff;
		
		if( pstEntry->iReferUnit>0 )
		{
			pszPtr	=	pstTop->pszHostBase + pstEntry->iReferOff;
			DR_GET_INT(iCount, pstEntry->iReferUnit, pszPtr);
			
			if( iCount<0 )	/* the value is too big. */
			{
				iRet	=	-1;
				break;
			}
		}
		else
			iCount	=	pstEntry->iCount;

		if( pstEntry->iCount && iCount>pstEntry->iCount )
		{
			iRet	=	-1;
			break;
		}
				
		if( pstEntry->iType<=DR_TYPE_COMPOSITE )
		{
			if( n>=DR_STACK_SIZE ) /* stack full. */
			{
				iRet	=	-1;
				break;
			}
				
			if( iCount<=0 )
			{
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
				continue;
			}
			
			if( DR_TYPE_UNION==pstEntry->iType )
			{
				pszPtr	=	pstTop->pszHostBase + pstEntry->iSelectOff;
				
				DR_GET_INT(iID, pstEntry->iSelectUnit, pszPtr);
				
				pstOldMeta	=	pstMeta;
	
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
	
				DR_GET_ENTRY(iEntry, pstMeta->entries, pstMeta->iEntries, iID);
				
				if( -1==iEntry || pstMeta->entries[iEntry].iVersion>iVersion )
				{
					pstMeta	=	pstOldMeta;
					DR_INC_ENTRY(pstTop, pstMeta, fChange);
					continue;
				}
			}
			else
			{
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iEntry	=	0;
			}

			pstTop++;
			n++;
			
			pstTop->iEntry		=	iEntry;
			pstTop->pstMeta		=	pstMeta;
			pstTop->iCount		=	iCount;
			pstTop->pszHostBase	=	pszHostStart;
			
			pstTop->iUnit		=	pstEntry->iRealUnit;
			pstTop->iOff		=	pstEntry->iRealOff;
			
			DR_CALC_TARGET(pstTop, pszNetStart);			
		}	
		else if( DR_TYPE_STRING==pstEntry->iType  )
		{
			if( 0==pstEntry->iCount )
				iCount	=	pszHostEnd - pszHostStart;
			
			iLen	=	strnlen(pszHostStart, iCount);
			
			if( iLen>=iCount ) /* the string has no space for null. */
			{
				iRet	=	-1;
				break;
			}
			
			iLen++;
			
			if( pszNetStart + iLen + sizeof(int)> pszNetEnd ) /* no buffer for storing. */
			{
				iRet	=	-1;
				break;
			}
			
			*(unsigned long*)pszNetStart	=	htonl((unsigned long)iLen);
			pszNetStart	+=	sizeof(int);
			
			memcpy(pszNetStart, pszHostStart, iLen);
			pszNetStart	+=	iLen;
			pszHostStart	+=	iCount;
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
		else
		{
			if( pszHostStart + pstEntry->iUnit*iCount>pszHostEnd ||
			    pszNetStart + pstEntry->iUnit*iCount>pszNetEnd )
			{
				iRet	=	-1; /* the buffer is too small. */
				break;
			}
			    
			switch( pstEntry->iUnit )
			{
			case 4:
				HTONNL(pszNetStart, pszHostStart, iCount);
				break;
				
			case 2:
				HTONNS(pszNetStart, pszHostStart, iCount);
				break;
	
			case 1:
				MEMCPY(pszNetStart, pszHostStart, iCount, DR_MIN_COPY);
				break;
	
			default:	/* must be 8 bytes. */
				HTONNQ(pszNetStart, pszHostStart, iCount);
				break;
			}
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
	}
	
	pstNet->iBuff	=	pszNetStart - pstNet->pszBuff;
	pstHost->iBuff	=	pszHostStart - pstHost->pszBuff;

	return iRet;
}

int dr_ntoh(DRMETA* pstMeta, DRDATA* pstHost, DRDATA* pstNet, int iVersion)
{
	DRSTACK s;
	int n;

	LPDRSTACKITEM pstTop;
	LPDRMETALIB pstLib;
	LPDRMETAENTRY pstEntry;
	
	LPDRMETA pstOldMeta;

	char* pszNetStart;
	char* pszNetEnd;
	char* pszHostStart;
	char* pszHostEnd;

	char* pszPtr;

	int iCount;
	int iRet;

	int fChange;
	int iID;
	
	int iEntry;
	int iLen;

	assert( DR_TYPE_UNION!=pstMeta->iType && pstMeta->iEntries>0 );
	
	if( !iVersion )
		iVersion	=	DR_MAX_VERSION;
		
	if( pstMeta->iVersion>iVersion )
	{
		pstNet->iBuff	=	0;
		pstHost->iBuff	=	0;
		return -1;
	}

	pstLib	=	DR_META_TO_LIB(pstMeta);
	
	pszNetStart	=	pstNet->pszBuff;
	pszNetEnd	=	pszNetStart + pstNet->iBuff;
	
	pszHostEnd	=	pstHost->pszBuff + pstHost->iBuff;

	n			=	1;
	pstTop		=	s;

	s[0].pszHostBase=	pstHost->pszBuff;
	s[0].pszNetBase	=	pszNetStart;
	s[0].pstMeta	=	pstMeta;
	s[0].iEntry	=	0;
	s[0].iCount	=	1;
	
	if( pstMeta->iSizeType )
	{
		pstTop->iUnit		=	pstMeta->iSizeType;
		pstTop->iOff		=	-pstMeta->iSizeType;
		DR_CALC_TARGET(pstTop, pszNetStart);
	}
	else
	{
		s[0].pszTarget	=	NULL;
		s[0].iUnit	=	0;
		s[0].iOff	=	0;
	}

	fChange		=	0;
	
	iRet		=	0;

	while( n )
	{
		if( fChange )
		{
			DR_GET_INT_NET(iLen, pstTop->iUnit, pstTop->pszTarget );
			pszNetStart	=	pstTop->pszNetBase + iLen;
			
			if( pszNetStart>pszNetEnd )
			{
				iRet	=	-1;
				break;
			}

			fChange	=	0;
			
			if( pstTop->iCount )
			{
				DR_CALC_TARGET(pstTop, pszNetStart);
			}
		}
		
		/* process the count. */
		if( pstTop->iCount<=0 )
		{
			pstTop--;
			n--;
			
			if( n ) 
			{
				pstMeta		=	pstTop->pstMeta;
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
			}
			
			continue;
		}
			
		pstEntry	=	pstMeta->entries + pstTop->iEntry;
		
		pszHostStart	=	pstTop->pszHostBase + pstEntry->iOff;

		if( pstEntry->iReferUnit>0 )
		{
			pszPtr	=	pstTop->pszHostBase + pstEntry->iReferOff;
			DR_GET_INT(iCount, pstEntry->iReferUnit, pszPtr);
			
			if( iCount<0 )	/* the value is too big. */
			{
				iRet	=	-1;
				break;
			}
		}
		else
			iCount	=	pstEntry->iCount;

		if( pstEntry->iCount && iCount>pstEntry->iCount )
		{
			iRet	=	-1;
			break;
		}
		
		if( pstEntry->iVersion > iVersion )
		{
			/* DR_DEC_COUNT(pstTop, pstMeta, fChange); */

			iCount	*=	pstEntry->iUnit;
			
			if( iCount<=1 )
				pszHostStart[0]	=	'\0';
			else
				memset(pszHostStart, 0, iCount);

			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		if( pstEntry->iType<=DR_TYPE_COMPOSITE )
		{
			if( n>=DR_STACK_SIZE ) /* stack full. */
			{
				iRet	=	-1;
				break;
			}
			
			if( iCount<=0 )
			{
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
				continue;
			}

			if( DR_TYPE_UNION==pstEntry->iType )
			{
				pszPtr	=	pstTop->pszHostBase + pstEntry->iSelectOff;
				DR_GET_INT(iID, pstEntry->iSelectUnit, pszPtr);
				
				pstOldMeta	=	pstMeta;
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				
				DR_GET_ENTRY(iEntry, pstMeta->entries, pstMeta->iEntries, iID);
				
				if( -1==iEntry || pstMeta->entries[iEntry].iVersion>iVersion )
				{
					pstMeta	=	pstOldMeta;
					DR_INC_ENTRY(pstTop, pstMeta, fChange);
					continue;
				}
			}
			else
			{
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iEntry	=	0;
			}
	
			pstTop++;
			n++;
			
			pstTop->iEntry		=	iEntry;
			pstTop->pstMeta		=	pstMeta;
			pstTop->iCount		=	iCount;
			pstTop->pszHostBase	=	pszHostStart;
			
			pstTop->iUnit		=	pstEntry->iRealUnit;
			pstTop->iOff		=	pstEntry->iRealOff;
			
			DR_CALC_TARGET(pstTop, pszNetStart);
		}
		else if( DR_TYPE_STRING==pstEntry->iType  )
		{
			if( !iCount )
				iCount	=	pszHostEnd - pszHostStart;
			
			if( pszNetStart + sizeof(int) >= pszNetEnd )
			{
				iRet	=	-1; /* net buffer too small. */
				break;
			}
			
			iLen	=	(int)(unsigned int)ntohl(*(unsigned long*)pszNetStart);
			
			if( iLen<1 )	/* string length can not be less than 1. */
			{
				iRet	=	-1;
				break;
			}
			
			if( pszNetStart + iLen + sizeof(int)> pszNetEnd )
			{
				iRet	=	-1; /* net buffer too small. */
				break;
			}
			
			if( iCount<iLen )
			{
				iRet	=	-1; /* host buffer too small. */
				break;
			}
			
			if( pszNetStart[sizeof(int)+iLen-1] )
			{
				iRet	=	-1; /* string's terminator not NULL. */
				break;
			}
			
			pszNetStart +=	sizeof(int);
						
			memcpy(pszHostStart, pszNetStart, iLen);
			pszHostStart	+=	iCount;
			pszNetStart	+=	iLen;
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
		else
		{
			if( pszHostStart + pstEntry->iUnit*iCount>pszHostEnd ||
			    pszNetStart + pstEntry->iUnit*iCount>pszNetEnd )
			{
				iRet	=	-1;	/* the buffer is too small. */
				break;
			}
			    
			switch( pstEntry->iUnit )
			{
			case 4:
				NTOHNL(pszHostStart, pszNetStart, iCount);
				break;
				
			case 2:
				NTOHNS(pszHostStart, pszNetStart, iCount);
				break;
	
			case 1:
				MEMCPY(pszHostStart, pszNetStart, iCount, DR_MIN_COPY);
				break;
	
			default:	/* must be 8 bytes. */
				NTOHNQ(pszHostStart, pszNetStart, iCount);
				break;
			}
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
	}
	
	pstNet->iBuff	=	pszNetStart - pstNet->pszBuff;
	pstHost->iBuff	=	pszHostStart - pstHost->pszBuff;

	return iRet;
}

int dr_output(DRMETA* pstMeta, DRDATA* pstXml, DRDATA* pstHost, int iVersion)
{
	DRSTACK s;
	int n;

	LPDRSTACKITEM pstTop;
	LPDRMETALIB pstLib;
	LPDRMETAENTRY pstEntry;
	
	DRMETA* pstOldMeta;

	char* pszHostStart;
	char* pszHostEnd;
	
	char* pszPtr;
	
	char* pszStart;
	int iBuff;
	int iSize;
	int iError;

	int iCount;
	int iRet;

	int i;
	int iID;
	
	int fChange;
	
	int iEntry;
	int iLen;

	assert( pstMeta && pstXml && pstHost );
	
	if( !iVersion )
		iVersion	=	DR_MAX_VERSION;
	
	if( pstMeta->iVersion>iVersion )
		return -1;

	pstLib	=	DR_META_TO_LIB(pstMeta);
	
	iBuff	=	pstXml->iBuff - 1;
	pszStart=	pstXml->pszBuff;
	pszStart[0]	=	'\0';
	pszStart[iBuff]	=	'\0';
	
	pszHostEnd	=	pstHost->pszBuff + pstHost->iBuff;

	s[0].pszHostBase=	pstHost->pszBuff;
	s[0].pszNetBase	=	NULL;
	s[0].pszTarget	=	NULL;
	s[0].pstMeta	=	pstMeta;
	s[0].iUnit	=	0;
	s[0].iEntry	=	0;
	s[0].iCount	=	1;

	n		=	1;

	pstTop		=	s;
	
	fChange		=	0;
	
	iRet		=	0;
	iError		=	0;
	
	iSize	=	snprintf(pszStart, iBuff, "<%s>\n", pstMeta->szName);
	DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);

	while( n )
	{
		if( fChange && DR_TYPE_STRUCT==pstMeta->iType )
		{
			iSize	=	snprintf(pszStart, iBuff, "</%s>\n", pstMeta->szName);
			DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);

			if( pstTop->iCount>0 )
			{
				iSize	=	snprintf(pszStart, iBuff, "\n<%s>\n", pstMeta->szName);
				DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
			}
		}
		
		fChange	=	0;
		
		/* process the count. */
		if( pstTop->iCount<=0 )
		{
			pstTop--;
			n--;
			
			if( n )
			{
				pstMeta		=	pstTop->pstMeta;
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
			}
			
			continue;
		}
			
		pstEntry	=	pstMeta->entries + pstTop->iEntry;
		
		if( pstEntry->iVersion > iVersion )
		{
			/* DR_DEC_COUNT(pstTop, pstMeta, fChange); */
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		if( pstEntry->iIO & DR_IO_NOOUTPUT )
		{
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		pszHostStart	=	pstTop->pszHostBase + pstEntry->iOff;
		
		if( pstEntry->iReferUnit>0 )
		{
			pszPtr	=	pstTop->pszHostBase + pstEntry->iReferOff;
			DR_GET_INT(iCount, pstEntry->iReferUnit, pszPtr);
			
			if( iCount<0 )	/* the value is too big. */
			{
				iRet	=	-1;
				break;
			}
		}
		else
			iCount	=	pstEntry->iCount;
				
		if( pstEntry->iType<=DR_TYPE_COMPOSITE )
		{
			if( n>=DR_STACK_SIZE ) /* stack full. */
			{
				iRet	=	-1;
				break;
			}
				
			if( iCount<=0 )
			{
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
				continue;
			}
			
			if( DR_TYPE_UNION==pstEntry->iType )
			{
				pszPtr	=	pstTop->pszHostBase + pstEntry->iSelectOff;
				
				DR_GET_INT(iID, pstEntry->iSelectUnit, pszPtr);
				
				pstOldMeta	=	pstMeta;
	
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
	
				DR_GET_ENTRY(iEntry, pstMeta->entries, pstMeta->iEntries, iID);
				
				if( -1==iEntry || pstMeta->entries[iEntry].iVersion>iVersion )
				{
					pstMeta	=	pstOldMeta;
					DR_INC_ENTRY(pstTop, pstMeta, fChange);
					continue;
				}
			}
			else
			{
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iEntry	=	0;
			}

			pstTop++;
			n++;
			
			pstTop->iEntry		=	iEntry;
			pstTop->pstMeta		=	pstMeta;
			pstTop->iCount		=	iCount;
			pstTop->pszHostBase	=	pszHostStart;
			
			pstTop->iUnit		=	1;
			pstTop->iOff		=	pstEntry->iRealOff;
			
			if( DR_TYPE_STRUCT==pstMeta->iType )
			{
				iSize	=	snprintf(pszStart, iBuff, "\n<%s>\n", pstMeta->szName);
				DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
			}
		}	
		else if( DR_TYPE_STRING==pstEntry->iType  )
		{
			if( 0==pstEntry->iCount )
				iCount	=	pszHostEnd - pszHostStart;
			
			iLen	=	strnlen(pszHostStart, iCount);
			
			if( iLen>=iCount ) /* the string has no space for null. */
			{
				iRet	=	-1;
				break;
			}
			
			iLen++;
			
			iSize	=	snprintf(pszStart, iBuff, "<%s>%s</%s>\n", pstEntry->szName,
					pszHostStart, pstEntry->szName);
			DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);

			pszHostStart	+=	iCount;
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
		else
		{
			if( pszHostStart + pstEntry->iUnit*iCount>pszHostEnd )
			{
				iRet	=	-1; /* the buffer is too small. */
				break;
			}
			
			iSize	=	snprintf(pszStart, iBuff, "<%s>", pstEntry->szName);
			DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);

			for(i=0; i<iCount; i++ )
			{
				switch( pstEntry->iType )
				{
				case DR_TYPE_LONG:
				case DR_TYPE_INT:
					iSize	=	snprintf(pszStart, iBuff, " %d ", *(int*)pszHostStart);
					break;

				case DR_TYPE_ULONG:
				case DR_TYPE_UINT:
					iSize	=	snprintf(pszStart, iBuff, " %u ", *(int*)pszHostStart);
					break;
					
				case DR_TYPE_SHORT:
					iSize	=	snprintf(pszStart, iBuff, " %d ", (int)*(short*)pszHostStart);
					break;
				case DR_TYPE_USHORT:
					iSize	=	snprintf(pszStart, iBuff, " %u ", 
							(unsigned int)*(unsigned short*)pszHostStart);
					break;
					
				case DR_TYPE_FLOAT:
					iSize	=	snprintf(pszStart, iBuff, " %f ", *(float*)pszHostStart);
					break;
				case DR_TYPE_DOUBLE:
					iSize	=	snprintf(pszStart, iBuff, " %f ", *(double*)pszHostStart);
					break;
		
				case DR_TYPE_CHAR:
					iSize	=	snprintf(pszStart, iBuff, " %d ", 
							(int)(unsigned char)pszHostStart[0]);
					break;
				case DR_TYPE_UCHAR:
					iSize	=	snprintf(pszStart, iBuff, " 0x%x ", 
							(int)(unsigned char)pszHostStart[0]);
					break;
		
				default:
					iSize	=	0;
					break;
				}

				DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
								
				pszHostStart	+=	pstEntry->iUnit;
			}
			
			iSize	=	snprintf(pszStart, iBuff, "</%s>\n", pstEntry->szName);
			DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
	}

	iSize	=	snprintf(pszStart, iBuff, "</%s>\n", pstMeta->szName);
	DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
	
	pstHost->iBuff	=	pszHostStart - pstHost->pszBuff;
/*	
	iSize	=	snprintf(pszStart, iBuff, "\n");
	DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
*/	
	pstXml->iBuff	=	pszStart - pstXml->pszBuff;

	if( iError || iRet<0 )
		return -1;
	else
		return 0;
}

int dr_output_fp(DRMETA* pstMeta, FILE* fp, DRDATA* pstHost, int iVersion)
{
	DRSTACK s;
	int n;

	LPDRSTACKITEM pstTop;
	LPDRMETALIB pstLib;
	LPDRMETAENTRY pstEntry;
	
	DRMETA* pstOldMeta;

	char* pszHostStart;
	char* pszHostEnd;
	
	char* pszPtr;
	
	int iCount;
	int iRet;

	int i;
	int iID;
	
	int fChange;
	
	int iEntry;
	int iLen;

	assert( pstMeta && fp && pstHost );
	
	if( !iVersion )
		iVersion	=	DR_MAX_VERSION;
	
	if( pstMeta->iVersion>iVersion )
		return -1;

	pstLib	=	DR_META_TO_LIB(pstMeta);
		
	pszHostEnd	=	pstHost->pszBuff + pstHost->iBuff;

	s[0].pszHostBase=	pstHost->pszBuff;
	s[0].pszNetBase	=	NULL;
	s[0].pszTarget	=	NULL;
	s[0].pstMeta	=	pstMeta;
	s[0].iUnit	=	0;
	s[0].iEntry	=	0;
	s[0].iCount	=	1;

	n		=	1;

	pstTop		=	s;
	
	fChange		=	0;
	
	iRet		=	0;
	
	fprintf(fp, "<%s>\n", pstMeta->szName);

	while( n )
	{
		if( fChange && DR_TYPE_STRUCT==pstMeta->iType )
		{
			fprintf(fp, "</%s>\n", pstMeta->szName);

			if( pstTop->iCount>0 )
				fprintf(fp, "\n<%s>\n", pstMeta->szName);
		}
		
		fChange	=	0;
		
		/* process the count. */
		if( pstTop->iCount<=0 )
		{
			pstTop--;
			n--;
			
			if( n )
			{
				pstMeta		=	pstTop->pstMeta;
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
			}
			
			continue;
		}
			
		pstEntry	=	pstMeta->entries + pstTop->iEntry;
		
		if( pstEntry->iVersion > iVersion )
		{
			/* DR_DEC_COUNT(pstTop, pstMeta, fChange); */
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		if( pstEntry->iIO & DR_IO_NOOUTPUT )
		{
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
			continue;
		}

		pszHostStart	=	pstTop->pszHostBase + pstEntry->iOff;
		
		if( pstEntry->iReferUnit>0 )
		{
			pszPtr	=	pstTop->pszHostBase + pstEntry->iReferOff;
			DR_GET_INT(iCount, pstEntry->iReferUnit, pszPtr);
			
			if( iCount<0 )	/* the value is too big. */
			{
				iRet	=	-1;
				break;
			}
		}
		else
			iCount	=	pstEntry->iCount;
				
		if( pstEntry->iType<=DR_TYPE_COMPOSITE )
		{
			if( n>=DR_STACK_SIZE ) /* stack full. */
			{
				iRet	=	-1;
				break;
			}
				
			if( iCount<=0 )
			{
				DR_INC_ENTRY(pstTop, pstMeta, fChange);
				continue;
			}
			
			if( DR_TYPE_UNION==pstEntry->iType )
			{
				pszPtr	=	pstTop->pszHostBase + pstEntry->iSelectOff;
				
				DR_GET_INT(iID, pstEntry->iSelectUnit, pszPtr);
				
				pstOldMeta	=	pstMeta;
	
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
	
				DR_GET_ENTRY(iEntry, pstMeta->entries, pstMeta->iEntries, iID);
				
				if( -1==iEntry || pstMeta->entries[iEntry].iVersion>iVersion )
				{
					pstMeta	=	pstOldMeta;
					DR_INC_ENTRY(pstTop, pstMeta, fChange);
					continue;
				}
			}
			else
			{
				pstMeta	=	DR_PTR_TO_META(pstLib, pstEntry->ptrMeta);
				iEntry	=	0;
			}

			pstTop++;
			n++;
			
			pstTop->iEntry		=	iEntry;
			pstTop->pstMeta		=	pstMeta;
			pstTop->iCount		=	iCount;
			pstTop->pszHostBase	=	pszHostStart;
			
			pstTop->iUnit		=	1;
			pstTop->iOff		=	pstEntry->iRealOff;
			
			if( DR_TYPE_STRUCT==pstMeta->iType )
				fprintf(fp, "\n<%s>\n", pstMeta->szName);
		}	
		else if( DR_TYPE_STRING==pstEntry->iType  )
		{
			if( 0==pstEntry->iCount )
				iCount	=	pszHostEnd - pszHostStart;
			
			iLen	=	strnlen(pszHostStart, iCount);
			
			if( iLen>=iCount ) /* the string has no space for null. */
			{
				iRet	=	-1;
				break;
			}
			
			iLen++;
			
			fprintf(fp, "<%s>%s</%s>\n", pstEntry->szName,
					pszHostStart, pstEntry->szName);

			pszHostStart	+=	iCount;
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
		else
		{
			if( pszHostStart + pstEntry->iUnit*iCount>pszHostEnd )
			{
				iRet	=	-1; /* the buffer is too small. */
				break;
			}
			
			fprintf(fp, "<%s>", pstEntry->szName);

			for(i=0; i<iCount; i++ )
			{
				switch( pstEntry->iType )
				{
				case DR_TYPE_LONG:
				case DR_TYPE_INT:
					fprintf(fp, " %d ", *(int*)pszHostStart);
					break;

				case DR_TYPE_ULONG:
				case DR_TYPE_UINT:
					fprintf(fp, " %u ", *(int*)pszHostStart);
					break;
					
				case DR_TYPE_SHORT:
					fprintf(fp, " %d ", (int)*(short*)pszHostStart);
					break;
				case DR_TYPE_USHORT:
					fprintf(fp, " %u ", (unsigned int)*(unsigned short*)pszHostStart);
					break;
					
				case DR_TYPE_FLOAT:
					fprintf(fp, " %f ", *(float*)pszHostStart);
					break;
				case DR_TYPE_DOUBLE:
					fprintf(fp, " %f ", *(double*)pszHostStart);
					break;
		
				case DR_TYPE_CHAR:
					fprintf(fp, " %d ", (int)(unsigned char)pszHostStart[0]);
					break;
				case DR_TYPE_UCHAR:
					fprintf(fp, " 0x%x ", (int)(unsigned char)pszHostStart[0]);
					break;
		
				default:
					break;
				}
								
				pszHostStart	+=	pstEntry->iUnit;
			}
			
			fprintf(fp, "</%s>\n", pstEntry->szName);
			
			DR_INC_ENTRY(pstTop, pstMeta, fChange);
		}
	}

	fprintf(fp, "</%s>\n", pstMeta->szName);
	
/*	
	iSize	=	snprintf(pszStart, iBuff, "\n");
	DR_CHECK_BUFF(pszStart, iBuff, iSize, iError);
*/

	return iRet;
}

int dr_output_file(DRMETA* pstMeta, const char* pszFile, DRDATA* pstHost, int iVersion)
{
	FILE* fp;
	int iRet;
	
	assert( pstMeta && pszFile && pstHost );
	
	fp	=	fopen(pszFile, "w");
	
	if( !fp )
		return -1;
		
	iRet	=	dr_output_fp(pstMeta, fp, pstHost, iVersion);
	
	fclose(fp);
	
	return iRet;
}

void dr_decl_version(int iVersion, FILE* fp)
{		
	fprintf(fp, "/********************************************************************\n");
	fprintf(fp, "**       This head file is generated by program,                   **\n");
	fprintf(fp, "**            Please do not change it directly.                    **\n");
	fprintf(fp, "********************************************************************/\n");
	fprintf(fp, "\n");

	if( iVersion>0 )
	{
		fprintf(fp, "/*       Version Number: %d          */\n", iVersion);
		fprintf(fp, "\n");
	}
}

void dr_decl_line(int iLine, FILE* fp)
{
	int i;

	for(i=0; i<iLine; i++)
	{
		fprintf(fp, "\n");
	}
}

void dr_decl_comment(const char* pszComment, int iLine, FILE* fp)
{
	fprintf(fp, "/*   %s   */", pszComment);

	dr_decl_line(iLine, fp);
}

void dr_decl_cpp_start(FILE* fp)
{
	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "extern \"C\" {\n");
	fprintf(fp, "#endif\n\n");
}

void dr_decl_cpp_end(FILE* fp)
{
	fprintf(fp, "#ifdef __cplusplus\n");
	fprintf(fp, "}\n");
	fprintf(fp, "#endif\n\n");
}

void dr_decl_mutex_start(const char* szName, FILE* fp)
{
	fprintf(fp, "#ifndef %s\n", szName);
	fprintf(fp, "#define %s\n\n", szName);
}

void dr_decl_mutex_end(const char* szName, FILE* fp)
{
	fprintf(fp, "#endif /* %s */\n", szName);
}

void dr_decl_pack(int iPack, FILE* fp)
{
	if( iPack>0 )
		fprintf(fp, "#pragma pack(%d)\n\n", iPack);
	else
		fprintf(fp, "#pragma pack()\n\n");
}

void dr_decl_macro(LPDRMACRO pstMacro, int iWidth, FILE* fp)
{
	char szMacro[DR_MACRO_LEN];

	strcpy(szMacro, pstMacro->szMacro);
	strupr(szMacro);
	fprintf(fp, "#define %-*s \t%d\n", iWidth, szMacro, pstMacro->iID);
}

void dr_decl_type(LPDRMETA pstMeta, int iWidth, FILE* fp)
{
	char szName[DR_NAME_LEN];

	strcpy(szName, pstMeta->szName);
	strupr(szName);

	if( DR_TYPE_UNION==pstMeta->iType )
	{
		fprintf(fp, "%s tag%s;\n", DR_TAG_UNION, 
			pstMeta->szName);		
		fprintf(fp, "typedef %s  tag%-*s \t%s;\n", 
			DR_TAG_UNION, iWidth, pstMeta->szName, szName);
		fprintf(fp, "typedef %s  tag%-*s \t*LP%s;\n", 
			DR_TAG_UNION, iWidth, pstMeta->szName, szName);
	}
	else
	{
		fprintf(fp, "%s tag%s;\n", DR_TAG_STRUCT, 
			pstMeta->szName);
		fprintf(fp, "typedef %s tag%-*s \t%s;\n", 
			DR_TAG_STRUCT, iWidth, pstMeta->szName, szName);
		fprintf(fp, "typedef %s tag%-*s \t*LP%s;\n", 
			DR_TAG_STRUCT, iWidth, pstMeta->szName, szName);
	}

	fprintf(fp, "\n");
}

int dr_metalib_to_hpp(DRMETALIB* pstLib, const char* pszFile, const char* pszLeading)
{
	LPDRMETA pstMeta;
	LPDRMACRO pstMacro;
	FILE* fp;

	char szHead[DR_MACRO_LEN];
	char szMacro[DR_MACRO_LEN];
	char szName[DR_NAME_LEN];
	
	int i;
	int n;

	n	=	DR_NAME_LEN;
	
	fp	=	fopen(pszFile, "w");
	
	if( NULL==fp )
		return -1;
		
	dr_decl_version(pstLib->iVersion, fp);

	pstMacro=	DR_GET_MACRO_TABLE(pstLib);

	os_file_to_macro(szHead, DR_MACRO_LEN, pszFile);

	dr_decl_mutex_start(szHead, fp);
		
	dr_decl_comment("Define Macros.", 2, fp);

	/* begin define macros. */
	for(i=0; i<pstLib->iCurMacros; i++)
	{
		dr_decl_macro(&pstMacro[i], n+8, fp);
	}

	dr_decl_line(2, fp);

	/* begin define extern c. */
	dr_decl_cpp_start(fp);
	
	dr_decl_comment("Structs/unions prototype.", 2, fp);
	
	/* define prototype of struct or union. */
	for(i=0; i<pstLib->iCurMetas; i++)
	{
		pstMeta	=	DR_IDX_TO_META(pstLib, i);
		dr_decl_type(pstMeta, n, fp);
	}

	dr_decl_line(2, fp);
	
	dr_decl_comment("Define structs/unions.", 2, fp);
	
	dr_decl_pack(1, fp);
	
	/* now define the struct or union. */
	for(i=0; i<pstLib->iCurMetas; i++)
	{
		pstMeta	=	DR_IDX_TO_META(pstLib, i);
		
		dr_meta_to_hpp(pstMeta, fp, pszLeading);

		dr_decl_line(1, fp);
	}

	dr_decl_line(1, fp);
	
	dr_decl_pack(0, fp);

	dr_decl_cpp_end(fp);
	
	dr_decl_mutex_end(szHead, fp);

	fclose(fp);

	return 0;
}
