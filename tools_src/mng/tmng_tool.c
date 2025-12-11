#include    <stdio.h>
#include    <stdlib.h>
#include    <curses.h>

#include    "mng/tmng.h"
#include    "comm/tini.h"
#include    "tdr/tdr.h"
//#include    "tlog/log4c.h"
#include    "tdr_metalib_kernel_i.h"
#include    "tdr_auxtools.h"

#define     CUR_VER    0x01

#define     CREATE_MIB          1
#define     CREATE_METEBASE     2

#define     DESTORY_MIB         3
#define     DESTORY_METEBASE    4

#define     DUMP_MIB            5
#define     DUMP_METEBASE       6

#define     LOAD_DATA           7
#define     LOAD_META           8

#define     REGIST_DATA         9
#define     MODIFY_DATA         10

#define     LIST_PROC           61


typedef struct tagGlobal
{
	int     argc;
	char**  argv;
    int     iAction;

    const char *pszLoadfile; 
    const char *pszAlias;
    const char *pszDomain;
    const char *pszMeta;
    const char *pszLib;
    const char *pszPath;
    const char *pszValue;
    int     iMibDataVersion;
    int     iDataSize;
    int     iProcId;
    int     iPeriods;

	char    szConfFile[80];
    char    szMibKey[KEY_SIZE];
    int     iMibCounts;
    int     iMibSize;
  
    char    szMetabaseKey[KEY_SIZE];
    int     iMetabaseCounts;
    int     iMetabaseSize;
}   GOLBALCFG;  

//log4c_category_t *g_ptLog = NULL ;

void FormatTime(time_t tSecond, char* sBuf, int iSize) 
{
    struct tm *pstm;

    if ( iSize < 20 ) 
    {
        sBuf[0]=0;
        return;
    }
    pstm = localtime( &tSecond );
    strftime( sBuf, iSize, "%m-%d %H:%M:%S", pstm);
    return;
}

int _usage(char* argv[] )
{
	const char* pszApp;

	pszApp	=	basename(argv[0]);

	printf("Common Usage: %s [options] \n", pszApp);
	printf("options:\n");
	printf("\t--create     : specify [mib|metabase]\n");
	printf("\t--destroy    : specify [mib|metabase]\n");    
    printf("\t--dump       : specify [mib|metabase]\n");    
    printf("\t               option  use with argumet --alias , default dump all data\n");    
    printf("\t               option  use with argumet --domain to specify data domain , default domain =\"CFG\"\n");     
    printf("\t               option  use with argumet --data-ver to specify version \n");            
    printf("\t               option  use with argumet --procid to specify data owner proc \n");    
    printf("\t--loadmeta   : specify meta lib filename\n");    
    printf("\t--loaddata   : specify data file use DR description\n");
    printf("\t               must be use with argumet --alias --lib --meta \n");    
    printf("\t               option  use with argumet --domain to specify data domain , default domain =\"CFG\"\n");     
    printf("\t               option  use with argumet --lib --meta  or --size to specify data size\n");        
    printf("\t               option  use with argumet --data-ver to specify version \n");        
    printf("\t               option  use with argumet --periods to specify report periods\n");        
    printf("\t               option  use with argumet --procid to specify data owner proc \n");        
    printf("\t--registdata : to regist data space in mib\n");
    printf("\t               must be use with argumet --alias to specify data name in mib\n");    
    printf("\t               option  use with argumet --domain to specify data domain , default domain =\"CFG\"\n");     
    printf("\t               option  use with argumet --data-ver to specify version \n");        
    printf("\t               option  use with argumet --procid to specify data owner proc \n"); 
    printf("\t               option  use with argumet --periods to specify report periods\n");            
    printf("\t--modify     : must be use with argumet --alias --path --value \n");
    printf("\t               option  use with argumet --domain to specify data domain, default domain =\"CFG\"\n");     
    printf("\t               option  use with argumet --data-ver to specify version \n");  
    printf("\t               option  use with argumet --procid to specify data owner proc \n");        
    printf("\t               option  use with argumet --periods to specify report periods\n");            
    printf("\t--alias      : specify alias of cfg data used in loadcfg or modify action.\n");
    printf("\t--domain     : specify domain of mib data used in loadcfg or modify action.\n");
    printf("\t--lib        : specify meta lib used in loadcfg action.\n");
    printf("\t--meta       : specify meta name used in loadcfg action.\n");
    printf("\t--data-ver   : specify data version used in loadcfg or modify action.\n");
    printf("\t--Periods    : specify data report periods used in registdata or modify action.\n");    
    printf("\t--procid     : specify data owner procid used in loadcfg , modify or dump action.\n");
    printf("\t--path       : specify entry path used in modify action.\n");
    printf("\t--values     : specify data value used in modify action.\n");
    printf("\t--size       : specify data size  used in regist data action.\n");
	printf("\t--config-file: \n");
	printf("\t          -C : specify the path of config file for this process.\n");    
	printf("\t--log-file   : specify the path of the file for logging.\n");
	printf("\t--listproc,-l: list process data \n");
	printf("\t--version,-v : print version information. \n");
	printf("\t--help, -h   : print help information. \n");
	return 0;
}


