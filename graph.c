#include <stdio.h>
#include <stdlib.h>

#include "instance.h"
#include "graph.h"

struct graph *construct_graph(struct instance *inst)
{
	struct graph *g = calloc(1, sizeof(struct graph));
	struct node *n = calloc(inst->num_ops, sizeof(struct node));

	g->inst = inst;
	g->num_nodes = inst->num_ops;
	g->nodes = n;

	int i, j;
	for (i = 0; i < inst->num_ops; ++i, ++n) {
		n->id = i;
		n->op = inst->ops + i;
	}

	for (i = 0; i < inst->num_jobs; ++i) {
		for (j = 0; j < inst->jobs[i].num_ops - 1; ++j) {
			g->arcs[inst->jobs[i].ops[j].id][inst->jobs[i].ops[j + 1].id] = JOB_ARC;
		}
	}

	return g;
}

void init_graph(struct graph *graph)
{
	
}

void destroy_graph(struct graph *graph)
{
	free(graph->nodes);
	free(graph);
}
