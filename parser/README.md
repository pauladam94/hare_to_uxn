# Parser
The parser should convert `Tokens` structure to an `Ast` structure.

If the `tokens` given does not respect the grammar of the Hare programming
language. Then the `parse` functions returns NULL and write errors in the
`error` stream.

PARSE FUNCTIONS API

bool parse_[name] (
FILE* error,
Tokens *tokens,
uint32_t *index,
[Name]* [name],
bool should_work
);

# return true
- we managed to parse [name] element from 'tokens' at the position 'index'.
- 'index' has increase according to the number of tokens read
- 'tokens' has not been mutated
- 'name' contains some sort of result from what has been parsed
- 'err' has not been mutated
- 'should_work' is not taken into account

# return false
- we did not managed to parse [name] element from 'tokens' at 'index'.
- 'index' is the value as before calling the function
- 'tokens' has not been mutated
- 'name' has not been changed from his state before the call
- 'should_work' : true
    - 'error' is mutated with an error saying we expected something specific
- 'should_work' : false
    - 'error' is not mutated

The should_work boolean means that the fonction should work or it's an
error. Inside a function, if you try to parse 'let', should_work = false
because other possibility can be parsed. 'let' is not the only possibility.


typedef {
    FILE* error;
    Tokens* tokens;
    uint32_t index;
    bool worked
    bool should_work;
    bool error_happened; // have to stop the parsing real error

} ParseState; // parse_state->worked

Expression parse_[name] (ParseState* parse_state);
