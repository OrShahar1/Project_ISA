#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

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

typedef struct line {
	Label label;
	char* args[MAX_ARGS_IN_LINE];

	int line_has_label;
} Line;

void parse_labels(FILE* asm_prog);
void parse_instructions(FILE* asm_prog);
void write_to_memory(FILE* memin);

void parse_line_into_args(char* line, char* line_args[]);
int line_has_label(char* line);
int get_opcode_from_line(char* line_arg);
int get_PC_increment(opcode);
int calc_command(int opcode, int rd, int rs, int rt, int imm);

int get_reg_from_arg(char* arg);
int get_imm_from_arg(char* arg);
void lowercase(char* str);
int str_to_int(char* str);

Line Lines[MAX_LINES];
int line_index = 0;
Label Labels[MAX_LINES];
int label_index = 0;
int Memory[MAX_LINES];
int mem_index = 0;
int PC = 0;

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

	parse_labels(asm_prog);
	parse_instructions(asm_prog);

	write_to_memory(memin);

	fclose(asm_prog);
	fclose(memin);

	return 0;
};

void parse_line_into_args(char* line, char* line_args[]) {
	
	for (int i = 0; i < MAX_ARGS_IN_LINE; i++) {
		line_args[i] = NULL;
	};

	char* comment_index = strchr(line, '#');
	if (comment_index != NULL) {
		if (comment_index == line) return;
		*comment_index = '\0';
	};

	char* line_cpy[MAX_LINE_LEN];
	strcpy(line_cpy, line);
	const char* delimiters = ":, \t\n";
	char* arg = strtok(line_cpy, delimiters);
	int i = 0;

	while (arg != NULL) {
		line_args[i] = arg;
		i++;
		arg = strtok(NULL, delimiters);
	};
};

int line_has_label(char* line) {
	char line_cpy[MAX_LINE_LEN];
	strcpy(line_cpy, line);
	char* colon_index = strchr(line_cpy, ':');

	if (colon_index == NULL) {
		return 0;
	}
	else {
		return 1;
	};
}

void get_label_from_line(char* line, char* label) {
	char line_cpy[MAX_LINE_LEN];
	strcpy(line_cpy, line);
	char* colon_index = strchr(line_cpy, ':');

	if (colon_index == NULL) {
		*label = NULL;
	}
	else {
		*colon_index = '\0';
		strcpy(label, line_cpy);
	};
};

int get_opcode_from_line(char* line_arg) {
	if (line_arg == NULL) return -1;
	char* opcodes[NUM_OF_OPCODES] = OPCODES;
	for (int i = 0; i < NUM_OF_OPCODES; i++) {
		if (strcmp(line_arg, opcodes[i]) == 0) {
			return i;
		};
	};
	return -1;
};

int get_PC_increment(opcode) {
	if (opcode == -1) return 0; //NULL
	if (opcode <= 7) return 1; //add,sub,and,or,all,sra,srl
	if (opcode <= 18) return 1; //beq,bne,blt,bgt,ble,bge,jal,lw,sw,reti,in,out
	if (opcode == 19) return 1; //halt
	if (opcode == 20) return 0; //.word
	return -1;
};

void parse_labels(FILE* asm_prog) {
	char line[MAX_LINE_LEN];
	char* line_args[MAX_ARGS_IN_LINE];
	char label[MAX_LABEL_LEN];
	int opcode;

	
	while (fgets(line, MAX_LINE_LEN, asm_prog) != NULL) {
		parse_line_into_args(line, line_args);
		int has_label = line_has_label(line);

		if (has_label == 1) {
			strcpy(Labels[label_index].name, line_args[0]);
			Labels[label_index].addr = PC;
			label_index++;
			opcode = get_opcode_from_line(line_args[1]);
			Lines[line_index].line_has_label = 1;
		} else {
			opcode = get_opcode_from_line(line_args[0]);
			Lines[line_index].line_has_label = 0;
		};
		line_index++;
		PC += get_PC_increment(opcode);
	};

	rewind(asm_prog);
	line_index = 0;
};

void parse_instructions(FILE* asm_prog) {
	char line[MAX_LINE_LEN];
	char* line_args[MAX_ARGS_IN_LINE];
	int opcode, rd, rs, rt, imm;
	int opcode_index;

	while (fgets(line, MAX_LINE_LEN, asm_prog) != NULL) {
		printf("%s\n", line);
		parse_line_into_args(line, line_args);
		opcode_index = 0;
		if (line_has_label(line) == 1) {
			opcode_index = 1;
		};
		if (line_args[opcode_index] == NULL) continue;
		opcode = get_opcode_from_line(line_args[opcode_index]);
		
		if (opcode == 20) {
			int address = str_to_int(line_args[opcode_index + 1]);
			int data = str_to_int(line_args[opcode_index + 2]);
			Memory[address] = data;
			printf("%08x\n", Memory[address]);
			continue;
		};

		rd  = get_reg_from_arg(line_args[opcode_index + 1]);
		rs  = get_reg_from_arg(line_args[opcode_index + 2]);
		rt  = get_reg_from_arg(line_args[opcode_index + 3]);
		imm = get_imm_from_arg(line_args[opcode_index + 4]);

		int command = calc_command(opcode, rd, rs, rt, imm);
		Memory[mem_index] = command;
		mem_index++;

	};
};

void write_to_memory(FILE* memin) {
	for (int i = 0; i < MAX_LINES; i++) {
		fprintf(memin, "%08X\n", Memory[i]);
	}
};

int calc_command(int opcode, int rd, int rs, int rt, int imm) {
	int command = 0;
	command += (opcode << 24);
	command += (rd << 20);
	command += (rs << 16);
	command += (rt << 12);
	command += (imm&0x00000fff);

	return command;
};

int get_reg_from_arg(char* arg) {
	char* regs[NUM_OF_REGS] = REGS;

	for (int i = 0; i < NUM_OF_REGS; i++) {
		if (strcmp(arg, regs[i]) == 0) {
			return i;
		};
	};
	if (strcmp(arg, "$0") == 0) return 0;

	return -1;
};

int get_imm_from_arg(char* arg) {
	for (int i = 0; i <= label_index; i++) {
		if (strcmp(arg, Labels[i].name) == 0) {
			return Labels[i].addr;
		};
	};
	return str_to_int(arg);
};

void lowercase(char* str) {
	for (int i = 0; str[i]; i++) {
		str[i] = tolower(str[i]);
	};
};

int str_to_int(char* str) {
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		lowercase(str);
		return strtol(str, NULL, 0);
	};
	return strtol(str, NULL, 10);
};


