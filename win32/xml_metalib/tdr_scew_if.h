/* scew parser wrapper functions, simply called scew iterface */
#ifndef TDR_SCEW_IF_H
#define TDR_SCEW_IF_H

#include <scew/scew.h>

#ifdef __cplusplus
extern "C"
{
#endif


scew_parser *tdr_parser_create(void);
scew_tree * tdr_parser_tree(scew_parser *parser);
unsigned int tdr_tree_save_file(scew_tree *tree, const char *file);

#define parser_free         scew_parser_free
#define parser_load_file    scew_parser_load_file
#define parser_load_fp      scew_parser_load_file_fp
#define parser_load_buffer  scew_parser_load_buffer

#define parser_error_code	scew_error_code
#define parser_error_string scew_error_string
#define parser_expact_error_code	scew_error_expat_code
#define parser_expact_error_string	scew_error_expat_string

#ifdef _cplusplus
}
#endif

#endif /* TDR_SCEW_IF_H */

