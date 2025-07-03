/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000
#define MAXLABELLENGTH 7
#define MAX_16BIT 32767
#define MIN_16BIT -32768

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int findLabelAddress(char *label);
int checkRange(char *arg);

typedef struct {
    char label[MAXLABELLENGTH];
    int address;
} labelEntry;

labelEntry labels[MAXLINELENGTH];
int labelCount = 0;

int main(int argc, char *argv[]) 
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr;
	char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], 
			 arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
	int address = 0;

	if (argc != 3) {
		printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
				argv[0]);
		exit(1);
	}

	inFileString = argv[1];
	outFileString = argv[2];

	inFilePtr = fopen(inFileString, "r");
	if (inFilePtr == NULL) {
		printf("error in opening %s\n", inFileString);
		exit(1);
	}
	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	/* TODO: Phase-1 label calculation */
	while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        if (strlen(label) > 0) {
            for (int i = 0; i < labelCount; i++) {
                if (!strcmp(labels[i].label, label)) {
                    printf("error: duplicate label %s\n", label);
                    exit(1);
                }
            }
            if (strlen(label) >= MAXLABELLENGTH) {
                printf("error: label too long %s\n", label);
                exit(1);
            }
            strcpy(labels[labelCount].label, label);
            labels[labelCount].address = address;
            labelCount++;
        }
        address++;
    }
	
	/* this is how to rewind the file ptr so that you start reading from the
		 beginning of the file */
	rewind(inFilePtr);
	address = 0;

	/* TODO: Phase-2 generate machine codes to outfile */
	while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
		int machineCode = 0;
		int opcodeNum;
        if (!strcmp(opcode, "add") || !strcmp(opcode, "nor")) { //r-type
			if (strcmp(opcode, "add") == 0) {
				opcodeNum = 0; //add
			} 
			else {
				opcodeNum = 1; //nor
			}
            machineCode += (opcodeNum << 22);
            machineCode += (checkRange(arg0) << 19);
            machineCode += (checkRange(arg1) << 16);
			machineCode += (checkRange(arg2) << 0); 
        } 
        else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")) { //i-type
			int offsetField;
			if (strcmp(opcode, "lw") == 0) {
				opcodeNum = 2; //lw
			} 
			else if (strcmp(opcode, "sw") == 0) {
				opcodeNum = 3; //sw
			} 
			else {
				opcodeNum = 4; //beq
			}
            machineCode += (opcodeNum << 22);
            machineCode += (checkRange(arg0) << 19);
            machineCode += (checkRange(arg1) << 16);
            if (isNumber(arg2)) {
                offsetField = checkRange(arg2);
            } 
			else {
                offsetField = findLabelAddress(arg2);
                if (!strcmp(opcode, "beq")) {
                    offsetField = offsetField - (address+1);
                }
            }
            if (offsetField < MIN_16BIT || offsetField > MAX_16BIT) {
                printf("error: offsetField out of range %d\n", offsetField);
                exit(1);
            }
            machineCode += (offsetField & 0xFFFF);
        } 
        else if (!strcmp(opcode, "jalr")) { //j-type
            machineCode += (5 << 22);
            machineCode += (checkRange(arg0) << 19);
            machineCode += (checkRange(arg1) << 16);
        } 
        else if (!strcmp(opcode, "halt")) { //o-type
            machineCode += (6 << 22);
        } 
        else if (!strcmp(opcode, "noop")) { //o-type
            machineCode += (7 << 22);
        } 
        else if (!strcmp(opcode, ".fill")) {
            if (isNumber(arg0)) {
                machineCode = atoi(arg0);
            } 
			else {
                machineCode = findLabelAddress(arg0);
            }
        }
        else {
            printf("error: unrecognized opcode %s\n", opcode);
            exit(1);
        }

		fprintf(outFilePtr, "%d\n", machineCode);
		address++;
	}

	if (inFilePtr) {
		fclose(inFilePtr);
	}
	if (outFilePtr) {
		fclose(outFilePtr);
	}
	return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
		char *arg1, char *arg2)
{
	char line[MAXLINELENGTH];
	char *ptr = line;

	/* delete prior values */
	label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

	/* read the line from the assembly-language file */
	if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		/* reached end of file */
		return(0);
	}

	/* check for line too long (by looking for a \n) */
	if (strchr(line, '\n') == NULL) {
		/* line too long */
		printf("error: line too long\n");
		exit(1);
	}

	/* is there a label? */
	ptr = line;
	if (sscanf(ptr, "%[^\t\n\r ]", label)) {
		/* successfully read label; advance pointer over the label */
		ptr += strlen(label);
	}

	/*
	 * Parse the rest of the line.  Would be nice to have real regular
	 * expressions, but scanf will suffice.
	 */
	sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%"
			"[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
	return(1);
}

int isNumber(char *string)
{
	/* return 1 if string is a number */
	int i;
	return( (sscanf(string, "%d", &i)) == 1);
}

int findLabelAddress(char *label) {
    for (int i = 0; i < labelCount; i++) {
        if (!strcmp(labels[i].label, label)) {
            return labels[i].address;
        }
    }
    printf("error: undefined label %s\n", label);
    exit(1);
}

int checkRange(char *arg) {
    int regNum = atoi(arg);
    if (regNum < 0 || regNum > 7) {
        printf("error: registers outside the range [0, 7]");
		exit(1);
    }
    return regNum; 
}
