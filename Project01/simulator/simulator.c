/* LC-2K Instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000 
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
int convertNum(int num);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

		/* TODO: */
		if (filePtr) {
			fclose(filePtr);
		}
        state.pc = 0; //program count
        int instruction_count = 0; //instruction count
        for (int i = 0; i < NUMREGS; i++) {
            state.reg[i] = 0;
        }
        while (1) {
            printState(&state);
            int opcode = (state.mem[state.pc] >> 22) & 0x7;
            int regA = (state.mem[state.pc] >> 19) & 0x7;
            int regB = (state.mem[state.pc] >> 16) & 0x7;
            int destReg = state.mem[state.pc] & 0x7;
            int offset = convertNum(state.mem[state.pc] & 0xFFFF);

            switch (opcode) {
                case 0: //add
                    state.reg[destReg] = state.reg[regA] + state.reg[regB];
                    state.pc++;
                    break;
                case 1: //nor
                    state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
                    state.pc++;
                    break;
                case 2: //lw
                    state.reg[regB] = state.mem[state.reg[regA] + offset];
                    state.pc++;
                    break;
                case 3: //sw
                    state.mem[state.reg[regA] + offset] = state.reg[regB];
                    state.pc++;
                    break;
                case 4: //beq
                    if (state.reg[regA] == state.reg[regB]) {
                        state.pc += offset;
                    }
                    state.pc++;
                    break;
                case 5: //jalr
                    state.reg[regB] = state.pc + 1;
                    state.pc = state.reg[regA];
                    break;
                case 6: //halt
                    printf("machine halted\n");
                    printf("total of %d instructions executed\n", instruction_count+1);
                    printf("final state of machine:\n");
                    state.pc++;
                    printState(&state);
                    exit(0);
                    break;
                case 7: //noop
                    state.pc++;
                    break;
                default:
                    break;
            }
            instruction_count++;
        }
    return(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
	/* convert a 16-bit number into a 32-bit Linux integer */
	if (num & (1 << 15)) {
		num -= (1 << 16);
	}
	return (num);
}
