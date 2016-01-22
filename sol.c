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
			if (neighborhood < 0 || neighborhood > 2) ++error;
		} else if (argv[i][1] == 'v' && argv[i][2] == '1') {
			++verbose;
		} else if (argv[i][1] == 'v' && argv[i][2] == '2') {
			verbose = 2;
		} else if (argv[i][1] == 'd') {
			++draw;
		} else if (argv[i][1] == 'w') {
			++write;
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
		printf("  -b            Set whether jobs are blocking or not\n");
		printf("  -n VAL        Set the neighborhood. VAL can contain the following:\n");
		printf("                  VAL=0  Only swaps on the critical path (default)\n");
		printf("                  VAL=1  Only naive swaps\n");
		printf("                  VAL=2  A combination of the above\n");
		printf("  -v1           Set the verbose option\n");
		printf("  -v2           Set the very verbose option\n");
		printf("  -d            Draw schedule and instance on search finish\n");
		printf("  -w            Write the solution to file.\n");
		return 0;
	}

	struct instance *inst = read_inst(fname);
	struct sa_state *sa = construct_sa_search(inst, verbose, draw, blocking, neighborhood, write);

	start_sa_search(sa, restarts);

	destroy_sa_search(sa);
	destroy_inst(inst);

	return 0;
}
