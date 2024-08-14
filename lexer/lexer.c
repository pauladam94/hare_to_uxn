#include "lexer.h"
#include "../utils/colors.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	FILE *error;
	FILE *file;
	int size_buff;
	int pos;
	char buff[100];
	uint16_t line;
	uint16_t column;
	bool failed;
	bool empty_token;
} LexerState;

///// ----- TOKEN ----- /////

Token token_one_char(LexerState *state, TokenType token_type, char c) {
	Token token;
	token.line = state->line;
	token.column = state->column;
	token.type = token_type;
	token.text = (char *)malloc(2 * sizeof(char));
	token.text[0] = c;
	token.text[1] = 0;
	return token;
}

Token token_only_type(LexerState *state, TokenType token_type) {
	Token token;
	token.line = state->line;
	token.column = state->column;
	token.type = token_type;
	token.text = NULL;
	return token;
}

Token token_append(Token token, char c) {
	uint8_t token_length = strlen(token.text);

	// store previous token text
	char *previous_text = token.text;

	// Allocate one more char
	// if (token_length == 2) { // TODO REMOVE
	// 	printf("%s %c\n", token.text, c);
	// }
	token.text = (char *)malloc((token_length + 2) * sizeof(char));

	// Reinitialize value according to the previous toke text
	for (uint32_t i = 0; i < token_length; i++) {
		token.text[i] = previous_text[i];
	}
	if (previous_text != NULL) {
		free(previous_text);
	}

	token.text[token_length] = c;
	token.text[token_length + 1] = 0;
	return token;
}

void fprintf_token_type(FILE *file, TokenType *token_type) {
	switch (*token_type) {
	case IDENTIFIER:
		fprintf(file, "Identifier");
		break;
	case NUMBER:
		fprintf(file, "Number");
		break;
	case CHAR_LITERAL:
		fprintf(file, "Character Literal");
		break;
	case STRING_LITERAL:
		fprintf(file, "String Literal");
		break;
	case COLON:
		fprintf(file, ":");
		break;
	case COMMA:
		fprintf(file, ",");
		break;
	case SEMICOLON:
		fprintf(file, ";");
		break;
	case LBRACE:
		fprintf(file, "{");
		break;
	case RBRACE:
		fprintf(file, "}");
		break;
	case LBRACKET:
		fprintf(file, "[");
		break;
	case RBRACKET:
		fprintf(file, "]");
		break;
	case LPAREN:
		fprintf(file, "(");
		break;
	case RPAREN:
		fprintf(file, ")");
		break;
	case VOID:
		fprintf(file, "void");
		break;
	case FN:
		fprintf(file, "fn");
		break;
	case LET:
		fprintf(file, "let");
		break;
	case RETURN:
		fprintf(file, "return");
		break;
	case IF:
		fprintf(file, "if");
		break;
	case ELSE:
		fprintf(file, "else");
		break;
	case FOR:
		fprintf(file, "for");
		break;
	case EQUAL:
		fprintf(file, "=");
		break;
	case EQUAL_EQUAL:
		fprintf(file, "==");
		break;
	case NOT_EQUAL:
		fprintf(file, "!=");
		break;
	case U8:
		fprintf(file, "u8");
		break;
	case U16:
		fprintf(file, "u16");
		break;
	case PLUS:
		fprintf(file, "+");
		break;
	case MINUS:
		fprintf(file, "-");
		break;
	case MULT:
		fprintf(file, "*");
		break;
	case DIVIDE:
		fprintf(file, "/");
		break;
	case LESS_THAN:
		fprintf(file, "<");
		break;
	case LESS_THAN_EQUAL:
		fprintf(file, "<=");
		break;
	case GREATER_THAN:
		fprintf(file, ">");
		break;
	case GREATER_THAN_EQUAL:
		fprintf(file, ">=");
		break;
	}
}

void fprintf_token(FILE *file, Token *token) {
	switch (token->type) {
	case IDENTIFIER:
		fprintf(file, "%s", token->text);
		break;
	case NUMBER:
		fprintf(file, "%s", token->text);
		break;
	case CHAR_LITERAL:
		if (token->text[0] == '\n') {
			fprintf(file, "'\\n'");
		} else {
			fprintf(file, "'%s'", token->text);
		}
		break;
	case STRING_LITERAL:
		fprintf(file, "\"%s\"", token->text);
		break;
	default:
		fprintf_token_type(file, &token->type);
		break;
	}
	// fprintf(file, "|l: %d|c: %d|t: ", token->line, token->column);
	// fprintf_token_type(file, &token->type);
}

