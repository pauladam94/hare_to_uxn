#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	FILE *error; // stream to output errors

	Tokens *tokens; // list of tokens
	uint32_t index; // where we are in the tokens

	bool abort; // have to stop the parsing
} ParseState;	    // parse_state->worked

Token current_token(ParseState *state) {
	return state->tokens->tokens[state->index];
}

///// ----- EXPRESSION FUNCTIONS ----- /////

/// Deletes completely recursively what expression points to
/// It does not free the expression it self
void expression_delete(Expression *expr, bool delete_itself) {
	switch (expr->tag) {
	case LET_E:
		free(expr->let.var);
		expression_delete(expr->let.e, true);
		break;
	case ADD_E:
		expression_delete(expr->add.lhs, true);
		expression_delete(expr->add.rhs, true);
		break;
	case SUB_E:
		expression_delete(expr->sub.lhs, true);
		expression_delete(expr->sub.rhs, true);
		break;
	case VARIABLE_E:
		free(expr->variable.name);
		break;
	case NUMBER_E:
		break;
	case SEQUENCE_E:
		for (int i = 0; i < expr->sequence.length; i++) {
			expression_delete(&expr->sequence.list[i], false);
		}
		free(expr->sequence.list);
		break;
	case ASSIGN_E:
		free(expr->assign.var);
		expression_delete(expr->assign.e, true);
		break;
	case DEREF_ASSIGN_E:
		expression_delete(expr->deref_assign.e1, true);
		expression_delete(expr->deref_assign.e2, true);
		break;
	case DEREF_E:
		expression_delete(expr->deref.e, true);
		break;
	case RETURN_E:
		expression_delete(expr->ret.e, true);
		break;
	case FUNCTION_CALL_E:
		free(expr->function_call.name);
		break;
	case CHAR_LITERAL_E:
		break;
	case STRING_LITERAL_E:
		break;
	}
	if (delete_itself) {
		free(expr);
	}
}

///// ----- AST FUNCTIONS ----- /////

// Free a complete AST structure
void ast_delete(Ast *ast) {
	for (int i = 0; i < ast->length; i++) {
		free(ast->functions[i].name);
		expression_delete(ast->functions[i].expr, true);
		// free(ast->functions[i].expr);
	}
	free(ast->functions);
	free(ast);
}

Ast *ast_new(void) {
	Ast *ast = malloc(sizeof(*ast));
	ast->length = 0;
	ast->capacity = 0;
	ast->functions = NULL;
	return ast;
}

// function should not be NULL
void ast_append_function(Ast *ast, Function function) {
	ast->length += 1;
	if (ast->length > ast->capacity) {
		if (ast->capacity == 0) {
			ast->capacity = 1;
		} else {
			ast->capacity *= 2;
		}
		Function *previous_functions = ast->functions;
		ast->functions =
		    (Function *)malloc(ast->capacity * sizeof(Function));
		for (int i = 0; i < ast->length - 1; i++) {
			ast->functions[i] = previous_functions[i];
		}
		free(previous_functions);
	}
	ast->functions[ast->length - 1] = function;
}

///// ----- fprintf FUNCTIONS ----- /////

void fprintf_program_type(FILE *file, ProgramType *program_type) {
	switch (*program_type) {
	case NONE:
		fprintf(file, "none");
		break;
	case U8_T:
		fprintf(file, "u8");
		break;
	case U16_T:
		fprintf(file, "u16");
		break;
	case VOID_T:
		fprintf(file, "void");
		break;
	}
}

