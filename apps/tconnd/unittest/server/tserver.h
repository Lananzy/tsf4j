#ifndef _TSERVER_H_
#define _TSERVER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "../protocol/protocol.h"
#include "tplayer.h"
#include "pal/pal.h"
#include "tdr/tdr.h"
#include "apps/tconnapi/tframehead.h"
#include "apps/tconnapi/tconnapi.h"
#include "tapp/tapp.h"
#include "tlog/tlog.h"


#define MSG_BUFFER_SIZE 65536
#define CSPKGNAME                     "CSPKG"
#define MAX_ECHOFACTOR  10

struct tagTServerEnv
{
       TMEMPOOL *pstPlayerPool;
	int iConHandle;
	int iConndID;
	int iEchoFactor;
	TLOGCATEGORYINST* pstLog;

	LPTDRMETA pstCSPkgMeta;	/*meta of cs pkg meta*/
	char szBuffer[MSG_BUFFER_SIZE]; 
	CSPKG stpkg;

	int iNoEncrypt;
};

typedef struct tagTServerEnv	TSERVERENV;
typedef struct tagTServerEnv	*LPTSERVERENV;




int tserver_init(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv);


int tserver_fini(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv);


int tserver_tick(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv);


int tserver_proc(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv);


int tserver_def_opt(TAPPCTX* pstAppCtx, TSERVERENV* pstEnv);


int tserver_deal_client_msg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,char *pszbuff,int ibuff);

int tserver_deal_client_pkg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,CSPKG *pstPkg);

int tserver_deal_hello_pkg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,CSPKG *pstPkg);

int tserver_send_client_pkg(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame,CSPKG *pstPkg);


int tplayer_open_connection(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame);


int tplayer_close_connection(TSERVERENV* pstEnv,TFRAMEHEAD *pstFrame);





#ifdef __cplusplus
}
#endif

#endif

