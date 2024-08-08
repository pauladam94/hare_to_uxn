#include "compiler_utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	uint8_t length;
	ProgramType *types;
	char *names;
} VariableTypes;

typedef struct {
	uint16_t size;
	VariableTypes var_types;
} FunctionLayout;

typedef struct {
	uint8_t length;
	FunctionLayout *functions;
} ProgramLayout;

///// ----- Partial Uxn Program ----- /////

/// The partial Uxn Program is used when compiling the AST piece by piece.
/// At the end of the traversal of the AST, we should assemble all those partial
/// uxn program to form the final complete uxn program
typedef struct {
	uint16_t capacity;
	uint16_t length;
	char **comments;
	bool *is_instruction;
	UxnInstruction *instructions;
} UxnPartialProgram;

///// ----- PARTIAL PROGRAM ----- /////
UxnPartialProgram *empty_partial_program(void) {
	UxnPartialProgram *program = malloc(sizeof(*program));
	program->capacity = 0;
	program->length = 0;
	program->comments = NULL;
	program->is_instruction = NULL;
	program->instructions = NULL;
	return program;
}

/// Delete of a partial program
/// This does not delete comments strings because
void uxn_partial_program_delete(UxnPartialProgram *partial_program) {
	free(partial_program->comments);
	free(partial_program->is_instruction);
	free(partial_program->instructions);
	free(partial_program);
}

// Returns a partial uxn program that is a combination of p1 and p2
// Free p1 and p2 in the process
UxnPartialProgram *concat_program(UxnPartialProgram *p1,
				  UxnPartialProgram *p2) {
	UxnPartialProgram *res = malloc(sizeof(*res));

	res->length = p1->length + p2->length;
	res->capacity = res->length;

	res->comments = malloc(sizeof(char *) * res->capacity);
	res->is_instruction = malloc(sizeof(bool) * res->capacity);
	res->instructions = malloc(sizeof(UxnInstruction) * res->capacity);

	int i = 0;
	for (int j = 0; j < p1->length; j++) {
		res->comments[i] = p1->comments[j];
		res->is_instruction[i] = p1->is_instruction[j];
		res->instructions[i] = p1->instructions[j];
		i++;
	}
	for (int j = 0; j < p2->length; j++) {
		res->comments[i] = p2->comments[j];
		res->is_instruction[i] = p2->is_instruction[j];
		res->instructions[i] = p2->instructions[j];
		i++;
	}

	uxn_partial_program_delete(p1);
	uxn_partial_program_delete(p2);
	return res;
}

// add to the program 'p' (in place) one instruction which has
// instruction, is_intruction, comment to describe it
void append_instruction(UxnPartialProgram *p, char *comment,
			bool is_instruction, UxnInstruction instruction) {
	p->length++;
	if (p->capacity < p->length) {
		if (p->capacity == 0) {
			p->capacity = 2;
		} else {
			p->capacity *= 2;
		}
		char **prev_comments = p->comments;
		bool *prev_is_instruction = p->is_instruction;
		UxnInstruction *prev_instructions = p->instructions;

		p->comments = malloc(sizeof(char *) * p->capacity);
		p->is_instruction = malloc(sizeof(bool) * p->capacity);
		p->instructions = malloc(sizeof(UxnInstruction) * p->capacity);

		for (int i = 0; i < p->length - 1; i++) {
			p->comments[i] = prev_comments[i];
			p->is_instruction[i] = prev_is_instruction[i];
			p->instructions[i] = prev_instructions[i];
		}
		free(prev_comments);
		free(prev_is_instruction);
		free(prev_instructions);
	}

	p->comments[p->length - 1] = comment;
	p->is_instruction[p->length - 1] = is_instruction;
	p->instructions[p->length - 1] = instruction;
}

/// Write a partial function to an UxnProgram a a certain 'pos'
/// This functions delete PartialUxnProgram
void write_partial_program(FILE *error, UxnPartialProgram *partial_program,
			   UxnProgram *p, uint16_t pos) {
	for (uint16_t i = 0; i < partial_program->length; i++) {
		p->comments[pos + i] = partial_program->comments[i];
		p->is_written[pos + i] = true;
		p->is_instruction[pos + i] = partial_program->is_instruction[i];
		p->memory[pos + i] = partial_program->instructions[i];
	}
	uxn_partial_program_delete(partial_program);
}

///// ----- Uxn Program ----- /////

/// Completely free the program that is totally heap allocated
void uxn_program_delete(UxnProgram *program) {
	for (int i = 0; i < 0x10000; i++) {
		if (program->is_written[i]) {
			// if (program->comments[i] != NULL) {
			// 	free(program->comments[i]);
			// }
		}
	}
	free(program);
	return;
}

///// ----- Program Layout ----- /////

// uint8_t get_number_of_variable FunctionLayout
FunctionLayout compute_function_layout(Function *function);
ProgramLayout compute_program_layout(Ast *ast);

