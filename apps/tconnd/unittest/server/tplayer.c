#include "tplayer.h"


int tplayer_init(TMEMPOOL** ppstPool)
{
    return tmempool_new(ppstPool, PLAYER_ONLINE,sizeof(Player));
}


int tplayer_fini(TMEMPOOL** ppstPool)
{

    return tmempool_destroy(ppstPool);

}


LPPlayer tplayer_get(TMEMPOOL* pstPool, int iIdx)
{

   return (LPPlayer)tmempool_get(pstPool, iIdx);

}

int tplayer_free(TMEMPOOL* pstPool, int iIdx)
{
   return tmempool_free(pstPool, iIdx);

}

int tplayer_alloc(TMEMPOOL* pstPool)
{
  return tmempool_alloc(pstPool);

}




