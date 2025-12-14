#include "tdr/tdr_os.h"
#include <assert.h>
#include "tdr_scew_if.h"

#ifdef WIN32
#pragma warning(disable:4996)
#endif

static void parser_convert_element(scew_element *element);
static int parser_set_default_encoding(scew_parser *parser);


static int unknown_encoding_handler(void *data, const XML_Char *encoding, 
                                   XML_Encoding *info)
{
    int i;



    if (0==tdr_stricmp(encoding, "GBK") ||
        0==tdr_stricmp(encoding, "GB2312"))
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

static void parser_convert_string(unsigned char *str)
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

static void parser_convert_element(scew_element *element)
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

static int parser_set_encoding(scew_parser *parser, const char *encoding)
{
	XML_Parser xmlparser;
	enum XML_Status ret;

	xmlparser = scew_parser_expat(parser);

	XML_SetUnknownEncodingHandler(xmlparser, unknown_encoding_handler, NULL);

	ret = XML_SetEncoding(xmlparser, encoding);

	return (ret == XML_STATUS_OK) ? 0 : -1;
}

static int parser_set_default_encoding(scew_parser *parser)
{
	return parser_set_encoding(parser, "GBK");
}

static void tree_set_encoding(scew_tree *tree, const char *encoding)
{
	scew_tree_set_xml_encoding(tree, encoding);
}

static void tree_set_default_encoding(scew_tree *tree)
{
	tree_set_encoding(tree, "GBK");
}

static void tree_set_standalone(scew_tree *tree, int standalone)
{
	scew_tree_set_xml_standalone(tree, standalone);
}

scew_tree* tdr_parser_tree(scew_parser *parser)
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

scew_parser* tdr_parser_create(void)
{
	scew_parser *parser;

	parser = scew_parser_create();

	if (!parser)
		return NULL;

	scew_parser_ignore_whitespaces(parser, 1);

	parser_set_default_encoding(parser);

	return parser;
}

unsigned int tdr_tree_save_file(scew_tree *tree, const char *file)
{
	tree_set_default_encoding(tree);
	tree_set_standalone(tree, 1);

	return scew_writer_tree_file(tree, file);
}
