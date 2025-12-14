/**
*
* @file     tdr_dr2xml.c  
* @brief    将DR二进制文件转换成XML文件的控制台工具
* 
* @author jackyai  
* @version 1.0
* @date 2007-06-01 
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
#define	TDR_DR2XML_BUILD_VERSION		1101010		//0.1 Beta01 build 010
#define TDR_MAX_OUTPUT_FILE_LEN			256 /**<输出文件的名字最大长度*/


#define  TDR_DEFAULT_DR_OUTPUT_FILE		"a.xml"

struct tagTDRDR2Option 
{
	char *pszInputFile;
	char szOutputFileName[TDR_MAX_OUTPUT_FILE_LEN]; /**<输出文件的名字*/
};
typedef struct tagTDRDR2Option   TDRDR2XMLOPTION;
typedef struct tagTDRDR2Option *LPTDRDR2XMLOPTION;

static void tdr_show_help(const char *a_pszObj);

/**分析运行工具的选项
*@retval 0 正确分析出各选项
*@retval >0 正确分析出各选项,但指示此运行工具不必继续执行
*@retval <0 分析选项失败
*/
static int tdr_parse_option(OUT LPTDRDR2XMLOPTION a_pstOption, IN int a_argc, IN char *a_argv[]);

int main(int argc, char *argv[])
{
	LPTDRMETALIB pstLib=NULL;
	TDRDR2XMLOPTION stOption;
	int iRet;

	memset(&stOption, 0, sizeof(stOption));
	strncpy(stOption.szOutputFileName, TDR_DEFAULT_DR_OUTPUT_FILE, TDR_MAX_OUTPUT_FILE_LEN);
	iRet = tdr_parse_option(&stOption, argc, argv);	
	if (0 != iRet)
	{
	//	getchar();
		return iRet;
	}

	
	iRet = tdr_load_metalib(&pstLib, stOption.pszInputFile);
	if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("从二进制元数据文件加载元数据库失败, 原因是: %s\n", tdr_error_string(iRet));
	}


	if (!TDR_ERR_IS_ERROR(iRet))
	{
		iRet = tdr_save_xml_file(pstLib, stOption.szOutputFileName);
		if (TDR_ERR_IS_ERROR(iRet))
		{
			printf("将元数据库转换成XML格式的文件失败, 原因是: %s\n", tdr_error_string(iRet));
		}else
		{
			printf("成功将二进制元数据文件转换成XML格式的描述文件<%s>\n", stOption.szOutputFileName);
		}
	}

	tdr_free_lib(&pstLib);

	//getchar();

	return iRet;
}

void tdr_show_help(const char *a_pszObj)
{
	assert(NULL != a_pszObj);
	printf("用法: %s [-o --out_file=FILE] [-h --help] [-v --version] DRFILE \n", a_pszObj);
	printf("将二进制格式元数据文件转换成一个XML格式的元数据描述文件, 参数DRFILE指定保存二进制格式元数据文件的文件名\n");
	printf("选项:\n");
	printf("-o, --out_file=FILE 指定输出文件的名字,缺省为%s\n", TDR_DEFAULT_DR_OUTPUT_FILE);
	printf("-h, --help 输出使用帮助\n");
	printf("-v, --version 输出版本信息\n");
}

int tdr_parse_option(OUT LPTDRDR2XMLOPTION a_pstOption, IN int a_argc, IN char *a_argv[])
{
	int opt;
	int iRet = 0;

	assert(NULL != a_pstOption);
	assert(0 < a_argc);
	assert(NULL != a_argv);

	while (1)
	{
		//int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option stlong_options[] = {
			{"help", 0, 0, 'h'},
			{"out_file", 1, 0, 'o'},
			{"version", 0, 0, 'v'},
			{0, 0, 0, 0}
		};

		opt = getopt_long (a_argc, a_argv, "o:hv",
			stlong_options, &option_index);

		if (opt == -1)
			break;

		if ((opt == '?') || (opt == ':') )
		{
			iRet = 1;
			break;
		}

		switch( opt )
		{
		case 'o':
			{
				if (NULL == strrchr(optarg, '.'))
				{
					snprintf(a_pstOption->szOutputFileName, TDR_MAX_OUTPUT_FILE_LEN -1, "%s.xml", optarg);
				}else
				{
					snprintf(a_pstOption->szOutputFileName, TDR_MAX_OUTPUT_FILE_LEN -1, "%s", optarg);
				}
				a_pstOption->szOutputFileName[TDR_MAX_OUTPUT_FILE_LEN -1] = '\0';

				break;
			}
		
		case 'h': /* show the help information. */
			{
				iRet = 1;
				tdr_show_help(a_argv[0]);
				break;
			}
		case 'v':
			{
				iRet = 1;
				printf("%s version: %s\n" , a_argv[0], tdr_parse_version_string(TDR_DR2XML_BUILD_VERSION));
				break;
			}
		default:
			{
				iRet = -1;
				printf("%s: 无效选项 --%c \n", a_argv[0], (char)opt);
				printf("可以执行 xml2dr -h 获取更多信息\n");
				break;
			}			
		}
	}/*while( -1 != (opt=getopt(argc, argv, "o:t:hv")) )*/

	if (0 != iRet)
	{
		return iRet;
	}

	if (optind >= a_argc)
	{
		printf("必须通过参数指定保存元数据库二进制文件的文件名\n");
		return -2;
	}
	a_pstOption->pszInputFile = a_argv[optind];

	return iRet;
}



