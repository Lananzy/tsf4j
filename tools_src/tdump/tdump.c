/*
**  @file $RCSfile: tdump.c,v $
**  general description of this module
**  $Id: tdump.c,v 1.1 2008/08/19 03:40:37 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/08/19 03:40:37 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tdr/tdr.h"

#include "tapp/tapp.h"
#include "tlog/tlog.h"
#include "tlog/tlogbin.h"

#include <assert.h>

#include "tdr/tdr_metalib_manage_i.h"

#include "tdump.h"

extern unsigned char g_szMetalib_tdump[];

static TDUMPCONF gs_stConf;
static TDUMPCTX gs_stCtx;

static char gs_szBuff[0x10000];


LPTDRMETA tdump_type_to_meta(TDUMPMAPLISTINST* a_pstMapListInst, int a_iType)
{
	TDUMPMAPINST* pstMapInst;
	int i;

	for(i=0; i<a_pstMapListInst->iCount; i++)
	{
		pstMapInst	=	a_pstMapListInst->astInsts + i;

		if( pstMapInst->iType==a_iType )
			return pstMapInst->pstMeta;
	}

	return NULL;
}

int tdump_init_liblist(TDUMPLIBLISTINST* a_pstLibListInst, TDUMPLIBLIST* a_pstLibList)
{
	int iRet=0;
	TDUMPLIB* pstLib;
	TDUMPLIBINST* pstLibInst;

	a_pstLibListInst->iCount	=	0;

	while(a_pstLibListInst->iCount<a_pstLibList->iCount)
	{
		pstLib		=	a_pstLibList->astLibs + a_pstLibListInst->iCount;
		pstLibInst	=	a_pstLibListInst->astInsts+ a_pstLibListInst->iCount;

		STRNCPY(pstLibInst->szName, pstLib->szName, sizeof(pstLibInst->szName));

		if( tdr_load_metalib(&pstLibInst->pstLib, pstLib->szPath)<0)
		{
			iRet	=	-1;
			break;
		}

		a_pstLibListInst->iCount++;
	}

	return iRet;
}

int tdump_find_meta(TDUMPLIBLISTINST* a_pstLibListInst, const char* a_pszMeta, const char* a_pszLibName, LPTDRMETA* a_ppstFind)
{
	int i;
	LPTDRMETALIB pstLib;
	LPTDRMETA pstMeta;

	if( a_pszLibName && a_pszLibName[0] )
	{
		pstLib	=	NULL;
		
		for(i=0; i<a_pstLibListInst->iCount; i++)
		{
			if( strcasecmp(a_pstLibListInst->astInsts[i].szName, a_pszLibName) )
				continue;

			pstLib	=	a_pstLibListInst->astInsts[i].pstLib;

			pstMeta	=	tdr_get_meta_by_name(pstLib, a_pszMeta);

			if( !pstMeta )
				continue;

			if( a_ppstFind )
			{
				*a_ppstFind	=	pstMeta;
			}

			return 0;
		}
	}
	else
	{
		for(i=0; i<a_pstLibListInst->iCount; i++)
		{
			pstLib	=	a_pstLibListInst->astInsts[i].pstLib;

			pstMeta	=	tdr_get_meta_by_name(pstLib, a_pszMeta);

			if( !pstMeta )
				continue;

			if( a_ppstFind )
			{
				*a_ppstFind	=	pstMeta;
			}

			return 0;
		}
	}

	return -1;
}

int tdump_init_maplist(TDUMPMAPLISTINST* a_pstMapListInst, TDUMPLIBLISTINST* a_pstLibListInst, TDUMPMAPLIST* a_pstMapList)
{
	TDUMPMAP* pstMap;
	TDUMPMAPINST* pstMapInst;

	a_pstMapListInst->iCount	=	0;

	while(a_pstMapListInst->iCount<a_pstMapList->iCount)
	{
		pstMap		=	a_pstMapList->astMaps + a_pstMapListInst->iCount;
		pstMapInst	=	a_pstMapListInst->astInsts + a_pstMapListInst->iCount;

		if( tdump_find_meta(a_pstLibListInst, pstMap->szMeta, pstMap->szLib, &pstMapInst->pstMeta)<0 )
		{
			return -1;
		}

		a_pstMapListInst->iCount++;
	}

	return 0;
}

int tdump_init_wrapper(TDUMPWRAPPERINST* a_pstWrapperInst, TDUMPLIBLISTINST* a_pstLibListInst, TDUMPWRAPPER* a_pstWrapper)
{
	TDUMPWRAPPER stDefault;
	TDRSIZEINFO* pstSizeInfo;
	LPTDRMETA pstMeta=NULL;
	int iCount;

	memset(a_pstWrapperInst, 0, sizeof(*a_pstWrapperInst));

	if( '\0'==a_pstWrapper->szMeta[0] )
	{

		memset(&stDefault, 0, sizeof(stDefault));

		strcpy(stDefault.szMeta, "TLOGBinPkg");
		strcpy(stDefault.szHeadLen, "Head.HeadLen");
		strcpy(stDefault.szBodyLen, "Head.BodyLen");
		strcpy(stDefault.szMsgID, "Head.ID");
		strcpy(stDefault.szMsgCls, "Head.Cls");
		strcpy(stDefault.szMsgTime, "Head.Time.sec");
		strcpy(stDefault.szMsgType, "Head.Type");
		strcpy(stDefault.szMsgVer, "Head.BodyVer");

		a_pstWrapper	=	&stDefault;

		pstMeta	=	tdr_get_meta_by_name((LPTDRMETALIB)tlog_get_meta_data(), a_pstWrapper->szMeta);
		a_pstWrapperInst->pstMeta	=	pstMeta;
	}
	else
	{
		tdump_find_meta(a_pstLibListInst, a_pstWrapper->szMeta, a_pstWrapper->szLib, &pstMeta);
		a_pstWrapperInst->pstMeta	=	pstMeta;
	}

	if( !a_pstWrapperInst->pstMeta )
		return -1;

	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stHeadLen, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szHeadLen);
	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stBodyLen, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szBodyLen);
	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stPkgLen, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szPkgLen);
	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stMsgID, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szMsgID);
	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stMsgCls, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szMsgCls);
	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stMsgTime, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szMsgTime);
	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stMsgType, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szMsgType);
	tdr_sizeinfo_to_off_i(&a_pstWrapperInst->stMsgVer, pstMeta, TDR_INVALID_INDEX, a_pstWrapper->szMsgVer);

	iCount	=	0;
	if( a_pstWrapperInst->stHeadLen.iUnitSize ) iCount++;
	if( a_pstWrapperInst->stBodyLen.iUnitSize ) iCount++;
	if( a_pstWrapperInst->stPkgLen.iUnitSize ) iCount++;

	if( 2!=iCount )
		return -1;

	pstSizeInfo	=	&a_pstWrapperInst->stHeadLen;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	pstSizeInfo	=	&a_pstWrapperInst->stBodyLen;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	pstSizeInfo	=	&a_pstWrapperInst->stPkgLen;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	pstSizeInfo	=	&a_pstWrapperInst->stMsgID;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	pstSizeInfo	=	&a_pstWrapperInst->stMsgCls;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	pstSizeInfo	=	&a_pstWrapperInst->stMsgTime;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	pstSizeInfo	=	&a_pstWrapperInst->stMsgType;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	pstSizeInfo	=	&a_pstWrapperInst->stMsgVer;
	if( pstSizeInfo->iUnitSize )
	{
		if( a_pstWrapperInst->iMinHeadLen < pstSizeInfo->iNOff + pstSizeInfo->iUnitSize )
			a_pstWrapperInst->iMinHeadLen	=	pstSizeInfo->iNOff + pstSizeInfo->iUnitSize;
	}

	return 0;
}

int tdump_init_conf(TDUMPCONF* a_pstConf, const char* a_pszPath)
{
	LPTDRMETALIB pstLib;
	LPTDRMETA pstMeta;
	TDRDATA stHost;

	pstLib	=	(LPTDRMETALIB) g_szMetalib_tdump;

	pstMeta	=	tdr_get_meta_by_name(pstLib, "TDumpConf");

	if( !pstMeta )
		return -1;

	stHost.pszBuff	=	(char*) a_pstConf;
	stHost.iBuff	=	(int)sizeof(*a_pstConf);

	return tdr_input_file(pstMeta,&stHost, a_pszPath, 0, 0);
}


int tdump_showhelp(int argc, char* argv[])
{
	assert(argc>=1);

	printf("Usage: %s [-l once limit] [-L total limit] [-t starttime] [-T lastsec] [-f config] [-g] [-h] [ [-H]|[-B]|[-A] ] file1 file2 ... \n", basename(argv[0]) );

	printf("       -h: Show this help screen.\n");
	printf("       -f: Specify config file, default is \'tlog.xml\'.\n");
	printf("       -g: Use this process to genarate config template, the following files (file1 file2 ...) will be used for writting template.\n");
	printf("       -l: Specify the max msg in each file to be processed.\n");
	printf("       -L: Specify the max msg total to be processed.\n");
	printf("       -t: Specify the start time for filtering msg, format is \"%s\".\n", TDUMP_DEF_TIME_FORMAT);
	printf("       -T: Specify the last sec for filtering msg.\n");
	printf("       -H: Only dump the head of the msg.\n");
	printf("       -B: Only dump the body of the msg.\n");
	printf("       -A: Dump the whole msg.\n");

	return 0;
}

int tdump_proc_msg_once(TDUMPCTX* pstCtx)
{
	LPTDUMPWRAPPERINST pstWrapperInst;
	TDRSIZEINFO* pstSizeInfo;
	char* pszData;
	LPTDRMETA pstMsgMeta;
	char* pszMsg;

	TDRDATA stHost;
	TDRDATA stNet;

	int iMsgID;
	int iMsgCls;
	int iMsgTime;
	int iMsgType;
	int iMsgVer;
	int iHeadLen;
	int iBodyLen;;
	int iPkgLen;

	pstWrapperInst	=	&pstCtx->stConfInst.stWrapper;

	if( pstCtx->iData < pstWrapperInst->iMinHeadLen )
		return -1;

	pszMsg	=	pstCtx->pszBuff + pstCtx->iOff;

	pstSizeInfo	=	&pstWrapperInst->stHeadLen;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iHeadLen, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iHeadLen	=	0;
	}

	pstSizeInfo	=	&pstWrapperInst->stBodyLen;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iBodyLen, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iBodyLen	=	0;
	}

	pstSizeInfo	=	&pstWrapperInst->stPkgLen;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iPkgLen, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iPkgLen	=	0;
	}

	pstSizeInfo	=	&pstWrapperInst->stMsgID;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iMsgID, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iMsgID	=	0;
	}

	pstSizeInfo	=	&pstWrapperInst->stMsgCls;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iMsgCls, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iMsgCls	=	0;
	}

	pstSizeInfo	=	&pstWrapperInst->stMsgTime;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iMsgTime, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iMsgTime	=	0;
	}

	pstSizeInfo	=	&pstWrapperInst->stMsgType;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iMsgType, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iMsgType	=	0;
	}

	pstSizeInfo	=	&pstWrapperInst->stMsgVer;
	if( pstSizeInfo->iUnitSize )
	{
		pszData	=	pszMsg + pstSizeInfo->iNOff;
		TDR_GET_INT_NET(iMsgVer, pstSizeInfo->iUnitSize, pszData);
	}
	else
	{
		iMsgVer	=	0;
	}

	if( 0==pstWrapperInst->stHeadLen.iUnitSize )
		iHeadLen	=	iPkgLen - iBodyLen;
	else if( 0==pstWrapperInst->stBodyLen.iUnitSize )
		iBodyLen	=	iPkgLen - iHeadLen;
	else
		iPkgLen		=	iHeadLen + iBodyLen;

	if( pstCtx->iData<iPkgLen )
		return -1;

	pstCtx->iOff	+=	iPkgLen;
	pstCtx->iData	-=	iPkgLen;

	if( 0!=tlogfilter_match_vec(&pstCtx->pstConf->stFilter, iMsgID, iMsgCls) )
		return 0;

	/* dump this message. */

	pstMsgMeta	=	tdump_type_to_meta(&pstCtx->stConfInst.stMapList, iMsgType);

	if( pstMsgMeta )
	{
		if( pstCtx->iDumpHead )
		{
			stNet.pszBuff	=	pszMsg;
			stNet.iBuff	=	iHeadLen;

			stHost.pszBuff	=	pstCtx->pszHost;
			stHost.iBuff	=	pstCtx->iHost;

			tdr_ntoh(pstWrapperInst->pstMeta, &stHost, &stNet, iMsgVer);

			tdr_output_fp(pstWrapperInst->pstMeta, stdout, &stHost, 0, 0);
		}

		if( pstCtx->iDumpBody )
		{
			stNet.pszBuff	=	pszMsg + iHeadLen;
			stNet.iBuff	=	iBodyLen;

			stHost.pszBuff	=	pstCtx->pszHost;
			stHost.iBuff	=	pstCtx->iHost;

			tdr_ntoh(pstMsgMeta, &stHost, &stNet, iMsgVer);

			tdr_output_fp(pstMsgMeta, stdout, &stHost, 0, 0);
		}
	}
	else
	{
		if( pstCtx->iDumpBody )
		{
			stNet.pszBuff	=	pszMsg;
			stNet.iBuff	=	iPkgLen;
		}
		else
		{
			stNet.pszBuff	=	pszMsg;
			stNet.iBuff	=	iHeadLen;
		}

		stHost.pszBuff	=	pstCtx->pszHost;
		stHost.iBuff	=	pstCtx->iHost;

		tdr_ntoh(pstWrapperInst->pstMeta, &stHost, &stNet, iMsgVer);

		tdr_output_fp(pstWrapperInst->pstMeta, stdout, &stHost, 0, 0);
	}

	fprintf(stdout, "\n");

	return 1;
}

