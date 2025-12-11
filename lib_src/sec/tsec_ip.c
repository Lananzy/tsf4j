/** @file $RCSfile: tsec_ip.c,v $
  general description of this module
  $Id: tsec_ip.c,v 1.3 2008/08/13 04:03:49 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008/08/13 04:03:49 $
@version $Revision: 1.3 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/




#include "sec/tsec.h"
#include "sec/tsec_i.h"
#include "tdr/tdr.h"
#include "tdr/tdr_metalib_manage.h"

#include "tsec_ipacl_desc.h"


TSECSET s_stTsec ;
TSECGLOBAL mstTsecIni ;

int s_iInited ;
TSECARGS s_stValues ;

LPTLOGCATEGORYINST g_IpLog ;

TSECIPACL *g_pstIPAcl = NULL ;

extern unsigned char g_szMetalib_tsec_ip[] ;
char g_szRuleFile[SIZE_256B] ;


/**
  various internal function defined
*/
int tsec_ip_load_rules_i ( ) ;
int tsec_ip_addrule_i ( IN const char *a_pszSrcIP, IN const char *a_pszDstIP, IN const int a_iLimit, INOUT TSECIPACL *a_pstRules ) ;
int tsec_ip_scope ( IN const char *a_pszIP, INOUT unsigned long *a_plBegin, INOUT unsigned long *a_plEnd ) ;
unsigned long tsec_ip_netaddr ( IN const char *a_pszIP ) ;

int tsec_ip_counter_i ( IN const char *a_pszSrcIP, IN const int a_iFlag ) ;
int tsec_ip_reset_i ( IN const char *a_pszSrcIP ) ;
int tsec_srcip_verify_i ( IN const char *a_pszSrcIP ) ;
int tsec_dstip_verify_i ( IN const char *a_pszSrcIP, IN const char *a_pszDstIP ) ;


