/*
**  @file $RCSfile: tconndkeymgr_conn.h,v $
**  general description of this module
**  $Id: tconndkeymgr_conn.h,v 1.1 2009/01/23 01:51:45 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/01/23 01:51:45 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef _TCONNDKEYMGR_CONN_H_
#define _TCONNDKEYMGR_CONN_H_

#ifdef __cplusplus
extern "C" 
{
#endif

#include "pal/pal.h"
#include "pal/tree.h"
#include "pal/queue.h"
#include "tapp/tapp.h"

//#include "tconndkeymgr_desc.h"
#include "tconnddef.h"

struct PDUNode;

typedef struct tconndNode
{
	unsigned	int 	iBusid;
	
	TAILQ_ENTRY(tconndNode)		next;
	RB_HEAD(PDUTree, PDUNode)	pduNodeRoot;
} tconndNode;

typedef struct  PDUNode
{
	SVRSKEYDATA		stSvrsKey;
	RB_ENTRY(PDUNode)	entry;
	
} PDUNode;

typedef struct connection
{
	int 			iSock;
	unsigned int 	iTimeout;
	
	char 		szBuffRead[4096+1];
	
	int 			iBytesRead;
	int 			iBytesSend;
	
	TAILQ_ENTRY(connection)	next;

} CONNECTION, *LPCONNECTION;

typedef struct server
{
	int 		srvfd;
	fd_set 	master_set;
	
	LPTCONNDKEYMGR			pstConfinst;
	TAILQ_HEAD(,connection)	stConnQueue;	//链接的链表
	TAILQ_HEAD(,tconndNode) 	stTcndQueue;	//tconnd的链表

	TAPPDATA				stBusidList;
	TAPPDATA				stSvrskeyList;

	LPTDRMETA				pstMeta;
	
} SERVER, *LPSERVER;

extern int tconndkeymgr_connection_init(SERVER *pstSrv);
extern int tconndkeymgr_connection_poll(SERVER *pstSrv);
extern void tconndkeymgr_connection_free(SERVER *pstSrv);

#ifdef __cplusplus
}
#endif

#endif /* _TCONNDKEYMGR_CONN_H_ */
