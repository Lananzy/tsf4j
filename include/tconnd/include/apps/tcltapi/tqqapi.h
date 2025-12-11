/*
**  @file $RCSfile: tqqapi.h,v $
**  general description of this module
**  $Id: tqqapi.h,v 1.27 2009/09/28 07:52:15 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009/09/28 07:52:15 $
**  @version $Revision: 1.27 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TQQAPI_H
#define TQQAPI_H

#ifdef __cplusplus
extern "C"
{
#endif


/** @defgroup tqqapi
* @{
*/

#include "tdr/tdr.h"
#include "apps/tcltapi/tqqdef.h"
#include "apps/tcltapi/tpdudef.h"


#ifndef IN
#define  IN
#endif

#ifndef OUT
#define  OUT
#endif

#ifndef INOUT
#define INOUT
#endif


/** 错误码 */
typedef enum
{
         TQQAPI_ERR_NETWORK_FAILURE          =1,    	/**< 网络连接关闭*/
         TQQAPI_ERR_TDR_NTOH_HEAD             =2,    	/**< 包头网络转本地序失败*/
         TQQAPI_ERR_NOT_ENOUGHBUF             =3,  	 /**< 接收缓冲区不够*/
         TQQAPI_ERR_DECRYPTION_HEAD          =4,   	/**< 包头解密失败*/
         TQQAPI_ERR_TDR_NTOH_BODY              =5, 	 /**< 包体网络转本地序失败*/
         TQQAPI_ERR_TDR_HTON_HEAD             =6,  	/**< 包头本地转网络序失败*/
         TQQAPI_ERR_TDR_HTON_BODY             =7,  	/**< 包体本地转网络序失败*/
         TQQAPI_ERR_RECV_TIMEOUT                =8,  	/**< 下行包收包超时*/
         TQQAPI_ERR_DECRYPTION_BODY          =9,  	/**< 解密包体失败*/
         TQQAPI_ERR_INVALID_CMD                  =10,  	/**< 非法包头类型*/
         TQQAPI_ERR_TDR_HTON_USERIDENT   =11,	/**< 内部定义*/
         TQQAPI_ERR_BUILD_AUTH                    =12,	/**< 内部定义*/
         TQQAPI_ERR_BUILD_SYNACK                 =13,	/**< 内部定义*/
         TQQAPI_ERR_COUNT
}TQQAPI_ERR;






struct tagTQQApiHandle;

typedef struct tagTQQApiHandle		TQQAPIHANDLE;
typedef struct tagTQQApiHandle		*HTQQAPI;
extern LPTDRMETA g_pstqqapiPduHead;

#define TQQAPI_PDUHEAD_HTON(net, host, ver)		tdr_hton(g_pstqqapiPduHead, net,host, ver)
#define TQQAPI_PDUHEAD_NTOH(host,net, ver)		tdr_ntoh(g_pstqqapiPduHead, host, net, ver)


/**
*@brief: 使用tqqapi_decode/encode函数前必须先调用tqqapi_create初始化
*@param:
*@retval
*   <0:fail
*   =0:success
*/
int  tqqapi_create();


/**
*@brief:解包:
*@param pszBuff[IN]:		接收缓冲区起始地址
*@param iBuff[IN]:     		长度
*@param pstHead[OUT]:	包头信息
*@param piHeadLen[OUT] 	解包长度
*@retval
*         = 0 for success
*         <0 failed,返回tdr错误码
*@see tdr_error_string(iRet)
*/
int tqqapi_decode(IN const char* pszBuff,IN int iBuff,OUT TPDUHEAD* pstHead,OUT int* piHeadLen);


/**
*@brief:打包
*@param        pszBuff[IN,OUT]: 	打包缓冲区
*@param        iBuff[IN,OUT]:     	缓冲区长度
*@param        pstHead[IN]:         包头
*@retval
*           =0 for success
*           <0 failed,返回tdr错误码
*@see tdr_error_string(iRet)
*/
int tqqapi_encode(INOUT char* pszBuff,INOUT int* piBuff, IN TPDUHEAD* pstHead);





/**
*@brief  连接服务器若成功分配连接句柄
*@param a_pszUri[IN]:URL 地址eg:172.25.40.97:9088
*@param a_iTimeout [IN]:连接阻塞超时时间
*@param a_phQQClt[OUT] 连接句柄
*@retval
*       >=0 success,返回socket
*       =-1  connect 失败,检查链路是否正常
*       =-2  calloc 句柄失败,检查内存是否不够
*/
int tqqapi_open(IN const char* a_pszUri,IN int a_iTimeout,OUT HTQQAPI* a_phQQClt);


