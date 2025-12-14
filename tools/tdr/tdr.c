/**
*
* @file     tdr.c  
* @brief    TDR工具将元数据描述库转换成二进制文件，C定义文件，h定义文件
* 
* @author jackyai  
* @version 1.0
* @date 2007-12-27 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "tgetopt.h"
#include "tdr/tdr.h"
#include "tdr_tools.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

#ifdef WIN32
	#ifndef snprintf
		#define snprintf _snprintf
	#endif
#endif

/*
版本:00101010
分解:00        1        01              010
变换:0.1       beta    01   build    010

*/
#define	TDR_BUILD_VERSION		1101010		//0.1 Beta01 build 010
#define TDR_MAX_FILE_LEN			1024 /**<输出文件的名字最大长度*/


#define  TDR_DEFAULT_DR_OUTPUT_FILE		"a.tdr"
#define TDR_DR_FILE_SUFFIX			".tdr"
#define  TDR_DEFAULT_DR_COUTPUT_FILE		"a.c"

enum tagTDROperID
{
	TDR_OPER_UNDEFINE = 0,	/**<未定义*/
	TDR_OPER_XML2DR,		/**<转化成二进制库*/
	TDR_OPER_XML2C,			/**<xml2c*/
	TDR_OPER_XML2H,			/**<xml2h*/
};
typedef enum tagTDROperID TDROPERID;

