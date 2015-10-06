#include <stdio.h>

#include "instance.h"
#include "graph.h"
#include "schedule.h"
#include "search.h"

int main(int argc, char **argv)
{
	char *fname = "instances/17.txt";
	if (argc == 2) {
		fname = argv[1];
	}

	struct instance *inst = read_inst(fname);
	struct sa_state *sa = construct_sa_search(inst);

	start_sa_search(sa);

	destroy_sa_search(sa);
	destroy_inst(inst);

	return 0;
}
