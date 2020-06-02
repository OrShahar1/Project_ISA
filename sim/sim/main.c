#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MEM_ADDRS 4096
#define NUM_OF_REGISTERS 16
#define NUM_OF_IOREGISTERS 18
#define DISK_SECTORS 128
#define WORDS_IN_SECTOR 128 // 512 bytes/4=128 words
#define OPCODE_MASK 0x000000FF
#define REGISTER_MASK 0x0000000F
#define BRANCH_MASK 0x00000FFF //8=1000 to keep the sign of imm
#define NUM_OF_OPCODES 19
#define CLOCK_LIMIT 0xffffffff

typedef enum reg {
	$zero = 0,
	$imm, $v0, $a0, $a1, $t0, $t1, $t2, $t3, $s0, $s1, $s2, $gp, $sp, $fp, $ra
}reg;

typedef enum opcode {
	add = 0,
	sub,and,or,sll,sra,srl,beq,bne,blt,bgt,ble,bge,jal,lw,sw,reti,in,out,halt
}opcode;

typedef enum IOreg {
	irq0enable = 0,
	irq1enable,irq2enable,irq0status,irq1status,irq2status,irqhandler,irqreturn,clks,leds,display,timerenable,
	timercurrent,timermax,diskcmd,disksector,diskbuffer,diskstatus
}IOreg;


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
	int IORegisters[NUM_OF_IOREGISTERS];
	unsigned int *memory;
};


struct CPU * sim_init();
void fetch_address(struct CPU *cpu);
int signExtension(int instruction);
unsigned int *parseMemin(char *memory_file);
void fileOut(unsigned int *data, char *out_file, int start_index, int length);
void parseDisk(unsigned int(*disk)[WORDS_IN_SECTOR], char *disk_file);
void diskOut(unsigned int(*disk)[WORDS_IN_SECTOR], char *out_file);
void cyclesOut(int cycles, char *out_file);

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
void write_trace(struct CPU *cpu, FILE *trace_file_desc);


int main(int argc, char** argv)
{
	const char *IOregisters_names[] = { "irq0enable",
		"irq1enable","irq2enable","irq0status","irq1status","irq2status","irqhandler","irqreturn",
		"clks","leds","display","timerenable","timercurrent","timermax","diskcmd","disksector","diskbuffer","diskstatus" };
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

	//open trace,leds,display files
	FILE *trace_file_desc,*leds_file_desc,*display_file_desc,*hwreg_file_desc;
	trace_file_desc = fopen(argv[6], "w");
	assert(trace_file_desc != NULL);
	hwreg_file_desc = fopen(argv[7], "w");
	assert(hwreg_file_desc != NULL);
	leds_file_desc = fopen(argv[9], "w");
	assert(leds_file_desc != NULL);
	display_file_desc = fopen(argv[9], "w");
	assert(display_file_desc != NULL);

	fetch_address(cpu);
	while (cpu->inst->opcode != halt) //halt=19
	{
		write_trace(cpu, trace_file_desc);
		executeInstruction(cpu);

		if (cpu->inst->opcode == out)
		{
			//write hwregtrace.txt
			fprintf(hwreg_file_desc, "%d WRITE %s %08x\n", cpu->IORegisters[clks],
				(IOregisters_names[cpu->inst->rs + cpu->inst->rt]), cpu->IORegisters[cpu->inst->rs + cpu->inst->rt]);

			if (cpu->inst->rs + cpu->inst->rt == leds)
			{
				//write leds.txt
				fprintf(leds_file_desc, "%d %08x\n", cpu->IORegisters[clks], cpu->IORegisters[leds]);
			}
			else if (cpu->inst->rs + cpu->inst->rt == display)
			{
				//write display.txt
				fprintf(display_file_desc, "%d %08x\n", cpu->IORegisters[clks], cpu->IORegisters[display]);

			}
		}
		else if (cpu->inst->opcode == in)
		{
			//write hwregtrace.txt
			fprintf(hwreg_file_desc, "%d READ %s %08x\n", cpu->IORegisters[clks],
				(IOregisters_names[cpu->inst->rs + cpu->inst->rt]), cpu->IORegisters[cpu->inst->rs + cpu->inst->rt]);
		}
		fetch_address(cpu);
	}
	write_trace(cpu, trace_file_desc); //for halt instruction
	cyclesOut(cpu->IORegisters[clks], argv[8]);
	fileOut(cpu->memory, argv[4],0, MAX_MEM_ADDRS); //memout
	fileOut(cpu->memory, argv[5],2, NUM_OF_REGISTERS); //regout
	diskOut(disk, argv[11]);
	fclose(trace_file_desc);
	fclose(leds_file_desc);
	fclose(display_file_desc);
	fclose(hwreg_file_desc);
}


