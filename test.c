#include "compiler_to_uxn/compiler.h"
#include "utils/colors.h"
#include "utils/files.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	/// There are 4 part of tests :
	/// - Lexer
	/// - Parser
	/// - Compiler
	/// - Executer
	/// Each one of them execute after the other and have severals steps.
	/// If any steps in any part of testing fail then the program stops.
	/// Errors are put as much as possible in the path_dir/error file.
	/// After each successful part a [Ok [insert name part]] is printed.

	if (argc != 2) {
		red();
		printf("[Error: %s takes 1 argument (a directory)]\n", argv[0]);
		reset();
		return -1;
	}

	char *path_dir = argv[1];
	blue();
	printf("[Test: %s]", path_dir);
	reset();
	fflush(stdout);

	char path_error[100];
	sprintf(path_error, "%s/error", path_dir);

	char path_code[100];
	char path_expected[100];
	char path_result[100];
	char path_2_result[100];
	FILE *file_code;
	FILE *file_expected;
	FILE *file_result;
	FILE *file_2_result;
	FILE *error;

	///// ----- LEXER TEST ----- /////
	/// 1. Try to lex path_dir/code, and write it to path_dir/lexer_result
	/// 2. Compare path_dir/lexer_result and path_dir/lexer_expected
	sprintf(path_code, "%s/main.ha", path_dir);
	sprintf(path_expected, "%s/lexer_expected", path_dir);
	sprintf(path_result, "%s/lexer_result", path_dir);

	file_code = fopen(path_code, "r");
	if (file_code == NULL) {
		red();
		printf("[Error during lexing : Cannot open '%s']\n", path_code);
		reset();
		return 0;
	};

	/// 1. Try to lex path_dir/code, and write it to path_dir/lexer_result
	error = fopen(path_error, "w");
	Tokens *tokens = lexify(error, file_code);
	if (tokens == NULL) {
		red();
		printf("[Error lexing %s]\n", path_code);
		reset();
		return 0;
	}
	fclose(file_code);
	fclose(error);

	file_result = fopen(path_result, "w");
	if (file_result == NULL) {
		red();
		printf("[Error opening '%s']\n", path_result);
		reset();
		// TODO tokens_free_complete(tokens); because else memory leaks
		return 0;
	}
	fprintf_tokens(file_result, tokens);
	fclose(file_result);

	/// 2. Compare path_dir/lexer_result and path_dir/lexer_expected
	file_expected = fopen(path_expected, "r");
	file_result = fopen(path_result, "r");
	if (file_expected == NULL) {
		red();
		printf("[Error opening '%s']\n", path_expected);
		reset();
		// TODO free tokens
		return 0;
	}
	if (file_result == NULL) {
		red();
		printf("[Error opening '%s']\n", path_result);
		reset();
		// TODO free tokens
		return 0;
	}
	if (!files_equal(file_expected, file_result)) {
		red();
		printf("[Error: lexing result not the one expected]\n");
		reset();
		return 0;
	}
	fclose(file_expected);
	fclose(file_result);

	green();
	printf("[OK lexer]");
	reset();
	fflush(stdout);

	///// ----- PARSER TEST ----- /////
	/// 1. Try to parse the list of tokens (in path_dir/lexer_result)
	/// 2. Print the Ast in path_dir/parser_result
	/// 3. Parse and lex the file path_dir/parser_result
	/// 4. Lex path_dir/parser_result2 and compare with previous results

	sprintf(path_result, "%s/parser_result", path_dir);
	sprintf(path_2_result, "%s/parser_2_result", path_dir);

	/// 1. Try to parse the list of tokens (in path_dir/lexer_result)
	error = fopen(path_error, "w");
	Ast *ast = parse(error, tokens);
	fclose(error);

	if (ast == NULL) {
		red();
		printf("[Error parsing]\n");
		reset();
		return 0;
	}

	/// 2. Print the Ast in path_dir/parser_result
	file_result = fopen(path_result, "w");
	fprintf_ast(file_result, ast);
	fclose(file_result);

	/// 3. Parse and lex the file path_dir/parser_result
	error = fopen(path_error, "w");
	file_code = fopen(path_result, "r");
	Tokens *tokens_2 = lexify(error, file_code);
	if (tokens_2 == NULL) {
		red();
		printf("[Error lexing %s]\n", path_result);
		reset();
		return 0;
	}
	fclose(file_code);
	fclose(error);

	error = fopen(path_error, "w");
	Ast *ast_2 = parse(error, tokens_2);
	fclose(error);

	if (ast_2 == NULL) {
		red();
		printf(
		    "[Error parsing %s]\n", path_result);
		reset();
		return 0;
	}

	// writing the second result of parsing
	file_result = fopen(path_2_result, "w");
	fprintf_ast(file_result, ast_2);
	ast_delete(ast_2);
	fclose(file_result);

	// We check that we obtain the same result twice
	file_result = fopen(path_result, "r");
	file_2_result = fopen(path_2_result, "r");
	if (!files_equal(file_result, file_2_result)) {
		red();
		printf("[Error: parsing again does not give the same output "
		       "as the first parse]\n");
		reset();
		return 0;
	}
	fclose(file_result);
	fclose(file_2_result);

	/// 4. Lex path_dir/parser_result2 and compare with previous results
	sprintf(path_expected, "%s/lexer_expected", path_dir);
	sprintf(path_result, "%s/lexer_2_result", path_dir);

	file_result = fopen(path_result, "w");
	file_2_result = fopen(path_2_result, "r");

	error = fopen(path_error, "w");
	tokens = lexify(error, file_2_result);
	if (tokens == NULL) {
		red();
		printf("[Error lexing the second parser output]\n");
		reset();
		return 0;
	}
	fclose(error);

	fprintf_tokens(file_result, tokens);
	tokens_delete(tokens);
	fclose(file_result);
	fclose(file_2_result);

	file_result = fopen(path_result, "r");
	file_expected = fopen(path_expected, "r");
	if (!files_equal(file_result, file_expected)) {
		red();
		printf("[Error: Lexing again the parsing output not the same "
		       "as the first lexing]\n");
		reset();
		return 0;
	}
	fclose(file_result);
	fclose(file_expected);

	green();
	printf("[OK parser]");
	reset();
	fflush(stdout);

	// TODO remove // just to test parse memory leak
	// ast_delete(ast);
	// printf("\n");
	// return 0;

	///// ----- COMPILER TEST ----- /////
	/// 1. Try to compile
	sprintf(path_result, "%s/code.uxntal", path_dir);

	error = fopen(path_error, "w");
	UxnProgram *uxn_program = compile_to_uxn(ast, error);
	fclose(error);

	if (uxn_program == NULL) {
		red();
		printf("[Error during compiling]\n");
		reset();
		return 0;
	}
	file_result = fopen(path_result, "w");
	fprintf_uxn_program(file_result, uxn_program);
	fclose(file_result);

	green();
	printf("[OK compiler]");
	reset();
	fflush(stdout);

	///// ----- EXECUTION TEST ----- /////
	/// 1. Assembling the path_dir/code.uxntal file in path_dir/code.rom
	/// 2. Executing path_dir/code.rom, put output in path_dir/output_result
	/// 3. Compare file path_dir/output_expected and path_dir/output_result

	char command[200];

	/// 1. Assembling the path_dir/code.uxntal file in path_dir/code.rom
	sprintf(command, "uxnasm %s/code.uxntal %s/code.rom > %s/uxnasm_result",
		path_dir, path_dir, path_dir);
	system(command);

	/// 2. Executing path_dir/code.rom, put output in path_dir/output_result
	sprintf(command, "uxncli %s/code.rom > %s/output_result", path_dir,
		path_dir);
	system(command);

	/// 3. Compare file path_dir/output_expected and path_dir/output_result
	sprintf(path_result, "%s/output_result", path_dir);
	sprintf(path_expected, "%s/output_expected", path_dir);

	file_result = fopen(path_result, "r");
	if (file_result == NULL) {
		red();
		printf("[Error: opening %s]\n", path_result);
		reset();
		return 0;
	}
	file_expected = fopen(path_expected, "r");
	if (file_expected == NULL) {
		red();
		printf("[Error: opening %s]\n", path_expected);
		reset();
		return 0;
	}
	if (!files_equal(file_result, file_expected)) {
		red();
		printf("[Error: Output is not the one expected]\n");
		reset();
		return 0;
	}
	fclose(file_result);
	fclose(file_expected);

	green();
	printf("[OK execution]\n");
	reset();
	fflush(stdout);

	uxn_program_delete(uxn_program);
	return 0;
}
