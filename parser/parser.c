#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///// ----- EXPRESSION FUNCTIONS ----- /////

/// Deletes completely recursively what expression points to
/// It does not free the expression it self
void expression_delete(Expression *expr) {
	switch (expr->tag) {
	case LET_E: {
		free(expr->let.var);
		expression_delete(expr->let.e);
		break;
	}
	case ADD_E: {
		expression_delete(expr->add.lhs);
		expression_delete(expr->add.rhs);
		break;
	}
	case SUB_E: {
		expression_delete(expr->sub.lhs);
		expression_delete(expr->sub.rhs);
		break;
	}
	case VARIABLE_E: {
		free(expr->variable.name);
		break;
	}
	case NUMBER_E: {
		break;
	}
	case SEQUENCE_E: {
		for (int i = 0; i < expr->sequence.length; i++) {
			expression_delete(&expr->sequence.list[i]);
		}
		free(expr->sequence.list);
		break;
	}
	case ASSIGN_E: {
		free(expr->assign.var);
		expression_delete(expr->assign.e);
		break;
	}
	case DEREF_ASSIGN_E: {
		expression_delete(expr->deref_assign.e1);
		expression_delete(expr->deref_assign.e2);
		break;
	}
	case RETURN_E: {
		expression_delete(expr->ret.e);
		break;
	}
	case FUNCTION_CALL_E: {
		free(expr->function_call.name);
		break;
	}
	case CHAR_LITERAL_E: {
		break;
	}
	}
	// free(expr); TODO : do that or free in ast_delete
}

void expression_not_tokens_delete(Expression *expr) {
	switch (expr->tag) {
	case LET_E: {
		// free(expr->let.var);
		expression_not_tokens_delete(expr->let.e);
		break;
	}
	case ADD_E: {
		expression_not_tokens_delete(expr->add.lhs);
		expression_not_tokens_delete(expr->add.rhs);
		break;
	}
	case SUB_E: {
		expression_not_tokens_delete(expr->sub.lhs);
		expression_not_tokens_delete(expr->sub.rhs);
		break;
	}
	case VARIABLE_E: {
		// free(expr->variable.name);
		break;
	}
	case NUMBER_E: {
		break;
	}
	case SEQUENCE_E: {
		for (int i = 0; i < expr->sequence.length; i++) {
			expression_not_tokens_delete(&expr->sequence.list[i]);
		}
		free(expr->sequence.list);
		break;
	}
	case ASSIGN_E: {
		expression_not_tokens_delete(expr->assign.e);
		break;
	}
	case DEREF_ASSIGN_E: {
		expression_not_tokens_delete(expr->deref_assign.e1);
		expression_not_tokens_delete(expr->deref_assign.e2);
		break;
	}
	case RETURN_E: {
		expression_not_tokens_delete(expr->ret.e);
		break;
	}
	case FUNCTION_CALL_E: {
		break;
	}
	case CHAR_LITERAL_E: {
		break;
	}
	}
	free(expr);
}

///// ----- AST FUNCTIONS ----- /////

// Free a complete AST structure
void ast_delete(Ast *ast) {
	for (int i = 0; i < ast->length; i++) {
		free(ast->functions[i].name);
		expression_delete(ast->functions[i].expr);
		free(ast->functions[i].expr);
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
	case U8_T: {
		fprintf(file, "u8");
		break;
	}
	case U16_T: {
		fprintf(file, "u16");
		break;
	}
	case VOID_T: {
		fprintf(file, "void");
		break;
	}
	}
}