///// ----- TOKENS ----- /////

Tokens *tokens_empty(void) {
	Tokens *result = malloc(sizeof(*result));
	result->capacity = 0;
	result->length = 0;
	result->tokens = NULL;
	return result;
}

void tokens_resize(Tokens *tokens) {
	if (tokens->capacity < tokens->length) {
		if (tokens->capacity == 0) {
			tokens->capacity = 1;
		} else {
			tokens->capacity *= 2;
		}
		// store previous tokens
		Token *previous_tokens = tokens->tokens;
		// Allocate to have space according to the capacity
		tokens->tokens =
		    (Token *)malloc(tokens->capacity * sizeof(Token));
		// Reinitialize value according to previous tokens
		for (uint32_t i = 0; i < tokens->length - 1; i++) {
			tokens->tokens[i] = previous_tokens[i];
		}
		free(previous_tokens);
	}
}

// Add one token to the struct tokens
void tokens_append(Tokens *tokens, Token token) {
	tokens->length++;
	tokens_resize(tokens);
	tokens->tokens[tokens->length - 1] = token;
}

void tokens_delete(Tokens *tokens) {
	for (uint32_t i = 0; i < tokens->length; i++) {
		if (tokens->tokens[i].text != NULL) {
			free(tokens->tokens[i].text);
		}
	}
	free(tokens->tokens);
	free(tokens);
}

void fprintf_tokens(FILE *file, Tokens *tokens) {
	for (uint32_t i = 0; i < tokens->length; i++) {
		Token *token = &tokens->tokens[i];
		fprintf_token(file, token);
		fprintf(file, "\n");
	}
}

///// ---- LEXING NEXT FUNCTIONS ----- /////

// return the next character of the state file
// return '\0' if it is the end of the file
char next_char(LexerState *state) {
	char c;
	if (state->buff[state->pos] == '\0') {
		if (fgets(state->buff, state->size_buff, state->file) == NULL) {
			return '\0';
		} else {
			state->pos = 1;
		}
	} else {
		state->pos += 1;
	}
	c = state->buff[state->pos - 1];
	if (c == '\n') {
		state->line += 1;
		state->column = 0;
	} else {
		state->column += 1;
	}
	return c;
}

Token next_char_literal(LexerState *state, char *c) {
	Token token;
	*c = next_char(state);
	if (*c == '\'') {
		state->failed = true;
		token.type = CHAR_LITERAL;
		return token;
	}
	if (*c == '\\') {
		*c = next_char(state);
		if (*c == 'n') {
			token = token_one_char(state, CHAR_LITERAL, '\n');
		} else {
			state->failed = true;
			token.type = CHAR_LITERAL;
			return token;
		}
	} else {
		token = token_one_char(state, CHAR_LITERAL, *c);
	}
	*c = next_char(state);
	if (*c != '\'') {
		printf("TODO make this a recoverable error\n");
	}
	return token;
}

Token next_string_literal(LexerState *state, char *c) {
	*c = next_char(state);
	if (*c == '\"') {
		printf("don't accept empty string_literal");
	}
	Token token = token_one_char(state, STRING_LITERAL, *c);
	while (true) {
		*c = next_char(state);
		if (*c == '\0') {
			fprintf(state->error,
				"error missing end of string literal");
			state->failed = true;
			return token;
		}
		if (*c != '\"') {
			token = token_append(token, *c);
			continue;
		}
		*c = next_char(state);
		break;
	}
	return token;
}

// Apply next_char until arriving on the end of the line
void next_comment(LexerState *state, char *c) {
	if (*c == '\n') {
		return;
	}
	while (true) {
		*c = next_char(state);
		if (*c != '\n') {
			continue;
		}
		return;
	}
}

Token next_number(LexerState *state, char *c) {
	Token token = token_one_char(state, NUMBER, *c);
	while (true) {
		*c = next_char(state);
		if (isdigit(*c)) {
			token = token_append(token, *c);
			continue;
		}
		return token;
	}
	return token;
}

