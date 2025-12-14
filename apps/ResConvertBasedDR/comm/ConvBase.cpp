#include "stdafx.h"
#include <shlwapi.h>
#include "ConvDefine.h"
#include "ConvBase.h"





CString GetRootPath()
{
	char szPath[MAX_LENGTH_LINE];
	GetModuleFileName(NULL, szPath, MAX_LENGTH_LINE);
	CString path = szPath;
	int pos = path.ReverseFind('\\');
	return path.Left(pos + 1);
}

CString MakePath(CString &szBasePath, CString &szPath)
{
	CString temp(szPath);

	temp.Replace('/', '\\');

	temp.TrimLeft(" \\/");
	temp.TrimRight(" \\/");

	if (PathIsRelativeA(szPath))
	{
		temp = szBasePath + temp;
	}
	
	return temp;
}

bool IsDigitStr(CString &szTmp)
{
	int iLen = szTmp.GetLength();
	char *pszChar = szTmp.GetBuffer(0);
	int i;

	for (i= 0; i < iLen; i++)
	{
		if (((pszChar[i] < '0') || (pszChar[i] > '9')) && ('+' != pszChar[i]) && ('-' != pszChar[i]))
		{
			break;
		}
	}

	if (i < iLen)
	{
		return false;
	}

	return true;
}

void ConvertChineseNumber(CString& cNumber)
{
	cNumber.Replace("１", "1");
	cNumber.Replace("２", "2");
	cNumber.Replace("３", "3");
	cNumber.Replace("４", "4");
	cNumber.Replace("５", "5");
	cNumber.Replace("６", "6");
	cNumber.Replace("７", "7");
	cNumber.Replace("８", "8");
	cNumber.Replace("９", "9");
	cNumber.Replace("０", "0");
	cNumber.Replace("．", ".");
	cNumber.Replace("－", "-");
}

/*取出cChar中的整数, cNumber保存整数，cChar保存小数部分*/
void GetNumber(IN CString& szCell, OUT CString& szNumber)
{
	CString cChar = szCell;
	ConvertChineseNumber(cChar);
	const char* s = (LPCTSTR)cChar;
	int i;
	CString st;

	/*找出第一个0－9或. - 号*/
	for( i=0; i<cChar.GetLength(); i++ )
	{
		if( (*s>='0' && *s<='9') || *s=='.' || *s=='-' )
			break;
		s++;
	}
	if(i >= cChar.GetLength())
	{
		szNumber = "";
		return;
	}
	cChar = cChar.Mid(i);



	/*查找数字串末尾：第一个非0－9 . - 字符*/
	s = (LPCTSTR)cChar;
	for( i=0; i<cChar.GetLength(); i++ )
	{
		if( (*s<'0' || *s>'9') && *s!='.' && *s!='-' )
			break;
		s++;
	}

	szNumber = cChar.Left(i);
	while( szNumber.Left(1)=="0" )
	{
		if ((1 >= szNumber.GetLength()) || (szNumber.Left(2) == "0."))
		{
			break;
		}else
		{
			szNumber = szNumber.Mid(1); /*去掉最前面的0*/
		}
	}

	/*XXXX.YYYY 小数处理，四舍五入*/
	if( (i=szNumber.Find("."))>0 )
	{
		st = szNumber.Mid(i+1);
		if( atoi(st.Left(1))>=5 )
		{
			szNumber = szNumber.Left(i);
			i = atoi(szNumber);
			i++;
			szNumber.Format("%d", i);
		}
		else
		{
			szNumber = szNumber.Left(i);
		}
	}
}


/*取出百分数*/
void GetPercent(IN CString& cNumber, OUT CString &szPresent)
{
	szPresent = cNumber;
	ConvertChineseNumber(szPresent);
	if( szPresent.Left(1)=="0" )
		szPresent = szPresent.Mid(1);

	if( szPresent.Left(1) == "." )
	{
		szPresent = szPresent.Mid(1);
		if( szPresent.GetLength()<2 )
			szPresent+="0";
		else
			szPresent = szPresent.Left(2);

		if( szPresent.Left(1)=="0" )
			szPresent = szPresent.Mid(1);

	}else
	{
		szPresent = "100";
	}	
}


