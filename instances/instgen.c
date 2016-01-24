#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char **argv)
{
	srand(time(NULL));

	int size = 0;
	int width = 0;
	if (argc != 4 || sscanf(argv[2], "%d", &size) != 1 || sscanf(argv[3], "%d", &width) != 1 ||
			size < 1 || size > 5 || width < 1 || width > 2) {
		fprintf(stderr, "Please provide correct arguments:\n  ./instgen OUTPUT_FILE_NAME SIZE WIDTH\nWhere SIZE is an integer in the range [1,5] and WIDTH is an integer in the range [1,2].\n");
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
	int num_types = 1 + size;
	int num_machines_low = size * width;
	int num_machines_high = size * width * 2;

	int num_jobs = (int)pow(2, size + 2);
	int num_ops = (int)pow(2, size + 2);

	int i, j;
	int n, type, proc, idle;
	fprintf(fp, "types %d\n", num_types);
	for (i = 0; i < num_types; ++i) {
		n = (rand() % (num_machines_high - num_machines_low)) + num_machines_low;
		fprintf(fp, "%d\n", n);
	}
	fprintf(fp, "jobs %d\n", num_jobs);
	for (i = 0; i < num_jobs; ++i) {
		fprintf(fp, "ops %d\n", num_ops);
		for (j = 0; j < num_ops; ++j) {
			type = rand() % num_types;
			proc = (rand() % (proc_low - proc_high)) + proc_low;
			idle = (rand() % (idle_low - idle_high)) + idle_low;
			fprintf(fp, "%d %d %d\n", type, proc, idle);
		}
	}
	
	fclose(fp);
	return 0;
}
