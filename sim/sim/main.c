#include "cpu.h"
#include "filesManager.h"
#include "operations.h"

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define DISK_PROCESS_TIME 1024

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
	unsigned int **disk= (unsigned int**)malloc(sizeof(unsigned int*)*DISK_SECTORS);
	//initiliaze each sector to 0
	int i;
	for (i = 0; i < DISK_SECTORS; i++)
	{
		disk[i] = (unsigned int*)malloc(sizeof(unsigned int)*WORDS_IN_SECTOR);
		for (int j = 0; j < WORDS_IN_SECTOR; j++)
		{
			disk[i][j] = 0;
		}
	}
	parseDisk(disk,argv[2]);
	struct irq2 *irq2= Irq2Init(argv[3]);

	//open trace,leds,display files
	FILE *trace_file_desc,*leds_file_desc,*display_file_desc,*hwreg_file_desc;
	trace_file_desc = fopen(argv[6], "w");
	assert(trace_file_desc != NULL);
	hwreg_file_desc = fopen(argv[7], "w");
	assert(hwreg_file_desc != NULL);
	leds_file_desc = fopen(argv[9], "w");
	assert(leds_file_desc != NULL);
	display_file_desc = fopen(argv[10], "w");
	assert(display_file_desc != NULL);
	i = 0;
	unsigned int disk_process_counter=0;
	while (cpu->inst->opcode != halt) //halt=19;	true for first time initalized opcode to 0 from sim_init
	{
		//handle all interrupts
		int irq = (cpu->IORegisters[irq0enable] && cpu->IORegisters[irq0status]) ||
			(cpu->IORegisters[irq1enable] && cpu->IORegisters[irq1status]) ||
			(cpu->IORegisters[irq2enable] && cpu->IORegisters[irq2status]);
		if (irq == 1 && (!cpu->is_in_irq)) //there is no other irq at the moment
		{
			cpu->is_in_irq = true;
			cpu->IORegisters[irqreturn] = cpu->PC;
			cpu->PC = cpu->IORegisters[irqhandler];
		}
		fetch_address(cpu); //fetch the correct PC instruction with/without iterrupt
		write_trace(cpu, trace_file_desc);
		
		//turn on irq2status according to irq2.txt
		if (!irq2->is_file_read_completed)
		{
			if (irq2->next_occurence == cpu->IORegisters[clks])
			{		
				cpu->IORegisters[irq2status] = 1;
				if (fscanf(irq2->file_desc, "%u", &(irq2->next_occurence)) == EOF)
				{
					irq2->is_file_read_completed = true;
				}
			}
		}
		if (cpu->inst->opcode == out)
		{
			//hwregtrace.txt WRITE
			fprintf(hwreg_file_desc, "%d WRITE %s %08x\n", cpu->IORegisters[clks],
				IOregisters_names[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]], cpu->registers[cpu->inst->rd]);
			
			//write leds.txt
			if (cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt] == leds &&(cpu->IORegisters[leds])!=cpu->registers[cpu->inst->rd])
			{	//print leds only when there is MODIFICATION in leds
				fprintf(leds_file_desc, "%d %08x\n", cpu->IORegisters[clks], cpu->registers[cpu->inst->rd]);
			}
			//write display.txt
			else if (cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt] == display && (cpu->IORegisters[display]) != cpu->registers[cpu->inst->rd])
			{  //print display only when there is MODIFICATION in display
				fprintf(display_file_desc, "%d %08x\n", cpu->IORegisters[clks], cpu->registers[cpu->inst->rd]);

			}
		}
		else if (cpu->inst->opcode == in)
		{
			//hwregtrace.txt READ
			fprintf(hwreg_file_desc, "%d READ %s %08x\n", cpu->IORegisters[clks],
				IOregisters_names[cpu->registers[cpu->inst->rs] + cpu->registers[cpu->inst->rt]], cpu->IORegisters[cpu->registers[cpu->inst->rs]+ cpu->registers[cpu->inst->rt]]);
		}
		//execute opretion. NOTE: must be before handle disk
		executeInstruction(cpu);
		//handle disk
		if (cpu->IORegisters[diskcmd] != 0 && cpu->IORegisters[diskstatus] ==0)
		{
			cpu->IORegisters[diskstatus] = 1; //busy disk
			if (cpu->IORegisters[diskcmd] == 1) //READ sector
			{
				for (i = 0; i < WORDS_IN_SECTOR; i++)
				{
					cpu->memory[(cpu->IORegisters[diskbuffer] + i)% MAX_MEM_ADDRS] = disk[cpu->IORegisters[disksector]][i];

				}
			}
			else //WRITE sector
			{
				for (i = 0; i < WORDS_IN_SECTOR; i++)
				{
					disk[cpu->IORegisters[disksector]][i] = cpu->memory[(cpu->IORegisters[diskbuffer] + i)% MAX_MEM_ADDRS];
				}
			}
		}
		if (cpu->IORegisters[diskstatus] == 1) //busy disk
		{
			if (disk_process_counter >= DISK_PROCESS_TIME)
			{
				//free disk and raise irq1
				cpu->IORegisters[diskcmd] = 0;
				cpu->IORegisters[diskstatus] = 0;
				cpu->IORegisters[irq1status] = 1;
				disk_process_counter = 0;
			}
			else  //still counting untill DISK_PROCESS_TIME cycles is reached
			{
				disk_process_counter += 1;
			}
		}
		fetch_address(cpu);
	}
	write_trace(cpu, trace_file_desc); //for halt instruction
	executeInstruction(cpu); //for halt instruction

	//write out files
	cyclesOut(cpu->IORegisters[clks], argv[8]); //cycles
	fileOut(cpu->memory, argv[4],0, MAX_MEM_ADDRS); //memout
	fileOut(cpu->registers, argv[5],2, NUM_OF_REGISTERS); //regout
	diskOut(disk, argv[11]);  //diskout
	//close all file descriptors
	fclose(trace_file_desc);
	fclose(leds_file_desc);
	fclose(display_file_desc);
	fclose(hwreg_file_desc);
	fclose(irq2->file_desc);
	//free all allocated memory in simulator
	freeCPU(cpu);
	free(irq2);
	for (i = 0; i < DISK_SECTORS; i++)
	{
		free(disk[i]);
	}
	free(disk);
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
