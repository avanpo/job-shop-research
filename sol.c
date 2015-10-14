#include <stdio.h>

#include "instance.h"
#include "graph.h"
#include "schedule.h"
#include "search.h"

int main(int argc, char **argv)
{
	char *fname = "instances/17.txt";
	int restarts = 0;
	if (argc == 2 || argc == 3) {
		fname = argv[1];
	}
	if (argc == 3) {
		sscanf(argv[2], "%d", &restarts);
	}

	struct instance *inst = read_inst(fname);
	struct sa_state *sa = construct_sa_search(inst);

	start_sa_search(sa, restarts);

	destroy_sa_search(sa);
	destroy_inst(inst);

	return 0;
}
