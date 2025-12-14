/* scew parser wrapper functions, simply called scew iterface */
#ifndef TDR_SCEW_IF_H
#define TDR_SCEW_IF_H

#include <scew/scew.h>



void parser_convert_element(scew_element *element);
int parser_set_default_encoding(scew_parser *parser);

scew_parser *parser_create(void);
#define parser_free         scew_parser_free
#define parser_load_file    scew_parser_load_file
#define parser_load_fp      scew_parser_load_file_fp
#define parser_load_buffer  scew_parser_load_buffer

#define parser_error_code	scew_error_code
#define parser_error_string scew_error_string
#define parser_expact_error_code	scew_error_expat_code
#define parser_expact_error_string	scew_error_expat_string

scew_tree * parser_tree(scew_parser *parser);
unsigned int tree_save_file(scew_tree *tree, const char *file);

int CreateXmlTreeByFileName(scew_tree *& a_ppstTree, CString & szFilename, CString & szError);

#endif /* TDR_SCEW_IF_H */

