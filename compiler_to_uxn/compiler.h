#include "compiler_utils.h"

void uxn_program_delete(Program *uxn_program);

// If an error occured compiling the 'ast' parameter this function :
// - returns a NULL pointer
// - write as much error information in the stream 'error'
Program *compile_to_uxn(FILE* error, Ast *ast);
