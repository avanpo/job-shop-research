#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char **argv)
{
	srand(time(NULL));

	int size = 0;
	int width = 0;
	if (argc != 4 || scanf(argv[2], &size) != 1 || scanf(argv[3], &width) != 1 ||
			size < 1 || size > 5 || width < 1 || width > 3) {
		fprintf(stderr, "Please provide correct arguments:\n  ./instgen output-file-name size width\nWhere size is an integer in the range [1,5] and width is an integer in the range [1,3].\n");
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(argv[1], "w");
	if (fp == NULL) {
		fprintf(stderr, "File \"%s\" failed to open.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// op time settings
	int proc_low = 1;
	int proc_high = 10;
	int idle_low = 0;
	int idle_high = 10;

	// calculate sizes, ranges
	int num_types = (int)pow(2, size);
	int num_machines_low = size * width;
	int num_machines_high = size * width * 2;

	int num_jobs = (int)pow(2, size + 1);
	int num_ops_low = (int)pow(2, size);
	int num_ops_high = (int)pow(2, size + 2);

	int i, j;
	int n, type, proc, idle;
	fprintf(fp, "types %d\n", num_types);
	for (i = 0; i < num_types; ++i) {
		n = (rand() % (num_machines_high + num_machines_low)) + num_machines_low;
		fprintf(fp, "%d\n", n);
	}
	fprintf(fp, "jobs %d\n", num_jobs);
	for (i = 0; i < num_jobs; ++i) {
		n = (rand() % (num_ops_low + num_ops_high)) + num_ops_low;
		fprintf(fp, "ops %d\n", n);
		for (j = 0; j < n; ++j) {
			type = rand() % num_types;
			proc = (rand() % (proc_low + proc_high)) + proc_low;
			idle = (rand() % (idle_low + idle_high)) + idle_low;
			fprintf(fp, "%d %d %d\n", type, proc, idle);
		}
	}
	
	fclose(fp);
	return 0;
}
