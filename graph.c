#include <stdio.h>
#include <stdlib.h>

#include "instance.h"
#include "graph.h"
#include "schedule.h"

static int serialize_node(struct graph *graph, struct node *node, int *start_times);

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
		t->num_ops = inst->types[i].num_ops;
		t->ops_order = calloc(inst->types[i].num_ops, sizeof(int));
	}

	for (i = 0; i < inst->num_ops; ++i, ++n) {
		n->id = i;
		n->op = inst->ops + i;
		if (n->op == n->op->job->ops) {
			n->prev = NULL;
		} else {
			n->prev = n - 1;
		}
		n->type = &g->types[n->op->type];
	}

	return g;
}

void destroy_graph(struct graph *graph)
{
	int i;
	for (i = 0; i < graph->num_types; ++i) {
		free(graph->types[i].end_times);
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
}

void serialize_graph(struct graph *graph)
{
	struct node *n;

	int *progress = calloc(graph->num_types, sizeof(int));
	int *serialized = calloc(graph->num_nodes, sizeof(int));
	int *start_times = calloc(graph->num_nodes, sizeof(int));

	int i, j, l, makespan = 0;
	for (i = graph->num_nodes, j = 0; i; j %= graph->num_types) {
		if (progress[j] == graph->types[j].num_ops) {
			++j;
			continue;
		}
		n = graph->nodes + graph->types[j].ops_order[progress[j]];
		if (n->prev == NULL || serialized[n->prev->id]) {
			l = serialize_node(graph, n, start_times);
			serialized[n->id]++;

			if (l > makespan) {
				makespan = l;
			}

			++progress[j];
			--i;
		} else {
			++j;
		}
	}
	graph->schedule->makespan = makespan;

	free(progress);
	free(serialized);
	free(start_times);
}

void print_graph(struct graph *graph)
{
	int i, j;
	for (i = 0; i < graph->num_types; ++i) {
		printf("Type %d: %d machines\n", i, graph->types[i].num_machines);
		printf("  Operation priority:");
		for (j = 0; j < graph->types[i].num_ops; ++j) {
			printf(" %2d", graph->types[i].ops_order[j]);
		}
		printf("\n");
	}
}

static int serialize_node(struct graph *graph, struct node *node, int *start_times)
{
	int job_release = 0;
	if (node->prev != NULL) {
		job_release = start_times[node->prev->id] + node->prev->op->proc_time +
			node->prev->op->idle_time;
	}

	int machine = 0;
	int machine_release = node->type->end_times[machine];
	int t;
	for (t = 0; t < node->type->num_machines; ++t) {
		if (node->type->end_times[t] < machine_release) {
			machine_release = node->type->end_times[t];
			machine = t;
		}
	}

	int start_time = job_release > machine_release ? job_release : machine_release;
	int finish_time = start_time + node->op->proc_time;

	graph->schedule->types[node->type->id].machines[machine].op_start_times[node->id] = start_time;
	start_times[node->id] = start_time;
	node->type->end_times[machine] = finish_time;

	return finish_time;
}