void fprintf_expression(FILE *file, Expression *expr) {
	switch (expr->tag) {
	case LET_E:
		fprintf(file, "let ");
		fprintf(file, "%s", expr->let.var);
		if (expr->let.type != NONE) {
			fprintf(file, ": ");
			fprintf_program_type(file, &expr->let.type);
		}
		fprintf(file, " = ");
		fprintf_expression(file, expr->let.e);
		break;
	case ADD_E:
		fprintf_expression(file, expr->add.lhs);
		fprintf(file, " + ");
		fprintf_expression(file, expr->add.rhs);
		break;
	case SUB_E:
		fprintf_expression(file, expr->add.lhs);
		fprintf(file, " - ");
		fprintf_expression(file, expr->add.rhs);
		break;
	case VARIABLE_E:
		fprintf(file, "%s", expr->variable.name);
		break;
	case NUMBER_E:
		if (expr->number.is_written_in_hexa) {
			fprintf(file, "0%x", expr->number.value);
		} else {
			fprintf(file, "%d", expr->number.value);
		}
		break;
	case SEQUENCE_E:
		for (int i = 0; i < expr->sequence.length; i++) {
			fprintf_expression(file, &expr->sequence.list[i]);
			if (i != expr->sequence.length) {
				fprintf(file, ";\n");
			}
		}
		break;
	case ASSIGN_E:
		fprintf(file, "%s", expr->assign.var);
		fprintf(file, " = ");
		fprintf_expression(file, expr->assign.e);
		break;
	case DEREF_ASSIGN_E:
		fprintf(file, "*");
		fprintf_expression(file, expr->deref_assign.e1);
		fprintf(file, " = ");
		fprintf_expression(file, expr->deref_assign.e2);
		break;
	case DEREF_E:
		fprintf(file, "*");
		fprintf_expression(file, expr->deref.e);
		break;
	case RETURN_E:
		fprintf(file, "return ");
		fprintf_expression(file, expr->ret.e);
		break;
	case FUNCTION_CALL_E:
		fprintf(file, "%s()", expr->function_call.name);
		break;
	case CHAR_LITERAL_E:
		fprintf(file, "'%c'", expr->char_literal.c);
		break;
	case STRING_LITERAL_E:
		fprintf(file, "\"%c\"", expr->char_literal.c);
		break;
	}
}

void fprintf_function(FILE *file, Function *function) {
	fprintf(file, "fn %s () ", function->name);
	fprintf_program_type(file, &function->type);
	fprintf(file, " = {\n");
	fprintf_expression(file, function->expr);
	fprintf(file, "\n};");
}

// Input :
// - file name to write the tokens
// - Ast that is goinf to be written in the file
void fprintf_ast(FILE *file, Ast *ast) {
	for (int i = 0; i < ast->length; i++) {
		fprintf_function(file, &ast->functions[i]);
		fprintf(file, "\n");
	}
}

void fprintf_line_column(ParseState *state) {
	fprintf(state->error,
		"At line %d, column %d: ", current_token(state).line,
		current_token(state).column);
}

void fprintf_current_token(ParseState *state) {
	fprintf(state->error, "'");
	fprintf_token(state->error, &state->tokens->tokens[state->index]);
	// fprintf(state->error, "' a");
	// fprintf_token_type(state->error,
	// 		   &state->tokens->tokens[state->index].type);
}

///// ----- PARSE FUNCTIONS ----- /////

bool parse_token_type(ParseState *state, TokenType token_type, bool required) {
	if (current_token(state).type == token_type) {
		state->index++;
		return true;
	}
	if (required) {
		fprintf_line_column(state);
		fprintf(state->error, "Expected a '");
		fprintf_token_type(state->error, &token_type);
		fprintf(state->error, "' but got ");
		fprintf_current_token(state);
		fprintf(state->error, "\n");
	}
	return false;
}

ProgramType *parse_program_type(ParseState *state, bool required) {
	if (current_token(state).type == VOID) {
		state->index++;
		ProgramType *type = malloc(sizeof(*type));
		*type = VOID_T;
		return type;
	} else if (current_token(state).type == U8) {
		state->index++;
		ProgramType *type = malloc(sizeof(*type));
		*type = U8_T;
		return type;
	} else if (current_token(state).type == U16) {
		state->index++;
		ProgramType *type = malloc(sizeof(*type));
		*type = U16_T;
		return type;
	}

	if (required) {
		fprintf_line_column(state);
		fprintf(state->error, "Expected type but got ");
		fprintf_current_token(state);
		fprintf(state->error, "\n");
	}

	ProgramType *type = malloc(sizeof(*type));
	*type = U16_T;
	return type;
}

