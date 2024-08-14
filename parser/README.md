# Parser
The parser should convert `Tokens` structure to an `Ast` structure.

If the `tokens` given does not respect the grammar of the Hare programming
language. Then the `parse` functions returns NULL and write errors in the
`error` stream.

# PARSE FUNCTIONS API

## Parse State

```C
typedef struct {
    FILE *error;    // stream to output errors
    Tokens *tokens;
    uint32_t index; // position in the tokens
    bool abort;     // have to stop the parsing or error
} ParseState;
```

## Functions

```C
// returns on the stack for simple parsing
// Error handling with `state->abort` only
[Name] parse_[name] (ParseState* state);

// return a pointer
// Error handling with 
Expression* parse_[name] (ParseState* state);
```

## Error handling
If an error occured :
- error message is written in the `state->error` `file`
- returns NULL if it couldn't parse [name].
- change `state->abort` to `false` to say that the parsing should stop.
- `state->index` should not change

## Success
If no error occured :
- `state->index` move forward to the next tokens
- the function allocates what it has parse and return it
