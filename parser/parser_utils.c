#include "parser_utils.h"
#include <stdlib.h>

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
	case SUB_E:
	case MULT_E:
	case DIV_E:
	case EQUAL_EQUAL_E:
	case NOT_EQUAL_E:
	case GREATER_THAN_EQUAL_E:
	case GREATER_THAN_E:
	case LESS_THAN_EQUAL_E:
	case LESS_THAN_E:
		expression_delete(expr->binary.lhs, true);
		expression_delete(expr->binary.rhs, true);
		break;
	case VARIABLE_E:
		free(expr->variable.name);
		break;
	case NUMBER_E:
		break;
	case SEQUENCE_E:
		for (int i = 0; i < expr->sequence.len; i++) {
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
	case IF_ELSE_E:
		expression_delete(expr->if_else.if_body, true);
		expression_delete(expr->if_else.cond, true);
		if (expr->if_else.else_body == NULL) {
			break;
		}
		expression_delete(expr->if_else.else_body, true);
	}
	if (delete_itself) {
		free(expr);
	}
}

///// ----- AST FUNCTIONS ----- /////

// Free a complete AST structure
void ast_delete(Ast *ast) {
	for (int i = 0; i < ast->len; i++) {
		free(ast->functions[i].name);
		expression_delete(ast->functions[i].expr, true);
		// free(ast->functions[i].expr);
	}
	free(ast->functions);
	free(ast);
}

Ast *ast_new(void) {
	Ast *ast = malloc(sizeof(*ast));
	ast->len = 0;
	ast->cap = 0;
	ast->functions = NULL;
	return ast;
}

// function should not be NULL
void ast_append(Ast *ast, Function function) {
	ast->len += 1;
	if (ast->len > ast->cap) {
		ast->cap = (ast->cap == 0) ? 1 : ast->cap * 2;
		ast->functions = (Function *)realloc(
		    ast->functions, ast->cap * sizeof(Function));
	}
	ast->functions[ast->len - 1] = function;
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

int precedence(ExpressionType type) {
	switch (type) {
	case NOT_EQUAL_E:
	case EQUAL_EQUAL_E:
	case GREATER_THAN_EQUAL_E:
	case GREATER_THAN_E:
	case LESS_THAN_EQUAL_E:
	case LESS_THAN_E:
		return 3;
	case ADD_E:
	case SUB_E:
		return 2;
	case MULT_E:
	case DIV_E:
		return 1;
	default:
		return 0;
	}
}

TokenType binary_tag_to_token_type(ExpressionType type) {
	switch (type) {
	case ADD_E:
		return PLUS;
	case SUB_E:
		return MINUS;
	case MULT_E:
		return MULT;
	case DIV_E:
		return DIVIDE;
	case NOT_EQUAL_E:
		return NOT_EQUAL;
	case EQUAL_EQUAL_E:
		return EQUAL_EQUAL;
	case GREATER_THAN_EQUAL_E:
		return GREATER_THAN_EQUAL;
	case GREATER_THAN_E:
		return GREATER_THAN;
	case LESS_THAN_EQUAL_E:
		return LESS_THAN_EQUAL;
	case LESS_THAN_E:
		return LESS_THAN;
	default:
		return VOID;
	}
}
// For
ExpressionType token_type_to_binary_tag(TokenType type) {
	switch (type) {
	case PLUS:
		return ADD_E;
	case MINUS:
		return SUB_E;
	case MULT:
		return MULT_E;
	case DIVIDE:
		return DIV_E;
	case NOT_EQUAL:
		return EQUAL_EQUAL_E;
	case EQUAL_EQUAL:
		return EQUAL_EQUAL_E;
	case GREATER_THAN_EQUAL:
		return GREATER_THAN_EQUAL_E;
	case GREATER_THAN:
		return GREATER_THAN_E;
	case LESS_THAN_EQUAL:
		return LESS_THAN_EQUAL_E;
	case LESS_THAN:
		return LESS_THAN_E;
	default:
		printf("unreachable");
		return LET_E; // this should no happend
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
	case SUB_E:
	case MULT_E:
	case DIV_E:
	case NOT_EQUAL_E:
	case EQUAL_EQUAL_E:
	case GREATER_THAN_EQUAL_E:
	case GREATER_THAN_E:
	case LESS_THAN_EQUAL_E:
	case LESS_THAN_E:
		fprintf_expression(file, expr->binary.lhs);
		fprintf(file, " ");
		TokenType type = binary_tag_to_token_type(expr->tag);
		fprintf_token_type(file, &type);
		fprintf(file, " ");
		fprintf_expression(file, expr->binary.rhs);
		break;
	case VARIABLE_E:
		fprintf(file, "%s", expr->variable.name);
		break;
	case NUMBER_E:
		if (expr->number.is_written_in_hexa) {
			fprintf(file, "0x%x", expr->number.value);
		} else {
			fprintf(file, "%d", expr->number.value);
		}
		break;
	case SEQUENCE_E:
		for (int i = 0; i < expr->sequence.len; i++) {
			fprintf_expression(file, &expr->sequence.list[i]);
			if (i != expr->sequence.len) {
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
		if (expr->char_literal.c == '\n') {
			fprintf(file, "'\\n'");
		} else {
			fprintf(file, "'%c'", expr->char_literal.c);
		}
		break;
	case STRING_LITERAL_E:
		fprintf(file, "\"%c\"", expr->char_literal.c);
		break;
	case IF_ELSE_E:
		fprintf(file, "if (");
		fprintf_expression(file, expr->if_else.cond);
		fprintf(file, ") {\n");
		fprintf_expression(file, expr->if_else.if_body);
		fprintf(file, "}");
		if (expr->if_else.else_body == NULL) {
			break;
		}
		fprintf(file, " else {\n");
		fprintf_expression(file, expr->if_else.else_body);
		fprintf(file, "\n}");
	}
}

void fprintf_function(FILE *file, Function *function) {
	fprintf(file, "fn %s (", function->name);
	for (int i = 0; i < function->args.len; i++) {
		fprintf(file, "%s :", function->args.args[i].name);
		fprintf_program_type(file, &function->args.args[i].type);
		if (i != function->args.len - 1) {
			fprintf(file, ",");
		}
	}
	fprintf(file, ") ");
	fprintf_program_type(file, &function->type);
	fprintf(file, " = {\n");
	fprintf_expression(file, function->expr);
	fprintf(file, "\n};");
}

// Input :
// - file name to write the tokens
// - Ast that is goinf to be written in the file
void fprintf_ast(FILE *file, Ast *ast) {
	for (int i = 0; i < ast->len; i++) {
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
