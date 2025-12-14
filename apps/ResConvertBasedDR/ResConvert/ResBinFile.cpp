#include "stdafx.h"
#include <assert.h>
#include "ResBinFile.h"

CResBinFile::CResBinFile(void)
{
}

CResBinFile::~CResBinFile(void)
{
}



BOOL CResBinFile::Open(LPCTSTR lpszFileName,  UINT nOpenFlags,   CFileException* pError/* = NULL*/)
{
	BOOL bSucc = true;

	bSucc = CFile::Open(lpszFileName, nOpenFlags, pError);
	if (!bSucc)
	{
		return bSucc;
	}

	/*如果是写资源文件则，将头部空间预留出来*/
	if (nOpenFlags & CFile::modeWrite)
	{
		CFile::Seek(sizeof(RESHEAD), CFile::begin);


		TRACE("Pos: %ulld\n", CFile::GetPosition());
	}

	return true;
}

/*将二进制资源数据写道文件末尾
*@param[in] pszBuff 保存数据的缓冲区首地址
*@param[in] nCount	资源数据的字节数
*@return 成功返回0，失败返回非零值
*/
int CResBinFile::AppendWriteBinData(const unsigned char *pszBuff, UINT nCount)
{
	int iRet = 0;

	//ULONGLONG dwActual = CFile::SeekToEnd();
	//TRACE("Pos: %ulld\n", CFile::GetPosition());

	try
	{
		CFile::Write(pszBuff, nCount);
	}
	catch (CException* e)
	{
		iRet = -1;
	}

	return iRet;
}


int CResBinFile::WriteResHead(int iCount, int iUnit)
{
	RESHEAD stHead;
	int iRet = 0;

	stHead.iMagic = RES_FILE_MAGIC;
	stHead.iVersion = RES_TRANSLATE_VERSION;
	stHead.iCount = iCount;
	stHead.iUnit = iUnit;

	CFile::SeekToBegin();
	try
	{
		CFile::Write(&stHead, sizeof(stHead));
	}
	catch (CException* e)
	{
		iRet = -1;
	}

	return iRet;
}

int CResBinFile::WriteResHead(RESHEAD &stResHead)
{
	int iRet = 0;
	
	CFile::SeekToBegin();
	try
	{
		CFile::Write(&stResHead, sizeof(stResHead));
	}
	catch (CException* e)
	{
		iRet = -1;
	}

	return iRet;
}


int CResBinFile::ReadResHead(RESHEAD &stResHead)
{
	UINT nLen;

	CFile::SeekToBegin();
	nLen = CFile::Read(&stResHead, sizeof(stResHead));
	if (nLen != sizeof(stResHead))
	{
		return -1;
	}

	if (stResHead.iMagic != RES_FILE_MAGIC)
	{
		return -2;
	}

	if (stResHead.iVersion != RES_TRANSLATE_VERSION)
	{
		return -3;
	}

	return 0;
}

/*从文件当前位置读取资源信息
*@param[in] pszBuff 保存数据的缓冲区首地址,其空间由调用者提供
*@param[in] nCount	指定读取字节数,缓冲区的大小必须不小于此数
*@return 成功返回0，失败返回非零值
*/
int CResBinFile::ReadBinData(unsigned char *pszBuff, UINT nCount)
{
	assert(NULL != pszBuff);
	assert(0 < nCount);

	UINT nBytesRead = CFile::Read( pszBuff, nCount );
	if (nBytesRead != nCount)
	{
		return -1;
	}

	return 0;
}


