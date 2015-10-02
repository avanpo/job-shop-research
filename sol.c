#include <stdio.h>

#include "instance.h"
#include "graph.h"
#include "schedule.h"

int main(int argc, char **argv)
{
	char *fname = "instances/1.txt";
	if (argc == 2) {
		fname = argv[1];
	}

	struct instance *inst = read_inst(fname);
	struct graph *graph = construct_graph(inst);

	init_graph(graph);
	serialize_graph(graph);

	print_schedule_labeled(graph->schedule);
	print_longest_path(graph);

	destroy_graph(graph);
	destroy_inst(inst);

	return 0;
}
