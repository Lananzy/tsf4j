#include "stdafx.h"
#include <assert.h>
#include "scew_if.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

static int unknown_encoding_handler(void *data, const XML_Char *encoding, 
                                   XML_Encoding *info)
{
    int i;

    if (0==_stricmp(encoding, "GBK") ||
        0==_stricmp(encoding, "GB2312"))
    {
    	for (i = 0; i < 256; i++)
		{
			info->map[i] = i;
		}

		info->data = NULL;
		info->convert = NULL;
		info->release = NULL;

		return XML_STATUS_OK;
    }

	return XML_STATUS_ERROR;
}

void parser_convert_string(unsigned char *str)
{
	unsigned char *from = str;
	unsigned char *to = str;

	if (!str) 
		return;

	while (from[0])
	{
		if (from[0] & 0x80)
		{
			assert(from[1] & 0x80);

			to[0] = ((from[0] & 0x3)<<6) | (from[1] & 0x3f);
			to++;
			from += 2;
		}
		else
		{
			to[0] = from[0];
			to++;
			from++;
		}
	}

	to[0] = '\0';
}

void parser_convert_element(scew_element *element)
{
	scew_element *next;
	scew_attribute *attr = NULL;

	parser_convert_string((unsigned char *) scew_element_name(element));
	parser_convert_string((unsigned char *) scew_element_contents(element));

	attr = scew_attribute_next(element, NULL);
	while (attr)
	{
		parser_convert_string((unsigned char*) scew_attribute_name(attr));
		parser_convert_string((unsigned char*) scew_attribute_value(attr));
		attr = scew_attribute_next(element, attr);
	}

	next = scew_element_next(element, NULL);
	while (next)
	{
		parser_convert_element(next);
		next = scew_element_next(element, next);
	}
}

int parser_set_encoding(scew_parser *parser, const char *encoding)
{
	XML_Parser xmlparser;
	enum XML_Status ret;

	xmlparser = scew_parser_expat(parser);

	XML_SetUnknownEncodingHandler(xmlparser, unknown_encoding_handler, NULL);

	ret = XML_SetEncoding(xmlparser, encoding);

	return (ret == XML_STATUS_OK) ? 0 : -1;
}

int parser_set_default_encoding(scew_parser *parser)
{
	return parser_set_encoding(parser, "GBK");
}

void tree_set_encoding(scew_tree *tree, const char *encoding)
{
	scew_tree_set_xml_encoding(tree, encoding);
}

void tree_set_default_encoding(scew_tree *tree)
{
	tree_set_encoding(tree, "GBK");
}

void tree_set_standalone(scew_tree *tree, int standalone)
{
	scew_tree_set_xml_standalone(tree, standalone);
}

scew_tree* parser_tree(scew_parser *parser)
{
	scew_tree *tree;
	scew_element *root;

	tree = scew_parser_tree(parser);

	if (!tree)
		return NULL;

	root = scew_tree_root(tree);

	if (root)
		parser_convert_element(root);

	return tree;
}

scew_parser* parser_create(void)
{
	scew_parser *parser;

	parser = scew_parser_create();

	if (!parser)
		return NULL;

	scew_parser_ignore_whitespaces(parser, 1);

	parser_set_default_encoding(parser);

	return parser;
}

unsigned int tree_save_file(scew_tree *tree, const char *file)
{
	tree_set_default_encoding(tree);
	tree_set_standalone(tree, 1);

	return scew_writer_tree_file(tree, file);
}

int CreateXmlTreeByFileName(scew_tree *& a_ppstTree, CString & szFilename, CString & szError)
{
	scew_parser* pstParser = NULL;
	scew_tree *pstTempTree = NULL;
	int iRet = 0;

	a_ppstTree = NULL;

	pstParser =	parser_create();
	if( NULL==pstParser )
	{
		szError.AppendFormat(_T("\nerror: \t创建XML解析器失败, error<%d> for %s\r\n"), parser_error_code(), parser_error_string(parser_error_code()));
		return -1;
	}

	if (!parser_load_file(pstParser, szFilename) )
	{
		const char *pszExpactError = parser_expact_error_string(parser_expact_error_code(pstParser));
		int iLine = scew_error_expat_line(pstParser);
		int iColumn = scew_error_expat_column(pstParser);
		szError.AppendFormat(_T("\nerror: \t将XML文件<%s>的信息加载到解析器失败: %s, 位置<line:%d, column:%d>\r\n"), szFilename, 
			pszExpactError, iLine, iColumn);
		iRet = -2;
	} 


	//分析XML元素树
	if (0 <= iRet)
	{
		pstTempTree = parser_tree(pstParser);
		if (NULL == pstTempTree)
		{
			szError.AppendFormat(_T("\nerror: \t解析器解析XML失败: %s\r\n"), 
				parser_expact_error_string(parser_expact_error_code(pstParser)));

			iRet = -3;
		}else
		{
			a_ppstTree = pstTempTree;
		} 
	}


	//释放解析器的资源
	parser_free( pstParser );

	return iRet;
}

