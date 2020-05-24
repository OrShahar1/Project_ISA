#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check() {
	FILE* memin = fopen("memin.txt", "r");
	FILE* ref = fopen("fib_ref.txt", "r");
	char line1[501];
	char line2[501];


	while (fgets(line1, 501, memin) != NULL && fgets(line2, 501, ref) != NULL) {
		if (strcmp(line1, line2) != 0) {
			printf("files dont match:\n%s\n%s\n", line1, line2);
			return 1;
		}
	}

	return 0;
}

int add_to_command(int arg, const char* type) {
	int shift = 0;
		if(type=="opcode") {
			shift = 24;
		}
		if(type=="rd") {
			shift = 20;
		}
		if(type=="rs") {
			shift = 16;
		}
		if(type=="rt") {
			shift = 12;
		}
	return arg << shift;
};