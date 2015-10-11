#ifndef GRAPH_H
#define GRAPH_H

#include "instance.h"
#include "schedule.h"

struct graph {
	struct instance *inst;
	int num_types;
	struct node_type *types;
	int num_nodes;
	struct node *nodes;

	struct node *last;
	struct schedule *schedule;
};

struct node_type {
	int id;
	int num_machines;
	int prev_start_time;
	int prev_start_op;
	int *end_times;
	int *end_ops;
	int num_ops;
	int *ops_order;
	int *ops_order_backup;
};

struct node {
	int id;
	struct operation *op;
	struct node_type *type;
	struct node *prev;

	struct node *prev_in_path;
	int start_time;
	int machine;
};

struct graph *construct_graph(struct instance *inst);
void destroy_graph(struct graph *graph);

void init_graph(struct graph *graph);
void serialize_graph(struct graph *graph);

void swap_operations(struct node *n1, struct node *n2);
void reverse_swap(struct node_type *type);
int get_longest_path(struct graph *graph, int *path);

void print_graph(struct graph *graph);
void print_longest_path(struct graph *graph);

#endif
