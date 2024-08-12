#include "compiler_utils.h"

Instruction binary_tag_to_instruction(ExpressionType type) {
	switch (type) {
	case ADD_E:
		return ADD;
	case SUB_E:
		return SUB;
	case MULT_E:
		return MUL;
	case DIV_E:
		return DIV;
	default:
		return BRK;
	}
}

// clang-format off
void fprintf_uxn_instruction(FILE *file, Instruction *inst) {
	switch (*inst) {
	case BRK: fprintf(file, "BRK"); break;
	case LIT2: fprintf(file, "LIT2"); break;
	case LITr: fprintf(file, "LITr"); break;
	case LIT2r: fprintf(file, "LIT2r"); break;
	case LIT: fprintf(file, "LIT"); break;
	case JCI: fprintf(file, "JCI"); break;
	case JMI: fprintf(file, "JMI"); break;
	case JSI: fprintf(file, "JSI"); break;
	case INC: fprintf(file, "INC"); break;
	case INC2: fprintf(file, "INC2"); break;
	case INCr: fprintf(file, "INCr"); break;
	case INC2r: fprintf(file, "INC2r"); break;
	case INCk: fprintf(file, "INCk"); break;
	case INC2k: fprintf(file, "INC2k"); break;
	case INCkr: fprintf(file, "INCkr"); break;
	case INC2kr: fprintf(file, "INC2kr"); break;
	case POP: fprintf(file, "POP"); break;
	case POP2: fprintf(file, "POP2"); break;
	case POPr: fprintf(file, "POPr"); break;
	case POP2r: fprintf(file, "POP2r"); break;
	case POPk: fprintf(file, "POPk"); break;
	case POP2k: fprintf(file, "POP2k"); break;
	case POPkr: fprintf(file, "POPkr"); break;
	case POP2kr: fprintf(file, "POP2kr"); break;
	case NIP: fprintf(file, "NIP"); break;
	case NIP2: fprintf(file, "NIP2"); break;
	case NIPr: fprintf(file, "NIPr"); break;
	case NIP2r: fprintf(file, "NIP2r"); break;
	case NIPk: fprintf(file, "NIPk"); break;
	case NIP2k: fprintf(file, "NIP2k"); break;
	case NIPkr: fprintf(file, "NIPkr"); break;
	case NIP2kr: fprintf(file, "NIP2kr"); break;
	case SWP: fprintf(file, "SWP"); break;
	case SWP2: fprintf(file, "SWP2"); break;
	case SWPr: fprintf(file, "SWPr"); break;
	case SWP2r: fprintf(file, "SWP2r"); break;
	case SWPk: fprintf(file, "SWPk"); break;
	case SWP2k: fprintf(file, "SWP2k"); break;
	case SWPkr: fprintf(file, "SWPkr"); break;
	case SWP2kr: fprintf(file, "SWP2kr"); break;
	case ROT: fprintf(file, "ROT"); break;
	case ROT2: fprintf(file, "ROT2"); break;
	case ROTr: fprintf(file, "ROTr"); break;
	case ROT2r: fprintf(file, "ROT2r"); break;
	case ROTk: fprintf(file, "ROTk"); break;
	case ROT2k: fprintf(file, "ROT2k"); break;
	case ROTkr: fprintf(file, "ROTkr"); break;
	case ROT2kr: fprintf(file, "ROT2kr"); break;
	case DUP: fprintf(file, "DUP"); break;
	case DUP2: fprintf(file, "DUP2"); break;
	case DUPr: fprintf(file, "DUPr"); break;
	case DUP2r: fprintf(file, "DUP2r"); break;
	case DUPk: fprintf(file, "DUPk"); break;
	case DUP2k: fprintf(file, "DUP2k"); break;
	case DUPkr: fprintf(file, "DUPkr"); break;
	case DUP2kr: fprintf(file, "DUP2kr"); break;
	case OVR: fprintf(file, "OVR"); break;
	case OVR2: fprintf(file, "OVR2"); break;
	case OVRr: fprintf(file, "OVRr"); break;
	case OVR2r: fprintf(file, "OVR2r"); break;
	case OVRk: fprintf(file, "OVRk"); break;
	case OVR2k: fprintf(file, "OVR2k"); break;
	case OVRkr: fprintf(file, "OVRkr"); break;
	case OVR2kr: fprintf(file, "OVR2kr"); break;
	case EQU: fprintf(file, "EQU"); break;
	case EQU2: fprintf(file, "EQU2"); break;
	case EQUr: fprintf(file, "EQUr"); break;
	case EQU2r: fprintf(file, "EQU2r"); break;
	case EQUk: fprintf(file, "EQUk"); break;
	case EQU2k: fprintf(file, "EQU2k"); break;
	case EQUkr: fprintf(file, "EQUkr"); break;
	case EQU2kr: fprintf(file, "EQU2kr"); break;
	case NEQ: fprintf(file, "NEQ"); break;
	case NEQ2: fprintf(file, "NEQ2"); break;
	case NEQr: fprintf(file, "NEQr"); break;
	case NEQ2r: fprintf(file, "NEQ2r"); break;
	case NEQk: fprintf(file, "NEQk"); break;
	case NEQ2k: fprintf(file, "NEQ2k"); break;
	case NEQkr: fprintf(file, "NEQkr"); break;
	case NEQ2kr: fprintf(file, "NEQ2kr"); break;
	case GTH: fprintf(file, "GTH"); break;
	case GTH2: fprintf(file, "GTH2"); break;
	case GTHr: fprintf(file, "GTHr"); break;
	case GTH2r: fprintf(file, "GTH2r"); break;
	case GTHk: fprintf(file, "GTHk"); break;
	case GTH2k: fprintf(file, "GTH2k"); break;
	case GTHkr: fprintf(file, "GTHkr"); break;
	case GTH2kr: fprintf(file, "GTH2kr"); break;
	case LTH: fprintf(file, "LTH"); break;
	case LTH2: fprintf(file, "LTH2"); break;
	case LTHr: fprintf(file, "LTHr"); break;
	case LTH2r: fprintf(file, "LTH2r"); break;
	case LTHk: fprintf(file, "LTHk"); break;
	case LTH2k: fprintf(file, "LTH2k"); break;
	case LTHkr: fprintf(file, "LTHkr"); break;
	case LTH2kr: fprintf(file, "LTH2kr"); break;
	case JMP: fprintf(file, "JMP"); break;
	case JMP2: fprintf(file, "JMP2"); break;
	case JMPr: fprintf(file, "JMPr"); break;
	case JMP2r: fprintf(file, "JMP2r"); break;
	case JMPk: fprintf(file, "JMPk"); break;
	case JMP2k: fprintf(file, "JMP2k"); break;
	case JMPkr: fprintf(file, "JMPkr"); break;
	case JMP2kr: fprintf(file, "JMP2kr"); break;
	case JCN: fprintf(file, "JCN"); break;
	case JCN2: fprintf(file, "JCN2"); break;
	case JCNr: fprintf(file, "JCNr"); break;
	case JCN2r: fprintf(file, "JCN2r"); break;
	case JCNk: fprintf(file, "JCNk"); break;
	case JCN2k: fprintf(file, "JCN2k"); break;
	case JCNkr: fprintf(file, "JCNkr"); break;
	case JCN2kr: fprintf(file, "JCN2kr"); break;
	case JSR: fprintf(file, "JSR"); break;
	case JSR2: fprintf(file, "JSR2"); break;
	case JSRr: fprintf(file, "JSRr"); break;
	case JSR2r: fprintf(file, "JSR2r"); break;
	case JSRk: fprintf(file, "JSRk"); break;
	case JSR2k: fprintf(file, "JSR2k"); break;
	case JSRkr: fprintf(file, "JSRkr"); break;
	case JSR2kr: fprintf(file, "JSR2kr"); break;
	case STH: fprintf(file, "STH"); break;
	case STH2: fprintf(file, "STH2"); break;
	case STHr: fprintf(file, "STHr"); break;
	case STH2r: fprintf(file, "STH2r"); break;
	case STHk: fprintf(file, "STHk"); break;
	case STH2k: fprintf(file, "STH2k"); break;
	case STHkr: fprintf(file, "STHkr"); break;
	case STH2kr: fprintf(file, "STH2kr"); break;
	case LDZ: fprintf(file, "LDZ"); break;
	case LDZ2: fprintf(file, "LDZ2"); break;
	case LDZr: fprintf(file, "LDZr"); break;
	case LDZ2r: fprintf(file, "LDZ2r"); break;
	case LDZk: fprintf(file, "LDZk"); break;
	case LDZ2k: fprintf(file, "LDZ2k"); break;
	case LDZkr: fprintf(file, "LDZkr"); break;
	case LDZ2kr: fprintf(file, "LDZ2kr"); break;
	case STZ: fprintf(file, "STZ"); break;
	case STZ2: fprintf(file, "STZ2"); break;
	case STZr: fprintf(file, "STZr"); break;
	case STZ2r: fprintf(file, "STZ2r"); break;
	case STZk: fprintf(file, "STZk"); break;
	case STZ2k: fprintf(file, "STZ2k"); break;
	case STZkr: fprintf(file, "STZkr"); break;
	case STZ2kr: fprintf(file, "STZ2kr"); break;
	case LDR: fprintf(file, "LDR"); break;
	case LDR2: fprintf(file, "LDR2"); break;
	case LDRr: fprintf(file, "LDRr"); break;
	case LDR2r: fprintf(file, "LDR2r"); break;
	case LDRk: fprintf(file, "LDRk"); break;
	case LDR2k: fprintf(file, "LDR2k"); break;
	case LDRkr: fprintf(file, "LDRkr"); break;
	case LDR2kr: fprintf(file, "LDR2kr"); break;
	case STR: fprintf(file, "STR"); break;
	case STR2: fprintf(file, "STR2"); break;
	case STRr: fprintf(file, "STRr"); break;
	case STR2r: fprintf(file, "STR2r"); break;
	case STRk: fprintf(file, "STRk"); break;
	case STR2k: fprintf(file, "STR2k"); break;
	case STRkr: fprintf(file, "STRkr"); break;
	case STR2kr: fprintf(file, "STR2kr"); break;
	case LDA: fprintf(file, "LDA"); break;
	case LDA2: fprintf(file, "LDA2"); break;
	case LDAr: fprintf(file, "LDAr"); break;
	case LDA2r: fprintf(file, "LDA2r"); break;
	case LDAk: fprintf(file, "LDAk"); break;
	case LDA2k: fprintf(file, "LDA2k"); break;
	case LDAkr: fprintf(file, "LDAkr"); break;
	case LDA2kr: fprintf(file, "LDA2kr"); break;
	case STA: fprintf(file, "STA"); break;
	case STA2: fprintf(file, "STA2"); break;
	case STAr: fprintf(file, "STAr"); break;
	case STA2r: fprintf(file, "STA2r"); break;
	case STAk: fprintf(file, "STAk"); break;
	case STA2k: fprintf(file, "STA2k"); break;
	case STAkr: fprintf(file, "STAkr"); break;
	case STA2kr: fprintf(file, "STA2kr"); break;
	case DEI: fprintf(file, "DEI"); break;
	case DEI2: fprintf(file, "DEI2"); break;
	case DEIr: fprintf(file, "DEIr"); break;
	case DEI2r: fprintf(file, "DEI2r"); break;
	case DEIk: fprintf(file, "DEIk"); break;
	case DEI2k: fprintf(file, "DEI2k"); break;
	case DEIkr: fprintf(file, "DEIkr"); break;
	case DEI2kr: fprintf(file, "DEI2kr"); break;
	case DEO: fprintf(file, "DEO"); break;
	case DEO2: fprintf(file, "DEO2"); break;
	case DEOr: fprintf(file, "DEOr"); break;
	case DEO2r: fprintf(file, "DEO2r"); break;
	case DEOk: fprintf(file, "DEOk"); break;
	case DEO2k: fprintf(file, "DEO2k"); break;
	case DEOkr: fprintf(file, "DEOkr"); break;
	case DEO2kr: fprintf(file, "DEO2kr"); break;
	case ADD: fprintf(file, "ADD"); break;
	case ADD2: fprintf(file, "ADD2"); break;
	case ADDr: fprintf(file, "ADDr"); break;
	case ADD2r: fprintf(file, "ADD2r"); break;
	case ADDk: fprintf(file, "ADDk"); break;
	case ADD2k: fprintf(file, "ADD2k"); break;
	case ADDkr: fprintf(file, "ADDkr"); break;
	case ADD2kr: fprintf(file, "ADD2kr"); break;
	case SUB: fprintf(file, "SUB"); break;
	case SUB2: fprintf(file, "SUB2"); break;
	case SUBr: fprintf(file, "SUBr"); break;
	case SUB2r: fprintf(file, "SUB2r"); break;
	case SUBk: fprintf(file, "SUBk"); break;
	case SUB2k: fprintf(file, "SUB2k"); break;
	case SUBkr: fprintf(file, "SUBkr"); break;
	case SUB2kr: fprintf(file, "SUB2kr"); break;
	case MUL: fprintf(file, "MUL"); break;
	case MUL2: fprintf(file, "MUL2"); break;
	case MULr: fprintf(file, "MULr"); break;
	case MUL2r: fprintf(file, "MUL2r"); break;
	case MULk: fprintf(file, "MULk"); break;
	case MUL2k: fprintf(file, "MUL2k"); break;
	case MULkr: fprintf(file, "MULkr"); break;
	case MUL2kr: fprintf(file, "MUL2kr"); break;
	case DIV: fprintf(file, "DIV"); break;
	case DIV2: fprintf(file, "DIV2"); break;
	case DIVr: fprintf(file, "DIVr"); break;
	case DIV2r: fprintf(file, "DIV2r"); break;
	case DIVk: fprintf(file, "DIVk"); break;
	case DIV2k: fprintf(file, "DIV2k"); break;
	case DIVkr: fprintf(file, "DIVkr"); break;
	case DIV2kr: fprintf(file, "DIV2kr"); break;
	case AND: fprintf(file, "AND"); break;
	case AND2: fprintf(file, "AND2"); break;
	case ANDr: fprintf(file, "ANDr"); break;
	case AND2r: fprintf(file, "AND2r"); break;
	case ANDk: fprintf(file, "ANDk"); break;
	case AND2k: fprintf(file, "AND2k"); break;
	case ANDkr: fprintf(file, "ANDkr"); break;
	case AND2kr: fprintf(file, "AND2kr"); break;
	case ORA: fprintf(file, "ORA"); break;
	case ORA2: fprintf(file, "ORA2"); break;
	case ORAr: fprintf(file, "ORAr"); break;
	case ORA2r: fprintf(file, "ORA2r"); break;
	case ORAk: fprintf(file, "ORAk"); break;
	case ORA2k: fprintf(file, "ORA2k"); break;
	case ORAkr: fprintf(file, "ORAkr"); break;
	case ORA2kr: fprintf(file, "ORA2kr"); break;
	case EOR: fprintf(file, "EOR"); break;
	case EOR2: fprintf(file, "EOR2"); break;
	case EORr: fprintf(file, "EORr"); break;
	case EOR2r: fprintf(file, "EOR2r"); break;
	case EORk: fprintf(file, "EORk"); break;
	case EOR2k: fprintf(file, "EOR2k"); break;
	case EORkr: fprintf(file, "EORkr"); break;
	case EOR2kr: fprintf(file, "EOR2kr"); break;
	case SFT: fprintf(file, "SFT"); break;
	case SFT2: fprintf(file, "SFT2"); break;
	case SFTr: fprintf(file, "SFTr"); break;
	case SFT2r: fprintf(file, "SFT2r"); break;
	case SFTk: fprintf(file, "SFTk"); break;
	case SFT2k: fprintf(file, "SFT2k"); break;
	case SFTkr: fprintf(file, "SFTkr"); break;
	case SFT2kr: fprintf(file, "SFT2kr"); break;
	}
}
// clang-format on

void fprintf_uxn_program(FILE *file, Program *uxn_program) {
	for (int i = 0; i < 0x10000; i++) {
		if (i == 0x100) {
			fprintf(file, "|0100");
		}
		if (!uxn_program->is_written[i]) {
			continue;
		}
		fprintf(file, " ");
		if (uxn_program->is_instruction[i]) {
			fprintf_uxn_instruction(file, &uxn_program->memory[i]);
		} else {
			fprintf(file, "%02x", (uint8_t)uxn_program->memory[i]);
		}
		if (uxn_program->comments[i] != NULL) {
			fprintf(file, " ( %s )\n", uxn_program->comments[i]);
		}
	}
}
