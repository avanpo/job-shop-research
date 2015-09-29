#include "instance.h"
#include "graph.h"

struct graph *construct_graph(struct instance *inst)
{
	struct r_graph *graph = calloc(1, sizeof(struct r_graph));
	struct node *nodes = calloc(inst->num_ops, sizeof(struct node));

	graph->instance = inst;
	graph->nodes = nodes;

	int i, j;
	for (i = 0; i < inst->num_ops; i++, nodes++) {
		nodes->id = i;
		nodes->op = inst->ops[i];
	}

	for (i = 0; i < inst->num_jobs; i++) {
		for (j = 0; j < inst->jobs[i].num_ops - 1; j++) {
			graph->arcs[inst->jobs[i].ops[j]][inst->jobs[i].ops[j + 1]] = JOB_ARC;
		}
	}

	return graph;
}

void *destroy_graph(struct graph *graph)
{
	free(graph->nodes);
	free(graph);
}
