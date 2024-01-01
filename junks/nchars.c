#include <stdio.h>
#include <stdlib.h>

void nope() { puts("fuck you"); exit(1); }

int main(int argc, char *argv[]) {
	if (argc != 2) {
		nope();
	}

	long int count = strtol(argv[1], 0, 10);

	if (count < 0) {
		nope();
	}

	for (int i = 0; i < count; i++) {
		putchar('a');
	}
	putchar('\n');

	return 0;
}

