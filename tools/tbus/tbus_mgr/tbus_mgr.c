/** @file $RCSfile: tbus_mgr.c,v $
  general description of this module
  $Id: tbus_mgr.c,v 1.14 2009/03/24 03:00:40 jacky Exp $
@author $Author: jacky $
@date $Date: 2009/03/24 03:00:40 $
@version $Revision: 1.14 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/



#include "tdr/tdr.h"
#include "tbus/tbus.h"
#include "tbus/tbus_config_mng.h"
#include "tbus_mgr.h"
#include "tbus_view_channel.h"
#include "tbus/tbus_log.h"
#include "tbus/tbus_kernel.h"

#define TBUSMGR_CONFINFO "TbusMgr"
#define TBUS_GCIM_CONF_INFO "TbusGCIM"

#define TBUS_MGR_MAJOR  0x00
#define TBUS_MGR_MINOR  0x00
#define TBUS_MGR_REV    0x02
#define TBUS_MGR_BUILD  0x02

#define TBUS_MGR_CONF_XML "tbusmgr.xml"






extern unsigned char g_szMetalib_TbusConf[] ;


/**
  various internal function defined
*/
static int commandLine ( int argc, const char *argv[], LPTBUSMGROPTIONS a_pstOption) ;
static int initialize (LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM, OUT LPTBUSSHMGCIM *a_ppShmGCIM) ;
static void printHelp ( ) ; 

static int list ( IN LPTBUSSHMGCIM a_pShmGCIM ) ;
static int set (IN LPTBUSGCIM a_pstGCIM, IN LPTBUSSHMGCIM a_pShmGCIM) ;
static int del ( IN LPTBUSSHMGCIM a_pShmGCIM, int a_idx ) ;

static int load_config_of_old_format(LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM);


void printHelp ( ) 
{
	printf ( "\n" ) ;
	printf ( "Routes managing tool of tbus module for IERD. Version %d.%d.%dbuild%d.\n",
		TBUS_MGR_MAJOR, TBUS_MGR_MINOR, TBUS_MGR_REV, TBUS_MGR_BUILD);
	printf ( "Usage:\n" ) ;
	printf ( "tbusmgr ([-W,--write] | [-D,--delete=ID] | [-S,--see=PROCESSID [ [-m,--meta-name=NAME]\n"
		"	[-B,--meta-lib=FILE] [-o,--out-file] [-n,--see-msg-num]] |[-L,--list]) "
		"	[-C,--conf-file=FILE][-k,--bus-key=KEY] [--old-conf] [--debug][-v] [-h]\n" );
	printf ( "主要操作选项:\n " ) ;
	printf ( "-W,--write\t写GCIM(全局通道信息)\n"
			 "\t根据配置将Tbus系统的路由表信息写到共享内存中，使用本操作还必须通过\n"
			 "\t--conf-file指定Tbus系统的路由信息的配置文件\n" );	
	printf ( "-L,--list\t查看GCIM(全局通道信息)\n"
		"\t查看共享内存中的路由表，本操作必须指定--conf-file或--bus-key选项\n");
	printf ( "-D,--delete=INDEX\t删除GCIM(全局通道信息)中配置通道\n"
		"\t删除共享内存中Tbus路由表中指定索引值的通道，索引值INDEX从1开始编码，\n"
		"\t本操作必须指定--conf-file或--bus-key选项\n");
	printf ( "-S,--see=PROCESSID\t查看进程相关通道的消息概貌\n"
		"\t查看某个进程实体绑定的所有Tbus通道的消息，通过PROCESSID指定此进程实体ID，.\n"
		"\t本操作必须指定--conf-file或--bus-key选项,\n" );
	printf ( "公共配置项:\n");	
	printf ( "-C,--conf-file=FILE\n"
			"\t指定包含Tbus路由信息的配置文件. 如果没有指定配置文件，则试用缺省,\n"
			"\t配置文件\"%s\" will be uesed\n", TBUS_MGR_CONF_XML) ;	
	printf ( "-k,--bus-key=key\n"
		"\t指定存储Tbus路由表信息的共享内存key值，如果指定本选项的同时也指定了\n") ;
	printf("\t--conf-file选项，则存储路由表的共享内存key以本选项为准\n");
	printf("\t--old-conf选项，指定xml配置文件为老的格式，即通道地址通过UpAddr,DownAddr做区分的\n"
		"\t新的配置文件不再做这种区分，且通道的共享内存key也不用配置，将自动生成\n");
	printf ( "-g,--debug\t 打印一些调试信息\n" ) ;
	printf ( "-v,--version, -v\n"
			"\t 查看工具版本\n" ) ;
	printf ( "-h,--help, -h\n"
		"\t 查看本工具的帮助\n" ) ;
	printf ( "--see特有配置项:\n");
	printf ( "-B,--meta-lib\n"
		"\t指定tbus通道中消息对应数据结构的元数据描述文件\n");
	printf("-m,--meta_name\n"
		"\t指定tbus通道中消息对应数据结构在元数据描述库中的名字") ;	
	printf("-o,--out-file\n"
		"\t指定tbus通道中消息信息导出到指定文件中") ;	
	printf("-n,--see-msg-num\n"
		"\t详细察看的消息总数") ;	
	printf ( "\n" ) ;
	printf("试用示例:\n");
	printf("tbus_mgr --conf-file=../conf/tbusmgr.xml --write\n");
	printf("tbus_mgr --conf-file=../conf/tbusmgr.xml --write --old-conf\n");
	printf("tbus_mgr --see=1.2.3.4 --bus-key=20001\n");
	printf("tbus_mgr -S 1.2.3.4 -k 20001 -B cusprotocol.tdr -m CusPkg\n");
	printf("tbus_mgr --delete=2\n");
	printf("tbus_mgr --list --conf-file=../conf/tbusmgr.xml\n");
	printf ( "Authors: ARCH,IERD,Tencent\n" );
	printf ( "Bugs:    g_IERD_Rnd_Architecture@tencent.com\n" );
	printf ( "\n" ) ;
	return;
}