char *parse_identifier(ParseState *state, bool required) {
	Token token = current_token(state);
	if (token.type == IDENTIFIER) {
		state->index++;
		return token.text;
	}
	if (required) {
		fprintf_line_column(state);
		fprintf(state->error, "Expected identifier but got ");
		fprintf_current_token(state);
		fprintf(state->error, "\n");
	}
	return NULL;
}

Expression *parse_number(ParseState *state) {
	// Number in hexadecimal form "0x123" (lex as '0' 'x123')
	if (state->tokens->tokens[state->index].type == NUMBER &&
	    strlen(state->tokens->tokens[state->index].text) == 0 &&
	    state->tokens->tokens[state->index].text[0] == '0') {
		state->index++;
		int i;
		Expression *e = malloc(sizeof(*e));
		if (sscanf(current_token(state).text + 1, "%x", &i) != EOF) {
			state->index++;
			e->tag = NUMBER_E;
			e->number.value = (uint32_t)i;
			e->number.is_written_in_hexa = true;
			return e;
		}
		state->index--;
	}

	// Number in decimal form
	if (current_token(state).type == NUMBER) {
		Expression *e = malloc(sizeof(*e));
		e->tag = NUMBER_E;
		e->number.value =
		    (uint32_t)atoi(state->tokens->tokens[state->index].text);
		// Here this cannot be free
		// because it is not necessarely the last time we look
		// at it
		e->number.is_written_in_hexa = false;
		state->index++;
		return e;
	}
	return NULL;
}

Expression *parse_char_literal(ParseState *state) {
	if (current_token(state).type == CHAR_LITERAL) {
		Expression *e = malloc(sizeof(*e));
		e->tag = CHAR_LITERAL_E;
		e->char_literal.c = current_token(state).text[0];
		free(current_token(state).text);
		state->index++;
		return e;
	}
	return NULL;
}

// TODO
Expression *parse_args_call(ParseState *state, bool required);
// TODO
Expression *parse_args_def(ParseState *state, bool required);

Expression *parse_expr_1(ParseState *state, bool required);

Expression *parse_let(ParseState *state) {
	if (!parse_token_type(state, LET, false)) {
		state->abort = false;
		return NULL;
	}
	char *var = parse_identifier(state, true);
	if (var == NULL) {
		state->abort = true;
		return NULL;
	}
	if (!parse_token_type(state, COLON, true)) {
		state->abort = true;
		return NULL;
	}

	ProgramType *type = parse_program_type(state, true);
	if (type == NULL || !parse_token_type(state, EQUAL, true)) {
		state->abort = true;
		return NULL;
	}
	Expression *e = parse_expr_1(state, true);
	if (state->abort || e == NULL) {
		state->abort = true;
		return NULL;
	}
	Expression *expr = malloc(sizeof(*expr));
	expr->tag = LET_E;
	expr->let.e = e;
	expr->let.type = *type;
	free(type);
	expr->let.var = var;
	return expr;
}

Expression *parse_deref_assign_or_deref(ParseState *state) {
	if (!parse_token_type(state, MULT, false)) {
		state->abort = false;
		return NULL;
	}
	Expression *e1 = parse_expr_1(state, true);
	if (state->abort || e1 == NULL) {
		state->abort = true;
		return NULL;
	}
	if (!parse_token_type(state, EQUAL, false)) {
		Expression *expr = malloc(sizeof(*expr));
		expr->tag = DEREF_E;
		expr->deref.e = e1;
		return expr;
	}
	Expression *e2 = parse_expr_1(state, true);
	if (state->abort || e2 == NULL) {
		state->abort = true;
		return NULL;
	}
	Expression *expr = malloc(sizeof(*expr));
	expr->tag = DEREF_ASSIGN_E;
	expr->deref_assign.e1 = e1;
	expr->deref_assign.e2 = e2;
	return expr;
}