/*TDR需要的选项*/
struct tagTDROption
{
	TDRBOOLEAN bDoDumpLib;	/**<值为true则导出元数据描述库*/
	TDRBOOLEAN bIsOldTag;	/**<指明xml描述库使用的标签集是否是老的格式，其值为true表示是*/
	TDROPERID enID;
	char szOutFile[TDR_MAX_FILE_LEN];
	char szOutPath[TDR_MAX_FILE_LEN];
	TDRHPPRULE stRule;
	int iXMLFileNum;		/**< 需转换的XML文件的数目*/
	char *paszXMLFile[TDR_MAX_FILES_IN_ONE_PARSE]; /**<各XML文件的名字*/
};
typedef struct tagTDROption TDROPTION;
typedef struct tagTDROption *LPTDROPTION;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void tdr_show_help(const char *a_pszObj);
static int tdr_def_opt(LPTDROPTION pstOption, int argc, char *argv[]);
static int tdr_xml2dr(LPTDROPTION pstOption);
static int tdr_xml2c(LPTDROPTION pstOption);
static int tdr_xml2h(LPTDROPTION pstOption);
static int tdr_make_path_file(char *pszFileBuff, int iBuff, const char *pszPath, char *pszFile, char *pszSuffix);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void tdr_show_help(const char *a_pszObj)
{
	char *pszApp;

	assert(NULL != a_pszObj);
	pszApp = tdr_basename(a_pszObj);
	printf("\n%s - 将多个XML格式的元数据描述库转换成二进制格式、C语言定义格式描述及生成数据定义的C语言头文件。如果多个xml文件之间存在依赖关系，则被依赖的xml文件必须放在参数表的前面\n", pszApp);
	printf("\n用法: \n");
	printf("%s ( (-B --xml2dr [-o --out_file=FILE] )  |  (-C --xml2c [-o --out_file=FILE]) |\n" 
		"(-H --xml2h [-p --no_type_prefix]  [-s --add_custom_prefix=CUSTOM_PREFIX]  [-l --no_lowercase_prefix] [-c --no_type_declare] ) )\n"
		"[-O --out_path=PATH] [-f --old_xml_tagset] [-d --dump_lib ] [-h -? --help] [-v -version] xmlfile... \n\n", pszApp);
	printf("主要转换操作选项：\n"
		"\t-B, --xml2dr	指定使用将多个xml格式的元数据描述库转换成二进制个数描述库的功能（xml2dr）\n"
		"\t-C, --xml2c	指定使用将多个xml格式的元数据描述库转换成C语言定义格式描述库的功能（xml2c）\n"
		"\t-H, --xml2h	指定使用利用多个xml格式的元数据描述生成数据的C语言头文件的功能（xml2h）。\n"
		"\t\t 生成的.h头文件与xml文件一一对应，即每个xml文件会转换为一个相应头文件；除文件后缀外，头文件与xml文件名相同\n"
		"\nxml2dr的可选项：\n"
		"\t-o, --out_file=FILE	指定转换出来的二进制描述库的文件名，如果没有指定此选择，则缺省以a.tdr为文件名\n"
		"\nxml2c的可选项：\n"
		"\t-o, --out_file=FILE	指定转换出来的C语言定义格式描述库的文件名，如果没有指定此选择，则缺省以a.c为文件名\n"
		"\nxml2h的可选项：\n"
		"\t-p, --no_type_prefix	生成的结构体（struct）/联合体（union）成员名不添加表示此成员类型的前缀。如果不指定此选择，缺省会添加表示成员类型的前缀\n"
		"\t-s, --add_custom_prefix=CUSTOM_PREFIX	生成的结构体（struct）/联合体（union）成员名添加自定义的前缀。"
			"\t\tCUSTOM_PREFIX为要添加的前缀串，目前其最多只能包含%d个字符\n"
		"-l, --no_lowercase_prefix	如果指定此选项，则生成的结构体（struct）/联合体（union）成员名不强制将首字母改成小写\n"
		"-c, --no_type_declare	如果指定了此选项则在元数据的C语句头文件中不生成结构体（struct）/联合体（union）的类型声明.\n"
		"\n转换公共选项：\n"
		"-O, --out_path=PATH	指定输出文件的保存路径。如果不指定此选项，对于xml2dr和xml2c其输出目录缺省为当前工作目录；对于xml2h其输出目录为对应xml所在目录.\n"
		"-f --old_xml_tagset	此选项指明xml元数据描述库是采用原来FO定义的老的XML标签进行描述的，转换工具必须按照老的格式去解析。\n"
		"\t\t如果没有指定此选项，则转换工具会按照TDR定义的xml标签去解析xml元数据描述库.\n"
		"\t-d, --dump_lib	以可视化的方式打印出元数据描述库,并输出到\"dump_lib.txt\"文件中\n"
		"\n其他选项：\n"
		"-?,-h, --help	输出此工具的帮助列表\n"
		"-v, --version	输出此工具的构建版本\n",
		TDR_MAX_CUSTOM_NAME_PREFIX_LEN-1);
		
	printf("\n使用示例:\n");
	printf("%s -B -o ov_res.tdr ov_res.xml   #xml格式元数据库生成.tdr二进制库\n"
		"%s -C -o ov_res.c --old_xml_tagset  ov_res.xml		#使用老标签集的xml格式元数据库生成.c文件\n"
		"%s -H -O \"include\" --add_custom_prefix=\"m_\" --no_type_prefix		#xml元数据库生成.h文件，生成的文件保存在include目录,\n"
		"\t结构体（struct）/联合体（union）成员名添加前缀\"m_\"，但不添加类型前缀\n",
		pszApp, pszApp, pszApp);	
}

