/** @file $RCSfile: tsec.c,v $
  Various functions invoked by applications for tsf-g-security module
  $Id: tsec.c,v 1.3 2008/08/13 04:03:49 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008/08/13 04:03:49 $
@version 1.0
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include "sec/tsec.h"
#include "sec/tsec_i.h"

TSECSET s_stTsec ;
TSECGLOBAL mstTsecIni ;
extern LPTLOGCATEGORYINST g_pstCat;

/**
  various internal function defined
*/
int tsec_ip_oper_i ( IN const int a_iHandler, IN const char *a_pszValue, IN const int a_iLen ) ;



/**
  @brief multi functions execution
  @retval 0 -- ok
  @retval !0 -- failed
  @note
*/
int tsec_begin_trans (
  IN const int a_iHandler
)
{
  int iRet = TSEC_OK ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_begin_trans ( %i )", a_iHandler ) ;

  if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_begin_trans handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    return TSEC_ARG_ERROR ;
  }

  s_stTsec.ptTsecAPI[a_iHandler].iTrans = TSEC_TRANS_SET ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_begin_trans ( %i )=%i", a_iHandler, iRet ) ;

  return iRet ;
}


/**
  @brief multi functions execution
  @retval 0 -- ok
  @retval !0 -- failed
  @note
*/
int tsec_end_trans (
  IN const int a_iHandler
)
{
  int iRet = TSEC_OK ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_end_trans ( %i )", a_iHandler ) ;

  if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_end_trans handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    return TSEC_ARG_ERROR ;
  }

  s_stTsec.ptTsecAPI[a_iHandler].iTrans = TSEC_TRANS_UNSET ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_end_trans ( %i )=%i", a_iHandler, iRet ) ;

  return iRet ;
}


/**
  @brief Encrypted function: DES, 3DES, AES, IDEA, Blowfish
  @retval 0 -- encrypted ok
  @retval !0 -- encrypted failed
  @note
*/
int tsec_encrypt (
  IN const int a_iHandler,
  IN const char *a_pszKeys,
  IN const void *a_pvData,
  IN const ssize_t a_iDataLen,
  INOUT void *a_pvBuffer,
  INOUT ssize_t *a_piLen
)
{
  int iRet = TSEC_OK ;
  char sValue[SIZE_512B] = { 0 },
       sType[SIZE_16B] = { 0 },
       sCtrFlag[SIZE_32B] = { 0 } ;
  int iFuncIdx = 0,
      iErrCode = 0,
      iLen = 0 ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_encrypt ( %i, '%s', data, %i, buffer, %i )", a_iHandler, a_pszKeys, a_iDataLen, *a_piLen ) ;

  /**
    check service so status
  */
  if ( SVC_NOT_INITED == s_stTsec.iInited )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec module not inited, no service avaliable" ) ;
    iRet = TSEC_SVC_DISABLE ;
  }
  else if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_encrypt handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    iRet = TSEC_ARG_ERROR ;
  }

  if ( TSEC_OK != iRet )
  {
    return iRet ;
  }

  /**
    generated a various buffer
  */

  /**
    if iTrans == TSEC_TRANS_SET, invoke functions one by one
  */
  do {

    /**
      check function avaliablity
    */
    if ( FUNC_NOT_INITED == s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].iRun )
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_crypto %i function not avaliable", a_iHandler ) ;
      iErrCode = FUNC_NOT_INITED ;
      iFuncIdx ++ ; /**< point to next function */
      continue ;
    }

    /**
      get type
    */
    tsec_get_value ( CRYPTO_TYPE, sType, sizeof(sType), s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sType ) STRNCPY ( sType, CRYPTO_DES_ECB, sizeof(sType) ) ;

    iLen = snprintf ( sValue, sizeof(sValue), "%s=%s;%s=%s", CRYPTO_TYPE, sType, CRYPTO_KEY, a_pszKeys ) ;

    /**
      real enctypted function invoked
    */
    iRet = s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].so_proc ( sValue, iLen, a_pvData, a_iDataLen, a_pvBuffer, a_piLen ) ;

    /**
      check ctrl flag
    */
    STRNCPY ( sCtrFlag, s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].stItem.sCtrFlag, sizeof(sCtrFlag) ) ;
    tlog_debug(g_pstCat,0,0, "DEBUG: tsec_encrypt, func %i ctrl flag '%s' ", a_iHandler, sCtrFlag ) ;
    if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUIRED ) )
    {
      /**< only return when all REQUIRED module invoked */
      if ( TSEC_OK == iRet )
      {
        /**< just continue */

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_encrypt, func %i ctrl flag '%s' failed", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUISITE ) )
    {
      /**< if invoked successfully, continue invoked other functions; otherwise, return immediately */
      if ( TSEC_OK == iRet )
      {

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_encrypt, func %i ctrl flag '%s' failed, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
        break ; /**< loop break */
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_SUFFICIENT ) )
    {
      /**< if invoked successfully, just return; otherwise, continue invoke other functions */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_encrypt, func %i ctrl flag '%s' ok, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ; /**< reset iErrCode */
        break ; /**< loop break */
      }
      else
      {
        /**< just continue, do not set iErrCode */

      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_OPTIONAL ) )
    {
      /**< skip any successful or failed invoked result */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_encrypt, func %i ctrl flag '%s' ok, loop continue", a_iHandler, sCtrFlag ) ;

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_encrypt, func %i ctrl flag '%s' failed, loop continue", a_iHandler, sCtrFlag ) ;
      }

    }
    else
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_encrypt %i function unknown ctrl flag", a_iHandler ) ;
      iErrCode = TSEC_CTRL_UNKNOWN ;
      break ;
    }

    iFuncIdx ++ ; /**< point to next function */

  } while ( (iFuncIdx < s_stTsec.ptTsecAPI[a_iHandler].iCryptoCnts) && (TSEC_TRANS_SET == s_stTsec.ptTsecAPI[a_iHandler].iTrans) ) ;


  tlog_debug(g_pstCat,0,0, "RETN: tsec_encrypt ( buffer, %i )=%i, ErrCode %i", *a_piLen, iRet, iErrCode ) ;

  return ( (0 == iErrCode)? iRet : iErrCode ) ;
}


