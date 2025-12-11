/*
**  @file $RCSfile: tipc.c,v $
**  general description of this module
**  $Id: tipc.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/tstring.h"
#include "pal/tstdio.h"
#include "pal/tipc.h"
#include "pal/terrno.h"

#include <assert.h>

#ifdef WIN32 

#pragma warning(disable:4996)

#endif /* WIN32 */

#ifdef WIN32

static unsigned int calc_token_i(const char* pszText)
{
	unsigned int uiCode	=	0;

	while(pszText[0])
	{
		uiCode	+=	(uiCode<<5) + pszText[0];
		pszText++;
	}

	return uiCode;
}

key_t ftok(const char *pathname, int proj_id)
{
	char szPath[MAX_PATH];
	int iToken;

	if( !_fullpath(szPath, pathname, sizeof(szPath)) )
	{
		errno	=	EBADF;
		return -1;
	}

	iToken	=	(int)calc_token_i(szPath);

	return (iToken<<8) | (proj_id & 0xff);
}

#endif /* WIN32 */

int tipc_path2name(char* a_pszBuff, int a_iBuff, const char* a_pszPath, const char* a_pszPrefix)
{
	int i;
	int iLen;
	char* pszPtr;
	int iPrefix;
	int iPath;
#ifdef WIN32
	int iIsPath;
#endif
	
	assert( a_pszBuff && a_pszPath && a_iBuff>0 );

	a_pszBuff[0]	=	0;

#ifdef WIN32 
	
	iPath	=	(int) GetShortPathName(a_pszPath, NULL, 0);

	if( 0==iPath )
	{
		iIsPath	=	0;
		iPath	=	strlen(a_pszPath);
	}
	else
	{
		iIsPath	=	1;
	}
#else
	iPath	=	strlen(a_pszPath);
#endif

	if( 0==iPath )
		return -1;

	if( (char*)0==a_pszPrefix )
		iPrefix	=	0;
	else
		iPrefix	=	strlen(a_pszPrefix);

	iLen	=	iPath + iPrefix;

	if( iLen>=a_iBuff )
		return -1;

	if( (char*)0==a_pszPrefix )
		pszPtr	=	a_pszBuff;
	else
	{
		strcpy(a_pszBuff, a_pszPrefix);
		pszPtr	=	a_pszBuff + iPrefix;
	}

#ifdef WIN32 
	if( iIsPath )
	{
		if(0==GetShortPathName(a_pszPath, pszPtr, a_iBuff-iPrefix))
			return -1;
	}
	else
	{
		strcpy(pszPtr, a_pszPath);
	}
#else
	strcpy(pszPtr, a_pszPath);
#endif
	/* do mapping. */
	i	=	0;

	while(pszPtr[i])
	{
		if(TIPC_MAP_CHAR_FROM==pszPtr[i] )
			pszPtr[i]	=	TIPC_MAP_CHAR_TO;
		else
			pszPtr[i]	=	tolower(pszPtr[i]);

		i++;
	}

	pszPtr[i]	=	0;

	return 0;
}

int tipc_key2name(char* a_pszBuff, int a_iBuff, key_t a_iKey)
{
	int iRet;

	if( !a_pszBuff || a_iBuff<=0 )
	{
		errno	=	EINVAL;
		return -1;
	}

	iRet	=	snprintf(a_pszBuff, a_iBuff, TIPC_PATTERN, a_iKey);

	if( iRet<0 || iRet>=a_iBuff )
	{
		errno	=	ENOMEM;
		return -1;
	}

	return 0;
}