int tdump_proc_msg_batch(TDUMPCTX* pstCtx, FILE* fp)
{
	int iBuff;
	int iRead;
	int iRet=0;

	iBuff	=	pstCtx->iBuff - pstCtx->iData;

	iRead	=	fread(pstCtx->pszBuff+pstCtx->iData, 1, iBuff, fp);

	if( iRead<0 )
		return -1;

	pstCtx->iData +=	iRead;

	while( 1 )
	{
		iRet	=	tdump_proc_msg_once(pstCtx);

		if( iRet<0 )
			break;

		pstCtx->iTotalMsg++;
		pstCtx->iThisMsg++;

		pstCtx->iTotalDump	+=	iRet;
		pstCtx->iThisDump	+=	iRet;

		if( pstCtx->iThisMsg >= pstCtx->iThisLimit )
			break;
	}

	if( pstCtx->iData>0 )
	{
		memmove(pstCtx->pszBuff, pstCtx->pszBuff+pstCtx->iOff, pstCtx->iData);
	}

	pstCtx->iOff	=	0;

	return 0;
}

int tdump_proc_file(TDUMPCTX* pstCtx, const char* pszFile)
{
	FILE* fp;
	int iRet;
	struct stat stStat;
	struct timeval stStart;
	struct timeval stEnd;
	int iMs;

	pstCtx->iThisMsg	=	0;
	pstCtx->iThisDump	=	0;

	pstCtx->iOff		=	0;
	pstCtx->iData		=	0;

	fp	=	fopen(pszFile, "rb");	
	if( !fp )
	{
		fprintf(stderr, "Err: Open file \'%s\' failed, errno=%d(%s)\n", pszFile, errno, strerror(errno));
		return -1;
	}

	gettimeofday(&stStart, NULL);

	fstat(fileno(fp), &stStat);

	fprintf(stderr, "Info: Process file \'%s\' ...... \n", pszFile);

	setvbuf(fp, gs_szBuff, _IOFBF, sizeof(gs_szBuff));

	while( 1 )
	{
		if( ftell(fp)>=stStat.st_size )
			break;

		iRet	=	tdump_proc_msg_batch(pstCtx, fp);

		if( iRet<0 )
			break;

		if( pstCtx->iThisMsg >= pstCtx->iThisLimit )
			break;

		if( feof(fp) )
			break;
	}

	gettimeofday(&stEnd, NULL);

	iMs	=	(stEnd.tv_sec - stStart.tv_sec) * 1000;
	iMs	+=	(stEnd.tv_usec - stStart.tv_usec)/1000;

	if( iRet<0 )
		fprintf(stderr, "      Failed, Use %.3f s, Total %d msg, Dump %d msg\n", iMs/1000.0f, pstCtx->iThisMsg, pstCtx->iThisDump);
	else
		fprintf(stderr, "      Success, Use %.3f s, Total %d msg, Dump %d msg\n", iMs/1000.0f, pstCtx->iThisMsg, pstCtx->iThisDump);

	return iRet;
}


