#include "pal/pal.h"
#include "comm/tparser.h"


#ifdef WIN32
#pragma warning(disable:4996)
#endif /* WIN32 */

const char* TPARSER_ENCODING_GBK	= "GBK";
const char* TPARSER_ENCODING_GB		= "gb2312";

#define TPARSER_ENCODING			TPARSER_ENCODING_GBK

void tree_set_encoding(scew_tree* pstTree, const char* pszEncoding)
{
	scew_tree_set_xml_encoding(pstTree, pszEncoding);
}

void tree_set_default_encoding(scew_tree* pstTree)
{
	tree_set_encoding(pstTree, TPARSER_ENCODING);
}

void tree_set_standalone(scew_tree* pstTree, int standalone)
{
	scew_tree_set_xml_standalone(pstTree, standalone);
}

unsigned int tree_save_file(scew_tree* pstTree, const char* pszFile)
{
	tree_set_default_encoding(pstTree);
	tree_set_standalone(pstTree, 1);

	return scew_writer_tree_file(pstTree, pszFile);
}

static int unknown_encoding_handler(void *data, const XML_Char *encoding, XML_Encoding *info)
{
	int i;

	if( 0==stricmp(encoding, TPARSER_ENCODING_GBK)  ||
	    0==stricmp(encoding, TPARSER_ENCODING_GB) )
	{
		for (i = 0; i < 256; ++i)
			info->map[i] = i;

		info->data = NULL;
		info->convert = NULL;
		info->release = NULL;

		return XML_STATUS_OK;
	}

	return XML_STATUS_ERROR;
}

void tparser_convert_string(unsigned char* pszStr)
{
	unsigned char* pszFrom;
	unsigned char* pszTo;

	pszFrom	=	pszStr;
	pszTo	=	pszStr;
	
	if( !pszStr )
		return;
	
	while( pszFrom[0] )
	{
		if( pszFrom[0] & 0x80 )
		{
			assert( pszFrom[1] & 0x80 );
			
			pszTo[0]	=	((pszFrom[0] & 0x3)<<6) | (pszFrom[1] & 0x3f);
			
			pszTo++;
			pszFrom		+=	2;
		}
		else
		{
			pszTo[0]	=	pszFrom[0];
			pszTo++;
			pszFrom++;
		}
	}
	
	pszTo[0]	=	'\0';
}

void tparser_convert_element(scew_element* pstElement)
{
	scew_element* pstNext;
	scew_attribute* pstAttr;
		
	pstAttr	=	scew_attribute_next(pstElement, NULL);
	
	tparser_convert_string((unsigned char*) scew_element_name(pstElement));
	tparser_convert_string((unsigned char*) scew_element_contents(pstElement));
	
	while(pstAttr)
	{
		tparser_convert_string((unsigned char*) scew_attribute_name(pstAttr));
		tparser_convert_string((unsigned char*) scew_attribute_value(pstAttr));
		
		pstAttr	=	scew_attribute_next(pstElement, pstAttr);
	}
	
	pstNext =	scew_element_next(pstElement, NULL);
	while( pstNext )
	{
		tparser_convert_element(pstNext);
		
		pstNext	=	scew_element_next(pstElement, pstNext);
	}
}

int tparser_set_encoding(scew_parser* pstParser, const char* pszEncoding)
{
	XML_Parser xmlparser;
	enum XML_Status ret;

	xmlparser	=	scew_parser_expat(pstParser);

	XML_SetUnknownEncodingHandler(xmlparser, unknown_encoding_handler, NULL);

	ret	=	XML_SetEncoding(xmlparser, pszEncoding);

	if( XML_STATUS_OK==ret )
		return 0;
	else
		return -1;
}

int tparser_set_default_encoding(scew_parser* pstParser)
{
	return tparser_set_encoding(pstParser, TPARSER_ENCODING);
}

scew_parser* tparser_create(void)
{
	scew_parser* pstParser;

	pstParser	=	scew_parser_create();

	if( NULL==pstParser )
		return NULL;

	scew_parser_ignore_whitespaces(pstParser, 1);

	tparser_set_default_encoding(pstParser);

	return pstParser;
}

scew_tree* tparser_tree(scew_parser* pstParser)
{
	scew_tree* pstTree;
	scew_element* pstRoot;

	pstTree	=	scew_parser_tree(pstParser);

	if( !pstTree )
		return NULL;

	pstRoot	=	scew_tree_root(pstTree);

	if( pstRoot )
		tparser_convert_element( pstRoot );
	
	return pstTree;
}