int main ( int argc, char ** argv ) 
{
	TBUSMGROPTIONS	stOption;
	TBUSGCIM	stGCIM;
	LPTBUSSHMGCIM pstShmGCIM = NULL;
	int iRet = 0;

	//printf("sizeof time_t:%d\n", sizeof(time_t));
	/* get options from command line */
	memset(&stOption, 0, sizeof(TBUSMGROPTIONS));
	stOption.iMgrOP = TBUS_MGR_OP_LIST;
	STRNCPY(stOption.szConfFile, TBUS_MGR_CONF_XML, sizeof(stOption.szConfFile) ) ;
	stOption.iDelID = -1;
	stOption.iMaxSeeMsg = TBUS_MGR_DEFAULT_SEE_MAX_NUM;	
	if ( commandLine ( argc, argv , &stOption) != 0 ) 
	{
		printf("failed to parse commandline, please check your args");
		return -1 ;
	}

	/* load config and attach shm*/
	if ( 0 != initialize (&stOption, &stGCIM, &pstShmGCIM) ) 
		return -1 ;

	switch(stOption.iMgrOP)
	{
	case TBUS_MGR_OP_WRITE:
		{
			iRet = set(&stGCIM, pstShmGCIM) ;
		}
		break;
	case TBUS_MGR_OP_DELETE:
		{
			if ( 0 < stOption.iDelID )
			{
				iRet = del(pstShmGCIM, stOption.iDelID) ;
				if ( TBUS_SUCCESS == iRet )
				{
					list(pstShmGCIM) ;
				}
			}
		}
		break;
	case TBUS_MGR_OP_VIEW:
		{
			iRet = ViewChannels(pstShmGCIM, &stOption);
		}
		break;
	default:
		iRet = list(pstShmGCIM) ;
	}

	return iRet;
}



