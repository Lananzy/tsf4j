/*
**  @file $RCSfile: tdump.h,v $
**  general description of this module
**  $Id: tdump.h,v 1.1 2008/08/19 03:40:37 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/08/19 03:40:37 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TDUMP_H
#define TDUMP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "tdumpconf.h"

#define TDUMP_MAX_MSG			0x7fffffff
#define TDUMP_DEF_TIME_FORMAT		"%Y-%m-%d %H:%M:%S"

struct tagTDumpLibInst
{
	char szName[TDUMP_NAME_LEN];
	LPTDRMETALIB pstLib;
};

typedef struct tagTDumpLibInst		TDUMPLIBINST;
typedef struct tagTDumpLibInst		*LPTDUMPLIBINST;

struct tagTDumpLibListInst
{
	int iCount;
	TDUMPLIBINST astInsts[TDUMP_MAX_LIB];
};

typedef struct tagTDumpLibListInst	TDUMPLIBLISTINST;
typedef struct tagTDumpLibListInst	*LPTDUMPLIBLISTINST;

struct tagTDumpMapInst
{
	char szName[TDUMP_NAME_LEN];
	int iType;
	LPTDRMETA pstMeta;
};

typedef struct tagTDumpMapInst		TDUMPMAPINST;
typedef struct tagTDumpMapInst		*LPTDUMPMAPINST;

struct tagTDumpMapListInst
{
	int iCount;
	TDUMPMAPINST astInsts[TDUMP_MAX_MAP];
};

typedef struct tagTDumpMapListInst	TDUMPMAPLISTINST;
typedef struct tagTDumpMapListInst	*LPTDUMPMAPLISTINST;

struct tagTDumpWrapperInst
{
	LPTDRMETA pstMeta;
	int iMinHeadLen;
	TDRSIZEINFO stHeadLen;
	TDRSIZEINFO stBodyLen;
	TDRSIZEINFO stPkgLen;
	TDRSIZEINFO stMsgID;
	TDRSIZEINFO stMsgCls;
	TDRSIZEINFO stMsgTime;
	TDRSIZEINFO stMsgType;
	TDRSIZEINFO stMsgVer;
};

typedef struct tagTDumpWrapperInst	TDUMPWRAPPERINST;
typedef struct tagTDumpWrapperInst	*LPTDUMPWRAPPERINST;

struct tagTDumpConfInst
{
	TDUMPLIBLISTINST stLibList;
	TDUMPMAPLISTINST stMapList;
	TDUMPWRAPPERINST stWrapper;
};

typedef struct tagTDumpConfInst		TDUMPCONFINST;
typedef struct tagTDumpConfInst		*LPTDUMPCONFINST;

struct tagTDumpCtx
{
	TDUMPCONF* pstConf;
	int iTotalMsg;
	int iTotalDump;
	int iThisMsg;
	int iThisDump;
	int iThisLimit;
	char* pszHost;
	int iHost;
	char* pszBuff;
	int iBuff;
	int iOff;
	int iData;
	int iDumpHead;
	int iDumpBody;
	TDUMPCONFINST stConfInst;
};

typedef struct tagTDumpCtx		TDUMPCTX;
typedef struct tagTDumpCtx		*LPTDUMPCTX;

LPTDRMETA tdump_type_to_meta(TDUMPMAPLISTINST* a_pstMapListInst, int a_iType);

int tdump_init_liblist(TDUMPLIBLISTINST* a_pstLibListInst, TDUMPLIBLIST* a_pstLibList);
int tdump_init_maplist(TDUMPMAPLISTINST* a_pstMapListInst, TDUMPLIBLISTINST* a_pstLibListInst, TDUMPMAPLIST* a_pstMapList);
int tdump_init_wrapper(TDUMPWRAPPERINST* a_pstWrapperInst, TDUMPLIBLISTINST* a_pstLibListInst, TDUMPWRAPPER* a_pstWrapper);

int tdump_init_conf(TDUMPCONF* a_pstConf, const char* a_pszPath);

#ifdef __cplusplus
}
#endif

#endif /* TDUMP_H */