void fprintf_expression(FILE *file, Expression *expr) {
	switch (expr->tag) {
	case LET_E: {
		fprintf(file, "let %s = ", expr->let.var);
		fprintf_expression(file, expr->let.e);
		break;
	}
	case ADD_E: {
		fprintf_expression(file, expr->add.lhs);
		fprintf(file, " + ");
		fprintf_expression(file, expr->add.rhs);
		break;
	}
	case SUB_E: {
		fprintf_expression(file, expr->add.lhs);
		fprintf(file, " - ");
		fprintf_expression(file, expr->add.rhs);
		break;
	}
	case VARIABLE_E: {
		fprintf(file, "%s", expr->variable.name);
		break;
	}
	case NUMBER_E: {
		if (expr->number.is_written_in_hexa) {
			fprintf(file, "0%x", expr->number.value);
		} else {
			fprintf(file, "%d", expr->number.value);
		}
		break;
	}
	case SEQUENCE_E: {
		for (int i = 0; i < expr->sequence.length; i++) {
			fprintf_expression(file, &expr->sequence.list[i]);
			if (i != expr->sequence.length) {
				fprintf(file, ";\n");
			}
		}
		break;
	}
	case ASSIGN_E: {
		fprintf(file, "%s", expr->assign.var);
		fprintf(file, " = ");
		fprintf_expression(file, expr->assign.e);
		break;
	}
	case DEREF_ASSIGN_E: {
		fprintf(file, "*");
		fprintf_expression(file, expr->deref_assign.e1);
		fprintf(file, " = ");
		fprintf_expression(file, expr->deref_assign.e2);
		break;
	}
	case RETURN_E: {
		fprintf(file, "return ");
		fprintf_expression(file, expr->ret.e);
		break;
	}
	case FUNCTION_CALL_E: {
		fprintf(file, "%s()", expr->function_call.name);
		break;
	}
	case CHAR_LITERAL_E: {
		fprintf(file, "'%c'", expr->char_literal.c);
		break;
	}
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

/// PARSE FUNCTIONS API
///
/// bool parse_[name] (Tokens *tokens, uint32_t *index, [Name]* [name],
///                        FILE* error, bool should_work);
///
/// # return true
/// - we managed to parse [name] element from 'tokens' at the position 'index'.
/// - 'index' has increase according to the number of tokens read
/// - 'tokens' has not been mutated
/// - 'name' contains some sort of result from what has been parsed
/// - 'err' has not been mutated
/// - 'should_work' is not taken into account
///
/// # return false
/// - we did not managed to parse [name] element from 'tokens' at 'index'.
/// - 'index' is the value as before calling the function
/// - 'tokens' has not been mutated
/// - 'name' has not been changed from his state before the call
/// - 'should_work' : true
///     - 'error' is mutated with an error saying we expected something specific
/// - 'should_work' : false
///     - 'error' is not mutated
///
/// The should_work boolean means that the fonction should work or it's an
/// error. Inside a function, if you try to parse 'let', should_work = false
/// because other possibility can be parsed. 'let' is not the only possibility.

bool parse_token_type(Tokens *tokens, uint32_t *index, TokenType token_type,
		      FILE *error, bool should_work) {
	if (tokens->tokens[*index].type == token_type) {
		*index += 1;
		return true;
	}
	if (should_work) {
		fprintf(error, "Expected '");
		fprintf_token_type(error, &token_type);
		fprintf(error, "' at line %d and column %d but got '",
			tokens->tokens[*index].line,
			tokens->tokens[*index].column);
		fprintf_token(error, &tokens->tokens[*index]);
		fprintf(error, "' a ");
		fprintf_token_type(error, &tokens->tokens[*index].type);
		fprintf(error, "\n");
	}
	return false;
}

bool parse_program_type(Tokens *tokens, uint32_t *index,
			ProgramType *program_type, FILE *error,
			bool should_work) {
	if (tokens->tokens[*index].type == VOID) {
		*program_type = VOID_T;
		*index += 1;
		return true;
	} else if (tokens->tokens[*index].type == U8) {
		*program_type = U8_T;
		*index += 1;
		return true;
	} else if (tokens->tokens[*index].type == U16) {
		*program_type = U16_T;
		*index += 1;
		return true;
	}

	if (should_work) {
		fprintf(
		    error,
		    "Expected program type at line %d and column %d but got '",
		    tokens->tokens[*index].line, tokens->tokens[*index].column);
		fprintf_token(error, &tokens->tokens[*index]);
		fprintf(error, "' a ");
		fprintf_token_type(error, &tokens->tokens[*index].type);
		fprintf(error, "\n");
	}
	return false;
}

bool parse_identifier(Tokens *tokens, uint32_t *index, char **s, FILE *error,
		      bool should_work) {
	if (tokens->tokens[*index].type == IDENTIFIER) {
		*s = tokens->tokens[*index].text;
		*index += 1;
		return true;
	}
	if (should_work) {
		fprintf(error, "expected identifier line %d and column %d\n",
			tokens->tokens[*index].line,
			tokens->tokens[*index].column);
		fprintf_token(error, &tokens->tokens[*index]);
		fprintf(error, "' a ");
		fprintf_token_type(error, &tokens->tokens[*index].type);
		fprintf(error, "\n");
	}
	return false;
}

bool parse_number(Tokens *tokens, uint32_t *index, Expression *e, FILE *error,
		  bool should_work) {

	// Number in hexadecimal form "0x123" (lex as '0' 'x123')
	if (tokens->tokens[*index].type == NUMBER &&
	    strlen(tokens->tokens[*index].text) == 0 &&
	    tokens->tokens[*index].text[0] == '0') {
		*index += 1;
		int i;
		if (sscanf(tokens->tokens[*index].text + 1, "%x", &i) != EOF) {
			*index += 1;
			e->tag = NUMBER_E;
			e->number.value = (uint32_t)i;
			e->number.is_written_in_hexa = true;
			return true;
		} else {
			index--;
		}
	}

	// Number in decimal form
	if (tokens->tokens[*index].type == NUMBER) {
		e->tag = NUMBER_E;
		e->number.value = (uint32_t)atoi(tokens->tokens[*index].text);
		// Here this cannot be free
		// because it is not necessarely the last time we look at it
		e->number.is_written_in_hexa = false;
		*index += 1;
		return true;
	}

	// TODO que ça marche aussi avec les nombres hexa
	// if (tokens->tokens[*index].type == NUMBER) {
	//
	// }

	// if (should_work) {
	// 	fprintf(
	// 	    error, "Expected number at line %d and column %d but got '",
	// 	    tokens->tokens[*index].line, tokens->tokens[*index].column);
	// 	fprintf_token(error, &tokens->tokens[*index]);
	// 	fprintf(error, "' a ");
	// 	fprintf_token_type(error, &tokens->tokens[*index].type);
	// 	fprintf(error, "\n");
	// }
	return false;
}

bool parse_char_literal(Tokens *tokens, uint32_t *index, Expression *expr,
			FILE *error, bool should_work) {
	if (tokens->tokens[*index].type == CHAR_LITERAL) {
		expr->tag = CHAR_LITERAL_E;
		expr->char_literal.c = tokens->tokens[*index].text[0];
		*index += 1;
		return true;
	}
	return false;
}

// TODO
bool parse_args_call(FILE *error);
// TODO
bool parse_args_def(FILE *error);

bool parse_let(Tokens *tokens, uint32_t *index, Expression *expr, FILE *error,
	       bool should_work) {
	return false;
}

bool parse_expr_1(Tokens *tokens, uint32_t *index, Expression *expr,
		  FILE *error, bool should_work);

bool parse_expr_2(Tokens *tokens, uint32_t *i, Expression *expr, FILE *error,
		  bool should_work) {
	uint32_t prev_index;
	Expression *e;
	Expression *e1;
	Expression *e2;

	// Trying to parse : 'char'
	if (parse_char_literal(tokens, i, expr, error, true && should_work)) {
		return true;
	}
	// Trying to parse :let var = e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_token_type(tokens, i, LET, error, false) &&
	    parse_identifier(tokens, i, &expr->let.var, error,
			     true && should_work) &&
	    parse_token_type(tokens, i, EQUAL, error, true && should_work) &&
	    parse_expr_1(tokens, i, e, error, true && should_work)) {
		expr->tag = LET_E;
		expr->let.e = e;
		return true;
	} else {
		free(e);
		e = NULL;
	}
	*i = prev_index;

	// Trying to parse : * e1 = e2
	prev_index = *i;
	e1 = malloc(sizeof(*e1));
	e2 = malloc(sizeof(*e2));
	if (parse_token_type(tokens, i, MULT, error, false) &&
	    parse_expr_1(tokens, i, e1, error, true && should_work) &&
	    parse_token_type(tokens, i, EQUAL, error, true && should_work) &&
	    parse_expr_1(tokens, i, e2, error, true && should_work)) {
		expr->tag = DEREF_ASSIGN_E;
		expr->deref_assign.e1 = e1;
		expr->deref_assign.e2 = e2;
		return true;
	} else {
		free(e1);
		free(e2);
		e1 = NULL;
		e2 = NULL;
	}
	*i = prev_index;

	// Try to parse : return e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_token_type(tokens, i, RETURN, error, false) &&
	    parse_expr_2(tokens, i, e, error, true && should_work)) {
		expr->tag = RETURN_E;
		expr->ret.e = e;
		return true;
	} else {
		free(e);
		e = NULL;
	}
	*i = prev_index;

	// Try to parse : var = e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_identifier(tokens, i, &expr->assign.var, error, false) &&
	    parse_token_type(tokens, i, EQUAL, error, false) &&
	    parse_expr_1(tokens, i, e, error, true && should_work)) {
		expr->tag = ASSIGN_E;
		expr->assign.e = e;
		return true;
	} else {
		free(e);
	}
	*i = prev_index;

	// Try to parse : function_name ( ) TODO args
	prev_index = *i;
	if (parse_identifier(tokens, i, &expr->function_call.name, error,
			     false) &&
	    parse_token_type(tokens, i, LPAREN, error, false) &&
	    parse_token_type(tokens, i, RPAREN, error, true && should_work)) {
		expr->tag = FUNCTION_CALL_E;
		return true;
	}
	*i = prev_index;

	if (parse_identifier(tokens, i, &expr->variable.name, error, false)) {
		expr->tag = VARIABLE_E;
		return true;
	}

	if (parse_number(tokens, i, expr, error, false)) {
		return true;
	}

	if (should_work) {
		fprintf(error,
			"Expected an Expression2 at line %d and column %d",
			tokens->tokens[*i].line, tokens->tokens[*i].column);
		fprintf(error, "\n");
	}
	return false;
}