Expression *parse_return(ParseState *state) {
	if (!parse_token_type(state, RETURN, false)) {
		state->abort = false;
		return NULL;
	}
	Expression *e = parse_expr_1(state, true);
	if (state->abort || e == NULL) {
		state->abort = true;
		return NULL;
	}
	Expression *expr = malloc(sizeof(*expr));
	expr->tag = RETURN_E;
	expr->ret.e = e;
	return expr;
}

Expression *parse_func_call_or_var_assign_or_var(ParseState *state) {
	char *identifier = parse_identifier(state, false);
	if (identifier == NULL) {
		state->abort = false;
		return NULL;
	}
	if (parse_token_type(state, LPAREN, false)) {
		if (parse_token_type(state, RPAREN, true)) {
			Expression *expr = malloc(sizeof(*expr));
			expr->tag = FUNCTION_CALL_E;
			expr->function_call.name = identifier;
			return expr;
		} else {
			state->abort = true;
			return NULL;
		}
	}

	if (parse_token_type(state, EQUAL, false)) {
		Expression *e = parse_expr_1(state, true);
		if (state->abort || e == NULL) {
			state->abort = true;
			return NULL;
		}
		Expression *expr = malloc(sizeof(*expr));
		expr->tag = ASSIGN_E;
		expr->assign.e = e;
		expr->assign.var = identifier;
		return expr;
	}

	Expression *expr = malloc(sizeof(*expr));
	expr->tag = VARIABLE_E;
	expr->variable.name = identifier;
	return expr;
}

Expression *parse_expr_2(ParseState *state, bool required) {
	Expression *e = NULL;

	// Char Literal: 'c'
	e = parse_char_literal(state);
	if (state->abort) {
		return NULL;
	}
	if (e != NULL) {
		return e;
	}

	// TODO string literal

	// Let: let var = e
	e = parse_let(state);
	if (state->abort) {
		return NULL;
	}
	if (e != NULL) {
		return e;
	}

	// Deref Assign or Deref:  *e = e  OR  *e
	e = parse_deref_assign_or_deref(state);
	if (state->abort) {
		return NULL;
	}
	if (e != NULL) {
		return e;
	}

	// Return : return e
	e = parse_return(state);
	if (state->abort) {
		return NULL;
	}
	if (e != NULL) {
		return e;
	}

	// Function call OR Assign OR Var : ident() OR ident = e OR ident
	e = parse_func_call_or_var_assign_or_var(state);
	if (state->abort) {
		return NULL;
	}
	if (e != NULL) {
		return e;
	}

	// NUMBER : n
	e = parse_number(state);
	if (state->abort) {
		return NULL;
	}
	if (e != NULL) {
		return e;
	}

	if (required) {
		fprintf(state->error,
			"Expected an Expression2 at line %d and column %d",
			current_token(state).line, current_token(state).column);
		fprintf(state->error, "\n");
	}
	return NULL;
}

Expression *parse_expr_1(ParseState *state, bool required) {
	Expression *lhs = parse_expr_2(state, true);
	if (state->abort || lhs == NULL) {
		if (required) {
			fprintf_line_column(state);
			fprintf(state->error, "Expected an Expression1 \n");
		}
		return NULL;
	}

	if (parse_token_type(state, PLUS, false)) {
		Expression *rhs = parse_expr_1(state, true);
		if (state->abort || rhs == NULL) {
			state->abort = required;
			free(lhs);
			return NULL;
		}

		Expression *expr = malloc(sizeof(*expr));
		expr->tag = ADD_E;
		expr->add.lhs = lhs;
		expr->add.rhs = rhs;
		return expr;
	}

	if (parse_token_type(state, MINUS, false)) {
		Expression *rhs = parse_expr_1(state, true);
		if (state->abort || rhs == NULL) {
			state->abort = required;
			free(lhs);
			return NULL;
		}
		Expression *expr = malloc(sizeof(*expr));
		expr->tag = SUB_E;
		expr->sub.lhs = lhs;
		expr->sub.rhs = rhs;
		return expr;
	}

	return lhs;
}

