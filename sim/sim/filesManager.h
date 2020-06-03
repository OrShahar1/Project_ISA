#ifndef FILESMANAGER_H
#define FILESMANAGER_H

#define MAX_MEM_ADDRS 4096
#define WORDS_IN_SECTOR 128 // 512 bytes/4=128 words
#define MAX_IRQ2_LENGTH 100 //TODO
#define DISK_SECTORS 128

struct irq2_occurences {
	unsigned int values[MAX_IRQ2_LENGTH]; //check maximal length
	int length;
};


unsigned int *parseMemin(char *memory_file);
void fileOut(unsigned int *data, char *out_file, int start_index, int length);
void parseDisk(unsigned int**disk, char *disk_file);
void diskOut(unsigned int**disk, char *out_file);
void cyclesOut(int cycles, char *out_file);
struct irq2_occurences *parseIrq2(char * irq2_file);

#endif // FILESMANAGER_H