int tdr_def_opt(LPTDROPTION pstOption, int argc, char *argv[])
{
	int iFile;
	int i;
	int opt;
	int iWriteLen;
	static struct option s_astLongOptions[] = {
		{"xml2dr", 0, NULL, 'B'},
		{"xml2c", 0, NULL, 'C'},
		{"xml2h", 0, NULL, 'H'},
		{"out_file", 1, NULL, 'o'},
		{"dump_lib", 0, NULL, 'd'},
		{"no_type_prefix", 0, NULL, 'p'},
		{"add_custom_prefix", 1, NULL, 's'},
		{"no_lowercase_prefix", 0, NULL, 'l'},
		{"no_type_declare", 0, NULL, 'c'},
		{"out_path", 1, NULL, 'O'},
		{"old_xml_tagset", 0, NULL, 'f'},
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},

		{0, 0, 0, 0}
	};

	assert(NULL != pstOption);
	assert(0 < argc);
	assert(NULL != argv);

	while (1)
	{
		int option_index = 0;
		
		opt = getopt_long (argc, argv, "BCHo:dps:lcO:fvh",
			s_astLongOptions, &option_index);

		if (opt == -1)
			break;
		
		switch(opt)
		{
		case 'B':
			pstOption->enID = TDR_OPER_XML2DR;
			break;
		case 'C':
			pstOption->enID = TDR_OPER_XML2C;
			break;
		case 'H':
			pstOption->enID = TDR_OPER_XML2H;
			break;
		case 'o':
			{
				iWriteLen = snprintf(pstOption->szOutFile, sizeof(pstOption->szOutFile), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(pstOption->szOutFile))))
				{
					printf("指定输出文件名太长，目前最多支持%u个字符的文件名\n", sizeof(pstOption->szOutFile) -1);
					exit(EINVAL);
				}
			}
			break;
		case 'd':
			pstOption->bDoDumpLib = TDR_TRUE;
			break;
		case 'p':
			pstOption->stRule.iRule |= TDR_HPPRULE_NO_TYPE_PREFIX;
			break;
		case 's':
			{
				pstOption->stRule.iRule |= TDR_HPPRULE_ADD_CUSTOM_PREFIX;
				iWriteLen = snprintf(pstOption->stRule.szCustomNamePrefix, sizeof(pstOption->stRule.szCustomNamePrefix), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(pstOption->stRule.szCustomNamePrefix))))
				{
					printf("指定自定义前缀太长，目前最多支持%u个字符的自定义前缀\n", sizeof(pstOption->stRule.szCustomNamePrefix) -1);
					exit(EINVAL);
				}
			}
			break;
		case 'l':
			pstOption->stRule.iRule |= TDR_HPPRULE_NO_LOWERCASE_PREFIX;
			break;
		case 'c':
			pstOption->stRule.iRule |= TDR_HPPRULE_NO_TYPE_DECLARE;
			break;
		case 'O':
			{
				iWriteLen = snprintf(pstOption->szOutPath, sizeof(pstOption->szOutPath), "%s", optarg);
				if ((0 > iWriteLen) || (iWriteLen >= (int)(sizeof(pstOption->szOutPath))))
				{
					printf("指定输出路径信息太长，目前最多支持%u个字符的路径信息\n", sizeof(pstOption->szOutPath) -1);
					exit(EINVAL);
				}
			}
			break;
		case 'f':
			pstOption->bIsOldTag = TDR_TRUE;
			break;
		case 'v':
			printf("%s version: %s at %s\n" , argv[0], tdr_parse_version_string(TDR_BUILD_VERSION), __DATE__);
			exit(0);
			break;
		case 'h':
		case '?':
			tdr_show_help(argv[0]);
			break;
		
		default:
			break;
		}/*switch(opt)*/
	}/*while (1)*/

	/*通过参数获取xml元数据库文件*/
	iFile = 0;
	for (i = optind; i < argc; i++)
	{
		pstOption->paszXMLFile[iFile] = argv[i];
		iFile++;
		if (iFile >= TDR_MAX_FILES_IN_ONE_PARSE)
		{
			printf("本工具目前最多只支持使用%d个XML文件来描述一个元数据描述库\n", TDR_MAX_FILES_IN_ONE_PARSE);
			break;
		}
	}
	if (0 >= iFile)
	{
		printf("至少必须指定一个XML元数据描述文件进行转换\n");
		return -1;
	}
	pstOption->iXMLFileNum = iFile;

	/*检查参数的有效性*/
	if (TDR_HPPRULE_DEFAULT == pstOption->enID )
	{
		printf("必须通过参数(-B,-C或-H)指定特定的转换操作\n");
		return -2;
	}

	return 0;
}