/**
  @brief Global inited function
  @retval 0 -- service inited ok
  @retval !0 -- service inited failed
  @note This function should be invoked before using any tsec functions
*/
TSEC_API int tsec_so_init ( IN const char *a_pszBuffer , IN LPTLOGCATEGORYINST a_pstLogCat    )
{
  int iRet = TSEC_OK ;

  TSECARGS stArgsSet ;

  /**
    init debug log
  */
  if (NULL != a_pstLogCat)
  {
	  g_IpLog = a_pstLogCat;
  }
  tlog_debug(g_IpLog,0,0, "CALL: tsec_so_init ( '%s' )", a_pszBuffer ) ;

  memset ( &stArgsSet, 0, sizeof(stArgsSet) ) ;

  stArgsSet.iArgsCnt = tsec_parse_args ( a_pszBuffer, stArgsSet.stArgs, TSEC_MAX_ARGS ) ;
  tlog_debug(g_IpLog,0,0, "DEBUG: ip verify tsec_so_init buffer parse item count %i", stArgsSet.iArgsCnt ) ;

  tsec_get_value ( IP_RULEFILE, g_szRuleFile, sizeof(g_szRuleFile), stArgsSet ) ;

  iRet = tsec_ip_load_rules_i ( ) ;
  if ( TSEC_OK == iRet )
  {
    s_iInited = SVC_INITED ;
  }
  else
  {
    /* module init failed */
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_so_init ( args )=%i, rule file '%s'", iRet, g_szRuleFile ) ;

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
  tlog_debug(g_IpLog,0,0, "CALL: tsec_init_ex()" ) ;

  /* just for annoying warning messages from complier */
  if ( (0 >= argc) || (NULL == argv) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_init_ex()=%i", iRet ) ;
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
  char sType[SIZE_16B] = { 0 },
       sSrcIP[SIZE_32B] = { 0 },
       sDstIP[SIZE_32B] = { 0 } ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_process ( \"%s\", %i, '%s', %i, OutBuffer, %i )",
      argsbuf, argsLen, (char *)apvData, aiDataLen, *apiLen ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == apvBuffer) )
  {
    ;
  }

  /**
    parse buffer first
  */
  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;
  s_stValues.iArgsCnt = tsec_parse_args ( argsbuf, s_stValues.stArgs, TSEC_MAX_ARGS ) ;
  tlog_debug(g_IpLog,0,0, "DEBUG: tsec_process buffer parse item count %i", s_stValues.iArgsCnt ) ;

  tsec_get_value ( IP_VERIFY_TYPE, sType, sizeof(sType), s_stValues ) ;
  tsec_get_value ( IP_SRC, sSrcIP, sizeof(sSrcIP), s_stValues ) ;
  tsec_get_value ( IP_DST, sDstIP, sizeof(sDstIP), s_stValues ) ;

  if ( 0 == strcasecmp ( TSEC_IP_REG, sType ) ) {
    iRet = tsec_ip_counter_i ( sSrcIP, IP_ADD_COUNTER ) ;

  } else if ( 0 == strcasecmp ( TSEC_IP_UNREG, sType ) ) {
    iRet = tsec_ip_counter_i ( sSrcIP, IP_DEC_COUNTER ) ;

  } else if ( 0 == strcasecmp ( TSEC_IP_RESET, sType ) ) {
    iRet = tsec_ip_reset_i ( sSrcIP ) ;

  } else if ( 0 == strcasecmp ( TSEC_IP_SRCIP, sType ) ) {
    iRet = tsec_srcip_verify_i ( sSrcIP ) ;

  } else if ( 0 == strcasecmp ( TSEC_IP_DSTIP, sType ) ) {
    iRet = tsec_dstip_verify_i ( sSrcIP, sDstIP ) ;

  } else {
    tlog_error(g_IpLog,0,0, "ERROR: tsec_process(), unknown type %s", sType ) ; 
    iRet = TSEC_ARG_ERROR ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_process()=%i", *apiLen, iRet ) ;

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
  tlog_debug(g_IpLog,0,0, "CALL: tsec_process_ex()" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_process_ex()=%i", iRet ) ;
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
  tlog_debug(g_IpLog,0,0, "CALL: tsec_de_process( \"%s\", %i, data, %i, apvBuffer, %i )",
      argsbuf, argsLen, aiDataLen, *apiLen ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == apvData ) || (NULL == apvBuffer) )
  {
    ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_de_process( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;
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
  tlog_debug(g_IpLog,0,0, "CALL: tsec_de_process_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_de_process_ex ( )=%i", iRet ) ;
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

  tlog_debug(g_IpLog,0,0, "CALL: tsec_fini ( )" ) ;


  tlog_debug(g_IpLog,0,0, "RETN: tsec_fini ( )=%i", iRet ) ;

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
  tlog_debug(g_IpLog,0,0, "CALL: tsec_fini_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_fini_ex ( )=%i", iRet ) ;
  return iRet ;
}


/**
  @brief read rules from file
  @retval 0 -- ininted ok
  @retval !0 -- ininted failed
  @note
*/
int tsec_ip_load_rules_i ( )
{
  int iRet = TSEC_OK ;
  int i = 0 ;
  int iLimit = 0 ;

  LPTDRMETALIB ptLib = NULL ;
  LPTDRMETA pstMeta = NULL ;
  TDRDATA stIPAcl ;
  struct tagIPACL stRawRules  ;

  TSECIPACL *pstRules = NULL ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_ip_load_rules_i()" ) ;

  /**
    read acl from xml file
  */
  ptLib = (LPTDRMETALIB) g_szMetalib_tsec_ip ;
  pstMeta = tdr_get_meta_by_name ( ptLib, TSEC_IPACL_META ) ;
  if ( NULL == pstMeta )
  {
    tlog_error(g_IpLog,0,0, "ERROR: tsec_ip_load_rules_i() tdr_get_meta_by_name failed" ) ;
    return TSEC_SYS_ERROR ;
  }

  memset ( &stIPAcl, 0, sizeof(stIPAcl) ) ;
  memset ( &stRawRules, 0, sizeof(stRawRules) ) ;

  stIPAcl.iBuff = sizeof(stRawRules) ;
  stIPAcl.pszBuff = (char *)&stRawRules ;

  iRet = tdr_input_file ( pstMeta, &stIPAcl, g_szRuleFile, TSEC_IP_CUR_VERSION, 2 ) ;
  if ( 0 > iRet )
  {
    tlog_error(g_IpLog,0,0, "ERROR: tsec_ip_load_rules_i() tdr_input_file failed %i", iRet ) ;
    return TSEC_SYS_ERROR ;
  }

  pstRules = ( TSECIPACL * ) malloc ( sizeof(TSECIPACL) ) ;
  if ( NULL == pstRules )
  {
    tlog_error(g_IpLog,0,0, "ERROR: tsec_ip_load_rules_i() malloc %ibytes failed", sizeof(TSECIPACL) ) ;
    return TSEC_SYS_ERROR ;
  }
  memset ( pstRules, 0, sizeof(TSECIPACL) ) ;

  /**
    parse raw acl
  */
  if ( 0 == strcasecmp ( stRawRules.szDefaultRule, TSEC_IP_ALLOW ) )
  {
    pstRules->iDefault = TSEC_IP_ALLOW_IDX ;
  }
  else
  {
    pstRules->iDefault = TSEC_IP_DENY_IDX ;
  }

  for ( i=0; i<stRawRules.iLimitCnt; i++ )
  {
    if ( 0 == strcasecmp ( stRawRules.astLimits[i].szLimit, TSEC_IP_ALLOW ) )
    {
      iLimit = TSEC_IP_ALLOW_IDX ;
    }
    else if ( 0 == strcasecmp ( stRawRules.astLimits[i].szLimit, TSEC_IP_DENY ) )
    {
      iLimit = TSEC_IP_DENY_IDX ;
    }
    else
    {
      iLimit = atoi(stRawRules.astLimits[i].szLimit) ;
      if ( 0 > iLimit )
      {
        iLimit = TSEC_IP_ALLOW_IDX ;
      }
      else if ( 0 == iLimit )
      {
        iLimit = TSEC_IP_DENY_IDX ;
      }
    }

    tsec_ip_addrule_i ( stRawRules.astLimits[i].szSrcIP, stRawRules.astLimits[i].szDstIP, iLimit, pstRules ) ;
  } /**< end raw ip acl parse */

  if ( NULL != g_pstIPAcl )
  {
    free ( g_pstIPAcl ) ;
    g_pstIPAcl = NULL ;
  }
  g_pstIPAcl = pstRules ;

  tlog_debug(g_IpLog,0,0, "RETN: tsec_ip_load_rules_i()=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_ip_addrule_i ( IN const char *a_pszSrcIP, IN const char *a_pszDstIP, IN const int a_iLimit, INOUT TSECIPACL *a_pstRules )
{
  int iRet = TSEC_OK,
      iCnt = 0 ;
  unsigned long lStartIP = 0,
                lEndIP = 0 ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_ip_addrule_i ( '%s', '%s', %i, TSECIPACL * )", a_pszSrcIP, a_pszDstIP, a_iLimit ) ;

  iCnt = a_pstRules->iLimitCnt ;

  if ( iCnt < TSEC_MAX_IP_RULES )
  {
    if ( (isdigit(a_pszSrcIP[0])) && (0 == tsec_ip_scope(a_pszSrcIP, &lStartIP, &lEndIP)) )
    {
      a_pstRules->stLimitIPs[iCnt].llSrcStart = lStartIP ;
      a_pstRules->stLimitIPs[iCnt].llSrcEnd = lEndIP ;
      a_pstRules->stLimitIPs[iCnt].iLimit = a_iLimit ;
      a_pstRules->iLimitCnt ++ ;
    }
    else
    {
      tlog_debug(g_IpLog,0,0, "ERROR: tsec_ip_addrule_i() invalid source IP domain '%s'", a_pszSrcIP ) ;
    }
  }
  else
  {
    tlog_debug(g_IpLog,0,0, "ERROR: tsec_ip_addrule_i() max rules counts %i reach", iCnt ) ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_ip_addrule_i ( '%s', '%s', %i, TSECIPACL * )=%i", a_pszSrcIP, a_pszDstIP, a_iLimit, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note only support format like 'IP/NetMask'
*/
int tsec_ip_scope ( IN const char *a_pszIP, INOUT unsigned long *a_plBegin, INOUT unsigned long *a_plEnd )
{
  int iRet = TSEC_OK,
      iMask = 0 ;
  char sIP[SIZE_128B] = { 0 },
       sNet[SIZE_32B] = { 0 },
       sMask[SIZE_32B] = { 0 } ;
  long lMask = 0,
       lNet = 0 ;
  char *pcIdx = NULL ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_ip_scope( '%s', unsigned long, unsigned long )", a_pszIP ) ;

  STRNCPY ( sIP, a_pszIP, sizeof(sIP) ) ; 

  *a_plBegin = *a_plEnd = 0 ;

  if ( NULL != (pcIdx = strchr(sIP, '/')) )
  {
    /**< format like 'IP/NetMask' */
    *pcIdx = '\0' ;
    STRNCPY ( sNet, sIP, sizeof(sNet) ) ;

    pcIdx ++ ;
    STRNCPY ( sMask, pcIdx, sizeof(sMask) ) ;
    iMask = atoi(pcIdx) ;

    if ( 32 < iMask )
    {
      iMask = 32 ;
    }
    else if ( 0 > iMask )
    {
      iMask = 0 ;
    }

    lMask = ~0 ;
    if ( 0 < iMask )
    {
      lMask <<= (32 - iMask) ;
    }

    lNet = tsec_ip_netaddr(sNet) ;

    *a_plBegin = lNet & lMask ;
    *a_plEnd = lNet | (~lMask ) ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_ip_scope( '%s', %li, %li )=%i", a_pszIP, *a_plBegin, *a_plEnd, iRet ) ;
  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
unsigned long tsec_ip_netaddr ( IN const char *a_pszIP )
{
  char sIP[SIZE_128B] = { 0 } ;
  char *pcIdx = NULL ;

  unsigned long lRet = TSEC_OK ;
  char *pcValue = (char *) &lRet;

  char s0[SIZE_16B], s1[SIZE_16B], s2[SIZE_16B], s3[SIZE_16B];

  tlog_debug(g_IpLog,0,0, "CALL: tsec_ip_netaddr( '%s' )", a_pszIP ) ;

  STRNCPY ( sIP, a_pszIP, sizeof(sIP) ) ; 

  if ( NULL != ( pcIdx = strchr ( sIP, '.' ) ) )
  {
    /**< first node */
    *pcIdx = '\0' ;
    STRNCPY ( s0, sIP, sizeof(s0) ) ;

    pcIdx ++ ;
    STRNCPY ( sIP, pcIdx, sizeof(sIP) ) ;
    if ( NULL != ( pcIdx = strchr ( sIP, '.' ) ) )
    {
      /**< second node */
      *pcIdx = '\0' ;
      STRNCPY ( s1, sIP, sizeof(s1) ) ;
  
      pcIdx ++ ;
      STRNCPY ( sIP, pcIdx, sizeof(sIP) ) ;
      if ( NULL != ( pcIdx = strchr ( sIP, '.' ) ) )
      {
        /**< third node */
        *pcIdx = '\0' ;
        STRNCPY ( s2, sIP, sizeof(s2) ) ;
    
        /**< fourth node */
        pcIdx ++ ;
        STRNCPY ( s3, pcIdx, sizeof(s3) ) ;
      }
      else
      {
        tlog_debug(g_IpLog,0,0, "ERROR: tsec_ip_netaddr( '%s' ) error IP2", a_pszIP ) ;
        return (unsigned long)-1 ;
      }
    }
    else
    {
      tlog_debug(g_IpLog,0,0, "ERROR: tsec_ip_netaddr( '%s' ) error IP1", a_pszIP ) ;
      return (unsigned long)-1 ;
    }

  }
  else
  {
    tlog_debug(g_IpLog,0,0, "ERROR: tsec_ip_netaddr( '%s' ) error IP0", a_pszIP ) ;
    return (unsigned long)-1 ;
  }

#ifdef linux
  pcValue[3] = (char) atoi(s0);
  pcValue[2] = (char) atoi(s1);
  pcValue[1] = (char) atoi(s2);
  pcValue[0] = (char) atoi(s3);
#else
  pcValue[0] = (char) atoi(s0);
  pcValue[1] = (char) atoi(s1);
  pcValue[2] = (char) atoi(s2);
  pcValue[3] = (char) atoi(s3);
#endif

  tlog_debug(g_IpLog,0,0, "RETN: tsec_ip_netaddr( '%s' )=%ld", lRet ) ;

  return lRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_ip_counter_i ( IN const char *a_pszSrcIP, IN const int a_iFlag )
{
  int iRet = TSEC_OK ;
  int iFound = 0 ;
  unsigned long lIP = 0 ;
  int i=0 ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_ip_counter_i(%s, %i)", a_pszSrcIP, a_iFlag ) ;

  lIP = tsec_ip_netaddr(a_pszSrcIP) ;

  for ( i=0; i<g_pstIPAcl->iLimitCnt; i++ )
  {
    if ( (lIP >= g_pstIPAcl->stLimitIPs[i].llSrcStart) && (lIP <= g_pstIPAcl->stLimitIPs[i].llSrcEnd) )
    {
      /* found */
      if ( 0 < g_pstIPAcl->stLimitIPs[i].iLimit )
      {
        if ( IP_ADD_COUNTER == a_iFlag )
        {
          g_pstIPAcl->stLimitIPs[i].iCurCnt ++ ;
        }
        else
        {
          g_pstIPAcl->stLimitIPs[i].iCurCnt -- ;
        }

        if ( g_pstIPAcl->stLimitIPs[i].iCurCnt > g_pstIPAcl->stLimitIPs[i].iLimit )
        {
          iRet = TSEC_IP_DENY_IDX ;
        }
        else
        {
          iRet = TSEC_IP_ALLOW_IDX ;
        }
      }
      else
      {
        iRet = g_pstIPAcl->stLimitIPs[i].iLimit ;
      }

      iFound = 1 ;
      break ;
    }
  }

  if ( 0 == iFound )
  {
    /* use default rules */
    iRet = g_pstIPAcl->iDefault ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_ip_counter_i(%s, %i)=%i", a_pszSrcIP, a_iFlag, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_ip_reset_i ( IN const char *a_pszSrcIP )
{
  int iRet = TSEC_OK ;
  unsigned long lIP = 0 ;
  int i=0 ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_ip_reset_i(%s)", a_pszSrcIP ) ;

  lIP = tsec_ip_netaddr(a_pszSrcIP) ;

  for ( i=0; i<g_pstIPAcl->iLimitCnt; i++ )
  {
    if ( (lIP >= g_pstIPAcl->stLimitIPs[i].llSrcStart) && (lIP <= g_pstIPAcl->stLimitIPs[i].llSrcEnd) )
    {
      /* found */
      g_pstIPAcl->stLimitIPs[i].iCurCnt = 0 ;
      break ;
    }
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_ip_reset_i(%s)=%i", a_pszSrcIP, iRet ) ;

  return iRet ;
}

/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_srcip_verify_i ( IN const char *a_pszSrcIP )
{
  int iRet = TSEC_OK ;
  int iFound = 0 ;
  unsigned long lIP = 0 ;
  int i=0 ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_srcip_verify_i( %s )", a_pszSrcIP ) ;

  lIP = tsec_ip_netaddr(a_pszSrcIP) ;

  for ( i=0; i<g_pstIPAcl->iLimitCnt; i++ )
  {
    if ( (lIP >= g_pstIPAcl->stLimitIPs[i].llSrcStart) && (lIP <= g_pstIPAcl->stLimitIPs[i].llSrcEnd) )
    {
      /* found */
      if ( 0 < g_pstIPAcl->stLimitIPs[i].iLimit )
      {
        /* connections count limit, treat this check as passed */
        iRet = TSEC_IP_ALLOW_IDX ;
      }
      else
      {
        iRet = g_pstIPAcl->stLimitIPs[i].iLimit ;
      }

      iFound = 1 ;
      break ;
    }
  }

  if ( 0 == iFound )
  {
    /* use default rules */
    iRet = g_pstIPAcl->iDefault ;
  }

  tlog_debug(g_IpLog,0,0, "RETN: tsec_srcip_verify_i( %s )=%i", a_pszSrcIP, iRet ) ;
  return iRet ;
}

/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_dstip_verify_i ( IN const char *a_pszSrcIP, IN const char *a_pszDstIP )
{
  int iRet = TSEC_OK ;
  int iFound = 0 ;
  unsigned long lSrcIP = 0,
                lDstIP = 0 ;
  int i=0 ;

  tlog_debug(g_IpLog,0,0, "CALL: tsec_dstip_verify_i(%s, %s)", a_pszSrcIP, a_pszDstIP ) ;

  lSrcIP = tsec_ip_netaddr(a_pszSrcIP) ;
  lDstIP = tsec_ip_netaddr(a_pszDstIP) ;

  for ( i=0; i<g_pstIPAcl->iLimitCnt; i++ )
  {
    if ( (lSrcIP >= g_pstIPAcl->stLimitIPs[i].llSrcStart) && (lSrcIP <= g_pstIPAcl->stLimitIPs[i].llSrcEnd)
        && (lDstIP >= g_pstIPAcl->stLimitIPs[i].llDstStart) && (lDstIP <= g_pstIPAcl->stLimitIPs[i].llDstEnd) )
    {
      /* found */
      if ( 0 < g_pstIPAcl->stLimitIPs[i].iLimit )
      {
        /* connections count limit, treat this check as passed */
        iRet = TSEC_IP_ALLOW_IDX ;
      }
      else
      {
        iRet = g_pstIPAcl->stLimitIPs[i].iLimit ;
      }

      iFound = 1 ;
      break ;
    }
  }

  if ( 0 == iFound )
  {
    /* use default rules */
    iRet = g_pstIPAcl->iDefault ;
  }

  tlog_debug(g_IpLog,0,0, "CALL: tsec_dstip_verify_i(%s, %s)=%i", a_pszSrcIP, a_pszDstIP, iRet ) ;
  return iRet ;
}

