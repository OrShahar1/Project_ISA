#ifndef  CPU_H
#define CPU_H

#include "operations.h"
#include <stdbool.h>

#define MAX_MEM_ADDRS 4096

typedef enum reg {
	$zero = 0,
	$imm, $v0, $a0, $a1, $t0, $t1, $t2, $t3, $s0, $s1, $s2, $gp, $sp, $fp, $ra,
	NUM_OF_REGISTERS
}reg;

typedef enum opcode {
	add = 0,
	sub, and, or , sll, sra, srl, beq, bne, blt, bgt, ble, bge, jal, lw, sw, reti, in, out, halt,
	NUM_OF_OPCODES
}opcode;

typedef enum IOreg {
	irq0enable = 0,
	irq1enable, irq2enable, irq0status, irq1status, irq2status, irqhandler, irqreturn, clks, leds, display, timerenable,
	timercurrent, timermax, diskcmd, disksector, diskbuffer, diskstatus,
	NUM_OF_IOREGISTERS
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
	bool is_in_irq;
};

//pointer to array of operation functions declartion
void(*operation[NUM_OF_OPCODES]) (struct CPU *cpu);

struct CPU * sim_init();
void fetch_address(struct CPU *cpu);
void executeInstruction(struct CPU *cpu);
void freeCPU(struct CPU *cpu);

#endif // ! CPU_H
