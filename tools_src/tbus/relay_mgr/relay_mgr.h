#ifndef TRELAY_MGR_H 
#define TRELAY_MGR_H

#include "tbus/tbus_macros.h"
#include "tbus/tbus_desc.h"

#ifdef __cplusplus
extern "C" {
#endif



#define TRELAY_MGR_PATH_LEN 512
#define TRELAY_MGR_DEFAULT_SEE_MAX_NUM  10

enum tagTrelayMgrOp
{
	TRELAY_MGR_OP_WRITE = 1,
	TRELAY_MGR_OP_DELETE,
	TRELAY_MGR_OP_LIST,
};

struct tagTrelaymgrOptions 
{
	char szConfFile[TRELAY_MGR_PATH_LEN];
	int iMgrOP;
	int iDelID;
	char szShmKey[TBUS_MAX_KEY_STRING_LEN];	
};



typedef struct tagTrelaymgrOptions  TRELAYMGROPTIONS;
typedef struct tagTrelaymgrOptions  *LPTRELAYMGROPTIONS;

#ifdef __cplusplus
}
#endif


#endif