bool parse_expr_1(Tokens *tokens, uint32_t *index, Expression *expr,
		  FILE *error, bool should_work) {
	uint32_t prev_index;

	Expression *lhs = malloc(sizeof(*lhs));

	if (!parse_expr_2(tokens, index, lhs, error, false)) {
		if (should_work) {
			fprintf(error,
				"Expected an Expression1 at line %d and column "
				"%d\n",
				tokens->tokens[*index].line,
				tokens->tokens[*index].column);
		}
		free(lhs);
		return false;
	}

	if (parse_token_type(tokens, index, PLUS, error, false)) {
		Expression *rhs = malloc(sizeof(*rhs));
		if (parse_expr_1(tokens, index, rhs, error,
				 true && should_work)) {
			expr->tag = ADD_E;
			expr->add.lhs = lhs;
			expr->add.rhs = rhs;
			return true;
		}
		free(rhs);
	}

	if (parse_token_type(tokens, index, MINUS, error, false)) {
		Expression *rhs = malloc(sizeof(*rhs));
		if (parse_expr_1(tokens, index, rhs, error, should_work)) {
			expr->tag = SUB_E;
			expr->add.lhs = lhs;
			expr->add.rhs = rhs;
			return true;
		}
		free(rhs);
	}

	*expr = *lhs;
	free(lhs);
	return true;
}

