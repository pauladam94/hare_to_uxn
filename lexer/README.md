# Lexer
The lexer cut into pieces the file given.

This process gives us back an element of type `Tokens`.

## Possible errors:
Here are the 3 possible errors :
- If there is not `'` to finish a char_literal that begins with `'`.
- If a char_literal contains more than one character.
- If there is not `"` to finish a string_literal that begins with `"`.

In thoses cases, the `lexify` fonctions returns NULL and free anything that it
has allocated.