int get_mng_cfg( GOLBALCFG *a_pstCfg, char* a_szCfgFile)
{
    int iRet;
    char szTmpStr[40];

    iRet = tini_read (a_szCfgFile, "MIB", "Key", a_pstCfg->szMibKey, sizeof(a_pstCfg->szMibKey));
    if( iRet )  
        strcpy(  a_pstCfg->szMibKey, DEFAULT_MIB_KEY ); 

    iRet = tini_read (a_szCfgFile, "MIB", "Count", szTmpStr, sizeof(szTmpStr));
    if( iRet ) 
        a_pstCfg->iMibCounts = DEFAULT_COUNT;
    else    
        a_pstCfg->iMibCounts = atoi(szTmpStr);
    
    iRet = tini_read (a_szCfgFile, "MIB", "Size", szTmpStr, sizeof(szTmpStr));
    if( iRet )
        a_pstCfg->iMibSize = DEFAULT_SIZE;
    else
        a_pstCfg->iMibSize= atoi(szTmpStr);
    

    iRet = tini_read (a_szCfgFile, "METABASE", "Key", a_pstCfg->szMetabaseKey, sizeof(a_pstCfg->szMetabaseKey));
    if( iRet )  
        strcpy(  a_pstCfg->szMetabaseKey, DEFAULT_METABASE_KEY ); 

    iRet = tini_read (a_szCfgFile, "METABASE", "Count", szTmpStr, sizeof(szTmpStr));
    if( iRet ) 
        a_pstCfg->iMetabaseCounts = DEFAULT_COUNT;
    else    
        a_pstCfg->iMetabaseCounts = atoi(szTmpStr);
    
    iRet = tini_read (a_szCfgFile, "METABASE", "Size", szTmpStr, sizeof(szTmpStr));
    if( iRet )
        a_pstCfg->iMetabaseSize = DEFAULT_SIZE;
    else
        a_pstCfg->iMetabaseSize= atoi(szTmpStr);


    return 0 ;
}


int set_entry_value(const LPTDRMETAENTRY a_pstEntry, 
                    const char* a_szValue,
                    char    *a_pValue,
                    int      a_iSize,
                    FILE    *a_fpError)
{
    int iRet = TDR_SUCCESS;

	long lVal;
	tdr_longlong llVal;

    assert(NULL != a_pstEntry);
    assert(NULL != a_fpError);
    assert(NULL != a_pValue);

	switch(tdr_get_entry_type(a_pstEntry))
	{
	case TDR_TYPE_STRING:
		{
			TDR_STRNCPY(a_pValue, &a_szValue[0], a_iSize);
			break;
		}		
	case TDR_TYPE_CHAR:
		{
			lVal = strtol(a_szValue,NULL, 0);
			if ((-128 > lVal) || (127 < lVal) )
			{
				fprintf(a_fpError, "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
					a_pstEntry->szName,  a_szValue);
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE);
			}else
			{
				*a_pValue = (char)lVal;	
			}
			break;
		}
	case TDR_TYPE_UCHAR:
		{
			lVal = strtol(a_szValue,NULL, 0);
			if ((0 > lVal) || (0xFF < lVal) )
			{
				fprintf(a_fpError, "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
					a_pstEntry->szName,  a_szValue);
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE);
			}else
			{
				*a_pValue = (unsigned char)lVal;	
			}
			break;
		}
	case TDR_TYPE_SHORT:
		{
			lVal = strtol(a_szValue,NULL, 0);
			if((-32768 > lVal) || (0x7FFF < lVal))
			{
				fprintf(a_fpError, "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
					a_pstEntry->szName,  a_szValue);
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE);
			}else
			{
				*(short *)a_pValue = (short)lVal;	
			}
			break;
		}
	case TDR_TYPE_USHORT:
		{
			lVal = strtol(a_szValue,NULL, 0);
			if ((0 > lVal) || (0xFFFF < lVal) )
			{
				fprintf(a_fpError, "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
					a_pstEntry->szName,  a_szValue);
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE);
			}else
			{
				*(unsigned short *)a_pValue = (unsigned short)lVal;	
			}
			break;
		}
	case TDR_TYPE_INT:
	case TDR_TYPE_LONG:
		{
			lVal = strtol(a_szValue,NULL, 0);
			if (((int)0x80000000 > (int)lVal) || (0x7FFFFFFF < (int)lVal) )
			{
				fprintf(a_fpError, "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
					a_pstEntry->szName,  a_szValue);
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE);
			}else
			{
				*(int *)a_pValue = (int)lVal;	
			}
			break;
		}
	case TDR_TYPE_UINT:
	case TDR_TYPE_ULONG:
		{
			//lVal = strtol(a_szValue,NULL, 0);
			llVal = TDR_ATOLL(a_szValue);
			if (0 > llVal) 
			{
				fprintf(a_fpError, "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
					a_pstEntry->szName,  a_szValue);
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE);
			}else
			{
				*(unsigned int *)a_pValue = (unsigned int)llVal;	
			}
			break;
		}
	case TDR_TYPE_LONGLONG:
		{
			llVal = TDR_ATOLL(a_szValue);
			*(tdr_longlong *)a_pValue = llVal;	
	
			break;
		}
	case TDR_TYPE_ULONGLONG:
		{
			llVal = TDR_ATOLL(a_szValue);
			if (0 > llVal)
			{
				fprintf(a_fpError, "\nerror:\t 成员<name　= %s>的数据值<%s>与类型不匹配",
					a_pstEntry->szName,  a_szValue);
				iRet = TDR_ERRIMPLE_MAKE_ERROR(TDR_ERROR_ENTRY_INVALID_DEFAULT_VALUE);
			}else
			{
				*(tdr_ulonglong *)a_pValue = (tdr_ulonglong)llVal;	
			}
			break;
		}
	case TDR_TYPE_FLOAT:
		{
			float  fVal;

			fVal = (float)atof(a_szValue);
			*(float *)a_pValue = fVal;
			break;
		}
	case TDR_TYPE_DOUBLE:
		{
			double  dVal;

			dVal = strtod(a_szValue, NULL);
			*(double *)a_pValue = dVal;
			break;
		}
	case TDR_TYPE_IP:
		{
			iRet = tdr_ineta_to_tdrip((tdr_ip_t *)a_pValue , &a_szValue[0]);
			break;
		}