/**
  @brief Descrypted function: DES, 3DES, AES, IDEA, Blowfish
  @retval 0 -- descrypted ok
  @retval !0 -- descrypted failed
  @note
*/
int tsec_descrypt (
  IN const int a_iHandler,
  IN const char *a_pszKeys,
  IN const void *a_pvData,
  IN const ssize_t a_iDataLen,
  INOUT void *a_pvBuffer,
  INOUT ssize_t *a_piLen
)
{
  int iRet = TSEC_OK ;
  char sValue[SIZE_512B] = { 0 },
       sType[SIZE_16B] = { 0 },
       sCtrFlag[SIZE_32B] = { 0 } ;
  int iFuncIdx = 0,
      iErrCode = 0,
      iLen = 0 ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_descrypt ( %i, '%s', data, %i, buffer, %i )", a_iHandler, a_pszKeys, a_iDataLen, *a_piLen ) ;

  if ( SVC_NOT_INITED == s_stTsec.iInited ) {
    tlog_error(g_pstCat,0,0, "ERROR: tsec module not inited, no service avaliable" ) ;
    iRet = TSEC_SVC_DISABLE ;
  }
  else if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_descrypt handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    iRet = TSEC_ARG_ERROR ;
  }

  if ( TSEC_OK != iRet )
  {
    return iRet ;
  }

  /**
    generated a key=value buffer
  */

  /**
    if iTrans == TSEC_TRANS_SET, invoke functions one by one
  */
  do {

    /**
      check function avaliablity
    */
    if ( FUNC_NOT_INITED == s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].iRun )
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_descrypt %i function not avaliable", a_iHandler ) ;
      iErrCode = FUNC_NOT_INITED ;
      iFuncIdx ++ ; /**< point to next function */
      continue ;
    }

    /**
      get type
    */
    tsec_get_value ( CRYPTO_TYPE, sType, sizeof(sType), s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sType ) STRNCPY ( sType, CRYPTO_DES_ECB, sizeof(sType) ) ;

    iLen = snprintf ( sValue, sizeof(sValue), "%s=%s;%s=%s", CRYPTO_TYPE, sType, CRYPTO_KEY, a_pszKeys ) ;


    /**
      real enctypted function invoked
    */
    iRet = s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].so_deproc ( sValue, iLen, a_pvData, a_iDataLen, a_pvBuffer, a_piLen ) ;

    /**
      check ctrl flag
    */
    STRNCPY ( sCtrFlag, s_stTsec.ptTsecAPI[a_iHandler].pstCrypto[iFuncIdx].stItem.sCtrFlag, sizeof(sCtrFlag) ) ;
    tlog_debug(g_pstCat,0,0, "DEBUG: tsec_descrypt, func %i ctrl flag '%s' ", a_iHandler, sCtrFlag ) ;
    if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUIRED ) )
    {
      /**< no matter if the result is ok or failed, just indicated to invoke next function */
      if ( TSEC_OK == iRet )
      {
        /**< just continue */

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_descrypt, func %i ctrl flag '%s' failed", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUISITE ) )
    {
      /**< if invoked successfully, continue to invoke next function; otherwise, return immediately */
      if ( TSEC_OK == iRet )
      {

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_descrypt, func %i ctrl flag '%s' failed, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
        break ; /**< loop break */
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_SUFFICIENT ) )
    {
      /**< if invoked successfully, just return; otherwise, continue to invoke next function */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_descrypt, func %i ctrl flag '%s' ok, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ; /**< reset iErrCode */
        break ; /**< loop break */
      }
      else
      {
        /**< just continue, do not set iErrCode */

      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_OPTIONAL ) )
    {
      /**< skip any successful or failed invoked result */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_descrypt, func %i ctrl flag '%s' ok, loop continue", a_iHandler, sCtrFlag ) ;
      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_descrypt, func %i ctrl flag '%s' failed, loop continue", a_iHandler, sCtrFlag ) ;
      }

    }
    else
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_descrypt %i function unknown ctrl flag", a_iHandler ) ;
      iErrCode = TSEC_CTRL_UNKNOWN ;
      break ;
    }

    iFuncIdx ++ ; /**< point to next function */

  } while ( (iFuncIdx < s_stTsec.ptTsecAPI[a_iHandler].iCryptoCnts) && (TSEC_TRANS_SET == s_stTsec.ptTsecAPI[a_iHandler].iTrans) ) ;


  tlog_debug(g_pstCat,0,0, "RETN: tsec_descrypt ( buffer, %i )=%i, ErrCode %i", *a_piLen, iRet, iErrCode ) ;

  return ( (0 == iErrCode)? iRet : iErrCode ) ;
}


