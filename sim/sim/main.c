#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MEM_ADDRS 4096
#define NUM_OF_REGISTERS 16
#define NUM_OF_IORegisters 18
#define DISK_SECTORS 128
#define WORDS_IN_SECTOR 128 // 512 bytes/4=128 words
#define OPCODE_MASK 0x000000FF
#define REGISTER_MASK 0x0000000F
#define BRANCH_MASK 0x00000FFF //8=1000 to keep the sign of imm

#define NUM_OF_OPCODES 19

typedef enum reg {
	$zero = 0,
	$imm, $v0, $a0, $a1, $t0, $t1, $t2, $t3, $s0, $s1, $s2, $gp, $sp, $fp, $ra
}reg;

typedef enum opcode {
	add = 0,
	sub,and,or,sll,sra,srl,beq,bne,blt,bgt,ble,bge,jal,lw,sw,reti,in,out,halt
}opcode;


struct instruction {
	opcode opcode;
	reg rd;
	reg rs;
	reg rt;
	int imm;
};

struct CPU {
	int PC;
	struct instruction *inst;
	int registers[NUM_OF_REGISTERS];
	int IORegisters[NUM_OF_IORegisters];
	unsigned int *memory;
};


struct CPU * sim_init();
void fetch_address(struct CPU *cpu);
int signExtension(int instruction);
unsigned int *parseMemin(char *memory_file);
void memoryOut(unsigned int *memory, char *out_file);
void parseDisk(unsigned int(*disk)[WORDS_IN_SECTOR], char *disk_file);
void diskOut(unsigned int(*disk)[WORDS_IN_SECTOR], char *out_file);

void (*operation[NUM_OF_OPCODES]) (struct CPU *cpu);

void add_op(struct CPU *cpu);
void sub_op(struct CPU *cpu);
void and_op(struct CPU *cpu);
void or_op(struct CPU *cpu);
void sll_op(struct CPU *cpu);
void sra_op(struct CPU *cpu);
void srl_op(struct CPU *cpu);
void beq_op(struct CPU *cpu);
void beq_op(struct CPU *cpu);
void bne_op(struct CPU *cpu);
void blt_op(struct CPU *cpu);
void bgt_op(struct CPU *cpu);
void ble_op(struct CPU *cpu);
void bge_op(struct CPU *cpu);
void jal_op(struct CPU *cpu);
void lw_op(struct CPU *cpu);
void sw_op(struct CPU *cpu);
void reti_op(struct CPU *cpu);
void in_op(struct CPU *cpu);
void out_op(struct CPU *cpu);
void halt_op(struct CPU *cpu);

void initializeOperators();
void executeInstruction(struct CPU *cpu);


int main(int argc, char** argv)
{
	//init cpu struct
	struct CPU *cpu =sim_init();
	//parse input files
	cpu->memory = parseMemin(argv[1]);
	unsigned int(*disk)[WORDS_IN_SECTOR]= (unsigned int(*)[WORDS_IN_SECTOR])malloc(sizeof(*disk)*DISK_SECTORS);
	int i;
	//initiliaze each sector to 0
	for (i = 0; i < DISK_SECTORS; i++)
	{
		memset(&disk[i], 0x00000000, WORDS_IN_SECTOR * sizeof(unsigned int));
	}
	parseDisk(disk,argv[2]);
	//unsigned int irq2_occurences=parseIrq2(argv[3]);

	fetch_address(cpu);
	while (cpu->inst->opcode != halt) //19
	{
		executeInstruction(cpu);
		fetch_address(cpu);
	}

	memoryOut(cpu->memory, argv[4]);
	diskOut(disk, argv[11]);

}


unsigned int *parseMemin(char *memory_file)
{
	unsigned int *memory=(int *)malloc(sizeof(int)*MAX_MEM_ADDRS);
	FILE *memin_file_desc;
	memin_file_desc = fopen(memory_file, "r");
	assert(memin_file_desc != NULL);
	int i = 0;
	while (fscanf(memin_file_desc, "%X", memory+i) != EOF)
	{
		i += 1;
	}
	while (i < MAX_MEM_ADDRS)
	{
		memory[i] = 0x00000000;
		i += 1;
	}
	fclose(memin_file_desc);
	return memory;
}

void memoryOut(unsigned int *memory, char *out_file)
{
	FILE *memout_file_desc;
	memout_file_desc = fopen(out_file, "w");
	assert(memout_file_desc != NULL);
	int i;
	for (i = 0; i < MAX_MEM_ADDRS; i++)
	{
		fprintf(memout_file_desc, "%X\n", memory[i]);	//is it ok if diskout is empty to return 0x000000 for all addrs?												
	}
	fclose(memout_file_desc);
}


void parseDisk(unsigned int (*disk)[WORDS_IN_SECTOR],char *disk_file)
{
	FILE *disk_file_desc;
	disk_file_desc = fopen(disk_file, "r");
	assert(disk_file_desc != NULL);
	int i = 0,j=0;
	while (fscanf(disk_file_desc, "%08X", &(disk[i][j])) != EOF)
	{
		j += 1;
		if (j % 128 == 0) {
			i += 1; //assuming number of lines in file is less than 128*128
			j = 0;
		}
	}
	fclose(disk_file_desc);
}

