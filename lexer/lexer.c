#include "lexer.h"
#include "../utils/colors.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	FILE *file;
	int size_buff;
	int pos;
	char buff[100];
	uint16_t line;
	uint16_t column;
} Stream;

///// ----- TOKEN ----- /////

Token token_one_char(Stream *stream, TokenType token_type, char c) {
	Token token;
	token.line = stream->line;
	token.column = stream->column;
	token.type = token_type;
	token.text = (char *)malloc(2 * sizeof(char));
	token.text[0] = c;
	token.text[1] = 0;
	return token;
}

Token token_only_type(Stream *stream, TokenType token_type) {
	Token token;
	token.line = stream->line;
	token.column = stream->column;
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
	case EQUAL:
		fprintf(file, "=");
		break;
	case EQUALEQUAL:
		fprintf(file, "==");
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
char next_char(Stream *stream) {
	char c;
	if (stream->buff[stream->pos] == '\0') {
		if (fgets(stream->buff, stream->size_buff, stream->file) ==
		    NULL) {
			return '\0';
		} else {
			stream->pos = 1;
		}
	} else {
		stream->pos += 1;
	}
	c = stream->buff[stream->pos - 1];
	if (c == '\n') {
		stream->line += 1;
		stream->column = 0;
	} else {
		stream->column += 1;
	}
	return c;
}

Token next_char_literal(Stream *stream, char *c) {
	Token token;
	*c = next_char(stream);
	if (*c == '\'') {
		printf("don't accept empty char_literal");
	}
	if (*c == '\\') {
		*c = next_char(stream);
		if (*c == 'n') {
			token = token_one_char(stream, CHAR_LITERAL, '\n');
		}
	} else {
		token = token_one_char(stream, CHAR_LITERAL, *c);
	}
	*c = next_char(stream);
	if (*c != '\'') {
		printf("TODO make this a recoverable error\n");
	}
	return token;
}

Token next_string_literal(Stream *stream, char *c) {
	*c = next_char(stream);
	if (*c == '\"') {
		printf("don't accept empty string_literal");
	}
	Token token = token_one_char(stream, CHAR_LITERAL, *c);
	while (true) {
		*c = next_char(stream);
		if (*c == '\0') {
			printf("error missing end of string literal");
		}
		if (*c != '\"') {
			token = token_append_char(token, *c);
			continue;
		}
		*c = next_char(stream);
		break;
	}
	return token;
}

// Apply next_char until arriving on the end of the line
void next_comment(Stream *stream, char *c) {
	if (*c == '\n') {
		return;
	}
	while (true) {
		*c = next_char(stream);
		if (*c != '\n') {
			continue;
		}
		return;
	}
}

Token next_number(Stream *stream, char *c) {
	Token token = token_one_char(stream, NUMBER, *c);
	while (true) {
		*c = next_char(stream);
		if (isdigit(*c)) {
			token = token_append_char(token, *c);
			continue;
		}
		return token;
	}
	return token;
}

Token next_ident(Stream *stream, char *c) {
	Token token = token_one_char(stream, IDENTIFIER, *c);
	while (true) {
		*c = next_char(stream);
		if (isalpha(*c) || isdigit(*c) || *c == '_') {
			token = token_append_char(token, *c);
			continue;
		}
		if (strcmp("fn", token.text) == 0) {
			free(token.text);
			return token_only_type(stream, FN);
		} else if (strcmp("let", token.text) == 0) {
			free(token.text);
			return token_only_type(stream, LET);
		} else if (strcmp("return", token.text) == 0) {
			free(token.text);
			return token_only_type(stream, RETURN);
		} else if (strcmp("void", token.text) == 0) {
			free(token.text);
			return token_only_type(stream, VOID);
		} else if (strcmp("u8", token.text) == 0) {
			free(token.text);
			return token_only_type(stream, U8);
		} else if (strcmp("u16", token.text) == 0) {
			free(token.text);
			return token_only_type(stream, U16);
		}
		return token;
	}
}

// This function returns the next token of the stream and advance in the stream
Token next_token(Stream *stream, char *c, bool *empty_token) {
	Token token;
	if (isspace(*c)) {
		*c = next_char(stream);
		*empty_token = true;
		return token;
	}
	if (isdigit(*c)) {
		return next_number(stream, c);
	} else if (isalpha(*c) || *c == '_') {
		return next_ident(stream, c);
	}
	switch (*c) {
	case '/': {
		*c = next_char(stream);
		if (*c == '/') {
			next_comment(stream, c);
			*empty_token = true;
			return token;
		}
		return token_only_type(stream, DIVIDE);
	}
	case ':':
		token = token_only_type(stream, COLON);
		break;
	case ',':
		token = token_only_type(stream, COMMA);
		break;
	case ';':
		token = token_only_type(stream, SEMICOLON);
		break;
	case '{':
		token = token_only_type(stream, LBRACE);
		break;
	case '}':
		token = token_only_type(stream, RBRACE);
		break;
	case '[':
		token = token_only_type(stream, LBRACKET);
		break;
	case ']':
		token = token_only_type(stream, RBRACKET);
		break;
	case '(':
		token = token_only_type(stream, LPAREN);
		break;
	case ')':
		token = token_only_type(stream, RPAREN);
		break;
	case '=':
		*c = next_char(stream);
		if (*c == '=') {
			*c = next_char(stream);
			return token_only_type(stream, EQUALEQUAL);
		}
		return token_only_type(stream, EQUAL);
		break;
	case '+':
		token = token_only_type(stream, PLUS);
		break;
	case '-':
		token = token_only_type(stream, MINUS);
		break;
	case '*':
		token = token_only_type(stream, MULT);
		break;
	case '\'':
		token = next_char_literal(stream, c);
		break;
	case '\"':
		token = next_string_literal(stream, c);
		break;
	default:
		break;
	}

	*c = next_char(stream);
	return token;
}

// Input : file name to lexify
// Ouput : Result (list tokens) : this can be an error
Tokens *lexify(FILE *error, FILE *file) {
	Tokens *tokens = tokens_empty();

	bool empty_token = false;

	Stream stream;
	stream.line = 1;
	stream.column = 0;
	stream.pos = 0;
	stream.file = file;
	stream.size_buff = 100;
	stream.buff[0] = '\0';

	char c = next_char(&stream);
	while (true) {
		if (c == '\0') {
			break;
		}
		Token token = next_token(&stream, &c, &empty_token);
		if (empty_token) {
			empty_token = false;
			continue;
		}
		tokens_append_token(tokens, token);
	}
	return tokens;
}