/**
  @brief Hash function: MD2, MD5, SHA, SHA1
  @retval 0 -- hash ok
  @retval !0 -- hash failed
  @note
*/
int tsec_hash (
  IN const int a_iHandler,
  IN const void *a_pvData,
  IN const ssize_t a_iDataLen,
  INOUT void *a_pvBuffer,
  INOUT ssize_t *a_piLen
)
{
  int iRet = TSEC_OK ;
  char sValue[SIZE_512B] = { 0 },
       sType[SIZE_16B] = { 0 },
       sCtrFlag[SIZE_32B] = { 0 } ;
  int iFuncIdx = 0,
      iErrCode = 0,
      iLen = 0 ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_hash ( %i, data, %i, buffer, %i )", a_iHandler, a_iDataLen, *a_piLen ) ;

  /**
    check service so status
  */
  if ( SVC_NOT_INITED == s_stTsec.iInited )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec module not inited, no service avaliable" ) ;
    iRet = TSEC_SVC_DISABLE ;
  }
  else if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_hash handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    iRet = TSEC_ARG_ERROR ;
  }

  if ( TSEC_OK != iRet )
  {
    return iRet ;
  }

  /**
    generated a various buffer
  */

  /**
    if iTrans == TSEC_TRANS_SET, invoke functions one by one
  */
  do {

    /**
      check function avaliablity
    */
    if ( FUNC_NOT_INITED == s_stTsec.ptTsecAPI[a_iHandler].pstHash[iFuncIdx].iRun )
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_hash %i function not avaliable", a_iHandler ) ;
      iErrCode = FUNC_NOT_INITED ;
      iFuncIdx ++ ; /**< point to next function */
      continue ;
    }

    /**
      get type
    */
    tsec_get_value ( HASH_TYPE, sType, sizeof(sType), s_stTsec.ptTsecAPI[a_iHandler].pstHash[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sType ) STRNCPY ( sType, HASH_DEFAULT, sizeof(sType) ) ;

    iLen = snprintf ( sValue, sizeof(sValue), "%s=%s", HASH_TYPE, sType ) ;


    /**
      real hash function invoked
    */
    iRet = s_stTsec.ptTsecAPI[a_iHandler].pstHash[iFuncIdx].so_proc ( sValue, iLen, a_pvData, a_iDataLen, a_pvBuffer, a_piLen ) ;

    /**
      check ctrl flag
    */
    STRNCPY ( sCtrFlag, s_stTsec.ptTsecAPI[a_iHandler].pstHash[iFuncIdx].stItem.sCtrFlag, sizeof(sCtrFlag) ) ;
    tlog_debug(g_pstCat,0,0, "DEBUG: tsec_hash, func %i ctrl flag '%s' ", a_iHandler, sCtrFlag ) ;
    if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUIRED ) )
    {
      /**< only return when all REQUIRED module invoked */
      if ( TSEC_OK == iRet )
      {
        /**< just continue */

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_hash, func %i ctrl flag '%s' failed", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUISITE ) )
    {
      /**< if invoked successfully, continue invoked other functions; otherwise, return immediately */
      if ( TSEC_OK == iRet )
      {

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_hash, func %i ctrl flag '%s' failed, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
        break ; /**< loop break */
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_SUFFICIENT ) )
    {
      /**< if invoked successfully, just return; otherwise, continue invoke other functions */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_hash, func %i ctrl flag '%s' ok, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ; /**< reset iErrCode */
        break ; /**< loop break */
      }
      else
      {
        /**< just continue, do not set iErrCode */

      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_OPTIONAL ) )
    {
      /**< skip any successful or failed invoked result */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_hash, func %i ctrl flag '%s' ok, loop continue", a_iHandler, sCtrFlag ) ;

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_hash, func %i ctrl flag '%s' failed, loop continue", a_iHandler, sCtrFlag ) ;
      }

    }
    else
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_hash %i function unknown ctrl flag", a_iHandler ) ;
      iErrCode = TSEC_CTRL_UNKNOWN ;
      break ;
    }

    iFuncIdx ++ ; /**< point to next function */

  } while ( (iFuncIdx < s_stTsec.ptTsecAPI[a_iHandler].iHashCnts) && (TSEC_TRANS_SET == s_stTsec.ptTsecAPI[a_iHandler].iTrans) ) ;


  tlog_debug(g_pstCat,0,0, "RETN: tsec_hash ( buffer, %i )=%i, ErrCode %i", *a_piLen, iRet, iErrCode ) ;

  return ( (0 == iErrCode)? iRet : iErrCode ) ;
}


