# Lexer
The lexer cut into pieces the file given.

This process gives us back an element of type `Tokens`.
`Tokens` is a vector of `Token`.

## Lexer State

```C
typedef struct {
    FILE *error;      // where error message should be written
    FILE *file;       // file that is being lexed
    int size_buff;    // size off buffer contain
    int pos;          // position inside the buffer
    char buff[100];   // buff that contains the line of `error`
    uint16_t line;    // current line in `file`
    uint16_t column;  // current column in `file`
    bool failed;      // true if lexinng failed
    bool empty_token; // true if empty token is parse (mostly white space)
} LexerState;
```

## Error handling
Here are the 3 possible errors :
- If there is not `'` to finish a char_literal that begins with `'`.
- If a char_literal contains more than one character.
- If there is not `"` to finish a string_literal that begins with `"`.

In thoses cases, the `lexify` fonctions returns NULL and free anything that it
has allocated.
