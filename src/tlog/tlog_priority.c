/*
**  @file $RCSfile: tlog_priority.c,v $
**  general description of this module
**  $Id: tlog_priority.c,v 1.1 2008/08/05 07:04:59 steve Exp $
**  @author $Author: steve $
**  @date $Date: 2008/08/05 07:04:59 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/pal.h"
#include "tlog/tlog_priority_def.h"
#include "tlog/tlog_priority.h"

static const char* const gs_aszPrio[] = {
    "FATAL", 
    "ALERT",
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    "TRACE",
    "NOTSET",
    "UNKNOWN"
};

static const size_t gs_iPrio = sizeof(gs_aszPrio) / sizeof(gs_aszPrio[0]);

const char* tlog_priority_to_string(int a_iPrio)
{
	a_iPrio	/=	100;

	if ( (a_iPrio< 0) || (a_iPrio>=(int)gs_iPrio) ) 
		a_iPrio	= (int)gs_iPrio - 1;

	return gs_aszPrio[a_iPrio];
}    

int tlog_priority_to_int(const char* a_pszName)
{
	size_t i; 

	if(a_pszName)
	{
		for (i = 0; i < gs_iPrio; i++)
		{
	    		if (!strncasecmp(gs_aszPrio[i], a_pszName, strlen(gs_aszPrio[i])))
				return i * 100;
		}
	}
    
	return TLOG_PRIORITY_UNKNOWN;
}    