/**
  @brief asymmetrical crypto function: RSA
  @retval 0 -- encrypted ok
  @retval !0 -- encrypted failed
  @note
*/
int tsec_aencrypt (
  IN const int a_iHandler,
  IN const char *a_pszKeys,
  IN const void *a_pvData,
  IN const ssize_t a_iDataLen,
  INOUT void *a_pvBuffer,
  INOUT ssize_t *a_piLen
)
{
  int iRet = TSEC_OK ;
  char sValue[SIZE_512B] = { 0 },
       sType[SIZE_16B] = { 0 },
       sBits[SIZE_16B] = { 0 },
       sSub[SIZE_16B] = { 0 },
       sCtrFlag[SIZE_32B] = { 0 } ;
  int iFuncIdx = 0,
      iErrCode = 0,
      iLen = 0 ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_aencrypt ( %i, '%s', data, %i, buffer, %i )", a_iHandler, a_pszKeys, a_iDataLen, *a_piLen ) ;

  /**
    check service so status
  */
  if ( SVC_NOT_INITED == s_stTsec.iInited )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec module not inited, no service avaliable" ) ;
    iRet = TSEC_SVC_DISABLE ;
  }
  else if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_aencrypt handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    iRet = TSEC_ARG_ERROR ;
  }

  if ( TSEC_OK != iRet )
  {
    return iRet ;
  }

  /**
    generated a various buffer
  */

  /**
    if iTrans == TSEC_TRANS_SET, invoke functions one by one
  */
  do {

    /**
      check function avaliablity
    */
    if ( FUNC_NOT_INITED == s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].iRun )
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_aencrypt %i function not avaliable", a_iHandler ) ;
      iErrCode = FUNC_NOT_INITED ;
      iFuncIdx ++ ; /**< point to next function */
      continue ;
    }

    /**
      get type
    */
    tsec_get_value ( ACRYPTO_TYPE, sType, sizeof(sType), s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sType ) STRNCPY ( sType, ACRYPTO_RSA, sizeof(sType) ) ;

    /**
      get key generated bits
    */
    tsec_get_value ( ACRYPTO_BITS, sBits, sizeof(sBits), s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sBits ) snprintf ( sBits, sizeof(sBits), "%i", ACRYPTO_BITS_DEFAULT ) ;

    /**
      get pubcrypt
    */
    tsec_get_value ( ACRYPTO_SUB, sSub, sizeof(sType), s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sSub ) snprintf ( sSub, sizeof(sSub), "%i", ACRYPTO_SUB_DEFAULT ) ;

    iLen = snprintf ( sValue, sizeof(sValue),
        "%s=%s;%s=%s;%s=%s;%s=%s",
        ACRYPTO_TYPE, sType, ACRYPTO_FILE, a_pszKeys, ACRYPTO_SUB, sSub, ACRYPTO_BITS, sBits ) ;

    /**
      real enctypted function invoked
    */
    iRet = s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].so_proc ( sValue, iLen, a_pvData, a_iDataLen, a_pvBuffer, a_piLen ) ;

    /**
      check ctrl flag
    */
    STRNCPY ( sCtrFlag, s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.sCtrFlag, sizeof(sCtrFlag) ) ;
    tlog_debug(g_pstCat,0,0, "DEBUG: tsec_aencrypt, func %i ctrl flag '%s' ", a_iHandler, sCtrFlag ) ;
    if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUIRED ) )
    {
      /**< only return when all REQUIRED module invoked */
      if ( TSEC_OK == iRet )
      {
        /**< just continue */

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_aencrypt, func %i ctrl flag '%s' failed", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUISITE ) )
    {
      /**< if invoked successfully, continue invoked other functions; otherwise, return immediately */
      if ( TSEC_OK == iRet )
      {

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_aencrypt, func %i ctrl flag '%s' failed, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
        break ; /**< loop break */
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_SUFFICIENT ) )
    {
      /**< if invoked successfully, just return; otherwise, continue invoke other functions */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_aencrypt, func %i ctrl flag '%s' ok, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ; /**< reset iErrCode */
        break ; /**< loop break */
      }
      else
      {
        /**< just continue, do not set iErrCode */

      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_OPTIONAL ) )
    {
      /**< skip any successful or failed invoked result */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_aencrypt, func %i ctrl flag '%s' ok, loop continue", a_iHandler, sCtrFlag ) ;

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_aencrypt, func %i ctrl flag '%s' failed, loop continue", a_iHandler, sCtrFlag ) ;
      }

    }
    else
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_crypto %i function unknown ctrl flag", a_iHandler ) ;
      iErrCode = TSEC_CTRL_UNKNOWN ;
      break ;
    }

    iFuncIdx ++ ; /**< point to next function */

  } while ( (iFuncIdx < s_stTsec.ptTsecAPI[a_iHandler].iAsyCnts) && (TSEC_TRANS_SET == s_stTsec.ptTsecAPI[a_iHandler].iTrans) ) ;


  tlog_debug(g_pstCat,0,0, "RETN: tsec_aencrypt ( buffer, %i )=%i, ErrCode %i", *a_piLen, iRet, iErrCode ) ;

  return ( (0 == iErrCode)? iRet : iErrCode ) ;
}


