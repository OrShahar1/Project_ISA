#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 4096	 //Number of lines in a program is limited by the num of addresses we can have
#define MAX_LABEL_LEN 51 //The maximal label length + '\0' charachter
#define MAX_LINE_LEN 501 //The maximal line length + '\0' charachter
#define MAX_ARGS_IN_LINE 6 //Line can have up to 5 args + label

#define OPCODES {"add", "sub", "and", "or", "sll", "sra", "srl", "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt", ".word"}
#define NUM_OF_OPCODES 21

#define REGS {"$zero", "$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra"}
#define NUM_OF_REGS 16

typedef struct label {
	char name[MAX_LABEL_LEN];
	int addr;
} Label;

void parse_labels(FILE* asm_prog); //Going over the asm_file and saving the adresses and names of the labels in the struct Lables.
void parse_instructions(FILE* asm_prog); //Going over the asm_file again and parsing each line to a 32-bit command, saving it in Memory.
void write_to_memory(FILE* memin); //Printing Memory array into the memin file.

void parse_line_into_args(char* line, char* line_args[]); //Gets a line and returns an array of the arguments in that line.
bool line_has_label(char* line); //Checks whether the input line contains a label, returns TRUE if it does.
int get_opcode_from_line(char* line_arg); //Gets a line and returns the line's opcode number.
int get_PC_increment(opcode); //Gets an opcode and returns the next PC increment size.
int calc_command(int opcode, int rd, int rs, int rt, int imm); //Gets each line's 5 argument, calculates and returns the equivelant command in 32-bit.

int get_reg_from_arg(char* arg); //Gets an argument string and returns the matching register number.
int get_imm_from_arg(char* arg); //Gets an immediate string and returns the matching label address or imm number.
void lowercase(char* str); //Gets a srting and turn all it's charachters to lowercase charachters.
int str_to_int(char* str); //Gets a number string (hex or decimal) and returns the matching number.

Label Labels[MAX_LINES];
int label_index = 0;
int Memory[MAX_LINES];
int mem_index = 0;
int PC = 0;
int last_line = 0;

int main(int argc, char** argv) {
	if (argc < 3) {
		printf("Not enough arguments!\n");
		return 1;
	};

	FILE* asm_prog = fopen(argv[1], "r"); 
	FILE* memin = fopen(argv[2], "w");

	if (asm_prog == NULL || memin == NULL) {
		printf("Couldn't open files, exiting\n");
		return 1;
	};
	
	parse_labels(asm_prog); //first iteration over the asm program - saving the label addresses.
	parse_instructions(asm_prog); //second iteration over the asm program - tuering each line into a 32-bit command.

	write_to_memory(memin); //writing the commands to memin

	fclose(asm_prog);
	fclose(memin);

	return 0;
};

void parse_line_into_args(char* line, char* line_args[]) {
	
	for (int i = 0; i < MAX_ARGS_IN_LINE; i++) { //in case we don't have all 6 args in the line
		line_args[i] = NULL;
	};

	char* comment_index = strchr(line, '#');
	if (comment_index != NULL) { //line has a comment
		if (comment_index == line) return; //line starts with a comment - nothing to parse
		*comment_index = '\0'; //cutting off the comment part of the line
	};

	char line_cpy[MAX_LINE_LEN];
	strcpy(line_cpy, line); //we want line variable to remain the same so it can indicate if there's a label in the line
	const char* delimiters = ":, \t\n";
	char* arg = strtok(line_cpy, delimiters); //take one argument at a time
	int i = 0;

	while (arg != NULL) {
		line_args[i] = arg;
		i++;
		arg = strtok(NULL, delimiters); //get next argument
	};
};

bool line_has_label(char* line) {
	char* colon_index = strchr(line, ':'); //if the line has a colon it has a label 

	return (colon_index != NULL);
}

void get_label_from_line(char* line, char* label) {
	char line_cpy[MAX_LINE_LEN];
	strcpy(line_cpy, line); //we want line variable to remain the same
	char* colon_index = strchr(line_cpy, ':');

	if (colon_index == NULL) {
		label = NULL;
	}
	else {
		*colon_index = '\0'; //cut the line in the end of the label
		strcpy(label, line_cpy);
	};
};

int get_opcode_from_line(char* line_arg) {
	if (line_arg == NULL) return -1; //indicates we couldn't find an opcode
	char* opcodes[NUM_OF_OPCODES] = OPCODES;
	for (int i = 0; i < NUM_OF_OPCODES; i++) { //going over all the opcodes and comparing them to the argument
		if (strcmp(line_arg, opcodes[i]) == 0) {
			return i;
		};
	};
	return -1; //indicates we couldn't find an opcode
};

