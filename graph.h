#ifndef GRAPH_H
#define GRAPH_H

#include "instance.h"
#include "schedule.h"

struct graph {
	struct instance *inst;
	int blocking;
	int neighborhood;

	int num_types;
	struct node_type *types;
	int num_nodes;
	struct node *nodes;

	struct node_type *type_backup;

	struct node *last;
	struct schedule *schedule;
};

struct node_type {
	struct node *nodes;

	int id;
	int num_machines;
	int num_ops;
	int *ops_order;
	int *ops_order_backup;

	// for serialization
	int prev_start_time;
	int prev_start_op;
	int *blocked;
	int *end_times;
	int *end_ops;
};

struct node {
	int id;
	struct operation *op;
	struct node_type *type;
	struct node *prev;
	struct node *next;
	int order_index;

	// for serialization
	struct node *prev_in_path;
	int start_time;
	int machine;
};

struct graph *construct_graph(struct instance *inst, int blocking, int neighborhood);
void destroy_graph(struct graph *graph);

void init_graph(struct graph *graph);
int serialize_graph(struct graph *graph);

void neighborhood_naive(struct node *n);
void neighborhood_left_shift(struct graph *graph, struct node *n_ref);
void neighborhood_crit_path(struct node *n1, struct node *n2);
void reverse_swap(struct graph *graph, struct node_type *type);
int get_longest_path(struct graph *graph, int *path);

void print_graph(struct graph *graph);
void print_type(struct node_type *t, int backup);
void print_longest_path(struct graph *graph);

#endif
