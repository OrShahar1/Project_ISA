#include "filesManager.h"
#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


unsigned int *parseMemin(char *memory_file)
{
	unsigned int *memory = (int *)malloc(sizeof(int)*MAX_MEM_ADDRS);
	FILE *memin_file_desc;
	memin_file_desc = fopen(memory_file, "r");
	assert(memin_file_desc != NULL);
	int i = 0;
	while (fscanf(memin_file_desc, "%X", memory + i) != EOF)
	{
		i += 1;
	}
	while (i < MAX_MEM_ADDRS)
	{
		memory[i] = 0;
		i += 1;
	}
	fclose(memin_file_desc);
	return memory;
}

void fileOut(unsigned int *data, char *out_file, int start_index, int length)
{
	FILE *file_desc;
	file_desc = fopen(out_file, "w");
	assert(file_desc != NULL);
	int i;
	for (i = start_index; i < length; i++)
	{
		fprintf(file_desc, "%08X\n", data[i]);												
	}
	fclose(file_desc);
}


void parseDisk(unsigned int**disk, char *disk_file)
{
	FILE *disk_file_desc;
	disk_file_desc = fopen(disk_file, "r");
	assert(disk_file_desc != NULL);
	int i = 0, j = 0;
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

void diskOut(unsigned int**disk, char *out_file)
{
	FILE *disk_file_desc;
	disk_file_desc = fopen(out_file, "w");
	assert(disk_file_desc != NULL);
	int i, j;
	for (i = 0; i < DISK_SECTORS; i++)
	{
		for (j = 0; j <WORDS_IN_SECTOR; j++)
		{
			fprintf(disk_file_desc, "%08X\n", disk[i][j]);
		}
	}
	fclose(disk_file_desc);
}


void cyclesOut(int cycles, char *out_file)
{
	FILE *cycles_file_desc;
	cycles_file_desc = fopen(out_file, "w");
	assert(cycles_file_desc != NULL);
	fprintf(cycles_file_desc, "%d\n", cycles);
	fclose(cycles_file_desc);
}


struct irq2 *Irq2Init(char * irq2_file)
{
	struct irq2 *irq2  = (struct irq2 *)malloc(sizeof(struct irq2));
	irq2->is_file_read_completed = false;
	irq2->file_desc = fopen(irq2_file, "r");
	assert(irq2->file_desc != NULL);

	if (fscanf(irq2->file_desc, "%u", &(irq2->next_occurence)) == EOF)
	{
		irq2->is_file_read_completed = true;
	}
	return irq2;
}
