# Compiler

## Compiler State

```C
typedef struct {
    FILE* error;
} CompilerState;
```

## Compile Steps

1. Get the main function
2. Compile all functions (without the address of other functions)
3. 
4. Concatenate all the functions code
5. Write all the functions to the program
