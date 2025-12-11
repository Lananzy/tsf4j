/*
**  @file $RCSfile: tlogload.h,v $
**  general description of this module
**  $Id: tlogload.h,v 1.7 2008/11/12 10:09:31 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/11/12 10:09:31 $
**  @version $Revision: 1.7 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TLOGLOAD_H
#define TLOGLOAD_H

#include "tlog/tlog.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TLOG_DEF_LAYOUT_FORMAT "[%d.%u][(%f:%l) (%F)] %m%n"

#define TLOG_DEF_CATEGORY_TEXTROOT		"text"
#define TLOG_DEF_CATEGORY_DATAROOT      "data"
#define TLOG_DEF_CATEGORY_TEXTTRACE     "texttrace"
#define TLOG_DEF_CATEGORY_TEXTERR       "texterr"
#define TLOG_DEF_CATEGORY_BUS           "texttrace.bus"

TLOGCONF* tlog_get_cfg_from_mib(int a_iMib, const char *a_pszDomain, const char *a_pszName, int a_iProcID);
int tlog_init_cfg_from_file(TLOGCONF* a_pstConf, const char *a_pszPath);
int tlog_init_cfg_default(TLOGCONF* a_pstConf, const char* a_pszPath);

#ifdef __cplusplus
}
#endif

#endif /* TLOGLOAD_H */


