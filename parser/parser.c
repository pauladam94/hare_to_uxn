#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		if (expr->let.type != NONE) {
			fprintf(file, ": ");
			fprintf_program_type(file, &expr->let.type);
		}
		fprintf(file, "%s = ", expr->let.var);
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

bool parse_token_type(FILE *error, Tokens *tokens, uint32_t *index,
		      TokenType token_type, bool should_work) {
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

bool parse_program_type(FILE *error, Tokens *tokens, uint32_t *index,
			ProgramType *program_type, bool should_work) {
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
		fprintf(error,
			"Expected program type at line %d and column "
			"%d but got '",
			tokens->tokens[*index].line,
			tokens->tokens[*index].column);
		fprintf_token(error, &tokens->tokens[*index]);
		fprintf(error, "' a ");
		fprintf_token_type(error, &tokens->tokens[*index].type);
		fprintf(error, "\n");
	}
	return false;
}

bool parse_identifier(FILE *error, Tokens *tokens, uint32_t *index, char **s,
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

bool parse_number(FILE *error, Tokens *tokens, uint32_t *index, Expression *e,
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
		}
		index--;
	}

	// Number in decimal form
	if (tokens->tokens[*index].type == NUMBER) {
		e->tag = NUMBER_E;
		e->number.value = (uint32_t)atoi(tokens->tokens[*index].text);
		// Here this cannot be free
		// because it is not necessarely the last time we look
		// at it
		e->number.is_written_in_hexa = false;
		*index += 1;
		return true;
	}
	return false;
}

bool parse_char_literal(FILE *error, Tokens *tokens, uint32_t *index,
			Expression *expr, bool should_work) {
	if (tokens->tokens[*index].type == CHAR_LITERAL) {
		expr->tag = CHAR_LITERAL_E;
		expr->char_literal.c = tokens->tokens[*index].text[0];
		free(tokens->tokens[*index].text);
		*index += 1;
		return true;
	}
	return false;
}

// TODO
bool parse_args_call(FILE *error);
// TODO
bool parse_args_def(FILE *error);

bool parse_let(FILE *error, Tokens *tokens, uint32_t *index, Expression *expr,
	       bool should_work) {
	return false;
}

bool parse_expr_1(FILE *error, Tokens *tokens, uint32_t *index,
		  Expression *expr, bool should_work);

