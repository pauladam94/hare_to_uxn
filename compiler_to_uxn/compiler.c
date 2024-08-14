#include "compiler_utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///// ----- Variable Layout ----- /////
typedef struct {
	bool defined;
	uint8_t size;
	uint8_t addr;
} VariableInfo;

typedef struct {
	uint8_t capacity;
	uint8_t length;
	ProgramType *types;
	char **names;
} VariableLayout;

void var_layout_delete(VariableLayout vars) {
	free(vars.types);
	free(vars.names);
}

void var_layout_resize(VariableLayout *vars) {
	if (vars->capacity < vars->length) {
		if (vars->capacity == 0) {
			vars->capacity = 1;
		} else {
			vars->capacity *= 2;
		}
		char **prev_names = vars->names;
		ProgramType *prev_types = vars->types;
		vars->names = malloc(sizeof(*vars->names) * vars->capacity);
		vars->types = malloc(sizeof(*vars->types) * vars->capacity);
		for (int i = 0; i < vars->length - 1; i++) {
			vars->names[i] = prev_names[i];
			vars->types[i] = prev_types[i];
		}
		if (prev_names != NULL) {
			free(prev_names);
			free(prev_types);
		}
	}
}

void var_layout_append(VariableLayout *vars, ProgramType type, char *name) {
	vars->length++;
	var_layout_resize(vars);
	vars->names[vars->length - 1] = name;
	vars->types[vars->length - 1] = type;
}

// This gets the position of the last inserted variable with the name `name`
VariableInfo var_layout_get_addr(VariableLayout *vars, char *name) {
	uint8_t addr = 0;
	for (int i = 0; i < vars->length; i++) {
		VariableInfo var_info;
		if (vars->types[i] == U8_T) {
			var_info.size = 1;
		} else if (vars->types[i] == U16_T) {
			var_info.size = 2;
		}
		// The parser avoided to have variable with `void` type
		if (strcmp(name, vars->names[i]) == 0) {
			var_info.defined = true;
			var_info.addr = addr;
			return var_info;
		}
		addr += var_info.size;
	}
	VariableInfo info;
	info.defined = false;
	return info;
}

///// ----- PARTIAL PROGRAM ----- /////

/// The partial Uxn Program is used when compiling the AST piece by piece.
/// At the end of the traversal of the AST, we should assemble all those partial
/// uxn program to form the final complete uxn program
typedef struct {
	uint16_t capacity;
	uint16_t length;
	char **comments;
	bool *is_instruction;
	Instruction *instructions;
} PartProgram;

PartProgram *part_program_empty(void) {
	PartProgram *program = malloc(sizeof(*program));
	program->capacity = 0;
	program->length = 0;
	program->comments = NULL;
	program->is_instruction = NULL;
	program->instructions = NULL;
	return program;
}

void uxn_part_program_free(PartProgram part_program) {
	free(part_program.comments);
	free(part_program.is_instruction);
	free(part_program.instructions);
}

/// Delete of a partial program
/// This does not delete comments strings because they are string literal
/// This suppose that partial_program is allocated on the heap
void uxn_part_program_delete(PartProgram *part_program) {
	uxn_part_program_free(*part_program);
	free(part_program);
}

// Returns a partial uxn program that is a combination of p1 and p2
// Free p1 and p2 in the process
PartProgram *concat_program(PartProgram *p1, PartProgram *p2) {
	PartProgram *res = malloc(sizeof(*res));

	res->length = p1->length + p2->length;
	res->capacity = res->length;

	res->comments = malloc(sizeof(char *) * res->capacity);
	res->is_instruction = malloc(sizeof(bool) * res->capacity);
	res->instructions = malloc(sizeof(Instruction) * res->capacity);

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

	uxn_part_program_delete(p1);
	uxn_part_program_delete(p2);
	return res;
}

void part_program_resize(PartProgram *p) {
	if (p->capacity < p->length) {
		if (p->capacity == 0) {
			p->capacity = 2;
		} else {
			p->capacity *= 2;
		}
		char **prev_comments = p->comments;
		bool *prev_is_instruction = p->is_instruction;
		Instruction *prev_instructions = p->instructions;

		p->comments = malloc(sizeof(char *) * p->capacity);
		p->is_instruction = malloc(sizeof(bool) * p->capacity);
		p->instructions = malloc(sizeof(Instruction) * p->capacity);

		for (int i = 0; i < p->length - 1; i++) {
			p->comments[i] = prev_comments[i];
			p->is_instruction[i] = prev_is_instruction[i];
			p->instructions[i] = prev_instructions[i];
		}
		if (prev_comments != NULL) {
			free(prev_comments);
			free(prev_is_instruction);
			free(prev_instructions);
		}
	}
}

