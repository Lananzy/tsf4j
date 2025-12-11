/** @file $RCSfile: tsec_hash.c,v $
  Various hash functions for tsf4g-security module
  $Id: tsec_hash.c,v 1.3 2008/08/13 04:03:49 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008/08/13 04:03:49 $
@version $Revision: 1.3 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/




#include <openssl/evp.h>

#include "sec/tsec.h"
#include "sec/tsec_i.h"


TSECSET s_stTsec ;
TSECGLOBAL mstTsecIni ;

static int s_iInited ;
static TSECARGS s_stValues ;

LPTLOGCATEGORYINST g_HashLog ;


/**
  various internal function defined
*/
int tsec_so_hash (
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const EVP_MD *type
) ;



/**
  @brief Global inited function
  @retval 0 -- service inited ok
  @retval !0 -- service inited failed
  @note This function should be invoked before using any tsec functions
*/
TSEC_API int tsec_so_init ( IN const char *a_pszBuffer , IN LPTLOGCATEGORYINST a_pstLogCat)
{
  int iRet = TSEC_OK ;

  /**
    init debug log
  */
  if (NULL != a_pstLogCat)
  {
	  g_HashLog = a_pstLogCat;
  }
  

  tlog_debug(g_HashLog,0,0, "CALL: tsec_so_init ( '%s' )", a_pszBuffer ) ;

  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;

  s_iInited = SVC_INITED ;

  tlog_debug(g_HashLog,0,0, "RETN: tsec_so_init ( const char * )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_init_ex ( IN int argc, IN char** argv, INOUT void* extbuf, INOUT ssize_t *extbufLen )
{
  int iRet = TSEC_OK ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_init_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (0 >= argc) || (NULL == argv) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_HashLog,0,0, "RETN: tsec_init_ex ( )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_process (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;
  char sType[SIZE_16B] = { 0 } ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_process ( \"%s\", %i, data, %i, apvBuffer, %i )",
      argsbuf, argsLen, aiDataLen, *apiLen ) ;

  /**
    parse buffer first
  */
  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;
  s_stValues.iArgsCnt = tsec_parse_args ( argsbuf, s_stValues.stArgs, TSEC_MAX_ARGS ) ;
  tlog_debug(g_HashLog,0,0, "DEBUG: tsec_process buffer parse item count %i", s_stValues.iArgsCnt ) ;


  /**
    get type
  */
  tsec_get_value ( HASH_TYPE, sType, sizeof(sType), s_stValues ) ;

  if ( 0 == strcasecmp ( HASH_MD2, sType ) ) {
    iRet = tsec_so_hash ( apvData, aiDataLen, apvBuffer, apiLen, EVP_md2() ) ;

  } else if ( 0 == strcasecmp ( HASH_MD5, sType ) ) {
    iRet = tsec_so_hash ( apvData, aiDataLen, apvBuffer, apiLen, EVP_md5() ) ;

  } else if ( 0 == strcasecmp ( HASH_SHA, sType ) ) {
    iRet = tsec_so_hash ( apvData, aiDataLen, apvBuffer, apiLen, EVP_sha() ) ;

  } else if ( 0 == strcasecmp ( HASH_SHA1, sType ) ) {
    iRet = tsec_so_hash ( apvData, aiDataLen, apvBuffer, apiLen, EVP_sha1() ) ;

  } else {
    /**< default is MD5 mode */
    iRet = tsec_so_hash ( apvData, aiDataLen, apvBuffer, apiLen, EVP_md5() ) ;
  }

  tlog_debug(g_HashLog,0,0, "RETN: tsec_process( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_process_ex (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *data,
  IN const ssize_t dataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  INOUT void *extbuf,
  INOUT ssize_t *extbufLen
)
{
  int iRet = TSEC_OK ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_process_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_HashLog,0,0, "RETN: tsec_process_ex ( )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_de_process (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_de_process( \"%s\", %i, data, %i, apvBuffer, %i )",
      argsbuf, argsLen, aiDataLen, *apiLen ) ;

  /* just for annoying warning messages from complier */
  if ((NULL == apvData) || (NULL == apvBuffer) )
  {
    ;
  }

  tlog_debug(g_HashLog,0,0, "RETN: tsec_de_process( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_de_process_ex (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *data,
  IN const ssize_t dataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  INOUT void *extbuf,
  INOUT ssize_t *extbufLen
)
{
  int iRet = TSEC_OK ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_de_process_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_HashLog,0,0, "RETN: tsec_de_process_ex ( )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_fini ( )
{
  int iRet = TSEC_OK ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_fini ( )" ) ;


  tlog_debug(g_HashLog,0,0, "RETN: tsec_fini ( )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_fini_ex ( INOUT void *extbuf, INOUT ssize_t *extbufLen )
{
  int iRet = TSEC_OK ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_fini_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_HashLog,0,0, "RETN: tsec_fini_ex ( )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_hash (
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const EVP_MD *type
)
{
  int iRet = TSEC_OK ;
  unsigned int iOutLen = 0 ;
  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  EVP_MD_CTX context ;

  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  tlog_debug(g_HashLog,0,0, "CALL: tsec_so_hash( data, %i, apvBuffer, %i, EVP_MD * )", aiDataLen, *apiLen ) ;

  EVP_DigestInit ( &context, type ) ;

  EVP_DigestUpdate ( &context, pchInBuf, aiDataLen ) ;

  EVP_DigestFinal ( &context, pchOutBuf, &iOutLen ) ;

  *apiLen = iOutLen ;

  tlog_debug(g_HashLog,0,0, "RETN: tsec_so_hash( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}

