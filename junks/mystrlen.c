#include <stdio.h>
#include <string.h>


int main() {
	char word[] = "hello there, this is a longer sort of string";
	printf("%li\n", sizeof(word));
	printf("%li\n", strlen(word));
	return 0;
}
