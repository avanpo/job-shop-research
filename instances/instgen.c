#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main(int argc, char **argv)
{
	srand(time(NULL));

	int size = 0;
	int types = 0;
	int machines = 0;
	if (argc != 5 || sscanf(argv[2], "%d", &size) != 1 || sscanf(argv[3], "%d", &types) != 1 ||
			sscanf(argv[4], "%d", &machines) != 1 || size < 1 || size > 4 ||
			types < 1 || types > 3 || machines < 1 || machines > 3) {
		fprintf(stderr, "Please provide correct arguments:\n  ./instgen OUTPUT_FILE_NAME JOBS TYPES MACHINES\nWhere JOBS is an integer in the range [1,4] and TYPES, MACHINES are integers in the range [1,3].\n");
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(argv[1], "w");
	if (fp == NULL) {
		fprintf(stderr, "File \"%s\" failed to open.\n", argv[1]);
		exit(EXIT_FAILURE);
	}


	// op time settings
	int proc_low = 1;
	int proc_high = 20 * size;
	int idle_low = 0;
	int idle_high = 0;

	// calculate sizes, ranges
	int num_types = 2 * types;
	int num_machines = 2 * machines;

	int num_jobs = (int)pow(2, 1 + size);
	int num_ops = (int)pow(2, 1 + size);

	int i, j;
	int type, proc, idle;
	fprintf(fp, "types %d\n", num_types);
	for (i = 0; i < num_types; ++i) {
		fprintf(fp, "%d\n", num_machines);
	}
	fprintf(fp, "jobs %d\n", num_jobs);
	for (i = 0; i < num_jobs; ++i) {
		fprintf(fp, "ops %d\n", num_ops);
		for (j = 0; j < num_ops; ++j) {
			type = rand() % num_types;
			proc = (rand() % (proc_high - proc_low)) + proc_low;
			idle = (rand() % ((idle_high - idle_low) > 0 ? idle_high - idle_low : 1)) + idle_low;
			fprintf(fp, "%d %d %d\n", type, proc, idle);
		}
	}
	
	fclose(fp);
	return 0;
}
