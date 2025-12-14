#pragma once

#include <stdio.h>

#include "../comm/ResConv.h"

#define RES_HEAD_META_NAME "ResHead"

class CResBinFile : public CFile
{
public:
	CResBinFile(void);
	virtual ~CResBinFile(void);

	virtual BOOL Open(
		LPCTSTR lpszFileName,
		UINT nOpenFlags,
		CFileException* pError = NULL 
		);

	/*将二进制资源数据写道文件末尾
	*@param[in] pszBuff 保存数据的缓冲区首地址
	*@param[in] nCount	资源数据的字节数
	*@return 成功返回0，失败返回非零值
	*/
	int AppendWriteBinData(const unsigned char *pszBuff, UINT nCount);

	/*写资源文件头部
	*@param[in] iCount 资源数据的个数
	*@param[in] iUnit 单个资源数据的存储空间
	*@return 成功返回0，失败返回非零值
	*/
	int WriteResHead(int iCount, int iUnit);

	/*写资源文件头部
	*@param[in] iCount 资源数据的个数
	*@param[in] stResHead 资源数据头部结构
	*@return 成功返回0，失败返回非零值
	*/
	int WriteResHead(RESHEAD &stResHead);

	/*写资源文件头部
	*@param[out] stResHead 资源数据头部结构
	*@return 成功返回0，失败返回非零值
	*/
	int ReadResHead(RESHEAD &stResHead);

	

	/*从文件当前位置读取资源信息
	*@param[in] pszBuff 保存数据的缓冲区首地址,其空间由调用者提供
	*@param[in] nCount	指定读取字节数,缓冲区的大小必须不小于此数
	*@return 成功返回0，失败返回非零值
	*/
	int ReadBinData(unsigned char *pszBuff, UINT nCount);

private:
	FILE *m_fpBinFile;
};
