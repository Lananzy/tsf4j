#ifndef _TCLIENT_H
#define  _TCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pal/pal.h"
#include "tdr/tdr.h"
#include "tapp/tapp.h"
#include "tlog/tlog.h"
#include "apps/tcltapi/tqqapi.h"
#include "tclient_def.h"
#include "../protocol/protocol.h"
#include "comm/tmempool.h"

#define METANAME  "CSPKG"

#define RECVBUFF          204800
#define SENDBUFF          204800
#define RECVTIMEOUT    2000
#define CONNTIMEOUT   2000
#define INTERVAL   10

#define MAX_EPOLL_EVENTS 1024


#define MAX_ONLINE       1000
#define MIN_ONLINE        1

#define MAX_SPEED         30
#define MIN_SPEED          1

#define MAX_SIZE            2048
#define MIN_SIZE             32

struct tagTClientEnv
{
	LPTDRMETA pstMeta;
	int  iepfd;

	
	//int  iMilliRecv;
	//int iMilliSend;
	int iSlipCount;
	int iSlipPos;
	int iPackageSize;
	int iMaxOnline;

	
       int iMaxSpeed;
	int iSpeedFactor;

	int iLastRecvSucc;
	int iLastSendSucc;
	
       int iTotalRecvSucc;
	int iTotalSendSucc;

	TMEMPOOL *pstClientPool;
	

	
	TLOGCATEGORYINST* pstLog;
	LPCLIENT_DEF pstConf;
	
	
};

typedef struct tagTClientEnv	TClientEnv;
typedef struct tagTClientEnv	*LPTClientEnv;


struct tagTClientInst
{
     int iID;
     HTQQAPI   hqqapi;
     TPDUHEAD   stHead;
     CSPKG    stRecvPDU;
     CSPKG    stSendPDU;
     
};

typedef struct tagTClientInst    TClientInst;
typedef struct tagTClientInst   *LPTClientInst;




int tclient_init(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);
int tclient_fini(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);
int tclient_tick(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);
int tclient_proc(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);

int tclient_def_opt(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);


int tclient_proc_recv(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);

int tclient_proc_send(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);


int tclient_login_player(TAPPCTX* pstAppCtx, TClientEnv* pstEnv);

int tclient_init_qqapi(TClientEnv* pstEnv,TClientInst *pstClientInst);

int tclient_proc_recv_packge(TClientEnv* pstEnv,TClientInst *pstClientInst);

int tclient_proc_send_packge(TClientEnv* pstEnv,TClientInst *pstClientInst);

#ifdef __cplusplus
}
#endif

#endif