Expression *parse_expr(ParseState *state) {
	if (!parse_token_type(state, LBRACE, false)) {
		return NULL;
	}

	Expression *expr = NULL;

	// Sequence : (expression ;)*
	while (true) {
		bool required = false;
		if (expr == NULL) {
			required = true;
		}
		Expression *expr_next = parse_expr_1(state, required);
		if (state->abort) {
			return NULL;
		}
		if (expr_next == NULL) {
			break;
		}

		if (!parse_token_type(state, SEMICOLON, true)) {
			state->abort = true;
			return NULL;
		}

		if (expr == NULL) {
			expr = malloc(sizeof(*expr));
			expr->tag = SEQUENCE_E;
			expr->sequence.length = 0;
			expr->sequence.list = NULL;
		}
		expr->sequence.length++;
		Expression *prev_list = expr->sequence.list;
		expr->sequence.list =
		    malloc(expr->sequence.length * sizeof(Expression));
		for (int i = 0; i < expr->sequence.length - 1; i++) {
			expr->sequence.list[i] = prev_list[i];
		}
		if (prev_list != NULL) {
			free(prev_list);
		}

		expr->sequence.list[expr->sequence.length - 1] = *expr_next;
		free(expr_next);
	}

	if (expr == NULL && state->abort) {
		fprintf(state->error, "Empty block are not allowed\n");
		return NULL;
	}

	if (!parse_token_type(state, RBRACE, true)) {
		state->abort = true;
		return NULL;
	}

	return expr;
}

// fn indentifier () type = expression
Function *parse_function(ParseState *state, bool required) {
	uint32_t prev_index;

	if (!parse_token_type(state, FN, false)) {
		return NULL;
	}

	char *name = parse_identifier(state, required);
	if (name == NULL) {
		state->abort = required;
		return NULL;
	}

	if (!parse_token_type(state, LPAREN, true)) {
		state->abort = required;
		return NULL;
	}

	if (!parse_token_type(state, RPAREN, true)) {
		state->abort = required;
		return NULL;
	}

	ProgramType *type = parse_program_type(state, true);
	if (type == NULL) {
		state->abort = required;
		return NULL;
	}

	if (!parse_token_type(state, EQUAL, true)) {
		state->abort = required;
		return NULL;
	}

	Expression *expr = parse_expr(state);
	if (state->abort || expr == NULL) {
		state->abort = required;
		return NULL;
	}

	Function *function = malloc(sizeof(*function));
	function->name = name;
	function->expr = expr;
	function->type = *type;
	free(type);
	return function;
}

// Input : list of tokens
// Ouput : result of an Ast constructed from those tokens
Ast *parse(FILE *error, Tokens *tokens) {
	Ast *ast = ast_new();

	ParseState state;
	state.error = error;
	state.tokens = tokens;
	state.index = 0;
	state.abort = false;

	while (state.index < state.tokens->length) {
		Function *function = parse_function(&state, true);
		if (state.abort) {
			return NULL;
		}
		if (function == NULL) {
			break;
		}
		if (parse_token_type(&state, SEMICOLON, true)) {
			ast_append_function(ast, *function);
			free(function);
		} else {
			fprintf_line_column(&state);
			fprintf(error, "Missing ';' at end of function\n");
			return false;
		}
	}

	if (state.index != tokens->length) {
		ast_delete(ast);
		ast = NULL;
	}
	tokens_free_numbers(tokens);
	free(tokens);
	return ast;
}
