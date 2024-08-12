#include <stdint.h>
#include "parser_utils.h"

// Input : list of tokens
// Ouput : Ast constructed from those tokens or NULL if there is a parsing error
Ast *parse(FILE *error, Tokens *tokens);

void ast_delete(Ast *ast);

void fprintf_expression(FILE *file, Expression *expr);

// Input :
// - file : file stream to write the tokens
// - ast : the AST structure to write in the file steam
void fprintf_ast(FILE *file, Ast *ast);