bool parse_expr(Tokens *tokens, uint32_t *index, Expression *expr, FILE *error,
		bool should_work) {
	bool expression_is_empty = true;

	if (!parse_token_type(tokens, index, LBRACE, error, true)) {
		return false;
	}

	// trying parsing : (expression ;)*
	uint32_t prev_index = *index;
	Expression expr_next;
	while (parse_expr_1(tokens, index, &expr_next, error, false) &&
	       parse_token_type(tokens, index, SEMICOLON, error,
				true && should_work)) {
		prev_index = *index;
		if (expression_is_empty) {
			expression_is_empty = false;
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

		expr->sequence.list[expr->sequence.length - 1] = expr_next;
	}
	*index = prev_index;

	if (expression_is_empty && should_work) {
		fprintf(error, "Empty block are not allowed\n");
		return false;
	}

	if (!parse_token_type(tokens, index, RBRACE, error, true)) {
		return false;
	}

	return true;
}

// fn indentifier () type = expression
bool parse_function(Tokens *tokens, uint32_t *index, Function *function,
		    FILE *error) {
	char *function_name;
	uint32_t prev_index;

	Expression *expression = malloc(sizeof(*expression));
	prev_index = *index;
	if (parse_token_type(tokens, index, FN, error, true) &&
	    parse_identifier(tokens, index, &function_name, error, true) &&
	    parse_token_type(tokens, index, LPAREN, error, true) &&
	    parse_token_type(tokens, index, RPAREN, error, true) &&
	    parse_program_type(tokens, index, &function->type, error, true) &&
	    parse_token_type(tokens, index, EQUAL, error, true) &&
	    parse_expr(tokens, index, expression, error, true)) {
		function->name = function_name;
		function->expr = expression;
		return true;
	} else {
		free(expression);
	}
	*index = prev_index;

	return false;
}

// Input : list of tokens
// Ouput : result of an Ast constructed from those tokens
Ast *parse(Tokens *tokens, FILE *error) {
	uint32_t index = 0;
	Ast *ast = ast_new();

	while (index < tokens->length) {
		Function function;
		if (parse_function(tokens, &index, &function, error)) {
			if (parse_token_type(tokens, &index, SEMICOLON, error,
					     true)) {
				// function is stack allocated and copied here
				ast_append_function(ast, function);
			} else {
				fprintf(error,
					"Function should be separated by ';'");
				// expression_free(function.expr);
				return false;
			}
		} else {
			break;
		}
	}

	if (index != tokens->length) {
		ast_delete(ast);
		ast = NULL;
		fprintf(error, "Error during Parsing\n");
	}
	tokens_free_numbers(tokens);
	free(tokens);
	return ast;
}