// add to the program 'p' (in place) one instruction which has
// instruction, is_intruction, comment to describe it
void append_number(PartProgram *p, char *comment, uint16_t n) {
	if (n < 0x1000) {
		p->length++;
		part_program_resize(p);

		p->comments[p->length - 1] = comment;
		p->is_instruction[p->length - 1] = false;

		p->instructions[p->length - 1] = n;
	} else {
		p->length += 2;
		part_program_resize(p);
		p->comments[p->length - 2] = comment;
		p->is_instruction[p->length - 2] = false;
		p->is_instruction[p->length - 1] = false;

		p->instructions[p->length - 2] = n;
		p->instructions[p->length - 1] = n;
	}
}

// add to the program 'p' (in place) one instruction which has
// instruction, is_intruction, comment to describe it
void append_instruction(PartProgram *p, char *comment, Instruction inst) {
	p->length++;
	part_program_resize(p);

	p->comments[p->length - 1] = comment;
	p->is_instruction[p->length - 1] = true;
	p->instructions[p->length - 1] = inst;
}

/// Write a partial function to an UxnProgram a a certain 'pos'
/// This functions delete PartialUxnProgram
void write_part_program(PartProgram part_program, Program *p, uint16_t pos) {
	for (uint16_t i = 0; i < part_program.length; i++) {
		p->comments[pos + i] = part_program.comments[i];
		p->is_written[pos + i] = true;
		p->is_instruction[pos + i] = part_program.is_instruction[i];
		p->memory[pos + i] = part_program.instructions[i];
	}
	uxn_part_program_free(part_program);
}

///// ----- Uxn Program ----- /////