/**
  @brief asymmetrical crypto function: RSA
  @retval 0 -- descrypted ok
  @retval !0 -- descrypted failed
  @note
*/
int tsec_adescrypt (
  IN const int a_iHandler,
  IN const char *a_pszKeys,
  IN const void *a_pvData,
  IN const ssize_t a_iDataLen,
  INOUT void *a_pvBuffer,
  INOUT ssize_t *a_piLen
)
{
  int iRet = TSEC_OK ;
  char sValue[SIZE_512B] = { 0 },
       sType[SIZE_16B] = { 0 },
       sSub[SIZE_16B] = { 0 },
       sBits[SIZE_16B] = { 0 },
       sCtrFlag[SIZE_32B] = { 0 } ;
  int iFuncIdx = 0,
      iErrCode = 0,
      iLen = 0 ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_adescrypt(%i, '%s', data, %i, buffer, %i)", a_iHandler, a_pszKeys, a_iDataLen, *a_piLen ) ;

  if ( SVC_NOT_INITED == s_stTsec.iInited ) {
    tlog_error(g_pstCat,0,0, "ERROR: tsec module not inited, no service avaliable" ) ;
    iRet = TSEC_SVC_DISABLE ;
  }
  else if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_adescrypt handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    iRet = TSEC_ARG_ERROR ;
  }

  if ( TSEC_OK != iRet )
  {
    return iRet ;
  }

  /**
    generated a key=value buffer
  */

  /**
    if iTrans == TSEC_TRANS_SET, invoke functions one by one
  */
  do {

    /**
      check function avaliablity
    */
    if ( FUNC_NOT_INITED == s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].iRun )
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_adescrypt %i function not avaliable", a_iHandler ) ;
      iErrCode = FUNC_NOT_INITED ;
      iFuncIdx ++ ; /**< point to next function */
      continue ;
    }

    /**
      get type
    */
    tsec_get_value ( ACRYPTO_TYPE, sType, sizeof(sType), s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sType ) STRNCPY ( sType, ACRYPTO_RSA, sizeof(sType) ) ;

    /**
      get key generated bits
    */
    tsec_get_value ( ACRYPTO_BITS, sBits, sizeof(sBits), s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sBits ) snprintf ( sBits, sizeof(sBits), "%i", ACRYPTO_BITS_DEFAULT ) ;

    /**
      get pubcrypt
    */
    tsec_get_value ( ACRYPTO_SUB, sSub, sizeof(sSub), s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.stArgFields ) ;
    if ( '\0' == *sSub ) snprintf ( sSub, sizeof(sSub), "%i", ACRYPTO_SUB_DEFAULT ) ;

    iLen = snprintf ( sValue, sizeof(sValue),
        "%s=%s;%s=%s;%s=%s;%s=%s",
        ACRYPTO_TYPE, sType, ACRYPTO_FILE, a_pszKeys, ACRYPTO_SUB, sSub, ACRYPTO_BITS, sBits ) ;

    /**
      real enctypted function invoked
    */
    iRet = s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].so_deproc ( sValue, iLen, a_pvData, a_iDataLen, a_pvBuffer, a_piLen ) ;

    /**
      check ctrl flag
    */
    STRNCPY ( sCtrFlag, s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].stItem.sCtrFlag, sizeof(sCtrFlag) ) ;
    tlog_debug(g_pstCat,0,0, "DEBUG: tsec_adescrypt, func %i ctrl flag '%s' ", a_iHandler, sCtrFlag ) ;
    if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUIRED ) )
    {
      /**< no matter if the result is ok or failed, just indicated to invoke next function */
      if ( TSEC_OK == iRet )
      {
        /**< just continue */

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_adescrypt, func %i ctrl flag '%s' failed", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUISITE ) )
    {
      /**< if invoked successfully, continue to invoke next function; otherwise, return immediately */
      if ( TSEC_OK == iRet )
      {

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_adescrypt, func %i ctrl flag '%s' failed, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
        break ; /**< loop break */
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_SUFFICIENT ) )
    {
      /**< if invoked successfully, just return; otherwise, continue to invoke next function */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_adescrypt, func %i ctrl flag '%s' ok, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ; /**< reset iErrCode */
        break ; /**< loop break */
      }
      else
      {
        /**< just continue, do not set iErrCode */

      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_OPTIONAL ) )
    {
      /**< skip any successful or failed invoked result */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_adescrypt, func %i ctrl flag '%s' ok, loop continue", a_iHandler, sCtrFlag ) ;
      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_adescrypt, func %i ctrl flag '%s' failed, loop continue", a_iHandler, sCtrFlag ) ;
      }

    }
    else
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_adescrypt %i function unknown ctrl flag", a_iHandler ) ;
      iErrCode = TSEC_CTRL_UNKNOWN ;
      break ;
    }

    iFuncIdx ++ ; /**< point to next function */

  } while ( (iFuncIdx < s_stTsec.ptTsecAPI[a_iHandler].iAsyCnts) && (TSEC_TRANS_SET == s_stTsec.ptTsecAPI[a_iHandler].iTrans) ) ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_adescrypt ( buffer, %i )=%i, ErrCode %i", *a_piLen, iRet, iErrCode ) ;

  return ( (0 == iErrCode)? iRet : iErrCode ) ;
}