/**
*@brief 分配句柄
*@param a_phQQClt[OUT] 连接句柄
*@retval
*       >=0 success,返回socket
*       =-1  calloc 句柄失败,检查内存是否不够
*/
int tqqapi_new(OUT HTQQAPI* a_phQQClt);


/**
*@brief  设置连接句柄socket
*@param a_phQQClt[IN] 连接句柄
*@param a_s[IN] socket
*@retval
*/
void tqqapi_attach(IN HTQQAPI a_hQQClt,IN int a_s);


/**
*@brief 根据包体meta分配发送/接收缓冲区大小
*@param a_hQQClt[IN] 	连接句柄
*@param a_pstSendMeta[IN] 上行tdr 结构meta
*@param a_pstSendMeta[IN] 下行行tdr 结构meta,若一致同上行meta
*@retval
*		=0 for success
*             <0 calloc failed,检查内存是否不够
*/
int tqqapi_set_pdu_meta(IN HTQQAPI a_hQQClt,IN LPTDRMETA a_pstSendMeta,IN  LPTDRMETA a_pstRecvMeta);


/**
*@brief   连接tconnd,测试号码方式
*@param   a_phQQClt[IN] 	  连接句柄
*@param   uin[IN]			  QQ号
*@param   pszSvrkey[IN]	  16字节签名密钥
*@param   iAuthType[IN]        QQ签名类型,一般设为3
 *              				  1:0x82
 *              				  2:0x82 
 *                         			  3:0xde               
 *						  其它:不支持
*@param   a_iTimeOut[IN]      收包阻塞超时时间
*@param   iEncMethod[IN] 
						0 :不加密
						2: tea 加解密 算法
						3: taes加解密算法
*@retval
*             0:for success
*            <0:for fail
*             use tqqapi_get_err/tqqapi_get_errorstring获得错误码或者提示信息
*@see
*     		  tqqapi_get_err
*		  tqqapi_get_errorstring
*/
int tqqapi_create_initial_connection(IN HTQQAPI a_phQQClt,IN long uin, IN const char *pszSvrkey,IN int iAuthType,IN int a_iTimeOut,IN int iEncMethod);



/**
*@brief 重新连接tconnd
*@param  	   a_phQQClt[IN] 		连接句柄
*@param          uin[IN]				QQ号
*@param          nPos[IN]			连接占位索引
*@param          szConnectKey[IN]	16 字节 密钥
*@param           szIdentity[IN]		16 字节验证码
*@param          iRelayType[IN]           重连类型
*								1:跨服跳转
*								2:断线重连
*@param          a_iTimeOut:收包超时时间
*@param          iEncMethod:0 :不加密2: tea 加解密  3: taes加解密
*@retval
*             0:for success
*            <0:for fail
*             use tqqapi_get_err/tqqapi_get_errorstring获得错误码或者提示信息
*@see
*     		  tqqapi_get_err
*		  tqqapi_get_errorstring
*/
int tqqapi_create_relay_connection(IN HTQQAPI a_phQQClt,IN long uin,IN int nPos,IN const char * szConnectKey,IN const char *szIdentity,IN int a_iTimeOut,IN int iRelayType,IN int iEncMethod);



/**
*@brief 发送消息
*@param    a_hQQClt[IN] 		连接句柄
*@param    a_pstHead[IN] 		PDU 包头, 对于通信消息,包头可以设为NULL
*@param    a_pszMsg[IN]		应用消息发送缓冲区
*@param    a_iMsg[IN]               消息大小 
*@param    a_iTimeout		      阻塞发送超时时间: 设为-1等待发送成功,零为不等待单位/ms
*@retval
*		 =0:for success
*             <0:for fail
*             use tqqapi_get_err/tqqapi_get_errorstring获得错误码或者提示信息
*@see
*     		  tqqapi_get_err
*		  tqqapi_get_errorstrin
*		  tqqapi_send_buffer
*		  tqqapi_write_msg
*	         tqqapi_write_buff
*/
int tqqapi_send_msg(IN HTQQAPI a_hQQClt, IN TPDUHEAD* a_pstHead,IN const char* a_pszMsg,IN int a_iMsg,IN int a_iTimeout);