///// ----- COMPILE ----- /////
UxnPartialProgram *compile_expr(FILE *error, Expression *expr) {
	switch (expr->tag) {
	case LET_E: {
		fprintf(error, "let todo\n");
		break;
	}
	case ADD_E: {
		UxnPartialProgram *lhs = compile_expr(error, expr->add.lhs);
		if (lhs == NULL) {
			break;
		}
		UxnPartialProgram *rhs = compile_expr(error, expr->add.rhs);
		if (rhs == NULL) {
			uxn_partial_program_delete(lhs);
			break;
		}
		UxnPartialProgram *add = empty_partial_program();
		append_instruction(add, "Add expression", true, ADD);
		return concat_program(concat_program(lhs, rhs), add);
	}
	case SUB_E: {
		UxnPartialProgram *lhs = compile_expr(error, expr->add.lhs);
		if (lhs == NULL) {
			break;
		}
		UxnPartialProgram *rhs = compile_expr(error, expr->add.rhs);
		if (rhs == NULL) {
			uxn_partial_program_delete(lhs);
			break;
		}
		UxnPartialProgram *add = empty_partial_program();
		append_instruction(add, "Sub expression", true, ADD);
		return concat_program(concat_program(lhs, rhs), add);
	}
	case SEQUENCE_E: {
		UxnPartialProgram *sequence = empty_partial_program();
		for (int i = 0; i < expr->sequence.length; i++) {
			UxnPartialProgram *e =
			    compile_expr(error, &expr->sequence.list[i]);
			if (e == NULL) {
				fprintf(
				    error,
				    "Error compiling expression sequence\n");
				uxn_partial_program_delete(sequence);
				return NULL;
			}
			sequence = concat_program(sequence, e);
		}
		return sequence;
	}
	case ASSIGN_E: {
		fprintf(error, "assign todo\n");
		break;
	}
	case DEREF_ASSIGN_E: { // *e1 = e2
		UxnPartialProgram *e1 =
		    compile_expr(error, expr->deref_assign.e1);
		if (e1 == NULL) {
			break;
		}
		UxnPartialProgram *e2 =
		    compile_expr(error, expr->deref_assign.e2);
		if (e1 == NULL) {
			break;
		}
		append_instruction(e1, "Deref Assign", true, DEO);
		return concat_program(e2, e1);
	}
	case VARIABLE_E: {
		// have to know where variables are located
		fprintf(error, "variable todo\n");
		break;
	}
	case NUMBER_E: {
		UxnPartialProgram *number = empty_partial_program();
		if (expr->number.value < 255) {
			append_instruction(number, NULL, true, LIT);
		} else {
			append_instruction(number, NULL, true, LIT2);
		}
		append_instruction(number, NULL, false, expr->number.value);
		return number;
	}
	case RETURN_E: {
		fprintf(error, "return todo\n");
		break;
	}
	case FUNCTION_CALL_E: {
		fprintf(error, "function call todo\n");
		break;
	}
	case CHAR_LITERAL: {

		break;
	}
	}
	fprintf(error, "Error compiling expression\n");
	return NULL;
}

UxnPartialProgram *compile_function(FILE *error, Function *function) {
	// TODO handle arguments
	fprintf(error, "handle arguments and other");
	return compile_expr(error, function->expr);
}

UxnProgram *compile_to_uxn(Ast *ast, FILE *error) {
	// Initialize Program
	UxnProgram *program = malloc(sizeof(*program));
	for (int i = 0; i < 0x10000; i++) {
		program->is_written[i] = false;
		program->comments[i] = NULL;
		program->is_instruction[i] = false; // not useful
		program->memory[i] = BRK;	    // not useful
	}

	// Get the 'main' if there is one
	// If there is none return an error
	Function *main = NULL;
	for (int i = 0; i < ast->length; i++) {
		Function *f = &ast->functions[i];
		if (strcmp(f->name, "main") == 0) {
			main = f;
			break;
		}
	}
	if (main == NULL) {
		fprintf(error, "No main function in the file");
		program->memory[0x100] = BRK;
		program->is_written[0x100] = true;
		program->is_instruction[0x100] = true;

		// Force to do that
		// because 'asm' can't compile code with only zeros
		program->memory[0x101] = 42;
		program->is_written[0x101] = true;
		program->is_instruction[0x101] = false;
		ast_delete(ast);
		return program;
	}

	// 1. Get the size of all functions
	// 2. Decide where to place them (one on top of each other)

	// 3. Compile the different function
	UxnPartialProgram *partial_program = compile_function(error, main);

	if (partial_program == NULL) {
		ast_delete(ast);
		free(program);
		return NULL;
	}

	// 4. concatenate all the different function compiled

	// 5. Write in a complete uxn program
	write_partial_program(error, partial_program, program, 0x100);

	ast_delete(ast);
	return program;
}