int main(int argc, char *argv[])
{
	TDROPTION stOption;
	int iRet;

	memset(&stOption, 0, sizeof(stOption));
	stOption.bDoDumpLib = TDR_FALSE;
	stOption.bIsOldTag = TDR_FALSE;
	stOption.enID = TDR_OPER_UNDEFINE;
	stOption.stRule.iRule = TDR_HPPRULE_DEFAULT;
	iRet = tdr_def_opt(&stOption, argc, argv);
	if (0 != iRet)
	{
		return iRet;
	}

	switch(stOption.enID)
	{
	case TDR_OPER_XML2DR:
		iRet = tdr_xml2dr(&stOption);
		break;
	case TDR_OPER_XML2C:
		iRet = tdr_xml2c(&stOption);
		break;
	case TDR_OPER_XML2H:
		iRet = tdr_xml2h(&stOption);
		break;
	default:
		break;
	}


	return 0;
}


int tdr_make_path_file(char *pszFileBuff, int iBuff, const char *pszPath, char *pszFile, char *pszSuffix)
{
	int iLen;
	char *pch;

	assert(NULL != pszFileBuff);
	assert(NULL != pszPath);
	assert(NULL != pszFile);
	assert(NULL != pszSuffix);
	assert(0 < iBuff);

	if ('\0' == pszPath[0])
	{
		snprintf(pszFileBuff, iBuff,"%s", pszFile);
	}else
	{
		pch = strrchr(pszFile, TDR_OS_DIRSEP);
		if (NULL == pch)
		{
			pch = pszFile;
		}else
		{
			pch++;
		}	
		iLen = (int)strlen(pszPath);
		if ( TDR_OS_DIRSEP == pszPath[iLen])
		{
			snprintf(pszFileBuff, iBuff,"%s%s", pszPath, pch);
		}else 
		{
			snprintf(pszFileBuff, iBuff,"%s%c%s", pszPath, TDR_OS_DIRSEP, pch);
		}
	}
	

	pch = strrchr(pszFileBuff, '.');
	if (NULL == pch)
	{
		iLen = (int)strlen(pszFileBuff);
		pch = &pszFileBuff[iLen];
	}else
	{
		iLen = (int)(pch - pszFileBuff);
	}
	if ((iBuff - iLen) < (int)strlen(pszSuffix))
	{
		printf("生成的文件名太长，目前最多支持%d个字符\n", iBuff-1);
		return -1;
	}
	strncpy(pch, pszSuffix, iBuff - iLen);

	return 0;
}

int tdr_xml2dr(LPTDROPTION pstOption)
{
	LPTDRMETALIB pstLib=NULL;
	int iRet;
	char szLibFile[TDR_MAX_FILE_LEN]={0};
	int iTagSetVersion = TDR_XML_TAGSET_VERSION_1;


	assert(NULL != pstOption);

	/*生成元数据库的名字*/
	if ('\0' == pstOption->szOutFile[0])
	{
		strncpy(pstOption->szOutFile, TDR_DEFAULT_DR_OUTPUT_FILE, sizeof(pstOption->szOutFile));
	}
	
	iRet = tdr_make_path_file(szLibFile, sizeof(szLibFile), pstOption->szOutPath, pstOption->szOutFile, TDR_DR_FILE_SUFFIX);
	if (0 != iRet)
	{
		return iRet;
	}
		
	if (pstOption->bIsOldTag)
	{
		iTagSetVersion = TDR_XML_TAGSET_VERSION_0;
	}
	iRet = tdr_create_lib_multifile(&pstLib, (const char **)pstOption->paszXMLFile, pstOption->iXMLFileNum,
		iTagSetVersion, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("根据xml描述文件创建元数据库: %s\n", tdr_error_string(iRet));
		return iRet;
	}

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if (pstOption->bDoDumpLib)
		{
			tdr_dump_metalib_file(pstLib, "dump_lib.txt");
		}

		iRet = tdr_save_metalib(pstLib, szLibFile);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("将元数据库失保存为二进制文件<%s>失败, 原因是: %s\n", szLibFile, tdr_error_string(iRet));
		}else
		{
			printf("成功将XML文件转换成二进制元数据文件<%s>\n", szLibFile);
		}
	}

	tdr_free_lib(&pstLib);


	return iRet;
}

