/*
**  @file $RCSfile: tconnapi.h,v $
**  general description of this module
**  $Id: tconnapi.h,v 1.7 2009/09/23 07:41:26 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009/09/23 07:41:26 $
**  @version $Revision: 1.7 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TCONNAPI_H
#define TCONNAPI_H

#include "tdr/tdr_types.h"
#include "apps/tconnapi/tframehead.h"

#ifndef IN
#define  IN
#endif

#ifndef OUT
#define  OUT
#endif

#ifndef INOUT
#define INOUT
#endif


extern LPTDRMETA g_pstConnapiFrameHead;

#define TCONNAPI_FRAMEHEAD_HTON(net, host, ver)		tdr_hton(g_pstConnapiFrameHead, net,host, ver)
#define TCONNAPI_FRAMEHEAD_NTOH(host,net, ver)		tdr_ntoh(g_pstConnapiFrameHead, host, net, ver)


/** @defgroup tconnapi
* @{
*/

 
/** tconnd_recv 最大包不超过65536 */
#define TCONNAPI_MAX_BUFF		0x10000

#ifdef __cplusplus
extern "C"
{
#endif


/**
*@brief 初始化bus信息
*@param pszBuff[IN]  tbus share memory key,if set to 0,use 1688 by default
*@retval
*   <0:fail
*   =0:success
*/
int tconnapi_init(IN int iKey);


/**
*@brief FrameHead 解包
*@param pszBuff[IN]  		解包目的缓冲区
*@param iBuff[IN]  		目的缓冲区长度
*@param pstHead[IN]		FrameHead地址
*@param piHeadLen[OUT]	解包后网络长度
*@retval
*   <0:fail 检查error
*   =0:success
*@see
*/
int tconnapi_decode(IN const char* pszBuff,IN  int iBuff,OUT TFRAMEHEAD* pstHead,OUT int* piHeadLen);


/**
*@brief FrameHead 打包
*@param pszBuff[IN]  		解包目的缓冲区
*@param iBuff[IN]  		目的缓冲区长度
*@param pstHead[IN]		FrameHead地址
*@param piHeadLen[OUT]	解包后网络长度
*@retval
*   <0:fail 检查error
*   =0:success
*@see
*/
int tconnapi_encode(INOUT char* pszBuff,INOUT  int* piBuff, IN TFRAMEHEAD* pstHead);






/**
*@brief 创建句柄
*@param iProcID[IN]  bus ID
*@param piHandle[OUT]  句柄指针
*@retval
*     <0:fail
*     =0:success
*/
int tconnapi_create(IN int iProcID, OUT int* piHandle);

/**
*@brief 释放句柄
*@param piHandle[OUT]  句柄指针
*@retval
*/
void tconnapi_free(IN int* piHandle);

/**
*@brief 释放句柄
*@param piHandle[OUT]  句柄指针
*@retval
*    <0 fail
*    =0 success
*/
int tconnapi_connect(IN int iHandle,IN int iDst);

/**
*@brief  从bus中收包
*@param iHandle[OUT]  	句柄指针
*@param piSrc[INOUT]  	bus源地址
*@param pszBuff[INOUT]  	应用消息体缓冲区起始地址
*@param piBuff[INOUT]       缓冲区长度IN  消息体长度OUT
*@param piBuff[OUT]  		解包的FrameHead
*@retval
*    <0 没有收到包,See errno
*    =0 recv package success
*/
int tconnapi_recv(IN int iHandle,INOUT int *piSrc,INOUT char* pszBuff,INOUT int* piBuff,OUT TFRAMEHEAD* pstHead);


/**
*@brief  往bus中发包
*@param iHandle[IN]  	句柄指针
*@param iDst[IN]  		bus目的地址
*@param pszBuff[IN]  	应用消息体缓冲区起始地址
*@param iBuff[IN  ]      缓冲区长度
*@param pstHead[IN]  		打包的FrameHead
*@retval
*    <0 没有收到包,See errno
*    =0 recv package success
*/
int tconnapi_send(IN int iHandle,IN  int iDst, IN char* pszBuff,IN  int iBuff,IN TFRAMEHEAD* pstHead);

/**
*@brief  释放bus信息
*/
void tconnapi_fini(void);




/**
*@brief  内部接口
*/
int tconnapi_initialize(const char *a_pszGCIMKey, int a_iBusinessid);


/** @} */ 

#ifdef __cplusplus
}
#endif


#endif /* TCONNAPI_H */
