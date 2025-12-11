/** @file $RCSfile: bus_misc.h,v $
  Invoked function header file for tsf4g-bus module
  $Id: bus_misc.h,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _BUS_MISC_H
#define _BUS_MISC_H

#include "bus/bus_platform.h"
#include "bus_macros.h"
#include "bus_route.h"


#define GET_ADDR(a_iAddr) (inet_ntoa(*(struct in_addr *)&(a_iAddr)))


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int enableRoute ( IN const unsigned int a_iAddr, IN const int a_iAddrType, IN const SHMROUTE *a_ptRoute, INOUT ADDRINFO **a_pptAddrs ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int attachChl ( IN const unsigned int a_iAddr, IN const int a_iAddrType, IN const SHMROUTE *a_ptRoute, INOUT CHANNEL *a_ptChannel ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int getShmAny(void **ppvShm, int iShmKey, int iSize, int iFlag) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
char *hexString(unsigned char* sStr, int iStrLen) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
unsigned int getBackDst(BUSINTERFACE *pstBus) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
unsigned short busHeadLen(BUSPKGHEAD* pstHead) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
unsigned int busBodyLen(BUSPKGHEAD* pstHead) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
unsigned int busPkgLen(BUSPKGHEAD* pstHead) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
unsigned short busHeadCheckSum(BUSPKGHEAD* pstHead) ;


/**
  @brief Show Hex val of a string, if iFlag = 1, show printable character as char
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
void hexShow(unsigned char* sStr, int iLen, int iFlag) ;

#endif /**< _BUS_MISC_H */

