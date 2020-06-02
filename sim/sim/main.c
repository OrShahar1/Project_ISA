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
#define IMM_MASK 0x00000FFF
				

typedef enum reg {
	$zero = 0,
	$imm, $v0, $a0, $a1, $t0, $t1, $t2, $t3, $s0, $s1, $s2, $gp, $sp, $fp, $ra
}reg;

struct SIMP_registers {
	int registers_values[NUM_OF_REGISTERS];
};


struct instruction {
	int opcode;	  //TODO
	reg rd;
	reg rs;
	reg rt;
	int imm;
};

struct CPU {
	int PC;
	struct instruction *inst;
	struct SIMP_registers registers;
	int IORegisters[NUM_OF_IORegisters];
};


struct CPU * sim_init();
void fetch_address(const unsigned int *memory, struct CPU *cpu);
unsigned int *parseMemin(char *memory_file);
void memoryOut(unsigned int *memory, char *out_file);
unsigned int(*parseDisk(char *disk_file))[DISK_SECTORS];
void diskOut(unsigned int **disk, char *out_file);


int main(int argc, char** argv)
{
	//init cpu struct
	struct CPU *cpu =sim_init();
	//parse input files
	unsigned int *memory = parseMemin(argv[1]);
	unsigned int **disk = parseDisk(argv[2]);
	//unsigned int irq2_occurences=parseIrq2(argv[3]);

	fetch_address(memory,cpu);
	while (cpu->inst->opcode != 19) //change to enum HALT
	{
		cpu->PC += 1;
		fetch_address(memory, cpu);
	}

	memoryOut(memory, argv[4]);
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


unsigned int (*parseDisk(char *disk_file))[WORDS_IN_SECTOR]
{
	unsigned int (*disk)[WORDS_IN_SECTOR];
	disk = (unsigned int(*)[WORDS_IN_SECTOR])malloc(sizeof(*disk)*DISK_SECTORS);
	int i;
	//initiliaze each sector to 0
	for (i = 0; i < DISK_SECTORS; i++)
	{
		memset(&disk[i], 0x00000000, WORDS_IN_SECTOR*sizeof(unsigned int));
	}
	FILE *disk_file_desc;
	disk_file_desc = fopen(disk_file, "r");
	assert(disk_file_desc != NULL);
	i = 0;
	int j = 0;
	while (fscanf(disk_file_desc, "%08X", &(disk[i][j])) != EOF)
	{
		j += 1;
		if (j % 128 == 0) {
			i += 1; //assuming number of lines in file is less than 128*128
			j = 0;
		}
	}
	fclose(disk_file_desc);
	return disk;
}

void diskOut(unsigned int (*disk)[WORDS_IN_SECTOR], char *out_file)
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
	return cpu;
}

void fetch_address(const unsigned int *memory, struct CPU *cpu)
{
	const unsigned int current_instruction = memory[cpu->PC];
	cpu->inst->opcode = (current_instruction >>24)& OPCODE_MASK;
	cpu->inst->rd= (current_instruction >>20) & REGISTER_MASK;
	cpu->inst->rs= (current_instruction >> 16) & REGISTER_MASK;
	cpu->inst->rt= (current_instruction >> 12) & REGISTER_MASK;
	cpu->inst->imm= current_instruction & IMM_MASK;
}