/*
	case TDR_TYPE_WCHAR:
		{
			tdr_wchar_t swTemp[8];
			int iLen = 8*sizeof(tdr_wchar_t);

			iRet = tdr_chinesembstowcs((char *)&swTemp[0], &iLen, &a_szValue[0], 3);
			if (TDR_SUCCESS == iRet)
			{
				*(tdr_wchar_t *)a_pValue = swTemp[0];
			}	
			break;
		}
		
		
	case TDR_TYPE_WSTRING:
		{
			int iWcsLen;
			int iMbsLen;

			iMbsLen = strlen(&a_szValue[0]) + 1;
			iWcsLen = iMbsLen*sizeof(tdr_wchar_t);

			iRet = tdr_chinesembstowcs(a_pValue, &iWcsLen, &a_szValue[0], iMbsLen);
			if (!TDR_ERR_IS_ERROR(iRet))
			{
				a_iSize = iWcsLen;
			}
			break;
		}
*/		
	case TDR_TYPE_DATETIME:
		{
			iRet = tdr_str_to_tdrdatetime((tdr_datetime_t *)a_pValue, &a_szValue[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nwarning:\t 成员<name = %s>日期/时间(datetime)类型的值无效,正确datetime类型其值格式应为\"YYYY-MM-DD HH:MM:SS\".",
					a_pstEntry->szName);
			}
			break;
		}
	case TDR_TYPE_DATE:
		{
			iRet = tdr_str_to_tdrdate((tdr_date_t *)a_pValue, &a_szValue[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nwarning:\t 成员<name = %s>日期(date)类型的缺省值无效,正确date类型其值格式应为\"YYYY-MM-DD\".",
					 a_pstEntry->szName);
			}
			break;
		}
	case TDR_TYPE_TIME:
		{
			iRet = tdr_str_to_tdrtime((tdr_time_t *)a_pValue, &a_szValue[0]);
			if (TDR_ERR_IS_ERROR(iRet))
			{
				fprintf(a_fpError, "\nerror:\t 成员<name = %s>时间(time)类型的缺省值无效,正确date类型其值格式应为\"HHH:MM:SS\".",
				        a_pstEntry->szName);
			}
			break;
		}
	default:			
		fprintf(a_fpError, "\nwarning:\t 成员<name = %s>的类型目前暂不支持设置值.",
			        a_pstEntry->szName );
		a_iSize = 0;
		break;
	}

	
    return iRet;
}

