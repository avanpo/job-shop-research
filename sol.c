#include "instance.h"
#include "graph.h"

int main(int argc, char **argv)
{
	char *fname = "instances/1.txt";
	if (argc == 2) {
		fname = argv[1];
	}

	struct instance *inst = read_inst(fname);
	struct graph *grph = construct_graph(inst);

	destroy_inst(inst);
	destroy_graph(grph);

	return 0;
}
