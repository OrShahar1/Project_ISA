#include "operations.h"

#include <stdbool.h>

#define BRANCH_MASK 0x00000FFF //12 rightmost bit mask for branch operations, represent the memory space

void add_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}


void sub_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] - cpu->registers[cpu->inst->rt];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void and_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] & cpu->registers[cpu->inst->rt];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void or_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] | cpu->registers[cpu->inst->rt];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void sll_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
		return;
	}
	cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] << cpu->registers[cpu->inst->rt];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}


void sra_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
		return;
	}
	//arithmetic shift with sign extension
	if (cpu->registers[cpu->inst->rs] < 0 && cpu->registers[cpu->inst->rt] > 0)
	{
		int msb_mask = 1 << (31 - cpu->registers[cpu->inst->rt]);
		int result = (cpu->registers[cpu->inst->rs] & 0x7FFFFFFF) >> cpu->registers[cpu->inst->rt];
		int i,mask=0;
		for (i = 0; i < cpu->registers[cpu->inst->rt]; i++)
		{
			mask=mask| (1 << (31 - i));
		}
		result = (result | msb_mask)|mask;
		cpu->registers[cpu->inst->rd] = result;
	}
	else  //positive number, same operator as srl
	{
		cpu->registers[cpu->inst->rd] = cpu->registers[cpu->inst->rs] >> cpu->registers[cpu->inst->rt];

	}
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void srl_op(struct CPU *cpu)
{
	if (cpu->inst->rd == $zero || cpu->inst->rd == $imm)
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
		return;
	}
	//logical shift
	cpu->registers[cpu->inst->rd] = (unsigned)(cpu->registers[cpu->inst->rs]) >> cpu->registers[cpu->inst->rt];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void beq_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] == cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
	else
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
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
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
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
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
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
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
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
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
	}
}

void bge_op(struct CPU *cpu)
{
	if (cpu->registers[cpu->inst->rs] >= cpu->registers[cpu->inst->rt])
	{
		cpu->PC = cpu->registers[cpu->inst->rd] & BRANCH_MASK;
	}
	else
	{
		cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
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
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void sw_op(struct CPU *cpu)
{
	cpu->memory[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]] = cpu->registers[cpu->inst->rd];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void reti_op(struct CPU *cpu)
{
	cpu->PC = cpu->IORegisters[irqreturn];
	cpu->is_in_irq = false; //to let the cpu know we returned from interrupt
}


void in_op(struct CPU *cpu)
{
	cpu->registers[cpu->inst->rd] = cpu->IORegisters[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]];
	cpu->PC = (cpu->PC+ 1)% MAX_MEM_ADDRS;
}

void out_op(struct CPU *cpu)
{
	cpu->IORegisters[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]] = cpu->registers[cpu->inst->rd];
	cpu->PC = (cpu->PC + 1) % MAX_MEM_ADDRS;
}

void halt_op(struct CPU *cpu)
{
	return; //no action
}