void diskOut(unsigned int(*disk)[WORDS_IN_SECTOR], char *out_file)
{
	FILE *disk_file_desc;
	disk_file_desc = fopen(out_file, "w");
	assert(disk_file_desc != NULL);
	int i,j;
	for (i = 0; i < DISK_SECTORS; i++)
	{
		for (j = 0; j <WORDS_IN_SECTOR; j++)
		{
			fprintf(disk_file_desc, "%08X\n", disk[i][j]);
		}
	}
	fclose(disk_file_desc);
}


struct CPU * sim_init()
{
	struct CPU *cpu = (struct CPU*)malloc(sizeof(struct CPU));
	cpu->inst = (struct instruction*)malloc(sizeof(struct instruction));
	cpu->PC = 0;
	initializeOperators();
	cpu->registers[$zero] = 0;
	return cpu;
}

void fetch_address(struct CPU *cpu)
{
	unsigned int current_instruction = cpu->memory[cpu->PC];
	cpu->inst->opcode = (current_instruction >>24)& OPCODE_MASK;
	cpu->inst->rd= (current_instruction >>20) & REGISTER_MASK;
	cpu->inst->rs= (current_instruction >> 16) & REGISTER_MASK;
	cpu->inst->rt= (current_instruction >> 12) & REGISTER_MASK;
	cpu->inst->imm= signExtension(current_instruction);
	cpu->registers[$imm] = cpu->inst->imm;
}

int signExtension(int instruction)  //from 3 bytes to 8 bytes
{
	int value = (0x00000FFF & instruction);
	int mask = 0x00000800;
	if (mask & instruction) {
		value += 0xFFFFF000;
	}
	return value;
}

void add_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero|| cpu->inst->rd==$imm)
	{
		cpu->PC += 1;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt];
	cpu->PC += 1;
}


void sub_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC += 1;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] - cpu->registers[cpu->inst->rt];
	cpu->PC += 1;
}

void and_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC += 1;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] & cpu->registers[cpu->inst->rt];
	cpu->PC += 1;
}

void or_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC += 1;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] | cpu->registers[cpu->inst->rt];
	cpu->PC += 1;
}

void sll_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC += 1;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] << cpu->registers[cpu->inst->rt];
	cpu->PC += 1;
}


void sra_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC += 1;
		return;
	}
	//arithmetic shift with sign extension
	if (cpu->registers[cpu->inst->rs] < 0 && cpu->registers[cpu->inst->rt] > 0)
	{
		cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] >> cpu->registers[cpu->inst->rt] | ~(~0U >> cpu->registers[cpu->inst->rt]);
	}
	else
	{
		cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] >> cpu->registers[cpu->inst->rt];

	}
	cpu->PC += 1;
}

void srl_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC += 1;
		return;
	}
	//logical shift with sign extension
	cpu->registers[cpu->inst->rd] = (unsigned)(cpu->registers[cpu->inst->rs]) >> cpu->registers[cpu->inst->rt];
	cpu->PC += 1;
}

void beq_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] == cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
}

void bne_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] != cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
}

void blt_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] < cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
}

void bgt_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] > cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
}

void ble_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] <= cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
}

void bge_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] >= cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK; //address mask
	}
}

void jal_op(struct CPU *cpu)
{
	cpu->registers[$ra] = cpu->PC + 1;
	cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
}

void lw_op(struct CPU *cpu)
{
	cpu->registers[cpu->inst->rd] = cpu->memory[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]];
	cpu->PC += 1;
}

void sw_op(struct CPU *cpu)
{
	cpu->memory[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]] = cpu->registers[cpu->inst->rd];
	cpu->PC += 1;
}

void reti_op(struct CPU *cpu)
{
	cpu->PC = cpu->IORegisters[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]];
}


void in_op(struct CPU *cpu)
{
	cpu->registers[cpu->inst->rd] = cpu->IORegisters[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]];
	cpu->PC += 1;
}

void out_op(struct CPU *cpu)
{
	cpu->IORegisters[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]] = cpu->registers[cpu->inst->rd];
	cpu->PC += 1;
}

void halt_op(struct CPU *cpu)
{
	return;
}


void initializeOperators()	//intializing an array of pointers to functions
{
	operation[add] = add_op; //0
	operation[sub] = sub_op; //1
	operation[and] = and_op; //2
	operation[or] = or_op;	 //3
	operation[sll ] = sll_op; //4
	operation[sra] = sra_op; //5
	operation[srl] = srl_op; //6
	operation[beq] = beq_op; //7
	operation[bne] = bne_op; //8
	operation[blt] = blt_op; //9
	operation[bgt] = bgt_op; //10
	operation[ble] = ble_op; //11
	operation[bge] = bge_op; //12
	operation[jal] = jal_op; //13
	operation[lw] = lw_op; //14
	operation[sw] = sw_op; //15
	operation[reti] = reti_op; //16
	operation[in] = in_op; //17
	operation[out] = out_op; //18
	operation[halt] =halt_op; //19
}

void executeInstruction(struct CPU *cpu)
{
	(*operation[cpu->inst->opcode]) (cpu);
}