void write_trace(struct CPU *cpu,FILE *trace_file_desc)
{
	int i;
	fprintf(trace_file_desc, "%08X %08X ", cpu->PC, cpu->memory[cpu->PC]);
	for (i = 0; i < NUM_OF_REGISTERS - 1; i++)
	{
		fprintf(trace_file_desc, "%08x ", cpu->registers[i]);
	}
	fprintf(trace_file_desc, "%08x\n", cpu->registers[NUM_OF_REGISTERS - 1]); //last element no whitespace
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

void fileOut(unsigned int *data, char *out_file,int start_index,int length)
{
	FILE *file_desc;
	file_desc = fopen(out_file, "w");
	assert(file_desc != NULL);
	int i;
	for (i = start_index; i < length; i++)
	{
		fprintf(file_desc, "%08X\n", data[i]);	//is it ok if diskout is empty to return 0x000000 for all addrs?												
	}
	fclose(file_desc);
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
	int i;
	//intalizing all Registers to 0. not written in project instructions. but it is like that in your example trace.txt..
	for (i = 0; i < NUM_OF_REGISTERS; i++)
	{
		cpu->registers[i] = 0;
	}
	//intializing all IORegisters to 0
	for (i = 0; i < NUM_OF_IOREGISTERS; i++)
	{
		cpu->IORegisters[i] = 0;
	}
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
	else
	{
		cpu->PC += 1;
	}
}

void bne_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] != cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
	else
	{
		cpu->PC += 1;
	}
}

void blt_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] < cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
	else
	{
		cpu->PC += 1;
	}
}

void bgt_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] > cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
	else
	{
		cpu->PC += 1;
	}
}

void ble_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] <= cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
	else
	{
		cpu->PC += 1;
	}
}

void bge_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] >= cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK; //address mask
	}
	else
	{
		cpu->PC += 1;
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
	cpu->PC = cpu->IORegisters[irqreturn];
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
	int irq = (cpu->IORegisters[irq0enable] && cpu->IORegisters[irq0status]) ||
		(cpu->IORegisters[irq1enable] && cpu->IORegisters[irq1status]) ||
		(cpu->IORegisters[irq2enable] && cpu->IORegisters[irq2status]);
	if (irq == 1)
	{
		//if we are not dealing with the irq yet{
		cpu->IORegisters[irqreturn] = cpu->PC;
		cpu->PC = cpu->IORegisters[irqhandler];
		//}

	}
	else
	{
		(*operation[cpu->inst->opcode]) (cpu);
	}
	if (cpu->IORegisters[timerenable] == 1)
	{
		if (cpu->IORegisters[timercurrent] != cpu->IORegisters[timermax])
		{
			cpu->IORegisters[timercurrent] += 1;
		}
		else
		{
			cpu->IORegisters[irq0status] = 1;
			cpu->IORegisters[timercurrent] = 0;
		}
	}
	if (cpu->IORegisters[clks] == CLOCK_LIMIT)
	{
		cpu->IORegisters[clks] = 0;
	}
	else
	{
		cpu->IORegisters[clks] += 1;
	}
}

void cyclesOut(int cycles, char *out_file)
{
	FILE *cycles_file_desc;
	cycles_file_desc = fopen(out_file, "w");
	assert(cycles_file_desc != NULL);
	fprintf(cycles_file_desc, "%d\n", cycles);
	fclose(cycles_file_desc);
}
