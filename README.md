# Compiler Hare to UXN
This project has pedagogical purpose.

The book associated to this project is [here](https://pauladam94.github.io/compiler_lua_uxn).

- Lua is a simple interpreted language
- UXN is a stack machine

## Trouble Shoutting
- make can be not recent enough for certain variable maybe

## Programs Needed (Dependencies)
- make : build system
- time : can be removed in the `Makefile`
- C compiler (clang or GCC ...)
    - same flags as gcc or clang, flags can be changed
    - the C compiler can be changed in `Makefile`

## Run Uxn code compiled
Need of this commands to be accessible :
- `uxnasm` a Uxn assembleur of uxn code generated
- `uxncli` a Uxn cli emulator for uxn assembly (for testing and cli application)
- `uxnemu` a Uxn graphical emulator for uxn assembly

### On the web:
- [Here is](https://metasyn.srht.site/learn-uxn/) a playground of the uxntal
language.

### On NixOs
Install the `uxn` package to have those 3 binaries accessible. The makefile uses
it.

### On Other Linux distribution
You can use `uxn/uxnasm`, `uxn/uxncli` and `uxn/uxnemu` present in this
repository. You either have to put them in our path so that `uxnasm` and
`uxncli` are accessible for file `test.c`. You can also change the syscall done
at the end of `test.c` durint the execution phase.

# Count the number of line of code
To check the number of C line code `git ls-files '*.c' '*.h' | xargs wc -l`.

# Memory Leak
- None in lexer, parser or compiler

# TODO
- weird error 220 missing ; at lign 0 column 0; (end of function);
better error handling parser