int parse_option(OUT GOLBALCFG *a_pstGlData, IN int a_argc, IN char *a_argv[])
{
	int opt;
	int iRet = 0;
    char* pszApp;

	assert(NULL != a_pstGlData);
	assert(0 < a_argc);
	assert(NULL != a_argv);

    pszApp = basename(a_argv[0]);

    a_pstGlData->argc = a_argc;
    a_pstGlData->argv = a_argv;

	while (1)
	{
		int option_index = 0;
		static struct option stlong_options[] = {
			{"create",      1, 0, 'c'},
			{"dump",        1, 0, 'p'},
			{"destroy",     1, 0, 'd'},			
			{"loadmeta",    1, 0, 'L'},
			{"loaddata",    1, 0, 'G'},
			{"listproc",    0, 0, 'l'},
			{"registdata",  0, 0, 'r'},
			{"modify",      0, 0, 'm'},
			{"alias",       1, 0, 'A'},
			{"domain",      1, 0, 'n'},
			{"meta",        1, 0, 'M'},
			{"lib",         1, 0, 'B'},
			{"path",        1, 0, 'P'},
			{"value",       1, 0, 'U'},
			{"size",        1, 0, 's'},
			{"data-ver",    1, 0, 'V'},
			{"periods",     1, 0, 'e'},
			{"procid",      1, 0, 'R'},
			{"config-file", 1, 0, 'C'},
			{"log-file",    1, 0, 'g'},			
			{"help",        0, 0, 'h'},
			{"version",     0, 0, 'v'},
			{0,             0, 0, 0  }
		};

		opt = getopt_long (a_argc, a_argv, "C:hvl", stlong_options, &option_index);

		if (opt == -1)
			break;
		
		if ((opt == '?') || (opt == ':') )
		{
			iRet = 1;
			break;
		}

		switch( opt )
		{
    		case 'c': /* create mib or metabase */
        		{
                    if ( !strcasecmp(optarg, "mib") )  
                        a_pstGlData->iAction = CREATE_MIB ;
                    else if ( !strcasecmp(optarg, "metabase") )  
                        a_pstGlData->iAction = CREATE_METEBASE;
                    else
                    {
                        a_pstGlData->iAction = 0;
        				iRet = -1;
        				printf("%s: 无效选项 %s\n", pszApp, optarg);
        				printf("可以执行  -h 获取更多信息\n");                        
                    }
                    break;
        		}
    		case 'p': /* dump mib or metabase */
        		{
                    if ( !strcasecmp(optarg, "mib") )  
                        a_pstGlData->iAction = DUMP_MIB ;
                    else if ( !strcasecmp(optarg, "metabase") )  
                        a_pstGlData->iAction = DUMP_METEBASE;
                    else
                    {
                        a_pstGlData->iAction = 0;
        				iRet = -1;
        				printf("%s: 无效选项 %s\n", pszApp, optarg);
        				printf("可以执行  -h 获取更多信息\n");                        
                    }
                    break;
        		}
    		case 'd': /* destroy mib or metabase */
        		{
                    if ( !strcasecmp(optarg, "mib") )  
                        a_pstGlData->iAction = DESTORY_MIB ;
                    else if ( !strcasecmp(optarg, "metabase") )  
                        a_pstGlData->iAction = DESTORY_METEBASE;
                    else
                    {
                        a_pstGlData->iAction = 0;
        				iRet = -1;
        				printf("%s: 无效选项 %s\n", pszApp, optarg);
        				printf("可以执行  -h 获取更多信息\n");                        
                    }

                    break;
        		}
    		case 'L': /* load metabase */
        		{
                    a_pstGlData->iAction = LOAD_META;
                    a_pstGlData->pszLoadfile = optarg;
                    break;
        		}
    		case 'G': /* load cfg data into mib */
        		{
                    a_pstGlData->iAction = LOAD_DATA;
                    a_pstGlData->pszLoadfile = optarg;
                    break;
        		}
    		case 'r': /* regist data into mib */
        		{
                    a_pstGlData->iAction = REGIST_DATA;
                    break;
        		}
            
    		case 'm': /* modify cfg data into mib */
        		{
                    a_pstGlData->iAction = MODIFY_DATA;
                    break;
        		}
    		case 'l': /* list process */
        		{
                    a_pstGlData->iAction = LIST_PROC;
                    break;
        		}
            
    		case 'P': /* mib data version in load cfg data atcion*/
        		{
                    a_pstGlData->pszPath = optarg;
                    break;
        		}
    		case 'U': /* mib data version in load cfg data atcion*/
        		{
                    a_pstGlData->pszValue = optarg;
                    break;
        		}            
    		case 's': /* mib data size in regist data atcion*/
        		{
                    a_pstGlData->iDataSize= atoi(optarg);
                    break;
        		}            
    		case 'V': /* mib data version in load cfg data atcion*/
        		{
                    a_pstGlData->iMibDataVersion= atoi(optarg);
                    break;
        		}
    		case 'e': /* mib data periods in regist or modify data atcion*/
        		{
                    a_pstGlData->iPeriods = atoi(optarg);
                    break;
        		}
            
    		case 'R': /* mib data procid in load cfg data atcion*/
        		{
                    a_pstGlData->iProcId= inet_addr(optarg);
                    break;
        		}
    		case 'A': /*alias of cfg data in load cfg data atcion*/
        		{
                    a_pstGlData->pszAlias = optarg;
                    break;
        		}

    		case 'n': /*domain of mib data in loadcfg or modify  atcion*/
        		{
                    a_pstGlData->pszDomain = optarg;
                    break;
        		}
    		case 'M': /*meta name of cfg data in load cfg data atcion*/
        		{
                    a_pstGlData->pszMeta= optarg;
                    break;
        		}
    		case 'B': /*meta lib of cfg data in load cfg data atcion*/
        		{
                    a_pstGlData->pszLib= optarg;
                    break;
        		}
    		case 'C': /* cfg file name */
        		{
                    if ( access(optarg, 0) )
                    {  
                        printf("can not access %s\n", optarg);
                        iRet = 1;
                        break;
                    }
                    strcpy(a_pstGlData->szConfFile, optarg);
                    break;
        		}
            
    		case 'h': /* show the help information. */
    			{
    				iRet = 1;
    				_usage(a_argv);
    				break;
    			}
    		case 'v':
    			{
    				iRet = 1;
    				printf("%s version: %03d\n" , pszApp, CUR_VER);
    				break;
    			}
    		default:
    			{
    				iRet = -1;
    				printf("%s: 无效选项 \n", pszApp);
    				printf("可以执行  -h 获取更多信息\n");
    				break;
    		    }
		}
	}/*while (1)*/

	if (0 != iRet)
	{
		return iRet;
	}
	return iRet;
}


int tmng_load_meta(GOLBALCFG* pstCfg)
{
    int         iRet;
    char        chInput=0;
    HTMBDESC    hDesc=NULL;
    LPTDRMETALIB pstLib=NULL;

    if ( !strlen(pstCfg->pszLoadfile ))
    {
        printf( "缺少参数，请指定数据文件!!\n");
        return -1;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 0);
    if (iRet )
    {
        printf( "open metabase fail\n");
        return -2;
    }

    iRet = strlen( pstCfg->pszLoadfile );
    if ( strncmp( pstCfg->pszLoadfile+iRet-4 , ".xml", 4 ) )
    {
        iRet = tdr_load_metalib(&pstLib, pstCfg->pszLoadfile);
    }
    else
    {
        iRet= tdr_create_lib_file( &pstLib, pstCfg->pszLoadfile, 1, stderr);
    }
    
    if( iRet || NULL==pstLib ) return -3;

    printf ( "lib name=%s size=%d \n", tdr_get_metalib_name(pstLib), tdr_size(pstLib));

    tdr_dump_metalib(pstLib, stdout);
    
    printf("\n\nAre you sure load this lib into metabase: Y(es)/N(o), default No!\n");
    chInput = getchar();
    if ( 'Y' != chInput && 'y' != chInput )
    {
        printf("metalib %s can't laod into metabase, now exit!\n", pstCfg->pszLoadfile);  
        return -4;
    }
    
    tmb_lock(hDesc);
    iRet = tmb_append_unlocked(hDesc, pstLib );
    if( iRet )
    {
        printf( "load into metabase error %d\n", iRet); 
        return -5;
    }
    tmb_unlock(hDesc);            
    tdr_free_lib(&pstLib);
    tmb_close(&hDesc);
    return 0;
}

