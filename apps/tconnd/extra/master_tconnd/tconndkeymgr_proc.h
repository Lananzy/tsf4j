/*
**  @file $RCSfile: tconndkeymgr_proc.h,v $
**  general description of this module
**  $Id: tconndkeymgr_proc.h,v 1.1 2009/01/23 01:51:46 sean Exp $
**  @author $Author: sean $
**  @date $Date: 2009/01/23 01:51:46 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef _TCONNDKEYMGR_DESC_H
#define _TCONNDKEYMGR_DESC_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	TCONND_MAX_BUSIUNIT	256

#include "tapp/tapp.h"

#include "pal/queue.h"
#include "pal/tree.h"

#include "tconnddef.h"
#include "tconndkeymgr_conn.h"

typedef struct
{
	TCONNDKEYMGR*	pstTconndKeyMgr;
	SERVER*			pstServer;

	int				iHandle;	
	int				iDst;
	
} RSYSENV, *LPRSYSENV;

int process_0x30(LPRSYSENV pstEnv, SERVER *pstSrv, tagPkgHead* pstHead, TCONNDINNERMSG* pstInnerMsg);
int process_0x31 	(TAPPCTX* pstAppCtx, SERVER *pstSrv, unsigned int uiBusid);
int process_0x32 	(TAPPCTX* pstAppCtx, SERVER *pstSrv, unsigned int uiBusid);

int readXMLData(TAPPCTX* pstAppCtx, SERVER* pstSrv);
int writeXMLData(TAPPCTX* pstAppCtx, SERVER* pstSrv);

#ifdef __cplusplus
}
#endif

#endif /* _TCONNDKEYMGR_PROC_H */
