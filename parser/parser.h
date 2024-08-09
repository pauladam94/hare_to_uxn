#include "../lexer/lexer.h"
#include <stdint.h>

typedef enum {
	U8_T,
	U16_T,
	VOID_T,
} ProgramType;

typedef struct Expression {
	union {
		struct { // let var = e
			char *var;
			struct Expression *e;
		} let;
		struct { // lhs + rhs
			struct Expression *lhs;
			struct Expression *rhs;
		} add;
		struct { // lhs - rhs
			struct Expression *lhs;
			struct Expression *rhs;
		} sub;
		struct { // list[0] ; ... ; list[lenght] ; last_expression
			struct Expression *list;
			uint16_t length;
		} sequence;
		struct { // var = e
			char *var;
			struct Expression *e;
		} assign;
		struct { // *e1 = e2
			struct Expression *e1;
			struct Expression *e2;
		} deref_assign;
		struct { // *e
			struct Expression *e;
		} deref;
		struct { // name
			char *name;
		} variable;
		struct { // value
			uint32_t value;
			bool is_written_in_hexa;
		} number;
		struct { // return e
			struct Expression *e;
		} ret;
		struct { // name()
			char *name;
			// TODO args
			// struct Expression *args;
		} function_call;
		struct {
			char c;
		} char_literal;
	};
	// The _E means that is used for expression
	// it is used to sisambiguates with TokenType enum
	// uint8_t size; TODO
	// uint16_t line;
	// uint16_t column;
	enum {
		LET_E,
		ADD_E,
		SUB_E,
		SEQUENCE_E,
		ASSIGN_E,
		DEREF_ASSIGN_E,
		DEREF_E,
		VARIABLE_E,
		NUMBER_E,
		RETURN_E,
		FUNCTION_CALL_E,
		CHAR_LITERAL_E,
		STRING_LITERAL_E,
	} tag;
} Expression;

typedef struct {
	Expression *expr;
	char *name;
	ProgramType type;
} Function;

typedef struct {
	Function *functions;
	uint8_t capacity;
	uint8_t length;
} Ast;

// Input : list of tokens
// Ouput : Ast constructed from those tokens or NULL if there is a parsing error
Ast *parse(FILE* error, Tokens *tokens);

void ast_delete(Ast *ast);

// Input :
// - file : file stream to write the tokens
// - ast : the AST structure to write in the file steam
void fprintf_ast(FILE *file, Ast *ast);