int tmng_load_data(GOLBALCFG* pstCfg)
{
    int iRet;

    HTMIB       hMib=NULL;
    HTMBDESC    hDesc=NULL;
    LPTDRMETA   pstMeta=NULL;
    TDRDATA     stData;
    TMIBDATA    stMibData;
    char    chInput=0;

    if ( NULL == pstCfg->pszAlias || !strlen(pstCfg->pszAlias))
    {
        printf( "缺少参数，请使用--alias指定cfg data的名字!!\n");
        return -1;
    }

    if ( NULL == pstCfg->pszMeta || !strlen(pstCfg->pszMeta))
    {
        printf( "缺少参数，请使用--meta指定数据定义的名字!!\n");
        return -1;
    }

    if ( NULL == pstCfg->pszLib || !strlen(pstCfg->pszLib))
    {
        printf( "缺少参数，请使用--lib 指定数据定义的lib!!\n");
        return -1;
    }
    
    iRet = tmib_open(&hMib,pstCfg->szMibKey);
    if (iRet )
    {
        printf( "open mib fail\n");
        return -1;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    if (iRet )
    {
        printf( "open metabase fail\n");
        return -1;
    }

    iRet = tmb_open_metalib(hDesc, pstCfg->pszLib, 0);
    if (iRet )
    {
        printf( "cfg data=%s, lib=%s not found! \n", pstCfg->pszLoadfile, pstCfg->pszLib);
        return -1;
    }
    
    iRet = tmb_meta_by_name(hDesc, pstCfg->pszMeta, &pstMeta);
    if (NULL == pstMeta) 
    {
        printf( "data %s meta=%s define not found!\n" ,pstCfg->pszLoadfile, pstCfg->pszMeta);
        return -1;
    }

    stData.iBuff = tdr_get_meta_size(pstMeta);
    stData.pszBuff = (char*)malloc( stData.iBuff);
    
    iRet = tdr_input_file(pstMeta, &stData, pstCfg->pszLoadfile, 0, 0 );
    if (iRet) 
    {
        printf( "load data %s fail %d!\n" ,pstCfg->pszLoadfile, iRet);
        return -2;
    }

    memset( &stMibData, 0, sizeof(stMibData));
    if ( NULL == pstCfg->pszDomain || !strlen(pstCfg->pszDomain))
    {
        strcpy(stMibData.szDomain, CFG_DOMAIN); 
    }
    else
    {
        strncpy(stMibData.szDomain, pstCfg->pszDomain, sizeof(stMibData.szDomain)-1);                 
    }
    

    strcpy(stMibData.szName, pstCfg->pszAlias);
    strcpy(stMibData.szLib, pstCfg->pszLib);
    strcpy(stMibData.szMeta, pstCfg->pszMeta);
    stMibData.iVersion = pstCfg->iMibDataVersion;
    stMibData.iPeriods = pstCfg->iPeriods;
    stMibData.iProcID = pstCfg->iProcId ;
    stMibData.iSize = stData.iBuff;

    tdr_fprintf(pstMeta, stdout, &stData, 0);            

    printf("\n\nAre you sure load this data into mib: Y(es)/N(o), default No!\n");
    chInput = getchar();
    if ( 'Y' != chInput && 'y' != chInput )
    {
        printf("config data %s can't laod into mib !\n", pstCfg->pszLoadfile);  
        return -3;
    }

    iRet = tmib_register_data(hMib, &stMibData);
    if (iRet) 
    {
        printf( "regist mib %s fail %d!\n" ,pstCfg->pszLoadfile, iRet);
        return -4;
    }

    tmib_get_data(hMib, &stMibData,0);  
    memcpy( stMibData.pszData,  stData.pszBuff, stData.iBuff);
    
    free(stData.pszBuff);
    stData.pszBuff = NULL;
    tmb_close(&hDesc);
    tmib_close(&hMib);
    return 0;
}


int tmng_regist_data(GOLBALCFG* pstCfg)
{
    int iRet, iSize;

    HTMIB       hMib=NULL;
    HTMBDESC    hDesc=NULL;
    LPTDRMETA   pstMeta=NULL;
    TMIBDATA    stMibData;

    if ( NULL == pstCfg->pszAlias || !strlen(pstCfg->pszAlias))
    {
        printf( "缺少参数，请使用--alias指定cfg data的名字!!\n");
        return -1;
    }
/*
    if ( NULL == pstCfg->pszMeta || !strlen(pstCfg->pszMeta))
    {
        printf( "缺少参数，请使用--meta指定数据定义的名字!!\n");
        return -1;
    }

    if ( NULL == pstCfg->pszLib || !strlen(pstCfg->pszLib))
    {
        printf( "缺少参数，请使用--lib 指定数据定义的lib!!\n");
        return -1;
    }
    */


    if ( (NULL == pstCfg->pszMeta || !strlen(pstCfg->pszMeta)  ) && 0==pstCfg->iDataSize )
    {
        printf( "缺少参数，请使用--meta --lib指定数据定义 或者--size　指定数据空间大小!\n");
        return -1;
    }

    if ( (NULL != pstCfg->pszMeta && strlen(pstCfg->pszMeta)  ) 
        &&(NULL == pstCfg->pszLib || !strlen(pstCfg->pszLib)))
    {
        printf( "缺少参数，请使用--lib 指定数据定义的lib!!\n");
        return -1;
    }


    iRet = tmib_open(&hMib,pstCfg->szMibKey);
    if (iRet )
    {
        printf( "open mib fail\n");
        return -1;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    if (iRet )
    {
        printf( "open metabase fail\n");
        return -1;
    }

    memset( &stMibData, 0, sizeof(stMibData));
    if ( NULL!=pstCfg->pszLib && NULL!=pstCfg->pszMeta )
    {
        iRet = tmb_open_metalib(hDesc, pstCfg->pszLib, 0);
        if (iRet )
        {
            printf( "cfg data=%s, lib=%s not found! \n", pstCfg->pszLoadfile, pstCfg->pszLib);
            return -1;
        }
        
        iRet = tmb_meta_by_name(hDesc, pstCfg->pszMeta, &pstMeta);
        if (NULL == pstMeta) 
        {
            printf( "data %s meta=%s define not found!\n" ,pstCfg->pszLoadfile, pstCfg->pszMeta);
            return -1;
        }

        iSize = tdr_get_meta_size(pstMeta);
    }
    else if ( 0!=pstCfg->iDataSize )
    {
        iSize = pstCfg->iDataSize;
    }
    else
    {
        printf( "regist data 缺少参数!\n");    
        return -1;
    }

    if ( NULL == pstCfg->pszDomain || !strlen(pstCfg->pszDomain))
    {
        strcpy(stMibData.szDomain, CFG_DOMAIN); 
    }
    else
    {
        strncpy(stMibData.szDomain, pstCfg->pszDomain, sizeof(stMibData.szDomain)-1);                 
    }
    
    strcpy(stMibData.szName, pstCfg->pszAlias);
    if ( NULL != pstCfg->pszLib)  strcpy(stMibData.szLib, pstCfg->pszLib);
    if ( NULL != pstCfg->pszMeta) strcpy(stMibData.szMeta, pstCfg->pszMeta);
    stMibData.iVersion = pstCfg->iMibDataVersion;
    stMibData.iProcID = pstCfg->iProcId ;
    stMibData.iPeriods = pstCfg->iPeriods;
    stMibData.iSize = iSize;
    iRet = tmib_register_data(hMib, &stMibData);
    if (iRet) 
    {
        printf( "regist mib %s fail %d!\n" ,pstCfg->pszLoadfile, iRet);
        return -4;
    }

    tmb_close(&hDesc);
    tmib_close(&hMib);

    if ( iSize != stMibData.iSize )
    {
        printf("mib data %s already exist size=%d\n", pstCfg->pszAlias, stMibData.iSize);
        printf("argument --size specify value=%d, can't match!\n", pstCfg->iDataSize);
    }
    return 0;
}


int tmng_modify_data(GOLBALCFG* pstCfg)
{
    int iRet, iOff, iType;

    HTMIB       hMib=NULL;
    HTMBDESC    hDesc=NULL;
    LPTDRMETA   pstMeta=NULL;
    TDRDATA     stData;
    TMIBDATA    stMibData;
    LPTDRMETAENTRY pstEntry=NULL;
    char    chInput=0;
    if ( NULL == pstCfg->pszAlias || !strlen(pstCfg->pszAlias))
    {
        printf( "缺少参数，请使用--alias指定cfg data的名字!!\n");
        return -1;
    }

    if ( NULL == pstCfg->pszPath || !strlen(pstCfg->pszPath))
    {
        printf( "缺少参数，请使用--path 指定元数据路径!!\n");
        return -1;
    }

    if ( NULL == pstCfg->pszValue || !strlen(pstCfg->pszValue))
    {
        printf( "缺少参数，请使用--value 指定数据修改的值!!\n");
        return -1;
    }

    iRet = tmib_open(&hMib,pstCfg->szMibKey);
    if (iRet )
    {
        printf( "open mib fail\n");
        return -1;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    if (iRet )
    {
        printf( "open metabase fail\n");
        return -1;
    }
    
    memset( &stMibData, 0, sizeof(stMibData));  
    if ( NULL == pstCfg->pszDomain || !strlen(pstCfg->pszDomain))
    {
        strcpy(stMibData.szDomain, CFG_DOMAIN); 
    }
    else
    {
        strncpy(stMibData.szDomain, pstCfg->pszDomain, sizeof(stMibData.szDomain)-1);                 
    }
                    
    strcpy(stMibData.szName, pstCfg->pszAlias);
    stMibData.iVersion = pstCfg->iMibDataVersion;
    stMibData.iPeriods = pstCfg->iPeriods;
    stMibData.iProcID= pstCfg->iProcId;
    
    iRet = tmib_get_data(hMib, &stMibData, 0);
    if( iRet )
    {
        return -1;
    }

    iRet = tmb_open_metalib(hDesc, stMibData.szLib, stMibData.iVersion);
    if (iRet )
    {
        printf( "cfg data=%s, lib=%s not found! \n", pstCfg->pszAlias,  stMibData.szLib);
        return -1;
    }
    
    iRet = tmb_meta_by_name(hDesc, stMibData.szMeta, &pstMeta);
    if (NULL == pstMeta) 
    {
        printf( "data %s meta=%s define not found!\n" ,pstCfg->pszAlias, stMibData.szMeta);
        return -1;
    }

    iRet = tdr_entry_path_to_off( pstMeta, &pstEntry, &iOff, pstCfg->pszPath);
    if ( 0!=iRet )
    {
        printf( "data %s  meta path=%s define not found!\n" ,pstCfg->pszAlias, pstCfg->pszAlias);
        return -1;
    }

    iType = tdr_get_entry_type(pstEntry);

    if (  TDR_TYPE_UNION==iType || TDR_TYPE_STRUCT==iType )
    {
        printf( "data %s  meta path=%s 复合数据类型不支持修改!\n" ,pstCfg->pszAlias, pstCfg->pszAlias);
        return -1;
    }
   
    stData.iBuff = stMibData.iSize;
    stData.pszBuff = stMibData.pszData;
    
    tdr_fprintf(pstMeta, stdout, &stData, 0); 

    printf("\n\nAre you sure modyfy %s with values %s: Y(es)/N(o), default No!\n", pstCfg->pszPath, pstCfg->pszValue);
    chInput = getchar();
    if ( 'Y' != chInput && 'y' != chInput )
    {
        printf("config data %s can't modify, now exit!\n", pstCfg->pszPath);  
        iRet = tmb_close(&hDesc);
        iRet = tmib_close(&hMib);
        return -3;
    }

    iRet = set_entry_value( pstEntry, 
                            pstCfg->pszValue, 
                            stMibData.pszData+iOff, 
                            tdr_get_entry_unitsize(pstEntry), 
                            stdout);
    tmb_close(&hDesc);
    tmib_close(&hMib);
    return iRet;
}            

int tmng_dump_mib(GOLBALCFG* pstCfg)
{
    int         iRet=0;
    HTMIB       hMib=NULL;
    HTMBDESC    hDesc=NULL;

    TMIBDATA    stMibData;
    
    iRet = tmib_open(&hMib,pstCfg->szMibKey);
    if (iRet )  
    {
        printf( "open mib fail\n");
        return -1;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    if (iRet )
    {
        printf( "open metabase fail\n");
        return -1;
    }

    
    if( NULL!=pstCfg->pszAlias && strlen(pstCfg->pszAlias) )
    {
        memset( &stMibData, 0, sizeof(stMibData));  
        if ( NULL == pstCfg->pszDomain || !strlen(pstCfg->pszDomain))
        {
            strcpy(stMibData.szDomain, CFG_DOMAIN); 
        }
        else
        {
            strncpy(stMibData.szDomain, pstCfg->pszDomain, sizeof(stMibData.szDomain)-1);                 
        }
                        
        strcpy(stMibData.szName, pstCfg->pszAlias);
        stMibData.iVersion = pstCfg->iMibDataVersion;
        stMibData.iProcID= pstCfg->iProcId;
        
        iRet = tmib_get_data(hMib, &stMibData, 0);
        if( !iRet )
        {
            printf( " Domain=\"%s\" Name=\"%s\" Lib=\"%s\", Meta=\"%s\", ProcID=%s size=%d\n",
                        stMibData.szDomain, stMibData.szName,  stMibData.szLib, stMibData.szMeta,
                        inet_ntoa(*((struct in_addr*)&(stMibData.iProcID))),  stMibData.iSize );
        
            tmib_dump(hDesc, &stMibData, stdout);     
        }        
    }
    else
    {   /* dump all */
        tmib_dump_head(hMib, stdout);
        tmib_dump_all(hMib, pstCfg->szMetabaseKey, stdout);
        printf("\n");
    }
    
    tmb_close(&hDesc);    
    tmib_close(&hMib);
    
    return iRet;
}

int tmng_list_proc(GOLBALCFG* pstCfg)
{
    int i;
    int         iRet=0;
    char        szStart[30], szStop[30];
    HTMIB       hMib=NULL;
    HTMBDESC    hDesc=NULL;

    TMIBDATA stData;
    TMIBINFO *pstInfo;
    TMIBENTRY *pstEntry;
    LPMNGPROCATTR   pstProcAttr;
    
    iRet = tmib_open(&hMib,pstCfg->szMibKey);
    if (iRet )  
    {
        printf( "open mib fail\n");
        return -1;
    }

    iRet = tmb_open(&hDesc, pstCfg->szMetabaseKey, 1);
    if (iRet )
    {
        printf( "open metabase fail\n");
        return -1;
    }

    pstInfo = hMib->pstInfo;
    printf( "-------------------------------------------------------------------------------------\n");
    printf( "| ProcessID     | ProcName    | pid   | Status | User |     start    |     stop     |\n");
    printf( "-------------------------------------------------------------------------------------\n");
    
    for(i=0; i<pstInfo->iCurDatas; i++)
    {
        pstEntry    =    pstInfo->entries + i;

        if ( pstEntry->bDelete ) continue;
        if ( pstEntry->bExtern ) continue;
        if ( strcmp(pstEntry->szDomain, PROC_LIST_DOMAIN) ) continue;
        if ( strcmp(pstEntry->szMeta, PROC_ATTR ) )     continue;
        pstProcAttr = (LPMNGPROCATTR)(((int)pstInfo) + pstEntry->iOff + MIB_HEAD_SIZE);
        
        FormatTime(pstProcAttr->dwStart, szStart, sizeof(szStart)); 
        FormatTime(pstProcAttr->dwStop, szStop, sizeof(szStop));
        printf("  %-15s %-12s\t%-5d   %-6s  %-6s %-15s %-15s\n", 
            pstProcAttr->szProcID, pstProcAttr->szProcName, pstProcAttr->iPid, 
            pstProcAttr->iStatus?"Active":" ",  pstProcAttr->iStatus?pstProcAttr->szUser:" ",
            pstProcAttr->iStatus?szStart:" ",   pstProcAttr->iStatus?" ":(pstProcAttr->dwStop?szStop:" "));
    }
    printf( "\n\n");

    tmb_close(&hDesc);    
    tmib_close(&hMib);
    
    return iRet;
}

int main(int argc,char ** argv)
{
    int         iRet, i;

    char        szCmdLine[400], szTmp[80];
    HTMIB       hMib=NULL;
    HTMBDESC    hDesc=NULL;
    char*       pszApp;
    GOLBALCFG   stCfg;

    szCmdLine[0]=0;
    for(i=0; i<argc; i++)
    {
        sprintf(szTmp, "%s ", argv[i]);
        strcat(szCmdLine, szTmp );        
    }

    pszApp = basename(argv[0]);
    memset( &stCfg, 0, sizeof(GOLBALCFG));
	iRet = parse_option(&stCfg, argc, argv);	
	if ( iRet )
	{
		return iRet;
	}
   
    if ( !strlen(stCfg.szConfFile) )
        sprintf( stCfg.szConfFile, "./%s.conf", pszApp);

    iRet = get_mng_cfg( &stCfg, stCfg.szConfFile);
    if ( iRet )
    {
        printf("get config error\n");
        return -1;
    }
/*
	if ( 0 != log4c_init() )
	{
		g_ptLog = NULL;
	}else
	{
		g_ptLog = log4c_category_get ( "tmng" ) ;
	}
*/

//	log4c_category_debug ( g_ptLog, "%s operation:: %s\n",getenv("USER"), szCmdLine) ;

    switch ( stCfg.iAction )
    {
        case CREATE_MIB:
            {
                iRet = tmib_create(&hMib, stCfg.szMibKey, stCfg.iMibCounts, stCfg.iMibSize);
                if( iRet )
                {
                    printf("Create mib fail=%d!\n", iRet);
                    return -1;
                }
                else
                    printf("Create mib sucess!\n");
                break;
            }
        case CREATE_METEBASE:
            {
                iRet = tmb_create(&hDesc, stCfg.szMetabaseKey, stCfg.iMetabaseCounts, stCfg.iMetabaseSize);
                if( iRet )
                {
                    printf("Create metabase fail=%d!\n", iRet);
                    return -1;
                }
                else
                    printf("Create metabase sucess!\n");
                break;
            } 
        case DESTORY_MIB:
            {    
                iRet = tmib_destroy(stCfg.szMibKey);
                if( iRet )
                    printf("Destory mib fail=%d!\n", iRet);
                 
                else
                    printf("Destory mib sucess!\n");
                break;
            }   
        case DESTORY_METEBASE:
            {
                iRet = tmb_destroy(stCfg.szMetabaseKey);
                if( iRet )
                    printf("Destory metabase fail=%d!\n", iRet);
                 
                else
                    printf("Destory metabase sucess!\n");
                break;
            }
        case DUMP_MIB:
            {
                iRet =  tmng_dump_mib(&stCfg);
                break;
            }
        case DUMP_METEBASE:
            {
                iRet = tmb_open(&hDesc, stCfg.szMetabaseKey, 1);
                if (iRet )
                {
                    printf( "open metabase fail\n");
                    return -1;
                }

                tmb_dump(hDesc, 1, stdout);
                printf("\n");
                iRet = tmb_close(&hDesc);
                break;
    	    }            
        case LOAD_META:
            {
                iRet =  tmng_load_meta(&stCfg);
                if( iRet )
                {
                    printf( "load %s into metabase error %d\n", stCfg.pszLoadfile, iRet); 
                }
                else
                {
                    printf("load metalib %s  sucess!\n", stCfg.pszLoadfile);
                }
                break;
            }
        case LOAD_DATA:
            {
                iRet =  tmng_load_data(&stCfg);
                if( iRet )
                {
                    printf( "load %s into mib error %d\n", stCfg.pszLoadfile, iRet); 
                }
                else
                {
                    printf("load mib data %s  sucess!\n", stCfg.pszLoadfile);
                }
                break;
            }

        case REGIST_DATA:
            {
                iRet =  tmng_regist_data(&stCfg);
                if( iRet )
                {
                    printf( "regist %s in mib error %d\n", stCfg.pszAlias, iRet); 
                }
                else
                {
                    printf("regist %s  in mib sucess!\n", stCfg.pszAlias);
                }
                break;
            }

            
        case MODIFY_DATA:
            {
                iRet =  tmng_modify_data(&stCfg);
                if ( !iRet ) 
                {
                    printf("config data %s t modify  success!\n", stCfg.pszPath);  
                }
                else    
                {
                    printf("config data %s can't modify!\n", stCfg.pszPath);  
                }
                break;
            }
        case LIST_PROC:
            {
                iRet =  tmng_list_proc(&stCfg);
                if ( iRet ) 
                {
                    printf("list process err=%d!\n", iRet);  
                }
                break;
            }        
        case 0:
        default:
			{
				iRet = -1;
				printf("可以执行  -h 获取更多信息\n");
				break;
    		}            
            
    }
/*	
	if ( NULL != g_ptLog )
	{
		log4c_fini() ;
	}
  */  
    return 0;
}