int commandLine ( int argc, const char * argv[] , LPTBUSMGROPTIONS a_pstOption)
{
	static struct option s_astLongOptions[] = {
		{"conf-file", 1, NULL, 'C'},
		{"write", 0, NULL, 'W'},
		{"see", 1, NULL, 'S'},
		{"delete", 1, NULL, 'D'},
		{"list", 0, NULL, 'L'},
		{"bus-key", 1, NULL, 'k'},
		{"meta-lib", 1, NULL, 'B'},
		{"meta-name", 1, NULL, 'm'},
		{"out-file", 1, NULL, 'o'},
		{"see-msg-num", 1, NULL, 'n'},
		{"old-conf", 0, NULL, 'f'},
		{"debug", 0, NULL, 'g'},
		{"version", 0, 0, 'v'},		
		{"help", 0, 0, 'h'},

		{0, 0, 0, 0}
	};
	int opt = 0;
	char* pszApp = NULL ;

	assert(NULL != a_pstOption);

	pszApp = basename ( argv[0] ) ;
	
	while (1)
	{
		int option_index = 0;

		opt = getopt_long (argc, argv, "n:C:c:S:s:D:d:k:K:B:b:m:M:o:O:gLlwWfvh",
			s_astLongOptions, &option_index);

		if (opt == -1)
			break;

		switch(opt)
		{
		case 'C':
		case 'c':
			STRNCPY(a_pstOption->szConfFile, optarg, sizeof(a_pstOption->szConfFile) ) ;
			break;
		case 'W':
		case 'w':
			a_pstOption->iMgrOP = TBUS_MGR_OP_WRITE ;
			break;
		case 'D':
		case 'd':
			a_pstOption->iMgrOP = TBUS_MGR_OP_DELETE;
			a_pstOption->iDelID = atoi(optarg) ;
			break;
		case 'S':
		case 's':
			{
				STRNCPY(a_pstOption->szProcID, optarg, sizeof(a_pstOption->szProcID));
				a_pstOption->iMgrOP = TBUS_MGR_OP_VIEW;
			}
			break;
		case 'L':
		case 'l':
			{
				a_pstOption->iMgrOP = TBUS_MGR_OP_LIST ;
				break;
			}
		case 'k':
		case 'K':
			{
				STRNCPY(a_pstOption->szShmKey, optarg, sizeof(a_pstOption->szShmKey)) ;
				break;
			}
		case 'n':
			{
				a_pstOption->iMaxSeeMsg = atoi(optarg) ;
				break;
			}
		case 'B':
		case 'b':
			{
				STRNCPY(a_pstOption->szMetalibFile, optarg, sizeof(a_pstOption->szMetalibFile));
				break;
			}
		case 'm':
		case 'M':
			{
				STRNCPY(a_pstOption->szMetaName, optarg, sizeof(a_pstOption->szMetaName));
				break;
			}
		case 'o':
			{
				STRNCPY(a_pstOption->szOutFile, optarg, sizeof(a_pstOption->szOutFile));
				break;
			}
		case 'f':
			{
				a_pstOption->iIsOldCnf = 1; /*老格式*/
				break;
			}
		case 'g':
			{
				a_pstOption->iIsDebug= 1; /*debug*/
				break;
			}
		case 'v': /* show version. */
			printf("%s:version %d.%d.%dbuild%d.\n", pszApp, TBUS_MGR_MAJOR, TBUS_MGR_MINOR, TBUS_MGR_REV, TBUS_MGR_BUILD);
			return -1 ;
			break;

		case 'h':
		case '?':
			printHelp() ;
			return -1 ;
		default:
			break;
		}/*switch(opt)*/				
	}

	return 0 ;
}


int initialize (LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM, OUT LPTBUSSHMGCIM *a_ppShmGCIM) 
{
	int iRet = TBUS_SUCCESS ;
	unsigned int iSize = 0 ;
	LPTDRMETALIB ptLib = NULL ;
	LPTDRMETA ptMeta = NULL ;
	TDRDATA stDataTemp ;

	assert(NULL != a_pstOption);
	assert(NULL != a_pstGCIM);
	assert(NULL != a_ppShmGCIM);

	/*初始化日志系统*/
	tbus_init_log();
	tbus_set_logpriority(TLOG_PRIORITY_TRACE);


	/*没有指定路由表的共享内存key，或者要写GCIM，则尝试读取配置文件*/
	if (('\0' == a_pstOption->szShmKey[0]) || (TBUS_MGR_OP_WRITE == a_pstOption->iMgrOP))
	{
		if (!a_pstOption->iIsOldCnf)
		{
			ptLib = (LPTDRMETALIB) g_szMetalib_TbusConf ;
			ptMeta = tdr_get_meta_by_name ( ptLib, TBUS_GCIM_CONF_INFO ) ;
			if ( NULL == ptMeta )
			{
				fprintf ( stdout, "ERROR: initialize, tdr_get_meta_by_name(%s) failed\n",
					TBUS_GCIM_CONF_INFO) ;
				return -1 ;
			}

			stDataTemp.iBuff = sizeof(TBUSGCIM) ;
			stDataTemp.pszBuff = (char *)a_pstGCIM ;
			iRet = tdr_input_file ( ptMeta, &stDataTemp, a_pstOption->szConfFile, 
				0, TDR_IO_NEW_XML_VERSION ) ;
			if ( 0 > iRet )
			{
				fprintf ( stdout, "initialize, tdr_input_file() failed %s, error %s\n", 
					a_pstOption->szConfFile, tdr_error_string(iRet) ) ;
				return -1 ;
			}

			if (a_pstOption->iIsDebug)
				tdr_fprintf(ptMeta, stdout, &stDataTemp, 0);

			
		}else
		{
			iRet = load_config_of_old_format(a_pstOption, a_pstGCIM);
			if (0 != iRet)
			{
				return -1;
			}
		}/*if (!a_pstOption->iIsOldCnf)*/		
		STRNCPY(a_pstOption->szShmKey, a_pstGCIM->szGCIMShmKey, 
			sizeof(a_pstOption->szShmKey));
	}/*if ((0 > a_pstOption->iShmKey) || (TBUS_MGR_OP_WRITE == a_pstOption->iMgrOP))*/
		

	/* attach routes share memory */
	

	if (TBUS_MGR_OP_WRITE == a_pstOption->iMgrOP)
	{
		iSize = sizeof(TBUSSHMGCIM);
		iRet = tbus_create_gcimshm(a_ppShmGCIM, a_pstOption->szShmKey, a_pstGCIM->iBussinessID,
			iSize);
	}else
	{
		iRet = tbus_get_gcimshm(a_ppShmGCIM, a_pstOption->szShmKey, a_pstGCIM->iBussinessID,
			iSize, 0);
	}	
	if (0 != iRet)
	{
		printf("failed to get GCIM share memory by shmkey(%s) and bussinessid %d\n",
			a_pstOption->szShmKey, a_pstGCIM->iBussinessID);
	}

	return iRet ;
}