int get_PC_increment(opcode) {
	if (opcode == -1) return 0; //NULL
	if (opcode <= 7) return 1; //add,sub,and,or,all,sra,srl
	if (opcode <= 18) return 1; //beq,bne,blt,bgt,ble,bge,jal,lw,sw,reti,in,out
	if (opcode == 19) return 1; //halt
	if (opcode == 20) return 0; //.word
	return 0;
};

void parse_labels(FILE* asm_prog) {
	char line[MAX_LINE_LEN];
	char* line_args[MAX_ARGS_IN_LINE];
	int opcode;

	
	while (fgets(line, MAX_LINE_LEN, asm_prog) != NULL) { //going over all the lines in the file
		parse_line_into_args(line, line_args);
		bool has_label = line_has_label(line);

		if (has_label) { //save label's address
			strcpy(Labels[label_index].name, line_args[0]);
			Labels[label_index].addr = PC;
			label_index++;
			opcode = get_opcode_from_line(line_args[1]); //when we have a label the opcode is second
		} else {
			opcode = get_opcode_from_line(line_args[0]); //when we don't have a label the opcode is first
		};
		PC += get_PC_increment(opcode); //move the program counter, unless it's a word command or NULL
	};

	rewind(asm_prog); //go back to the begining of the file
};

void parse_instructions(FILE* asm_prog) {
	char line[MAX_LINE_LEN];
	char* line_args[MAX_ARGS_IN_LINE];
	int opcode, rd, rs, rt, imm;
	int opcode_index;

	while (fgets(line, MAX_LINE_LEN, asm_prog) != NULL) {  //going over all the lines in the file
		parse_line_into_args(line, line_args); //turning the line into a list of arguments
		opcode_index = 0; //when we don't have a label the opcode is first
		if (line_has_label(line)) {
			opcode_index = 1; //when we have a label the opcode is second
		};
		if (line_args[opcode_index] == NULL) continue;
		opcode = get_opcode_from_line(line_args[opcode_index]);
		
		if (opcode == 20) { //.word command
			int address = str_to_int(line_args[opcode_index + 1]);
			int data = str_to_int(line_args[opcode_index + 2]);
			Memory[address] = data; //storing the data in the right address in memory

			if (last_line < address) { last_line = address; }; //update address of the last line written to memory 
			continue;
		};

		rd  = get_reg_from_arg(line_args[opcode_index + 1]); //get rd's register number
		rs  = get_reg_from_arg(line_args[opcode_index + 2]); //get rs's register number
		rt  = get_reg_from_arg(line_args[opcode_index + 3]); //get rt's register number
		imm = get_imm_from_arg(line_args[opcode_index + 4]); //get the immediate as a integer, or label's address

		int command = calc_command(opcode, rd, rs, rt, imm); // calculate the 32-bit command from the arguments
		Memory[mem_index] = command; //write the command in the right place i nmemory
		if (last_line < mem_index) { last_line = mem_index; };  //update address of the last line written to memory 
		mem_index++;

	};
};

void write_to_memory(FILE* memin) {
	for (int i = 0; i <= last_line; i++) { //writing each line to memin file
		fprintf(memin, "%08X\n", Memory[i]);
	}
};

int calc_command(int opcode, int rd, int rs, int rt, int imm) {
	int command = 0;
	command += (opcode << 24); //bits [32:25]
	command += (rd << 20); //bits [24:21]
	command += (rs << 16); //bits [20:17]
	command += (rt << 12); //bits [16:13]
	command += (imm&0x00000fff); //bits [15:0]

	return command;
};

int get_reg_from_arg(char* arg) {
	char* regs[NUM_OF_REGS] = REGS;

	for (int i = 0; i < NUM_OF_REGS; i++) { //going over the registers and comparing them to the argument
		if (strcmp(arg, regs[i]) == 0) {
			return i;
		};
	};
	if (strcmp(arg, "$0") == 0) return 0; //"$zero" and "$0" are both 0

	return -1; //indicates we couldn't fine a matching register
};

int get_imm_from_arg(char* arg) {
	for (int i = 0; i <= label_index; i++) { //going over the labels we have and comparing them to the argument
		if (strcmp(arg, Labels[i].name) == 0) {
			return Labels[i].addr;
		};
	};
	return str_to_int(arg); //if we didn't find a matching label - treat the argument as an immediate number
};

void lowercase(char* str) {
	for (int i = 0; str[i]; i++) { //going over all charachters in the string
		str[i] = tolower(str[i]); //turning the charachter to lowercase
	};
};

int str_to_int(char* str) {
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) { //str is a hexadecimal number
		lowercase(str);
		return strtol(str, NULL, 0); 
	};
	return strtol(str, NULL, 10); //srt is a decimal number
};


