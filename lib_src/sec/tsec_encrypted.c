/** @file $RCSfile: tsec_encrypted.c,v $
  Various enctpyted and its' dsycrypted functions for tsf4g-security module
  $Id: tsec_encrypted.c,v 1.3 2008/08/13 04:03:49 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008/08/13 04:03:49 $
@version 1.0
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include <openssl/des.h>
#include <openssl/aes.h>
#include <openssl/evp.h>

#include "sec/tsec.h"
#include "sec/tsec_i.h"


TSECSET s_stTsec ;
TSECGLOBAL mstTsecIni ;

static int s_iInited ;
static TSECARGS s_stValues ;

LPTLOGCATEGORYINST g_CryptoLog = NULL;


/**
  various internal function defined
*/
int tsec_so_des_ecb_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;

int tsec_so_des_ecb_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;

int tsec_so_3des_ecb_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;

int tsec_so_3des_ecb_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;

int tsec_so_aes_cbc_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiAesBits
) ;

int tsec_so_aes_cbc_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiAesBits
) ;

int tsec_so_blowfish_cbc_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;

int tsec_so_blowfish_cbc_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;

#ifndef OPENSSL_NO_IDEA
int tsec_so_idea_cbc_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;

int tsec_so_idea_cbc_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
) ;
#endif