int tdump_gen_template(char* pszPath)
{
	LPTDRMETALIB pstLib;
	LPTDRMETA pstMeta;
	TDUMPCONF stConf;
	TDRDATA stHost;

	pstLib	=	(LPTDRMETALIB) g_szMetalib_tdump;

	pstMeta	=	tdr_get_meta_by_name(pstLib, "TDumpConf");

	if( !pstMeta )
		return -1;

	memset(&stConf, 0, sizeof(stConf));

	stConf.stLibList.iCount	=	1;
	stConf.stMapList.iCount	=	1;
	stConf.stFilter.iCount	=	1;

	stHost.pszBuff	=	(char*)&stConf;
	stHost.iBuff	=	(int)sizeof(stConf);

	if( pszPath && pszPath[0] )
		return tdr_output_file(pstMeta, pszPath, &stHost, 0, 0);
	else
		return tdr_output_fp(pstMeta, stdout, &stHost, 0, 0);
}

time_t tdump_time(const char* pszStr)
{
	struct tm stTm;

	strptime(pszStr, TDUMP_DEF_TIME_FORMAT, &stTm);

	return mktime(&stTm);
}

int main(int argc, char* argv[])
{
	char* pszPath = "tdump.xml";
	int opt;
	int i;
	int fGenTemplate=0;
	int iTotalLimit=TDUMP_MAX_MSG;
	int iOnceLimit=TDUMP_MAX_MSG;
	int iRet;
	time_t tStartTime=0;
	int iLastSec=0;
	struct timeval stStart;
	struct timeval stEnd;
	int iMs;

	memset(&gs_stCtx, 0, sizeof(gs_stCtx));

	gs_stCtx.pstConf	=	&gs_stConf;

	gs_stCtx.iDumpHead	=	1;
	gs_stCtx.iDumpBody	=	1;

	while( -1 != (opt=getopt(argc, argv, "l:L:t:T:f:ghHBA")) )
	{
		switch(opt)
		{
		case 'h':
			tdump_showhelp(argc, argv);
			return 0;
		case 'f':
			pszPath	=	optarg;
			break;
		case 'g':
			fGenTemplate	=	1;
			break;
		case 'l':
			iOnceLimit	=	strtol(optarg, NULL, 0);
			break;
		case 'L':
			iTotalLimit	=	strtol(optarg, NULL, 0);
			break;
		case 't':
			tStartTime	=	tdump_time(optarg);
			break;
		case 'T':
			iLastSec	=	strtol(optarg, NULL, 0);
			break;
		case 'H':
			gs_stCtx.iDumpHead	=	1;
			gs_stCtx.iDumpBody	=	0;
			break;
		case 'B':
			gs_stCtx.iDumpHead	=	0;
			gs_stCtx.iDumpBody	=	1;
			break;
		case 'A':
			gs_stCtx.iDumpHead	=	1;
			gs_stCtx.iDumpBody	=	1;
			break;
		default:
			break;
		}
	}

	if( fGenTemplate )
	{
		return tdump_gen_template(pszPath);
	}

	if( optind==argc )
	{
		tdump_showhelp(argc, argv);
		return 0;
	}

	if( tdump_init_conf(&gs_stConf, pszPath)<0 )
	{
		fprintf(stderr, "Err: Can not init config file \'%s\'\n", pszPath);
		return -1;
	}


	if( TDUMP_MAX_MSG!=iTotalLimit )
		gs_stConf.iTotalLimit	=	iTotalLimit;

	if( 0==gs_stConf.iTotalLimit )
		gs_stConf.iTotalLimit	=	TDUMP_MAX_MSG;

	if( TDUMP_MAX_MSG!=iOnceLimit )
		gs_stConf.iOnceLimit	=	iOnceLimit;

	if( 0==gs_stConf.iOnceLimit )
		gs_stConf.iOnceLimit	=	TDUMP_MAX_MSG;

	if( 0!=tStartTime )
	{
		gs_stConf.tStartTime	=	tStartTime;
		gs_stConf.iLastSec	=	iLastSec;
	}

	iRet	=	tdump_init_liblist(&gs_stCtx.stConfInst.stLibList, &gs_stConf.stLibList);
	if( iRet<0 )
	{
		fprintf(stderr, "Err: Can not init liblist\n");
		return -1;
	}


	iRet	=	tdump_init_maplist(&gs_stCtx.stConfInst.stMapList, &gs_stCtx.stConfInst.stLibList, &gs_stConf.stMapList);
	if( iRet<0 )
	{
		fprintf(stderr, "Err: Can not init maplist\n");
		return -1;
	}

	iRet	=	tdump_init_wrapper(&gs_stCtx.stConfInst.stWrapper, &gs_stCtx.stConfInst.stLibList, &gs_stConf.stWrapper);
	if( iRet<0 )
	{
		fprintf(stderr, "Err: Can not init wrapper\n");
		return -1;
	}

	gs_stCtx.iBuff			=	tdr_get_meta_size(gs_stCtx.stConfInst.stWrapper.pstMeta);
	gs_stCtx.pszBuff		=	calloc(1, gs_stCtx.iBuff);
	gs_stCtx.iHost			=	gs_stCtx.iBuff;
	gs_stCtx.pszHost		=	calloc(1, gs_stCtx.iHost);

	if( !gs_stCtx.pszBuff || !gs_stCtx.pszHost )
	{
		fprintf(stderr, "Err: Can not alloc buffer size %d bytes\n", gs_stCtx.iBuff);
		return -1;
	}

	fprintf(stderr, "Info: Begin process files ...\n");

	gettimeofday(&stStart, NULL);

	for(i=optind; i<argc; i++)
	{
		gs_stCtx.iThisLimit	=	gs_stConf.iOnceLimit;
		if( gs_stCtx.iThisLimit > gs_stConf.iTotalLimit - gs_stCtx.iTotalMsg )
			gs_stCtx.iThisLimit	=	 gs_stConf.iTotalLimit - gs_stCtx.iTotalMsg;

		if( gs_stCtx.iThisLimit<=0 )
			break;

		iRet	=	tdump_proc_file(&gs_stCtx, argv[i]);
	}

	gettimeofday(&stEnd, NULL);

	iMs	=	(stEnd.tv_sec - stStart.tv_sec) * 1000;
	iMs	+=	(stEnd.tv_usec - stStart.tv_usec)/1000;

	fprintf(stderr, "Info: End process, Use %.3f s, Total read %d msg, dump %d msg\n", iMs/1000.0f, gs_stCtx.iTotalMsg, gs_stCtx.iTotalDump);

	return 0;
}


