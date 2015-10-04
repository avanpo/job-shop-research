#include <stdio.h>
#include <stdlib.h>

#include "instance.h"
#include "graph.h"
#include "schedule.h"

static void deserialize_graph(struct graph *graph);
static int serialize_node(struct graph *graph, struct node *node);

struct graph *construct_graph(struct instance *inst)
{
	struct graph *g = calloc(1, sizeof(struct graph));
	struct node_type *t = calloc(inst->num_types, sizeof(struct node_type));
	struct node *n = calloc(inst->num_ops, sizeof(struct node));

	g->inst = inst;
	g->num_types = inst->num_types;
	g->types = t;
	g->num_nodes = inst->num_ops;
	g->nodes = n;

	g->schedule = construct_schedule(inst);

	int i, j;
	for (i = 0; i < inst->num_types; ++i, ++t) {
		t->id = i;
		t->num_machines = inst->types[i].num_machines;
		t->end_times = calloc(inst->types[i].num_machines, sizeof(int));
		t->end_ops = calloc(inst->types[i].num_machines, sizeof(int));
		t->num_ops = inst->types[i].num_ops;
		t->ops_order = calloc(inst->types[i].num_ops, sizeof(int));
	}

	for (i = 0; i < inst->num_ops; ++i, ++n) {
		n->id = i;
		n->op = inst->ops + i;
		n->type = &g->types[n->op->type];
		if (n->op == n->op->job->ops) {
			n->prev = NULL;
		} else {
			n->prev = n - 1;
		}
		n->prev_in_path = NULL;
	}

	return g;
}

void destroy_graph(struct graph *graph)
{
	int i;
	for (i = 0; i < graph->num_types; ++i) {
		free(graph->types[i].end_times);
		free(graph->types[i].end_ops);
		free(graph->types[i].ops_order);
	}
	free(graph->types);
	free(graph->nodes);
	destroy_schedule(graph->schedule);
	free(graph);
}

void init_graph(struct graph *graph)
{
	int t, o, i;
	for (t = 0; t < graph->num_types; ++t) {
		for (o = 0, i = 0; o < graph->num_nodes; ++o) {
			if (graph->inst->types[t].ops[o]) {
				graph->types[t].ops_order[i] = o;
				++i;
			}
		}
	}
	serialize_graph(graph);
}

void serialize_graph(struct graph *graph)
{
	deserialize_graph(graph);
	struct node *n, *last;

	int *progress = calloc(graph->num_types, sizeof(int));
	int *serialized = calloc(graph->num_nodes, sizeof(int));

	int i, j, l, makespan = 0;
	for (i = graph->num_nodes, j = 0; i; j %= graph->num_types) {
		if (progress[j] == graph->types[j].num_ops) {
			++j;
			continue;
		}
		n = graph->nodes + graph->types[j].ops_order[progress[j]];
		if (n->prev == NULL || serialized[n->prev->id]) {
			l = serialize_node(graph, n);
			serialized[n->id]++;

			if (l > makespan) {
				makespan = l;
				last = n;
			}

			++progress[j];
			--i;
		} else {
			++j;
		}
	}
	graph->last = last;
	graph->schedule->makespan = makespan;

	free(progress);
	free(serialized);
}

void swap_operations(struct node *n1, struct node *n2)
{
	if (n1->type != n2->type) {
		fprintf(stderr, "Cannot swap operations %d and %d: machine types do not match.\n", n1->id, n2->id);
		exit(EXIT_FAILURE);
	} else if (n1->id == n2->id) {
		fprintf(stderr, "Cannot swap operation %d with itself.\n", n1->id);
		exit(EXIT_FAILURE);
	}
	struct node_type *t = n1->type;
	int i, a, b;
	for (i = 0, a = -1, b = -1; i < t->num_ops; ++i) {
		if (t->ops_order[i] == n1->id) {
			a = i;
		} else if (t->ops_order[i] == n2->id) {
			b = i;
		}
	}
	if (a >= 0 && b >= 0) {
		t->ops_order[a] = n2->id;
		t->ops_order[b] = n1->id;
		return;
	}
	fprintf(stderr, "Cannot find operations %d and %d in machine type %d.\n", n1->id, n2->id, t->id);
	exit(EXIT_FAILURE);
}

int get_longest_path(struct graph *graph, int *path)
{
	int *reverse_path = calloc(graph->num_nodes, sizeof(int));
	struct node *n;
	int i, j;
	for (i = 0, n = graph->last; n; ++i, n = n->prev_in_path) {
		reverse_path[i] = n->id;
	}
	for (--i, j = 0; i >= 0; --i, ++j) {
		path[j] = reverse_path[i];
	}
	free(reverse_path);
	return j;
}

void print_graph(struct graph *graph)
{
	printf("Printing graph representation\n");
	int i, j;
	for (i = 0; i < graph->num_types; ++i) {
		printf("type %d: %d machines\n", i, graph->types[i].num_machines);
		printf("  operation priority:");
		for (j = 0; j < graph->types[i].num_ops; ++j) {
			printf(" %2d", graph->types[i].ops_order[j]);
		}
		printf("\n");
	}
}

void print_longest_path(struct graph *graph)
{
	printf("Printing longest path (makespan %d)\n", graph->last->start_time + graph->last->op->proc_time);
	printf("time |  id | job | tp m | proc idle\n");
	printf("-----+-----+-----+------+----------\n");
	int *path = calloc(graph->num_nodes, sizeof(int));
	int path_len = get_longest_path(graph, path);
	struct node *n;
	int i;
	for (i = 0; i < path_len; ++i) {
		n = graph->nodes + path[i];
		printf("%4d |  %02d | %3d |  %1d %1d | %4d %4d\n", n->start_time, n->id, n->op->job->id, n->type->id,
				n->machine, n->op->proc_time, n->op->idle_time);
	}
	printf("-----+-----+-----+------+----------\n");
	free(path);
}

static int serialize_node(struct graph *graph, struct node *node)
{
	// calculate time at which earlier job operations
	// are finished
	int job_release = 0;
	if (node->prev != NULL) {
		job_release = node->prev->start_time + node->prev->op->proc_time +
			node->prev->op->idle_time;
	}

	// calculate time at which at least 1 machine is
	// available
	int machine = 0;
	int machine_release = node->type->end_times[machine];
	int t;
	for (t = 0; t < node->type->num_machines; ++t) {
		if (node->type->end_times[t] < machine_release) {
			machine_release = node->type->end_times[t];
			machine = t;
		}
	}

	int start_time;
	if (job_release >= machine_release) {
		start_time = job_release;
		node->prev_in_path = node->prev;
	} else {
		start_time = machine_release;
		node->prev_in_path = graph->nodes + node->type->end_ops[machine];
	}
	int finish_time = start_time + node->op->proc_time;

	graph->schedule->types[node->type->id].machines[machine].op_start_times[node->id] = start_time;
	node->start_time = start_time;
	node->machine = machine;
	node->type->end_times[machine] = finish_time;
	node->type->end_ops[machine] = node->id;

	return finish_time;
}

static void deserialize_graph(struct graph *graph)
{
	int i, j;
	for (i = 0; i < graph->num_types; ++i) {
		for (j = 0; j < graph->types[i].num_machines; ++j) {
			graph->types[i].end_times[j] = 0;
			graph->types[i].end_ops[j] = 0;
		}
	}
}