int load_config_of_old_format(LPTBUSMGROPTIONS a_pstOption, OUT LPTBUSGCIM a_pstGCIM)
{
	TBUSMGR stTbusMgr;
	LPTDRMETALIB ptLib = NULL ;
	LPTDRMETA ptMeta = NULL ;
	TDRDATA stDataTemp ;
	int iRet = 0;
	unsigned int i;

	assert(NULL != a_pstOption);
	assert(NULL != a_pstGCIM);

	ptLib = (LPTDRMETALIB) g_szMetalib_TbusConf ;
	ptMeta = tdr_get_meta_by_name ( ptLib, TBUSMGR_CONFINFO ) ;
	if ( NULL == ptMeta )
	{
		fprintf ( stdout, "ERROR: initialize, tdr_get_meta_by_name(%s) failed\n",
			TBUSMGR_CONFINFO) ;
		return -1 ;
	}

	memset(&stTbusMgr, 0, sizeof(stTbusMgr));
	stDataTemp.iBuff = sizeof(stTbusMgr) ;
	stDataTemp.pszBuff = (char *)&stTbusMgr ;
	iRet = tdr_input_file ( ptMeta, &stDataTemp, a_pstOption->szConfFile, 
		0, TDR_IO_NEW_XML_VERSION ) ;
	if ( 0 > iRet )
	{
		fprintf ( stdout, "initialize, tdr_input_file() failed %s, error %s\n", 
			a_pstOption->szConfFile, tdr_error_string(iRet) ) ;
		return -1 ;
	}

	/*convert old format data to new format*/
	a_pstGCIM->iBussinessID = TBUS_DEFAUL_BUSSINESS_ID;
	snprintf(a_pstGCIM->szGCIMShmKey, sizeof(a_pstGCIM->szGCIMShmKey), "%u", stTbusMgr.dwRoutesShmKey);
	STRNCPY(a_pstGCIM->szAddrTemplet, TBUS_DEFAULT_ADDRESS_TEMPLET, sizeof(a_pstGCIM->szAddrTemplet));
	for (i = 0; i < stTbusMgr.dwUsedCnt; i++)
	{
		LPCHANNELCNF pstChannel = &a_pstGCIM->astChannels[i];
		LPROUTEINFO pstRoute = &stTbusMgr.astRoutes[i];
		pstChannel->dwPriority = pstRoute->dwPriority;
		STRNCPY(pstChannel->aszAddress[0], pstRoute->szUpAddr, sizeof(pstChannel->aszAddress[0]));
		STRNCPY(pstChannel->aszAddress[1], pstRoute->szDownAddr, sizeof(pstChannel->aszAddress[1]));
		pstChannel->dwRecvSize = pstRoute->dwUpSize;
		pstChannel->dwSendSize = pstRoute->dwDownSize;
		pstChannel->dwAddressCount = 2;
		STRNCPY(pstChannel->szDesc, pstRoute->szDesc, sizeof(pstChannel->szDesc));
	}/*for (i = 0; i < stTbusMgr.dwUsedCnt; i++)*/
	a_pstGCIM->dwCount = i;

	return 0;
}