/**
  @brief ip connection counter register function
  @param[in] a_iHandler -- service handle, this valude is obtained from tsec_open_handler()
  @param[in] a_pszSrcIP -- source ip, format "xxx.xxx.xxx.xxx", could not be NULL, if no value, just set it empty string
  @retval 0 -- invoked ok
  @retval !0 -- invoked failed
  @note
*/
int tsec_ip_reg (
  IN const int a_iHandler,
  IN const char *a_pszSrcIP
)
{
  int iRet = TSEC_OK,
      iLen = 0 ;
  char sValue[SIZE_512B] = { 0 } ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_ip_reg ( %i, '%s' )", a_iHandler, a_pszSrcIP ) ;

  /**
    generated a various buffer
  */

  /**
    get type
  */
  iLen = snprintf ( sValue, sizeof(sValue),
      "%s=%s;%s=%s",
      IP_VERIFY_TYPE, TSEC_IP_REG, IP_SRC, a_pszSrcIP
  ) ;

  iRet = tsec_ip_oper_i ( a_iHandler, sValue, iLen ) ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_ip_reg ( %i, '%s' )=%i", a_iHandler, a_pszSrcIP, iRet ) ;

  return iRet ;
}


/**
  @brief ip connection counter unregister function
  @param[in] a_iHandler -- service handle, this valude is obtained from tsec_open_handler()
  @param[in] a_pszSrcIP -- source ip, format "xxx.xxx.xxx.xxx", could not be NULL, if no value, just set it empty string
  @retval 0 -- invoked ok
  @retval !0 -- invoked failed
  @note
*/
int tsec_ip_unreg (
  IN const int a_iHandler,
  IN const char *a_pszSrcIP
)
{
  int iRet = TSEC_OK,
      iLen = 0 ;
  char sValue[SIZE_512B] = { 0 } ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_ip_unreg ( %i, '%s' )", a_iHandler, a_pszSrcIP ) ;

  /**
    generated a various buffer
  */

  /**
    get type
  */
  iLen = snprintf ( sValue, sizeof(sValue),
      "%s=%s;%s=%s",
      IP_VERIFY_TYPE, TSEC_IP_UNREG, IP_SRC, a_pszSrcIP
  ) ;

  iRet = tsec_ip_oper_i ( a_iHandler, sValue, iLen ) ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_ip_unreg ( %i, '%s' )=%i", a_iHandler, a_pszSrcIP, iRet ) ;

  return iRet ;
}


