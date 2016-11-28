#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.h"

int read_matrix(const char *file_name, int *m , int max){
	FILE *f = fopen(file_name, "rt");
	if (f == NULL) {
		perror("File io");
		return EXIT_FAILURE;
	}
	int i = 0;

	while (!feof(f) && i < max ) {
		fscanf(f, "%d", &m[i++]);
	}
	fclose(f);
	return EXIT_SUCCESS;
}