Token next_ident(LexerState *state, char *c) {
	Token token = token_one_char(state, IDENTIFIER, *c);
	while (true) {
		*c = next_char(state);
		if (isalpha(*c) || isdigit(*c) || *c == '_') {
			token = token_append(token, *c);
			continue;
		}
		if (strcmp("fn", token.text) == 0) {
			free(token.text);
			return token_only_type(state, FN);
		} else if (strcmp("let", token.text) == 0) {
			free(token.text);
			return token_only_type(state, LET);
		} else if (strcmp("return", token.text) == 0) {
			free(token.text);
			return token_only_type(state, RETURN);
		} else if (strcmp("void", token.text) == 0) {
			free(token.text);
			return token_only_type(state, VOID);
		} else if (strcmp("if", token.text) == 0) {
			free(token.text);
			return token_only_type(state, IF);
		} else if (strcmp("else", token.text) == 0) {
			free(token.text);
			return token_only_type(state, ELSE);
		} else if (strcmp("for", token.text) == 0) {
			free(token.text);
			return token_only_type(state, FOR);
		} else if (strcmp("u8", token.text) == 0) {
			free(token.text);
			return token_only_type(state, U8);
		} else if (strcmp("u16", token.text) == 0) {
			free(token.text);
			return token_only_type(state, U16);
		}
		return token;
	}
}

// This function returns the next token of the state and advance in the state
Token next_token(LexerState *state, char *c) {
	Token token;
	token.type = VOID;
	if (isspace(*c)) {
		*c = next_char(state);
		state->empty_token = true;
		return token;
	}
	if (isdigit(*c)) {
		return next_number(state, c);
	} else if (isalpha(*c) || *c == '_') {
		return next_ident(state, c);
	}
	switch (*c) {
	case '/': {
		*c = next_char(state);
		if (*c == '/') {
			next_comment(state, c);
			state->empty_token = true;
			return token;
		}
		return token_only_type(state, DIVIDE);
	}
	case ':':
		token = token_only_type(state, COLON);
		break;
	case ',':
		token = token_only_type(state, COMMA);
		break;
	case ';':
		token = token_only_type(state, SEMICOLON);
		break;
	case '{':
		token = token_only_type(state, LBRACE);
		break;
	case '}':
		token = token_only_type(state, RBRACE);
		break;
	case '[':
		token = token_only_type(state, LBRACKET);
		break;
	case ']':
		token = token_only_type(state, RBRACKET);
		break;
	case '(':
		token = token_only_type(state, LPAREN);
		break;
	case ')':
		token = token_only_type(state, RPAREN);
		break;
	case '=':
		*c = next_char(state);
		if (*c == '=') {
			*c = next_char(state);
			return token_only_type(state, EQUAL_EQUAL);
		}
		return token_only_type(state, EQUAL);
	case '!':
		*c = next_char(state);
		if (*c == '=') {
			*c = next_char(state);
			return token_only_type(state, NOT_EQUAL);
		}
	case '+':
		token = token_only_type(state, PLUS);
		break;
	case '-':
		token = token_only_type(state, MINUS);
		break;
	case '*':
		token = token_only_type(state, MULT);
		break;
	case '>':
		*c = next_char(state);
		if (*c == '=') {
			*c = next_char(state);
			return token_only_type(state, GREATER_THAN_EQUAL);
		}
		return token_only_type(state, GREATER_THAN);
	case '<':
		*c = next_char(state);
		if (*c == '=') {
			*c = next_char(state);
			return token_only_type(state, LESS_THAN_EQUAL);
		}
		return token_only_type(state, LESS_THAN);
	case '\'':
		token = next_char_literal(state, c);
		break;
	case '\"':
		token = next_string_literal(state, c);
		break;
	default:
		break;
	}

	if (state->failed) {
		return token;
	}
	*c = next_char(state);
	return token;
}

// Input : file name to lexify
// Ouput : Result (list tokens) : this can be an error
Tokens *lexify(FILE *error, FILE *file) {
	Tokens *tokens = tokens_empty();

	LexerState state;
	state.error = error;
	state.line = 1;
	state.column = 0;
	state.pos = 0;
	state.file = file;
	state.size_buff = 100;
	state.buff[0] = '\0';
	state.failed = false;
	state.empty_token = false;

	char c = next_char(&state);
	while (true) {
		if (c == '\0') {
			break;
		}
		Token token = next_token(&state, &c);
		if (state.failed) {
			tokens_delete(tokens);
			return NULL;
		}

		if (state.empty_token) {
			state.empty_token = false;
			continue;
		}
		tokens_append(tokens, token);
	}
	return tokens;
}
