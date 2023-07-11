#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char* argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: inf tag interval\n");
	} else {
		const char* tag = argv[1];
		int interval = atoi(argv[2]);
		printf("\n");
		for(int i = 0; i < 4; i++){
			printf("%s\n", tag);
			sleep(interval);
		}
		printf("%s", tag);
	}
}

