#ifndef TRESLOADER_H
#define TRESLOADER_H

#include "tdr/tdr.h"


#define RL_MAX_PATH_LEN		256

#define RL_LOADMODE_XMLV1	1		/*读取嵌套结构体以类型名标识的xml资源文件*/
#define RL_LOADMODE_XMLV2	2		/*读取嵌套结构体以成员名标识的xml资源文件*/
#define RL_LOADMODE_BIN		3		/*读取二进制数据格式的资源文件*/

#ifndef RES_ID_ARRAY
#define RES_ID_ARRAY	1
#endif
/*表示该参数只是输入参数*/
#ifndef IN
#define IN  
#endif


/*表示该参数只是输出参数*/
#ifndef OUT
#define OUT
#endif


/*表示该参数既是输入参数，又是输出参数*/
#ifndef INOUT
#define INOUT
#endif

/* automatically include the correct library on windows */
#ifdef WIN32

# ifdef _DEBUG
#  define LIB_RESLOADER_D    "_d"
# else
#  define LIB_RESLOADER_D
# endif /* _DEBUG */



# if defined(LIB_RESLOADER_D)
# pragma comment( lib, "libresloader"  LIB_RESLOADER_D ".lib" )
# else
# pragma comment( lib, "libresloader.lib" )
# endif


#endif


#ifdef __cplusplus
extern "C"
{
#endif
	

	/* 初始化资源读取接口
	*@param[in] pszResDir	资源文件所在目录
	*@param[in] pszTDRFile 描述资源结构体的元数据库文件文件名
	*@param[in] cLoadMode	资源文件的格式
	*@return 成功返回0，失败返回负数
	*@note 本接口非线程安全,在多线程环境,应用需自行互斥调用
	*/
int rl_init(IN const char *pszResDir, IN const char *pszTDRFile, IN char cLoadMode);


/* 从资源文件中加载资源到缓冲区中，缓冲区的空间在函数外分配
*@param[in] pszFilePath	资源文件名
*@param[in] pszBuff 保存数据的缓冲区首地址
*@param[in] iBuff	缓冲区的可用字节数
*@param[in] iUnit	单个资源信息结构体的在缓冲区中的存储空间，通过这个参数调用者可以为每个资源分配比实际存储空间更大的空间。
	如果此参数的值为0，则单个资源信息结构体的存储空间为实际所需空间
*@return 成功返回获取的资源个数，失败返回0或负数
*/
int rl_sload(IN const char* pszFilePath, IN char* pszBuff, IN int iBuff, IN int iUnit);

/* 从资源文件中加载资源到缓冲区中，缓冲区的空间在函数内分配
*@param[out] pszBuff 保存数据的缓冲区首地址
*@param[out] iBuff	获取缓冲区的字节数
*@param[out] iUnit	获取单个资源信息结构体的在缓冲区中的存储空间
*@param[in] pszFilePath	资源文件名
*
*note	保存资源信息的缓冲区必须调用free进行释放
*@return 成功返回0，失败返回非零值
*/
int rl_cload(OUT void** ppvBuff, OUT int *piBuff, OUT int *piUnit, IN const char* pszFilePath );

/* 从XML格式的资源文件中加载资源到缓冲区中，缓冲区的空间在函数内分配
*@param[in] pszBuff 保存数据的缓冲区首地址
*@param[in] iBuff	缓冲区的可用字节数
*@param[in] iUnit	单个资源信息结构体的在缓冲区中的存储空间，通过这个参数调用者可以为每个资源分配比实际存储空间更大的空间。
*@param[in] pszFilePath	资源文件名
*@param[in] iIOVersion XML格式输入/输出的版本,目前支持两个版本1,2  
*@param[in] pstMetalib 元数据描述库句柄
*@param[in] pszMeta 资源结构体元数据描述名
*
*@note 如果a_iIOVersion指定除1，2以外的值，则强制按照版本2进行处理
*@return 成功返回获取的资源个数，失败返回0或负数
*/
int rl_xload_with_head(IN char* pszBuff, IN int iBuff, IN int iUnit, IN const char* pszXMLFilePath, IN int iIOVersion, IN LPTDRMETALIB pstMetalib, IN const char *pszMeta);


/* 根据资源信息的关键字信息，在资源文件查找特定资源信息
*@param[in] pszBuff 保存资源数据的缓冲区首地址
*@param[in] iCount	此缓冲区中保存的资源信息结构体的个数
*@param[in] iUnit	单个资源信息结构体的在缓冲区中的存储空间
*@param[in] iKey	查找关键字
*@return 如果成功找到返回此资源信息的首地址，否则返回NULL
*
*@pre \e 此资源信息结构的第一个成员作为查找关键字，其类型必须是整数类型。
*/
char* rl_find(char* pszBuff, int iCount, int iUnit, int iKey);

/* 根据资源信息的关键字信息，在资源文件查找特定资源信息
*@param[in] pszBuff 保存资源数据的缓冲区首地址
*@param[in] iCount	此缓冲区中保存的资源信息结构体的个数
*@param[in] iUnit	单个资源信息结构体的在缓冲区中的存储空间
*@param[in] iKey	查找关键字
*@return 如果成功找到返回此资源信息的首地址，否则返回NULL
*
*@pre \e 此资源信息结构的第一个成员作为查找关键字，其类型必须是长整数类型。
*/
char* rl_find_ll(char* pszBuff, int iCount, int iUnit, tdr_longlong llKey);


/* 根据资源信息的关键字信息，在资源文件查找特定资源信息
*@param[in] pszBuff 保存资源数据的缓冲区首地址
*@param[in] iCount	此缓冲区中保存的资源信息结构体的个数
*@param[in] iUnit	单个资源信息结构体的在缓冲区中的存储空间
*@param[in] iKey	查找关键字
*@return 如果成功找到返回此资源信息的首地址，否则返回NULL
*
*@pre \e 此资源信息结构的第一个成员作为查找关键字，其类型必须是短整数类型。
*/
char* rl_find_n(char* pszBuff, int iCount, int iUnit, short nKey);

#ifdef __cplusplus
}
#endif

#endif /* TRESLOADER_H */
