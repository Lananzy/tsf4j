/*
**  @file $RCSfile: tconnmgr.h,v $
**  general description of this module
**  $Id: tconnmgr.h,v 1.7 2009/03/16 08:48:30 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009/03/16 08:48:30 $
**  @version $Revision: 1.7 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TCONNMGR_H
#define TCONNMGR_H

#include "tdr/tdr.h"
#include "tapp/tapp.h"
#include "tlog/tlog.h"
#include "apps/tcltapi/tqqdef.h"
#include "apps/tcltapi/tpdudef.h"
#include "tconnddef.h"
#include "tconnpool.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagTDRInstList	TDRINSTLIST;
typedef struct tagTDRInstList	*LPTDRINSTLIST;

typedef struct tagPDULenTDRParserInst	PDULENTDRPARSERINST;
typedef struct tagPDULenTDRParserInst	*LPPDULENTDRPARSERINST;

typedef struct tagPDULenTHTTParserInst	PDULENTHTTPPARSERINST;
typedef struct tagPDULenTHTTParserInst	*LPPDULENTHTTPPARSERINST;

typedef struct tagPDULenNoneParserInst	PDULENNONEPARSERINST;
typedef struct tagPDULenNoneParserInst	*LPPDULENNONEPARSERINST;

typedef struct tagPDULenQQParserInst	PDULENQQPARSERINST;
typedef struct tagPDULenQQParserInst	*LPPDULENQQPARSERINST;

typedef union tagPDULenParserInst	PDULENPARSERINST;
typedef union tagPDULenParserInst	*LPPDULENPARSERINST;

typedef struct tagConfInst		CONFINST;
typedef struct tagConfInst		*LPCONFINST;

typedef struct tagTConnThread	TCONNTHREAD;
typedef struct tagTConnThread	*LPTCONNTHREAD;

typedef int (*PFNTCONND_GET_PKGLEN)(TCONNINST* pstInst, LPTCONNTHREAD pstThread);


struct tagTDRInst
{
	char szName[TCONND_NAME_LEN];
	LPTDRMETALIB pstLib;
};

typedef struct tagTDRInst		TDRINST;
typedef struct tagTDRInst		*LPTDRINST;

struct tagTDRInstList
{
	int iCount;
	TDRINST astInsts[TCONND_MAX_TDR];
};




/* 使用TDR方法来分析数据包的长度 */
struct tagPDULenTDRParserInst
{
	LPTDRMETA pstPkg;

	int iPkgLenNOff;	/*记录PDU长度成员相对PDU总结构的偏移*/
	int iPkgLenUnitSize;	/*记录PDU长度成员的存储空间*/
	int iHeadLenNOff;	/*记录PDU头部长度成员相对PDU总结构的偏移*/
	int iHeadLenUnitSize;	/*记录PDU头部长度成员的存储空间*/
	int iBodyLenNOff;	/*记录PDU消息体长度成员相对PDU总结构的偏移*/
	int iBodyLenUnitSize;	/*记录PDU消息体长度成员的存储空间*/

	int iHeadLenMultiplex;
	int iBodyLenMultiplex;
	int iPkgLenMultiplex;           
};



struct tagPDULenQQParserInst
{
	LPTDRMETA pstMetaHead;
	LPTDRMETA pstMetaGameSig;
	LPTDRMETA pstMetaSigForS2;

	//add for unify auth
	LPTDRMETA pstMetaUniSig;
       LPTDRMETA pstMetaUniEncSig;

	//add for syn
	LPTDRMETA pstMetaSyn;
	
	LPTDRMETA pstMetaUserIdent;
	LPTDRMETA pstMetaPDUIdent;
	int iRes;
	int iMaxPkgLen;
	int iHeadLenNOff;
	int iHeadLenUnitSize;
	int iBodyLenNOff;
	int iBodyLenUnitSize;
};


/* 分析协议数据单元(PDU)长度信息的数据结构，用于将数据流分解成应用定义的数据通信消息 */
union tagPDULenParserInst
{
	PDULENTDRPARSERINST stTDRParser;                     	/* PDULENPARSERID_BY_TDR,  使用TDR方法进行分析 */
	PDULENQQPARSERINST stQQParser;
};

struct tagPDUInst
{
	char szName[TCONND_NAME_LEN];
	int iUnit;
	int iUpUnit;
	int iDownUnit;
	int iMinLen;
	int iLenParsertype;                  	/*  Ver.11  Bind Macrosgroup:PDULenParserID,*/
	PDULENPARSERINST stLenParser;                     	/*  Ver.11 分析协议数据单元(PDU)长度信息的成员 */
	PFNTCONND_GET_PKGLEN pfnGetPkgLen; /*分析数据包长度的回调函数*/
	PDU* pstPDU;
};

typedef struct tagPDUInst		PDUINST;
typedef struct tagPDUInst		*LPPDUINST;

struct tagPDUInstList
{
	int iCount;
	PDUINST astInsts[TCONND_MAX_PDU];
};

typedef struct tagPDUInstList	PDUINSTLIST;
typedef struct tagPDUInstList	*LPPDUINSTLIST;

struct tagTransInst
{
	char szName[TCONND_NAME_LEN];
	//int iLoadRatio;
	int iPDULoc;
	int iLisViewerLoc;
	int iSerViewerLoc;
	int iLisCount;
	int aiLisLoc[TCONND_MAX_NETTRANS];
	int iSerCount;
	int aiSerLoc[TCONND_MAX_NETTRANS];

