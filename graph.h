#ifndef GRAPH_H
#define GRAPH_H

#include "instance.h"

#define NO_ARC  0
#define JOB_ARC 1
#define ORD_ARC 2

struct graph {
	struct instance *inst;
	int num_factories;
	struct factory *factories;
	int num_nodes;
	struct node *nodes;
	int arcs[MAX_OPERATIONS][MAX_OPERATIONS];

	struct operation *last_op;
	int makespan;
};

struct factory {
	int id;
	int num_machines;
	int *end_times;
	int num_ops;
	int *ops_order;
};

struct node {
	int id;
	struct operation *op;
	struct factory *factory;
	struct node *prev;

	int serialized;
	int start_time;
};

struct graph *construct_graph(struct instance *inst);
void destroy_graph(struct graph *graph);

void init_graph(struct graph *graph);

void print_graph(struct graph *graph);

#endif
