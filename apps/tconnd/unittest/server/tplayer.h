#ifndef _TPLAYER_H_
#define  _TPLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "comm/tmempool.h"
#include "apps/tconnapi/tframehead.h"

#define  PLAYER_ONLINE 5000

struct tagPlayer;

typedef struct tagPlayer Player;
typedef struct tagPlayer *LPPlayer;


struct tagPlayer
{
    int uin;
    int iID;
    TFRAMEHEAD stFrame;
};

int tplayer_init(TMEMPOOL** ppstPool);

int tplayer_fini(TMEMPOOL** ppstPool);

LPPlayer tplayer_get(TMEMPOOL* pstPool, int iIdx);

int tplayer_free(TMEMPOOL* pstPool, int iIdx);

int tplayer_alloc(TMEMPOOL* pstPool);


#ifdef __cplusplus
}
#endif


#endif










