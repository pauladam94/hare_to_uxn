#include "lexer.h"
#include "../utils/colors.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///// ----- TOKEN ----- /////

Token token_one_char(TokenType token_type, char c, uint16_t line,
		     uint16_t column) {
	Token token;
	token.line = line;
	token.column = column;
	token.type = token_type;
	token.text = (char *)malloc(2 * sizeof(char));
	token.text[0] = c;
	token.text[1] = 0;
	return token;
}

Token token_only_type(TokenType token_type, uint16_t line, uint16_t column) {
	Token token;
	token.line = line;
	token.column = column;
	token.type = token_type;
	token.text = NULL;
	return token;
}

Token token_append_char(Token token, char c) {
	uint8_t token_length = strlen(token.text);

	// store previous token text
	char *previous_text = token.text;

	// Allocate one more char
	token.text = (char *)malloc((token_length + 2) * sizeof(char));

	// Reinitialize value according to the previous toke text
	for (uint32_t i = 0; i < token_length; i++) {
		token.text[i] = previous_text[i];
	}

	free(previous_text);

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
	case SYMBOL:
		fprintf(file, "Symbol");
		break;
	case CHAR_LITERAL:
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
	case IDENTIFIER:
		fprintf(file, "%s", token->text);
		break;
	case NUMBER:
		fprintf(file, "%s", token->text);
		break;
	case SYMBOL:
		fprintf(file, "%s", token->text);
		break;
	case CHAR_LITERAL:
		fprintf(file, "'%s'", token->text);
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

// Add one token to the struct tokens
void tokens_append_token(Tokens *tokens, Token token) {
	tokens->length++;
	tokens_adapt_array(tokens);

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

///// ---- LEXING NEXT FUNCTIONS ----- /////

// return the next character of the stream file
// return '\0' if it is the end of the file
char next_char(FILE *file, char *buff, int size_buff, int *pos, uint16_t *line,
	       uint16_t *column) {
	char c;
	if (buff[*pos] == '\0') {
		if (fgets(buff, size_buff, file) == NULL) {
			return '\0';
		} else {
			*pos = 1;
		}
	} else {
		*pos += 1;
	}
	c = buff[*pos - 1];
	if (c == '\n') {
		*line += 1;
		*column = 0;
	} else {
		*column += 1;
	}
	return c;
}

Token next_char_literal(FILE *file, char *buff, int size_buff, int *pos,
			uint16_t *line, uint16_t *column, char *c) {
	*c = next_char(file, buff, size_buff, pos, line, column);
	Token token = token_one_char(CHAR_LITERAL, *c, *line, *column);
	*c = next_char(file, buff, size_buff, pos, line, column);
	if (*c != '\'') {
		printf("TODO make this a recoverable error\n");
	}
	return token;
}

// Apply next_char until arriving on the end of the line
void next_comment(FILE *file, char *buff, int size_buff, int *pos,
		  uint16_t *line, uint16_t *column, char *c) {
	if (*c == '\n') {
		return;
	}
	while (true) {
		*c = next_char(file, buff, size_buff, pos, line, column);
		if (*c != '\n') {
			continue;
		}
		return;
	}
}

Token next_number(FILE *file, char *buff, int size_buff, int *pos,
		  uint16_t *line, uint16_t *column, char *c) {
	Token token = token_one_char(NUMBER, *c, *line, *column);
	while (true) {
		*c = next_char(file, buff, size_buff, pos, line, column);
		if (isdigit(*c)) {
			token = token_append_char(token, *c);
			continue;
		}
		return token;
	}
	return token;
}

Token next_ident(FILE *file, char *buff, int size_buff, int *pos,
		 uint16_t *line, uint16_t *column, char *c) {
	Token token = token_one_char(IDENTIFIER, *c, *line, *column);
	while (true) {
		*c = next_char(file, buff, size_buff, pos, line, column);
		if (isalpha(*c) || isdigit(*c) || *c == '_') {
			token = token_append_char(token, *c);
			continue;
		}
		if (strcmp("fn", token.text) == 0) {
			free(token.text);
			return token_only_type(FN, *line, *column);
		} else if (strcmp("let", token.text) == 0) {
			free(token.text);
			return token_only_type(LET, *line, *column);
		} else if (strcmp("return", token.text) == 0) {
			free(token.text);
			return token_only_type(RETURN, *line, *column);
		} else if (strcmp("void", token.text) == 0) {
			free(token.text);
			return token_only_type(VOID, *line, *column);
		} else if (strcmp("u8", token.text) == 0) {
			free(token.text);
			return token_only_type(U8, *line, *column);
		} else if (strcmp("u16", token.text) == 0) {
			free(token.text);
			return token_only_type(U16, *line, *column);
		}
		return token;
	}
}

// This function returns the next token of the stream and advance in the stream
Token next_token(FILE *file, char *buff, int size_buff, int *pos,
		 uint16_t *line, uint16_t *column, char *c, bool *empty_token) {
	Token token;
	if (isspace(*c)) {
		*c = next_char(file, buff, size_buff, pos, line, column);
		*empty_token = true;
		return token;
	}
	if (isdigit(*c)) {
		return next_number(file, buff, size_buff, pos, line, column, c);
	} else if (isalpha(*c) || *c == '_') {
		return next_ident(file, buff, size_buff, pos, line, column, c);
	}
	switch (*c) {
	case '/': {
		*c = next_char(file, buff, size_buff, pos, line, column);
		if (*c == '/') {
			next_comment(file, buff, size_buff, pos, line, column,
				     c);
			*empty_token = true;
			return token;
		}
		return token_only_type(DIVIDE, *line, *column);
	}
	case ':':
		token = token_only_type(COLON, *line, *column);
		break;
	case ',':
		token = token_only_type(COMMA, *line, *column);
		break;
	case ';':
		token = token_only_type(SEMICOLON, *line, *column);
		break;
	case '{':
		token = token_only_type(LBRACE, *line, *column);
		break;
	case '}':
		token = token_only_type(RBRACE, *line, *column);
		break;
	case '[':
		token = token_only_type(LBRACKET, *line, *column);
		break;
	case ']':
		token = token_only_type(RBRACKET, *line, *column);
		break;
	case '(':
		token = token_only_type(LPAREN, *line, *column);
		break;
	case ')':
		token = token_only_type(RPAREN, *line, *column);
		break;
	case '=':
		token = token_only_type(EQUAL, *line, *column);
		break;
	case '+':
		token = token_only_type(PLUS, *line, *column);
		break;
	case '-':
		token = token_only_type(MINUS, *line, *column);
		break;
	case '*':
		token = token_only_type(MULT, *line, *column);
		break;
	case '\'':
		token = next_char_literal(file, buff, size_buff, pos, line,
					  column, c);
		break;
	default:
		break;
	}

	*c = next_char(file, buff, size_buff, pos, line, column);
	return token;
}

// Input : file name to lexify
// Ouput : Result (list tokens) : this can be an error
Tokens *lexify(FILE *file) {
	Tokens *tokens = tokens_empty();

	bool empty_token = false;

	uint16_t line = 1;
	uint16_t column = 0;

	int size_buff = 100;
	char buff[size_buff];
	buff[0] = '\0';
	int pos = 0;

	char *c = malloc(sizeof(char));
	*c = next_char(file, buff, size_buff, &pos, &line, &column);
	while (true) {
		if (*c == '\0') {
			break;
		}
		Token token = next_token(file, buff, size_buff, &pos, &line,
					 &column, c, &empty_token);
		if (empty_token) {
			empty_token = false;
			continue;
		}
		tokens_append_token(tokens, token);
	}
	free(c);
	return tokens;
}
