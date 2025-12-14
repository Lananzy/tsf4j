// resprint.cpp : Defines the entry point for the console application.
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tgetopt.h"
#include "resprint.h"
#include "tdr/tdr.h"


#define MAX_PATH_LEN			260 /**<文件目录或文件名最大长度*/

struct tagResPrintOption 
{
	char szMetaName[TDR_NAME_LEN];		/**<元数据名*/
	char szMetalib[MAX_PATH_LEN];	/**<二进制元数据文件*/
	char szResFile[MAX_PATH_LEN];	/**<二进制资源文件文件*/
	char szMetaSetName[TDR_NAME_LEN];
	char szMetaCountName[TDR_NAME_LEN];
};
typedef struct tagResPrintOption   RESPRINTOPTION;
typedef struct tagResPrintOption *LPRESPRINTOPTION;

int ParseOption(OUT RESPRINTOPTION *pstOption, IN int a_argc, IN char *a_argv[]);


void ShowHelp()
{
	printf("Usage: resprint -B metalib_binfile -M meta [-h]  resfile \n");
	printf("-B: specify the binary file of metalib\n");
	printf("-M: specify Meta Name\n");
	printf("-S: specify Meta set Name\n");
	printf("-C: specify Meta count Name\n");
	printf("-h: show this information\n");
}

int main(int argc, char* argv[])
{
	RESPRINTOPTION stOption;
	int iRet;
	char *pchDot;
	char szXmlFile[MAX_PATH_LEN];
	int iLen;

	memset(&stOption, 0, sizeof(stOption));
	iRet = ParseOption(&stOption, argc, argv);	
	if (0 != iRet)
	{
		return iRet;
	}
	
	strncpy(szXmlFile, stOption.szResFile, sizeof(szXmlFile));
	pchDot = strchr(szXmlFile, '.');
	if (NULL != pchDot)
	{
		*pchDot = '\0';
	}
	iLen = strlen(szXmlFile) + strlen(".xml");	
	if (iLen >= (int)sizeof(szXmlFile))
	{
		printf("资源文件的文件名<%s>太长\n", stOption.szResFile);
	}
	strcat(szXmlFile, ".xml");

		

	iRet = res_print(stOption.szResFile, szXmlFile, stOption.szMetalib, stOption.szMetaName,
		stOption.szMetaSetName, stOption.szMetaCountName);

	return iRet;
}

int ParseOption(OUT RESPRINTOPTION *pstOption, IN int a_argc, IN char *a_argv[])
{
	int opt;
	int iRet = 0;

	assert(NULL != pstOption);
	assert(0 < a_argc);
	assert(NULL != a_argv);

	while (1)
	{
		//int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option stlong_options[] = {
			{"help", 0, 0, 'h'},
			{"metalib_binfile", 1, 0, 'b'},
			{"meta", 0, 0, 'm'},
			{"metasetname", 0, 0, 's'},
			{"metacountname", 0, 0, 's'},
			{0, 0, 0, 0}
		};

		opt = getopt_long (a_argc, a_argv, "S:s:c:C:B:b:m:M:h",
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
		case 'B':
		case 'b':
			{
				strncpy(pstOption->szMetalib, optarg, sizeof(pstOption->szMetalib));
				pstOption->szMetalib[sizeof(pstOption->szMetalib) - 1] = '\0';	

				break;
			}
		case 'M':
		case 'm':
			{
				strncpy(pstOption->szMetaName, optarg, sizeof(pstOption->szMetaName));
				pstOption->szMetaName[sizeof(pstOption->szMetaName) - 1] = '\0';

				break;
			}
		case 'S':
		case 's':
			{
				strncpy(pstOption->szMetaSetName, optarg, sizeof(pstOption->szMetaSetName));
				pstOption->szMetaSetName[sizeof(pstOption->szMetaSetName) - 1] = '\0';

				break;
			}
		case 'C':
		case 'c':
			{
				strncpy(pstOption->szMetaCountName, optarg, sizeof(pstOption->szMetaCountName));
				pstOption->szMetaCountName[sizeof(pstOption->szMetaCountName) - 1] = '\0';

				break;
			}
		case 'h': /* show the help information. */
			{
				iRet = 1;
				ShowHelp();
				break;
			}		
		default:
			{
				iRet = -1;
				printf("%s: 无效选项 --%c \n", a_argv[0], (char)opt);
				printf("可以执行 resprint -h 获取更多信息\n");
				break;
			}			
		}
	}/*while (1)*/

	if (0 != iRet)
	{
		return iRet;
	}

	if (pstOption->szMetalib[0] == '\0')
	{
		printf("必须通过参数指定二进制元数据库文件\n");
		return -2;
	}
	if (pstOption->szMetaName[0] == '\0')
	{
		printf("必须通过参数指定资源文件结构体的元数据描述名\n");
		return -3;
	}

	if (optind >= a_argc)
	{
		printf("必须通过参数指定保存二进制资源文件\n");
		return -4;
	}
	strncpy(pstOption->szResFile, a_argv[optind], sizeof(pstOption->szResFile));
	pstOption->szResFile[sizeof(pstOption->szResFile) - 1] = '\0';


	return iRet;
}


