#include "lexer.h"
#include "../utils/colors.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	TOKEN_FINISHED,
	TOKEN_NOT_FINISHED,
} State;

///// ----- TOKEN ----- /////

Token token_empty(void) {
	Token result;
	result.line = 0;
	result.column = 0;
	result.type = 0;
	result.text = NULL;
	return result;
}

Token *token_set_one_char(Token *token, TokenType token_type, char c,
			  uint16_t line, uint16_t column) {
	token->line = line;
	token->column = column;
	token->type = token_type;
	token->text = (char *)malloc(2 * sizeof(char));
	token->text[0] = c;
	token->text[1] = 0;
	return token;
}

State token_append_char(Token *token, char c, uint16_t line, uint16_t column) {
	// Every return FINISH should set line_end and column_end

	// White spaces imply next token
	if (isspace(c)) {
		return TOKEN_FINISHED;
	}
	// char c cannot be a white space anymore

	// First character of a token
	if (token->text == NULL) {
		TokenType token_type = SYMBOL;
		if (isdigit(c)) {
			token_type = NUMBER;
		} else if (isalpha(c) || c == '_') {
			token_type = IDENTIFIER;
		}
		token_set_one_char(token, token_type, c, line, column);
		return TOKEN_NOT_FINISHED;
	}
	// token in now initialized with at least one character

	// Check if the character c ends the current token
	switch (token->type) {
	case IDENTIFIER: {
		if (!isalnum(c) && c != '_') {
			return TOKEN_FINISHED;
		}
		break;
	}
	case NUMBER: {
		if (!isdigit(c)) {
			return TOKEN_FINISHED;
		}
		break;
	}
	case SYMBOL: {
		if (isalnum(c) || c == '_') {
			return TOKEN_FINISHED;
		}
		switch (c) {
		case '{':
			return TOKEN_FINISHED;
		case '}':
			return TOKEN_FINISHED;
		case '(':
			return TOKEN_FINISHED;
		case ')':
			return TOKEN_FINISHED;
		case ':':
			return TOKEN_FINISHED;
		case ';':
			return TOKEN_FINISHED;
		case '\'':
			return TOKEN_FINISHED;
		case '\"':
			return TOKEN_FINISHED;
		case '\\':
			return TOKEN_FINISHED;
		default:
			break;
		}
		break;
	}
	default: {
		printf("\nNot reachable during lexing\n");
	}
	}

	uint8_t token_length = strlen(token->text);

	// store previous token text
	char *previous_text = token->text;

	// Allocate one more char
	token->text = (char *)malloc((token_length + 2) * sizeof(char));

	// Reinitialize value according to the previous toke text
	for (uint32_t i = 0; i < token_length; i++) {
		token->text[i] = previous_text[i];
	}

	free(previous_text);

	token->text[token_length] = c;
	token->text[token_length + 1] = 0;

	return TOKEN_NOT_FINISHED;
}

void fprintf_token_type(FILE *file, TokenType *token_type) {
	switch (*token_type) {
	case IDENTIFIER:
		fprintf(file, "Identifier");
		break;
	case NUMBER:
		fprintf(file, "Number");
		break;
	case SYMBOL:
		fprintf(file, "Symbol");
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
	case EQUAL:
		fprintf(file, "=");
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
	case SINGLE_QUOTE:
		fprintf(file, "'");
		break;
	}
}

