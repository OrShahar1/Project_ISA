#include "cpu.h"
#include <stdbool.h>
#include <stdlib.h>

#define OPCODE_MASK 0x000000FF
#define REGISTER_MASK 0x0000000F
#define CLOCK_LIMIT 0xffffffff

int signExtension(int instruction);
void initializeOperators();

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
	cpu->is_in_irq = false;
	cpu->inst->opcode = 0; //so that we can enter while loop in main
	return cpu;
}

void fetch_address(struct CPU *cpu)
{
	//fetches from memory array to cpu->inst
	unsigned int current_instruction = cpu->memory[cpu->PC];
	cpu->inst->opcode = (current_instruction >> 24)& OPCODE_MASK;
	cpu->inst->rd = (current_instruction >> 20) & REGISTER_MASK;
	cpu->inst->rs = (current_instruction >> 16) & REGISTER_MASK;
	cpu->inst->rt = (current_instruction >> 12) & REGISTER_MASK;
	cpu->inst->imm = signExtension(current_instruction); //signExtension supports also negative numbers in $imm
	cpu->registers[$imm] = cpu->inst->imm;
}


int signExtension(int instruction)  //from 3 bytes to 8 bytes, supports negative numbers
{
	int value = (0x00000FFF & instruction);
	int mask = 0x00000800;
	if (mask & instruction) {
		value += 0xFFFFF000;
	}
	return value;
}


void initializeOperators()	//intializing an array of pointers to functions
{
	operation[add] = add_op; //0
	operation[sub] = sub_op; //1
	operation[and] = and_op; //2
	operation[or ] = or_op;	 //3
	operation[sll] = sll_op; //4
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
	operation[halt] = halt_op; //19
}

void executeInstruction(struct CPU *cpu)
{
	//handle timer
	if (cpu->IORegisters[timerenable] == 1)
	{
		if (cpu->IORegisters[timercurrent] != cpu->IORegisters[timermax])
		{
			if (cpu->IORegisters[timercurrent] == CLOCK_LIMIT)	//if timermax initialized after timecurrent>timemax
			{
				cpu->IORegisters[timercurrent] = 0;

			}
			else
			{
				cpu->IORegisters[timercurrent] += 1;

			}
		}
		else
		{
			cpu->IORegisters[irq0status] = 1;
			cpu->IORegisters[timercurrent] = 0;
		}
	}
	if (cpu->inst->opcode < NUM_OF_OPCODES)	 //legal opcode
	{
		(*operation[cpu->inst->opcode]) (cpu);	//execute instruction with/without irq

	}
	else //undefined opcode	('ignore and continue')
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
	}
	//handle clock
	if (cpu->IORegisters[clks] == CLOCK_LIMIT)
	{
		cpu->IORegisters[clks] = 0;
	}
	else
	{
		cpu->IORegisters[clks] += 1;
	}
}


void freeCPU(struct CPU *cpu)
{
	free(cpu->inst);
	free(cpu->memory);
	free(cpu);
}
