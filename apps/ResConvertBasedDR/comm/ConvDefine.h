#ifndef CONVDEFINE_H
#define CONVDEFINE_H

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

#define MAX_LENGTH_LINE	1024
#define MAX_PARAM_NUM	128


/*xml tag*/
#define CONV_TREE_XML_TAG_ROOT "ConvList"
#define CONV_TREE_XML_TAG_ISNEWCONFIGFORMAT	"IsNewConfigFormat"
#define CONV_TREE_XML_TAG_METALIBFILE "MetalibFile"
#define CONV_TREE_XML_TAG_FILENAME "File"
#define CONV_TREE_XML_TAG_EntryMapPATH "EntryMapFilesPath"
#define CONV_TREE_XML_TAG_PATH "Path"
#define CONV_TREE_XML_TAG_BINFILEPATH "BinFilesPath"
#define CONV_TREE_XML_TAG_EXCELFILEPATH "ExcelFilesPath"
#define CONV_TREE_XML_TAG_RESSTYLELIST "ResStyleList"
#define CONV_TREE_XML_TAG_RESSTYLE "ResStyle"
#define CONV_TREE_XML_TAG_RESNAME "Name"
#define CONV_TREE_XML_TAG_RESID "ID"
#define CONV_TREE_XML_TAG_CONVTREE "ConvTree"
#define CONV_TREE_XML_TAG_COMMNODE "CommNode"
#define CONV_TREE_XML_TAG_RESNODE "ResNode"
#define CONV_TREE_XML_TAG_META "Meta"
#define CONV_TREE_XML_TAG_BINFILE "BinFile"
#define CONV_TREE_XML_TAG_ENTRYMAPFILE "EntryMapFile"
#define CONV_TREE_XML_TAG_EntryMapVERSION "EntryMapVersion"
#define CONV_TREE_XML_TAG_SORTMETHOD "Sort"
#define CONV_TREE_XML_TAG_BINSTYLES "BinStyles"
#define CONV_TREE_XML_TAG_EXCELFILE "ExcelFile"
#define CONV_TREE_XML_TAG_EXCLUDESHEET "ExcludeSheet"
#define CONV_TREE_XML_TAG_KEYWORDFILE "KeywordFile"
#define CONV_TREE_XML_TAG_ISMUTILTABLES "IsMutiTables"
#define CONV_TREE_XML_TAG_TRUE "true"
#define CONV_TREE_XML_TAG_FALSE "false"
#define CONV_TREE_XML_TAG_SUBTABLENAME "SubTableName"
#define CONV_TREE_XML_TAG_SUBTABLESIZE "SubTableSize"
#define CONV_TREE_XML_TAG_SUBTABLEIDCOL "SubTableIDColumn"
#define CONV_TREE_XML_TAG_SUBTABLEIDNAME "SubTableIDName"
#define CONV_TREE_XML_TAG_RECORDSETNAME "RecordSetName"
#define CONV_TREE_XML_TAG_RECORDCOUNTNAME "RecordCountName"

#define  CONV_TREE_XML_TAG_MAPBYENTRYNAME "MapByEntryName"
#define CONV_TREE_XML_TAG_INCLUDESHEET "IncludeSheet"

#define INI_FILE_VAR_FLAG	"$"	
#define INI_FILE_EXCLUDE_SHEET	"$ExcludeSheetName"
#define INI_FILE_KEYWORDS_FILE	"$Keywords"
#define INI_FILE_META_NAME	"$RecordName"
#define INI_FILE_IS_MUTILTABLE	"$IsMultiTable"
#define INI_FILE_SUBTABLESIZE	"$SubTableSize"
#define INI_FILE_SUBTABLENAME	"$SubTableName"
#define INI_FILE_SUBTABLE_IDNAME	"$SubTableIDName"
#define INI_FILE_SUBTABLE_IDCOL	"$SubTableIDColumn"
#define INI_FILE_RECORDSSETNAME "$RecordSetName"
#define INI_FILE_RECORDCOUNTNAME "$RecordCountName"

#define INI_GET_EXCEL_VALUE_FLAG_NUMBER	'@'
#define INI_GET_EXCEL_VALUE_FLAG_PERSENT '%'
#define INI_GET_EXCEL_VALUE_FLAG_ALLINFO '$'
#define INI_GET_EXCEL_VALUE_FLAG_INTELLIGENT '#'

#define EXCEL_ROW_OF_ENTRY_TITLE	1

#define MAX_CONVERT_COUNT_ONE_ENTRY	10000	/*对于可变长度的数组，最多转换元素数*/

#endif