void fprintf_token(FILE *file, Token *token) {
	switch (token->type) {
	case IDENTIFIER: {
		fprintf(file, "%s", token->text);
		break;
	}
	case NUMBER: {
		fprintf(file, "%s", token->text);
		break;
	}
	case SYMBOL: {
		fprintf(file, "%s", token->text);
		break;
	}
	default: {
		fprintf_token_type(file, &token->type);
		break;
	}
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

void tokens_adapt_array(Tokens *tokens) {
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

// Input :
// - tokens : struct that holds a list of tokens
// - token : should be a heap allocated token
// Procedure :
// - Add token to tokens
void tokens_append_token(Tokens *tokens, Token token) {
	tokens->length++;
	tokens_adapt_array(tokens);

	if (strcmp(":", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = COLON;
	} else if (strcmp(",", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = COMMA;
	} else if (strcmp(";", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = SEMICOLON;
	} else if (strcmp("{", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = LBRACE;
	} else if (strcmp("}", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = RBRACE;
	} else if (strcmp("[", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = LBRACKET;
	} else if (strcmp("]", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = RBRACKET;
	} else if (strcmp("(", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = LPAREN;
	} else if (strcmp(")", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = RPAREN;
	} else if (strcmp("void", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = VOID;
	} else if (strcmp("fn", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = FN;
	} else if (strcmp("let", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = LET;
	} else if (strcmp("return", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = RETURN;
	} else if (strcmp("=", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = EQUAL;
	} else if (strcmp("u8", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = U8;
	} else if (strcmp("u16", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = U16;
	} else if (strcmp("+", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = PLUS;
	} else if (strcmp("-", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = MINUS;
	} else if (strcmp("*", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = MULT;
	} else if (strcmp("/", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = DIVIDE;
	} else if (strcmp("'", token.text) == 0) {
		free(token.text);
		token.text = NULL;
		token.type = SINGLE_QUOTE;
	}

	tokens->tokens[tokens->length - 1] = token;
}

// TODO better : know if we have to free each indivual token or not.
void tokens_free_numbers(Tokens *tokens) {
	for (uint32_t i = 0; i < tokens->length; i++) {
		if (tokens->tokens[i].text != NULL &&
		    tokens->tokens[i].type == NUMBER) {
			free(tokens->tokens[i].text);
		}
	}
	if (tokens->tokens != NULL) {
		free(tokens->tokens);
		tokens->capacity = 0;
		tokens->length = 0;
		tokens->tokens = NULL;
	}
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

// return the next character of the stream file
// return '\0' if it is the end of the file
char next_char(FILE *file, char *buff, int size_buff, int *pos) {
	if (buff[*pos] == '\0') {
		if (fgets(buff, size_buff, file) == NULL) {
			return '\0';
		} else {
			*pos = 1;
			return buff[0];
		}
	} else {
		*pos += 1;
		return buff[*pos - 1];
	}
}

// Apply next_char until arriving on the end of the line
void comment(FILE *file, char *buff, int size_buff, int *pos, char c) {
	if (c == '\n') {
		return;
	}
	while (next_char(file, buff, size_buff, pos) != '\n') {
	}
}

void finish_token(Tokens *tokens, Token token) {}

// Input : file name to lexify
// Ouput : Result (list tokens) : this can be an error
Tokens *lexify(FILE *file) {
	Tokens *tokens = tokens_empty();
	Token token = token_empty();
	// Token * token = NULL;

	uint16_t line = 1;
	uint16_t column = 1;

	int size_buff = 100;
	char buff[size_buff];
	buff[0] = '\0';
	int pos = 0;

	while (true) {
		char c = next_char(file, buff, size_buff, &pos);
		switch (c) {
		case '\0':
			return tokens;
		case '\n':
			line++;
			column = 1;
			break;
		case '{':
			finish_token(tokens, token);

			break;
		}
		switch (token_append_char(&token, c, line, column)) {
		case TOKEN_FINISHED: {
			if (token.text != NULL) {
				if (strcmp(token.text, "//") == 0) {
					free(token.text);
					token.text = NULL;
					if (c != '\n') {
						line++;
					}
					comment(file, buff, size_buff, &pos, c);
					column = 1;
				} else {
					tokens_append_token(tokens, token);
				}
				token = token_empty();
				column++;
				token_append_char(&token, c, line, column);
				continue;
			} else {
				column++;
			}
			break;
		}
		case TOKEN_NOT_FINISHED: {
			column++;
			break;
		}
		}
	}
	return tokens;
}
