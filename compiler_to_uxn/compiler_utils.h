#include "../parser/parser.h"

/*
Uxn has 64kb of memory, 16 devices, 2 stacks of 256 bytes,
5-bits opcodes and 3 modes.

The list below show the standard opcodes
and their effect on a given stack a b c.

- PC: Program Counter
- |: Return Stack
- [M]: Memory
- [D+*]: Device Memory
- a8: a byte
- a16: a short.

LIT a b c [PC]
JCI a b (c8)PC+=[PC]
JMI a b c PC+=[PC]
JSI a b c | PC PC+=[PC]

BRK .        EQU a b==c          LDZ a b [c8]     ADD a b+c
INC a b c+1  NEQ a b!=c          STZ a [c8]=b     SUB a b-c
POP a b      GTH a b>c           LDR a b [PC+c8]  MUL a b*c
NIP a c      LTH a b<c           STR a [PC+c8]=b  DIV a b/c
SWP a c b    JMP a b PC+=c       LDA a b [c16]    AND a b&c
ROT b c a    JCN a (b8)PC+=c     STA a [c16]=b    ORA a b|c
DUP a b c c  JSR a b | PC PC+=c  DEI a b [D+c8]   EOR a b^c
OVR a b c b  STH a b | c         DEO a [D+c8]=b   SFT a b>>c8l<<c8h

The basic version of every operation operate
- with 8 bits word
- on the working stack
- delete what has been read onto the stack

They are 3 modes that can change that :
2 : operates on 16 bits words
r : make the instruction works with the return stack
k : don't touch at anything that the instruction read.
*/

// clang-format off
typedef enum {
BRK, LIT2, LITr, LIT2r,  LIT,   JCI,   JMI,    JSI,
INC, INC2, INCr, INC2r, INCk, INC2k, INCkr, INC2kr,
POP, POP2, POPr, POP2r, POPk, POP2k, POPkr, POP2kr,
NIP, NIP2, NIPr, NIP2r, NIPk, NIP2k, NIPkr, NIP2kr,
SWP, SWP2, SWPr, SWP2r, SWPk, SWP2k, SWPkr, SWP2kr,
ROT, ROT2, ROTr, ROT2r, ROTk, ROT2k, ROTkr, ROT2kr,
DUP, DUP2, DUPr, DUP2r, DUPk, DUP2k, DUPkr, DUP2kr,
OVR, OVR2, OVRr, OVR2r, OVRk, OVR2k, OVRkr, OVR2kr,

EQU, EQU2, EQUr, EQU2r, EQUk, EQU2k, EQUkr, EQU2kr,
NEQ, NEQ2, NEQr, NEQ2r, NEQk, NEQ2k, NEQkr, NEQ2kr,
GTH, GTH2, GTHr, GTH2r, GTHk, GTH2k, GTHkr, GTH2kr,
LTH, LTH2, LTHr, LTH2r, LTHk, LTH2k, LTHkr, LTH2kr,
JMP, JMP2, JMPr, JMP2r, JMPk, JMP2k, JMPkr, JMP2kr,
JCN, JCN2, JCNr, JCN2r, JCNk, JCN2k, JCNkr, JCN2kr,
JSR, JSR2, JSRr, JSR2r, JSRk, JSR2k, JSRkr, JSR2kr,
STH, STH2, STHr, STH2r, STHk, STH2k, STHkr, STH2kr,

LDZ, LDZ2, LDZr, LDZ2r, LDZk, LDZ2k, LDZkr, LDZ2kr,
STZ, STZ2, STZr, STZ2r, STZk, STZ2k, STZkr, STZ2kr,
LDR, LDR2, LDRr, LDR2r, LDRk, LDR2k, LDRkr, LDR2kr,
STR, STR2, STRr, STR2r, STRk, STR2k, STRkr, STR2kr,
LDA, LDA2, LDAr, LDA2r, LDAk, LDA2k, LDAkr, LDA2kr,
STA, STA2, STAr, STA2r, STAk, STA2k, STAkr, STA2kr,
DEI, DEI2, DEIr, DEI2r, DEIk, DEI2k, DEIkr, DEI2kr,
DEO, DEO2, DEOr, DEO2r, DEOk, DEO2k, DEOkr, DEO2kr,

ADD, ADD2, ADDr, ADD2r, ADDk, ADD2k, ADDkr, ADD2kr,
SUB, SUB2, SUBr, SUB2r, SUBk, SUB2k, SUBkr, SUB2kr,
MUL, MUL2, MULr, MUL2r, MULk, MUL2k, MULkr, MUL2kr,
DIV, DIV2, DIVr, DIV2r, DIVk, DIV2k, DIVkr, DIV2kr,
AND, AND2, ANDr, AND2r, ANDk, AND2k, ANDkr, AND2kr,
ORA, ORA2, ORAr, ORA2r, ORAk, ORA2k, ORAkr, ORA2kr,
EOR, EOR2, EORr, EOR2r, EORk, EOR2k, EORkr, EOR2kr,
SFT, SFT2, SFTr, SFT2r, SFTk, SFT2k, SFTkr, SFT2kr,
} UxnInstruction;
// clang-format on

typedef struct { // 0x10000 bytes = 64ko of memory
	char *comments[0x10000];
	UxnInstruction memory[0x10000];
	bool is_instruction[0x10000];
	bool is_written[0x10000];
} UxnProgram;

// clang-format off
void fprintf_uxn_instruction(FILE *file, UxnInstruction *inst);
// clang-format off

void fprintf_uxn_program(FILE *file, UxnProgram *uxn_program);