bool parse_expr_2(FILE *error, Tokens *tokens, uint32_t *i, Expression *expr,
		  bool should_work) {
	uint32_t prev_index;
	Expression *e;
	Expression *e1;
	Expression *e2;

	// Trying to parse : 'char'
	if (parse_char_literal(error, tokens, i, expr, true && should_work)) {
		return true;
	}

	// if (parse_token_type(error, tokens, i, LET, false)) {
	// 	if (parse_identifier(error, tokens, i, &expr->let.var, true)) {
	//
	// 	} else {
	// 		return false;
	// 	}
	// }

	// Trying to parse : let var : type = e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_token_type(error, tokens, i, LET, false) &&
	    parse_identifier(error, tokens, i, &expr->let.var,
			     true && should_work) &&
	    parse_token_type(error, tokens, i, COLON, false) &&
	    parse_program_type(error, tokens, i, &expr->let.type, false) &&
	    parse_token_type(error, tokens, i, EQUAL, true && should_work) &&
	    parse_expr_1(error, tokens, i, e, true && should_work)) {
		expr->tag = LET_E;
		expr->let.e = e;
		return true;
	}
	free(e);
	e = NULL;
	*i = prev_index;

	// Trying to parse : let var = e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_token_type(error, tokens, i, LET, false) &&
	    parse_identifier(error, tokens, i, &expr->let.var,
			     true && should_work) &&
	    parse_token_type(error, tokens, i, EQUAL, true && should_work) &&
	    parse_expr_1(error, tokens, i, e, true && should_work)) {
		expr->tag = LET_E;
		expr->let.type = NONE;
		expr->let.e = e;
		return true;
	}
	free(e);
	e = NULL;
	*i = prev_index;

	// Trying to parse : *e1 = e2
	prev_index = *i;
	e1 = malloc(sizeof(*e1));
	e2 = malloc(sizeof(*e2));
	if (parse_token_type(error, tokens, i, MULT, false) &&
	    parse_expr_1(error, tokens, i, e1, true && should_work) &&
	    parse_token_type(error, tokens, i, EQUAL, true && should_work) &&
	    parse_expr_1(error, tokens, i, e2, true && should_work)) {
		expr->tag = DEREF_ASSIGN_E;
		expr->deref_assign.e1 = e1;
		expr->deref_assign.e2 = e2;
		return true;
	}
	free(e1);
	free(e2);
	e1 = NULL;
	e2 = NULL;
	*i = prev_index;

	// Trying to parse : *e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_token_type(error, tokens, i, MULT, false) &&
	    parse_expr_1(error, tokens, i, e, true && should_work)) {
		expr->tag = DEREF_E;
		expr->deref.e = e;
		return true;
	}
	free(e);
	e = NULL;
	*i = prev_index;

	// Try to parse : return e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_token_type(error, tokens, i, RETURN, false) &&
	    parse_expr_1(error, tokens, i, e, true && should_work)) {
		expr->tag = RETURN_E;
		expr->ret.e = e;
		return true;
	}
	free(e);
	e = NULL;
	*i = prev_index;

	// Try to parse : var = e
	prev_index = *i;
	e = malloc(sizeof(*e));
	if (parse_identifier(error, tokens, i, &expr->assign.var, false) &&
	    parse_token_type(error, tokens, i, EQUAL, false) &&
	    parse_expr_1(error, tokens, i, e, true && should_work)) {
		expr->tag = ASSIGN_E;
		expr->assign.e = e;
		return true;
	}
	free(e);
	*i = prev_index;

	// Try to parse : function_name ( ) TODO args
	prev_index = *i;
	if (parse_identifier(error, tokens, i, &expr->function_call.name,
			     false) &&
	    parse_token_type(error, tokens, i, LPAREN, false) &&
	    parse_token_type(error, tokens, i, RPAREN, true && should_work)) {
		expr->tag = FUNCTION_CALL_E;
		return true;
	}
	*i = prev_index;

	if (parse_identifier(error, tokens, i, &expr->variable.name, false)) {
		expr->tag = VARIABLE_E;
		return true;
	}

	if (parse_number(error, tokens, i, expr, false)) {
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

bool parse_expr_1(FILE *error, Tokens *tokens, uint32_t *index,
		  Expression *expr, bool should_work) {
	uint32_t prev_index;

	Expression *lhs = malloc(sizeof(*lhs));

	if (!parse_expr_2(error, tokens, index, lhs, false)) {
		if (should_work) {
			fprintf(error,
				"Expected an Expression1 at line %d "
				"and column "
				"%d\n",
				tokens->tokens[*index].line,
				tokens->tokens[*index].column);
		}
		free(lhs);
		return false;
	}

	if (parse_token_type(error, tokens, index, PLUS, false)) {
		Expression *rhs = malloc(sizeof(*rhs));
		if (parse_expr_1(error, tokens, index, rhs,
				 true && should_work)) {
			expr->tag = ADD_E;
			expr->add.lhs = lhs;
			expr->add.rhs = rhs;
			return true;
		}
		free(rhs);
	}

	if (parse_token_type(error, tokens, index, MINUS, false)) {
		Expression *rhs = malloc(sizeof(*rhs));
		if (parse_expr_1(error, tokens, index, rhs, should_work)) {
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

bool parse_expr(FILE *error, Tokens *tokens, uint32_t *index, Expression *expr,
		bool should_work) {
	bool expression_is_empty = true;

	if (!parse_token_type(error, tokens, index, LBRACE, true)) {
		return false;
	}

	// trying parsing : (expression ;)*
	uint32_t prev_index = *index;
	Expression expr_next;
	while (parse_expr_1(error, tokens, index, &expr_next, false) &&
	       parse_token_type(error, tokens, index, SEMICOLON,
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

	if (!parse_token_type(error, tokens, index, RBRACE, true)) {
		return false;
	}

	return true;
}

// fn indentifier () type = expression
bool parse_function(FILE *error, Tokens *tokens, uint32_t *index,
		    Function *function) {
	char *function_name;
	uint32_t prev_index;

	Expression *expression = malloc(sizeof(*expression));
	prev_index = *index;
	if (parse_token_type(error, tokens, index, FN, true) &&
	    parse_identifier(error, tokens, index, &function_name, true) &&
	    parse_token_type(error, tokens, index, LPAREN, true) &&
	    parse_token_type(error, tokens, index, RPAREN, true) &&
	    parse_program_type(error, tokens, index, &function->type, true) &&
	    parse_token_type(error, tokens, index, EQUAL, true) &&
	    parse_expr(error, tokens, index, expression, true)) {
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
Ast *parse(FILE *error, Tokens *tokens) {
	uint32_t index = 0;
	Ast *ast = ast_new();

	while (index < tokens->length) {
		Function function;
		if (parse_function(error, tokens, &index, &function)) {
			if (parse_token_type(error, tokens, &index, SEMICOLON,
					     true)) {
				ast_append_function(ast, function);
			} else {
				fprintf(error, "Function should be "
					       "separated by ';'");
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