/**
  @brief ip connection conunter reset function
  @param[in] a_iHandler -- service handle, this valude is obtained from tsec_open_handler()
  @param[in] a_pszSrcIP -- source ip, format "xxx.xxx.xxx.xxx", could not be NULL, if no value, just set it empty string
  @retval 0 -- invoked ok
  @retval !0 -- invoked failed
  @note
*/
int tsec_ip_reset (
  IN const int a_iHandler,
  IN const char *a_pszSrcIP
)
{
  int iRet = TSEC_OK,
      iLen = 0 ;
  char sValue[SIZE_512B] = { 0 } ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_ip_reset ( %i, '%s' )", a_iHandler, a_pszSrcIP ) ;

  /**
    generated a various buffer
  */

  /**
    get type
  */
  iLen = snprintf ( sValue, sizeof(sValue),
      "%s=%s;%s=%s",
      IP_VERIFY_TYPE, TSEC_IP_RESET, IP_SRC, a_pszSrcIP
  ) ;

  iRet = tsec_ip_oper_i ( a_iHandler, sValue, iLen ) ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_ip_reset ( %i, '%s' )=%i", a_iHandler, a_pszSrcIP, iRet ) ;

  return iRet ;
}


/**
  @brief source ip verfify function
  @param[in] a_iHandler -- service handle, this valude is obtained from tsec_open_handler()
  @param[in] a_pszSrcIP -- source ip, format "xxx.xxx.xxx.xxx", could not be NULL, if no value, just set it empty string
  @retval 0 -- invoked ok
  @retval !0 -- invoked failed
  @note
*/
int tsec_ip_srcip_verify (
  IN const int a_iHandler,
  IN const char *a_pszSrcIP
)
{
  int iRet = TSEC_OK,
      iLen = 0 ;
  char sValue[SIZE_512B] = { 0 } ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_ip_srcip_verify ( %i, '%s' )", a_iHandler, a_pszSrcIP ) ;

  /**
    generated a various buffer
  */

  /**
    get type
  */
  iLen = snprintf ( sValue, sizeof(sValue),
      "%s=%s;%s=%s",
      IP_VERIFY_TYPE, TSEC_IP_SRCIP, IP_SRC, a_pszSrcIP
  ) ;

  iRet = tsec_ip_oper_i ( a_iHandler, sValue, iLen ) ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_ip_srcip_verify ( %i, '%s' )=%i", a_iHandler, a_pszSrcIP, iRet ) ;

  return iRet ;
}


/**
  @brief source ip --> destination ip verfify function
  @param[in] a_iHandler -- service handle, this valude is obtained from tsec_open_handler()
  @param[in] a_pszSrcIP -- source ip, format "xxx.xxx.xxx.xxx", could not be NULL, if no value, just set it empty string
  @param[in] a_pszDstIP -- destination ip, format "xxx.xxx.xxx.xxx", could not be NULL, if no value, just set it empty string 
  @retval 0 -- invoked ok
  @retval !0 -- invoked failed
  @note
*/
int tsec_ip_dstip_verify (
  IN const int a_iHandler,
  IN const char *a_pszSrcIP,
  IN const char *a_pszDstIP
)
{
  int iRet = TSEC_OK,
      iLen = 0 ;
  char sValue[SIZE_512B] = { 0 } ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_ip_dstip_verify ( %i, '%s', '%s' )", a_iHandler, a_pszSrcIP, a_pszDstIP ) ;

  /**
    generated a various buffer
  */

  /**
    get type
  */
  iLen = snprintf ( sValue, sizeof(sValue),
      "%s=%s;%s=%s;%s=%s",
      IP_VERIFY_TYPE, TSEC_IP_SRCIP, IP_SRC, a_pszSrcIP, IP_DST, a_pszDstIP
  ) ;

  iRet = tsec_ip_oper_i ( a_iHandler, sValue, iLen ) ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_ip_dstip_verify ( %i, '%s', '%s' )=%i", a_iHandler, a_pszSrcIP, a_pszDstIP, iRet ) ;

  return iRet ;
}