/// Completely free the program that is totally heap allocated
void uxn_program_delete(Program *program) {
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

///// ----- COMPILE ----- /////
PartProgram *compile_expr(FILE *error, Expression *expr, VariableLayout *vars) {
	switch (expr->tag) {
	case LET_E: {
		// Compile the expression
		PartProgram *let = compile_expr(error, expr->let.e, vars);
		if (let == NULL) {
			fprintf(error, "Error compiling expr\n");
			return NULL;
		}

		// Add the variable to the variable list
		var_layout_append(vars, expr->let.type, expr->let.var);
		// Get the address of this new variable
		VariableInfo var_info =
		    var_layout_get_addr(vars, expr->let.var);

		// Right now we assume that the value is of size 8 bits
		append_instruction(let, NULL, LIT);
		append_number(let, NULL, var_info.addr);
		append_instruction(let, "let def", STZ);
		return let;
	}
	case ADD_E:
	case SUB_E:
	case MULT_E:
	case DIV_E:
	case EQUAL_EQUAL_E:
	case NOT_EQUAL_E:
	case GREATER_THAN_E:
	case GREATER_THAN_EQUAL_E:
	case LESS_THAN_E:
	case LESS_THAN_EQUAL_E: {
		PartProgram *lhs = compile_expr(error, expr->binary.lhs, vars);
		if (lhs == NULL) {
			break;
		}
		PartProgram *rhs = compile_expr(error, expr->binary.rhs, vars);
		if (rhs == NULL) {
			uxn_part_program_delete(lhs);
			break;
		}
		PartProgram *add = part_program_empty();
		append_instruction(add, "Add expression",
				   binary_tag_to_instruction(expr->tag));
		return concat_program(concat_program(lhs, rhs), add);
	}
	case SEQUENCE_E: {
		PartProgram *sequence = part_program_empty();
		for (int i = 0; i < expr->sequence.length; i++) {
			PartProgram *e =
			    compile_expr(error, &expr->sequence.list[i], vars);
			if (e == NULL) {
				fprintf(error, "Error compiling sequence\n");
				fprintf_expression(error,
						   &expr->sequence.list[i]);
				uxn_part_program_delete(sequence);
				return NULL;
			}
			sequence = concat_program(sequence, e);
		}
		return sequence;
	}
	case ASSIGN_E: {
		char *name = expr->assign.var;

		// Compile the expression
		PartProgram *assign = compile_expr(error, expr->assign.e, vars);
		if (assign == NULL) {
			fprintf(error, "Error compiling expr\n");
			return NULL;
		}

		// Get the address of this new variable
		VariableInfo var_info = var_layout_get_addr(vars, name);

		// Right now we assume that the value is of size 8 bits
		append_instruction(assign, NULL, LIT);
		append_number(assign, NULL, var_info.addr);
		append_instruction(assign, "assign def", STZ);
		return assign;
	}
	case DEREF_ASSIGN_E: { // *e1 = e2
		PartProgram *e1 =
		    compile_expr(error, expr->deref_assign.e1, vars);
		if (e1 == NULL) {
			break;
		}
		PartProgram *e2 =
		    compile_expr(error, expr->deref_assign.e2, vars);
		if (e2 == NULL) {
			break;
		}
		append_instruction(e1, "Deref Assign", DEO);
		return concat_program(e2, e1);
	}
	case DEREF_E: { // *e
		PartProgram *e = compile_expr(error, expr->deref.e, vars);
		if (e == NULL) {
			break;
		}
		append_instruction(e, "deref", DEO);
		// append_intruction();
		return e;
	}
	case VARIABLE_E: {
		char *name = expr->variable.name;
		PartProgram *var = part_program_empty();

		// Get the variable address
		VariableInfo var_info = var_layout_get_addr(vars, name);
		if (!var_info.defined) {
			fprintf(error, "var '%s' not defined", name);
			return NULL;
		}
		// Put the address on the stack
		append_instruction(var, NULL, LIT);
		append_number(var, NULL, var_info.addr);
		append_instruction(var, "Var", LDZ);
		return var;
	}
	case NUMBER_E: {
		PartProgram *number = part_program_empty();
		if (expr->number.value < 0x1000) {
			append_instruction(number, NULL, LIT);
		} else {
			append_instruction(number, NULL, LIT2);
		}
		append_number(number, NULL, expr->number.value);
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
	case CHAR_LITERAL_E: {
		PartProgram *number = part_program_empty();
		append_instruction(number, NULL, LIT);
		append_number(number, "char literal", expr->char_literal.c);
		return number;
	}
	case STRING_LITERAL_E: {
		fprintf(error, "string literal todo\n");
		break;
	}
	case IF_ELSE_E: {
		PartProgram *if_body =
		    compile_expr(error, expr->if_else.if_body, vars);
		PartProgram *else_body =
		    compile_expr(error, expr->if_else.else_body, vars);
		PartProgram *cond =
		    compile_expr(error, expr->if_else.cond, vars);
		fprintf(error, "if else todo\n");
		break;
	}
	}
	return NULL;
}

PartProgram compile_function(FILE *error, Function *function) {

	VariableLayout vars;
	vars.names = NULL;
	vars.types = NULL;
	vars.length = 0;
	vars.capacity = 0;

	PartProgram result;
	PartProgram *expr = compile_expr(error, function->expr, &vars);
	if (expr == NULL) {
		result.length = 0;
	} else {
		result = *expr;
		free(expr);
	}
	var_layout_delete(vars);
	return result;
}

Program *compile_to_uxn(FILE *error, Ast *ast) {
	// No functions => stop
	if (ast->length == 0) {
		ast_delete(ast);
		return NULL;
	}

	// TODO: two functions with the same name => stop

	// Get the 'main' if there is one else stop the compilation
	bool found_main = false;
	int index_main = 0;
	for (int i = 0; i < ast->length; i++) {
		if (strcmp(ast->functions[i].name, "main") == 0) {
			found_main = true;
			index_main = i;
			break;
		}
	}
	if (!found_main) {
		fprintf(error, "No main function in the file");
		ast_delete(ast);
		return NULL;
	}

	PartProgram *func_binary = malloc(sizeof(*func_binary) * ast->length);
	uint16_t *func_pos = malloc(sizeof(*func_pos) * ast->length);
	func_pos[index_main] = 0x100;

	// 1. Compile the different function
	for (int i = 0; i < ast->length; i++) {
		PartProgram func = compile_function(error, &ast->functions[i]);
		if (func.length == 0) {
			fprintf(error, "Error compiling function '%s'",
				ast->functions[i].name);
			ast_delete(ast);
			return NULL;
		}
		func_binary[i] = func;
	}

	// 2. Compute the positions of every functions
	uint16_t pos = 0x100;
	pos += func_binary[index_main].length;
	for (int i = 0; i < ast->length; i++) {
		if (i == index_main) {
			break;
		}
		pos += func_binary[i].length;
		func_pos[i] = pos;
	}

	// 3. Complete Address of functions in the partials programs
	// BIG TODO
	if (func_binary == NULL) {
		ast_delete(ast);
		return NULL;
	}

	// 4. Write all functions in the complete program
	// Initialize Program
	Program *program = malloc(sizeof(*program));
	for (int i = 0; i < 0x10000; i++) {
		program->is_written[i] = false;
		program->comments[i] = NULL;
		program->is_instruction[i] = false; // not useful
		program->memory[i] = BRK;	    // not useful
	}
	for (int i = 0; i < ast->length; i++) {
		write_part_program(func_binary[i], program, func_pos[i]);
	}

	free(func_binary);
	free(func_pos);
	ast_delete(ast);
	return program;
}
