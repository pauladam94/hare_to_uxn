#include "../lexer/lexer.h"
#include <stdint.h>

typedef enum {
	NONE,
	U8_T,
	U16_T,
	VOID_T,
} ProgramType;

typedef enum {
	LET_E,

	ADD_E,
	SUB_E,
	MULT_E,
	DIV_E,
	NOT_EQUAL_E,
	EQUAL_EQUAL_E,
	GREATER_THAN_EQUAL_E,
	GREATER_THAN_E,
	LESS_THAN_EQUAL_E,
	LESS_THAN_E,

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
	IF_ELSE_E,
} ExpressionType;

typedef struct Expression {
	union {
		struct { // let var (: type) = e
			char *var;
			struct Expression *e;
			ProgramType type;
		} let;
		struct { // lhs 'operator' rhs
			struct Expression *lhs;
			struct Expression *rhs;
		} binary;
		struct { // list[0] ; ... ; list[lenght] ;
			struct Expression *list;
			uint16_t len;
		} sequence;
		struct { // var = e
			char *var;
			struct Expression *e;
		} assign;
		struct { // *var = e2, *number = e2
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
			uint16_t value;
			bool is_written_in_hexa;
		} number;
		struct { // return e
			struct Expression *e;
		} ret;
		struct { // name ( [arg ,]* arg )
			char *name;
			uint8_t len;
			struct Expression *args;
		} function_call;
		struct {
			char c;
		} char_literal;
		struct {
			struct Expression *cond;
			struct Expression *if_body;
			struct Expression *else_body;
		} if_else;
	};
	// The _E means that is used for expression
	// it is used to sisambiguates with TokenType enum
	// uint8_t size; TODO
	// uint16_t line;
	// uint16_t column;
	ExpressionType tag;
} Expression;

typedef struct {
	char *name;
	ProgramType type;
} Arg;

typedef struct {
	uint8_t len;
	Arg *args;
} Args;

typedef struct {
	Expression *expr;
	char *name;

	Args args;
	ProgramType type;
} Function;

typedef struct {
	Function *functions;
	uint8_t len;
	uint8_t cap;
} Ast;

typedef struct {
	FILE *error; // stream to output errors
	Tokens *tokens;
	uint32_t index; // position in the tokens

	bool abort; // have to stop the parsing or error
} ParseState;

// Get the current_token given the parsing state
Token current_token(ParseState *state);

/// Deletes completely recursively what expression points to
/// It does not free the expression it self
void expression_delete(Expression *expr, bool delete_itself);

// Free a complete AST structure
void ast_delete(Ast *ast);

Ast *ast_new(void);

// function should not be NULL
void ast_append(Ast *ast, Function function);

TokenType binary_tag_to_token_type(ExpressionType type);

ExpressionType token_type_to_binary_tag(TokenType type);

int precedence(ExpressionType type);

void fprintf_program_type(FILE *file, ProgramType *program_type);

void fprintf_expression(FILE *file, Expression *expr);

void fprintf_function(FILE *file, Function *function);

// Input :
// - file name to write the tokens
// - Ast that is goinf to be written in the file
void fprintf_ast(FILE *file, Ast *ast);

void fprintf_line_column(ParseState *state);

void fprintf_current_token(ParseState *state);
