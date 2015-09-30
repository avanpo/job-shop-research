#ifndef GRAPH_H
#define GRAPH_H

#include "instance.h"

#define NO_ARC  0
#define JOB_ARC 1
#define ORD_ARC 2

struct graph {
	struct instance *inst;
	int num_nodes;
	struct node *nodes;
	int arcs[MAX_JOBS][MAX_JOBS];
};

struct node {
	int id;
	struct operation *op;
	int start_time;
};

struct graph *construct_graph(struct instance *inst);
void destroy_graph(struct graph *graph);

void initialize_order(struct graph *graph);

#endif
