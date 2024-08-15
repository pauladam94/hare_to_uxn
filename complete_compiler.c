#include "compiler_to_uxn/compiler.h"
#include "utils/colors.h"
#include "utils/files.h"
#include <stdio.h>
#include <stdlib.h>

// This files compiles a file into a uxntal
// It dones way less error testing than `test.c`.

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Error: usage %s [..].ha [..].uxntal\n", argv[0]);
		return -1;
	}

	char *path_code = argv[1];
	char *path_uxntal = argv[2];

	FILE *file;

	// Lexer
	file = fopen(path_code, "r");
	if (file == NULL) {
		red();
		printf("[Cannot open '%s']\n", path_code);
		reset();
		return 0;
	};
	Tokens *tokens = lexify(stdout, file);
	if (tokens == NULL) {
		red();
		printf("[Error Lexer]\n");
		reset();
		return 0;
	}
	fclose(file);

	// Parser
	Ast *ast = parse(stdout, tokens);
	if (ast == NULL) {
		red();
		printf("[Parser Error]\n");
		reset();
		return 0;
	}

	// Compiler
	Program *uxn_program = compile_to_uxn(stdout, ast);
	if (uxn_program == NULL) {
		red();
		printf("[Compiler Error]\n");
		reset();
		return 0;
	}
	file = fopen(path_uxntal, "w");
	fprintf_uxn_program(file, uxn_program);
	fclose(file);

	green();
	printf("[Compilation Done]\n");
	reset();
	fflush(stdout);

	char command[200];
	sprintf(command, "uxnasm %s %s.rom", path_uxntal, path_uxntal);
	system(command);
	sprintf(command, "uxncli %s.rom", path_code);
	system(command);

	uxn_program_delete(uxn_program);
	return 0;
}