	int iWaitQueueHead;
	int iWaitQueueTail;
	unsigned int uiTokenAlloc;
	unsigned int uiTokenPass;

	time_t tLastActive;
	int iRes;

	int iConnPassed;
	time_t tLastConn;

	int iSendCheckInterval;
	int iPrevSendFailed;
	int iSendFailed;
	time_t tLastSendCheck;

	int iConnPermit;
	int iConnPermitLow;
	int iConnPermitHigh;
	int iConnMaxSpeed;

	int iPkgPermit;
	int iPkgPermitLow;
	int iPkgPermitHigh;
	int iPkgMaxSpeed;

	int iBytePermit;
	int iBytePermitLow;
	int iBytePermitHigh;
	int iByteMaxSpeed;

	int iRecvCheckInterval;
	int iRecvPkg;
	int iRecvByte;
	time_t tLastRecvCheck;

	CONNSTATINFO stConnInfo;

};

typedef struct tagTransInst		TRANSINST;
typedef struct tagTransInst		*LPTRANSINST;

struct tagTransInstList
{
	int iCount;
	TRANSINST astInsts[TCONND_MAX_NETTRANS];
};

typedef struct tagTransInstList	TRANSINSTLIST;
typedef struct tagTransInstList	*LPTRANSINSTLIST;

struct tagLisInst
{
	char szName[TCONND_NAME_LEN];
	int iRef;
	LISTENER* pstListener;
};

typedef struct tagLisInst		LISINST;
typedef struct tagLisInst		*LPLISINST;

struct tagLisInstList
{
	int iCount;
	LISINST astInsts[TCONND_MAX_NETTRANS];
};

typedef struct tagLisInstList	LISINSTLIST;
typedef struct tagLisInstList	*LPLISINSTLIST;


struct tagSerInst
{
	char szName[TCONND_NAME_LEN];
	int iDst;
	int iRef;
	int iRes;
	SERIALIZER * pstSerializer;
};

typedef struct tagSerInst		SERINST;
typedef struct tagSerInst		*LPSERINST;

struct tagSerInstList
{
	int iCount;
	SERINST astInsts[TCONND_MAX_NETTRANS];
};

typedef struct tagSerInstList	SERINSTLIST;
typedef struct tagSerInstList	*LPSERINSTLIST;

struct tagConfInst
{
	TDRINSTLIST stTDRInstList;
	PDUINSTLIST stPDUInstList;
	LISINSTLIST stLisInstList;
	SERINSTLIST stSerInstList;
	TRANSINSTLIST stTransInstList;
};


struct tagTConnThread
{
	int iID;
	int iIsValid;
	int iIsExit;
	int epfd;

	pthread_t tid;
	CONFINST* pstConfInst;
	TCONNPOOL* pstPool;
	int iTickCount;

       int iUpUnit;
	int iDownUnit;
	int iPkgUnit;
	int iPoolUnit;
	TAPPCTX* pstAppCtx;
	int iScanPos;

	
	char* pszSendBuff;
	int iSendBuff;

	char* pszRecvBuff;
	int iRecvBuff;
	char* pszMsgBuff;
	int iMsgBuff;

	int iLastSrc;
	int iLastDst;
	TFRAMEHEAD stFrameHead;
	TPDUHEAD stPDUHead;

	PDUINST* pstPDUInst;
	int iScanHerz;

	TLOGCATEGORYINST* pstLog;
	TLOGCATEGORYINST* pstStatLog;


	int iMsRecv;	/* the milli-second usecd to receive up message. */
	int iRecvSlices;  /* time slices to receive up message*/
	int iWaitToSend; /*byte wait to send in down channel   */
};


int tconnd_fini_tdrinstlist(TDRINSTLIST* pstTDRInstList);
int tconnd_init_tdrinstlist(TDRINSTLIST* pstTDRInstList, TDRLIST* pstTDRList);
int tconnd_find_lib(TDRINSTLIST* pstTDRInstList, const char* pszName, LPTDRMETALIB* ppstLib);
int tconnd_find_meta(TDRINSTLIST* pstTDRInstList, const char* pszName, LPTDRMETALIB pstPrefer, LPTDRMETA* ppstFind);
int tconnd_init_pduinst(TDRINSTLIST* pstTDRInstList, PDUINST* pstPDUInst, PDU* pstPDU);
int tconnd_init_pduinstlist(TDRINSTLIST* pstTDRInstList, PDUINSTLIST* pstPDUInstList, PDULIST* pstPDUList);
int tconnd_load_conffile(TCONND* pstConnd, LPTDRMETA pstMeta, const char* pszPath);
int tconnd_init_lisinstlist(LISINSTLIST* pstLisInstList, LISTENERLIST* pstListenerList);
int tconnd_fini_lisinstlist(LISINSTLIST* pstLisInstList);
int tconnd_init_serinstlist(SERINSTLIST* pstSerInstList, SERIALIZERLIST* pstSerializerList);
int tconnd_fini_serinstlist(SERINSTLIST* pstSerInstList);
int tconnd_init_transinstlist(TRANSINSTLIST* pstTransInstList, TCONND* pstConnd, LISINSTLIST* pstLisInstList, SERINSTLIST* pstSerInstList);
int tconnd_fini_confinst(CONFINST* pstConfInst);
int tconnd_init_confinst(CONFINST* pstConfInst, TCONND* pstConnd);

int tconnd_init_tconndrun(TAPPCTX* pstAppCtx, TCONND* pstConnd);

#ifdef __cplusplus
}
#endif

#endif /* TCONNMGR_H */
