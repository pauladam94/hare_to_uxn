#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

ProgramType parse_program_type(ParseState *state, bool required) {
	if (current_token(state).type == VOID) {
		state->index++;
		return VOID_T;
	} else if (current_token(state).type == U8) {
		state->index++;
		return U8_T;
	} else if (current_token(state).type == U16) {
		state->index++;
		return U16_T;
	}

	if (required) {
		fprintf_line_column(state);
		fprintf(state->error, "Expected type but got ");
		fprintf_current_token(state);
		fprintf(state->error, "\n");
	}

	ProgramType *type = malloc(sizeof(*type));
	*type = U16_T;
	state->abort = true;
	return VOID_T;
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
	if (current_token(state).type == NUMBER &&
	    strlen(current_token(state).text) == 1 &&
	    state->tokens->tokens[state->index].text[0] == '0') {
		char *prev_text = current_token(state).text;
		state->index++;
		int i;
		Expression *e = malloc(sizeof(*e));
		// TODO check more things if this really a number

		if (current_token(state).type == IDENTIFIER &&
		    current_token(state).text[0] == 'x' &&
		    sscanf(current_token(state).text + 1, "%x", &i) != EOF

		) {
			free(prev_text);
			free(current_token(state).text);
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
		free(current_token(state).text);
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

// Expression *parse_expr_add_sub(ParseState *state, bool required);
Expression *parse_binary_expr(ParseState *state, bool required);
Expression *parse_expr(ParseState *state);

Expression *parse_if_else(ParseState *state) {
	if (!parse_token_type(state, IF, false)) {
		return NULL;
	}
	if (!parse_token_type(state, LPAREN, true)) {
		state->abort = true;
		return NULL;
	}
	Expression *cond = parse_binary_expr(state, true);
	if (cond == NULL) {
		state->abort = true;
		return NULL;
	}
	if (!parse_token_type(state, RPAREN, true)) {
		state->abort = true;
		return NULL;
	}
	Expression *if_body = parse_expr(state);
	if (if_body == NULL) {
		state->abort = true;
		return NULL;
	}
	if (parse_token_type(state, ELSE, false)) {
		Expression *else_body = parse_expr(state);
		if (else_body == NULL) {
			state->abort = true;
			return NULL;
		}
		Expression *expr = malloc(sizeof(*expr));
		expr->tag = IF_ELSE_E;
		expr->if_else.if_body = if_body;
		expr->if_else.cond = cond;
		expr->if_else.else_body = else_body;
		return expr;
	}
	Expression *expr = malloc(sizeof(*expr));
	expr->tag = IF_ELSE_E;
	expr->if_else.if_body = if_body;
	expr->if_else.cond = cond;
	expr->if_else.else_body = NULL;

	return expr;
}
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

	ProgramType type = parse_program_type(state, true);
	if (state->abort || !parse_token_type(state, EQUAL, true)) {
		state->abort = true;
		return NULL;
	}
	Expression *e = parse_binary_expr(state, true);
	if (state->abort || e == NULL) {
		state->abort = true;
		return NULL;
	}
	Expression *expr = malloc(sizeof(*expr));
	expr->tag = LET_E;
	expr->let.e = e;
	expr->let.type = type;
	expr->let.var = var;
	return expr;
}

Expression *parse_deref_assign_or_deref(ParseState *state) {
	if (!parse_token_type(state, MULT, false)) {
		state->abort = false;
		return NULL;
	}

	Expression *e1 = parse_binary_expr(state, true);
	if (state->abort || e1 == NULL) {
		state->abort = true;
		return NULL;
	}

	if (e1->tag == ASSIGN_E) {
		Expression *variable = malloc(sizeof(*variable));
		variable->tag = VARIABLE_E;
		variable->variable.name = e1->assign.var;

		Expression *save_expr = e1->assign.e;

		e1->tag = DEREF_ASSIGN_E;
		e1->deref_assign.e2 = save_expr;
		e1->deref_assign.e1 = variable;
		return e1;
	}

	if (!parse_token_type(state, EQUAL, false)) {
		Expression *expr = malloc(sizeof(*expr));
		expr->tag = DEREF_E;
		expr->deref.e = e1;
		return expr;
	}
	Expression *e2 = parse_binary_expr(state, true);
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
	Expression *e = parse_binary_expr(state, true);
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
		Expression *e = parse_binary_expr(state, true);
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

Expression *parse_unary_expr(ParseState *state) {
	Expression *e = NULL;
	Expression *(*parse_unary[7])(ParseState *) = {
	    parse_char_literal, parse_let,
	    parse_if_else,	parse_deref_assign_or_deref,
	    parse_return,	parse_func_call_or_var_assign_or_var,
	    parse_number,
	};
	for (int i = 0; i < 7; i++) {
		e = parse_unary[i](state);
		if (state->abort) {
			return NULL;
		}
		if (e != NULL) {
			return e;
		}
	}
	return NULL;
}

Expression *parse_binary_expr(ParseState *state, bool required) {
	Expression *expr = parse_unary_expr(state);
	if (state->abort || expr == NULL) {
		if (required) {
			fprintf_line_column(state);
	 			fprintf(state->error, "Expected an Expression1 \n");
		}
		return NULL;
	}
	while (true) {
		TokenType type = VOID;
		TokenType bin_op[10] = {PLUS,	      MULT,
					MINUS,	      DIVIDE,
					EQUAL_EQUAL,  NOT_EQUAL,
					LESS_THAN,    LESS_THAN_EQUAL,
					GREATER_THAN, GREATER_THAN_EQUAL};
		for (int i = 0; i < 10; i++) {
			if (parse_token_type(state, bin_op[i], false)) {
				type = bin_op[i];
				break;
			}
		}
		if (type == VOID) {
			break;
		}
		ExpressionType next_tag = token_type_to_binary_tag(type);
		Expression *rhs = parse_unary_expr(state);
		if (state->abort || rhs == NULL) {
			state->abort = true;
			expression_delete(expr, true);
			return NULL;
		}
		if (precedence(expr->tag) <= precedence(next_tag)) {
			Expression *lhs = expr;
			expr = (Expression *)malloc(sizeof(*expr));
			expr->tag = next_tag;
			expr->binary.lhs = lhs;
			expr->binary.rhs = rhs;
		} else {
			Expression *lhs = (Expression *)malloc(sizeof(*expr));
			lhs->tag = next_tag;
			lhs->binary.lhs = expr->binary.rhs;
			lhs->binary.rhs = rhs;
			expr->binary.rhs = lhs;
		}
	}
	return expr;
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
		Expression *expr_next = parse_binary_expr(state, required);
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

Args parse_args(ParseState *state) {
	Args args;
	args.length = 0;
	args.args = NULL;
	while (true) {
		Arg arg;
		bool required = true;
		if (args.args == NULL) {
			required = false;
		}
		arg.name = parse_identifier(state, required);
		if (arg.name == NULL) {
			break;
		}

		if (!parse_token_type(state, COLON, true)) {
			state->abort = true;
			return args;
		}

		ProgramType arg_type = parse_program_type(state, true);
		if (state->abort) {
			return args;
		}

		args.length++;
		Arg *prev_args = args.args;
		args.args = malloc(sizeof(Arg) * args.length);
		for (int i = 0; i < args.length - 1; i++) {
			args.args[i] = prev_args[i];
		}
		if (prev_args != NULL) {
			free(prev_args);
		}

		arg.type = arg_type;
		args.args[args.length - 1] = arg;

		if (!parse_token_type(state, COMMA, false)) {
			return args;
		}
	}
	return args;
}

// fn indentifier () type = expression
Function *parse_function(ParseState *state, bool required) {
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

	Args args = parse_args(state);
	if (state->abort) {
		state->abort = required;
		return NULL;
	}

	if (!parse_token_type(state, RPAREN, true)) {
		state->abort = required;
		return NULL;
	}

	ProgramType type = parse_program_type(state, true);
	if (state->abort) {
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
	function->type = type;
	function->args = args;
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
			ast_append(ast, *function);
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

	// tokens_free_numbers(tokens);
	// tokens_free_numbers(tokens);
	free(tokens->tokens);
	free(tokens);
	return ast;
}