/**
  @brief core operation function for ip verify module
  @retval 0 -- invoked ok
  @retval !0 -- invoked failed
  @note
*/
int tsec_ip_oper_i (
  IN const int a_iHandler,
  IN const char *a_pszValue,
  IN const int a_iLen
)
{
  int iRet = TSEC_OK ;
  char sValue[SIZE_512B] = { 0 },
       sCtrFlag[SIZE_16B] = { 0 },
       sIn[SIZE_512B] = { 0 },
       sOut[SIZE_512B] = { 0 } ;
  int iFuncIdx = 0,
      iErrCode = 0,
      iInLen = 0,
      iOutLen = 0,
      iLen = 0 ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_ip_oper_i ( %i, '%s', %i )", a_iHandler, a_pszValue, a_iLen ) ;

  /**
    check service so status
  */
  if ( SVC_NOT_INITED == s_stTsec.iInited )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec module not inited, no service avaliable" ) ;
    iRet = TSEC_SVC_DISABLE ;
  }
  else if ( (0 > a_iHandler) || (a_iHandler > s_stTsec.iSvcCnt)  )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_ip_oper_i handler error %i, %i(svc cnt)", a_iHandler, s_stTsec.iSvcCnt ) ;
    iRet = TSEC_ARG_ERROR ;
  }

  if ( TSEC_OK != iRet )
  {
    return iRet ;
  }

  /**
    generated a various buffer
  */
  iInLen = strlen ( sIn ) ;
  iOutLen = strlen ( sOut ) ;

  /**
    if iTrans == TSEC_TRANS_SET, invoke functions one by one
  */
  do {

    /**
      check function avaliablity
    */
    if ( FUNC_NOT_INITED == s_stTsec.ptTsecAPI[a_iHandler].pstACrypto[iFuncIdx].iRun )
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_ip_oper_i %i function not avaliable", a_iHandler ) ;
      iErrCode = FUNC_NOT_INITED ;
      iFuncIdx ++ ; /**< point to next function */
      continue ;
    }

    iLen = snprintf ( sValue, sizeof(sValue),
        "%s",
        a_pszValue
    ) ;

    /**
      real enctypted function invoked
    */
    iRet = s_stTsec.ptTsecAPI[a_iHandler].pstIP[iFuncIdx].so_proc ( sValue, iLen, sIn, iInLen, sOut, &iOutLen ) ;

    /**
      check ctrl flag
    */
    STRNCPY ( sCtrFlag, s_stTsec.ptTsecAPI[a_iHandler].pstIP[iFuncIdx].stItem.sCtrFlag, sizeof(sCtrFlag) ) ;
    tlog_debug(g_pstCat,0,0, "DEBUG: tsec_ip_oper_i, func %i ctrl flag '%s' ", a_iHandler, sCtrFlag ) ;
    if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUIRED ) )
    {
      /**< only return when all REQUIRED module invoked */
      if ( TSEC_OK == iRet )
      {
        /**< just continue */

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_ip_oper_i, func %i ctrl flag '%s' failed", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_REQUISITE ) )
    {
      /**< if invoked successfully, continue invoked other functions; otherwise, return immediately */
      if ( TSEC_OK == iRet )
      {

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_ip_oper_i, func %i ctrl flag '%s' failed, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ;
        break ; /**< loop break */
      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_SUFFICIENT ) )
    {
      /**< if invoked successfully, just return; otherwise, continue invoke other functions */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_ip_oper_i, func %i ctrl flag '%s' ok, loop break", a_iHandler, sCtrFlag ) ;
        iErrCode = iRet ; /**< reset iErrCode */
        break ; /**< loop break */
      }
      else
      {
        /**< just continue, do not set iErrCode */

      }

    }
    else if ( 0 == strcasecmp ( sCtrFlag, TSEC_CTRL_OPTIONAL ) )
    {
      /**< skip any successful or failed invoked result */
      if ( TSEC_OK == iRet )
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_ip_oper_i, func %i ctrl flag '%s' ok, loop continue", a_iHandler, sCtrFlag ) ;

      }
      else
      {
        tlog_debug(g_pstCat,0,0, "DEBUG: tsec_ip_oper_i, func %i ctrl flag '%s' failed, loop continue", a_iHandler, sCtrFlag ) ;
      }

    }
    else
    {
      tlog_error(g_pstCat,0,0, "ERROR: tsec_ip_oper_i %i function unknown ctrl flag", a_iHandler ) ;
      iErrCode = TSEC_CTRL_UNKNOWN ;
      break ;
    }

    iFuncIdx ++ ; /**< point to next function */

  } while ( (iFuncIdx < s_stTsec.ptTsecAPI[a_iHandler].iAsyCnts) && (TSEC_TRANS_SET == s_stTsec.ptTsecAPI[a_iHandler].iTrans) ) ;


  tlog_debug(g_pstCat,0,0, "RETN: tsec_ip_oper_i(%i, '%s', %i)=%i, ErrCode %i", a_iHandler, a_pszValue, a_iLen, iRet, iErrCode );

  return ( (0 == iErrCode)? iRet : iErrCode ) ;
}

