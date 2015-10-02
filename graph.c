#include <stdio.h>
#include <stdlib.h>

#include "instance.h"
#include "graph.h"

struct graph *construct_graph(struct instance *inst)
{
	struct graph *g = calloc(1, sizeof(struct graph));
	struct factory *f = calloc(inst->num_types, sizeof(struct factory));
	struct node *n = calloc(inst->num_ops, sizeof(struct node));

	g->inst = inst;
	g->num_factories = inst->num_types;
	g->factories = f;
	g->num_nodes = inst->num_ops;
	g->nodes = n;

	int i, j;
	for (i = 0; i < inst->num_types; ++i, ++f) {
		f->id = i;
		f->num_machines = inst->types[i].num_machines;
		f->end_times = calloc(inst->types[i].num_machines, sizeof(int));
		f->num_ops = inst->types[i].num_ops;
		f->ops_order = calloc(inst->types[i].num_ops, sizeof(int));
	}

	for (i = 0; i < inst->num_ops; ++i, ++n) {
		n->id = i;
		n->op = inst->ops + i;
		if (n->op == n->op->job->ops) {
			n->prev = NULL;
		} else {
			n->prev = n - 1;
		}
		n->factory = &g->factories[n->op->type];
	}

	for (i = 0; i < inst->num_jobs; ++i) {
		for (j = 0; j < inst->jobs[i].num_ops - 1; ++j) {
			g->arcs[inst->jobs[i].ops[j].id][inst->jobs[i].ops[j + 1].id] = JOB_ARC;
		}
	}

	return g;
}

void destroy_graph(struct graph *graph)
{
	int i;
	for (i = 0; i < graph->factories[i].num_machines; ++i) {
		free(graph->factories[i].end_times);
		free(graph->factories[i].ops_order);
	}
	free(graph->factories);
	free(graph->nodes);
	free(graph);
}

void init_graph(struct graph *graph)
{
	int t, o, i;
	for (t = 0; t < graph->num_factories; ++t) {
		for (o = 0, i = 0; o < graph->num_nodes; ++o) {
			if (graph->inst->types[t].ops[o]) {
				graph->factories[t].ops_order[i] = o;
				++i;
			}
		}
	}
}

int serialize_node(struct node *node)
{
	int job_release = 0;
	if (node->prev != NULL) {
		job_release = node->prev->start_time + node->prev->op->proc_time +
			node->prev->op->idle_time;
	}

	int machine = 0;
	int machine_release = node->factory->end_times[machine];
	int f;
	for (f = 0; f < node->factory->num_machines; ++f) {
		if (node->factory->end_times[f] < machine_release) {
			machine_release = node->factory->end_times[f];
			machine = f;
		}
	}

	node->serialized = 1;
	node->start_time = job_release > machine_release ? job_release : machine_release;

	node->factory->end_times[machine] += node->op->proc_time;
	return node->factory->end_times[machine];
}

void serialize_graph(struct graph *graph)
{
	int i = graph->num_nodes;
	int j = 0;

	struct node *n;
	int *progress = calloc(graph->num_factories, sizeof(int));

	int longest = 0, l;
	while (i) {
		j %= graph->num_factories;
		if (progress[j] == graph->factories[j].num_ops) {
			++j;
			continue;
		}
		n = graph->nodes + graph->factories[j].ops_order[progress[j]];
		if (n->prev == NULL || n->prev->serialized) {
			l = serialize_node(n);
			if (l > longest) {
				longest = l;
			}
			++progress[j];
			--i;
		} else {
			++j;
		}
	}
	graph->makespan = longest;
	free(progress);
}

void print_graph(struct graph *graph)
{
	int i, j;
	for (i = 0; i < graph->num_factories; ++i) {
		printf("type %d: %d machines\n", i, graph->factories[i].num_machines);
		printf("  op priority:");
		for (j = 0; j < graph->factories[i].num_ops; ++j) {
			printf(" %d", graph->factories[i].ops_order[j]);
		}
		printf("\n");
	}
	printf("\n");
	printf(" legend: NO_ARC  .\n");
	printf("         JOB_ARC 1\n");
	printf("         ORD_ARC 2\n\n");
	printf(" op |");
	for (j = 0; j < graph->num_nodes; ++j) {
		printf(" %2d", j);
	}
	printf(" | mt\n ---+");
	for (j = 0; j < graph->num_nodes; ++j) {
		printf("---");
	}
	printf("-+---\n");
	for (i = 0; i < graph->num_nodes; ++i) {
		printf(" %2d |", i);
		for (j = 0; j < graph->num_nodes; ++j) {
			if (graph->arcs[i][j]) {
				printf(" %2d", graph->arcs[i][j]);
			} else {
				printf("  .", graph->arcs[i][j]);
			}
		}
		printf(" | %2d\n", graph->nodes[i].op->type);
	}
	printf(" ---+");
	for (j = 0; j < graph->num_nodes; ++j) {
		printf("---");
	}
	printf("-+---\n");
}