int list ( IN LPTBUSSHMGCIM a_pShmGCIM ) 
{
	int iRet = TBUS_SUCCESS ;
	unsigned int i = 0 ;
	LPTBUSSHMGCIMHEAD pstHead ;
	LPTBUSSHMCHANNELCNF pstShmChl;

	assert(NULL != a_pShmGCIM);

	pstHead = &a_pShmGCIM->stHead;
	fprintf ( stdout, "\n========================\n" ) ;

	tbus_rdlock(&pstHead->stRWLock);	

	/* share memory header list */
	fprintf ( stdout, "Shm version:\t%i\n", pstHead->dwVersion ) ;
	fprintf ( stdout, "Shm key:\t%i\n", pstHead->dwShmKey ) ;
	fprintf ( stdout, "Shm size:\t%i\n", pstHead->dwShmSize ) ;
	fprintf ( stdout, "DataAlign:\t%d  \nAlignLevel:%d\n", pstHead->iAlign, pstHead->iAlignLevel) ;
	fprintf ( stdout, "Routes max count:\t%i\n", pstHead->dwMaxCnt ) ;
	fprintf ( stdout, "Routes used count:\t%i\n", pstHead->dwUsedCnt ) ;
	tbus_dump_addrtemplet(&pstHead->stAddrTemplet, stdout);
	fprintf ( stdout, "\n" ) ;


	/* share memory routes information list */	
	for ( i=0; i<pstHead->dwUsedCnt; i++ )
	{
		pstShmChl = &a_pShmGCIM->astChannels[i];
		fprintf ( stdout, "Route %i information:\n", i+1 ) ;

		if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstShmChl))
		{
			fprintf ( stdout, "\tStatus:0x%x -- disable\n",  pstShmChl->dwFlag) ;
		}
		else
		{
			fprintf ( stdout, "\tStatus:0x%x -- enable\n", pstShmChl->dwFlag) ;
		}

		fprintf ( stdout, "\tPriority -- %i\n", pstShmChl->dwPriority ) ;
		fprintf ( stdout, "\tShmID -- %d\n", pstShmChl->iShmID ) ;
		fprintf ( stdout, "\tAddress -- %s\n", 
			tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[0]) ) ;
		fprintf ( stdout, "\tAddress -- %s\n", 
			tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[1]) ) ;
		fprintf ( stdout, "\t[%s] RecvQueueSize -- %u\n", 
			tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[0]),
			pstShmChl->dwRecvSize ) ;
		fprintf ( stdout, "\t[%s] SendQueueSize -- %u\n", 
			tbus_addr_nota_by_addrtemplet(&pstHead->stAddrTemplet, pstShmChl->astAddrs[0]),
			pstShmChl->dwSendSize ) ;
		fprintf ( stdout, "\tCreateTime -- %s", ctime(&pstShmChl->dwCTime)) ;
		if (!TBUS_GCIM_CHANNEL_IS_ENABLE(pstShmChl))
		{
			fprintf ( stdout, "\tDisableTime -- %s\n",  ctime(&pstShmChl->dwInvalidTime)) ;
		}
		else
		{
			fprintf ( stdout, "\tDisableTime -- N/A\n") ;
		}
	}	

	tbus_unlock(&pstHead->stRWLock);



	return iRet ;
}


int set (IN LPTBUSGCIM a_pstGCIM, IN LPTBUSSHMGCIM a_pShmGCIM) 
{
	int iRet = TBUS_SUCCESS;


	
	iRet = tbus_set_gcim(a_pShmGCIM, a_pstGCIM);
	if (TBUS_SUCCESS != iRet)
	{
		fprintf (stdout, "\nRoutes writing  failed\n\n" ) ;
	}else
	{
		list(a_pShmGCIM);
		fprintf (stdout, "\nRoutes writing...... done\n\n" ) ;
	}
	
	return iRet ;
}


int del ( IN LPTBUSSHMGCIM a_pShmGCIM, int a_iID ) 
{
	int iRet = TBUS_SUCCESS ;




	
	
	iRet = tbus_delete_channel_by_index(a_pShmGCIM, a_iID -1);
	if (TBUS_SUCCESS != iRet)
	{
		fprintf ( stdout, "\nfailed to delete Specified 'Route %i' \n\n", a_iID) ;
	}else
	{
		fprintf ( stdout, "\nSpecified 'Route %i' deleted...... done\n\n", a_iID) ;

	}

	return iRet ;
}