/**
  @brief Global inited function
  @retval 0 -- service inited ok
  @retval !0 -- service inited failed
  @note This function should be invoked before using any tsec functions
*/
TSEC_API int tsec_so_init ( IN const char *a_pszBuffer, IN LPTLOGCATEGORYINST a_pstLogCat   )
{
  int iRet = TSEC_OK ;

  /**
    init debug log
  */
  if (NULL != a_pstLogCat)
  {
	  g_CryptoLog = a_pstLogCat;
  }

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_init('%s')", a_pszBuffer ) ;

  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;

  s_iInited = SVC_INITED ;

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_init('%s')=%i", a_pszBuffer, iRet ) ;

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
  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_init_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (0 >= argc) || (NULL == argv) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_init_ex ( )=%i", iRet ) ;
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
  char sKey[SIZE_16B] = { 0 },
       sType[SIZE_16B] = { 0 } ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_process(\"%s\", %i, data, %i, apvBuffer, %i)", argsbuf, argsLen, aiDataLen, *apiLen ) ;

  /**
    parse buffer first
  */
  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;
  s_stValues.iArgsCnt = tsec_parse_args ( argsbuf, s_stValues.stArgs, TSEC_MAX_ARGS ) ;
  tlog_debug(g_CryptoLog,0,0,"DEBUG: tsec_process buffer parse item count %i", s_stValues.iArgsCnt ) ;

  tsec_get_value ( CRYPTO_KEY, sKey, sizeof(sKey), s_stValues ) ;
  tsec_get_value ( CRYPTO_TYPE, sType, sizeof(sType), s_stValues ) ;

  if ( 0 == strcasecmp ( CRYPTO_DES_ECB, sType ) ) {
    iRet = tsec_so_des_ecb_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_3DES_ECB, sType ) ) {
    iRet = tsec_so_3des_ecb_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_AES_128, sType ) ) {
    iRet = tsec_so_aes_cbc_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen, AES_128_BITS ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_AES_192, sType ) ) {
    iRet = tsec_so_aes_cbc_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen, AES_192_BITS ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_AES_256, sType ) ) {
    iRet = tsec_so_aes_cbc_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen, AES_256_BITS ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_BLOWFISH_CBC, sType ) ) {
    iRet = tsec_so_blowfish_cbc_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;

#ifndef OPENSSL_NO_IDEA
  } else if ( 0 == strcasecmp ( CRYPTO_IDEA_CBC, sType ) ) {
    iRet = tsec_so_idea_cbc_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;
#endif

  } else {
    /**< default is des_ecb mode */
    iRet = tsec_so_des_ecb_encrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;
  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_process( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

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
  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_process_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_process_ex ( )=%i", iRet ) ;
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
  char sKey[SIZE_16B] = { 0 },
       sType[SIZE_16B] = { 0 } ;

  tlog_debug(g_CryptoLog,0,0, "CALL: tsec_de_process(\"%s\", %i, data, %i, apvBuffer, %i)", argsbuf, argsLen, aiDataLen, *apiLen);

  /**
    parse buffer first
  */
  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;
  s_stValues.iArgsCnt = tsec_parse_args ( argsbuf, s_stValues.stArgs, TSEC_MAX_ARGS ) ;
  tlog_debug(g_CryptoLog,0,0,"DEBUG: tsec_de_process buffer parse item count %i", s_stValues.iArgsCnt ) ;

  tsec_get_value ( CRYPTO_KEY, sKey, sizeof(sKey), s_stValues ) ;
  tsec_get_value ( CRYPTO_TYPE, sType, sizeof(sType), s_stValues ) ;

  if ( 0 == strcasecmp ( CRYPTO_DES_ECB, sType ) ) {
    iRet = tsec_so_des_ecb_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_3DES_ECB, sType ) ) {
    iRet = tsec_so_3des_ecb_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_AES_128, sType ) ) {
    iRet = tsec_so_aes_cbc_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen, AES_128_BITS ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_AES_192, sType ) ) {
    iRet = tsec_so_aes_cbc_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen, AES_192_BITS ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_AES_256, sType ) ) {
    iRet = tsec_so_aes_cbc_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen, AES_256_BITS ) ;

  } else if ( 0 == strcasecmp ( CRYPTO_BLOWFISH_CBC, sType ) ) {
    iRet = tsec_so_blowfish_cbc_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;

#ifndef OPENSSL_NO_IDEA
  } else if ( 0 == strcasecmp ( CRYPTO_IDEA_CBC, sType ) ) {
    iRet = tsec_so_idea_cbc_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;
#endif

  } else {
    /**< default is des_ecb mode */
    iRet = tsec_so_des_ecb_decrypt ( sKey, apvData, aiDataLen, apvBuffer, apiLen ) ;
  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_de_process( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

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
  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_de_process_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_de_process_ex ( )=%i", iRet ) ;
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

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_fini ( )" ) ;


  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_fini ( )=%i", iRet ) ;

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
  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_fini_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_fini_ex ( )=%i", iRet ) ;
  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_des_ecb_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;
  int iMod = 0,
      iLen = 0 ;
  int i = 0 ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;
  unsigned char sBuffer[SIZE_16B] = { 0 } ;

  DES_cblock sDesKey = { 0 } ;
  DES_key_schedule sched ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_des_ecb_encrypt ( \"%s\", data, %i, apvBuffer, %i )", sKey, aiDataLen, *apiLen ) ;

  memset ( &sched, 0, sizeof(sched) ) ;

  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  /**
    generate key
  */
  {
    DES_string_to_key ( sKey, &sDesKey ) ;
    DES_set_key_unchecked ( &sDesKey, &sched ) ;
  }

  /**
    encrypt
  */
  iMod = aiDataLen % 8 ;
  iLen = aiDataLen - iMod ;

  for ( i=0; i<iLen; i+=8 ) {
    DES_ecb_encrypt ( (DES_cblock *)&(pchInBuf[i]), (DES_cblock *)(&(pchOutBuf[i])), &sched, DES_ENCRYPT ) ;
  }

  /**
    set odd bytes
  */
  if ( 0 != iMod ) {
    for ( i=0; i<iMod; i++ ) sBuffer[i] = pchInBuf[iLen+i] ;
  }
  sBuffer[7] = iMod ; /**< last byte set mod length */

  DES_ecb_encrypt ( (DES_cblock *)sBuffer, (DES_cblock *)(&(pchOutBuf[iLen])), &sched, DES_ENCRYPT ) ;
 
  /**
    total len
  */
  *apiLen = aiDataLen + 8 - iMod ;

  /**
    resources releases
  */
  {

  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_des_ecb_encrypt(..., OutBuf, OutLen=%i)=%i, verify bit %i", *apiLen, iRet, iMod ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_des_ecb_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;
  int iLen = 0,
      iLast = 0 ;
  int i = 0 ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  DES_cblock sDesKey = { 0 } ;
  DES_key_schedule sched ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_des_ecb_decrypt( )" ) ;

  memset ( &sched, 0, sizeof(sched) ) ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  /**
    check buffer
  */
  if ( 0 != aiDataLen % 8 ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: input data buffer len error %i%8 != 0", aiDataLen ) ;
    iRet = TSEC_ARG_ERROR ;
    return iRet ;
  }

  /**
    generate key
  */
  {
    DES_string_to_key ( sKey, &sDesKey ) ;
    DES_set_key_unchecked ( &sDesKey, &sched ) ;
  }

  /**
    descrypt
  */
  iLen = aiDataLen ;

  for ( i=0; i<iLen; i+=8 ) {
    DES_ecb_encrypt ( (DES_cblock *)&(pchInBuf[i]), (DES_cblock *)(&(pchOutBuf[i])), &sched, DES_DECRYPT ) ;
  }

  iLast = pchOutBuf[iLen - 1] ;

  if ( (7 < iLast) || (0 > iLast) ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: buffer decrypted failed, verify bit wrong %i", iLast ) ;
    iRet = TSEC_ARG_ERROR ;
  } else *apiLen = iLen - 8 + iLast ;


  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_des_ecb_decrypt(..., OutBuf, OutLen=%i)=%i, verify bit %i", *apiLen, iRet, iLast ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_3des_ecb_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;
  int iMod = 0,
      iLen = 0 ;
  int i = 0 ;

  DES_cblock sIn ;
  DES_cblock sOut ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;
  unsigned char sBuffer[SIZE_16B] = { 0 } ;

  DES_cblock sDesKey1 = { 0 },
             sDesKey2 = { 0 } ;
  DES_key_schedule sched, sched2 ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_3des_ecb_encrypt ( \"%s\", data, %i, apvBuffer, %i )", sKey, aiDataLen, *apiLen ) ;

  memset ( &sched, 0, sizeof(sched) ) ;
  memset ( &sched2, 0, sizeof(sched2) ) ;

  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  /**
    generate key
  */
  {
    DES_string_to_2keys ( sKey, &sDesKey1, &sDesKey2 ) ;
    DES_set_key_unchecked ( &sDesKey2, &sched2 ) ;

    memset ( &sDesKey1, 0, sizeof(sDesKey1) ) ;
    memset ( &sDesKey2, 0, sizeof(sDesKey2) ) ;

    DES_string_to_key ( sKey, &sDesKey1 ) ;
    DES_set_key_unchecked ( &sDesKey1, &sched ) ;
  }

  /**
    encrypt
  */
  iMod = aiDataLen % 8 ;
  iLen = aiDataLen - iMod ;

  memset ( sIn, 0, sizeof(sIn) ) ;
  memset ( sOut, 0, sizeof(sOut) ) ;

  for ( i=0; i<iLen; i+=8 ) {
    memcpy ( sIn, &(pchInBuf[i]), 8 ) ;
    DES_ecb3_encrypt ( sIn, sOut, &sched, &sched2, &sched, DES_ENCRYPT ) ;
    memcpy ( &(pchOutBuf[i]), sOut, 8 ) ;
    /*DES_ecb3_encrypt ( (DES_cblock *)(&(pchInBuf[i])), (DES_cblock *)(&(pchOutBuf[i])), &sched, &sched2, &sched, DES_ENCRYPT ) ;*/
  }

  /**
    set odd bytes
  */
  if ( 0 != iMod ) {
    for ( i=0; i<iMod; i++ ) sBuffer[i] = pchInBuf[iLen+i] ;
  }
  sBuffer[7] = iMod ; /**< last byte set mod length */

  memcpy ( sIn, sBuffer, 8 ) ;
  DES_ecb3_encrypt ( sIn, sOut, &sched, &sched2, &sched, DES_ENCRYPT ) ;
  memcpy ( &(pchOutBuf[iLen]), sOut, 8 ) ;
  /*DES_ecb3_encrypt ( (DES_cblock *)(sBuffer), (DES_cblock *)(&(pchOutBuf[iLen])), &sched, &sched2, &sched, DES_ENCRYPT ) ;*/
 
  /**
    total len
  */
  *apiLen = aiDataLen + 8 - iMod ;

  /**
    resources releases
  */
  {

  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_3des_ecb_encrypt( ..., OutBuf, OutLen=%i )=%i, verify bit %i", *apiLen, iRet, iMod ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_3des_ecb_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  DES_cblock sDesKey1 = { 0 },
             sDesKey2 = { 0 } ;
  DES_key_schedule sched, sched2 ;

  int iLen = 0,
      iLast = 0 ;
  int i = 0 ;

  DES_cblock sIn ;
  DES_cblock sOut ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_3des_ecb_decrypt ( \"%s\", data, %i, apvBuffer, %i )", sKey, aiDataLen, *apiLen ) ;

  memset ( &sched, 0, sizeof(sched) ) ;
  memset ( &sched2, 0, sizeof(sched2) ) ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  /**
    check buffer
  */
  if ( 0 != aiDataLen % 8 ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: tsec_so_3des_ecb_decrypt input data buffer len error %i%8 != 0", aiDataLen ) ;
    iRet = TSEC_ARG_ERROR ;
    return iRet ;
  }

  /**
    generate key
  */
  {
    DES_string_to_2keys ( sKey, &sDesKey1, &sDesKey2 ) ;
    DES_set_key_unchecked ( &sDesKey2, &sched2 ) ;

    memset ( &sDesKey1, 0, sizeof(sDesKey1) ) ;
    memset ( &sDesKey2, 0, sizeof(sDesKey2) ) ;

    DES_string_to_key ( sKey, &sDesKey1 ) ;
    DES_set_key_unchecked ( &sDesKey1, &sched ) ;
  }

  /**
    descrypt
  */
  memset ( sIn, 0, sizeof(sIn) ) ;
  memset ( sOut, 0, sizeof(sOut) ) ;

  iLen = aiDataLen ;

  for ( i=0; i<iLen; i+=8 ) {
    memcpy ( sIn, &(pchInBuf[i]), 8 ) ;
    DES_ecb3_encrypt ( sIn, sOut, &sched, &sched2, &sched, DES_DECRYPT ) ;
    memcpy ( &(pchOutBuf[i]), sOut, 8 ) ;
    /*DES_ecb3_encrypt ( (DES_cblock *)&(pchInBuf[i]), (DES_cblock *)(&(pchOutBuf[i])), &sched, &sched2, &sched, DES_DECRYPT ) ;*/
  }

  iLast = pchOutBuf[iLen - 1] ;

  if ( (7 < iLast) || (0 > iLast) ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: buffer decrypted failed, verify bit wrong %i", iLast ) ;
    iRet = TSEC_ARG_ERROR ;
  }
  else
  {
    *apiLen = iLen - 8 + iLast ;
  }

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_3des_ecb_decrypt( ..., OutBuf, OutLen=%i )=%i, verify bit %i", *apiLen, iRet, iLast ) ;

  return iRet ;
}


/**
  union to facilitate the conversion of an off_t to an array of bytes
*/
typedef union uniAes {   
  unsigned char bytes[AES_BLOCK_SIZE];
  off_t size;
} off_u;

/**
  @brief Encrypts data via speified by aiAesBits: AES-128-CBC / AES-196-CBC / AES-256-CBC
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_aes_cbc_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiAesBits
)
{
  int iRet = TSEC_OK,
      iOutLen = 0 ;
  int iMod = 0,
      iLen = 0 ;
  int i = 0 ;
  int iKeyLen = 0 ;

  AES_KEY ctx;
  unsigned char sKeyBuf[SIZE_16B+2] = { 0 } ;
  unsigned char iv[AES_BLOCK_SIZE] = { 0 } ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  unsigned char szInBuf[AES_BLOCK_SIZE] = { 0 } ;

  off_u uniBytes ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_aes_cbc_encrypt(\"%s\", data, %i, apvBuffer, %i, %i)",
      sKey, aiDataLen, *apiLen, aiAesBits ) ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  /**
    set up the AES key structure
  */
  iKeyLen = strlen ( sKey ) ;
  memcpy ( sKeyBuf, sKey, ((iKeyLen > 16)? 16 : iKeyLen) ) ;
  AES_set_encrypt_key(sKeyBuf, aiAesBits, &ctx) ;

  /**
    write the size as the first block of the data
  */
  uniBytes.size = aiDataLen ;
  AES_cbc_encrypt ( uniBytes.bytes, &(pchOutBuf[iOutLen]), AES_BLOCK_SIZE, &ctx, iv, AES_ENCRYPT);
  iOutLen += AES_BLOCK_SIZE ;

  /**
    encrypt with AES
  */
  iMod = aiDataLen % AES_BLOCK_SIZE ;
  iLen = aiDataLen - iMod ;

  for ( i=0; i<iLen; i+=AES_BLOCK_SIZE, iOutLen+=AES_BLOCK_SIZE ) {
    AES_cbc_encrypt ( &(pchInBuf[i]), &(pchOutBuf[iOutLen]), AES_BLOCK_SIZE, &ctx, iv, AES_ENCRYPT);
  }

  /**
    set last bytes
  */
  if ( 0 != iMod ) {
    for ( i=0; i<iMod; i++ ) szInBuf[i] = pchInBuf[iLen+i] ;
  }
  AES_cbc_encrypt ( szInBuf, &(pchOutBuf[iOutLen]), AES_BLOCK_SIZE, &ctx, iv, AES_ENCRYPT);
  iOutLen += AES_BLOCK_SIZE ;

  *apiLen = iOutLen ;

  pchOutBuf[*apiLen] = '\0' ; /**< set null terminated */

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_aes_cbc_encrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_aes_cbc_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiAesBits
)
{
  int iRet = TSEC_OK ;
  int iLen = aiDataLen ;
  int i = 0,
      iOutLen = 0 ;

  AES_KEY ctx;
  unsigned char sKeyBuf[SIZE_16B+2] = { 0 } ;
  unsigned char iv[AES_BLOCK_SIZE] = { 0 } ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  off_u uniBytes ;

  int iKeyLen = 0 ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_aes_cbc_decrypt( \"%s\", data, %i, apvBuffer, %i, %i )",
      sKey, aiDataLen, *apiLen, aiAesBits ) ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  /**
    check buffer
  */
  if ( 0 != aiDataLen % AES_BLOCK_SIZE ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: tsec_so_aes_cbc_decrypt input data buffer len error %i%8 != 0", aiDataLen ) ;
    iRet = TSEC_ARG_ERROR ;
    return iRet ;
  }

  /**
    set up the AES key structure
  */
  iKeyLen = strlen ( sKey ) ;
  memcpy ( sKeyBuf, sKey, ((iKeyLen > 16)? 16 : iKeyLen) ) ;
  AES_set_decrypt_key(sKeyBuf, aiAesBits, &ctx) ;

  /**
    read the size from the first block of the data
  */
  AES_cbc_encrypt ( &(pchInBuf[0]), uniBytes.bytes, AES_BLOCK_SIZE, &ctx, iv, AES_DECRYPT ) ;
  *apiLen = uniBytes.size ;

  /**
    descrypt with AES
  */
  for ( i=AES_BLOCK_SIZE; i<iLen; i+=AES_BLOCK_SIZE, iOutLen+=AES_BLOCK_SIZE ) {
    AES_cbc_encrypt ( &(pchInBuf[i]), &(pchOutBuf[iOutLen]), AES_BLOCK_SIZE, &ctx, iv, AES_DECRYPT ) ;
  }
  pchOutBuf[*apiLen] = '\0' ; /**< set null terminated */

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_aes_cbc_decrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_blowfish_cbc_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK,
      iOutLen = 0,
      iLastLen = 0 ;
  unsigned char sKeyBuf[SIZE_16B] = { 0 } ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  EVP_CIPHER_CTX ctx;
  unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 } ;

  int iKeyLen = 0 ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  iKeyLen = strlen ( sKey ) ;
  memcpy ( sKeyBuf, sKey, ((iKeyLen > 16)? 16 : iKeyLen) ) ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_blowfish_cbc_encrypt( \"%s\", data, %i, apvBuffer, %i )", sKey, aiDataLen, *apiLen ) ;

  EVP_CIPHER_CTX_init(&ctx);

  if ( !EVP_EncryptInit(&ctx, EVP_bf_cbc(), sKeyBuf, iv) ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_EncryptInit() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_EncryptUpdate(&ctx, pchOutBuf, &iOutLen, pchInBuf, aiDataLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_EncryptUpdate() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_EncryptFinal_ex(&ctx, &(pchOutBuf[iOutLen]), &iLastLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_EncryptFinal() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  iOutLen += iLastLen ;

  EVP_CIPHER_CTX_cleanup(&ctx);

  *apiLen = iOutLen ;

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_blowfish_cbc_encrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_blowfish_cbc_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK,
      iOutLen = 0,
      iLastLen = 0 ;
  unsigned char sKeyBuf[SIZE_16B+2] = { 0 } ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  EVP_CIPHER_CTX ctx;
  unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 } ;

  int iKeyLen = 0 ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  iKeyLen = strlen ( sKey ) ;
  memcpy ( sKeyBuf, sKey, ((iKeyLen > 16)? 16 : iKeyLen) ) ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_blowfish_cbc_decrypt( \"%s\", data, %i, apvBuffer, %i )", sKey, aiDataLen, *apiLen ) ;

  EVP_CIPHER_CTX_init(&ctx);

  if ( !EVP_DecryptInit(&ctx, EVP_bf_cbc(), sKeyBuf, iv) ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_DecryptInit() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_DecryptUpdate(&ctx, pchOutBuf, &iOutLen, pchInBuf, aiDataLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_DecryptUpdate() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_DecryptFinal_ex(&ctx, &(pchOutBuf[iOutLen]), &iLastLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_DecryptFinal() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  iOutLen += iLastLen ;

  EVP_CIPHER_CTX_cleanup(&ctx);

  *apiLen = iOutLen ;

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_blowfish_cbc_decrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}



#ifndef OPENSSL_NO_IDEA
/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_idea_cbc_encrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK,
      iOutLen = 0,
      iLastLen = 0 ;
  unsigned char sKeyBuf[SIZE_16B] = { 0 } ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  EVP_CIPHER_CTX ctx;
  unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 } ;

  int iKeyLen = 0 ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  iKeyLen = strlen ( sKey ) ;
  memcpy ( sKeyBuf, sKey, ((iKeyLen > 16)? 16 : iKeyLen) ) ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_idea_cbc_encrypt( \"%s\", data, %i, apvBuffer, %i )", sKey, aiDataLen, *apiLen ) ;

  EVP_CIPHER_CTX_init(&ctx);

  if ( !EVP_EncryptInit(&ctx, EVP_idea_cbc(), sKeyBuf, iv) ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_EncryptInit() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_EncryptUpdate(&ctx, pchOutBuf, &iOutLen, pchInBuf, aiDataLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_EncryptUpdate() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_EncryptFinal_ex(&ctx, &(pchOutBuf[iOutLen]), &iLastLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_EncryptFinal() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  iOutLen += iLastLen ;

  EVP_CIPHER_CTX_cleanup(&ctx);

  *apiLen = iOutLen ;

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_idea_cbc_encrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_idea_cbc_decrypt (
  IN const char *sKey,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK,
      iOutLen = 0,
      iLastLen = 0 ;
  unsigned char sKeyBuf[SIZE_16B+2] = { 0 } ;

  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  EVP_CIPHER_CTX ctx;
  unsigned char iv[EVP_MAX_IV_LENGTH] = { 0 } ;

  int iKeyLen = 0 ;

  *apiLen = 0 ;
  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  iKeyLen = strlen ( sKey ) ;
  memcpy ( sKeyBuf, sKey, ((iKeyLen > 16)? 16 : iKeyLen) ) ;

  tlog_debug(g_CryptoLog,0,0,"CALL: tsec_so_idea_cbc_decrypt( \"%s\", data, %i, apvBuffer, %i )", sKey, aiDataLen, *apiLen ) ;

  EVP_CIPHER_CTX_init(&ctx);

  if ( !EVP_DecryptInit(&ctx, EVP_idea_cbc(), sKeyBuf, iv) ) {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_DecryptInit() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_DecryptUpdate(&ctx, pchOutBuf, &iOutLen, pchInBuf, aiDataLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_DecryptUpdate() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  if ( !EVP_DecryptFinal_ex(&ctx, &(pchOutBuf[iOutLen]), &iLastLen) )
  {
    tlog_error(g_CryptoLog,0,0, "ERROR: EVP_DecryptFinal() failed\n" ) ;
    iRet = TSEC_SYS_ERROR ;
    return iRet ;
  }

  iOutLen += iLastLen ;

  EVP_CIPHER_CTX_cleanup(&ctx);

  *apiLen = iOutLen ;

  tlog_debug(g_CryptoLog,0,0,"RETN: tsec_so_idea_cbc_decrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}
#endif