/**
*@brief 接收消息  : 
*@param      a_hQQClt[IN] 		 	连接句柄
*@param      a_pstHead[IN]			接收消息的PDU包头,若不关心包头可以设为NULL
*@param      a_pszMsg[IN,OUT]		应用消息体接收缓冲区
*@param      a_iMsg[IN,OUT]		接收缓冲区大小(IN)  消息体大小(OUT)
*@param      a_iTimeout[IN]			接收超时时间: 若阻塞设为-1,零为不等待单位/ms
*@retval
*             =1:receive one package
*             =0:receive NULL
*             <0:for fail
*             use tqqapi_get_err/tqqapi_get_errorstring获得错误码或者提示信息
*@see
*     		  tqqapi_get_err
*		  tqqapi_get_errorstrin
*		  tqqapi_send_buffer
*		  tqqapi_write_msg
*	         tqqapi_write_buff
*/
int tqqapi_recv_msg(IN HTQQAPI a_hQQClt,OUT TPDUHEAD* a_pstHead,INOUT char* a_pszMsg,INOUT int* a_piMsg,IN int a_iTimeout);






/**
*@brief   获得句柄错误码
*@param  a_hQQClt[IN]  连接句柄
*@retval  错误码
*          
*/
int tqqapi_get_err(IN HTQQAPI a_hQQClt);



/**
*@brief   获得句柄错误信息
*@param  a_hQQClt[IN]  连接句柄
*@retval  错误字符串
*          
*/
const char * tqqapi_get_errstring(IN HTQQAPI a_hQQClt);


/**
*@brief  释放句柄
*@param  a_phQQClt[IN]  连接句柄指针
*@retval  错误字符串
*          
*/
int tqqapi_free(IN HTQQAPI* a_phQQClt);



/**
*@brief 发送缓存,不打包消息体
*@see
* 		  tqqapi_send_msg
*/
int tqqapi_send_buffer(IN HTQQAPI a_hQQClt, IN TPDUHEAD* a_pstHead,IN const char* a_pszBuffer,IN int iBufferLen,IN int a_iTimeout);


/**
*@brief 发送缓存,不解包消息体
*@see
* 		  tqqapi_recv_msg
*/
int tqqapi_recv_buffer(IN HTQQAPI a_hQQClt, IN TPDUHEAD* a_pstHead,IN const char* a_pszBuffer,INOUT int * piBufferLen,IN int a_iTimeout);

/**
*@brief 发送消息,不支持超时设定
*@see
* 		  tqqapi_send_msg
*/
int  tqqapi_write_msg(IN HTQQAPI a_hQQClt, IN TPDUHEAD* a_pstHead,IN const char* a_pszMsg,IN int a_iMsg);

/**
*@brief 接收消息,不支持超时设定
*@see
* 		  tqqapi_recv_msg
*/
int  tqqapi_read_msg(IN HTQQAPI a_hQQClt,OUT TPDUHEAD* a_pstHead,INOUT char* a_pszMsg,INOUT int* a_piMsg);

/**
*@brief 发送缓存,不支持超时设定
*@see
* 		  tqqapi_send_buffer
*/
int  tqqapi_write_buff(IN HTQQAPI a_hQQClt, IN TPDUHEAD* a_pstHead,IN const char* a_pszBuffer,IN int iBufferLen);


/**
*@brief 接收缓存,不支持超时设定
*@see
* 		  tqqapi_send_buffer
*/
int  tqqapi_read_buff(IN HTQQAPI a_hQQClt, IN TPDUHEAD* a_pstHead,IN const char* a_pszBuffer,INOUT int * piBufferLen);


/**
*@brief  设置通信加解密方法及密钥
*@param     a_hQQClt[IN]     连接句柄
*@param     pszGameKey:	16字节通信密钥
*@param     iEncMethod:
*		    0:不加密  
*  		    2:使用tea算法	
*		    3:使用taes算法	
*@retval
*			=0 for success
*                    <0 failed
*/
int  tqqapi_set_gamekey(IN HTQQAPI a_hQQClt,IN  const char* pszGameKey,IN int iEncMethod);


/**
*@brief  获取密钥r: 
*@param      a_hQQClti[IN] 			连接句柄
*@param      pszGameKey[OUT]		16字节密钥
*@retval
*			 =0 for success
*                    <0 failed,未设置通信密钥或不加密

*/
int   tqqapi_get_gamekey(IN  HTQQAPI a_hQQClt, OUT char* pszGameKey);


