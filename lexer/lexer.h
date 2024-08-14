#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
	IDENTIFIER, // a2 - ab_cd - x - ...
	NUMBER,	    // 234 - 1 - 0172 - ...
	CHAR_LITERAL,
	STRING_LITERAL,

	// SYMBOLS
	COLON,	   // :
	COMMA,	   // ,
	SEMICOLON, // ;
	LBRACE,	   // {
	RBRACE,	   // }
	LBRACKET,  // [
	RBRACKET,  // ]
	LPAREN,	   // (
	RPAREN,	   // )

	FN,	     // fn
	LET,	     // let
	EQUAL,	     // =
	EQUAL_EQUAL, // ==
	NOT_EQUAL,   // !=
	RETURN,	     // return
	IF,	     // if
	ELSE,	     // else
	FOR,	     // for

	VOID, // void
	U8,   // u8
	U16,  // u16

	PLUS,		    // +
	MINUS,		    // -
	MULT,		    // *
	DIVIDE,		    // /
	LESS_THAN,	    // <
	LESS_THAN_EQUAL,    // <=
	GREATER_THAN,	    // >
	GREATER_THAN_EQUAL, // >=
} TokenType;

typedef struct {
	char *text;
	uint16_t line;
	uint16_t column;
	TokenType type;
} Token;

typedef struct {
	Token *tokens;
	uint32_t capacity;
	uint32_t length;
} Tokens;

// Input : file name
// Ouput : list of tokens of the file name
// Note : This functions deletes all the comments
Tokens *lexify(FILE *error, FILE *file);

// Free the struct tokens but not any string allocated on the heap
// void tokens_free(Tokens *tokens);
void tokens_free_numbers(Tokens *tokens);
void tokens_delete(Tokens *tokens);

void fprintf_token_type(FILE *file, TokenType *token_type);
void fprintf_token(FILE *file, Token *token);

// Input :
// - file name to write the tokens
// - Struct Tokens that stores some token
void fprintf_tokens(FILE *file, Tokens *tokens);