int tdr_xml2c(LPTDROPTION pstOption)
{
	LPTDRMETALIB pstLib=NULL;
	int iRet;
	char szCFile[TDR_MAX_FILE_LEN] = {0};
	int iTagSetVersion = TDR_XML_TAGSET_VERSION_1;

	assert(NULL != pstOption);

	if (pstOption->bIsOldTag)
	{
		iTagSetVersion = TDR_XML_TAGSET_VERSION_0;
	}

	/*生成元数据库的名字*/
	if ('\0' == pstOption->szOutFile[0])
	{
		strncpy(pstOption->szOutFile, TDR_DEFAULT_DR_COUTPUT_FILE, sizeof(pstOption->szOutFile));
	}
		
	iRet = tdr_make_path_file(szCFile, sizeof(szCFile), pstOption->szOutPath, pstOption->szOutFile, ".c");
	if (0 != iRet)
	{
		return iRet;
	}

	
	iRet = tdr_create_lib_multifile(&pstLib, (const char **)pstOption->paszXMLFile, pstOption->iXMLFileNum,
		iTagSetVersion, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("根据xml描述文件创建元数据库: %s\n", tdr_error_string(iRet));
		return iRet;
	}

	if (!TDR_ERR_IS_ERROR(iRet))
	{
		if (pstOption->bDoDumpLib)
		{
			tdr_dump_metalib_file(pstLib, "dump_lib.txt");
		}
		iRet = tdr_metalib_to_cfile(pstLib, szCFile);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("将XML描述文件转换成.c文件<%s>失败: %s\n", szCFile, tdr_error_string(iRet));
		}else
		{
			printf("成功将XML描述文件转换成.c文件<%s>\n", szCFile);
		}

	}

	tdr_free_lib(&pstLib);


	return iRet;
}

int tdr_xml2h(LPTDROPTION pstOption)
{
	LPTDRMETALIB pstLib=NULL;
	int iRet;
	char szHFile[TDR_MAX_FILE_LEN] = {0};
	int iTagSetVersion = TDR_XML_TAGSET_VERSION_1;
	int i;

	assert(NULL != pstOption);

	if (pstOption->bIsOldTag)
	{
		iTagSetVersion = TDR_XML_TAGSET_VERSION_0;
	}

	iRet = tdr_create_lib_multifile(&pstLib, (const char **)pstOption->paszXMLFile, pstOption->iXMLFileNum,
		iTagSetVersion, stderr);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("根据xml描述文件创建元数据库: %s\n", tdr_error_string(iRet));
		return iRet;
	}
	if (pstOption->bDoDumpLib)
	{
		tdr_dump_metalib_file(pstLib, "dump_lib.txt");
	}
	for (i = 0 ; i < pstOption->iXMLFileNum; i++)
	{
		
		iRet = tdr_make_path_file(szHFile, sizeof(szHFile), pstOption->szOutPath, pstOption->paszXMLFile[i], ".h");


		iRet = tdr_metalib_to_hpp_spec(pstLib, (const char *) pstOption->paszXMLFile[i], iTagSetVersion,
			szHFile, &pstOption->stRule, stderr);		
		if (TDR_ERR_IS_ERROR(iRet))
		{
			break;
		}else
		{
			printf("成功将XML描述文件<%s>转换成.h文件<%s>\n", pstOption->paszXMLFile[i], szHFile);
		}

	}/*for (i = 0 ; i < stOption.iXMLFileNum; i++)*/


	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("将XML描述文件<%s>转换成.h文件失败: %s\n", pstOption->paszXMLFile[i], tdr_error_string(iRet));
	}

	tdr_free_lib(&pstLib);
	
	return iRet;
}