/**
*@brief	 设置应用消息体协议版本,一般不要设,兼容服务器协议
*@param   a_hQQClt[IN]		句柄
*@param   a_iSelfVersion[IN]	客户端协议版本
*@param   a_iPeerVersion[IN]	服务器协议版本
*@retval
*/
void tqqapi_set_version(IN HTQQAPI a_hQQClt,IN  int a_iSelfVersion,IN int a_iPeerVersion);



/**
*@brief 设置客户端连接信息
*@param    a_hQQClt[IN]    		句柄
*@param   a_iPos[IN] 			连接索引
*@param   a_pszIdentity[IN] 	连接验证16 字节字符串
*/
void tqqapi_set_identity(IN HTQQAPI a_hQQClt,IN  int a_iPos,IN  const char * a_pszIdentity);

/**
*@brief 获取客户端连接信息
*@param    a_hQQClt[IN]    		句柄
*@param   a_iPos[OUT] 			连接索引
*@param   a_pszIdentity[OUT] 	连接验证16 字节字符串
*/
void tqqapi_get_identity(IN HTQQAPI a_hQQClt,OUT int *a_iPos,OUT char * a_pszIdentity);


/**
*@brief 获取客户端连接socket
*@param    a_hQQClt[IN]    		句柄
*@retval  返回socket
*
*/
int	tqqapi_get_sock(IN HTQQAPI a_phQQClt);




/**
*@brief 设置socket 发送缓冲区
*@param    a_hQQClt[IN]    		句柄
*@param    a_hQQClt[IN]    		缓冲区大小
*@retval  
* 		0 = success
*            <0 fail
*/
int 	tqqapi_set_sendbuff(IN HTQQAPI a_phQQClt,IN  int a_iSize);


/**
*@brief 设置socket 接收缓冲区
*@param    a_hQQClt[IN]    		句柄
*@param    a_hQQClt[IN]    		缓冲区大小
*@retval  
* 		0 = success
*            <0 fail
*/
int 	tqqapi_set_recvbuff(IN HTQQAPI a_phQQClt, IN int a_iSize);











void              		tqqapi_init_base(TPDUBASE* a_pstBase);
const char*   		tqqapi_get_meta_data(void);
char *           		tqqapi_randstr(char* pszBuff, int iLen);
int                		tqqapi_detach(HTQQAPI a_hQQClt);
int                		tqqapi_get_sequence(HTQQAPI a_hQQClt);
void                      tqqapi_set_sequence(HTQQAPI a_hQQClt,int iSequence);
unsigned short       tqqapi_check_headsum(unsigned short *szbuf,int bufflen);
int             		tqqapi_extract_identinfo(HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead, TPDUIDENTINFO* pstIdentInfo);
int                		tqqapi_extract_Sessionkey(HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead,char *pszSessionkey);
int                		tqqapi_extract_syninfo(HTQQAPI a_hQQClt,TPDUHEAD* a_pstHead);
int                		tqqapi_make_authinfo(TQQAUTHINFO* pstAuthInfo, TQQGAMESIG* pstGameSig, TQQSIGFORS2* pstSigForS2, long uin, char* pszSvrSKey);
int                		tqqapi_extract_authinfo(TQQGAMESIG* pstGameSig, TQQSIGFORS2* pstSigForS2, long uin, TQQAUTHINFO* pstAuthInfo,char* pszSvrSKey);
int                		tqqapi_make_QQUnify_Authinfo(TQQUNIFIEDAUTHINFO *pstUnifyAuthInfo,long uin ,char* pszSvrSKey,char *pszSessionkey,unsigned int uCltIP);
int                		tqqapi_extract_QQUnify_Authinfo(TQQUNIFIEDAUTHINFO *pstUnifyAuthInfo,char* pszSvrSKey);
int		     		tqqapi_build_auth_msg( HTQQAPI a_hQQClt, TPDUHEAD * a_pstHead, long uin, const char *pszSvrkey,int iAuthType,unsigned int uCltIP,int iEncMethod);
int 				tqqapi_build_synack_msg(HTQQAPI a_hQQClt,TPDUHEAD* a_pstHead);
int                		tqqapi_build_relay_msg( HTQQAPI a_hQQClt, TPDUHEAD* a_pstHead,long uin,int nPos,const char * szConnectKey,const char *szIdentity,int iRelayType,int iEncMethod); 
int 				tqqapi_open2(const char* a_pszUri);



/** @} */ 







#ifdef __cplusplus
}
#endif

#endif 


