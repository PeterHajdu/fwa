#include <stdio.h>
#include <stdlib.h>

void usage() {
	printf("usage: fwa <list of files to watch>\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	if (argc < 2)
		usage();

	printf("%s\n%s\n", argv[1], argv[2]);
	return 0;
}
