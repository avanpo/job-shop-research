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

void destroy_graph(struct graph *graph)
{
	free(graph->nodes);
	free(graph);
}

void init_graph(struct graph *graph)
{
	
}

void print_graph(struct graph *graph)
{
	int i, j;
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
