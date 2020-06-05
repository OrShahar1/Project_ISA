#ifndef FILESMANAGER_H
#define FILESMANAGER_H

#include <stdio.h>
#include <stdbool.h>

#define WORDS_IN_SECTOR 128 // 512 bytes/4=128 words
#define DISK_SECTORS 128

struct irq2 {
	unsigned int next_occurence;
	bool is_file_read_completed;
	FILE *file_desc;
};


unsigned int *parseMemin(char *memory_file);
void fileOut(unsigned int *data, char *out_file, int start_index, int length);
void parseDisk(unsigned int**disk, char *disk_file);
void diskOut(unsigned int**disk, char *out_file);
void cyclesOut(int cycles, char *out_file);
struct irq2 *Irq2Init(char * irq2_file);

#endif // FILESMANAGER_H
