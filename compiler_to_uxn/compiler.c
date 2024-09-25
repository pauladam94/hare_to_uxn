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
	uint8_t cap;
	uint8_t len;
	ProgramType *types;
	char **names;
} VariableLayout;

typedef struct {
	FILE *error;
	VariableLayout vars;
} CompilerState;

void var_layout_delete(VariableLayout vars) {
	free(vars.types);
	free(vars.names);
}

void var_layout_resize(VariableLayout *vars) {
	if (vars->cap < vars->len) {
		if (vars->cap == 0) {
			vars->cap = 1;
		} else {
			vars->cap *= 2;
		}
		char **prev_names = vars->names;
		ProgramType *prev_types = vars->types;
		vars->names = malloc(sizeof(*vars->names) * vars->cap);
		vars->types = malloc(sizeof(*vars->types) * vars->cap);
		for (int i = 0; i < vars->len - 1; i++) {
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
	vars->len++;
	var_layout_resize(vars);
	vars->names[vars->len - 1] = name;
	vars->types[vars->len - 1] = type;
}

// This gets the position of the last inserted variable with the name `name`
VariableInfo var_layout_get_addr(VariableLayout *vars, char *name) {
	uint8_t addr = 0;
	for (int i = 0; i < vars->len; i++) {
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

typedef struct {
	uint16_t cap;
	uint16_t len;
	uint16_t *pos; // positions inside of the PartProgram
	char **names;
} FunAddr;

/// The partial Uxn Program is used when compiling the AST piece by piece.
/// At the end of the traversal of the AST, we should assemble all those partial
/// uxn program to form the final complete uxn program
typedef struct {
	uint16_t cap;
	uint16_t len;
	char **comments;
	bool *is_inst;
	Instruction *inst;
	FunAddr fun_addr;
} PartProgram;

PartProgram *part_program_empty(void) {
	PartProgram *program = malloc(sizeof(*program));
	program->cap = 0;
	program->len = 0;
	program->comments = NULL;
	program->is_inst = NULL;
	program->inst = NULL;

	FunAddr fun_addr;
	fun_addr.len = 0;
	fun_addr.cap = 0;
	fun_addr.pos = NULL;
	fun_addr.names = NULL;

	program->fun_addr = fun_addr;
	return program;
}

void part_program_free(PartProgram p) {
	// if (p.fun_addr.pos != NULL) {
	// 	free(p.fun_addr.pos);
	// 	free(p.fun_addr.names);
	// }
	free(p.comments);
	free(p.is_inst);
	free(p.inst);
}

/// Delete of a partial program
/// This does not delete comments strings because they are string literal
/// This suppose that partial_program is allocated on the heap
void part_program_delete(PartProgram *p) {
	part_program_free(*p);
	free(p);
}

// Returns a partial uxn program that is a combination of p1 and p2
// Free p1 and p2 in the process
PartProgram *concat_program(PartProgram *p1, PartProgram *p2) {
	PartProgram *res = malloc(sizeof(*res));

	res->len = p1->len + p2->len;
	res->cap = res->len;

	res->comments = malloc(sizeof(*res->comments) * res->cap);
	res->is_inst = malloc(sizeof(*res->is_inst) * res->cap);
	res->inst = malloc(sizeof(*res->inst) * res->cap);

	// concat memory
	int i = 0;
	for (int j = 0; j < p1->len; j++) {
		res->comments[i] = p1->comments[j];
		res->is_inst[i] = p1->is_inst[j];
		res->inst[i] = p1->inst[j];
		i++;
	}
	for (int j = 0; j < p2->len; j++) {
		res->comments[i] = p2->comments[j];
		res->is_inst[i] = p2->is_inst[j];
		res->inst[i] = p2->inst[j];
		i++;
	}

	// concat fun_wait
	res->fun_addr.len = p1->fun_addr.len + p2->fun_addr.len;
	res->fun_addr.cap = res->fun_addr.len;
	if (res->fun_addr.cap != 0) {
		res->fun_addr.pos =
		    malloc(sizeof(*res->fun_addr.pos) * res->fun_addr.cap);
		res->fun_addr.pos =
		    malloc(sizeof(*res->fun_addr.pos) * res->fun_addr.cap);
	}
	i = 0;
	for (int j = 0; j < p1->fun_addr.len - 1; j++) {
		res->fun_addr.pos[i] = p1->fun_addr.pos[j];
		res->fun_addr.names[i] = p1->fun_addr.names[j];
		i++;
	}
	for (int j = 0; j < p2->fun_addr.len - 1; j++) {
		// offset of p1->len because fun_addr are positional
		res->fun_addr.pos[i] = p2->fun_addr.pos[j] + p1->len;
		res->fun_addr.names[i] = p2->fun_addr.names[j];
		i++;
	}

	part_program_delete(p1);
	part_program_delete(p2);
	return res;
}

void part_program_resize(PartProgram *p) {
	if (p->cap < p->len) {
		if (p->cap == 0) {
			p->cap = 2;
		} else {
			p->cap *= 2;
		}
		char **prev_comments = p->comments;
		bool *prev_is_instruction = p->is_inst;
		Instruction *prev_instructions = p->inst;

		p->comments = malloc(sizeof(*p->comments) * p->cap);
		p->is_inst = malloc(sizeof(*p->inst) * p->cap);
		p->inst = malloc(sizeof(*p->inst) * p->cap);

		for (int i = 0; i < p->len - 1; i++) {
			p->comments[i] = prev_comments[i];
			p->is_inst[i] = prev_is_instruction[i];
			p->inst[i] = prev_instructions[i];
		}
		if (prev_comments != NULL) {
			free(prev_comments);
			free(prev_is_instruction);
			free(prev_instructions);
		}
	}
}

void fun_wait_addr_resize(FunAddr *fun_wait) {
	if (fun_wait->cap < fun_wait->len) {
		if (fun_wait->cap == 0) {
			fun_wait->cap = 1;
		} else {
			fun_wait->cap *= 2;
		}
		char **prev_names = fun_wait->names;
		uint16_t *prev_positions = fun_wait->pos;

		fun_wait->names = malloc(sizeof(char *) * fun_wait->cap);
		fun_wait->pos = malloc(sizeof(uint16_t) * fun_wait->cap);

		for (int i = 0; i < fun_wait->len - 1; i++) {
			fun_wait->names[i] = prev_names[i];
			fun_wait->pos[i] = prev_positions[i];
		}
		if (prev_names != NULL) {
			free(prev_names);
			free(prev_positions);
		}
	}
}

void append_function_addr(PartProgram *p, char *name) {
	p->fun_addr.len++;
	fun_wait_addr_resize(&p->fun_addr);
	p->fun_addr.names[p->fun_addr.len - 1] = name;
	p->fun_addr.pos[p->fun_addr.len - 1] = p->fun_addr.len;
}

// add to the program 'p' (in place) one instruction which has
// instruction, is_intruction, comment to describe it
void append_number(PartProgram *p, char *comment, uint16_t n) {
	if (n < 0x1000) {
		p->len++;
		part_program_resize(p);

		p->comments[p->len - 1] = comment;
		p->is_inst[p->len - 1] = false;

		p->inst[p->len - 1] = n;
	} else {
		// TODO the right calculus
		p->len += 2;
		part_program_resize(p);
		p->comments[p->len - 2] = comment;
		p->is_inst[p->len - 2] = false;
		p->is_inst[p->len - 1] = false;

		p->inst[p->len - 2] = n % 0x1000;
		p->inst[p->len - 1] = n;
	}
}

// add to the program 'p' (in place) one instruction which has
// instruction, is_intruction, comment to describe it
void append_instruction(PartProgram *p, char *comment, Instruction inst) {
	p->len++;
	part_program_resize(p);

	p->comments[p->len - 1] = comment;
	p->is_inst[p->len - 1] = true;
	p->inst[p->len - 1] = inst;
}

/// Write a partial function to an UxnProgram a a certain 'pos'
/// This functions delete PartialUxnProgram
void write_part_program(PartProgram part_program, Program *p, uint16_t pos) {
	for (uint16_t i = 0; i < part_program.len; i++) {
		p->comments[pos + i] = part_program.comments[i];
		p->is_written[pos + i] = true;
		p->is_instruction[pos + i] = part_program.is_inst[i];
		p->memory[pos + i] = part_program.inst[i];
	}
	part_program_free(part_program);
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
PartProgram *compile_expr(CompilerState *state, Expression *expr) {
	switch (expr->tag) {
	case LET_E: {
		// Compile the expression
		PartProgram *let = compile_expr(state, expr->let.e);
		if (let == NULL) {
			fprintf(state->error, "compiling expr\n");
			return NULL;
		}

		// Add the variable to the variable list
		var_layout_append(&state->vars, expr->let.type, expr->let.var);
		// Get the address of this new variable
		VariableInfo var_info =
		    var_layout_get_addr(&state->vars, expr->let.var);

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
		PartProgram *lhs = compile_expr(state, expr->binary.lhs);
		if (lhs == NULL) {
			break;
		}
		PartProgram *rhs = compile_expr(state, expr->binary.rhs);
		if (rhs == NULL) {
			part_program_delete(lhs);
			break;
		}
		PartProgram *add = part_program_empty();
		append_instruction(add, "binary op",
				   binary_tag_to_instruction(expr->tag));
		return concat_program(concat_program(lhs, rhs), add);
	}
	case SEQUENCE_E: {
		PartProgram *sequence = part_program_empty();
		for (int i = 0; i < expr->sequence.len; i++) {
			PartProgram *e =
			    compile_expr(state, &expr->sequence.list[i]);
			if (e == NULL) {
				part_program_delete(sequence);
				return NULL;
			}
			sequence = concat_program(sequence, e);
		}
		return sequence;
	}
	case ASSIGN_E: {
		char *name = expr->assign.var;

		// Compile the expression
		PartProgram *assign = compile_expr(state, expr->assign.e);
		if (assign == NULL) {
			fprintf(state->error, "compiling expr\n");
			return NULL;
		}

		// Get the address of this new variable
		VariableInfo var_info = var_layout_get_addr(&state->vars, name);

		// Right now we assume that the value is of size 8 bits
		append_instruction(assign, NULL, LIT);
		append_number(assign, NULL, var_info.addr);
		append_instruction(assign, "assign def", STZ);
		return assign;
	}
	case DEREF_ASSIGN_E: { // *e1 = e2
		PartProgram *e1 = compile_expr(state, expr->deref_assign.e1);
		if (e1 == NULL) {
			break;
		}
		PartProgram *e2 = compile_expr(state, expr->deref_assign.e2);
		if (e2 == NULL) {
			break;
		}
		append_instruction(e1, "Deref Assign", DEO);
		return concat_program(e2, e1);
	}
	case DEREF_E: { // *e
		PartProgram *e = compile_expr(state, expr->deref.e);
		if (e == NULL) {
			break;
		}
		append_instruction(e, "Deref", LDZ);
		return e;
	}
	case VARIABLE_E: {
		char *name = expr->variable.name;
		PartProgram *var = part_program_empty();

		// Get the variable address
		VariableInfo var_info = var_layout_get_addr(&state->vars, name);
		if (!var_info.defined) {
			fprintf(state->error, "var '%s' not defined\n", name);
			break;
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
		fprintf(state->error, "return todo\n");
		break;
	}
	case FUNCTION_CALL_E: {
		PartProgram *p = part_program_empty();
		// Ajout de l'adresse actuelle( + 1) sur la return stack
		// JMP
		// Addresse de la function expr->fun_call.name
		// Ajout 
		fprintf(state->error, "function call todo\n");
		return p;
	}
	case CHAR_LITERAL_E: {
		PartProgram *number = part_program_empty();
		append_instruction(number, NULL, LIT);
		append_number(number, NULL, expr->char_literal.c);
		return number;
	}
	case STRING_LITERAL_E: {
		fprintf(state->error, "string literal todo\n");
		break;
	}
	case IF_ELSE_E: {
		PartProgram *cond = compile_expr(state, expr->if_else.cond);
		if (cond == NULL) {
			break;
		}
		PartProgram *if_body =
		    compile_expr(state, expr->if_else.if_body);
		if (if_body == NULL) {
			part_program_delete(cond);
			break;
		}
		PartProgram *else_body = NULL;
		if (expr->if_else.else_body != NULL) {
			else_body =
			    compile_expr(state, expr->if_else.else_body);
			if (else_body == NULL) {
				part_program_delete(cond);
				part_program_delete(if_body);
				break;
			}
		}
		if (else_body == NULL) {
			else_body = part_program_empty();
		}
		// from cond jump over else or go to else body
		append_instruction(cond, NULL, LIT);
		if (else_body->len > 255) {
			fprintf(state->error, "body else too large (> 255)");
			break;
		}
		append_number(cond, NULL, else_body->len + 3);
		append_instruction(cond, "if jump", JCN);

		if (if_body->len > 255) {
			fprintf(state->error, "body if too large (> 255)");
			break;
		}
		// end of else_body jump after if_body
		append_instruction(else_body, NULL, LIT);
		append_number(else_body, NULL, if_body->len);
		append_instruction(else_body, "else body and jump", JMP);
		return concat_program(cond, concat_program(else_body, if_body));
	}
	}
	fprintf(state->error, "compiling: ");
	fprintf_expression(state->error, expr);
	fprintf(state->error, "\n");
	return NULL;
}

PartProgram compile_function(FILE *error, Function *function) {

	VariableLayout vars;
	vars.names = NULL;
	vars.types = NULL;
	vars.len = 0;
	vars.cap = 0;

	CompilerState state;
	state.error = error;
	state.vars = vars;

	PartProgram result;

	PartProgram *expr = compile_expr(&state, function->expr);
	if (expr == NULL) {
		result.len = 0;
	} else {
		result = *expr;
		free(expr);
	}
	var_layout_delete(vars);
	return result;
}

Program *compile_to_uxn(FILE *error, Ast *ast) {
	// No functions => stop
	if (ast->len == 0) {
		ast_delete(ast);
		return NULL;
	}

	// Get the 'main' if there is one else stop the compilation
	bool found_main = false;
	int index_main = 0;
	for (int i = 0; i < ast->len; i++) {
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

	PartProgram *func_binary = malloc(sizeof(*func_binary) * ast->len);
	uint16_t *func_pos = malloc(sizeof(*func_pos) * ast->len);
	func_pos[index_main] = 0x100;

	// 1. Compile the different function
	for (int i = 0; i < ast->len; i++) {
		PartProgram func = compile_function(error, &ast->functions[i]);
		if (func.len == 0) {
			fprintf(error, "Error compiling function '%s'",
				ast->functions[i].name);
			ast_delete(ast);
			return NULL;
		}
		func_binary[i] = func;
	}

	// 2. Compute the positions of every functions
	uint16_t pos = 0x100;
	pos += func_binary[index_main].len;
	for (int i = 0; i < ast->len; i++) {
		if (i == index_main) {
			break;
		}
		pos += func_binary[i].len;
		func_pos[i] = pos;
	}

	// 3. Complete Address of functions in the partials programs
	// Partial programs that calls other functions contains some waiting
	// address. They are filled thanks to information from phase 1. and 2.
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
	for (int i = 0; i < ast->len; i++) {
		write_part_program(func_binary[i], program, func_pos[i]);
	}

	free(func_binary);
	free(func_pos);
	ast_delete(ast);
	return program;
}
