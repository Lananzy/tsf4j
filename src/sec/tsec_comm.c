/** @file $RCSfile: tsec_comm.c,v $
  general description of this module
  $Id: tsec_comm.c,v 1.2 2008/08/13 03:57:46 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008/08/13 03:57:46 $
@version $Revision: 1.2 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include "sec/tsec.h"


/**
  @brief
  @retval
  @retval
  @param
  @note
*/
int tsec_parse_args ( IN const char *a_pszBuffer, INOUT TSECVAL *a_pstArgs, IN const int a_iTotalItem )
{
  int iCnt = 0;

  char *lpcValue = NULL,
       *lpcIndex = NULL,
       *lpcNode = NULL,
       *lpcDelim = "; " ; /**< delimeter index: semicolon and space */

  if ( NULL == a_pszBuffer ) return 0 ;

  /* tlog_debug(g_pstCat,0,0, "CALL: tsec_parse_args ( '%s', pstArgs *, %i )", a_pszBuffer, a_iTotalItem ) ; */


  if ( NULL == ( lpcValue = strdup ( a_pszBuffer ) ) ) return 0 ;

  /**
    first node
  */
  if ( NULL != ( lpcNode = strtok ( lpcValue, lpcDelim ) ) ) {
    if ( NULL != ( lpcIndex = strchr ( lpcNode, '=' ) ) ) {
      STRNCPY ( a_pstArgs[iCnt].sName, lpcNode, lpcIndex-lpcNode+1 ) ; /**< set '\0' at last byte */

      lpcIndex ++ ;
      STRNCPY ( a_pstArgs[iCnt].sValue, lpcIndex, sizeof(a_pstArgs[iCnt].sValue) ) ;

      iCnt ++ ;
    }
  } else {
    /**< null string, just return */

    if ( NULL != lpcValue ) free ( lpcValue ) ;
    lpcValue = NULL ;
    return 0 ;
  }


  /**
    parsed from second node
  */
  while ( NULL != ( lpcNode = strtok ( NULL, lpcDelim ) ) ) {

    if ( NULL != ( lpcIndex = strchr ( lpcNode, '=' ) ) ) {
      STRNCPY ( a_pstArgs[iCnt].sName, lpcNode, lpcIndex-lpcNode+1 ) ; /**< set '\0' at last byte */

      lpcIndex ++ ;
      STRNCPY ( a_pstArgs[iCnt].sValue, lpcIndex, sizeof(a_pstArgs[iCnt].sValue) ) ;

      iCnt ++ ;
      if ( a_iTotalItem <= iCnt ) {
        /* tlog_debug(g_pstCat,0,0, "DEBUG: Max item counts reach %i, just break", iCnt  ) ; */
        break ;
      }
    }
  }

  if ( NULL != lpcValue ) free ( lpcValue ) ;
  lpcValue = NULL ;

  /* tlog_debug(g_pstCat,0,0, "RETN: tsec_parse_args ( buffer, pstArgs *, cnt )=%i ", iCnt  ) ; */
  return iCnt ;
}


/**
  @brief
  @retval
  @retval
  @param
  @note
*/
int tsec_get_value ( IN const char *a_pszKey, INOUT char *a_pszValue, IN const ssize_t a_iValLen, IN const TSECARGS a_stArgs )
{
  int iRet = TSEC_FAILED ;

  /* tlog_debug(g_pstCat,0,0, "CALL: tsec_get_value ( '%s' )", a_pszKey ) ; */

  int i = 0 ;
  for ( i = 0; i<a_stArgs.iArgsCnt; i++ ) {
    if ( 0 == strcasecmp ( a_pszKey, a_stArgs.stArgs[i].sName ) ) {
      STRNCPY ( a_pszValue, a_stArgs.stArgs[i].sValue, a_iValLen ) ;
      iRet = TSEC_OK ;
      break ;
    }
  }

  /* tlog_debug(g_pstCat,0,0, "RETN: tsec_get_value ( '%s' )=%i, value '%s' ", a_pszKey, iRet, a_pszValue ) ; */
  return iRet ;
}

