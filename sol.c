#include <stdio.h>
#include <string.h>

#include "instance.h"
#include "graph.h"
#include "schedule.h"
#include "search.h"

int main(int argc, char **argv)
{
	char fname[256] = {0};
	int fset = 0;
	int error = 0;
	int restarts = 0;
	int verbose = 0;
	int draw = 0;
	int blocking = 0;
	int neighborhood = 0;
	int write = 0;
	double alpha = 0.99;

	int i;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-') ++error;

		if (argv[i][1] == 'f') {
			if (argv[i + 1][0] == '-') ++error;

			strncpy(fname, argv[++i], 255);
			++fset;
		} else if (argv[i][1] == 'r') {
			if (argv[i + 1][0] == '-') ++error;

			sscanf(argv[++i], "%d", &restarts);
		} else if (argv[i][1] == 'b') {
			++blocking;
		} else if (argv[i][1] == 'n') {
			if (argv[i + 1][0] == '-') ++error;

			sscanf(argv[++i], "%d", &neighborhood);
			if (neighborhood < 0 || neighborhood > 4) ++error;
		} else if (argv[i][1] == 'v' && argv[i][2] == '1') {
			++verbose;
		} else if (argv[i][1] == 'v' && argv[i][2] == '2') {
			verbose = 2;
		} else if (argv[i][1] == 'd') {
			++draw;
		} else if (argv[i][1] == 'w') {
			++write;
		} else if (argv[i][1] == 'a') {
			if (argv[i + 1][0] == '-') ++error;

			sscanf(argv[++i], "%lf", &alpha);
			if (alpha < 0.01 || alpha > 0.99999) ++error;
		} else {
			++error;
		}
	}

	if (error || !fset) {
		printf("Sol - Run local search on a job shop problem instance.\n");
		printf("Usage:\n");
		printf("  \033[1m./sol\033[0m [OPTIONS]... -f INSTANCE_FILE\n");
		printf("Options:\n");
		printf("  -r RESTARTS   Set the number of restarts\n");
		printf("  -a VAL        Set the cooling parameter alpha to VAL.\n");
		printf("  -b            Set whether jobs are blocking or not\n");
		printf("  -n VAL        Set the neighborhood. VAL can contain the following:\n");
		printf("                  VAL=0  Only swaps on the critical path (default)\n");
		printf("                  VAL=1  Only left shift operations\n");
		printf("                  VAL=2  Only single operation shifts\n");
		printf("                  VAL=3  Left shift and single shift neighborhoods\n");
		printf("                  VAL=4  All of the above\n");
		printf("  -v1           Set the verbose option\n");
		printf("  -v2           Set the very verbose option\n");
		printf("  -d            Draw schedule and instance on search finish\n");
		printf("  -w            Write the solution to file.\n");
		return 0;
	}

	struct instance *inst = read_inst(fname);
	struct sa_state *sa = construct_sa_search(inst, verbose, draw, blocking, neighborhood, write, alpha);

	start_sa_search(sa, restarts);

	destroy_sa_search(sa);
	destroy_inst(inst);

	return 0;
}
