#ifndef TBUS_VIEW_CHANNEL_H
#define TBUS_VIEW_CHANNEL_H

#include <stdio.h>
#include "tdr/tdr.h"
#include "tbus/tbus_macros.h"
#include "tbus_mgr.h"
#include "tbus/tbus_config_mng.h"

#ifdef __cplusplus
extern "C" {
#endif


struct tagViewChannelTools 
{
	LPTDRMETALIB pstLib;
	LPTDRMETA	pstMsgMeta;
	char *pszBuff;	/*存储本地数据的缓冲区*/
	int iSize;	/*缓冲区大小*/
	FILE *fpOut;
	int iMaxSeeMsg;
	int iCurSeeMsg;
};

typedef struct tagViewChannelTools  VIEWCHANNELTOOLS;
typedef struct tagViewChannelTools  *LPVIEWCHANNELTOOLS;

int ViewChannels(IN LPTBUSSHMGCIM a_pShmGCIM, LPTBUSMGROPTIONS a_pstOption);


#ifdef __cplusplus
}
#endif

#endif
